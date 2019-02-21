#include "button.h"
#include <k_api.h>

#define MY_BUTTON_CALL(func, argv)    \
do{                                 \
    if(func != NULL) func argv;     \
}while(0);

typedef struct my_button_manage{
    uint8_t            num;         /* 已注册的按键数 */
    timer_dev_t        timer;       /* 按键扫描用到的定时器 */
    my_button_t        *button_list[MY_BUTTON_LIST_MAX]; /*存储按键的指针 */
}my_button_manage;

static my_button_manage button_manage;


int my_button_register(my_button_t *button)
{
  
    hal_gpio_init(&button->pin);

    if(button->press_logic_level == 0){
        /*设置io口状态*/
        
       // hal_gpio_output_high(&button->pin);

    }else{
        /*set io status*/
       
       // hal_gpio_output_low(&button->pin);
    }
    button->cnt = 0;
    button->event = BUTTON_EVENT_NONE;

    button_manage.button_list[button_manage.num++] = button;
    return 0;
}

static void my_button_scanf(void *param)
{
    uint8_t i;
    uint16_t cnt_old,press_level;
    for(i = 0 ;i< button_manage.num; i++){
        cnt_old = button_manage.button_list[i]->cnt;
        hal_gpio_input_get(&(button_manage.button_list[i]->pin),&press_level);
        if(press_level == button_manage.button_list[i]->press_logic_level){
            button_manage.button_list[i]->cnt++;
            if(button_manage.button_list[i]->cnt == MY_BUTTON_DOWN_MS / MY_BUTTON_SCAN_SPACE_MS) {
                /* BUTTON_DOWN */
                button_manage.button_list[i]->event = BUTTON_EVENT_CLICK_DOWN;
                MY_BUTTON_CALL(button_manage.button_list[i]->cb,(button_manage.button_list[i]));
            }else if(button_manage.button_list[i]->cnt == MY_BUTTON_HOLD_MS / MY_BUTTON_SCAN_SPACE_MS){
                /* BUTTON_HOLD */
                button_manage.button_list[i]->event = BUTTON_EVENT_HOLD;
                MY_BUTTON_CALL(button_manage.button_list[i]->cb,(button_manage.button_list[i]));

            }else if(button_manage.button_list[i]->cnt > MY_BUTTON_HOLD_MS / MY_BUTTON_SCAN_SPACE_MS){
                /* BUTTON_HOLD_CYC*/
                button_manage.button_list[i]->event = BUTTON_EVENT_HOLD_CYC;
                MY_BUTTON_CALL(button_manage.button_list[i]->cb,(button_manage.button_list[i]));
            }

        }else{
            button_manage.button_list[i]->cnt = 0;
            if(cnt_old>MY_BUTTON_DOWN_MS / MY_BUTTON_SCAN_SPACE_MS && cnt_old < MY_BUTTON_HOLD_MS / MY_BUTTON_SCAN_SPACE_MS){
                /* BUTTON_CLICK_UP*/
                button_manage.button_list[i]->event = BUTTON_EVENT_CLICK_UP;
                MY_BUTTON_CALL(button_manage.button_list[i]->cb,(button_manage.button_list[i]));

            }else if(cnt_old>MY_BUTTON_HOLD_MS / MY_BUTTON_SCAN_SPACE_MS){
                /* BUTTON_HOLD_UP */
                button_manage.button_list[i]->event = BUTTON_EVENT_HOLD_UP;
                MY_BUTTON_CALL(button_manage.button_list[i]->cb,(button_manage.button_list[i]));
            }

        }

    }

}
int my_button_start()
{
  
    /* 创建定时器 */
    button_manage.timer.config.period = MY_BUTTON_SCAN_SPACE_MS * 1000 ;
    button_manage.timer.config.reload_mode = TIMER_RELOAD_AUTO;
    button_manage.timer.config.cb  = &my_button_scanf;
    button_manage.timer.config.arg = NULL;

    hal_timer_init(&button_manage.timer);
    
   
    /* 启动定时器 */
    hal_timer_start(&button_manage.timer);

    return 0;
}