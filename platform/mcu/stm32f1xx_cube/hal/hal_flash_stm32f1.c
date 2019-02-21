/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */
#include "stm32f1xx.h"
//#include "stm32f1xx_hal.h"

#ifdef HAL_FLASH_MODULE_ENABLED

#include <k_api.h>
#include "hal/soc/soc.h"
//#include "stm32f1xx_hal_flash.h"

#define ROUND_DOWN(a,b) (((a) / (b)) * (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))

extern const hal_logic_partition_t hal_partitions[];

/**
  * @brief  Get the bank of a given address.
  * @param  In: addr      Address in the FLASH Memory.
  * @retval Bank identifier.
  *           FLASH_BANK_1
  *           FLASH_BANK_2
  */

hal_logic_partition_t *hal_flash_get_info(hal_partition_t pno)
{
    hal_logic_partition_t *logic_partition;

    //logic_partition = (hal_logic_partition_t *)&hal_partitions[ pno ];

    return logic_partition;
}

int32_t hal_flash_write(hal_partition_t pno, uint32_t *poff, const void *buf , uint32_t buf_size)
{
    return 0;
}

int32_t hal_flash_read(hal_partition_t pno, uint32_t *poff, void *buf, uint32_t buf_size)
{
    return 0;
}

int32_t hal_flash_erase(hal_partition_t pno, uint32_t off_set,
                        uint32_t size)
{
    return 0;
}

int32_t hal_flash_enable_secure(hal_partition_t partition, uint32_t off_set, uint32_t size)
{
    return 0;
}

int32_t hal_flash_dis_secure(hal_partition_t partition, uint32_t off_set, uint32_t size)
{
    return 0;
}

#endif
