/*
 * Events.h
 *
 *  Created on: Oct 20, 2024
 *      Author: Richard
 */

#ifndef INC_EVENTS_H_
#define INC_EVENTS_H_

#include "stdint.h"

enum Device_Mode {Manual, Auto, Manual_Auto};

struct _Events_Set
{
    uint8_t PcRx_set:1;
    uint8_t PcTx_set:1;
    uint8_t I2C_Rx_set:1;
    uint8_t I2C_Reset_set:1;
    uint8_t Mode_Switch_Update_set:1;
    uint8_t Speed_Switch_Update_set:1;
    uint8_t Output_Update_set:1;
};


struct _Event
{
    uint8_t PcRx:1;
    uint8_t PcTx:1;
    uint8_t I2C_Rx:1;
    uint8_t I2C_Reset:1;
    uint8_t Mode_Switch_Update:1;
    uint8_t Speed_Switch_Update:1;
    uint8_t Output_Update:1;
};

struct _events
{
	struct _Events_Set* Events_Set;
	struct _Event* Event;
};

struct _Tvoc
{
	uint16_t Tvoc_Off;
	uint16_t Tvoc_1;
	uint16_t Tvoc_2;
	uint16_t Tvoc_3;
};

struct _NOx
{
	uint16_t NOx_Off;
	uint16_t NOx_1;
	uint16_t NOx_2;
	uint16_t NOx_3;
};


/*

#define _EventSet_PcRx()      { events.Events_Set->PcRx_set = 1; }
#define _EventClear_PcRx()    { events.Event->PcRx = 0; }

#define _EventSet_PcTx()      { events.Events_Set->PcTx_set = 1; }
#define _EventClear_PcTx()    { events.Event->PcTx = 0; }

#define _EventSet_I2C_Rx()      { events.Events_Set->I2C_Rx_set = 1; }
#define _EventClear_I2C_Rx()    { events.Event->I2C_Rx = 0; }

#define _EventSet_I2C_Reset()      { events.Events_Set->I2C_Reset_set = 1; }
#define _EventClear_I2C_Reset()    { events.Event->I2C_Reset = 0; }

#define _EventSet_Switch_Update()      { events.Events_Set->Switch_Update_set = 1; }
#define _EventClear_Switch_Update()    { events.Event->Switch_Update = 0; }
*/
enum Device_Mode Get_Device_Mode_State(void);

void Handle_Events(void);

void _Init_Events(void);

void _EventSet_PcRx(void);
void _EventClear_PcRx(void);

void _EventSet_PcTx(void);
void _EventClear_PcTx(void);

void _EventSet_I2C_Rx(void);
void _EventClear_I2C_Rx(void);

void _EventSet_I2C_Reset(void);
void _EventClear_I2C_Reset(void);

void _EventSet_Mode_Switch_Update(void);
void _EventClear_Mode_Switch_Update(void);

void _EventSet_Speed_Switch_Update(void);
void _EventClear_Speed_Switch_Update(void);

void _EventSet_Output_Update(void);
void _EventClear_Output_Update(void);

void set_tvoc (uint16_t Value);
void set_PM25 (uint16_t Value);
void set_PM1 (uint16_t Value);

void Init_Values(void);

#endif /* INC_EVENTS_H_ */
