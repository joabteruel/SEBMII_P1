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

#define REG_RTCSEC 0x00  // Register Address: Time Second
#define REG_RTCMIN 0x01  // Register Address: Time Minute
#define REG_RTCHOUR 0x02  // Register Address: Time Hour
#define REG_RTCWKDAY 0x03  // Register Address: Date Day of Week
#define REG_RTCDATE 0x04  // Register Address: Date Day
#define REG_RTCMTH 0x05  // Register Address: Date Month
#define REG_RTCYEAR 0x06  // Register Address: Date Year

/*Time related masks to decode from RTC*/
#define ASCII_NUMBER_MASK 0x30	//Mask used to add or subtract to an HEX number value to convert it from/to ASCII value
#define ASCII_LETTER_MASK 0x37	//Mask used to add or subtract to an HEX letter value to convert it from/to ASCII value
#define BCD_H 0xF0				//Mask used to make a bitwise operation with the high part of the BCD data
#define BCD_L 0x0F				//Mask used to make a bitwise operation with the low part of the BCD data
#define SECONDS_REG_SIZE 0x7F	//Used to set reading boundaries according to the seconds register size
#define MINUTES_REG_SIZE 0x7F	//Used to set reading boundaries according to the minutes register size
#define HOURS_REG_SIZE 0x3F		//Used to set reading boundaries according to the hours register size
#define DAY_REG_SIZE 0x2F		//Used to set reading boundaries according to the days register size
#define MONTH_REG_SIZE 0x1F		//Used to set reading boundaries according to the month register size
#define YEAR_REG_SIZE 0xFF		//Used to set reading boundaries according to the year register size
#define TIME_FORMAT_SIZE 0x40

typedef enum{
	FORMAT_24H = 0x00,
	FORMAT_12H = 0x01,
	FORMAT_AM = 0x00,
	FORMAT_PM = 0x01
}HourFormat_t;

typedef enum{
	OSCILLATOR_OFF = 0x00,
	OSCILLATOR_ON = 0x80
}StartOscillatorType;

/*Event Bits*/
#define EVENT_TIME_SET (1<<0)
#define EVENT_TIME_ERR (1<<1)

/*ASCII Symbols*/
#define ESC_KEY 0x1B //27

/*Typo definitions*/
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
	uint8_t timeformat;
	uint8_t ampm;
}ascii_time_t;

uint16_t asciiToHex(uint8_t *string);
uint8_t asciitoDec(uint8_t *string);
void os_init();

void menu0_Task(void *parameter);
void menu3_Task(void *parameter);
void timedateLCD_task(void* parameters);
void getTime_task(void *parameter);
void echo_Task(void * uart_module);
void osNotDeadLED(void * params);
void memread_task(void * uart_module);
void setTime_task(void * uart_module);
void setDate_task(void * uart_module);
void hourFormat_task(void * uart_module);
void timeTerminal_task(void * uart_module);
void dateTerminal_task(void * uart_module);


/*Constant menus definitions*/

static const uint8_t main_menuTxt[] =
		"\e[ ? 25 l"
		"\033[2J"
		"\033[2;2H\r"
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

static const uint8_t echo_menuTxt[] =
		"\033[2J"
		"\033[2;2H\r"
		"----------------------------\r\n"
		"|    Eco en display LCD    |\r\n"
		"----------------------------\r\n\n";

static const uint8_t memread_Txt[] =
		"\033[2J"
		"\033[2;2H\r"
		"----------------------------\r\n"
		"|    Lectura de Memoria    |\r\n"
		"----------------------------\r\n\n";

static const uint8_t setTime_Txt[] =
		"\033[2J"
		"\033[2;2H\r"
		"----------------------------\r\n"
		"|      Actualizar Hora     |\r\n"
		"----------------------------\r\n\n";

static const uint8_t setDate_Txt[] =
		"\033[2J"
		"\033[2;2H\r"
		"----------------------------\r\n"
		"|     Actualizar Fecha     |\r\n"
		"----------------------------\r\n\n";

static const uint8_t hourFormat_Txt[] =
		"\033[2J"
		"\033[2;2H\r"
		"------------------------------\r\n"
		"| Cambiar el formato de hora |\r\n"
		"------------------------------\r\n\n";

static const uint8_t terminalTime_Txt[] =
		"\033[2J"
		"\033[2;2H\r"
		"------------------------------\r\n"
		"|         Hora Actual        |\r\n"
		"------------------------------\r\n\n";

static const uint8_t terminalDate_Txt[] =
		"\033[2J"
		"\033[2;2H\r"
		"------------------------------\r\n"
		"|            Fecha           |\r\n"
		"------------------------------\r\n\n";

static const uint8_t errorMes_Txt[] =
		"\033[2J"
		"\033[5;10H\r"
		"***************************************************************\r\n"
		"*  Error: El recurso esta siendo utilizado por otra terminal  *\r\n"
		"***************************************************************\r\n\n";

#endif /* USERTASK_H_ */
