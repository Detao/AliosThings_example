#include "player.h"

void player_entry(void *parameter)
{
    if(parameter == NULL){
        return ;
    }
    Player_t player = (Player_t) parameter;
    uint8_t buffer[PLAYER_BUFFER_SIZE],size;
    while(1){
      if(player->status == PLAYER_RUNNING){
          size = player->song_time_all - player->song_time_pass;
          if(size > PLAYER_BUFFER_SIZE)  size = PLAYER_BUFFER_SIZE;
          size = player->decode->read(player->song_sheet[player->song_current - 1],player->song_time_pass,buffer,size);
          if(size > 0){
              player->audio->write(buffer,size);
              player->song_time_pass += size;
          }
          if(player->song_time_pass > player->song_time_all){
              player_next(player);
              player_show(player);
          }
      }else{
          player->audio->close();

          aos_sem_wait(&player->sem_play,AOS_WAIT_FOREVER);

          player->audio->open();

        }

    }
}

int player_play(Player_t player)
{
    if(player->status != PLAYER_RUNNING){

        krhino_intrpt_enter();
        player->status = PLAYER_RUNNING;
       
        krhino_intrpt_exit();

        aos_sem_signal(&player->sem_play);
    }
    return 0;
}
int player_stop(Player_t player)
{
    if(player->status != PLAYER_STOP){
        krhino_intrpt_enter();

        player->status = PLAYER_STOP;

        krhino_intrpt_exit();
    }
    return 0;
}
int player_last(Player_t player)
{
    uint16_t len;

    krhino_intrpt_enter();
    if(player->song_current > 1){
        player->song_current--;
    }else{
        player->song_current = player->song_num;
    }
    krhino_intrpt_exit();

    player->song_time_pass = 0;
    player->decode->control(player->song_sheet[player->song_current -1],DECODE_OPS_CMD_GET_LEN,&len);

    krhino_intrpt_enter();
    player->song_time_all = len;
    krhino_intrpt_exit();

    if(player->status != PLAYER_RUNNING){
        player_play(player);
    }
    return 0;
}
int player_next(Player_t player)
{
    uint16_t len;
    
    krhino_intrpt_enter();
    if(player->song_current < player->song_num){
        player->song_current++;
    }else{
        player->song_current = 1;
    }
    krhino_intrpt_exit();

    player->song_time_pass = 0;
    player->decode->control(player->song_sheet[player->song_current - 1],DECODE_OPS_CMD_GET_LEN,&len);

    krhino_intrpt_enter();
    player->song_time_all = len;
    if(player->status != PLAYER_RUNNING){
        player_play(player);
    }
    krhino_intrpt_exit();
    return 0;
}
int player_control(Player_t player,int cmd, void *arg)
{
 
    switch(cmd){
        case PLAYER_CMD_PLAY:
            player_play(player);
            break;
        case PLAYER_CMD_STOP:
            player_stop(player);
            break;
        case PLAYER_CMD_LAST:
            player_last(player);
            break;
        case PLAYER_CMD_NEXT :
            player_next(player);
            break;
        case PLAYER_CMD_SET_VOL:
            krhino_intrpt_enter();
            player->volume = *(uint8_t *)arg;
            krhino_intrpt_exit();
            player->audio->control(AUDIO_OPS_CMD_SET_VOL,&player->volume);
            break;
        case PLAYER_CMD_GET_VOL:
            *(uint8_t *)arg = player->volume;
            break;
        case PLAYER_CMD_GET_STATUS:
            *(uint8_t *)arg = player->status;
            break;
    }
    return 0;

}
int player_show(Player_t player)
{
    uint16_t percent;
    char name[PLAYER_SONG_NUM_MAX +1];
    printf("******************BEEP Player*******************\n");
    /* 打印歌单 */
    for(int i = 0 ; i < player->song_num; i++ ){
        player->decode->control(player->song_sheet[i],DECODE_OPS_CMD_GET_NAME,name);
        printf("%02d. %s\n" , i+1 ,name);
    }
    
    /* 打印播放状态 */
    if(player->status == PLAYER_RUNNING){
        printf("<--------Playing: ");
    }else{
        printf("<---------Pause: ");
    }
    /*打印当前歌曲*/
    player->decode->control(player->song_sheet[player->song_current -1 ],DECODE_OPS_CMD_GET_NAME,name);
    printf("%s",name);
    printf("---------->\n");
    /*打印播放进度*/
    percent = player->song_time_pass *100 / player->song_time_all;
    printf("Progress: %02d%%  Volume: %02d%% \n",percent,player->volume);
    printf("*************************************************\n");

    return 0;
}

int player_add_song(Player_t player,void *song)
{
    if(player->song_num == PLAYER_SONG_NUM_MAX){
        return -1;
    }
    krhino_intrpt_enter();
    player->song_sheet[player->song_num] = song;
    player->song_num++;
    krhino_intrpt_exit();
    return 0;
}


int player_start(Player_t player)
{
    uint8_t len;
    static uint8_t inited = 0;

    if(inited == 1){
        return -1;
    }

    if(player->song_num == 0){
        return -1;
    }

    player->decode->init();
    player->status = PLAYER_STOP;
    player->song_time_pass = 0;
    player->song_current = 1;
    player->volume = PLAYER_SOUND_SIZE_DEFAULT;
    player->decode->control(player->song_sheet[player->song_current - 1],DECODE_OPS_CMD_GET_LEN,&len);
    player->song_time_all = len;

    player->audio->init();
    player->audio->control(AUDIO_OPS_CMD_SET_VOL,&player->volume);

    if(aos_sem_new(&player->sem_play,1) != 0){
        return -1;
    }
    //aos_task_new_ext(&player->play_thread,"play_thread",player_entry,player,512,20);
    if(aos_task_new_ext(&player->play_thread,"play_thread",player_entry,player,512,20) != 0){
        aos_sem_free(&player->sem_play);
        return -1;
    }
    inited = 1;
    return 0;
}