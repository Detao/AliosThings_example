#ifndef __BEEP_H_
#define __BEEP_H_

#include <aos/aos.h>

#define BEEP_PWM   "PWM1"

int beep_init();
int beep_on();
int beep_off();
int beep_set(uint16_t freq ,uint16_t volume);

#endif /* __BEEP_H_ */