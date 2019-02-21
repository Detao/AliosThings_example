#include "led.h"

#include <hal/soc/soc.h>
#include <aos/aos.h>

gpio_dev_t led;

int led_init()
{
    led.port = LED_PIN;
    led.config = OUTPUT_PUSH_PULL;
    hal_gpio_init(&led);
    return 0;
}
int led_on()
{
    hal_gpio_output_high(&led);
    return 0;
}
int led_off()
{
    hal_gpio_output_low(&led);
    return 0;
}
int led_toggle()
{
    hal_gpio_output_toggle(&led);
    return 0;
}