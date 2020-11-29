/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

//For sprintf and memcpy
#include "string.h"
//For atoi
#include "stdlib.h"
//For ceil()
#include "math.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

//UJA Register Definitions

#define UJA_REG_WDGANDSTATUS 0
#define UJA_REG_MODECONTROL 1
#define UJA_REG_INTCON 2
#define UJA_REG_INTREAD 3

//WD_and_Status register
//RO
#define UJA_RO_RW 0
#define UJA_RO_R 1

//WMC
#define UJA_WMC_WND 0
#define UJA_WMC_TO 1

//NWP
#define UJA_NWP_8 0
#define UJA_NWP_16 1
#define UJA_NWP_32 2
#define UJA_NWP_64 3
#define UJA_NWP_128 4
#define UJA_NWP_256 5

//Mode_Control register
#define UJA_MC_STBY 0
#define UJA_MC_SLP 1
#define UJA_MC_V2OFF 2
#define UJA_MC_V2ON 3

//Int_Control register
#define UJA_V1UIE_OFF 0
#define UJA_V1UIE_ON 1

#define UJA_V2UIE_OFF 0
#define UJA_V2UIE_ON 1


//Message struct parameter
#define MSG_ENABLED 1
#define MSG_DISABLED 0



/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for ProcessCommand */
osThreadId_t ProcessCommandHandle;
const osThreadAttr_t ProcessCommand_attributes = {
  .name = "ProcessCommand",
  .priority = (osPriority_t) osPriorityHigh,
  .stack_size = 128 * 4
};
/* Definitions for SendTelemetry */
osThreadId_t SendTelemetryHandle;
const osThreadAttr_t SendTelemetry_attributes = {
  .name = "SendTelemetry",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for FeedWDG */
osThreadId_t FeedWDGHandle;
const osThreadAttr_t FeedWDG_attributes = {
  .name = "FeedWDG",
  .priority = (osPriority_t) osPriorityRealtime,
  .stack_size = 128 * 4
};
/* Definitions for ProcessCommandSem */
osSemaphoreId_t ProcessCommandSemHandle;
const osSemaphoreAttr_t ProcessCommandSem_attributes = {
  .name = "ProcessCommandSem"
};
/* USER CODE BEGIN PV */

struct message {
	uint32_t id;
	uint8_t  bit;
	uint8_t length;
	int32_t value;
	uint8_t enabled;
};


struct message messageArray[16];

const struct message defaultMessageArray[16] = {

		{ 0x360,   0,  16,   -1,  MSG_ENABLED  },  //Engine RPM
		{ 0x360,  32,  16,   -1,  MSG_ENABLED  },  //Throttle Position

		{ 0x361,  16,  16,   -1,  MSG_ENABLED  },  //Oil Pressure

		{ 0x3E0,   0,  16,   -1,  MSG_ENABLED  },  //Coolant Temperature

		{ 0x390,   0,  16,   -1,  MSG_ENABLED  },  //Brake Pressure
		{ 0x390,   0,  16,   -1,  MSG_ENABLED  },  //Brake Bias
		{ 0x390,   0,  16,   -1,  MSG_ENABLED  },  //Lat Accel
		{ 0x390,   0,  16,   -1,  MSG_ENABLED  },  //Long Accel
		{ 0x390,   0,  16,   -1,  MSG_ENABLED  },  //GPS Speed
		{ 0x390,   0,  16,   -1,  MSG_ENABLED  },  //Oil Temperature

		{ 0x373,   0,  16,   -1,  MSG_ENABLED  },  //EGT 1

		{ 0x368,   0,  16,   -1,  MSG_ENABLED  },  //Wideband

		{ 0x3EB,  32,  16,   -1,  MSG_ENABLED  },  //Ignition Angle

		{     0,   0,   0,   -1,  MSG_DISABLED },  //Disabled
		{     0,   0,   0,   -1,  MSG_DISABLED },  //Disabled
		{     0,   0,   0,   -1,  MSG_DISABLED }   //Disabled
};


uint8_t canData0[8];
CAN_RxHeaderTypeDef canHeader0;

uint8_t uart_rec_buff[24];
char uart_tx_buff[128];

uint8_t cmdbuff[24];
int8_t cmdbuffind = 0;

const uint8_t WD_SETUP = (UJA_REG_WDGANDSTATUS << 5) | (UJA_RO_RW << 4) | (UJA_WMC_WND << 3) | (UJA_NWP_128);

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART1_UART_Init(void);
void StartDefaultTask(void *argument);
void StartProcessCommand(void *argument);
void StartSendTelemetry(void *argument);
void StartFeedWDG(void *argument);

static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */

void DebugPrint(char *msg);
void Init_SBC(void);
void ConfigureCANFilters(struct message * messageArray, uint8_t size);
void Init_CAN(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//Prints message to UART1 for debugging
void DebugPrint(char *msg) {
	HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}


//Initializes UJA SBC
void Init_SBC(void) {

	HAL_GPIO_WritePin(UJA_CS_GPIO_Port, UJA_CS_Pin, 0);

	uint8_t SBC_Setup[2];

	//Setup WDG and Status register
	SBC_Setup[0] = 0x0;
	SBC_Setup[1] = WD_SETUP;


	//data[0] = WD_SETUP << 8;

	//result = HAL_SPI_Transmit(&hspi1, (uint8_t *)data, 2, 100);

	//Setup Mode Control register
	SBC_Setup[0] = 0x0;
	SBC_Setup[1] = (UJA_REG_MODECONTROL << 5) | (UJA_RO_RW << 4) | (UJA_MC_V2ON << 2);

	uint16_t data[] = {0b0010110000000000};
	HAL_SPI_Transmit(&hspi1, (uint8_t *)data, 1, 100);

	HAL_GPIO_WritePin(UJA_CS_GPIO_Port, UJA_CS_Pin, 1);
}



//Procedurally generates and sets CAN Filter configurations from the message[] struct array config
void ConfigureCANFilters(struct message * messageArray, uint8_t size) {
	uint32_t configuredIDs[size];
	int uniques = 0;
	for (int i = 0; i < size; i++) {
		struct message thismessage = messageArray[i];
		int create = 1;
		for (int j = 0; j < size; j++) {
			if (configuredIDs[j] == thismessage.id) {
				create = 0;
			}
		}
		if (create == 1 && thismessage.enabled) {
			//Add this ID to the list of already configured ID's to skip duplicates
			configuredIDs[uniques] = thismessage.id;
			DebugPrint("Creating new filter\r\n");
			CAN_FilterTypeDef filter;

			//This bit shifting was a massive PITA to figure out... see page 1092 of the RM for reasoning
			filter.FilterIdHigh = ((thismessage.id << 5)  | (thismessage.id >> (32 - 5))) & 0xFFFF;
			filter.FilterIdLow = (thismessage.id >> (11 - 3)) & 0xFFF8;

			//Masks set to full rank to check every bit against ID
			filter.FilterMaskIdHigh = 0xFFFF;
			filter.FilterMaskIdLow = 0xFFFF;

			//Filter options
			filter.FilterScale = CAN_FILTERSCALE_32BIT;
			filter.FilterActivation = ENABLE;
			filter.FilterMode = CAN_FILTERMODE_IDMASK;
			filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;

			//Set filter bank to the current count of uniques
			filter.FilterBank = uniques;

			//Finally pass filter to HAL
			HAL_CAN_ConfigFilter(&hcan, &filter);
			uniques++;
		}
	}
}

void Init_CAN(void) {
	//Configure all receive filters from the config array
	ConfigureCANFilters(messageArray, sizeof(messageArray) / sizeof(struct message));

	//Start CAN operation
	HAL_CAN_Start(&hcan);

	//Enable IRQ's
	HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);

	//Start receiving - don't think we need this here
	HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &canHeader0, canData0);
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
  MX_CAN_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */

  //Load default message configuration
  memcpy(&messageArray, &defaultMessageArray, sizeof(messageArray));

  Init_CAN();

  Init_SBC();

  //Start receiving UART
	HAL_UART_Receive_IT(&huart2, uart_rec_buff, 1);


  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of ProcessCommandSem */
  ProcessCommandSemHandle = osSemaphoreNew(1, 1, &ProcessCommandSem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of ProcessCommand */
  ProcessCommandHandle = osThreadNew(StartProcessCommand, NULL, &ProcessCommand_attributes);

  /* creation of SendTelemetry */
  SendTelemetryHandle = osThreadNew(StartSendTelemetry, NULL, &SendTelemetry_attributes);

  /* creation of FeedWDG */
  FeedWDGHandle = osThreadNew(StartFeedWDG, NULL, &FeedWDG_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL8;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* CAN_RX0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(CAN_RX0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(CAN_RX0_IRQn);
  /* CAN_RX1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(CAN_RX1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(CAN_RX1_IRQn);
  /* USART2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN;
  hcan.Init.Prescaler = 4;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_2TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_5TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = ENABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 38400;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, UJA_CS_Pin|LD2_Pin|LD1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : UJA_CS_Pin LD2_Pin LD1_Pin */
  GPIO_InitStruct.Pin = UJA_CS_Pin|LD2_Pin|LD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

//IRQ's

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

	//Check if delete received from debug uart and only delete if theres entered characters
	if (uart_rec_buff[0] == (uint8_t)127 && huart == &huart1) {
		if (cmdbuffind > 0) {
			DebugPrint((char *)uart_rec_buff);
			cmdbuffind--;
		}
	} else {
		//Send char to debug console
		DebugPrint((char *)uart_rec_buff);

		//Check if enter or cmdbuff reaches its limit (prevents overflow)
		if (uart_rec_buff[0] == *(uint8_t *)"\r" || cmdbuffind > 23) {
			DebugPrint("\n>");
			//Pass command onto task
			osSemaphoreRelease(ProcessCommandSemHandle);

		} else {
			cmdbuff[cmdbuffind] = uart_rec_buff[0];
			cmdbuffind++;
		}
	}
	//Start receiving again
	HAL_UART_Receive_IT(huart, uart_rec_buff, 1);
}


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &canHeader0, canData0);
	HAL_GPIO_TogglePin(LD1_GPIO_Port,LD1_Pin);
	//Parse received bytes using message array
	for(int i=0; i < sizeof(messageArray) / sizeof(struct message); i++){
		if(messageArray[i].id == canHeader0.StdId){
			//Calculate which byte position to start at
			int bytepos = messageArray[i].bit / 8;

			//Calculate number of bytes that need to be checked
			int bytes = ceil(messageArray[i].length / (float)8);

			uint32_t finalval = 0;
			int j = 0;
			//Iterate through all bytes that must be read for this message
			for (int b = bytepos; b < bytepos + bytes; b++) {
				uint8_t tempval;
				//If on last byte we may need to truncate unneeded parts dependent on length of data
				if (b == bytepos + bytes - 1) {
					//We need a left shift and a right shift to extract the bits we want
					uint8_t byteoffset = (messageArray[i].length - ((bytes-1) * 8));
					uint8_t leftshift = 8 - (messageArray[i].bit - (bytepos * 8)) - byteoffset;
					uint8_t rightshift = 8 - byteoffset;
					tempval = (canData0[b] << leftshift) >>  rightshift;
				} else {
					//Use the whole byte
					tempval = canData0[b];
				}
				//Calculate the size of the next data section to find the required shift
				int nextsize = messageArray[i].length - ((j+1) * 8);
				//Limit it to 0
				if (nextsize < 0) {
					nextsize = 0;
				}
				finalval += tempval << nextsize;
				j++;
			}
			//Finally set final value into message array struct
			messageArray[i].value = finalval;
		}
	}
}


/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(500);
    CAN_TxHeaderTypeDef header;
		header.StdId = 0x400;
		header.DLC = 2;
		header.IDE = CAN_ID_STD;
		header.RTR = CAN_RTR_DATA;

		uint32_t mailbox;
		uint8_t senddata[2] = {0, 100};

		HAL_CAN_AddTxMessage(&hcan, &header, senddata, &mailbox);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartProcessCommand */
/**
* @brief Function implementing the ProcessCommand thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartProcessCommand */
void StartProcessCommand(void *argument)
{
  /* USER CODE BEGIN StartProcessCommand */
  /* Infinite loop */
  for(;;)
  {
  	//Wait for semaphore passed by UART IRQ when the command buffer is ready for processing
    osSemaphoreAcquire(ProcessCommandSemHandle, osWaitForever);

    //Evaluate first char of the command buffer to determine command
		if (cmdbuff[0] == *(uint8_t *)"r") {
			//Restart command
			DebugPrint("Rebooting...\r\n\r\n");
			NVIC_SystemReset();

		} else if (cmdbuff[0] == *(uint8_t *)"d") {
			//Retreives the last value in message buffer for corrosponding ID
			int filter = atoi((char *)&cmdbuff[1]);
			char msg[32] = "";
			sprintf(msg, "%x: %l\r\n>", messageArray[filter].id, messageArray[filter].value);
			DebugPrint(msg);

		} else if (cmdbuff[0] == *(uint8_t *)"f") {
			//Command f can set a CAN filter and
			int filter = atoi((char *)&cmdbuff[1]);
			uint16_t id = atoi((char *)&cmdbuff[3]);
			uint8_t bit = atoi((char *)&cmdbuff[8]);
			uint8_t size = atoi((char *)&cmdbuff[11]);
			messageArray[filter].id = id;
			messageArray[filter].bit = bit;
			messageArray[filter].value = -1;
			messageArray[filter].length = size;
			messageArray[filter].enabled = MSG_ENABLED;
			ConfigureCANFilters(messageArray, 16);
		}
		//Resets cmdbuff index
		cmdbuffind = 0;
  }
  /* USER CODE END StartProcessCommand */
}

/* USER CODE BEGIN Header_StartSendTelemetry */
/**
* @brief Function implementing the SendTelemetry thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSendTelemetry */
void StartSendTelemetry(void *argument)
{
  /* USER CODE BEGIN StartSendTelemetry */
  /* Infinite loop */
  for(;;)
  {
  	//Transmit at 10hz
    osDelay(100);
    //HAL_GPIO_TogglePin(LD1_GPIO_Port,LD1_Pin);
  	HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    sprintf(&uart_tx_buff, "%l,%l,%l,%l,%l,%l,%l,%l,%l,%l,%l,%l,%l,%l,%l,%l\r\n",
        		messageArray[0].value,
    				messageArray[1].value,
    				messageArray[2].value,
    				messageArray[3].value,
    				messageArray[4].value,
    				messageArray[5].value,
    				messageArray[6].value,
    				messageArray[7].value,
    				messageArray[8].value,
    				messageArray[9].value,
    				messageArray[10].value,
    				messageArray[11].value,
    				messageArray[12].value,
    				messageArray[13].value,
    				messageArray[14].value,
    				messageArray[15].value);
		HAL_UART_Transmit_IT(&huart2, (uint8_t *)uart_tx_buff, strlen(uart_tx_buff));
  }
  /* USER CODE END StartSendTelemetry */
}

/* USER CODE BEGIN Header_StartFeedWDG */
/**
* @brief Function implementing the FeedWDG thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartFeedWDG */
void StartFeedWDG(void *argument)
{
  /* USER CODE BEGIN StartFeedWDG */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
    uint8_t WD_Data[2];
    WD_Data[0] = 0x0;
    WD_Data[1] = WD_SETUP;
    //HAL_SPI_Transmit_IT(&hspi1, WD_Data, 2);
  }
  /* USER CODE END StartFeedWDG */
}

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
