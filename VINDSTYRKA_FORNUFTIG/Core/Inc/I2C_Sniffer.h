/*
 * I2C_Sniffer.h
 *
 *  Created on: Oct 20, 2024
 *      Author: Richard
 */

#ifndef INC_I2C_SNIFFER_H_
#define INC_I2C_SNIFFER_H_

#include "stdint.h"
#include "stdbool.h"
#include "main.h"

enum Bus_State {Wait_Start, Wait_Stop, Read_Data, Read_Address, Reset};

enum Source {SCL, SDA};

#define I2C_Reset_Count 5

void I2C_Init(void);

//void Check_Reset(void);

//void I2C_Timer_Reset(void);

void I2C_Reset(void);

void Check_Interupt(uint16_t GPIO_Pin);

#endif /* INC_I2C_SNIFFER_H_ */
