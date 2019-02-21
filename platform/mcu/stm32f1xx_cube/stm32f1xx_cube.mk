NAME := stm32f1xx_cube
HOST_OPENOCD := stm32f1xx
$(NAME)_TYPE := kernel



$(NAME)_COMPONENTS += platform/arch/arm/armv7m
$(NAME)_COMPONENTS +=rhino hal libc cli

GLOBAL_DEFINES += CONFIG_AOS_KV_MULTIPTN_MODE
GLOBAL_DEFINES += CONFIG_AOS_KV_PTN=6
GLOBAL_DEFINES += CONFIG_AOS_KV_SECOND_PTN=7
GLOBAL_DEFINES += CONFIG_AOS_KV_PTN_SIZE=4096
GLOBAL_DEFINES += CONFIG_AOS_KV_BUFFER_SIZE=8192
GLOBAL_DEFINES += HAL_UART_MODULE_ENABLED
GLOBAL_DEFINES += HAL_FLASH_MODULE_ENABLED


GLOBAL_INCLUDES += Drivers/STM32F1xx_HAL_Driver/Inc  \
                   Drivers/STM32F1xx_HAL_Driver/Inc/Legacy
				    
GLOBAL_INCLUDES += Drivers/CMSIS/Device/ST/STM32F1xx/Include \
                   Drivers/CMSIS/Include                           
				         
			
				   
				    
$(NAME)_SOURCES := Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_haL_adc_ex.c \
                   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_can.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cec.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_crc.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dac_ex.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dac.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_eth.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio_ex.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_hcd.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_i2c.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_i2s.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_irda.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_iwdg.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_mmc.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_nand.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_nor.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pccard.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pcd_ex.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pcd.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rtc_ex.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rtc.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_sd.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_smartcard.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_spi_ex.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_spi.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_sram.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim_ex.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_usart.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_wwdg.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_adc.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_crc.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_dac.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_dma.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_exti.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_fsmc.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_gpio.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_i2c.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_pwr.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_rcc.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_rtc.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_sdmmc.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_spi.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_tim.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_usart.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_usb.c \
				   Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_utils.c \


                  

$(NAME)_SOURCES += aos/soc_impl.c \
                aos/aos.c \
				hal/hal_uart_stm32f1.c \
				hal/hal_flash_stm32f1.c \
				hal/hal_gpio_stm32f1.c \
				hal/hal_pwm_stm32f1.c  \
				hal/hw.c                \
				aos/trace_impl.c 


# lwip ?= 1
	
#sal ?=1

ifeq (1,$(sal))

$(NAME)_COMPONENTS += sal
module ?= wifi.mk3060
GLOBAL_DEFINES += DEV_SAL_MK3060

else ifeq (1,$(lwip))

$(NAME)_SOURCES += net/ethernetif.c  \
                   hal/dm9000.c
GLOBAL_INCLUDES += net

$(NAME)_COMPONENTS +=network.lwip
else 
GLOBAL_DEFINES += CONFIG_NO_TCPIP
endif

ifeq ($(COMPILER),armcc)
				GLOBAL_CFLAGS   += --c99 --cpu=Cortex-M3 -D__MICROLIB -g --split_sections
else ifeq ($(COMPILER),iar)
				GLOBAL_CFLAGS += --cpu=Cortex-M3 \
												 --cpu_mode=thumb \
												 --endian=little
else
				GLOBAL_CFLAGS += -mcpu=cortex-m3 \
												 -march=armv7-m  \
												 -mlittle-endian \
												 -mthumb -mthumb-interwork \
												 -w
				GLOBAL_CFLAGS  += -D__VFP_FP__
endif

ifeq ($(COMPILER),armcc)
				GLOBAL_ASMFLAGS += --cpu=Cortex-M3 -g --library_type=microlib --pd "__MICROLIB SETA 1"
else ifeq ($(COMPILER),iar)
				GLOBAL_ASMFLAGS += --cpu Cortex-M3 \
													 --cpu_mode thumb \
													 --endian little
else
				GLOBAL_ASMFLAGS += -mcpu=cortex-m3 \
													 -mlittle-endian \
													 -mthumb \
													 -w
endif

ifeq ($(COMPILER),armcc)
				GLOBAL_LDFLAGS += -L --cpu=Cortex-M3   \
													-L --strict \
													-L --xref -L --callgraph -L --symbols \
													-L --info=sizes -L --info=totals -L --info=unused -L --info=veneers -L --info=summarysizes
else ifeq ($(COMPILER),iar)
				GLOBAL_LDFLAGS += --silent --cpu=Cortex-M3.vfp

else
				GLOBAL_LDFLAGS += -mcpu=cortex-m3  \
													-mlittle-endian  \
													-mthumb -mthumb-interwork \
													--specs=nosys.specs \
													$(CLIB_LDFLAGS_NANO_FLOAT)
endif
