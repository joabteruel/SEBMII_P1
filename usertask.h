/*
 * usertask.h
 *
 *  Created on: Mar 13, 2018
 *      Author: Joab
 */

#ifndef USERTASK_H_
#define USERTASK_H_

#include "FreeRTOS.h"
#include "task.h"
#include "fsl_uart_freertos.h"
#include "queue.h"
#include "semphr.h"

#include "UART.h"
#include "I2C.h"
#include "SPI.h"
#include "LCDNokia5110.h"

/*I2C Devices address*/
#define RTC_DEVICE_ADD 0x6F
#define MEM_DEVICE_ADD 0xA0 >> 1

/*Time related masks to decode from RTC*/
#define ASCII_NUMBER_MASK 0x30	//Mask used to add or subtract to an HEX number value to convert it from/to ASCII value
#define ASCII_LETTER_MASK 0x37	//Mask used to add or subtract to an HEX letter value to convert it from/to ASCII value
#define BCD_H 0xF0				//Mask used to make a bitwise operation with the high part of the BCD data
#define BCD_L 0x0F				//Mask used to make a bitwise operation with the low part of the BCD data
#define SECONDS_REG_SIZE 0x7F	//Used to set reading boundaries according to the seconds register size
#define MINUTES_REG_SIZE 0x7F	//Used to set reading boundaries according to the minutes register size
#define HOURS_REG_SIZE 0x1F		//Used to set reading boundaries according to the hours register size
#define DAY_REG_SIZE 0x2F		//Used to set reading boundaries according to the days register size
#define MONTH_REG_SIZE 0x1F		//Used to set reading boundaries according to the month register size
#define YEAR_REG_SIZE 0xFF		//Used to set reading boundaries according to the year register size

/*Event Bits*/
#define EVENT_TIME_SET (1<<0)
#define EVENT_TIME_ERR (1<<1)

/*ASCII Symbols*/
#define ESC_KEY 0x1B //27

typedef struct
{
	uint8_t seconds_l;
	uint8_t seconds_h;
	uint8_t minutes_l;
	uint8_t minutes_h;
	uint8_t hours_l;
	uint8_t hours_h;
	uint8_t day_l;
	uint8_t day_h;
	uint8_t month_l;
	uint8_t month_h;
	uint8_t year_l;
	uint8_t year_h;
}ascii_time_t;

static const uint8_t main_menuTxt[] =
			"\e[ ? 25 l\033[2J\033[2;2H\r"
			"----------------------------\r\n"
			"|     ITESO -- SEMBII      |\r\n"
			"|       Practica 1         |\r\n"
			"----------------------------\r\n\n"
			"Opciones:\r\n\n"
			"  1)  Leer Memoria I2C\r\n"
			"  2)  Escribir memoria I2C\r\n"
			"  3)  Establecer Hora\r\n"
			"  4)  Establecer Fecha\r\n"
			"  5)  Formato de hora\r\n"
			"  6)  Leer hora\r\n"
			"  7)  Leer fecha\r\n"
			"  8)  Comunicacion con terminal 2\r\n"
			"  9)  Eco en LCD\r\n";

void os_init();
void menu0_Task(void *parameter);
void timedateLCD_task(void* parameters);
void getTime_task(void *parameter);
void echo_Task(void *parameter);
void osNotDeadLED(void * params);






#endif /* USERTASK_H_ */
