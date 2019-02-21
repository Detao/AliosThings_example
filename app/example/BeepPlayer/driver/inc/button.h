#ifndef __BUTTON_H_
#define __BUTTON_H_

#include <aos/aos.h>
#include <hal/soc/soc.h>

#define MY_BUTTON_LIST_MAX     10
#define MY_BUTTON_DOWN_MS         50
#define MY_BUTTON_HOLD_MS    700
#define MY_BUTTON_SCAN_SPACE_MS   20



typedef void (*my_button_callback)(void*);

typedef enum my_button_event{

    BUTTON_EVENT_CLICK_DOWN, /* 按键单击按下事件 */
    BUTTON_EVENT_CLICK_UP,   /* 按键单击结束事件 */
    BUTTON_EVENT_HOLD,       /* 按键长按开始事件 */
    BUTTON_EVENT_HOLD_CYC,   /* 按键长按周期性触发事件 */
    BUTTON_EVENT_HOLD_UP,    /* 按键长按结束事件 */
    BUTTON_EVENT_NONE        /* 无按键事件 */
}my_button_event;

typedef struct my_button{
    uint8_t             press_logic_level; /* 按键按下的电平 */
    uint16_t            cnt;               /* 连续扫描到按下状态的次数 */
    uint16_t            hold_cyc_period;   /* 长按周期回调的周期 */
    gpio_dev_t          pin;               /* 按键对应的pin编号 */
    my_button_event     event;             /* 按键出发的事件 */
    my_button_callback  cb;                /* 按键触发的回调函数 */           

}my_button_t,my_button;



int my_button_register(my_button_t *button);
int my_button_start();

#endif/* __BUTTON_H_ */