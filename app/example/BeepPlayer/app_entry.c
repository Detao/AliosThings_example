#include "app_entry.h"
#include "led.h"
#include "button.h"
#include "hal_pwm_stm32f1.h"
#include <hal/soc/soc.h>
#include "decode.h"
#include "key.h"

static void app_delay_action(void *arg)
{
    LOG("This is Devin Test %s:%d %s",__func__,__LINE__,aos_task_name());
    aos_post_delayed_action(5000,app_delay_action,NULL);

}
uint16_t dir=1;
static void led_delay_action(void *arg)
{
   // led_toggle();
   if(arg==NULL){
       return ;
   }
   pwm_dev_t *pwm = (pwm_dev_t*)arg;
   
//    if(dir) pwm->config.duty_cycle++;
//    else    pwm->config.duty_cycle--;
   
//    if(pwm->config.duty_cycle > 99) dir = 0;
//    if(pwm->config.duty_cycle == 0) dir = 1;
   pwm->config.duty_cycle = 1;
   pwm->config.freq = 1000;
   hal_pwm_para_chg(pwm,pwm->config);
   aos_post_delayed_action(10,led_delay_action,pwm);
}
static void button_cb(my_button *button)
{
    switch(button->event){
        case BUTTON_EVENT_CLICK_DOWN:
            printf("BUTTON_EVENT_CLICK_DOWN\r\n");
            break;
        case BUTTON_EVENT_HOLD_CYC:
            printf("BUTTON_EVENT_HOLD_CYC\r\n");
            break;
        default:
            ;
    }
    printf("button_call_back\r\n");
}

// void paly_action()
// {
//     beep_song_data data;
//     uint16_t i=0, len;
//     char name[20];
//     beep_song_get_name(&song1,&name);
//     printf("正在播放： %s",name);
//     len = beep_song_get_len(&song1);
//     while(i < len){
//         beep_song_get_data(&song1,i,&data);
//         beep_set(data.freq,1);
//         beep_on();
//         aos_msleep(data.sound_len);
//         beep_off();
//         aos_msleep(data.nosound_len);
//         i++;
//     }

// }

int application_start()
{
    LOG("application start.");
    led_init();
    pwm_dev_t pwm;
    //led_on();
    // my_button button;
    // button.pin.port = BUTTON_PIN;
    // button.pin.config = OUTPUT_PUSH_PULL;
    // button.press_logic_level=1;
    // button.cb=&button_cb;

    // my_button button1;
    // button1.pin.port = BUTTON_PIN1;
    // button1.pin.config = INPUT_PULL_UP;
    // button1.press_logic_level=0;
    // button1.cb=&button_cb;
    // pwm.port=HAL_PWM_6;
    // pwm.config.duty_cycle=50;
    // pwm.config.freq=2000;
    // hal_pwm_init(&pwm);
    // hal_pwm_start(&pwm);
    // beep_init();
    // beep_song_decode_init();
  
  
    player_init();
    key_init();
    //my_button_register(&button);
    // my_button_register(&button1);
    // my_button_start();
   // aos_post_delayed_action(1000,app_delay_action,NULL);
    //aos_post_delayed_action(1000,led_delay_action,&pwm);
    //aos_post_delayed_action(1000,paly_action,NULL);
    aos_loop_run();

    return 0;
}