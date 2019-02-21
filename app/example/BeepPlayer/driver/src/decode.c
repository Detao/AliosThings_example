#include "decode.h"

static const uint16_t freq_tab[12] = {262, 277, 294, 311, 330, 349, 369, 392, 415, 440, 466, 494}; //原始频率表 CDEFGAB
static const uint8_t sign_tab[7]  = {0, 2, 4, 5, 7, 9, 11};     //1~7在频率表中的位置
static const uint8_t length_tab[7] = {1, 2, 4, 8, 16, 32, 64};  //几分音符 2^0,2^1,2^2,2^3...
static uint16_t freq_tab_new[12]; //新的频率表

/*signature| 调号（0-11）  ： 是指乐曲升多少个半音演奏 */
/*octachord| 升降八度（-2到+2）：<0 降几个八度；>0 升几个八度*/
static void beep_song_decode_new_freq(uint8_t signature,uint8_t octachord)
{
    uint8_t i,j;
    for(i = 0; i<12 ; i++){
        j = i + signature;

        if(j > 11){
            freq_tab_new[i] = freq_tab[i]*2;
        }else{
            freq_tab_new[i] = freq_tab[i] + signature;
        }
        /* 升降八度 */
        if(octachord < 0){
            freq_tab_new[i] >>= (-octachord);
        }else{
            freq_tab_new[i] <<= octachord;

        }
    }

}
/*********************************************/
/* tone[in] 音高                */
/* length[in] 音长   */
/* freq[out] 解码后的频率  */
/* sond_lenth[out] 发声的时间    */
/* nosound_length[out] 不发声的时间  */
/********************************************/
static void beep_song_decode(uint16_t tone,uint16_t length, uint16_t *freq,uint16_t *sound_len,uint16_t *nosound_len)
{
    static const uint16_t div0_len = SEMIBREVE_LEN ;  //全音符长度
    uint16_t current_freq,note_len,note_sound_len;
    uint8_t note,range,sharp,note_div,effect,dotted;

    note = tone % 10;   //音符
    range = tone / 10 % 10;  //音区
    sharp = tone /100 ;      ///是否升半音
    current_freq = freq_tab_new[sign_tab[note-1] + sharp];

    if(note != 0){
        if(range == 1){
            current_freq >>= 1; //降八度
        }
        if(range == 3){
            current_freq <<= 1; //升八度
        }
        *freq = current_freq;
    } else{
        *freq = 0;
    }
    note_div =length_tab[length % 10];  //算出几分音符
    effect = length / 10 % 10;      //算出音符类型(0 普通 1 连音 2 顿音)
    dotted = length / 100 ;             //算出是否附点
    note_len = div0_len / note_div;

    if(dotted != 1){
        note_len = note_len + note_len/2;
    }

    if(effect != 1){
        if(effect == 0){
            note_sound_len = note_len * SOUND_SPACE;
        }else{
            note_sound_len = note_len /2 ;
        }
    }else{
        note_sound_len = note_len;
    }

    if(note == 0){
        note_sound_len = 0;
    }
    *sound_len = note_sound_len;
    *nosound_len = note_len - note_sound_len;

}

uint16_t beep_song_get_data(const beep_song *song ,uint16_t index ,beep_song_data *data)
 {
    beep_song_decode(song->data[index * 2],song->data[index *2 + 1],&data->freq,&data->nosound_len,&data->sound_len);
    return 0;
}

uint16_t beep_song_get_len(const beep_song *song)
{
    uint16_t cnt = 0;
    /* 歌曲已0x00 0x00 结尾，表示结束标志 */
    while(song->data[cnt]){
        cnt+=2;
    }
    return cnt/2;
}

int32_t beep_song_get_name(const beep_song *song ,char *name)
{
    int i=0;
    if(name == NULL)
    {
        return -1;
    }
    while(song->name[i]){
        name[i] = song->name[i];
        i++;
    }
    name[i] = '\0';

    return 0;
}

int beep_song_decode_init()
{
    beep_song_decode_new_freq(SOUND_SIGNATURE,SOUND_OCTACHORD);
    return 0;
}