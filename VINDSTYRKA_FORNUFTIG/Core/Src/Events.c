/*
 * Events.c
 *
 *  Created on: Oct 20, 2024
 *      Author: Richard
 */

#include "Events.h"
#include "I2C_sniffer.h"
#include "main.h"

struct _Events_Set Events_Set;
struct _Event Event;
struct _events events;

struct _Tvoc Tvox;
struct _NOx NOx;

enum Device_Mode Device_Mode_State;

uint8_t speed_switch;

uint16_t PM1_Value;
uint16_t PM25_Value;
uint16_t Tvoc_Value;

void _Init_Events()
{
	events.Event = &Event;
	events.Events_Set = &Events_Set;
}

void _EventSet_PcRx(void)      { events.Events_Set->PcRx_set = 1; }
void _EventClear_PcRx(void)    { events.Event->PcRx = 0; }

void _EventSet_PcTx(void)      { events.Events_Set->PcTx_set = 1; }
void _EventClear_PcTx(void)    { events.Event->PcTx = 0; }

void _EventSet_I2C_Rx(void)      { events.Events_Set->I2C_Rx_set = 1; }
void _EventClear_I2C_Rx(void)    { events.Event->I2C_Rx = 0; }

void _EventSet_I2C_Reset(void)      { events.Events_Set->I2C_Reset_set = 1; }
void _EventClear_I2C_Reset(void)    { events.Event->I2C_Reset = 0; }

void _EventSet_Mode_Switch_Update(void)      { events.Events_Set->Mode_Switch_Update_set = 1; }
void _EventClear_Mode_Switch_Update(void)    { events.Event->Mode_Switch_Update = 0; }

void _EventSet_Speed_Switch_Update(void)      { events.Events_Set->Speed_Switch_Update_set = 1; }
void _EventClear_Speed_Switch_Update(void)    { events.Event->Speed_Switch_Update = 0; }

void _EventSet_Output_Update(void)      { events.Events_Set->Output_Update_set = 1; }
void _EventClear_Output_Update(void)    { events.Event->Output_Update = 0; }

enum Device_Mode Get_Device_Mode_State(void)
{
	return Device_Mode_State;
}

void set_tvoc(uint16_t Value)
{
//Tvoc_Value = Value * 2;

	Tvoc_Value = (((Value << 8) | (Value >> 8)) & 0xFFFF) / 10;
}

void set_PM25(uint16_t Value)
{
	PM25_Value = (((Value << 8) | (Value >> 8)) & 0xFFFF) / 10;
}

void set_PM1(uint16_t Value)
{
	PM1_Value = (((Value << 8) | (Value >> 8)) & 0xFFFF) / 10;
}

void Init_Values(void)
{
	Tvox.Tvoc_Off = 0;
	Tvox.Tvoc_1 = 150;
	Tvox.Tvoc_2 = 250;
	Tvox.Tvoc_3 = 400;

	NOx.NOx_Off = 0;
	NOx.NOx_1 = 20;
	NOx.NOx_2 = 150;
	NOx.NOx_3 = 300;
}

uint8_t get_input(void)
{
	uint8_t return_value = 0;
	return_value += !HAL_GPIO_ReadPin(SW_POS_IN_1_GPIO_Port, SW_POS_IN_1_Pin);
	return_value += !HAL_GPIO_ReadPin(SW_POS_IN_2_GPIO_Port, SW_POS_IN_2_Pin) << 1;
	return_value += !HAL_GPIO_ReadPin(SW_POS_IN_3_GPIO_Port, SW_POS_IN_3_Pin) << 2;
	return_value += !HAL_GPIO_ReadPin(SW_POS_IN_4_GPIO_Port, SW_POS_IN_4_Pin) << 3;
	return return_value;
}

void set_output(uint8_t value)
{
	HAL_GPIO_WritePin(SW_POS_OUT_1_GPIO_Port, SW_POS_OUT_1_Pin, !(value == 8));
	HAL_GPIO_WritePin(SW_POS_OUT_2_GPIO_Port, SW_POS_OUT_2_Pin, !(value == 4));
	HAL_GPIO_WritePin(SW_POS_OUT_3_GPIO_Port, SW_POS_OUT_3_Pin, !(value == 2));
	HAL_GPIO_WritePin(SW_POS_OUT_4_GPIO_Port, SW_POS_OUT_4_Pin, !(value == 1));
}

void Mode_Switch_Event(void)
{
	uint8_t switch_Posistion = HAL_GPIO_ReadPin(Mode_Switch_1_GPIO_Port, Mode_Switch_1_Pin);
	switch_Posistion += (HAL_GPIO_ReadPin(Mode_Switch_2_GPIO_Port, Mode_Switch_2_Pin) << 1);
	if (switch_Posistion == 0b00) //auto mode
	{
		Device_Mode_State = Auto;
	}
	else if (switch_Posistion == 0b10) // auto + manual minimum
	{
		Device_Mode_State = Manual_Auto;
	}
	else /* if (switch_Posistion == 0b01) // manual */
	{
		Device_Mode_State = Manual;
		_EventSet_Speed_Switch_Update();
	}
}

void Speed_Switch_Event(void)
{
	speed_switch = get_input();
	_EventSet_Output_Update();
}

uint8_t Cal_Auto_Value(void)
{
	uint8_t Tvoc_Out = 1;
	uint8_t NOx_Out = 1;
	uint8_t NOx_Value = 0;

	if (Tvoc_Value > Tvox.Tvoc_1)
		Tvoc_Out = Tvoc_Out << 1;
	if (Tvoc_Value > Tvox.Tvoc_2)
		Tvoc_Out = Tvoc_Out << 1;
	if (Tvoc_Value > Tvox.Tvoc_3)
		Tvoc_Out = Tvoc_Out << 1;

	NOx_Value = PM1_Value + PM25_Value;

	if (NOx_Value > NOx.NOx_1)
		NOx_Out = NOx_Out << 1;
	if (NOx_Value > NOx.NOx_2)
		NOx_Out = NOx_Out << 1;
	if (NOx_Value > NOx.NOx_3)
		NOx_Out = NOx_Out << 1;

	return (NOx_Out > Tvoc_Out) ? NOx_Out : Tvoc_Out;
}

void Update_Output(void)
{
	uint8_t value_out = 1;
	switch (Device_Mode_State) {

		case Manual: //
			value_out = speed_switch;
			HAL_GPIO_WritePin(Mode_Led_GPIO_Port, Mode_Led_Pin, 1);
			break;
		case Auto: //
				value_out = Cal_Auto_Value();
				HAL_GPIO_WritePin(Mode_Led_GPIO_Port, Mode_Led_Pin, !(value_out > 1));
			break;
		case Manual_Auto: //
			uint8_t manual_out = speed_switch;
			uint8_t auto_out = Cal_Auto_Value();

			value_out = (manual_out > auto_out) ? manual_out : auto_out;
			HAL_GPIO_WritePin(Mode_Led_GPIO_Port, Mode_Led_Pin, !(auto_out > manual_out));

			break;
		default:
			break;
	}
	set_output(value_out);
}

void Handle_Events(void)
{
	if ( events.Event->I2C_Rx != 0 ) //TCC RX from PC
	{

	  _EventClear_I2C_Rx();
	}

	if ( events.Event->I2C_Reset != 0 ) //TCC RX from PC
	{
	  I2C_Reset();
	  _EventClear_I2C_Reset();
	}

	if ( events.Event->Mode_Switch_Update != 0 ) //TCC RX from PC
	{
		Mode_Switch_Event();
		_EventClear_Mode_Switch_Update();
	}

	if ( events.Event->Speed_Switch_Update != 0 ) //TCC RX from PC
	{
		Speed_Switch_Event();
		_EventClear_Speed_Switch_Update();
	}

	if ( events.Event->Output_Update != 0 ) //TCC RX from PC
	{
		Update_Output();
		_EventClear_Output_Update();

	}

	__disable_irq();
	void* tmp = events.Event;
	events.Event = events.Events_Set;
	events.Events_Set = tmp;
	__enable_irq();
}


