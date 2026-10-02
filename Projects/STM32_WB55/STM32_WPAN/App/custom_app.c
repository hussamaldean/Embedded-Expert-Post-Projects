/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    App/custom_app.c
  * @author  MCD Application Team
  * @brief   Custom Example Application (Server)
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
#include "app_common.h"
#include "dbg_trace.h"
#include "ble.h"
#include "custom_app.h"
#include "custom_stm.h"
#include "stm32_seq.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  /* My_Services */
  uint8_t               My_tx_char_Notification_Status;
  /* USER CODE BEGIN CUSTOM_APP_Context_t */

  /* USER CODE END CUSTOM_APP_Context_t */

  uint16_t              ConnectionHandle;
} Custom_App_Context_t;

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macros -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/**
 * START of Section BLE_APP_CONTEXT
 */

static Custom_App_Context_t Custom_App_Context;

/**
 * END of Section BLE_APP_CONTEXT
 */

uint8_t UpdateCharData[512];
uint8_t NotifyCharData[512];
uint16_t Connection_Handle;
/* USER CODE BEGIN PV */
#define MAX_DATA_LENGTH 20
static uint8_t  rxBuffer[MAX_DATA_LENGTH];
static uint16_t rxDataLength = 0;
static uint8_t  newDataReceived = 0;
static uint8_t  notificationsEnabled = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* My_Services */
static void Custom_My_tx_char_Update_Char(void);

static void Custom_My_tx_char_Send_Notification(void);

/* USER CODE BEGIN PFP */
uint8_t Send_Data_To_Phone(uint8_t *data, uint16_t length);
/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
void Custom_STM_App_Notification(Custom_STM_App_Notification_evt_t *pNotification)
{
  /* USER CODE BEGIN CUSTOM_STM_App_Notification_1 */

  /* USER CODE END CUSTOM_STM_App_Notification_1 */
  switch (pNotification->Custom_Evt_Opcode)
  {
    /* USER CODE BEGIN CUSTOM_STM_App_Notification_Custom_Evt_Opcode */

    /* USER CODE END CUSTOM_STM_App_Notification_Custom_Evt_Opcode */

    /* My_Services */
    case CUSTOM_STM_MY_TX_CHAR_NOTIFY_ENABLED_EVT:
      /* USER CODE BEGIN CUSTOM_STM_MY_TX_CHAR_NOTIFY_ENABLED_EVT */
    	notificationsEnabled = 1;
      /* USER CODE END CUSTOM_STM_MY_TX_CHAR_NOTIFY_ENABLED_EVT */
      break;

    case CUSTOM_STM_MY_TX_CHAR_NOTIFY_DISABLED_EVT:
      /* USER CODE BEGIN CUSTOM_STM_MY_TX_CHAR_NOTIFY_DISABLED_EVT */
    	notificationsEnabled = 0;
      /* USER CODE END CUSTOM_STM_MY_TX_CHAR_NOTIFY_DISABLED_EVT */
      break;

    case CUSTOM_STM_MY_RX_CHAR_WRITE_NO_RESP_EVT:
      /* USER CODE BEGIN CUSTOM_STM_MY_RX_CHAR_WRITE_NO_RESP_EVT */
    	uint16_t receivedLength = pNotification->DataTransfered.Length;
    	        uint8_t *receivedData = pNotification->DataTransfered.pPayload;

    	        // Copy received data to our safe buffer
    	        memcpy(rxBuffer, receivedData, receivedLength);
    	        rxDataLength = receivedLength;

    	        // Echo back the received data to confirm (optional debug print)
    	        APP_DBG_MSG(">> Data received from phone! Length: %d bytes\n", rxDataLength);

    	        // Example processing:
    	        if (rxDataLength >= 1) {
    	            switch (rxBuffer[0]) {
    	                case 0x01:
    	                    APP_DBG_MSG(">> Command 01 received\n");
    	                    // Do something
    	                    break;
    	                case 0x02:
    	                    APP_DBG_MSG(">> Command 02 received\n");
    	                    // Do something else
    	                    break;
    	                default:
    	                    APP_DBG_MSG(">> Unknown command: 0x%02X\n", rxBuffer[0]);
    	                    break;
    	            }
    	        }
      /* USER CODE END CUSTOM_STM_MY_RX_CHAR_WRITE_NO_RESP_EVT */
      break;

    case CUSTOM_STM_NOTIFICATION_COMPLETE_EVT:
      /* USER CODE BEGIN CUSTOM_STM_NOTIFICATION_COMPLETE_EVT */

      /* USER CODE END CUSTOM_STM_NOTIFICATION_COMPLETE_EVT */
      break;

    default:
      /* USER CODE BEGIN CUSTOM_STM_App_Notification_default */

      /* USER CODE END CUSTOM_STM_App_Notification_default */
      break;
  }
  /* USER CODE BEGIN CUSTOM_STM_App_Notification_2 */

  /* USER CODE END CUSTOM_STM_App_Notification_2 */
  return;
}

void Custom_APP_Notification(Custom_App_ConnHandle_Not_evt_t *pNotification)
{
  /* USER CODE BEGIN CUSTOM_APP_Notification_1 */

  /* USER CODE END CUSTOM_APP_Notification_1 */

  switch (pNotification->Custom_Evt_Opcode)
  {
    /* USER CODE BEGIN CUSTOM_APP_Notification_Custom_Evt_Opcode */

    /* USER CODE END P2PS_CUSTOM_Notification_Custom_Evt_Opcode */
    case CUSTOM_CONN_HANDLE_EVT :
      /* USER CODE BEGIN CUSTOM_CONN_HANDLE_EVT */

      /* USER CODE END CUSTOM_CONN_HANDLE_EVT */
      break;

    case CUSTOM_DISCON_HANDLE_EVT :
      /* USER CODE BEGIN CUSTOM_DISCON_HANDLE_EVT */

      /* USER CODE END CUSTOM_DISCON_HANDLE_EVT */
      break;

    default:
      /* USER CODE BEGIN CUSTOM_APP_Notification_default */

      /* USER CODE END CUSTOM_APP_Notification_default */
      break;
  }

  /* USER CODE BEGIN CUSTOM_APP_Notification_2 */

  /* USER CODE END CUSTOM_APP_Notification_2 */

  return;
}

void Custom_APP_Init(void)
{
  /* USER CODE BEGIN CUSTOM_APP_Init */

  /* USER CODE END CUSTOM_APP_Init */
  return;
}

/* USER CODE BEGIN FD */
uint8_t Send_Data_To_Phone(uint8_t *data, uint16_t length)
{
    if (!notificationsEnabled) return 0;
    if (length > MAX_DATA_LENGTH) length = MAX_DATA_LENGTH;
    memcpy(UpdateCharData, data, length);
    Custom_STM_App_Update_Char(CUSTOM_STM_MY_TX_CHAR, UpdateCharData);
    return 1;
}

uint16_t Get_Data_From_Phone(uint8_t *buffer, uint16_t maxLength) {
    uint16_t copyLength = (rxDataLength < maxLength) ? rxDataLength : maxLength;
    memcpy(buffer, rxBuffer, copyLength);
    return copyLength;
}

/**
 * @brief Simple test function to send an array
 */
void Send_Test_Data(void) {
    static uint8_t counter = 0;
    uint8_t testData[4];

    testData[0] = 0xAA;  // Header
    testData[1] = counter++;
    testData[2] = ~counter;
    testData[3] = 0x55;  // Footer

    Send_Data_To_Phone(testData, 4);
    APP_DBG_MSG(">> Sent test data: counter = %d\n", testData[1]);
}
/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/* My_Services */
__USED void Custom_My_tx_char_Update_Char(void) /* Property Read */
{
  uint8_t updateflag = 0;

  /* USER CODE BEGIN My_tx_char_UC_1*/

  /* USER CODE END My_tx_char_UC_1*/

  if (updateflag != 0)
  {
    Custom_STM_App_Update_Char(CUSTOM_STM_MY_TX_CHAR, (uint8_t *)UpdateCharData);
  }

  /* USER CODE BEGIN My_tx_char_UC_Last*/

  /* USER CODE END My_tx_char_UC_Last*/
  return;
}

__USED void Custom_My_tx_char_Send_Notification(void) /* Property Notification */
{
  uint8_t updateflag = 0;

  /* USER CODE BEGIN My_tx_char_NS_1*/

  /* USER CODE END My_tx_char_NS_1*/

  if (updateflag != 0)
  {
    Custom_STM_App_Update_Char(CUSTOM_STM_MY_TX_CHAR, (uint8_t *)UpdateCharData);
  }

  /* USER CODE BEGIN My_tx_char_NS_Last*/

  /* USER CODE END My_tx_char_NS_Last*/

  return;
}

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS*/



/* USER CODE END FD_LOCAL_FUNCTIONS*/
