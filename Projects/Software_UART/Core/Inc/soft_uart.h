/* soft_uart.h — Software UART via TIM3 Interrupt */
#ifndef SOFT_UART_H
#define SOFT_UART_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>
#include "tim.h"
/* ===================== User Configuration ===================== */

#define SOFT_UART_BAUDRATE         115200U
#define SOFT_UART_TIM_CLOCK_HZ     100000000UL

/* Timer ticks per UART bit */
#define SOFT_UART_TICKS_PER_BIT   ((SOFT_UART_TIM_CLOCK_HZ + (SOFT_UART_BAUDRATE / 2U)) \
                                   / SOFT_UART_BAUDRATE)

/* TX Buffer Size */
#define SOFT_UART_TX_BUF_SIZE      128U

/* ===================== API ===================== */

void  SoftUART_Init(void);
void  SoftUART_SendByte(uint8_t data);
void  SoftUART_SendBuffer(const uint8_t *data, uint32_t len);
void  SoftUART_SendString(const char *str);
bool  SoftUART_IsBusy(void);

#endif /* SOFT_UART_H */
