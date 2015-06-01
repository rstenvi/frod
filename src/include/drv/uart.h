

#ifndef __UART_H
#define __UART_H


#define UART_ERR_NOT_PRESENT    -1
#define UART_ERR_PUTC_NOT_READY -2
#define UART_ERR_GETC_NOT_READY -2


#define UART_SUCCESS 0

int8_t uart_init();
int8_t uart_putc(uint8_t c);
int16_t uart_getc();


#endif
