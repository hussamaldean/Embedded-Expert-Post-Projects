/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
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
#include "i2c.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* I2C1 init function */
void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  LL_I2C_InitTypeDef I2C_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  /**I2C1 GPIO Configuration
  PB8   ------> I2C1_SCL
  PB9   ------> I2C1_SDA
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_8|LL_GPIO_PIN_9;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */

  /** I2C Initialization
  */
  LL_I2C_DisableOwnAddress2(I2C1);
  LL_I2C_DisableGeneralCall(I2C1);
  LL_I2C_EnableClockStretching(I2C1);
  I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
  I2C_InitStruct.ClockSpeed = 100000;
  I2C_InitStruct.DutyCycle = LL_I2C_DUTYCYCLE_2;
  I2C_InitStruct.OwnAddress1 = 0;
  I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
  I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
  LL_I2C_Init(I2C1, &I2C_InitStruct);
  LL_I2C_SetOwnAddress2(I2C1, 0);
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/* USER CODE BEGIN 1 */


/**
 * @brief Scans I2C bus for connected slave devices
 * @param I2Cx I2C instance (e.g., I2C1, I2C2, I2C3)
 * @return Number of devices found
 */
uint8_t I2C_Scan(I2C_TypeDef *I2Cx)
{

	    printf("Scanning I2C bus...\r\n\r\n");

	    uint8_t devices_found = 0;

	    // Scan from address 0x08 to 0x77 (valid 7-bit I2C address range)
	    for(uint8_t address = 0x08; address < 0x78; address++)
	    {
	        // Generate START condition
	        LL_I2C_GenerateStartCondition(I2Cx);

	        // Wait for START condition to be generated
	        while(!LL_I2C_IsActiveFlag_SB(I2Cx));

	        // Send address with WRITE bit (LSB = 0)
	        LL_I2C_TransmitData8(I2Cx, (address << 1) | 0x00);

	        // Wait for address to be sent
	        while(!LL_I2C_IsActiveFlag_ADDR(I2Cx) && !LL_I2C_IsActiveFlag_AF(I2Cx));

	        // Check if ACK received (device found)
	        if(LL_I2C_IsActiveFlag_ADDR(I2Cx))
	        {
	            // Device responded
	            printf("Device found at address: 0x%02X\r\n", address);
	            devices_found++;
	        }

	        // Clear ADDR flag
	        LL_I2C_ClearFlag_ADDR(I2Cx);

	        // Clear AF flag if no device
	        LL_I2C_ClearFlag_AF(I2Cx);

	        // Generate STOP condition
	        LL_I2C_GenerateStopCondition(I2Cx);

	        // Small delay between scans
	        LL_mDelay(2);
	    }

	    printf("*****Scan Complete*****\r\n");
	    printf("Total devices found: %d\r\n\r\n", devices_found);
	    return devices_found;

}

/* USER CODE END 1 */

