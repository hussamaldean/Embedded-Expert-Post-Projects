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

#include "stdio.h"

#define I2C_DMA_INSTANCE  		DMA1

#define I2C_DMA_STREAM_TX    	LL_DMA_STREAM_1

#define I2C_DMA_SREAM_RX		LL_DMA_STREAM_0



/* Context to share the I2C instance with the ISR */
static volatile uint8_t I2C_DMA_Busy = 0;
static I2C_TypeDef *Active_I2Cx = NULL;


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

  /* I2C1 DMA Init */

  /* I2C1_TX Init */
  LL_DMA_SetChannelSelection(DMA1, LL_DMA_STREAM_1, LL_DMA_CHANNEL_0);

  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_STREAM_1, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

  LL_DMA_SetStreamPriorityLevel(DMA1, LL_DMA_STREAM_1, LL_DMA_PRIORITY_LOW);

  LL_DMA_SetMode(DMA1, LL_DMA_STREAM_1, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_STREAM_1, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_STREAM_1, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA1, LL_DMA_STREAM_1, LL_DMA_PDATAALIGN_BYTE);

  LL_DMA_SetMemorySize(DMA1, LL_DMA_STREAM_1, LL_DMA_MDATAALIGN_BYTE);

  LL_DMA_DisableFifoMode(DMA1, LL_DMA_STREAM_1);

  /* I2C1_RX Init */
  LL_DMA_SetChannelSelection(DMA1, LL_DMA_STREAM_0, LL_DMA_CHANNEL_1);

  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_STREAM_0, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

  LL_DMA_SetStreamPriorityLevel(DMA1, LL_DMA_STREAM_0, LL_DMA_PRIORITY_LOW);

  LL_DMA_SetMode(DMA1, LL_DMA_STREAM_0, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_STREAM_0, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_STREAM_0, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA1, LL_DMA_STREAM_0, LL_DMA_PDATAALIGN_BYTE);

  LL_DMA_SetMemorySize(DMA1, LL_DMA_STREAM_0, LL_DMA_MDATAALIGN_BYTE);

  LL_DMA_DisableFifoMode(DMA1, LL_DMA_STREAM_0);

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


void I2C_Mem_Read(I2C_TypeDef *I2Cx,uint8_t slaveAddress, uint16_t regAddress, uint8_t regLen, uint8_t *data, uint16_t length)
{
    // Write phase: Send register address
    LL_I2C_GenerateStartCondition(I2Cx);

    while(!LL_I2C_IsActiveFlag_SB(I2Cx));

    LL_I2C_TransmitData8(I2Cx, (slaveAddress) | 0x00);

    while(!LL_I2C_IsActiveFlag_ADDR(I2Cx));

    LL_I2C_ClearFlag_ADDR(I2Cx);

    if (regLen == 1)
    {
        LL_I2C_TransmitData8(I2Cx, regAddress & 0xFF);
        while(!LL_I2C_IsActiveFlag_TXE(I2Cx));
    }

    else if (regLen == 2)
    {
        // Send regAddress high byte
        LL_I2C_TransmitData8(I2Cx, (uint8_t)(regAddress >> 8));
        while(!LL_I2C_IsActiveFlag_TXE(I2Cx));

        // Send regAddress low byte
        LL_I2C_TransmitData8(I2Cx, (uint8_t)(regAddress & 0xFF));
        while(!LL_I2C_IsActiveFlag_TXE(I2Cx));
    }


    // Read phase: Get data bytes
    LL_I2C_GenerateStartCondition(I2Cx);

    while(!LL_I2C_IsActiveFlag_SB(I2Cx));

    LL_I2C_TransmitData8(I2Cx, (slaveAddress) | 0x01);

    while(!LL_I2C_IsActiveFlag_ADDR(I2Cx));

    LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_ACK);

    LL_I2C_ClearFlag_ADDR(I2Cx);


    // Read all bytes
    for(uint16_t i = 0; i < length; i++)
    {
        if(i == length - 1)
        {
            LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_NACK);  // If last byte, Send NACK
        }

        while(!LL_I2C_IsActiveFlag_RXNE(I2Cx));

        data[i] = LL_I2C_ReceiveData8(I2Cx);
    }

    LL_I2C_GenerateStopCondition(I2Cx);
}


void I2C_Mem_Write(I2C_TypeDef *I2Cx,uint8_t slaveAddress, uint16_t regAddress,uint8_t regLen, uint8_t *data, uint16_t length)
{
    // Generate START condition
    LL_I2C_GenerateStartCondition(I2Cx);
    while(!LL_I2C_IsActiveFlag_SB(I2Cx));

    // Send device address with write bit
    LL_I2C_TransmitData8(I2Cx, (slaveAddress ) | 0x00);
    while(!LL_I2C_IsActiveFlag_ADDR(I2Cx));
    LL_I2C_ClearFlag_ADDR(I2Cx);

    if (regLen == 1)
      {
          LL_I2C_TransmitData8(I2Cx, regAddress & 0xFF);
          while(!LL_I2C_IsActiveFlag_TXE(I2Cx));
      }

      else if (regLen == 2)
      {
          // Send regAddress high byte
          LL_I2C_TransmitData8(I2Cx, (uint8_t)(regAddress >> 8));
          while(!LL_I2C_IsActiveFlag_TXE(I2Cx));

          // Send regAddress low byte
          LL_I2C_TransmitData8(I2Cx, (uint8_t)(regAddress & 0xFF));
          while(!LL_I2C_IsActiveFlag_TXE(I2Cx));
      }

    // Send all data bytes
    for(uint16_t i = 0; i < length; i++)
    {
        LL_I2C_TransmitData8(I2Cx, data[i]);
        while(!LL_I2C_IsActiveFlag_TXE(I2Cx));
    }

    // Generate STOP condition
    LL_I2C_GenerateStopCondition(I2Cx);


}


void I2C_Mem_Write_DMA(I2C_TypeDef *I2Cx,
                            uint8_t  slaveAddress,
                            uint16_t regAddress,
                            uint8_t  regLen,
                            uint8_t *data,
                            uint16_t length)
{
    if (length == 0) return;
    if ((regLen != 1) && (regLen != 2)) return;
    if (I2C_DMA_Busy) return; /* Prevent overlapping transfers */

    I2C_DMA_Busy = 1;

    /* Save I2C context for the Interrupt Handler */
    Active_I2Cx = I2Cx;

    /* ---- 2. Prepare DMA1 Stream 1 ---- */
    LL_DMA_DisableStream(I2C_DMA_INSTANCE, I2C_DMA_STREAM_TX);
    while (LL_DMA_IsEnabledStream(I2C_DMA_INSTANCE, I2C_DMA_STREAM_TX)) { /* wait */ }

    /* Clear pending DMA flags */
    LL_DMA_ClearFlag_TC1	(I2C_DMA_INSTANCE) ;
    LL_DMA_ClearFlag_HT1  	(I2C_DMA_INSTANCE);
    LL_DMA_ClearFlag_TE1  	(I2C_DMA_INSTANCE);
    LL_DMA_ClearFlag_FE1  	(I2C_DMA_INSTANCE);
    LL_DMA_ClearFlag_DME1 	(I2C_DMA_INSTANCE);

    /* Update Memory Address, peripheral address and Data Length for the new payload */
    LL_DMA_SetMemoryAddress(I2C_DMA_INSTANCE, I2C_DMA_STREAM_TX, (uint32_t)data);
    LL_DMA_SetPeriphAddress(I2C_DMA_INSTANCE, I2C_DMA_STREAM_TX,  (uint32_t)&I2Cx->DR);
    LL_DMA_SetDataLength   (I2C_DMA_INSTANCE, I2C_DMA_STREAM_TX, length);

    /* Enable DMA Transfer Complete Interrupt */
    LL_DMA_EnableIT_TC(I2C_DMA_INSTANCE, I2C_DMA_STREAM_TX);

    /* ---- 3. Manual I2C sequence: START + Slave Address + Register Address ---- */
    LL_I2C_GenerateStartCondition(I2Cx);
    while (!LL_I2C_IsActiveFlag_SB(I2Cx)) { /* wait for START */ }

    LL_I2C_TransmitData8(I2Cx, (uint8_t)(slaveAddress));
    while (!LL_I2C_IsActiveFlag_ADDR(I2Cx)) { /* wait for ACK */ }
    LL_I2C_ClearFlag_ADDR(I2Cx);

    if (regLen == 1)
          {
              LL_I2C_TransmitData8(I2Cx, regAddress & 0xFF);
              while(!LL_I2C_IsActiveFlag_TXE(I2Cx));
          }

	  else if (regLen == 2)
	  {
		  // Send regAddress high byte
		  LL_I2C_TransmitData8(I2Cx, (uint8_t)(regAddress >> 8));
		  while(!LL_I2C_IsActiveFlag_TXE(I2Cx));

		  // Send regAddress low byte
		  LL_I2C_TransmitData8(I2Cx, (uint8_t)(regAddress & 0xFF));
		  while(!LL_I2C_IsActiveFlag_TXE(I2Cx));
	  }

    /* Wait until register address is completely emptied from the DR */
    while (!LL_I2C_IsActiveFlag_TXE(I2Cx)) { }

    /* ---- 4. Arm DMA & I2C DMA Request ---- */
    LL_I2C_EnableDMAReq_TX(I2Cx);
    LL_DMA_EnableStream(I2C_DMA_INSTANCE, I2C_DMA_STREAM_TX);

    /* Function returns immediately. DMA pushes the payload in the background. */
}

__weak void I2C1_DMA_Tx_Completed(void)
{

}



void I2C_Mem_Read_DMA(I2C_TypeDef *I2Cx, uint8_t slaveAddress, uint16_t regAddress, uint8_t regLen, uint8_t *data, uint16_t length)
{
    if (length == 0) return;
    if ((regLen != 1) && (regLen != 2)) return;
    if (I2C_DMA_Busy) return; /* Prevent overlapping transfers */

    I2C_DMA_Busy = 1;

    /* Save I2C context for the Interrupt Handler */
    Active_I2Cx = I2Cx;

    /* ---- 1. Prepare DMA1 Stream 0 ---- */
    LL_DMA_DisableStream(I2C_DMA_INSTANCE, I2C_DMA_SREAM_RX);
    while (LL_DMA_IsEnabledStream(I2C_DMA_INSTANCE, I2C_DMA_SREAM_RX)) { /* wait */ }

    /* Clear pending DMA flags */
    LL_DMA_ClearFlag_TC0(I2C_DMA_INSTANCE);
    LL_DMA_ClearFlag_HT0(I2C_DMA_INSTANCE);
    LL_DMA_ClearFlag_TE0(I2C_DMA_INSTANCE);
    LL_DMA_ClearFlag_FE0(I2C_DMA_INSTANCE);
    LL_DMA_ClearFlag_DME0(I2C_DMA_INSTANCE);




    /* Update Memory Address, peripheral address and Data Length for the new payload */
    LL_DMA_SetMemoryAddress(I2C_DMA_INSTANCE, I2C_DMA_SREAM_RX, (uint32_t)data);
    LL_DMA_SetPeriphAddress(I2C_DMA_INSTANCE, I2C_DMA_SREAM_RX, (uint32_t)&I2Cx->DR);
    LL_DMA_SetDataLength(I2C_DMA_INSTANCE, I2C_DMA_SREAM_RX, length);

    /* Enable DMA Transfer Complete Interrupt */
    LL_DMA_EnableIT_TC(I2C_DMA_INSTANCE, I2C_DMA_SREAM_RX);

    /* ---- 2. Write phase: Send register address (Polling) ---- */
    // Note: Removed printfs to prevent I2C timeouts

    LL_I2C_GenerateStartCondition(I2Cx);
    while(!LL_I2C_IsActiveFlag_SB(I2Cx));

    LL_I2C_TransmitData8(I2Cx, (slaveAddress & 0xFE)); // Ensure Write bit (0)
    while(!LL_I2C_IsActiveFlag_ADDR(I2Cx));
    LL_I2C_ClearFlag_ADDR(I2Cx);

    if (regLen == 1)
    {
        LL_I2C_TransmitData8(I2Cx, regAddress & 0xFF);
        while(!LL_I2C_IsActiveFlag_TXE(I2Cx));
    }
    else if (regLen == 2)
    {
        LL_I2C_TransmitData8(I2Cx, (uint8_t)(regAddress >> 8));
        while(!LL_I2C_IsActiveFlag_TXE(I2Cx));

        LL_I2C_TransmitData8(I2Cx, (uint8_t)(regAddress & 0xFF));
        while(!LL_I2C_IsActiveFlag_TXE(I2Cx));
    }

    /* ---- 3. Read phase: Get data bytes via DMA ---- */
    LL_I2C_GenerateStartCondition(I2Cx);
    while(!LL_I2C_IsActiveFlag_SB(I2Cx));

    LL_I2C_TransmitData8(I2Cx, (slaveAddress | 0x01)); // Ensure Read bit (1)

    while(!LL_I2C_IsActiveFlag_ADDR(I2Cx));

    /* CRITICAL I2C DMA READ RULE:
       To properly receive N bytes via DMA, NACK must be set BEFORE clearing ADDR flag.
       If length == 1, set NACK. If length > 1, set ACK, but DMA/Peripheral will handle NACK
       for the last byte if EVERR/TCIE is managed.
       Simplification for DMA: Set NACK before clearing ADDR */
    if(length == 1)
    {
        LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_NACK);
    }
    else
    {
        LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_ACK);
    }

    LL_I2C_ClearFlag_ADDR(I2Cx);

    /* ---- 4. Arm DMA & I2C DMA Request ---- */
    /* CRITICAL FIX: Enable RX DMA request, NOT TX */
    LL_I2C_EnableDMAReq_RX(I2Cx);

    /* Start the DMA Stream */
    LL_DMA_EnableStream(I2C_DMA_INSTANCE, I2C_DMA_SREAM_RX);
}



__weak void I2C1_DMA_Rx_Completed(void)
{

}



void DMA1_Stream0_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TC0(I2C_DMA_INSTANCE))
    {
        LL_DMA_ClearFlag_TC0(I2C_DMA_INSTANCE);

        /* Disable DMA TC Interrupt and Stream */
        LL_DMA_DisableIT_TC(I2C_DMA_INSTANCE, I2C_DMA_SREAM_RX);
        LL_DMA_DisableStream(I2C_DMA_INSTANCE, I2C_DMA_SREAM_RX);

        /* Terminate I2C Transaction */
        LL_I2C_GenerateStopCondition(Active_I2Cx);

        /* CRITICAL FIX: Disable RX DMA request */
        LL_I2C_DisableDMAReq_RX(Active_I2Cx);

        /* Mark transfer as complete */
        I2C_DMA_Busy = 0;

        I2C1_DMA_Rx_Completed();
    }
}


void DMA1_Stream1_IRQHandler(void)
{


    /* Check if Transfer Complete flag is set */
    if (LL_DMA_IsActiveFlag_TC1(I2C_DMA_INSTANCE))
    {
        LL_DMA_ClearFlag_TC1(I2C_DMA_INSTANCE);

        /* Disable DMA TC Interrupt and Stream */
        LL_DMA_DisableIT_TC(I2C_DMA_INSTANCE,I2C_DMA_STREAM_TX);
        LL_DMA_DisableStream(I2C_DMA_INSTANCE,I2C_DMA_STREAM_TX);

        /* Wait for I2C Byte Transfer Finished (last byte physically shifted out) */
        while (!LL_I2C_IsActiveFlag_BTF(Active_I2Cx)) { }

        /* Terminate I2C Transaction */
        LL_I2C_GenerateStopCondition(Active_I2Cx);
        LL_I2C_DisableDMAReq_TX(Active_I2Cx);

        /* Mark transfer as complete */
        I2C_DMA_Busy = 0;

        I2C1_DMA_Tx_Completed();
    }
}


/* USER CODE END 1 */

