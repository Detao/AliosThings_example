#include "key.h"
#include "button.h"
#include "player.h"

static my_button btn_last,btn_next,btn_play; 
extern Player player;

static void beep_key_press_long(uint32_t pin)
{
    uint8_t volume;

    switch(pin)
    {
        case KEY_PLAY_PIN:
            break;
        case KEY_NEXT_PIN:
            player_control(&player,PLAYER_CMD_GET_VOL,&volume);
            if(volume<99){
                volume++;
                player_control(&player,PLAYER_CMD_SET_VOL,&volume);
            }
            break;
        case KEY_LAST_PIN:
            player_control(&player,PLAYER_CMD_GET_VOL,&volume);
            if(volume>1){
                volume--;
                player_control(&player,PLAYER_CMD_SET_VOL,&volume);
            }
            break;
    }
}
static void beep_key_press_short(uint32_t pin)
{
    switch (pin)
    {
        case KEY_PLAY_PIN:
            if(player.status == PLAYER_RUNNING){
                player_control(&player,PLAYER_CMD_STOP,NULL);
            }else{
                player_control(&player,PLAYER_CMD_PLAY,NULL);
            }
            player_show(&player);
            break;
        case KEY_LAST_PIN:
            player_control(&player,PLAYER_CMD_LAST,NULL);
            player_show(&player);
            break;
        case KEY_NEXT_PIN:
            player_control(&player,PLAYER_CMD_NEXT,NULL);
            player_show(&player);
            break;
    
        default:
            break;
    }
}
void btn_cb(my_button *button)
{
    switch (button->event)
    {
        case BUTTON_EVENT_CLICK_UP:
            printf("BUTTON_EVET_CLICK_UP\n");
            beep_key_press_short(button->pin.port);
            break;
        case BUTTON_EVENT_HOLD_CYC:
            beep_key_press_long(button->pin.port);
            break;
        default:
            break;
    }

}

int key_init()
{
    btn_last.press_logic_level = KEY_PRESS_LEVEL;
    btn_last.hold_cyc_period = 100;
    btn_last.cb = (my_button_callback) btn_cb;

    btn_next = btn_play = btn_last;

    btn_last.pin.port = KEY_LAST_PIN;
    btn_last.pin.config = INPUT_PULL_UP;
    btn_play.pin.port = KEY_PLAY_PIN;
    btn_play.pin.config = INPUT_PULL_UP;
    btn_next.pin.port = KEY_NEXT_PIN;
    btn_play.pin.config = INPUT_PULL_UP;

    my_button_register(&btn_last);
    my_button_register(&btn_next);
    my_button_register(&btn_play);
    my_button_start();

    return 0;
}