#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include "stm32f4xx.h"
#include <stdint.h>

/* -----------------------------------------------
 * Ring buffer size - must be power of 2 for efficient masking
 * 64 byte is enough for our command protocol to be more safe so if a new message arrives befioire the first one ends we will have a safe place
 * [MOVE command (10 bytes total):
Byte 0     : command type   (0x01 = MOVE)
Byte 1     : direction      (0 = backward, 1 = forward)
Byte 2-5   : step count     (4 bytes, unsigned, big machines need more than 255 steps)
Byte 6-9   : step delay     (4 bytes, microseconds between pulses, controls speed)
 * ]
 *------------------------------------------------- */
#define UART_RX_BUFFER_SIZE		64

/* -----------------------------------------------
 * Function decleration
 *------------------------------------------------- */

/* -----------------------------------------------
 * Initializes USART2 at given baud rate
 * Configures PA2 as TX, PA3 as RX in Alternative Function mode
 * Enables RX interrupt
 *------------------------------------------------- */
void UART_Init(uint32_t baudrate);

/* -----------------------------------------------
 * Send a single byte - blocks until TX register is empty
 *------------------------------------------------- */
void UART_SendByte(uint8_t byte);

/* -----------------------------------------------
 * Sends null-terminated string byte by byte
 *------------------------------------------------- */
void UART_SendString(const char *str);

/* -----------------------------------------------
 * sends formatted string - works like printf
 *------------------------------------------------- */
void UART_SendFormatted(const char *fmt, ...);

/* -----------------------------------------------
 *cehck if a 10 bytes message is ready to be read
 * Returns 1 if a complete line was found, 0 if not yet
 * You call this from the main loop - it never blocks
 *------------------------------------------------- */

uint8_t UART_IsMessageReady(void);

/*this function copy the 10 bytes messgae out into a buffer*/
void UART_CopyMessage(uint8_t *buf);

/* -----------------------------------------------
 * Return how many bytes are waiting in the RX ring buffer
 *------------------------------------------------- */
uint16_t UART_RxAvailable(void);


#endif /* UART_DRIVER_H */
