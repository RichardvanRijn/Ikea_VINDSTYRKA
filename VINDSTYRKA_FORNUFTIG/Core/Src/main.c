/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdbool.h"
#include "Com.h"


//#include "I2C_Sniffer.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
struct __attribute__((packed)) Measurement_Data { //
	uint8_t Size;
	uint16_t PM10;
	uint8_t PM10_CS;
	uint16_t PM25;
	uint8_t PM25_CS;
	uint16_t PM40;
	uint8_t PM40_CS;
	uint16_t PM100;
	uint8_t PM100_CS;
	uint16_t HUM;
	uint8_t HUM_CS;
	uint16_t TEMP;
	uint8_t TEMP_CS;
	uint16_t VOC;
	uint8_t VOC_CS;
};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t I2C_Reset_Counter = 0;

char bufferH_[ReceiveSize];				//physical location of data being handled
void *BufferH;							//pointer to location of data being handled


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void Transmit(void* bufferT, uint8_t Length_p, USART_TypeDef* USART_P) // transmit info over uart debug port
  {
		volatile static uint8_t i;

		for (i = 0; i < Length_p; i++)
		{
			while ( !(READ_BIT(USART_P->SR, USART_SR_TXE)));
			USART_P->DR = *((uint8_t*)(bufferT+i));
		}

  }

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{

	if( GPIO_Pin == SCL_Pin_Pin || GPIO_Pin == SDA_Pin_Pin )
	{
		Check_Interupt(GPIO_Pin);
		I2C_Reset_Counter = 0;
	}

	if ( GPIO_Pin == Mode_Switch_1_Pin || GPIO_Pin == Mode_Switch_2_Pin )
	{
		_EventSet_Mode_Switch_Update();
	}
	if ( GPIO_Pin == SW_POS_IN_1_Pin || GPIO_Pin == SW_POS_IN_2_Pin	|| GPIO_Pin == SW_POS_IN_3_Pin || GPIO_Pin == SW_POS_IN_4_Pin )
	{
		_EventSet_Speed_Switch_Update();
	}
}

void HAL_IncTick(void)
{
  uwTick += uwTickFreq;
	I2C_Reset_Counter++;
	if ( I2C_Reset_Counter > I2C_Reset_Count )
	{
		I2C_Reset_Counter = 0;
		_EventSet_I2C_Reset();
		//I2C_Timer_Reset();
	}
}

void Enable_Interupts(void)
{
	  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

	  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

	  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

	  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

	  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
	  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  _Init_Events();
  Init_Values();
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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  I2C_Init();
  InitCom();
  BufferH = &bufferH_;	//bind pointer to physical location
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  _EventSet_Mode_Switch_Update();
  _EventSet_Speed_Switch_Update();
  //Enable_Interupts();
  while (1)
  {
	  while ( R_Pop(&BufferH) ) // check if an message is available in the circular buffer
	  {
		  volatile uint8_t size = *(uint8_t *)BufferH; // get the first byte from the buffer this holds the size of the packet
		  //size = size;
		  //Transmit((uint8_t *)BufferH, size, USART2); // transmit the buffer over the debug uart port
		  if (size == 21)
		  {
			  struct Measurement_Data *Measurement_Data = (void *) BufferH;
			  set_tvoc(Measurement_Data->VOC);
			  set_PM25(Measurement_Data->PM25);
			  set_PM1(Measurement_Data->PM10);
			  if ( Get_Device_Mode_State() != Manual)
				  _EventSet_Output_Update();
			  Transmit(Measurement_Data, sizeof(struct Measurement_Data), USART1); // transmit the buffer over the debug uart port
//			  Transmit((uint8_t *)BufferH, size + 1, USART2);
//			  Transmit((uint8_t *)(Measurement_Data->VOC), sizeof(Measurement_Data->VOC), USART1);
//			  Transmit((uint8_t *)(Measurement_Data->PM25), sizeof(Measurement_Data->PM25), USART1);
//			  Transmit((uint8_t *)(Measurement_Data->PM10), sizeof(Measurement_Data->PM10), USART1);
		  } // could make packets for all the data block but am to lazy
		 /* if (size == 9 || size == 12 || size == 21 || size == 3)
		  {
			  Transmit((uint8_t *)BufferH, size + 1, USART1);
		  }*/
	  }
	  Handle_Events();

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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

  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
  /**USART1 GPIO Configuration
  PA9   ------> USART1_TX
  PA10   ------> USART1_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_FLOATING;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART1);
  LL_USART_Enable(USART1);
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SW_POS_OUT_1_Pin|SW_POS_OUT_2_Pin|SW_POS_OUT_3_Pin|SW_POS_OUT_4_Pin
                          |Mode_Led_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : SW_POS_IN_1_Pin SW_POS_IN_2_Pin SW_POS_IN_3_Pin SW_POS_IN_4_Pin */
  GPIO_InitStruct.Pin = SW_POS_IN_1_Pin|SW_POS_IN_2_Pin|SW_POS_IN_3_Pin|SW_POS_IN_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SCL_Pin_Pin */
  GPIO_InitStruct.Pin = SCL_Pin_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SCL_Pin_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SDA_Pin_Pin */
  GPIO_InitStruct.Pin = SDA_Pin_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SDA_Pin_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SW_POS_OUT_1_Pin SW_POS_OUT_2_Pin SW_POS_OUT_3_Pin SW_POS_OUT_4_Pin */
  GPIO_InitStruct.Pin = SW_POS_OUT_1_Pin|SW_POS_OUT_2_Pin|SW_POS_OUT_3_Pin|SW_POS_OUT_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : Mode_Led_Pin */
  GPIO_InitStruct.Pin = Mode_Led_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(Mode_Led_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Mode_Switch_1_Pin Mode_Switch_2_Pin */
  GPIO_InitStruct.Pin = Mode_Switch_1_Pin|Mode_Switch_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
