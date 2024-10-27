/*
 * com.h
 *
 *  Created on: 10 Dec 2019
 *      Author: RvRijn
 */

#ifndef COM_H_
#define COM_H_

#include "stdbool.h"

#define ReceiveCommandSize 6
#define ReceiveSize 30

void InitCom();

bool R_bufferFull();

bool R_bufferEmpty();

bool R_Push(void** Buffer_p);

bool R_Pop(void** Command_p);

//void Transmit(void *bufferT, uint8_t Length_p, USART_TypeDef* USART_P);

#endif /* COM_H_ */
