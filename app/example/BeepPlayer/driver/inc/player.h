#ifndef __PLAYER_H_
#define __PLAYER_H_

#include <aos/aos.h>

#define PLAYER_BUFFER_SIZE   20
#define PLAYER_SOUND_SIZE_DEFAULT 1

typedef enum PLAYER_STATUS{
    PLAYER_RUNNING, //正在播放
    PLAYER_STOP    //播放停止
}PLAYER_STATUS;

enum AUDIO_OPS_CMD{
    AUDIO_OPS_CMD_SET_VOL
};

enum DECODE_OPS_CMD{
    DECODE_OPS_CMD_GET_NAME,
    DECODE_OPS_CMD_GET_LEN
};

enum PLAYER_CMD{
    PLAYER_CMD_PLAY,
    PLAYER_CMD_STOP,
    PLAYER_CMD_LAST,
    PLAYER_CMD_NEXT,
    PLAYER_CMD_SET_VOL,
    PLAYER_CMD_GET_VOL,
    PLAYER_CMD_GET_STATUS
};
#define PLAYER_SONG_NUM_MAX  10
typedef struct audio_ops{
    int (*init)(void);
    int (*open)(void);
    int (*close)(void);
    int (*control)(int cmd,void *arg);
    int (*write)(void *buffer, int size);

}audio_ops;


typedef struct decode_ops{
    int (*init)(void);
    int (*control)(void* song,int cmd,void *arg);
    int (*read)(void *song ,int index,void *buffer,int size);

}decode_ops;

typedef struct Player{
    PLAYER_STATUS  status;
    uint8_t        volume;          //音量
    uint8_t        song_current;     // 当前播放歌曲
    uint8_t        song_num;          //歌曲总数
    uint16_t       song_time_pass;    //已经播放时间
    uint16_t       song_time_all;     // 播放总时间
    void           *song_sheet[PLAYER_SONG_NUM_MAX];  //歌曲列表

    aos_sem_t       sem_play;
    aos_task_t      play_thread;
    audio_ops       *audio;
    decode_ops      *decode;

}Player,*Player_t;

int player_start(Player_t player);
int player_add_song(Player_t player,void *song);
int player_show(Player_t player);
int player_control(Player_t player,int cmd, void *arg);
#endif /* __PLAYER_H_ */