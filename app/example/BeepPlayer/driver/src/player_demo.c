
#include <aos/aos.h>
#include "app_entry.h"

#include "player.h"
#include "song_data.h"
#include "decode.h"
#include "beep.h"

Player player;
audio_ops audio;
decode_ops decode;

uint8_t beep_volume = 1;

int decode_read(void *song ,int index,void *buffer,int size)
{
    beep_song_get_data(song,index,buffer);
    return 1;
}
int audio_write(void *buffer,int size)
{
    beep_song_data *data = buffer;
    beep_on();
    beep_set(data->freq,beep_volume);
    aos_msleep(data->sound_len);
    beep_off();
    aos_msleep(data->nosound_len);

    return size;
}

int decode_control(void *song ,int cmd, void *arg)
{
    if(cmd == DECODE_OPS_CMD_GET_NAME){
        beep_song_get_name(song,arg);
    }else if(cmd == DECODE_OPS_CMD_GET_LEN){
        *(uint16_t *)arg = beep_song_get_len(song);
    }
    return 0;
}

int audio_control(int cmd, void *arg)
{
    if(cmd == AUDIO_OPS_CMD_SET_VOL)
        beep_volume = *(uint8_t *)arg;
    return beep_volume;
}

int player_init(void )
{
    decode.init = beep_song_decode_init;
    decode.read = decode_read;
    decode.control = decode_control;

    audio.init = beep_init;
    audio.open = beep_on;
    audio.close = beep_off;
    audio.write = audio_write;
    audio.control = audio_control;

    player.decode = &decode;
    player.audio = &audio;

    player_add_song(&player,(void *)&song1);
    player_add_song(&player,(void *)&song2);
    player_add_song(&player,(void *)&song3);
    player_add_song(&player,(void *)&song4);
    player_start(&player);

    player_control(&player,PLAYER_CMD_PLAY,NULL);
    player_show(&player);

    return 0;
}