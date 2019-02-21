#include "hal/hal.h"
#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"
#include "hal_pwm_stm32f1.h"

 TIM_HandleTypeDef 	 hal_TIM_handle[PORT_TIM_MAX_NUM];
 //TIM_OC_InitTypeDef  hal_ch_handle[PORT_ch_MAX_NUM];     	 
int32_t get_tim_group(pwm_dev_t *pwm, TIM_TypeDef **TIMx)
{
    uint16_t group = 0;
    int32_t ret=0;
    if(pwm == NULL){
        return -1;
    }
    group = pwm->port / CH_IN_GROUP;
    switch(group){
        case TIM_2:
            *TIMx = TIM2;
           break;
        case TIM_3:
            *TIMx = TIM3;
            break;
        case TIM_4:
            *TIMx = TIM4;
            break;
        default:
            ret = -1;
            break;
    }
    return ret;

}

int32_t get_tim_ch(uint32_t ch)
{
    return   ch % CH_IN_GROUP;
}
TIM_OC_InitTypeDef hal_ch_handle;
int32_t hal_pwm_init(pwm_dev_t *pwm)
{
    TIM_HandleTypeDef *pwmhandle;
   
    TIM_TypeDef  *TIMx;
    int32_t ret = -1;
    int16_t ch;
    uint8_t tim;
    if(pwm == NULL){
        return -1;
    }

    //ppwmhandle = &hal_pwm_handle[pwm->port];

    get_tim_group(pwm,&TIMx);
    pwmhandle = &hal_TIM_handle[pwm->port / CH_IN_GROUP];
    

    pwmhandle->Instance = TIMx;          	
    pwmhandle->Init.Prescaler=PWM_CLK/PWM_PSC/(pwm->config.freq) - 1;        
    pwmhandle->Init.CounterMode=TIM_COUNTERMODE_UP;
    pwmhandle->Init.Period=PWM_PSC;       
    pwmhandle->Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(pwmhandle);  

    
    ch = get_tim_ch(pwm->port);
    switch (ch){
        case CH_1:
            hal_ch_handle.OCMode=TIM_OCMODE_PWM1; 
            hal_ch_handle.Pulse=((float)pwm->config.duty_cycle/100)*PWM_PSC;          
            hal_ch_handle.OCPolarity=TIM_OCPOLARITY_LOW; 
            HAL_TIM_PWM_ConfigChannel(pwmhandle,&hal_ch_handle,TIM_CHANNEL_1);
            break;
        case CH_2:
            hal_ch_handle.OCMode=TIM_OCMODE_PWM1; 
            hal_ch_handle.Pulse=((float)pwm->config.duty_cycle/100)*PWM_PSC;          
            hal_ch_handle.OCPolarity=TIM_OCPOLARITY_LOW; 
            HAL_TIM_PWM_ConfigChannel(pwmhandle,&hal_ch_handle,TIM_CHANNEL_2);
            break;
        case CH_3:
            hal_ch_handle.OCMode=TIM_OCMODE_PWM1; 
            hal_ch_handle.Pulse=((float)pwm->config.duty_cycle/100)*PWM_PSC;          
            hal_ch_handle.OCPolarity=TIM_OCPOLARITY_LOW; 
        HAL_TIM_PWM_ConfigChannel(pwmhandle,&hal_ch_handle,TIM_CHANNEL_3);
            break;
        case CH_4:
            hal_ch_handle.OCMode=TIM_OCMODE_PWM1; 
            hal_ch_handle.Pulse=((float)pwm->config.duty_cycle/100)*PWM_PSC;          
            hal_ch_handle.OCPolarity=TIM_OCPOLARITY_LOW; 
            HAL_TIM_PWM_ConfigChannel(pwmhandle,&hal_ch_handle,TIM_CHANNEL_4);
            break;
        default:
            ret = -1;
            break;
    }

    return ret;
   
}


int32_t hal_pwm_start(pwm_dev_t *pwm)
{
    int32_t ret=0;
    TIM_HandleTypeDef *pwmhandle;
    uint16_t ch;
    if(pwm==NULL){
        return -1;
    }
    pwmhandle = &hal_TIM_handle[pwm->port / CH_IN_GROUP];
    ch=get_tim_ch(pwm->port);
     switch (ch){
        case CH_1:
            HAL_TIM_PWM_Start(pwmhandle,TIM_CHANNEL_1);//¿ªÆôPWMÍ¨µÀ2
            break;
        case CH_2:
           HAL_TIM_PWM_Start(pwmhandle,TIM_CHANNEL_2);//¿ªÆôPWMÍ¨µÀ2
            break;
        case CH_3:
            HAL_TIM_PWM_Start(pwmhandle,TIM_CHANNEL_3);//¿ªÆôPWMÍ¨µÀ2
            break;
        case CH_4:
           HAL_TIM_PWM_Start(pwmhandle,TIM_CHANNEL_4);//¿ªÆôPWMÍ¨µÀ2
            break;
        default:
            ret = -1;
            break;
    }     
    return ret;

}


int32_t hal_pwm_stop(pwm_dev_t *pwm)
{
    int32_t ret=0;
    TIM_HandleTypeDef *pwmhandle;
    uint16_t ch;
    if(pwm==NULL){
        return -1;
    }
    pwmhandle = &hal_TIM_handle[pwm->port / CH_IN_GROUP];
    ch = get_tim_ch(pwm->port);
    switch (ch){
        case CH_1:
           HAL_TIM_PWM_Stop(pwmhandle,TIM_CHANNEL_1);
            break;
        case CH_2:
           HAL_TIM_PWM_Stop(pwmhandle,TIM_CHANNEL_2);
            break;
        case CH_3:
            HAL_TIM_PWM_Stop(pwmhandle,TIM_CHANNEL_3);
            break;
        case CH_4:
            HAL_TIM_PWM_Stop(pwmhandle,TIM_CHANNEL_4);    
            break;
        default:
            ret = -1;
            break;
    }     
    return ret;
}

static void TIM_setCompare1(TIM_TypeDef *TIMx,uint16_t Compare1)
{
    if(TIMx==NULL){
        return ;
    }
    TIMx->CCR1 = Compare1;
}
static void TIM_setCompare2(TIM_TypeDef *TIMx,uint16_t Compare2)
{
    if(TIMx==NULL){
        return ;
    }
    TIMx->CCR2 = Compare2;
}
static void TIM_setCompare3(TIM_TypeDef *TIMx,uint16_t Compare3)
{
    if(TIMx==NULL){
        return ;
    }
    TIMx->CCR1 = Compare3;
}
static void TIM_setCompare4(TIM_TypeDef *TIMx,uint16_t Compare4)
{
    if(TIMx==NULL){
        return ;
    }
    TIMx->CCR1 = Compare4;
}
static void TIM_setPSC(TIM_TypeDef *TIMx,uint32_t prescaler)
{
     if(TIMx==NULL){
        return ;
    }
    TIMx->PSC =  prescaler;
}
int32_t hal_pwm_para_chg(pwm_dev_t *pwm, pwm_config_t para)
{
    TIM_HandleTypeDef *pwmhandle;
    uint16_t ch ,duty;
    uint32_t prescaler;
    int32_t ret = -1;

    if(!pwm){
        return -1;
    }
    pwmhandle = &hal_TIM_handle[pwm->port / CH_IN_GROUP];
    duty = ((float)para.duty_cycle/100)*PWM_PSC;
    prescaler = PWM_CLK/PWM_PSC/para.freq;
    TIM_setPSC(pwmhandle->Instance,prescaler);
    ch = get_tim_ch(pwm->port);
    switch (ch){
        case CH_1:
           TIM_setCompare1(pwmhandle->Instance,duty);
            break;
        case CH_2:
           TIM_setCompare2(pwmhandle->Instance,duty);
            break;
        case CH_3:
            TIM_setCompare3(pwmhandle->Instance,duty);
            break;
        case CH_4:
            TIM_setCompare4(pwmhandle->Instance,duty);    
            break;
        default:
            ret = -1;
            break;
    }     
    return ret;
}



int32_t hal_pwm_finalize(pwm_dev_t *pwm)
{
    // HAL_TIM_PWM_DeInit(&pwmhandle);
    return 0;

}