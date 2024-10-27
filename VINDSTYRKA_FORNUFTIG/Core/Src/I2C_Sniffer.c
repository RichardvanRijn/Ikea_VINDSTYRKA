/*
 * I2C_Sniffer.c
 *
 *  Created on: Oct 20, 2024
 *      Author: Richard
 */

#include "I2C_Sniffer.h"
//#include "stdint.h"
//#include "stdbool.h"
#include "Com.h"

#include "Events.h"

volatile uint8_t Bit_Count = 9;
volatile uint8_t Data_Byte = 0;
volatile uint8_t rbuf_index = 1;

volatile uint8_t counter = 0;

char BufferR_[ReceiveSize];				//physical location of UART receiving data
void *BufferR;

bool SCL_Enable = false;
bool SDA_Enable = false;


enum Bus_State main_State;

void Set_SCL_RisingNFalling(bool RisingNFalling) //set SCL interrupt to Rising edge when bool = true, set interrupt to falling edge when bool = false
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /*Configure GPIO pin : SCL_Pin_Pin */
  if (RisingNFalling)
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  else
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;

  GPIO_InitStruct.Pin = SCL_Pin_Pin;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
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
  GPIO_InitStruct.Pull = GPIO_NOPULL;
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

void I2C_Init(void)
{
	  main_State = Wait_Start;
	  BufferR = &BufferR_;	//bind pointer to physical location
	  Set_SDA_RisingNFalling(false); // set SDA external interrupt to Falling edge
	  Set_SDA_INT(true);	// initiate SDA interrupt for bus state = wait_start
	  Set_SCL_INT(false); 	// initiate SCL interrupt for bus state = wait_start
}

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
  if ( main_State == Reset)
  	  _EventSet_I2C_Reset();
}

void Check_Interupt(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == SDA_Pin_Pin && SDA_Enable == true) // If The INT Source Is EXTI Line9 (A9 Pin)
		Bus_State_Handler(SDA);
	else if(GPIO_Pin == SCL_Pin_Pin && SCL_Enable == true) // If The INT Source Is EXTI Line9 (A9 Pin)
		Bus_State_Handler(SCL);
}

void I2C_Reset(void)
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
/*
void Check_Reset(void)
{
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
}

void I2C_Timer_Reset(void) //Timer if there is more than 5mS no interupt and the bus state is not Wait_Start asume something whent wrong and reset the bus state to Wait_Start
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
*/
