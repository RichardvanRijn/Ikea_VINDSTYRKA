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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
struct __attribute__((packed)) Measurement_Data {					//Measurement values struct
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
enum Bus_State {Wait_Start, Wait_Stop, Read_Data, Read_Address, Reset};

enum Source {SCL, SDA};

char bufferH_[ReceiveSize];				//physical location of data being handled
void *BufferH;							//pointer to location of data being handled

char BufferR_[ReceiveSize];				//physical location of UART receiving data
void *BufferR;

bool SCL_Enable = false;
bool SDA_Enable = false;

volatile uint8_t Bit_Count = 9;
volatile uint8_t Data_Byte = 0;
volatile uint8_t rbuf_index = 1;

volatile uint8_t counter = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

enum Bus_State main_State = Wait_Start;

void Set_SCL_RisingNFalling(bool RisingNFalling) //set SCL interrupt to Rising edge when bool = true, set interrupt to falling edge when bool = false
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /*Configure GPIO pin : SCL_Pin_Pin */
  if (RisingNFalling)
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  else
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;

  GPIO_InitStruct.Pin = SCL_Pin_Pin;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(SCL_Pin_GPIO_Port, &GPIO_InitStruct);
}

void Set_SDA_RisingNFalling(bool RisingNFalling) //set SDA interrupt to Rising edge when bool = true, set interrupt to falling edge when bool = false
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /*Configure GPIO pin : SDA_Pin_Pin */
  if (RisingNFalling)
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  else
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;

  GPIO_InitStruct.Pin = SDA_Pin_Pin;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(SDA_Pin_GPIO_Port, &GPIO_InitStruct);
}

void Set_SCL_INT(bool Enable) //Enable or disable SCL external interrupt, required they share an interrupt flag
{
  /*Configure GPIO pin : SCL_Pin_Pin */
  SCL_Enable = Enable;
}

void Set_SDA_INT(bool Enable) //Enable or disable SCL external interrupt, required they share an interrupt flag
{
  /*Configure GPIO pin : SCL_Pin_Pin */
  SDA_Enable = Enable;
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
  Set_SDA_RisingNFalling(false); // set SDA external interrupt to Falling edge
  Set_SDA_INT(true);	// initiate SDA interrupt for bus state = wait_start
  Set_SCL_INT(false); 	// initiate SCL interrupt for bus state = wait_start
  BufferR = &BufferR_;	//bind pointer to physical location
  BufferH = &bufferH_;	//bind pointer to physical location
  InitCom();
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  void Transmit(void* bufferT, uint8_t Length_p, USART_TypeDef* USART_P) // transmit info over uart debug port
  {
	volatile static uint8_t i;

	for (i = 0; i < Length_p; i++)
	{
		while ( !(READ_BIT(USART_P->ISR, USART_ISR_TXE)) );
		USART_P->TDR = *((uint8_t*)(bufferT+i));
	}
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  while ( R_Pop(&BufferH)) // check if an message is available in the circular buffer
	  {
		  volatile uint8_t size = *(uint8_t *)BufferH; // get the first byte from the buffer this holds the size of the packet
		  //size = size;
		  //Transmit((uint8_t *)BufferH, size, USART2); // transmit the buffer over the debug uart port
		  /*if (size == 21)
		  {
			  struct Measurement_Data *Measurement_Data = (void *) BufferH;
			  Transmit(Measurement_Data, sizeof(struct Measurement_Data), USART2); // transmit the buffer over the debug uart port
//			  Transmit((uint8_t *)BufferH, size + 1, USART2);
		  }*/ // could make packets for all the data block but am to lazy
		  if (size == 9 || size == 12 || size == 21 || size == 3)
		  {
			  Transmit((uint8_t *)BufferH, size + 1, USART2);
		  }
	  }
	  if (main_State == Reset) // reset the bus state machine
	  {
		  Bit_Count = 9;
		  Data_Byte = 0;
		  rbuf_index = 1;
		  Set_SDA_RisingNFalling(false);
		  Set_SDA_INT(true);
		  Set_SCL_INT(false);
		  (*(uint8_t *)(BufferR)) = 0; //clear buffer packet size
		  main_State = Wait_Start;
	  }
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
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
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

  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  /**USART2 GPIO Configuration
  PA2   ------> USART2_TX
  PA3   ------> USART2_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2|LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART2, &USART_InitStruct);
  LL_USART_DisableIT_CTS(USART2);
  LL_USART_ConfigAsyncMode(USART2);
  LL_USART_Enable(USART2);
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
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SDA_Pin_Pin */
  GPIO_InitStruct.Pin = SDA_Pin_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SDA_Pin_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SCL_Pin_Pin */
  GPIO_InitStruct.Pin = SCL_Pin_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SCL_Pin_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void Bus_State_Handler(uint8_t source) // I2C bus state interupt handler. the state of the bus is changed acording to the state and the interupt source (SDA or SCL)
{

  switch (main_State) {
	  case Wait_Start: // wait for I2C start
		  if (source == SDA) // if the SDA gets a falling edge this is the start of the I2C start
		  {
			  if(HAL_GPIO_ReadPin(SCL_Pin_GPIO_Port, SCL_Pin_Pin)/* && !HAL_GPIO_ReadPin(SDA_Pin_GPIO_Port, SDA_Pin_Pin) */)
			  {
				  Set_SCL_RisingNFalling(true);
				  Set_SDA_INT(false);
				  Set_SCL_INT(true);
				  main_State = Read_Address;
			  }
			  else
				main_State = Reset;
		  }
		  else if (source == SCL) // After the SDA went low, SCK should go low
		  {
			  if(!HAL_GPIO_ReadPin(SCL_Pin_GPIO_Port, SCL_Pin_Pin))  //&& !HAL_GPIO_ReadPin(SDA_Pin_GPIO_Port, SDA_Pin_Pin) // (do not check if SDA pin is low or high, the start and the first bit are to close together for the microcontroller to check the correct state)
			  {
				  Set_SCL_RisingNFalling(true);
				  Set_SDA_INT(false);
				  Set_SCL_INT(true);
				  main_State = Read_Address; // start is received next is I2C address
			  }
			  else
				main_State = Reset;
		  }
		  else
			  main_State = Reset;
		  break;
	  case Read_Address: // address byte = 9 bit long 7 address, 1 read/wrtie, 1 ack/nack
		  if (source == SCL)
		  {
			  if (Bit_Count >= 3) // bit count is counting down from 9, when less than 3 are left the bits are no longer part of the address so not important to store
				  Data_Byte |= (HAL_GPIO_ReadPin(SDA_Pin_GPIO_Port, SDA_Pin_Pin) << (Bit_Count - 3));
		  	  Bit_Count--;
		  	  if (Bit_Count == 2) // check if the received address corresponds with the known sensor address of 0x69 or 105 in base 10
		  	  {
		  		  if (Data_Byte != 105)
		  			main_State = Reset;
		  		  break;
		  	  }
		  	  if (Bit_Count == 1) // check read / write, if bit is write ignore everything, reset and wait for the next message
		  	  {
				  if (!HAL_GPIO_ReadPin(SDA_Pin_GPIO_Port, SDA_Pin_Pin))
					  main_State = Reset;
				  break;
		  	  }
		  	  if (Bit_Count == 0) // ack / nack bit, check if the sensor responds a ack meaning the address was correctly received by the sensor
		  	  {
				  if (!HAL_GPIO_ReadPin(SDA_Pin_GPIO_Port, SDA_Pin_Pin)) // ack received go to read data
				  {
					  Data_Byte = 0;
				  	  Bit_Count = 9;
					  main_State = Read_Data;
				  }
				  else
					  main_State = Reset;
				  break;
		  	  }
		  }
		  else
			  main_State = Reset;
		  break;
	  case Read_Data: // Data byte = 9 bit long 8 data, 1 ack/nack
		  if (source == SCL)
		  {
			  if (Bit_Count >= 2) // bit count is counting down from 9, when less than 2 are left the bits are no longer part of the data so not important to store
				  Data_Byte |= (HAL_GPIO_ReadPin(SDA_Pin_GPIO_Port, SDA_Pin_Pin) << (Bit_Count - 2));
			  Bit_Count--;
			  if (Bit_Count == 1) // last data bit received store the received byte in the packet buffer
			  {
				  (*(uint8_t *)(BufferR))++; // add one to the packer buffer size
				  (*(uint8_t *)(BufferR + rbuf_index++)) = Data_Byte; //add data to the packet buffer and set the point one higher for the next byte
			  }
			  if (Bit_Count == 0) // ack/nack received check. ack = more data is to come, nack = end of packet stop receiving data and wait for stop
			  {
				  if (HAL_GPIO_ReadPin(SDA_Pin_GPIO_Port, SDA_Pin_Pin))
				  {
					  main_State = Wait_Stop; // data packet received wait for stop
				  }
				  else
				  {
					  Data_Byte = 0;
				  	  Bit_Count = 9;
					  main_State = Read_Data; // reset counters and get next byte
				  }
				  break;
			  }
		  }
		  else
			  main_State = Reset;
		  break;
	  case Wait_Stop: // wait for stop first scl goes high than SDA goes high
		  if (source == SCL)
		  {
			  if(/*HAL_GPIO_ReadPin(SCL_Pin_GPIO_Port, SCL_Pin_Pin) &&*/ !HAL_GPIO_ReadPin(SDA_Pin_GPIO_Port, SDA_Pin_Pin)) // SCL went high
			  {
				  Set_SDA_RisingNFalling(true);
				  Set_SDA_INT(true);
				  Set_SCL_INT(false);
				  R_Push(&BufferR);

				  main_State = Reset;
			  }
			  else
				main_State = Reset;
		  }
		  else if (source == SDA)
		  {
			  if(HAL_GPIO_ReadPin(SCL_Pin_GPIO_Port, SCL_Pin_Pin) /*&& HAL_GPIO_ReadPin(SDA_Pin_GPIO_Port, SDA_Pin_Pin) */) // SDA went high
			  {
				  R_Push(&BufferR);
				  main_State = Reset;
			  }
			  else
				  main_State = Reset;
		  }
		  else
			  main_State = Reset;
		  break;
	  default:
		  main_State = Reset;
		  break;
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) // external interupt handler check if the interupt is generated from SDA or SCL (both share an interupt flag)
{

	if(GPIO_Pin == SDA_Pin_Pin && SDA_Enable == true) // If The INT Source Is EXTI Line9 (A9 Pin)
		Bus_State_Handler(SDA);
	else if(GPIO_Pin == SCL_Pin_Pin && SCL_Enable == true) // If The INT Source Is EXTI Line9 (A9 Pin)
		Bus_State_Handler(SCL);
	counter = 0;
}

void Timer_Reset(void) //Timer if there is more than 5mS no interupt and the bus state is not Wait_Start asume something whent wrong and reset the bus state to Wait_Start
{

	counter++;
	if (counter > 5 && main_State != Wait_Start)
	{
	  Bit_Count = 9;
	  Data_Byte = 0;
	  Set_SDA_RisingNFalling(false);
	  Set_SDA_INT(true);
	  Set_SCL_INT(false);
	  main_State = Wait_Start;
	}
}

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
