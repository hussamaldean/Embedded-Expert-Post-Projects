/**
 ******************************************************************************
 * @file    soft_uart.c
 * @brief   Timer-driven (single timer) software UART for STM32F411RE @ 100 MHz
 *
 *  TIM2 and the pins (PA0 = TX, PA1 = RX) are configured by STM32CubeMX. This
 *  driver uses the generated htim2 handle and its update interrupt, which
 *  fires at BAUD * OVERSAMPLING = 9600 * 8 = 76.8 kHz (one tick per 1/8 bit).
 *
 *  TX (SoftUART_Transmit_IT):
 *      A buffer is sent byte by byte, back-to-back. Each byte is a 10-bit
 *      frame (start, 8 data LSB-first, stop) shifted out one bit every
 *      OVERSAMPLING ticks. When the last stop bit has completed,
 *      soft_uart_tx_cplt_callback() is called (from the ISR).
 *
 *  RX (SoftUART_ReceiveToIdle_IT):
 *      The user supplies a buffer. Each received byte is stored in it. When
 *      the line stays idle (high) for SOFTUART_IDLE_TICKS after the last stop
 *      bit, or the buffer becomes full, soft_uart_rx_cplt_callback(buf,size)
 *      is called (from the ISR).
 *
 *  Full duplex with a single timer.
 ******************************************************************************
 */

#include "soft_uart.h"

#define SOFTUART_BIT_TICKS   (SOFTUART_OVERSAMPLING)          /* ticks / bit  */
#define SOFTUART_HALF_TICKS  (SOFTUART_OVERSAMPLING / 2u)     /* to the centre */

/* ------------------------------- TX state --------------------------------- */
static uint8_t          tx_active;
static const uint8_t   *tx_ptr;      /* buffer being sent            */
static uint16_t         tx_len;
static uint16_t         tx_idx;
static volatile uint16_t tx_frame;   /* 10-bit frame, LSB first      */
static volatile uint8_t  tx_bit;     /* 0..9                         */
static volatile uint8_t  tx_tick;    /* within-bit tick counter      */

/* ------------------------------- RX state --------------------------------- */
static uint8_t          rx_active;
static uint8_t         *rx_buf;      /* user buffer                  */
static uint16_t         rx_len;
static volatile uint16_t rx_idx;
static volatile uint16_t rx_idle;    /* consecutive high ticks       */
static volatile uint8_t  rx_done;
static volatile uint16_t rx_last_size;

static volatile uint8_t  rx_busy;    /* mid-frame                    */
static volatile uint8_t  rx_tick;
static volatile uint8_t  rx_bit;
static volatile uint8_t  rx_shift;

/* ========================================================================== */
/*                                  Init                                      */
/* ========================================================================== */
void SoftUART_Init(void)
{
    HAL_GPIO_WritePin(SOFTUART_TX_PORT, SOFTUART_TX_PIN, GPIO_PIN_SET);

    tx_active = 0u;
    tx_ptr    = 0;
    tx_len    = 0u;
    tx_idx    = 0u;
    tx_bit    = 0u;
    tx_tick   = 0u;

    rx_active = 0u;
    rx_buf    = 0;
    rx_len    = 0u;
    rx_idx    = 0u;
    rx_idle   = 0u;
    rx_done   = 0u;
    rx_busy   = 0u;
    rx_tick   = 0u;
    rx_bit    = 0u;
    rx_shift  = 0u;

    HAL_NVIC_EnableIRQ(SOFTUART_TIM_IRQn);      /* safe if CubeMX did it too */
    HAL_TIM_Base_Start_IT(&SOFTUART_TIM_HANDLE);
}

/* ========================================================================== */
/*                                  TX                                        */
/* ========================================================================== */
/* Build frame and drive the start bit (called with the line at rest, either
 * from main under lock or from the ISR between frames). */
static void SoftUART_StartByte(uint8_t byte)
{
    tx_frame = (uint16_t)(((uint16_t)byte << 1) | 0x0200u); /* start=0, stop=1 */
    tx_bit   = 0u;
    tx_tick  = 0u;

    HAL_GPIO_WritePin(SOFTUART_TX_PORT, SOFTUART_TX_PIN, GPIO_PIN_RESET);
    tx_frame >>= 1u;            /* bit0 is now data bit 0 */
}

uint8_t SoftUART_TransmitBusy(void)
{
    return tx_active;
}

HAL_StatusTypeDef SoftUART_Transmit_IT(const uint8_t *data, uint16_t len)
{
    if ((data == 0) || (len == 0u))
    {
        return HAL_ERROR;
    }
    if (tx_active)
    {
        return HAL_BUSY;
    }

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    tx_ptr    = data;
    tx_len    = len;
    tx_idx    = 0u;
    tx_active = 1u;
    SoftUART_StartByte(data[0]);

    __set_PRIMASK(primask);
    return HAL_OK;
}

HAL_StatusTypeDef SoftUART_Transmit(const uint8_t *data, uint16_t len)
{
    if ((data == 0) || (len == 0u))
    {
        return HAL_ERROR;
    }
    while (tx_active) { }                       /* wait for the line to be free */
    if (SoftUART_Transmit_IT(data, len) != HAL_OK)
    {
        return HAL_ERROR;
    }
    while (tx_active) { }
    return HAL_OK;
}

HAL_StatusTypeDef SoftUART_TransmitByte(uint8_t byte)
{
    return SoftUART_Transmit(&byte, 1u);
}

void SoftUART_AbortTransmit(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    tx_active = 0u;
    tx_idx    = tx_len;
    HAL_GPIO_WritePin(SOFTUART_TX_PORT, SOFTUART_TX_PIN, GPIO_PIN_SET);

    __set_PRIMASK(primask);
}

/* ========================================================================== */
/*                                  RX                                        */
/* ========================================================================== */
/* Reporting helper: end the current reception and notify the user. Runs in
 * the timer ISR context. */
static void SoftUART_RxReport(uint16_t size)
{
    rx_active    = 0u;
    rx_done      = 1u;
    rx_last_size = size;
    soft_uart_rx_cplt_callback(rx_buf, size);
}

uint8_t SoftUART_ReceiveBusy(void)
{
    return rx_active;
}

HAL_StatusTypeDef SoftUART_ReceiveToIdle_IT(uint8_t *buf, uint16_t len)
{
    if ((buf == 0) || (len == 0u))
    {
        return HAL_ERROR;
    }
    if (rx_active)
    {
        return HAL_BUSY;
    }

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    rx_buf  = buf;
    rx_len  = len;
    rx_idx  = 0u;
    rx_idle = 0u;
    rx_done = 0u;
    rx_active = 1u;

    __set_PRIMASK(primask);
    return HAL_OK;
}

HAL_StatusTypeDef SoftUART_ReceiveToIdle(uint8_t *buf, uint16_t len,
                                         uint16_t *recv_len, uint32_t timeout_ms)
{
    if (SoftUART_ReceiveToIdle_IT(buf, len) != HAL_OK)
    {
        return HAL_BUSY;
    }

    uint32_t start = HAL_GetTick();
    while (!rx_done)
    {
        if ((timeout_ms != 0u) && ((HAL_GetTick() - start) >= timeout_ms))
        {
            SoftUART_AbortReceive();
            return HAL_TIMEOUT;
        }
    }

    if (recv_len != 0)
    {
        *recv_len = rx_last_size;
    }
    rx_done = 0u;
    return HAL_OK;
}

void SoftUART_AbortReceive(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    rx_active = 0u;
    rx_busy   = 0u;
    rx_done   = 0u;

    __set_PRIMASK(primask);
}

/* ========================================================================== */
/*                          Tick (called from ISR)                           */
/* ========================================================================== */
void SoftUART_Tick(void)
{
    /* -------------------------------- TX --------------------------------- */
    if (tx_active)
    {
        if (++tx_tick >= SOFTUART_BIT_TICKS)
        {
            tx_tick = 0u;
            tx_bit++;

            if (tx_bit < 10u)
            {
                HAL_GPIO_WritePin(SOFTUART_TX_PORT, SOFTUART_TX_PIN,
                                  (tx_frame & 1u) ? GPIO_PIN_SET : GPIO_PIN_RESET);
                tx_frame >>= 1u;
            }
            else
            {
                /* Byte finished -> chain the next one or report completion. */
                tx_idx++;
                if (tx_idx < tx_len)
                {
                    SoftUART_StartByte(tx_ptr[tx_idx]);
                }
                else
                {
                    HAL_GPIO_WritePin(SOFTUART_TX_PORT, SOFTUART_TX_PIN, GPIO_PIN_SET);
                    tx_active = 0u;
                    soft_uart_tx_cplt_callback();
                }
            }
        }
    }

    /* -------------------------------- RX --------------------------------- */
    if (!rx_busy)
    {
        if (HAL_GPIO_ReadPin(SOFTUART_RX_PORT, SOFTUART_RX_PIN) == GPIO_PIN_RESET)
        {
            /* Falling edge = potential start bit. */
            rx_idle = 0u;
            rx_busy = 1u;
            rx_tick = 0u;
            rx_bit  = 0u;
            rx_shift = 0u;
        }
        else
        {
            /* Line idle: count consecutive high ticks for IDLE detection. */
            if (rx_idle < 0xFFFFu)
            {
                rx_idle++;
            }
            if (rx_active && (rx_idx > 0u) && (rx_idle >= SOFTUART_IDLE_TICKS))
            {
                SoftUART_RxReport((uint16_t)rx_idx);
            }
        }
    }
    else
    {
        rx_tick++;

        if (rx_tick > SOFTUART_HALF_TICKS)
        {
            uint8_t d = (uint8_t)(rx_tick - SOFTUART_HALF_TICKS);

            if ((d % SOFTUART_BIT_TICKS) == 0u)
            {
                uint8_t centre = (uint8_t)(d / SOFTUART_BIT_TICKS);  /* 1..9 */

                if (centre <= 8u)                 /* data bit centre */
                {
                    rx_shift >>= 1u;
                    if (HAL_GPIO_ReadPin(SOFTUART_RX_PORT, SOFTUART_RX_PIN) != GPIO_PIN_RESET)
                    {
                        rx_shift |= 0x80u;
                    }
                    rx_bit++;
                }
                else                              /* stop bit centre (9) */
                {
                    if ((HAL_GPIO_ReadPin(SOFTUART_RX_PORT, SOFTUART_RX_PIN) != GPIO_PIN_RESET)
                        && (rx_bit == 8u))
                    {
                        if (rx_active)
                        {
                            rx_buf[rx_idx++] = rx_shift;
                            if (rx_idx >= rx_len)      /* buffer full */
                            {
                                SoftUART_RxReport(rx_len);
                            }
                        }
                    }
                    rx_busy = 0u;
                }
            }
        }
        else if (rx_tick == SOFTUART_HALF_TICKS)  /* verify start bit */
        {
            if (HAL_GPIO_ReadPin(SOFTUART_RX_PORT, SOFTUART_RX_PIN) != GPIO_PIN_RESET)
            {
                rx_busy = 0u;                     /* false start / noise */
            }
        }
    }
}

/* ========================================================================== */
/*                     HAL glue: route the timer IRQ                         */
/* ========================================================================== */
/* NOTE: if you already define HAL_TIM_PeriodElapsedCallback() elsewhere,
 * delete this one and call SoftUART_Tick() for SOFTUART_TIM_INSTANCE there.  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == SOFTUART_TIM_INSTANCE)
    {
        SoftUART_Tick();
    }
}

/* ========================================================================== */
/*                        Default (weak) callbacks                            */
/* ========================================================================== */
__weak void soft_uart_tx_cplt_callback(void)
{
    /* Override in your application code. */
}

__weak void soft_uart_rx_cplt_callback(uint8_t *buf, uint16_t size)
{
    (void)buf;
    (void)size;
    /* Override in your application code. */
}
