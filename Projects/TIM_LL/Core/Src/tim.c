/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "tim.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* TIM2 init function */
void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  TIM_InitStruct.Prescaler = 15999;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 1000-LL_TIM_IC_FILTER_FDIV1_N2;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM2, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM2);
  LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
  TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_TOGGLE;
  TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
  TIM_OC_InitStruct.CompareValue = 0;
  TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  LL_TIM_OC_Init(TIM2, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
  LL_TIM_OC_DisableFast(TIM2, LL_TIM_CHANNEL_CH1);
  LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM2);
  /* USER CODE BEGIN TIM2_Init 2 */
  TIM_InitStruct.Autoreload = 1000;
  LL_TIM_Init(TIM2, &TIM_InitStruct);
  /* USER CODE END TIM2_Init 2 */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    /**TIM2 GPIO Configuration
    PA5     ------> TIM2_CH1
    */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 1 */
void TIM_Delay_ms(TIM_TypeDef *TIMx, uint32_t ms)
{

	LL_TIM_DisableCounter(TIMx);

	LL_TIM_SetAutoReload(TIMx, ms);

//	// Force an update event so ARR is loaded into the active register
//	// and CNT is reloaded with ARR (down-count mode)
	LL_TIM_GenerateEvent_UPDATE(TIMx);
//
	LL_TIM_ClearFlag_UPDATE(TIMx);

	LL_TIM_EnableCounter(TIMx);

	while (!LL_TIM_IsActiveFlag_UPDATE(TIMx))
	{
		// Blocking wait
	}

	// Stop the timer and clear the flag
	LL_TIM_DisableCounter(TIMx);
	LL_TIM_ClearFlag_UPDATE(TIMx);
}


/**
 * @brief  Start PWM output on a specific timer channel
 * @param  TIMx: Timer instance (e.g., TIM2, TIM3, TIM4, TIM5)
 * @param  channel: Channel number (1-4)
 * @retval None
 *
 * @note   Assumes timer is already configured (PSC, ARR, PWM mode, GPIO AF)
 *         via CubeMX. This function enables the channel and starts the timer.
 */
void PWM_Start(TIM_TypeDef *TIMx, uint8_t channel)
{
    /* Enable the capture/compare channel output */
    switch (channel)
    {
        case 1:
            LL_TIM_CC_EnableChannel(TIMx, LL_TIM_CHANNEL_CH1);
            break;
        case 2:
            LL_TIM_CC_EnableChannel(TIMx, LL_TIM_CHANNEL_CH2);
            break;
        case 3:
            LL_TIM_CC_EnableChannel(TIMx, LL_TIM_CHANNEL_CH3);
            break;
        case 4:
            LL_TIM_CC_EnableChannel(TIMx, LL_TIM_CHANNEL_CH4);
            break;
        default:
            return; // Invalid channel
    }

    /* Enable counter */
    LL_TIM_EnableCounter(TIMx);

    /* Force update generation to load PSC/ARR/CCR immediately */
    LL_TIM_GenerateEvent_UPDATE(TIMx);
}

/**
 * @brief  Update PWM duty cycle on a specific timer channel
 * @param  TIMx: Timer instance (e.g., TIM2, TIM3, TIM4, TIM5)
 * @param  channel: Channel number (1-4)
 * @param  duty_cycle: Duty cycle in percent (0.0f to 100.0f)
 * @retval None
 *
 * @note   Uses the timer's current ARR value to compute the compare value.
 *         For your TIM2 config (PSC=0, ARR=100), duty 50% -> CCR=50.
 */
void PWM_SetDutyCycle(TIM_TypeDef *TIMx, uint8_t channel, float duty_cycle)
{
    /* Clamp duty cycle to valid range */
    if (duty_cycle < 0.0f)   duty_cycle = 0.0f;
    if (duty_cycle > 100.0f) duty_cycle = 100.0f;

    /* Read current auto-reload value to compute compare value */
    uint32_t arr = LL_TIM_GetAutoReload(TIMx);
    uint32_t ccr = (uint32_t)((duty_cycle / 100.0f) * (float)(arr + 1));

    /* Clamp CCR to max ARR+1 (100% duty needs CCR > ARR) */
    if (ccr > (arr + 1)) ccr = arr + 1;

    /* Set the compare value on the requested channel */
    switch (channel)
    {
        case 1:
            LL_TIM_OC_SetCompareCH1(TIMx, ccr);
            break;
        case 2:
            LL_TIM_OC_SetCompareCH2(TIMx, ccr);
            break;
        case 3:
            LL_TIM_OC_SetCompareCH3(TIMx, ccr);
            break;
        case 4:
            LL_TIM_OC_SetCompareCH4(TIMx, ccr);
            break;
        default:
            return; // Invalid channel
    }
}

/**
 * @brief  Start Output Compare output on a specific timer channel
 * @param  TIMx: Timer instance (e.g., TIM2, TIM3, TIM4, TIM5)
 * @param  channel: Channel number (1-4)
 * @retval None
 *
 * @note   Assumes timer (PSC, ARR, OCxM, GPIO AF) is already configured
 *         via CubeMX. This function enables the channel and starts the counter.
 */
void OC_Start(TIM_TypeDef *TIMx, uint8_t channel)
{
    switch (channel)
    {
        case 1: LL_TIM_CC_EnableChannel(TIMx, LL_TIM_CHANNEL_CH1); break;
        case 2: LL_TIM_CC_EnableChannel(TIMx, LL_TIM_CHANNEL_CH2); break;
        case 3: LL_TIM_CC_EnableChannel(TIMx, LL_TIM_CHANNEL_CH3); break;
        case 4: LL_TIM_CC_EnableChannel(TIMx, LL_TIM_CHANNEL_CH4); break;
        default: return;
    }

    LL_TIM_EnableCounter(TIMx);
    LL_TIM_GenerateEvent_UPDATE(TIMx);
}

/**
 * @brief  Set the compare value on a specific timer channel (OC mode)
 * @param  TIMx: Timer instance (e.g., TIM2, TIM3, TIM4, TIM5)
 * @param  channel: Channel number (1-4)
 * @param  compare: Compare value (caller is responsible for keeping it <= ARR)
 * @retval None
 */
void OC_SetCompare(TIM_TypeDef *TIMx, uint8_t channel, uint32_t compare)
{
    switch (channel)
    {
        case 1: LL_TIM_OC_SetCompareCH1(TIMx, compare); break;
        case 2: LL_TIM_OC_SetCompareCH2(TIMx, compare); break;
        case 3: LL_TIM_OC_SetCompareCH3(TIMx, compare); break;
        case 4: LL_TIM_OC_SetCompareCH4(TIMx, compare); break;
        default: return;
    }
}

/* USER CODE END 1 */

