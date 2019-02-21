#ifndef __DECODE_H_
#define __DECODE_H_

#include <aos/aos.h>

#define SONG_NAME_LENGTH_MAX 30 
#define SONG_DATA_LENGTH_MAX 500

#define SEMIBREVE_LEN (1600)

#define SOUND_SIGNATURE 0    //调号： 升半个音
#define SOUND_OCTACHORD 1   //升将八度 ：升一个八度
#define SOUND_SPACE (4/5)   //定义普通音符演奏的长度分率，// 每4分音符间隔

typedef struct  beep_song_data 
{
    uint16_t freq;
    uint16_t sound_len;
    uint16_t nosound_len;
}beep_song_data;

typedef struct beep_song{
    const uint8_t name[SONG_NAME_LENGTH_MAX];
    const uint8_t data[SONG_DATA_LENGTH_MAX];
}beep_song;

uint16_t beep_song_get_len(const beep_song *song);
int32_t beep_song_get_name(const beep_song *song , char *name);
uint16_t beep_song_get_data(const beep_song *song ,uint16_t index ,beep_song_data *data);
int beep_song_decode_init();

#endif /* __DECODE_H_ */