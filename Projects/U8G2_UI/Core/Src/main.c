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
#include "i2c.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "math.h"
#include "u8g2.h"
#include "stdio.h"
#include "math.h"
#include "stdlib.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

u8g2_t myDisplay;

// --- UI and Simulation Variables ---
static float sim_temp = 25.0f;
static float sim_cpu = 50.0f;
static uint8_t sim_load = 50;
static uint32_t start_tick = 0;



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
	switch(msg)
	{
		case U8X8_MSG_DELAY_MILLI:
			HAL_Delay(arg_int);
			break;
		case U8X8_MSG_GPIO_CS:
			HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, arg_int);
			break;
		case U8X8_MSG_GPIO_DC:
			HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, arg_int);
			break;

		case U8X8_MSG_GPIO_RESET:
			HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, arg_int);
			break;


	}
	return 1;
}

uint8_t u8x8_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{

	switch(msg)
	{
		case U8X8_MSG_BYTE_SET_DC:
			HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, arg_int);
			break;
		case U8X8_MSG_BYTE_SEND:
			HAL_SPI_Transmit(&hspi1, (uint8_t *)arg_ptr, arg_int, 1000);
			break;
		case U8X8_MSG_BYTE_START_TRANSFER:
			HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
			break;
		case U8X8_MSG_BYTE_END_TRANSFER:
			HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
			break;

		default: break;

	}
	return 1;
}

uint8_t u8x8_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  static uint8_t buffer[32];		/* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
  static uint8_t buf_idx;
  uint8_t *data;

  switch(msg)
  {
    case U8X8_MSG_BYTE_SEND:
      data = (uint8_t *)arg_ptr;
      while( arg_int > 0 )
      {
	buffer[buf_idx++] = *data;
	data++;
	arg_int--;
      }
      break;
    case U8X8_MSG_BYTE_START_TRANSFER:
      buf_idx = 0;
      break;
    case U8X8_MSG_BYTE_END_TRANSFER:
		#define OLED_Addr (0x3C<<1)
    	HAL_I2C_Master_Transmit(&hi2c1, OLED_Addr, buffer, buf_idx, 100);
      break;
    default:
      return 0;
  }
  return 1;
}

// Helper function to draw a horizontal progress bar
void draw_progress_bar(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t percentage) {
    // 1. Draw the outer border
    u8g2_DrawFrame(&myDisplay, x, y, w, h);

    // 2. Calculate fill width based on percentage
    uint8_t fill_w = (uint8_t)((w - 2) * percentage / 100.0f);

    // 3. Draw the solid fill inside the border
    if (fill_w > 0) {
        u8g2_DrawBox(&myDisplay, x + 1, y + 1, fill_w, h - 2);
    }
}

// The main UI rendering function
void render_ui(void) {
    // 1. Gather data
    uint32_t uptime = (HAL_GetTick() - start_tick) / 1000; // Seconds since boot

    // 2. Format data into strings
    char temp_str[20];
    char cpu_str[20];
    char time_str[20];
    char load_str[20];

    // Convert seconds to HH:MM:SS
    uint32_t h = uptime / 3600;
    uint32_t m = (uptime % 3600) / 60;
    uint32_t s = uptime % 60;

    snprintf(temp_str, sizeof(temp_str), "Core:  %.1f C", sim_temp);
    snprintf(cpu_str, sizeof(cpu_str), "CPU:   %.1f %%", sim_cpu);
    snprintf(time_str, sizeof(time_str), "Time:  %02d:%02d:%02d", h, m, s);
    snprintf(load_str, sizeof(load_str), "%d%%", sim_load);

    // 3. Clear the internal buffer
    u8g2_ClearBuffer(&myDisplay);

    // 4. Draw the Header (Using a bold font)
    u8g2_SetFont(&myDisplay, u8g2_font_6x13B_tr);
    u8g2_DrawStr(&myDisplay, 0, 10, "TEMP STATUS");
    u8g2_DrawHLine(&myDisplay, 0, 13, 128); // Underline the header

    // 5. Draw the Text Data (Using a standard font)
    u8g2_SetFont(&myDisplay, u8g2_font_6x10_tr);
    u8g2_DrawStr(&myDisplay, 0, 28, temp_str);
    u8g2_DrawStr(&myDisplay, 0, 42, cpu_str);
    u8g2_DrawStr(&myDisplay, 0, 54, time_str);

    // 6. Draw the Progress Bar
    u8g2_SetFont(&myDisplay, u8g2_font_5x7_tr);
    u8g2_DrawStr(&myDisplay, 0, 63, "LOAD");
    draw_progress_bar(25, 56, 85, 7, sim_load);

    // Draw percentage text at the end of the bar
    u8g2_DrawStr(&myDisplay, 112, 63, load_str);

    // 7. Send the buffer to the screen
    u8g2_SendBuffer(&myDisplay);
}




#define NUM_BARS 16
#define MAX_BAR_HEIGHT 40

void render_audio_visualizer(void) {
    static uint32_t last_update = 0;
    static float time_counter = 0.0f;

    /* Persistent peak markers that slowly fall down */
    static uint8_t peak_heights[NUM_BARS] = {0};
    static float peak_fall_speed[NUM_BARS] = {0};

    uint32_t now = HAL_GetTick();

    /* Update ~30 frames per second (non-blocking) */
    if (now - last_update > 33) {
        last_update = now;
        time_counter += 0.15f;

        u8g2_ClearBuffer(&myDisplay);

        /* 1. Draw UI Header */
        u8g2_SetFont(&myDisplay, u8g2_font_5x7_tr);
        u8g2_DrawStr(&myDisplay, 0, 7, "AUDIO SCAN");

        /* Sweeping needle that triggers the bars */
        uint8_t needle_x = (uint8_t)((sinf(time_counter * 0.5f) + 1.0f) * 60.0f);
        u8g2_DrawLine(&myDisplay, needle_x, 2, needle_x + 4, 7);
        u8g2_DrawLine(&myDisplay, needle_x, 2, needle_x - 4, 7);
        u8g2_DrawVLine(&myDisplay, needle_x, 0, 8);

        /* Right aligned dB text */
        u8g2_DrawStr(&myDisplay, 100, 7, "-12.4dB");

        u8g2_DrawHLine(&myDisplay, 0, 9, 128);

        /* 2. Draw the 16 Spectrum Bars */
        for (int i = 0; i < NUM_BARS; i++) {
            /* Simulate an audio curve (lower freqs have higher amplitude, falls off to right) */
            float base_amp = (1.0f - (float)i / NUM_BARS) * 0.6f + 0.1f;

            /* Create dynamic waveforms with slightly different sine frequencies */
            float wave1 = sinf(time_counter + i * 0.3f);
            float wave2 = sinf(time_counter * 1.3f + i * 0.1f);
            float combined_wave = (wave1 + wave2) / 2.0f; /* Range: -1.0 to 1.0 */

            uint8_t bar_val = (uint8_t)((combined_wave + 1.0f) * base_amp * (MAX_BAR_HEIGHT / 2.0f));

            /* Force trigger specific bars when the needle passes over them */
            if (abs(needle_x - (i * 8 + 4)) < 6) {
                bar_val = MAX_BAR_HEIGHT - (rand() % 10);
            }

            if (bar_val > MAX_BAR_HEIGHT) bar_val = MAX_BAR_HEIGHT;

            /* Update persistent peaks */
            if (bar_val > peak_heights[i]) {
                peak_heights[i] = bar_val;
                peak_fall_speed[i] = 0;
            } else {
                peak_fall_speed[i] += 0.2f; /* Gravity accelerating */
                peak_heights[i] -= (uint8_t)peak_fall_speed[i];
                if (peak_heights[i] > MAX_BAR_HEIGHT) peak_heights[i] = 0; /* Underflow check */
            }

            /* Draw the main bar (Solid) */
            uint8_t bar_x = i * 8;
            uint8_t bar_y = 10 + (MAX_BAR_HEIGHT - bar_val);
            u8g2_DrawBox(&myDisplay, bar_x + 1, bar_y, 6, bar_val);

            /* Draw the peak marker (line on top) */
            uint8_t peak_y = 10 + (MAX_BAR_HEIGHT - peak_heights[i]);
            u8g2_DrawHLine(&myDisplay, bar_x + 1, peak_y, 6);

            /* Draw a centered baseline shadow */
            uint8_t mid_y = 10 + (MAX_BAR_HEIGHT / 2);
            u8g2_DrawHLine(&myDisplay, bar_x + 1, mid_y, 6);
        }

        /* 3. Draw the Sub-bass bouncing bar at the bottom */
        float sub_wave = (sinf(time_counter * 0.8f) + 1.0f) / 2.0f; /* 0.0 to 1.0 */
        uint8_t sub_h = (uint8_t)(sub_wave * 10.0f);
        u8g2_DrawBox(&myDisplay, 0, 64 - sub_h, 128, sub_h);

        /* 4. Draw moving tick marks over the sub bar */
        for (int i = 0; i < 8; i++) {
            int tick_x = ((int)(time_counter * 10) + i * 20) % 140 - 10;
            u8g2_DrawVLine(&myDisplay, tick_x, 60, 4);
        }

        u8g2_SendBuffer(&myDisplay);
    }
}

/* Add to your main loop:
while(1) {
    render_audio_visualizer();
}
*/




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
  MX_SPI1_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  //u8g2_Setup_st7920_s_128x64_f(&myDisplay, U8G2_R0, u8x8_spi, u8x8_gpio_and_delay);
  u8g2_Setup_ssd1306_i2c_128x64_noname_f(&myDisplay, U8G2_R0, u8x8_i2c, u8x8_gpio_and_delay);

    u8g2_InitDisplay(&myDisplay);
    u8g2_SetPowerSave(&myDisplay, 0);
    u8g2_ClearBuffer(&myDisplay);

	// Record start time
	start_tick = HAL_GetTick();

	uint32_t last_update = 0;
	float time_counter = 0.0f; // Used for sine wave simulation
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//	  uint32_t now = HAL_GetTick();
////
////	  // Update UI roughly 20 times per second (every 50ms)
//	          if (now - last_update > 50) {
//	              last_update = now;
//
//	              // Increment time counter (0.05f matches the 50ms interval)
//	              time_counter += 0.05f;
//
//	              // --- Simulate Data Changing ---
//
//	              // Temperature oscillates smoothly between 20.0 and 45.0 C
//	              // sin(x) returns -1 to 1. We scale it to 0 to 1, then multiply by 25, and add 20.
//	              sim_temp = 20.0f + ((sinf(time_counter * 0.5f) + 1.0f) / 2.0f) * 25.0f;
//
//	              // CPU oscillates smoothly between 10% and 90%
//	              sim_cpu = 10.0f + ((sinf(time_counter * 0.8f + 1.0f) + 1.0f) / 2.0f) * 80.0f;
//
//	              // Load increments and decrements based on CPU
//	              // Use a sine wave for smooth 0-100% sweeping
//	              sim_load = (uint8_t)(((sinf(time_counter * 0.4f) + 1.0f) / 2.0f) * 100.0f);
//
//	              // --- Render the UI ---
//	              render_ui();
//	          }

	  render_audio_visualizer();
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
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 100;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
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
