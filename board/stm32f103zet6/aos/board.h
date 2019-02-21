#ifndef __BOARD__H_
#define __BOARD__H_

#define HARDWARE_REVISION   "V1.0"
#define MODEL               "STM32F1"

#ifdef BOOTLOADER
#define STDIO_UART 0
#define STDIO_UART_BUADRATE 115200
#else
#define STDIO_UART 0
#define STDIO_UART_BUADRATE 115200
#endif


typedef enum{
    PORT_UART_STD,
    PORT_UART_2,
    PORT_UART_AT,
    PORT_UART_SIZE,
    PORT_UART_INVALID = 255,
}PORT_UART_TYPE;



#endif /*__BOARD__H_*/
