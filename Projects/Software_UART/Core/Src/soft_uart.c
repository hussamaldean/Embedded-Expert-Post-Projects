/* soft_uart.c — Software UART via TIM3 Interrupt */

#include "soft_uart.h"
#include "main.h"

/* ===================== Private Variables ===================== */

static volatile uint8_t  tx_buffer[SOFT_UART_TX_BUF_SIZE];
static volatile uint16_t tx_head = 0;  /* Where new data is added */
static volatile uint16_t tx_tail = 0;  /* What the ISR is currently sending */

static volatile uint8_t  tx_shift_reg = 0;
static volatile uint8_t  tx_bit_count = 0;
static volatile bool     tx_busy = false;

/* ===================== Initialization ===================== */

void SoftUART_Init(void)
{
    /* Ensure PA6 is High (Idle state) */
    HAL_GPIO_WritePin(soft_tx_GPIO_Port, soft_tx_Pin, GPIO_PIN_SET);
}

/* ===================== Core TX Logic ===================== */

void SoftUART_SendByte(uint8_t data)
{
    /* Calculate next head index */
    uint16_t next_head = (tx_head + 1U) % SOFT_UART_TX_BUF_SIZE;

    /* Wait if buffer is full */
    while (next_head == tx_tail) { __WFE(); }

    /* Disable interrupt temporarily to safely update the buffer */
    __disable_irq();
    tx_buffer[tx_head] = data;
    tx_head = next_head;
    __enable_irq();

    /* Start the timer if it's not already running */
    if (!tx_busy)
    {
        tx_busy = true;
        htim3.Instance->CNT = 0;
        HAL_TIM_Base_Start_IT(&htim3);
    }
}

void SoftUART_SendBuffer(const uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        SoftUART_SendByte(data[i]);
    }
}

void SoftUART_SendString(const char *str)
{
    while (*str)
    {
        SoftUART_SendByte((uint8_t)*str++);
    }
}

bool SoftUART_IsBusy(void)
{
    return tx_busy;
}

/* ===================== Interrupt Handler ===================== */

/* Put this function in your stm32f4xx_it.c file,
   or keep it here if you are not using CubeMX's IT file.
   It MUST be named exactly TIM3_IRQHandler. */

void TIM3_IRQHandler(void)
{
    /* Clear the update interrupt flag manually (bypass HAL overhead) */
    TIM3->SR = ~TIM_SR_UIF;

    if (tx_bit_count == 0)
    {
        /* Load next byte from the ring buffer */
        if (tx_head != tx_tail)
        {
            tx_shift_reg = tx_buffer[tx_tail];
            tx_tail = (tx_tail + 1U) % SOFT_UART_TX_BUF_SIZE;

            /* Send Start Bit (Low) */
            HAL_GPIO_WritePin(soft_tx_GPIO_Port, soft_tx_Pin, GPIO_PIN_RESET);

            tx_bit_count = 1; /* Start processing data bits on next interrupt */
        }
        else
        {
            /* Buffer empty: Stop the timer */
            HAL_TIM_Base_Stop_IT(&htim3);
            tx_busy = false;
        }
    }
    else if (tx_bit_count <= 8)
    {
        /* Send Data Bits (LSB first) */
        if (tx_shift_reg & 0x01U)
        {
            HAL_GPIO_WritePin(soft_tx_GPIO_Port, soft_tx_Pin, GPIO_PIN_SET);
        }
        else
        {
            HAL_GPIO_WritePin(soft_tx_GPIO_Port, soft_tx_Pin, GPIO_PIN_RESET);
        }

        tx_shift_reg >>= 1;
        tx_bit_count++;
    }
    else
    {
        /* Send Stop Bit (High) */
        HAL_GPIO_WritePin(soft_tx_GPIO_Port, soft_tx_Pin, GPIO_PIN_SET);
        tx_bit_count = 0; /* Reset for next byte */
    }
}
