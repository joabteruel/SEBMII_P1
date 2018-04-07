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
TaskHandle_t echoTask_handle;

SemaphoreHandle_t i2cbus_mutex;
SemaphoreHandle_t spibus_mutex;

EventGroupHandle_t getTime_eventB;

QueueHandle_t g_time_queue;

void os_init()
{
	i2cbus_mutex = xSemaphoreCreateMutex();
	spibus_mutex = xSemaphoreCreateMutex();

	getTime_eventB = xEventGroupCreate();

	g_time_queue = xQueueCreate(1, sizeof(ascii_time_t*));
}




void menu0_Task(void *parameter)
{
	uint8_t recBuffer[1];
	size_t dataSize;
	menuTask_handle = xTaskGetCurrentTaskHandle();

	while (1)
	{

		/**VT100 command for hide cursor*/
		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"\e[ ? 25 l", sizeof("\e[ ? 25 l"));
		/*VT100 command for clearing the screen*/
		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"\033[2J", sizeof("\033[2J"));
		/** VT100 command for positioning the cursor in x and y position*/
		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"\033[5;10H", sizeof("\033[5;10H"));
		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"Sistemas Basados en Micros\r",
				sizeof("Sistemas Basados en Micros\r"));
		/** VT100 command for positioning the cursor in x and y position*/
		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"\033[7;10H", sizeof("\033[7;10H"));
		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"    ITESO\r",
				sizeof("    ITESO\r"));
		/** VT100 command for positioning the cursor in x and y position*/
		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"\033[9;10H", sizeof("\033[9;10H"));
		UART_RTOS_Send(getHandleUART0(), (uint8_t *)" Opciones:\r",
				sizeof(" Opciones:\r"));

		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"\033[11;10H",
				sizeof("\033[11;10H"));
		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"  1)  Leer Memoria I2C\r",
				sizeof("  1)  Leer Memoria I2C\r"));

		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"\033[12;10H",
				sizeof("\033[12;10H"));
		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"  2)  Escribir memoria I2C\r",
				sizeof("  2)  Escribir memoria I2C\r"));

		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"\033[13;10H",
				sizeof("\033[13;10H"));
		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"  3)  Establecer Hora\r",
				sizeof("  3)  Establecer Hora\r"));

		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"\033[14;10H",
				sizeof("\033[14;10H"));
		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"  4)  Establecer Fecha\r",
				sizeof("  4)  Establecer Fecha\r"));

		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"\033[15;10H",
				sizeof("\033[15;10H"));
		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"  5)  Formato de hora\r",
				sizeof("  5)  Formato de hora\r"));

		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"\033[16;10H",
				sizeof("\033[16;10H"));
		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"  6)  Leer hora\r",
				sizeof("  6)  Leer hora\r"));

		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"\033[17;10H",
				sizeof("\033[17;10H"));
		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"  7)  Leer fecha\r",
				sizeof("  7)  Leer fecha\r"));

		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"\033[18;10H",
				sizeof("\033[18;10H"));
		UART_RTOS_Send(getHandleUART0(),(uint8_t *)
				"  8)  Comunicacion con terminal 2\r",
				sizeof("  8)  Comunicacion con terminal 2\r"));

		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"\033[19;10H",
				sizeof("\033[19;10H"));
		UART_RTOS_Send(getHandleUART0(), (uint8_t *)"  9)  Eco en LCD\r",
				sizeof("  9)  Eco en LCD\r"));

		UART_RTOS_Receive(getHandleUART0(), recBuffer, sizeof(recBuffer),
				&dataSize);

		switch(recBuffer[0]-ASCII_NUMBER_MASK)
		{
		case 9:
			vTaskSuspend(getTime_handle);
			vTaskSuspend(timedateLCD_handle);
			xTaskCreate(echo_Task, "echo0_task", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES-2, NULL);
			vTaskSuspend(NULL);
			break;
		}




//		de_menu_buffer = menu_buffer[0];
//
//		if( (*de_menu_buffer == de_menu_buffer_b) ){
//		if ((de_menu_buffer == de_menu_buffer_b) && (0x38 != de_menu_buffer))
//		{
//			/*VT100 command for clearing the screen*/
//			UART_RTOS_Send(getHandleUART0(), "\033[2J", sizeof("\033[2J"));
//			/** VT100 command for positioning the cursor in x and y position*/
//			UART_RTOS_Send(getHandleUART0(), "\033[5;10H",
//					sizeof("\033[5;10H"));
//			UART_RTOS_Send(getHandleUART0(),
//					"No es permitido acceder al mismo recurso",
//					sizeof("No es permitido acceder al mismo recurso"));
//			vTaskDelay(2500);
//		}
//		else
//		{ //llave del else abajo del switch case
//
//			switch (menu_buffer[0] - 0x30)
//			{
//
//			case 1:
//				xTaskCreate(read_Mem_Task, "read_mem_task",
//						configMINIMAL_STACK_SIZE, NULL, 1, NULL);
//				//vTaskResume(readMem_handle);
//				vTaskSuspend(menu_handle);
//				break;
//			case 2:
//				xTaskCreate(write_Mem_Task, "write_mem_task",
//						configMINIMAL_STACK_SIZE, NULL, 1, NULL);
//				//vTaskResume(writeMem_handle);
//				vTaskSuspend(menu_handle);
//				break;
//			case 3:
//				xTaskCreate(set_hour_Task, "set_hour_task",
//						configMINIMAL_STACK_SIZE, NULL, 1, NULL); //
//				//vTaskResume(set_hour_handle);//
//				vTaskSuspend(menu_handle);
//				break;
//			case 4:
//				xTaskCreate(set_date_Task, "set_date_task",
//						configMINIMAL_STACK_SIZE, NULL, 1, NULL);			//
//				//vTaskResume(set_date_handle);//
//				vTaskSuspend(menu_handle);
//				break;
//			case 5:
//				xTaskCreate(format_hour_Task, "format_hour_task",
//						configMINIMAL_STACK_SIZE, NULL, 1, NULL);			//
//				//vTaskResume(format_handle);//
//				vTaskSuspend(menu_handle);
//				break;
//			case 6:
//				xTaskCreate(read_hour_Task, "read_hour_task",
//						configMINIMAL_STACK_SIZE, NULL, 1, NULL);
//				//vTaskResume(read_hour_handle);
//				vTaskSuspend(menu_handle);
//				break;
//			case 7:
//				xTaskCreate(read_date_Task, "read_date_task",
//						configMINIMAL_STACK_SIZE, NULL, 1, NULL);
//				//vTaskResume(read_date_handle);
//				vTaskSuspend(menu_handle);
//				break;
//			case 8:
//				xTaskCreate(chat_Task, "chat_task", 250, NULL, 1, NULL);
//				//vTaskResume(chat_handle);
//				vTaskSuspend(menu_handle);
//				break;
//			case 9:
//				vTaskDelete(getTime_handle);
//				vTaskDelete(timedateLCD_handle);
//				xTaskCreate(echo_Task, "echo_task", configMINIMAL_STACK_SIZE,
//						NULL, 1, NULL);			//
//				//vTaskResume(echo_handle);//
//				vTaskSuspend(menu_handle);
//				break;
//			default:
//				UART_RTOS_Send(getHandleUART0(), "\n\r Seleccion no valida",
//						sizeof("\n\r Seleccion no valida"));
//				vTaskDelay(1500);
//				/*VT100 command for clearing the screen*/
//				UART_RTOS_Send(getHandleUART0(), "\033[2J",
//						sizeof("\033[2J"));
//				break;
//			}			//switch case
//			de_menu_buffer = 0;
//		}			//if que checa si el otro recurso esta utilizado

	}			//while
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
			LCDNokia_sendString((uint8_t*)"Current Time");
			LCDNokia_gotoXY(16, 1);
			LCDNokia_sendChar((uint8_t) asciiDate->hours_h); /*! It prints H*/
			LCDNokia_sendChar((uint8_t) asciiDate->hours_l); /*! It prints h*/
			LCDNokia_sendChar(':'); /*! It prints a character*/
			LCDNokia_sendChar((uint8_t) asciiDate->minutes_h); /*! It prints M*/
			LCDNokia_sendChar((uint8_t) asciiDate->minutes_l); /*! It prints m*/
			LCDNokia_sendChar(':'); /*! It prints a character*/
			LCDNokia_sendChar((uint8_t) asciiDate->seconds_h); /*! It prints S*/
			LCDNokia_sendChar((uint8_t) asciiDate->seconds_l); /*! It prints s*/
			LCDNokia_gotoXY(0, 3);
			LCDNokia_sendString((uint8_t*)"Current Date");
			LCDNokia_gotoXY(8, 4);
			LCDNokia_sendChar((uint8_t) asciiDate->day_h); /*! It prints a character*/
			LCDNokia_sendChar((uint8_t) asciiDate->day_l); /*! It prints a character*/
			LCDNokia_sendChar('/'); /*! It prints a character*/
			LCDNokia_sendChar((uint8_t) asciiDate->month_h); /*! It prints a character*/
			LCDNokia_sendChar((uint8_t) asciiDate->month_l); /*! It prints a character*/
			LCDNokia_sendChar('/');
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

	static uint8_t timeBuffer[7];
	ascii_time_t *asciiDate;
	status_t i2c_transfer;
	bool ioerror = false;


	/*Start Timer*/
	xSemaphoreTake(i2cbus_mutex, portMAX_DELAY);
	I2C_Write(I2C0, RTC_DEVICE_ADD, 0x00, 0x80);
	xSemaphoreGive(i2cbus_mutex);

	while (1)
	{
		if (ioerror)
		{
			ioerror = false;
			xSemaphoreTake(i2cbus_mutex, portMAX_DELAY);
			I2C_Write(I2C0, RTC_DEVICE_ADD, 0x00, 0x80);
			xSemaphoreGive(i2cbus_mutex);
		}

		xSemaphoreTake(i2cbus_mutex,portMAX_DELAY);
		i2c_transfer = I2C_Read(I2C0, RTC_DEVICE_ADD, 0x00, timeBuffer, 7);
		xSemaphoreGive(i2cbus_mutex);

		if(kStatus_Success == i2c_transfer)
		{
			timeBuffer[0] = timeBuffer[0] & SECONDS_REG_SIZE;
			timeBuffer[1] = timeBuffer[1] & MINUTES_REG_SIZE;
			timeBuffer[2] = timeBuffer[2] & HOURS_REG_SIZE;
			timeBuffer[4] = timeBuffer[4] & DAY_REG_SIZE;
			timeBuffer[5] = timeBuffer[5] & MONTH_REG_SIZE;
			timeBuffer[6] = timeBuffer[6] & YEAR_REG_SIZE;

			asciiDate = pvPortMalloc(sizeof(ascii_time_t));
			asciiDate->seconds_l = ((timeBuffer[0] & BCD_L)) + ASCII_NUMBER_MASK;
			asciiDate->seconds_h = ((timeBuffer[0] & BCD_H) >> 4) + ASCII_NUMBER_MASK;
			asciiDate->minutes_l = ((timeBuffer[1] & BCD_L)) + ASCII_NUMBER_MASK;
			asciiDate->minutes_h = ((timeBuffer[1] & BCD_H) >> 4) + ASCII_NUMBER_MASK;
			asciiDate->hours_l = ((timeBuffer[2] & BCD_L)) + ASCII_NUMBER_MASK;
			asciiDate->hours_h = ((timeBuffer[2] & BCD_H) >> 4) + ASCII_NUMBER_MASK;
			asciiDate->day_l = ((timeBuffer[4] & BCD_L)) + ASCII_NUMBER_MASK;
			asciiDate->day_h = ((timeBuffer[4] & BCD_H) >> 4) + ASCII_NUMBER_MASK;
			asciiDate->month_l = ((timeBuffer[5] & BCD_L)) + ASCII_NUMBER_MASK;
			asciiDate->month_h = ((timeBuffer[5] & BCD_H) >> 4) + ASCII_NUMBER_MASK;
			asciiDate->year_l = ((timeBuffer[6] & BCD_L)) + ASCII_NUMBER_MASK;
			asciiDate->year_h = ((timeBuffer[6] & BCD_H) >> 4) + ASCII_NUMBER_MASK;
			xQueueSend(g_time_queue, &asciiDate, portMAX_DELAY);
			xEventGroupSetBits(getTime_eventB, EVENT_TIME_SET);
		}
		else
		{
			ioerror = true;
			xEventGroupSetBits(getTime_eventB, EVENT_TIME_ERR);
		}
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

void echo_Task(void *parameter)
{
	echoTask_handle = xTaskGetCurrentTaskHandle();
	uint8_t recvBuffer[1];
	uint8_t maxChar = 0;
	size_t dataSize;

	UART_RTOS_Send(getHandleUART0(), (uint8_t *)"\033[2J", sizeof("\033[2J"));
	UART_RTOS_Send(getHandleUART0(), (uint8_t *)"\033[1;1H", sizeof("\033[1;1H"));
	UART_RTOS_Send(getHandleUART0(), (uint8_t *)"Escribir texto: ",
			sizeof("Escribir texto: "));
	UART_RTOS_Send(getHandleUART0(), (uint8_t *)"\033[2;1H", sizeof("\033[2;1H"));

	xSemaphoreTake(spibus_mutex, portMAX_DELAY);
	LCDNokia_clear();
	xSemaphoreGive(spibus_mutex);

	while (1)
	{

		/* Send data */
		UART_RTOS_Receive(getHandleUART0(), recvBuffer,
				sizeof(recvBuffer), &dataSize);
		if (dataSize > 0)
		{
			/* Echo the received data */
			UART_RTOS_Send(getHandleUART0(), (uint8_t *) recvBuffer, dataSize);
		}

		if (ESC_KEY == recvBuffer[0])
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
		LCDNokia_sendChar(recvBuffer[0]);
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
