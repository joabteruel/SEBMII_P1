/*
 * usertask.c
 *
 *  Created on: Mar 13, 2018
 *      Author: Joab
 */

#include "usertask.h"

TaskHandle_t menuTask_handle;
TaskHandle_t timedateLCD_handle;
TaskHandle_t getTime_handle;
TaskHandle_t setTime_handle;
TaskHandle_t setDate_handle;
TaskHandle_t hourFormat_handle;
TaskHandle_t echoTask_handle;
TaskHandle_t timeTerminal_handle;
TaskHandle_t dateTerminal_handle;

SemaphoreHandle_t spibus_mutex;
SemaphoreHandle_t i2cbus_mutex;

EventGroupHandle_t getTime_eventB;
EventGroupHandle_t timeTerminal_eventB;

QueueHandle_t g_time_queue;

uint16_t asciiToHex(uint8_t *string)
{
	volatile uint16_t hexAddress;
	hexAddress = 0x0000;
	while (*string)
	{
		hexAddress = hexAddress << 4;
		if (*string >= 'A' && *string <= 'F')
		{
			hexAddress |= *string - ASCII_LETTER_MASK;
			*string++;
		}
		else
		{
			hexAddress |= *string - ASCII_NUMBER_MASK;
			*string++;
		}
	}
	return hexAddress;
}

uint8_t asciitoDec(uint8_t *string)
{
	uint8_t decNum = 0;
	while(*string)
	{
		decNum = decNum*10 + (*string - ASCII_NUMBER_MASK);
		*string++;
	}
	return decNum;
}

void os_init()
{
	spibus_mutex = xSemaphoreCreateMutex();
	i2cbus_mutex = xSemaphoreCreateMutex();
	getTime_eventB = xEventGroupCreate();
	timeTerminal_eventB = xEventGroupCreate();

	g_time_queue = xQueueCreate(1, sizeof(ascii_time_t*));
}

void menu0_Task(void *parameter)
{
	uint8_t recvBuffer;
	menuTask_handle = xTaskGetCurrentTaskHandle();

	while (1)
	{
		UART_putString(UART_0, (uint8_t*) main_menuTxt);
		recvBuffer = UART_Echo(UART_0);
		switch (recvBuffer - ASCII_NUMBER_MASK)
		{
		case 1:
			xTaskCreate(memread_task, "memread_task", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES-2, NULL);
			vTaskSuspend(NULL);
			break;
		case 3:
			xTaskCreate(setTime_task, "setTime_task", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES-2, NULL);
			vTaskSuspend(NULL);
			break;
		case 4:
			xTaskCreate(setDate_task, "setDate_task", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES-2, NULL);
			vTaskSuspend(NULL);
			break;
		case 5:
			xTaskCreate(hourFormat_task, "hourFormat_task", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES-2, NULL);
			vTaskSuspend(NULL);
			break;
		case 6:
			xTaskCreate(timeTerminal_task, "timeTerminal_task", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES-2, NULL);
			vTaskSuspend(NULL);
			break;
		case 7:
			xTaskCreate(dateTerminal_task, "dateTerminal_task", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES-2, NULL);
			vTaskSuspend(NULL);
			break;
		case 9:
			vTaskSuspend(getTime_handle);
			vTaskSuspend(timedateLCD_handle);
			xTaskCreate(echo_Task, "echo_task", configMINIMAL_STACK_SIZE, NULL,
					configMAX_PRIORITIES - 2, NULL);
			vTaskSuspend(NULL);
			break;
		default:
			UART_putString(UART_0, (uint8_t*)
					"\033[2J"
					"\033[5;10H"
					"Seleccion no valida");
			vTaskDelay(pdMS_TO_TICKS(1500));
		}
	}
}

void osNotDeadLED(void * params)
{
	while (1)
	{
		LED_BLUE_TOGGLE();
		vTaskDelay(pdMS_TO_TICKS(3000));
	}
}

void timedateLCD_task(void* parameters)
{
	timedateLCD_handle = xTaskGetCurrentTaskHandle();
	ascii_time_t *asciiDate;

	while (1)
	{
		xEventGroupWaitBits(getTime_eventB, (EVENT_TIME_SET|EVENT_TIME_ERR), pdFALSE, pdFALSE,
		portMAX_DELAY);
		EventBits_t event = xEventGroupGetBits(getTime_eventB);
		xEventGroupClearBits(getTime_eventB, EVENT_TIME_ERR|EVENT_TIME_SET);
		if(EVENT_TIME_ERR == (event&EVENT_TIME_ERR))
		{
			xSemaphoreTake(spibus_mutex, portMAX_DELAY);
			LCDNokia_clear();
			LCDNokia_gotoXY(0, 3);
			LCDNokia_sendString((uint8_t*)"I/O ERROR");
			xSemaphoreGive(spibus_mutex);
		}
		else
		{
			do
			{
				xQueueReceive(g_time_queue, &asciiDate, portMAX_DELAY);
			} while (0 != uxQueueMessagesWaiting(g_time_queue));

			xSemaphoreTake(spibus_mutex, portMAX_DELAY);
			LCDNokia_clear();
			LCDNokia_gotoXY(0, 0);
			LCDNokia_sendString((uint8_t*)"Hora Actual:");
			LCDNokia_gotoXY(5, 1);
			LCDNokia_sendChar((uint8_t) asciiDate->hours_h); /*! It prints H*/
			LCDNokia_sendChar((uint8_t) asciiDate->hours_l); /*! It prints h*/
			LCDNokia_sendChar(':'); /*! It prints a character*/
			LCDNokia_sendChar((uint8_t) asciiDate->minutes_h); /*! It prints M*/
			LCDNokia_sendChar((uint8_t) asciiDate->minutes_l); /*! It prints m*/
			LCDNokia_sendChar(':'); /*! It prints a character*/
			LCDNokia_sendChar((uint8_t) asciiDate->seconds_h); /*! It prints S*/
			LCDNokia_sendChar((uint8_t) asciiDate->seconds_l); /*! It prints s*/
			if(FORMAT_12H == asciiDate->timeformat)
			{
				switch(asciiDate->ampm)
				{
				case FORMAT_AM:
					LCDNokia_sendString((uint8_t*)" am");
					break;
				case FORMAT_PM:
					LCDNokia_sendString((uint8_t*)" pm");
					break;
				}
			}
			LCDNokia_gotoXY(0, 3);
			LCDNokia_sendString((uint8_t*)"Fecha:");
			LCDNokia_gotoXY(8, 4);
			LCDNokia_sendChar((uint8_t) asciiDate->day_h); /*! It prints a character*/
			LCDNokia_sendChar((uint8_t) asciiDate->day_l); /*! It prints a character*/
			LCDNokia_sendChar('-'); /*! It prints a character*/
			LCDNokia_sendChar((uint8_t) asciiDate->month_h); /*! It prints a character*/
			LCDNokia_sendChar((uint8_t) asciiDate->month_l); /*! It prints a character*/
			LCDNokia_sendChar('-');
			LCDNokia_sendString((uint8_t*)"20"); /*! It prints a character*/
			LCDNokia_sendChar((uint8_t) asciiDate->year_h); /*! It prints a character*/
			LCDNokia_sendChar((uint8_t) asciiDate->year_l); /*! It prints a character*/
			xSemaphoreGive(spibus_mutex);
			vPortFree(asciiDate);
		}
	}
}

void getTime_task(void *parameter)
{
	getTime_handle = xTaskGetCurrentTaskHandle();

	static uint8_t timeBuffer[8];
	ascii_time_t *asciiDate;
	status_t i2c_transfer;
	bool ioerror = false;

	/*Start Timer*/
	xSemaphoreTake(i2cbus_mutex,portMAX_DELAY);
	I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCSEC, OSCILLATOR_ON);
	xSemaphoreGive(i2cbus_mutex);

	while (1)
	{
		//Check if RTC is recovering from disconnect error
		if (ioerror)
		{
			ioerror = false;
			xSemaphoreTake(i2cbus_mutex,portMAX_DELAY);
			I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCSEC, OSCILLATOR_ON);
			xSemaphoreGive(i2cbus_mutex);
		}

		xSemaphoreTake(i2cbus_mutex,portMAX_DELAY);
		i2c_transfer = I2C_Read(I2C0, RTC_DEVICE_ADD, REG_RTCSEC, timeBuffer, 7);
		xSemaphoreGive(i2cbus_mutex);


		if(kStatus_Success == i2c_transfer)
		{
			timeBuffer[7] = timeBuffer[2] & TIME_FORMAT_SIZE;
			timeBuffer[0] = timeBuffer[0] & SECONDS_REG_SIZE;
			timeBuffer[1] = timeBuffer[1] & MINUTES_REG_SIZE;
			timeBuffer[2] = timeBuffer[2] & HOURS_REG_SIZE;
			timeBuffer[4] = timeBuffer[4] & DAY_REG_SIZE;
			timeBuffer[5] = timeBuffer[5] & MONTH_REG_SIZE;
			timeBuffer[6] = timeBuffer[6] & YEAR_REG_SIZE;

			asciiDate = pvPortMalloc(sizeof(ascii_time_t));
			asciiDate->timeformat = ((timeBuffer[7] & TIME_FORMAT_SIZE) >> 6);
			asciiDate->seconds_l = ((timeBuffer[0] & BCD_L)) + ASCII_NUMBER_MASK;
			asciiDate->seconds_h = ((timeBuffer[0] & BCD_H) >> 4) + ASCII_NUMBER_MASK;
			asciiDate->minutes_l = ((timeBuffer[1] & BCD_L)) + ASCII_NUMBER_MASK;
			asciiDate->minutes_h = ((timeBuffer[1] & BCD_H) >> 4) + ASCII_NUMBER_MASK;
			if(FORMAT_24H == asciiDate->timeformat)
			{
				asciiDate->hours_l = ((timeBuffer[2] & BCD_L)) + ASCII_NUMBER_MASK;
				asciiDate->hours_h = ((timeBuffer[2] & BCD_H) >> 4) + ASCII_NUMBER_MASK;
			}
			else
			{
				asciiDate->hours_l = ((timeBuffer[2] & BCD_L)) + ASCII_NUMBER_MASK;
				asciiDate->hours_h = ((timeBuffer[2] & 0x10) >> 4) + ASCII_NUMBER_MASK;
				asciiDate->ampm = ((timeBuffer[2] & 0x20) >> 5);
			}
			asciiDate->day_l = ((timeBuffer[4] & BCD_L)) + ASCII_NUMBER_MASK;
			asciiDate->day_h = ((timeBuffer[4] & BCD_H) >> 4) + ASCII_NUMBER_MASK;
			asciiDate->month_l = ((timeBuffer[5] & BCD_L)) + ASCII_NUMBER_MASK;
			asciiDate->month_h = ((timeBuffer[5] & BCD_H) >> 4) + ASCII_NUMBER_MASK;
			asciiDate->year_l = ((timeBuffer[6] & BCD_L)) + ASCII_NUMBER_MASK;
			asciiDate->year_h = ((timeBuffer[6] & BCD_H) >> 4) + ASCII_NUMBER_MASK;
			xQueueSend(g_time_queue, &asciiDate, portMAX_DELAY);
			xEventGroupSetBits(getTime_eventB, EVENT_TIME_SET);
			//xEventGroupSetBits(timeTerminal_eventB, EVENT_TIME_SET);
		}
		else
		{
			ioerror = true;
			xEventGroupSetBits(getTime_eventB, EVENT_TIME_ERR);
			//xEventGroupSetBits(timeTerminal_eventB, EVENT_TIME_ERR);
		}
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

void echo_Task(void *parameter)
{
	echoTask_handle = xTaskGetCurrentTaskHandle();
	uint8_t recvBuffer;
	uint8_t maxChar = 0;

	UART_putString(UART_0, (uint8_t*)echo_menuTxt);

	xSemaphoreTake(spibus_mutex, portMAX_DELAY);
	LCDNokia_clear();
	xSemaphoreGive(spibus_mutex);

	while (1)
	{
		recvBuffer = UART_Echo(UART_0);

		if (ESC_KEY == recvBuffer)
		{
			xSemaphoreTake(spibus_mutex, portMAX_DELAY);
			LCDNokia_clear();
			xSemaphoreGive(spibus_mutex);
			vTaskResume(timedateLCD_handle);
			vTaskResume(getTime_handle);
			vTaskResume(menuTask_handle);
			vTaskDelete(echoTask_handle);
		}

		xSemaphoreTake(spibus_mutex, portMAX_DELAY);
		LCDNokia_sendChar(recvBuffer);
		xSemaphoreGive(spibus_mutex);
		maxChar++;
		if (maxChar >= 72)
		{
			xSemaphoreTake(spibus_mutex, portMAX_DELAY);
			LCDNokia_clear();
			xSemaphoreGive(spibus_mutex);
			maxChar = 0;
		}
	}
}

void memread_task(void *parameters){

	uint16_t subaddress = 0;
	uint16_t readlen = 0;
	uint8_t charAddress[5];
	uint8_t charlen[3];
	uint8_t charCounter = 0;
	static uint8_t *recvBuff;

	UART_putString(UART_0, (uint8_t*)memread_Txt);

	while(1){

		UART_putString(UART_0, (uint8_t*)"\r\nIngrese la direccion de memoria inicial: 0x");
		while(4 > charCounter)
		{
			charAddress[charCounter] = UART_Echo(UART_0);
			charCounter++;
		}
		charCounter = 0;
		subaddress = asciiToHex(charAddress);
		UART_putString(UART_0, (uint8_t*)"\r\nIngrese la cantidad de bytes a leer: ");
		while(2 > charCounter)
		{
			charlen[charCounter] = UART_Echo(UART_0);
			charCounter++;
		}
		charCounter = 0;
		readlen = asciitoDec(charlen);

		xSemaphoreTake(i2cbus_mutex, portMAX_DELAY);
		if(kStatus_Fail == I2C_MEMRead(I2C0, MEM_DEVICE_ADD, subaddress, recvBuff, readlen))
		{
			UART_putString(UART_0, (uint8_t*) "ERROR READING MEMORY");
		}
		else
		{
			UART_putBytes(UART_0, recvBuff, readlen);
		}
		xSemaphoreGive(i2cbus_mutex);

		//TODO: FInish task
	}
}

void setTime_task(void * params)
{
	setTime_handle = xTaskGetCurrentTaskHandle();
	uint8_t charCounter = 0;
	uint8_t newTime[5];
	ascii_time_t *asciiDate;
	uint8_t timeFormat;

	UART_putString(UART_0, (uint8_t*)setTime_Txt);

	while(1)
	{
		do
		{
			xQueueReceive(g_time_queue, &asciiDate, portMAX_DELAY);
		} while (0 != uxQueueMessagesWaiting(g_time_queue));

		UART_putString(UART_0, (uint8_t*)"\r\nIntroduzca la hora en formato HH:MM --> ");
		while(4 > charCounter)
		{
			newTime[charCounter] = UART_Echo(UART_0);
			if(ESC_KEY == newTime[charCounter])
			{
				vTaskResume(menuTask_handle);
				vTaskDelete(setTime_handle);
			}
			charCounter++;
		}
		charCounter = 0;
		if (FORMAT_12H == asciiDate->timeformat)
		{
			UART_putString(UART_0,
					(uint8_t*) "\r\nSe esta usando un formato de 12 Horas,\r\nindica si la nueva hora es AM o PM (a/p)");
			switch(UART_Echo(UART_0))
			{
			case 'a':
				timeFormat = FORMAT_AM;
				break;
			case 'p':
				timeFormat = FORMAT_PM;
				break;
			}
		}
		UART_putString(UART_0, (uint8_t*)"\r\nLa hora introducida es ");
		UART_putBytes(UART_0, &newTime[0], 1);
		UART_putBytes(UART_0, &newTime[1], 1);
		UART_putBytes(UART_0, (uint8_t*)":", 1);
		UART_putBytes(UART_0, &newTime[2], 1);
		UART_putBytes(UART_0, &newTime[3], 1);
		if (FORMAT_12H == asciiDate->timeformat)
		{
			switch(timeFormat)
			{
			case FORMAT_AM:
				UART_putString(UART_0, (uint8_t*)" am");
				break;
			case FORMAT_PM:
				UART_putString(UART_0, (uint8_t*)" pm");
			}
		}
		UART_putString(UART_0, (uint8_t*)"\r\nPresiona cualquier tecla para confirmar o ESC para cancelar...");
		if(ESC_KEY == UART_Echo(UART_0))
		{
			vTaskResume(menuTask_handle);
			vTaskDelete(setTime_handle);
		}
		else
		{
			xSemaphoreTake(i2cbus_mutex, portMAX_DELAY);
			I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCSEC, OSCILLATOR_OFF);
			xSemaphoreGive(i2cbus_mutex);
			if (FORMAT_12H == asciiDate->timeformat)
			{
				switch (timeFormat)
				{
				case FORMAT_PM:
					xSemaphoreTake(i2cbus_mutex, portMAX_DELAY);
					I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCHOUR,
							((FORMAT_PM<<5) | (newTime[0]-ASCII_NUMBER_MASK)<<4 | (newTime[1]-ASCII_NUMBER_MASK) | (asciiDate->timeformat<<6)));
					xSemaphoreGive(i2cbus_mutex);
					break;
				case FORMAT_AM:
					xSemaphoreTake(i2cbus_mutex, portMAX_DELAY);
					I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCHOUR,
							((FORMAT_AM<<5)  | (newTime[0]-ASCII_NUMBER_MASK)<<4 | (newTime[1]-ASCII_NUMBER_MASK) | (asciiDate->timeformat<<6)));
					xSemaphoreGive(i2cbus_mutex);
					break;
				}
			}
			else
			{	xSemaphoreTake(i2cbus_mutex, portMAX_DELAY);
				I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCHOUR, (newTime[0]-ASCII_NUMBER_MASK)<<4 | (newTime[1]-ASCII_NUMBER_MASK));
				xSemaphoreGive(i2cbus_mutex);
			}
			xSemaphoreTake(i2cbus_mutex, portMAX_DELAY);
			I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCMIN, (newTime[2]-ASCII_NUMBER_MASK)<<4 | (newTime[3]-ASCII_NUMBER_MASK));
			I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCSEC, OSCILLATOR_ON);
			xSemaphoreGive(i2cbus_mutex);
			vTaskResume(menuTask_handle);
			vTaskDelete(setTime_handle);
		}
	}

}

void setDate_task(void * params)
{
	setDate_handle = xTaskGetCurrentTaskHandle();
	uint8_t charCounter = 0;
	uint8_t newDate[7];

	UART_putString(UART_0, (uint8_t*)setDate_Txt);

	while(1)
	{
		UART_putString(UART_0, (uint8_t*)"\r\nIntroduzca la fecha en formato DD/MM/AA --> ");
		while(6 > charCounter)
		{
			newDate[charCounter] = UART_Echo(UART_0);
			if(ESC_KEY == newDate[charCounter])
			{
				vTaskResume(menuTask_handle);
				vTaskDelete(setDate_handle);
			}
			charCounter++;
		}
		charCounter = 0;
		UART_putString(UART_0, (uint8_t*)"\r\nLa fecha introducida es ");
		UART_putBytes(UART_0, &newDate[0], 1);
		UART_putBytes(UART_0, &newDate[1], 1);
		UART_putBytes(UART_0, (uint8_t*)"-", 1);
		UART_putBytes(UART_0, &newDate[2], 1);
		UART_putBytes(UART_0, &newDate[3], 1);
		UART_putBytes(UART_0, (uint8_t*)"-20", 1);
		UART_putBytes(UART_0, &newDate[4], 1);
		UART_putBytes(UART_0, &newDate[5], 1);
		UART_putString(UART_0, (uint8_t*)"\r\nPresiona cualquier tecla para confirmar o ESC para cancelar...");
		if(ESC_KEY == UART_Echo(UART_0))
		{
			vTaskResume(menuTask_handle);
			vTaskDelete(setDate_handle);
		}
		else
		{
			xSemaphoreTake(i2cbus_mutex, portMAX_DELAY);
			I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCSEC, OSCILLATOR_OFF);
			I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCDATE, (newDate[0]-ASCII_NUMBER_MASK)<<4 | (newDate[1]-ASCII_NUMBER_MASK));
			I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCMTH, (newDate[2]-ASCII_NUMBER_MASK)<<4 | (newDate[3]-ASCII_NUMBER_MASK));
			I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCYEAR, (newDate[4]-ASCII_NUMBER_MASK)<<4 | (newDate[5]-ASCII_NUMBER_MASK));
			I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCSEC, OSCILLATOR_ON);
			xSemaphoreGive(i2cbus_mutex);
			vTaskResume(menuTask_handle);
			vTaskDelete(setDate_handle);
		}
	}
}

void hourFormat_task(void * params)
{
	hourFormat_handle = xTaskGetCurrentTaskHandle();
	ascii_time_t *asciiDate;

	UART_putString(UART_0, (uint8_t*) hourFormat_Txt);

	while (1)
	{
		do
		{
			xQueueReceive(g_time_queue, &asciiDate, portMAX_DELAY);
		} while (0 != uxQueueMessagesWaiting(g_time_queue));

		UART_putString(UART_0, (uint8_t*) "\r\nEl formato de hora actual es: ");
		switch (asciiDate->timeformat)
		{
		case FORMAT_24H:
			UART_putString(UART_0,
					(uint8_t*) "* 24 Horas *\r\n\nDeseas cambiarlo a 12 Horas? (y/n)");
			if ('y' == UART_Echo(UART_0))
			{
				xSemaphoreTake(i2cbus_mutex, portMAX_DELAY);
				I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCSEC, OSCILLATOR_OFF);
				I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCHOUR,
						((FORMAT_12H << 6)
								| (asciiDate->hours_h - ASCII_NUMBER_MASK) << 4
								| (asciiDate->hours_l - ASCII_NUMBER_MASK)));
				I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCSEC, OSCILLATOR_ON);
				xSemaphoreGive(i2cbus_mutex);
				UART_putString(UART_0,
						(uint8_t*) "\r\n\nEl formato ha sido cambiado, presiona una tecla para salir...");
				UART_Echo(UART_0);
				vTaskResume(menuTask_handle);
				vTaskDelete(hourFormat_handle);
			}
			else
			{
				UART_putString(UART_0,
						(uint8_t*) "\r\n\nEl formato permanece igual, presiona una tecla para salir...");
				UART_Echo(UART_0);
				vTaskResume(menuTask_handle);
				vTaskDelete(hourFormat_handle);
			}
			break;
		case FORMAT_12H:
			UART_putString(UART_0,
					(uint8_t*) "* 12 Horas *\r\n\nDeseas cambiarlo a 24 Horas? (y/n)");
			if ('y' == UART_Echo(UART_0))
			{
				xSemaphoreTake(i2cbus_mutex, portMAX_DELAY);
				I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCSEC, OSCILLATOR_OFF);
				I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCHOUR,
						((FORMAT_24H << 6)
								| (asciiDate->hours_h - ASCII_NUMBER_MASK) << 4
								| (asciiDate->hours_l - ASCII_NUMBER_MASK)));
				I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCSEC, OSCILLATOR_ON);
				xSemaphoreGive(i2cbus_mutex);
				UART_putString(UART_0,
						(uint8_t*) "\r\n\nEl formato ha sido cambiado, presiona una tecla para salir...");
				UART_Echo(UART_0);
				vTaskResume(menuTask_handle);
				vTaskDelete(hourFormat_handle);
			}
			else
			{
				UART_putString(UART_0,
						(uint8_t*) "\r\n\nEl formato permanece igual, presiona una tecla para salir...");
				UART_Echo(UART_0);
				vTaskResume(menuTask_handle);
				vTaskDelete(hourFormat_handle);
			}
			break;
		}
	}
}

void timeTerminal_task(void *params)
{
	timeTerminal_handle = xTaskGetCurrentTaskHandle();
	ascii_time_t *asciiDate;

	UART_putString(UART_0, (uint8_t*) terminalTime_Txt);

	while (1)
	{
		xEventGroupWaitBits(timeTerminal_eventB,
				(EVENT_TIME_SET | EVENT_TIME_ERR), pdFALSE, pdFALSE,
				portMAX_DELAY);
		EventBits_t event = xEventGroupGetBits(timeTerminal_eventB);
		xEventGroupClearBits(timeTerminal_eventB,
				EVENT_TIME_ERR | EVENT_TIME_SET);
		if (EVENT_TIME_ERR == (event & EVENT_TIME_ERR))
		{
			UART_putString(UART_0, (uint8_t*) terminalTime_Txt);
			UART_putString(UART_0,
					(uint8_t*) "\r\n\n --Error de lectura, verificar conexion al bus de I2C--");
		}
		else
		{
			do
			{
				xQueueReceive(g_time_queue, &asciiDate, portMAX_DELAY);
			} while (0 != uxQueueMessagesWaiting(g_time_queue));

			UART_putString(UART_0, (uint8_t*) terminalTime_Txt);
			UART_putString(UART_0, (uint8_t*) "\r\n\nLa hora actual es: \r\n\n"
					"      ");
			UART_putBytes(UART_0, &asciiDate->hours_h, 1);
			UART_putBytes(UART_0, &asciiDate->hours_l, 1);
			UART_putBytes(UART_0, (uint8_t*) ":", 1);
			UART_putBytes(UART_0, &asciiDate->minutes_h, 1);
			UART_putBytes(UART_0, &asciiDate->minutes_l, 1);
			UART_putBytes(UART_0, (uint8_t*) ":", 1);
			UART_putBytes(UART_0, &asciiDate->seconds_h, 1);
			UART_putBytes(UART_0, &asciiDate->seconds_l, 1);
			if (FORMAT_12H == asciiDate->timeformat)
			{
				switch (asciiDate->ampm)
				{
				case FORMAT_AM:
					UART_putString(UART_0,
							(uint8_t*) " am\r\n\n\n\nPresione cualquier tecla para salir");
					break;
				case FORMAT_PM:
					UART_putString(UART_0,
							(uint8_t*) " pm\r\n\n\n\nPresione cualquier tecla para salir");
					break;
				}
			}
			else
			{
				UART_putString(UART_0,
						(uint8_t*) "\r\n\n\n\n -- Presione cualquier tecla para salir --");
			}
			vPortFree(asciiDate);
//			UART_Echo(UART_0);
//			vTaskResume(menuTask_handle);
//			vTaskDelete(timeTerminal_handle);
		}
	}
}

void dateTerminal_task(void * params)
{
	dateTerminal_handle = xTaskGetCurrentTaskHandle();
	ascii_time_t *asciiDate;

	UART_putString(UART_0, (uint8_t*) terminalDate_Txt);

	while (1)
	{
		xEventGroupWaitBits(timeTerminal_eventB,
				(EVENT_TIME_SET | EVENT_TIME_ERR), pdFALSE, pdFALSE,
				portMAX_DELAY);
		EventBits_t event = xEventGroupGetBits(timeTerminal_eventB);
		xEventGroupClearBits(timeTerminal_eventB,
				EVENT_TIME_ERR | EVENT_TIME_SET);
		if (EVENT_TIME_ERR == (event & EVENT_TIME_ERR))
		{
			UART_putString(UART_0, (uint8_t*) terminalDate_Txt);
			UART_putString(UART_0,
					(uint8_t*) "\r\n\n --Error de lectura, verificar conexion al bus de I2C--");
		}
		else
		{
			do
			{
				xQueueReceive(g_time_queue, &asciiDate, portMAX_DELAY);
			} while (0 != uxQueueMessagesWaiting(g_time_queue));

			UART_putString(UART_0, (uint8_t*) terminalDate_Txt);
			UART_putString(UART_0, (uint8_t*) "\r\n\nLa fecha de hoy es: \r\n\n"
					"      ");
			UART_putBytes(UART_0, &asciiDate->day_h, 1);
			UART_putBytes(UART_0, &asciiDate->day_l, 1);
			UART_putBytes(UART_0, (uint8_t*) "-", 1);
			UART_putBytes(UART_0, &asciiDate->month_h, 1);
			UART_putBytes(UART_0, &asciiDate->month_l, 1);
			UART_putString(UART_0, (uint8_t*) "-20");
			UART_putBytes(UART_0, &asciiDate->year_h, 1);
			UART_putBytes(UART_0, &asciiDate->year_l, 1);
			UART_putString(UART_0,
					(uint8_t*) "\r\n\n\n\n -- Presione cualquier tecla para salir --");
			vPortFree(asciiDate);
			UART_Echo(UART_0);
			vTaskResume(menuTask_handle);
			vTaskDelete(dateTerminal_handle);
		}
	}

}
