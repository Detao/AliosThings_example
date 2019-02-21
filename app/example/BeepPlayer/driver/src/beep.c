#include "beep.h"
#include <hal/soc/soc.h>
#include "hal_pwm_stm32f1.h"

pwm_dev_t pwm;

int beep_init()
{   
    pwm.port=HAL_PWM_6;
    pwm.config.duty_cycle=50;
    pwm.config.freq=2000;
    hal_pwm_init(&pwm);
    return 0;

}

int beep_on()
{
    hal_pwm_start(&pwm);   
    return 0;

}
int beep_off()
{
    hal_pwm_stop(&pwm);
    return 0;
}
int beep_set(uint16_t freq, uint16_t volume)
{
    pwm_config_t para;
    para.duty_cycle = volume;
    para.freq = freq;
    hal_pwm_para_chg(&pwm,para);
    return 0;
}
