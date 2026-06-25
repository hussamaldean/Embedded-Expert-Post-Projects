/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "i2s.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "math.h"
#include "CS43L22.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//#define dataSize 100
//
#define F_SAMPLE	96000.0f
#define F_OUT		4000.0f
#define PI 			3.14159f

#define SAMPLES_PER_CYCLE	(F_SAMPLE / F_OUT)  // 24.0
#define NUM_CYCLES			10
#define BUFFER_SIZE			((int)(SAMPLES_PER_CYCLE * NUM_CYCLES) * 2)  // *2 for stereo

uint16_t dataI2S[BUFFER_SIZE];  // 480 elements for 10 cycles

float phase_increment = 2.0f * PI / SAMPLES_PER_CYCLE;
float current_phase = 0.0f;

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

//uint16_t dataI2S[dataSize];
//float mySinVal;
//float sample_dt;
//uint16_t sample_N;
//uint16_t i_t;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	if(hi2s->Instance==SPI3)
	{
		/*For debugging using oscilloscope*/
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_0);
	}

}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2S3_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
	CS43_Init(CS43L22_MODE_I2S);
	CS43_SetVolume(100);
	CS43_Enable_RightLeft(CS43_RIGHT_LEFT);
	CS43_Start();



	float phase_increment = 2.0f * PI / SAMPLES_PER_CYCLE;
	float current_phase = 0.0f;

	for(uint16_t i=0; i<BUFFER_SIZE/2; i++)  // 240 stereo pairs
	{
	    float mySinVal = sinf(current_phase);

	    // Scale to 16-bit signed range
	    int16_t right_value = (int16_t)(mySinVal * 32000.0f);
	    int16_t left_value = -right_value;  // Inverted signal

	    dataI2S[i*2] = right_value;      // Right channel
	    dataI2S[i*2 + 1] = left_value;   // Left channel (inverted)

	    current_phase += phase_increment;

	    // Keep phase in 0 to 2*PI range to prevent floating point drift
	    if(current_phase >= 2.0f * PI) {
	        current_phase -= 2.0f * PI;
	    }
	}

	HAL_I2S_Transmit_DMA(&hi2s3, dataI2S, BUFFER_SIZE);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  //tempData[0]=0xFFFF;
	  //tempData[1]=0xBEEF;
	  //tempData[2]=0xDEAD;
	  //HAL_I2S_Transmit(&hi2s3, tempData, 3,10);
	  HAL_Delay(10);
//	  for (int i=0;i<65535;i++)
//	  {
//
//		  HAL_I2S_Transmit(&hi2s3, tempData, 2,10);
//		  HAL_Delay(1);
//	  }

	  //HAL_I2S_Transmit(&hi2s3, dataI2S, sample_N, 1000);
  }

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
