/*
* Copyright (C) 2015-2017 Alibaba Group Holding Limited
*
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <aos/aos.h>
#include <vfs_conf.h>
#include <vfs_err.h>
#include <vfs_register.h>
#include <hal/base.h>
#include "common.h"
#include "sensor.h"
#include "sensor_drv_api.h"
#include "sensor_hal.h"


#define LIS331HH_I2C_ADDR1                 	(0x18)
#define LIS331HH_I2C_ADDR2                 	(0x19)
#define LIS331HH_I2C_ADDR_TRANS(n)         	((n)<<1)  
#define LIS331HH_I2C_ADDR                  	LIS331HH_I2C_ADDR_TRANS(LIS331HH_I2C_ADDR1)

#define LIS331HH_ACC_WHO_AM_I              	0x0F
#define LIS331HH_ACC_CTRL_REG1             	0x20
#define LIS331HH_ACC_CTRL_REG2             	0x21
#define LIS331HH_ACC_CTRL_REG3            	0x22
#define LIS331HH_ACC_CTRL_REG4             	0x23
#define LIS331HH_ACC_CTRL_REG5             	0x24
#define LIS331HH_ACC_STATUS_REG           	0x27
#define LIS331HH_ACC_OUT_X_L              	0x28
#define LIS331HH_ACC_OUT_X_H              	0x29
#define LIS331HH_ACC_OUT_Y_L              	0x2A
#define LIS331HH_ACC_OUT_Y_H              	0x2B
#define LIS331HH_ACC_OUT_Z_L              	0x2C
#define LIS331HH_ACC_OUT_Z_H              	0x2D

#define LIS331HH_ACC_RANGE_6G              	(0x0)
#define LIS331HH_ACC_RANGE_12G             	(0x1)
#define LIS331HH_ACC_RANGE_24G              (0x3)
#define LIS331HH_ACC_RANGE_MSK              (0x30)
#define LIS331HH_ACC_RANGE_POS              (4)

#define LIS331HH_ACC_SENSITIVITY_6G        	3
#define LIS331HH_ACC_SENSITIVITY_12G        6
#define LIS331HH_ACC_SENSITIVITY_24G        12

#define LIS331HH_SHIFT_EIGHT_BITS           (8)
#define LIS331HH_SHIFT_FOUR_BITS            (4)

#define LIS331HH_16_BIT_SHIFT               (0xFF)

#define LIS331HH_ACC_ODR_POWER_DOWN			(0x00)
#define LIS331HH_ACC_ODR_0_5_HZ             (0x40)
#define LIS331HH_ACC_ODR_1_HZ               (0x60)
#define LIS331HH_ACC_ODR_2_HZ               (0x80)
#define LIS331HH_ACC_ODR_5_HZ               (0xA0)
#define LIS331HH_ACC_ODR_10_HZ              (0xC0)
#define LIS331HH_ACC_ODR_50_HZ              (0x20)
#define LIS331HH_ACC_ODR_100_HZ             (0x28)
#define LIS331HH_ACC_ODR_400_HZ             (0x30)
#define LIS331HH_ACC_ODR_1000_HZ            (0x38)
#define LIS331HH_ACC_ODR_MSK                (0xF8)
#define LIS331HH_ACC_ODR_POS                (3)

#define LIS331HH_ACC_DEFAULT_ODR_100HZ      (100)

#define LIS331HH_BDU_ENABLE					(0x80)

#define LIS331HH_ACC_STATUS_ZYXDA			(0x08)

#define LIS331HH_GET_BITSLICE(regvar, bitname)\
((regvar & bitname##_MSK) >> bitname##_POS)
	
#define LIS331HH_SET_BITSLICE(regvar, bitname, val)\
	((regvar & ~bitname##_MSK) | ((val<<bitname##_POS)&bitname##_MSK))


static int32_t lis331hh_acc_factor[ACC_RANGE_MAX] = {0, 0, 0, 0, LIS331HH_ACC_SENSITIVITY_6G, 
																		LIS331HH_ACC_SENSITIVITY_12G, LIS331HH_ACC_SENSITIVITY_24G };
static int32_t cur_acc_factor = 0;


i2c_dev_t lis331hh_ctx = {
    .port = 3,
    .config.address_width = 8,
    .config.freq = 400000,
    .config.dev_addr = LIS331HH_I2C_ADDR,
};

static int drv_acc_st_lis331hh_set_power_mode(i2c_dev_t* drv, dev_power_mode_e mode)
{
    uint8_t value  = 0x00;
    int ret = 0;
    
    ret = sensor_i2c_read(drv, LIS331HH_ACC_CTRL_REG1, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }

    switch(mode){
        case DEV_POWER_ON:{
            value = LIS331HH_SET_BITSLICE(value,LIS331HH_ACC_ODR,LIS331HH_ACC_ODR_10_HZ);
            ret = sensor_i2c_write(drv, LIS331HH_ACC_CTRL_REG1, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
            if(unlikely(ret)){
                return ret;
            }
        }break;
        
        case DEV_POWER_OFF:{
            value = LIS331HH_SET_BITSLICE(value,LIS331HH_ACC_ODR,LIS331HH_ACC_ODR_POWER_DOWN);
            ret = sensor_i2c_write(drv, LIS331HH_ACC_CTRL_REG1, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
            if(unlikely(ret)){
                return ret;
            }
        }break;
            
        case DEV_SLEEP:{
            value = LIS331HH_SET_BITSLICE(value,LIS331HH_ACC_ODR,LIS331HH_ACC_ODR_10_HZ);
            ret = sensor_i2c_write(drv, LIS331HH_ACC_CTRL_REG1, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
            if(unlikely(ret)){
                return ret;
            }

        }break;

       default:break;
    }
    return 0;
}

static uint8_t drv_acc_st_lis331hh_hz2odr(uint32_t hz)
{
    if(hz > 400)
        return LIS331HH_ACC_ODR_1000_HZ;
    else if(hz > 100)
        return LIS331HH_ACC_ODR_400_HZ;
    else if(hz > 50)
        return LIS331HH_ACC_ODR_100_HZ;
    else if(hz > 10)
        return LIS331HH_ACC_ODR_50_HZ;
    else if(hz > 5)
        return LIS331HH_ACC_ODR_10_HZ;
    else if(hz > 2)
        return LIS331HH_ACC_ODR_5_HZ;
    else if(hz > 1)
        return LIS331HH_ACC_ODR_2_HZ;
	else
		return LIS331HH_ACC_ODR_1_HZ;

}

static int drv_acc_st_lis331hh_set_odr(i2c_dev_t* drv, uint32_t hz)
{
    int ret = 0;
    uint8_t value = 0x00;
    uint8_t odr = drv_acc_st_lis331hh_hz2odr(hz);

    ret = sensor_i2c_read(drv, LIS331HH_ACC_CTRL_REG1, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }

    value = LIS331HH_SET_BITSLICE(value,LIS331HH_ACC_ODR,odr);
	
    ret = sensor_i2c_write(drv, LIS331HH_ACC_CTRL_REG1, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }
    return 0;    
}

static int drv_acc_st_lis331hh_set_range(i2c_dev_t* drv, uint32_t range)
{

    int ret = 0;
    uint8_t value = 0x00;
    uint8_t tmp = 0;

    ret = sensor_i2c_read(drv, LIS331HH_ACC_CTRL_REG4, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }

    switch (range){
        case ACC_RANGE_6G:{
            tmp = LIS331HH_ACC_RANGE_6G;
        }break;
        
        case ACC_RANGE_12G:{
            tmp = LIS331HH_ACC_RANGE_12G;
        }break;
        
        case ACC_RANGE_24G:{
            tmp = LIS331HH_ACC_RANGE_24G;
        }break;
        
        default:break;
    }
    
    value  = LIS331HH_SET_BITSLICE(value,LIS331HH_ACC_RANGE,tmp);
    
    ret = sensor_i2c_write(drv, LIS331HH_ACC_CTRL_REG4, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }

    if((range >= ACC_RANGE_6G)&&(range <= ACC_RANGE_24G)){
        cur_acc_factor = lis331hh_acc_factor[range];
    }
    
    return 0;
}


static int drv_acc_st_lis331hh_set_bdu(i2c_dev_t* drv)
{
    uint8_t value = 0x00;
    int ret = 0;
    
    ret = sensor_i2c_read(drv, LIS331HH_ACC_CTRL_REG4, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }
    value |= LIS331HH_BDU_ENABLE;
    
    ret = sensor_i2c_write(drv, LIS331HH_ACC_CTRL_REG4, &value, I2C_DATA_LEN, I2C_OP_RETRIES);
    if(unlikely(ret)){
        return ret;
    }
    return 0;
}

static void drv_acc_st_lis331hh_irq_handle(void)
{
    /* no handle so far */
}

static int drv_acc_st_lis331hh_open(void)
{
    int ret = 0;
    ret  = drv_acc_st_lis331hh_set_power_mode(&lis331hh_ctx, DEV_POWER_ON);
    if(unlikely(ret)){
        return -1;
    }

    ret = drv_acc_st_lis331hh_set_range(&lis331hh_ctx, ACC_RANGE_6G);
    if(unlikely(ret)){
        return -1;
    }

    ret = drv_acc_st_lis331hh_set_odr(&lis331hh_ctx, LIS331HH_ACC_DEFAULT_ODR_100HZ);
    if(unlikely(ret)){
        return -1;
    }
     
    return 0;

}

static int drv_acc_st_lis331hh_close(void)
{
    int ret = 0;
    ret  = drv_acc_st_lis331hh_set_power_mode(&lis331hh_ctx, DEV_POWER_OFF);
    if(unlikely(ret)){
        return -1;
    }
    return 0;
}

static int drv_acc_st_lis331hh_read(void *buf, size_t len)
{

    int ret = 0;
    size_t size;
    uint8_t reg[6];
    accel_data_t *accel = (accel_data_t *)buf;
    if(buf == NULL){
        return -1;
    }

    size = sizeof(accel_data_t);
    if(len < size){
        return -1;
    }
	
	ret = sensor_i2c_read(&lis331hh_ctx, (LIS331HH_ACC_OUT_X_L | 0x80), reg, 6, I2C_OP_RETRIES);
	
	if(unlikely(ret)){
        return -1;
    }
    accel->data[DATA_AXIS_X] = (int16_t)((((int16_t)((int8_t)reg[1]))<< LIS331HH_SHIFT_EIGHT_BITS)|(reg[0]));	
	accel->data[DATA_AXIS_X] = accel->data[DATA_AXIS_X] >> LIS331HH_SHIFT_FOUR_BITS;
    accel->data[DATA_AXIS_Y] = (int16_t)((((int16_t)((int8_t)reg[3]))<< LIS331HH_SHIFT_EIGHT_BITS)|(reg[2]));
	accel->data[DATA_AXIS_Y] = accel->data[DATA_AXIS_Y] >> LIS331HH_SHIFT_FOUR_BITS;
	accel->data[DATA_AXIS_Z] = (int16_t)((((int16_t)((int8_t)reg[5]))<< LIS331HH_SHIFT_EIGHT_BITS)|(reg[4]));
	accel->data[DATA_AXIS_Z] = accel->data[DATA_AXIS_Z] >> LIS331HH_SHIFT_FOUR_BITS;
	
    if(cur_acc_factor != 0){
        accel->data[DATA_AXIS_X] = accel->data[DATA_AXIS_X] * cur_acc_factor;
        accel->data[DATA_AXIS_Y] = accel->data[DATA_AXIS_Y] * cur_acc_factor;
        accel->data[DATA_AXIS_Z] = accel->data[DATA_AXIS_Z] * cur_acc_factor;
    }
    
    accel->timestamp = aos_now_ms();

    return (int)size;
}

static int drv_acc_st_lis331hh_ioctl(int cmd, unsigned long arg)
{
    int ret = 0;
    dev_sensor_info_t *info = (dev_sensor_info_t *)arg;
    
    switch(cmd){
        case SENSOR_IOCTL_ODR_SET:{
            ret = drv_acc_st_lis331hh_set_odr(&lis331hh_ctx, arg);
            if(unlikely(ret)){
                return -1;
            }
        }break;
        case SENSOR_IOCTL_RANGE_SET:{
            ret = drv_acc_st_lis331hh_set_range(&lis331hh_ctx, arg);
            if(unlikely(ret)){
                return -1;
            }
        }break;
        case SENSOR_IOCTL_SET_POWER:{
            ret = drv_acc_st_lis331hh_set_power_mode(&lis331hh_ctx, arg);
            if(unlikely(ret)){
                return -1;
            }
        }break;
        case SENSOR_IOCTL_GET_INFO:{ 
            /* fill the dev info here */
            info->model = "LIS331HH";
            info->range_max = 24;
            info->range_min = 6;
            info->unit = mg;
			
        }break;
       default:break;
    }

    return 0;
}

int drv_acc_st_lis331hh_init(void){
    int ret = 0;
    sensor_obj_t sensor;
    memset(&sensor, 0, sizeof(sensor));

    /* fill the sensor obj parameters here */
    sensor.io_port    = I2C_PORT;
    sensor.tag        = TAG_DEV_ACC;
    sensor.path       = dev_acc_path;
    sensor.open       = drv_acc_st_lis331hh_open;
    sensor.close      = drv_acc_st_lis331hh_close;
    sensor.read       = drv_acc_st_lis331hh_read;
    sensor.write      = NULL;
    sensor.ioctl      = drv_acc_st_lis331hh_ioctl;
    sensor.irq_handle = drv_acc_st_lis331hh_irq_handle;

    ret = sensor_create_obj(&sensor);
    if(unlikely(ret)){
        return -1;
    }

    ret = drv_acc_st_lis331hh_set_range(&lis331hh_ctx, ACC_RANGE_6G);
    if(unlikely(ret)){
        return -1;
    }

	ret = drv_acc_st_lis331hh_set_bdu(&lis331hh_ctx);
    if(unlikely(ret)){
        return -1;
    }

    //set odr is 100hz, and will update
    ret = drv_acc_st_lis331hh_set_odr(&lis331hh_ctx, LIS331HH_ACC_DEFAULT_ODR_100HZ);
    if(unlikely(ret)){
        return -1;
    }

    /* update the phy sensor info to sensor hal */
    LOG("%s %s successfully \n", SENSOR_STR, __func__);
    return 0;
}

