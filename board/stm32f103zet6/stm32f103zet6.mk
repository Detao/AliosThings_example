NAME := stm32f103zet6

JTAG := stlink-v2-1

$(NAME)_TYPE := kernel
MODULE               := 1062
HOST_ARCH            := Cortex-M3
HOST_MCU_FAMILY      := stm32f1xx_cube
SUPPORT_BINS         := no
HOST_MCU_NAME        := STM32F103ZE-Nucleo
ENABLE_VFP           := 1

$(NAME)_SOURCES += aos/board_partition.c \
                   aos/soc_init.c

$(NAME)_SOURCES += Src/main.c \
                   Src/stm32f1xx_it.c \
                   Src/system_stm32f1xx.c 

$(NAME)_SOURCES += Src/stm32f1xx_hal_msp.c

$(NAME)_SOURCES += startup_stm32f10x_hd.s

GLOBAL_INCLUDES += . \
                   hal/ \
                   aos/ \
                   Inc/



GLOBAL_CFLAGS += -DSTM32F103xE
GLOBAL_CFLAGS += -DUSE_HAL_DRIVER
# GLOBAL_DEFINE += USE_HAL_DRIVER
# GLOBAL_DEFINE += STM32F103xE

GLOBAL_LDFLAGS += -T board/stm32f103zet6/STM32F103ZE_FLASH.ld 


CONFIG_SYSINFO_PRODUCT_MODEL := ALI_AOS_F103-nucleo
CONFIG_SYSINFO_DEVICE_NAME := F103-nucleo

GLOBAL_CFLAGS += -DSYSINFO_OS_VERSION=\"$(CONFIG_SYSINFO_OS_VERSION)\"
GLOBAL_CFLAGS += -DSYSINFO_PRODUCT_MODEL=\"$(CONFIG_SYSINFO_PRODUCT_MODEL)\"
GLOBAL_CFLAGS += -DSYSINFO_DEVICE_NAME=\"$(CONFIG_SYSINFO_DEVICE_NAME)\"
