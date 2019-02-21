#ifndef __HAL_PWM_STM32F1_H_
#define __HAL_PWM_STM32F1_H_


#define PORT_TIM_MAX_NUM   4 - 1
#define PORT_ch_MAX_NUM     4
#define PWM_PSC           (2000)
#define PWM_CLK           (72000000)

#define CH_IN_GROUP   4
#define TIM_2 0
#define TIM_3 1
#define TIM_4 2
#define TIM_5 3

/*tim channel*/
#define CH_1  0
#define CH_2  1
#define CH_3  2
#define CH_4  3

#define HAL_PWM_1          ((uint8_t)0)    /* tim2 pwm ch1 */
#define HAL_PWM_2          ((uint8_t)1)    /* tim2 pwm ch2 */
#define HAL_PWM_3          ((uint8_t)2)    /* tim2 pwm ch3 */
#define HAL_PWM_4          ((uint8_t)3)    /* tim2 pwm ch4 */

#define HAL_PWM_5          ((uint8_t)4)    /* tim3 pwm ch1 */
#define HAL_PWM_6          ((uint8_t)5)    /* tim3 pwm ch2 */
#define HAL_PWM_7          ((uint8_t)6)    /* tim3 pwm ch3 */
#define HAL_PWM_8          ((uint8_t)7)    /* tim3 pwm ch4 */

#define HAL_PWM_9          ((uint8_t)8)    /* tim4 pwm ch1 */
#define HAL_PWM_10          ((uint8_t)9)    /* tim4 pwm ch2 */
#define HAL_PWM_11          ((uint8_t)10)    /* tim4 pwm ch3 */
#define HAL_PWM_12          ((uint8_t)11)    /* tim4 pwm ch4 */

#define HAL_PWM_13          ((uint8_t)12)    /* tim5 pwm ch1 */
#define HAL_PWM_14          ((uint8_t)13)    /* tim5 pwm ch2 */
#define HAL_PWM_15          ((uint8_t)14)    /* tim5 pwm ch3 */
#define HAL_PWM_16          ((uint8_t)15)    /* tim5 pwm ch4 */

#define HAL_PWM_17          ((uint8_t)0)    /* tim1 pwm ch1 */
#define HAL_PWM_18          ((uint8_t)0)    /* tim1 pwm ch1 */



#endif/* __HAL_PWM_STM32F1_H_ */