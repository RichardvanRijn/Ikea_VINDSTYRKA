/*
 * com.c
 *
 *  Created on: 10 Dec 2019
 *      Author: RvRijn
 */

#include "Com.h"
#include "stdbool.h"
#include "stdint.h"

uint8_t Pushindex = 0;
uint8_t Popindex = ReceiveCommandSize - 1;

char CommandInternal_[ReceiveSize];		//physical location of internal command data
void *CommandInternal;					//pointer to receiving data

char Buffer_packet_Location[ReceiveCommandSize][ReceiveSize];	//physical location of buffered packets
void*	commandStack[ReceiveCommandSize];						//pointer to buffered packets

void InitCom()
{
	for (uint8_t I = 0; I < ReceiveCommandSize;I++)
	{
	  commandStack[I] = &Buffer_packet_Location[I];				//bind pointers to physical location
	}
}

bool R_bufferFull()
{
	return (Pushindex == Popindex);	//buffer is full when push and pop index are identical
}

bool R_bufferEmpty()
{
	return (((Popindex + 1) % ReceiveCommandSize) == Pushindex); //buffer is empty when popindex is 1 behind push index
}

bool R_Push(void** Buffer_p)
{
	//uint8_t temp;
	if (R_bufferFull())										// If the buffer is full the received packet in not placed in the buffer and wil be overwriten on the next receive
		return false;

	void *temp = *Buffer_p;									//swapping pointers to array's
	*Buffer_p = commandStack[Pushindex];
	commandStack[Pushindex] = temp;

	Pushindex = ((Pushindex + 1) % ReceiveCommandSize);

	return true;
}

//#include "stm32f3xx_ll_cortex.h"

bool R_Pop(void** Command_p)
{

	//uint8_t temp;
	if(R_bufferEmpty())
		return false;

	Popindex = ((Popindex + 1) % ReceiveCommandSize);

	void* temp = *Command_p;
	*Command_p = commandStack[Popindex];
	commandStack[Popindex] = temp;

	return true;
}

