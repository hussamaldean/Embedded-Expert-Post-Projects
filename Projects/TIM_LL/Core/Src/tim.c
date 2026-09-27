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

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  TIM_InitStruct.Prescaler = 15999;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_DOWN;
  TIM_InitStruct.Autoreload = 4294967295;
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
  LL_TIM_Init(TIM2, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM2);
  LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
  LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM2);
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

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
/* USER CODE END 1 */

