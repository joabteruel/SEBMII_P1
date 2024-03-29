/*
 * usertask.c
 *
 *  Created on: Mar 13, 2018
 *      Author: Joab
 */

#include "usertask.h"

TaskHandle_t menu0Task_handle;
TaskHandle_t menu3Task_handle;
TaskHandle_t timedateLCD_handle;
TaskHandle_t getTime_handle;
TaskHandle_t setTime_handle;
TaskHandle_t setDate_handle;
TaskHandle_t hourFormat_handle;
TaskHandle_t echoTask_handle;
TaskHandle_t timeTerminal_handle;
TaskHandle_t dateTerminal_handle;
TaskHandle_t chatTask_handle;

SemaphoreHandle_t spibus_mutex;
SemaphoreHandle_t i2cbus_mutex;
SemaphoreHandle_t uart_mutex;

EventGroupHandle_t getTime_eventB;
EventGroupHandle_t timeTerminal_eventB;
EventGroupHandle_t chatNotifications_eventB;
EventGroupHandle_t uart_interrupt_event;

QueueHandle_t g_time_queue;
QueueHandle_t g_uart0_queue;
QueueHandle_t g_uart3_queue;

uint8_t uart0_irqData;
uint8_t uart3_irqData;

void UART0_IRQHandler(void)
{
	if(UART0_IRQ_ENABLE == (UART0_IRQ_ENABLE & xEventGroupGetBitsFromISR(uart_interrupt_event)))
		/* If new data arrived. */
		if ((kUART_RxDataRegFullFlag | kUART_RxOverrunFlag)	& UART_GetStatusFlags(UART0))
		{
			xSemaphoreTakeFromISR(uart_mutex,pdFALSE);
			uart0_irqData = UART_ReadByte(UART0);
			xSemaphoreGiveFromISR(uart_mutex,pdFALSE);
			xEventGroupSetBitsFromISR(uart_interrupt_event, UART0_RX_INTERRUPT_EVENT, pdFALSE);
			portYIELD_FROM_ISR(pdFALSE);
		}
	UART0_DriverIRQHandler();

    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
      exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}

void UART3_IRQHandler(void)
{
	if(UART3_IRQ_ENABLE == (UART3_IRQ_ENABLE & xEventGroupGetBitsFromISR(uart_interrupt_event)))
		/* If new data arrived. */
		if ((kUART_RxDataRegFullFlag | kUART_RxOverrunFlag)	& UART_GetStatusFlags(UART3))
		{
			xSemaphoreTakeFromISR(uart_mutex,pdFALSE);
			uart3_irqData = UART_ReadByte(UART3);
			xSemaphoreGiveFromISR(uart_mutex,pdFALSE);
			xEventGroupSetBitsFromISR(uart_interrupt_event, UART3_RX_INTERRUPT_EVENT, pdFALSE);
			portYIELD_FROM_ISR(pdFALSE);
		}
	UART3_DriverIRQHandler();

    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
      exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}

uint16_t asciiToHex(uint8_t *string)
{
	volatile uint16_t hexAddress;
	hexAddress = 0x0000;
	uint8_t counter = 0;
	while (4 > counter)
	{
		hexAddress = hexAddress << 4;
		if (*string >= 'A' && *string <= 'F')
		{
			hexAddress |= *string - ASCII_LETTER_MASK;
			*string++;
			counter++;
		}
		else
		{
			hexAddress |= *string - ASCII_NUMBER_MASK;
			*string++;
			counter++;
		}
	}
	return hexAddress;
}

uint8_t asciitoDec(uint8_t *string)
{
	uint8_t decNum = 0;
	uint8_t counter = 0;
	while(2 > counter)
	{
		decNum = decNum*10 + (*string - ASCII_NUMBER_MASK);
		*string++;
		counter++;
	}
	return decNum;
}

void os_init()
{
	spibus_mutex = xSemaphoreCreateMutex();
	i2cbus_mutex = xSemaphoreCreateMutex();
	uart_mutex = xSemaphoreCreateMutex();
	getTime_eventB = xEventGroupCreate();
	timeTerminal_eventB = xEventGroupCreate();
	chatNotifications_eventB = xEventGroupCreate();

	uart_interrupt_event = xEventGroupCreate();

	g_time_queue = xQueueCreate(1, sizeof(ascii_time_t*));
	g_uart0_queue = xQueueCreate(30, sizeof(uint8_t));
	g_uart3_queue = xQueueCreate(30, sizeof(uint8_t));

}

void menu0_Task(void *parameter)
{
	uint8_t recvBuffer;
	menu0Task_handle = xTaskGetCurrentTaskHandle();

	while (1)
	{
		UART_putString(UART_0, (uint8_t*) main_menuTxt);
		recvBuffer = UART_Echo(UART_0);
		switch (recvBuffer - ASCII_NUMBER_MASK)
		{
		case 1:
			if(NULL != xTaskGetHandle("memread_task"))
			{
				UART_putString(UART_0, (uint8_t*) errorMes_Txt);
				vTaskDelay(pdMS_TO_TICKS(3000));
			}
			else
			{
				xTaskCreate(memread_task, "memread_task", configMINIMAL_STACK_SIZE + 50, (void*)UART_0, configMAX_PRIORITIES-2, NULL);
				vTaskSuspend(NULL);
			}
			break;
		case 2:
			if(NULL != xTaskGetHandle("memwrite_task"))
			{
				UART_putString(UART_0, (uint8_t*) errorMes_Txt);
				vTaskDelay(pdMS_TO_TICKS(3000));
			}
			else
			{
				xTaskCreate(memWrite_task, "memwrite_task", configMINIMAL_STACK_SIZE + 100, (void*)UART_0, configMAX_PRIORITIES-2, NULL);
				vTaskSuspend(NULL);
			}
			break;
		case 3:
			if(NULL != xTaskGetHandle("setTime_task"))
			{
				UART_putString(UART_0, (uint8_t*) errorMes_Txt);
				vTaskDelay(pdMS_TO_TICKS(3000));
			}
			else
			{
				xTaskCreate(setTime_task, "setTime_task", configMINIMAL_STACK_SIZE, (void*)UART_0, configMAX_PRIORITIES-2, NULL);
				vTaskSuspend(NULL);
			}
			break;
		case 4:
			if(NULL != xTaskGetHandle("setDate_task"))
			{
				UART_putString(UART_0, (uint8_t*) errorMes_Txt);
				vTaskDelay(pdMS_TO_TICKS(3000));
			}
			else
			{
				xTaskCreate(setDate_task, "setDate_task", configMINIMAL_STACK_SIZE, (void*)UART_0, configMAX_PRIORITIES-2, NULL);
				vTaskSuspend(NULL);
			}
			break;
		case 5:
			if(NULL != xTaskGetHandle("hourFormat_task"))
			{
				UART_putString(UART_0, (uint8_t*) errorMes_Txt);
				vTaskDelay(pdMS_TO_TICKS(3000));
			}
			else
			{
				xTaskCreate(hourFormat_task, "hourFormat_task", configMINIMAL_STACK_SIZE, (void*)UART_0, configMAX_PRIORITIES-2, NULL);
				vTaskSuspend(NULL);
			}
			break;
		case 6:
			if(NULL != xTaskGetHandle("timeTerminal_task"))
			{
				UART_putString(UART_0, (uint8_t*) errorMes_Txt);
				vTaskDelay(pdMS_TO_TICKS(3000));
			}
			else
			{
				xTaskCreate(timeTerminal_task, "timeTerminal_task", configMINIMAL_STACK_SIZE, (void*)UART_0, configMAX_PRIORITIES-2, NULL);
				vTaskSuspend(NULL);
			}
			break;
		case 7:
			if(NULL != xTaskGetHandle("dateTerminal_task"))
			{
				UART_putString(UART_0, (uint8_t*) errorMes_Txt);
				vTaskDelay(pdMS_TO_TICKS(3000));
			}
			else
			{
				xTaskCreate(dateTerminal_task, "dateTerminal_task", configMINIMAL_STACK_SIZE, (void*)UART_0, configMAX_PRIORITIES-2, NULL);
				vTaskSuspend(NULL);
			}
			break;
		case 8:
				xTaskCreate(chat_task, "chat_task", configMINIMAL_STACK_SIZE + 300, (void*)UART_0, configMAX_PRIORITIES-2, NULL);
				vTaskSuspend(NULL);
			break;
		case 9:
			vTaskSuspend(timedateLCD_handle);
			if(NULL != xTaskGetHandle("echo_task"))
			{
				UART_putString(UART_0, (uint8_t*) errorMes_Txt);
				vTaskDelay(pdMS_TO_TICKS(3000));
			}
			else
			{
			xTaskCreate(echo_Task, "echo_task", configMINIMAL_STACK_SIZE, (void*)UART_0,
					configMAX_PRIORITIES - 2, NULL);
			vTaskSuspend(NULL);
			}
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

void menu3_Task(void *parameter)
{
	uint8_t recvBuffer;
	menu3Task_handle = xTaskGetCurrentTaskHandle();

	while (1)
	{
		UART_putString(UART_3, (uint8_t*) main_menuTxt);
		recvBuffer = UART_Echo(UART_3);
		switch (recvBuffer - ASCII_NUMBER_MASK)
		{
		case 1:
			if(NULL != xTaskGetHandle("memread_task"))
			{
				UART_putString(UART_3, (uint8_t*) errorMes_Txt);
				vTaskDelay(pdMS_TO_TICKS(3000));
			}
			else
			{
			xTaskCreate(memread_task, "memread_task", configMINIMAL_STACK_SIZE + 50, (void*)UART_3, configMAX_PRIORITIES-2, NULL);
			vTaskSuspend(NULL);
			}
			break;
		case 2:
			if(NULL != xTaskGetHandle("memwrite_task"))
			{
				UART_putString(UART_3, (uint8_t*) errorMes_Txt);
				vTaskDelay(pdMS_TO_TICKS(3000));
			}
			else
			{
				xTaskCreate(memWrite_task, "memwrite_task", configMINIMAL_STACK_SIZE + 100, (void*)UART_3, configMAX_PRIORITIES-2, NULL);
				vTaskSuspend(NULL);
			}
			break;
		case 3:
			if(NULL != xTaskGetHandle("setTime_task"))
			{
				UART_putString(UART_3, (uint8_t*) errorMes_Txt);
				vTaskDelay(pdMS_TO_TICKS(3000));
			}
			else
			{
			xTaskCreate(setTime_task, "setTime_task", configMINIMAL_STACK_SIZE, (void*)UART_3, configMAX_PRIORITIES-2, NULL);
			vTaskSuspend(NULL);
			}
			break;
		case 4:
			if(NULL != xTaskGetHandle("setDate_task"))
			{
				UART_putString(UART_3, (uint8_t*) errorMes_Txt);
				vTaskDelay(pdMS_TO_TICKS(3000));
			}
			else
			{
			xTaskCreate(setDate_task, "setDate_task", configMINIMAL_STACK_SIZE, (void*)UART_3, configMAX_PRIORITIES-2, NULL);
			vTaskSuspend(NULL);
			}
			break;
		case 5:
			if(NULL != xTaskGetHandle("hourFormat_task"))
			{
				UART_putString(UART_3, (uint8_t*) errorMes_Txt);
				vTaskDelay(pdMS_TO_TICKS(3000));
			}
			else
			{
			xTaskCreate(hourFormat_task, "hourFormat_task", configMINIMAL_STACK_SIZE, (void*)UART_3, configMAX_PRIORITIES-2, NULL);
			vTaskSuspend(NULL);
			}
			break;
		case 6:
			if(NULL != xTaskGetHandle("timeTerminal_task"))
			{
				UART_putString(UART_3, (uint8_t*) errorMes_Txt);
				vTaskDelay(pdMS_TO_TICKS(3000));
			}
			else
			{
			xTaskCreate(timeTerminal_task, "timeTerminal_task", configMINIMAL_STACK_SIZE, (void*)UART_3, configMAX_PRIORITIES-2, NULL);
			vTaskSuspend(NULL);
			}
			break;
		case 7:
			if(NULL != xTaskGetHandle("dateTerminal_task"))
			{
				UART_putString(UART_3, (uint8_t*) errorMes_Txt);
				vTaskDelay(pdMS_TO_TICKS(3000));
			}
			else
			{
			xTaskCreate(dateTerminal_task, "dateTerminal_task", configMINIMAL_STACK_SIZE, (void*)UART_3, configMAX_PRIORITIES-2, NULL);
			vTaskSuspend(NULL);
			}
			break;
		case 8:
			xTaskCreate(chat_task, "chat2_task", configMINIMAL_STACK_SIZE + 300, (void*)UART_3, configMAX_PRIORITIES-2, NULL);
			vTaskSuspend(NULL);
			break;
		case 9:
			vTaskSuspend(timedateLCD_handle);
			if(NULL != xTaskGetHandle("echo_task"))
			{
				UART_putString(UART_3, (uint8_t*) errorMes_Txt);
				vTaskDelay(pdMS_TO_TICKS(3000));
			}
			else
			{
					xTaskCreate(echo_Task, "echo_task", configMINIMAL_STACK_SIZE, (void*)UART_3,
							configMAX_PRIORITIES - 2, NULL);
					vTaskSuspend(NULL);
			}
			break;
		default:
			UART_putString(UART_3, (uint8_t*)
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
			xEventGroupSetBits(timeTerminal_eventB, EVENT_TIME_SET);
		}
		else
		{
			ioerror = true;
			xEventGroupSetBits(getTime_eventB, EVENT_TIME_ERR);
			xEventGroupSetBits(timeTerminal_eventB, EVENT_TIME_ERR);
		}
		vTaskDelay(pdMS_TO_TICKS(500));
	}
}

void echo_Task(void * uart_module)
{
	echoTask_handle = xTaskGetCurrentTaskHandle();
	uint8_t recvBuffer;
	uint8_t maxChar = 0;

	UART_putString((UART_Module) uart_module, (uint8_t*) echo_menuTxt);

	xSemaphoreTake(spibus_mutex, portMAX_DELAY);
	LCDNokia_clear();
	xSemaphoreGive(spibus_mutex);

	while (1)
	{
		recvBuffer = UART_Echo((UART_Module) uart_module);

		if (ESC_KEY == recvBuffer)
		{
			xSemaphoreTake(spibus_mutex, portMAX_DELAY);
			LCDNokia_clear();
			xSemaphoreGive(spibus_mutex);
			vTaskResume(timedateLCD_handle);
			switch((UART_Module)uart_module)
			{
			case UART_0:
				vTaskResume(menu0Task_handle);
				break;
			case UART_3:
				vTaskResume(menu3Task_handle);
				break;
			}
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

void memread_task(void * uart_module){

	uint16_t subaddress = 0;
	uint16_t readlen = 0;
	uint8_t charAddress[4];
	uint8_t charlen[3];
	uint8_t charCounter = 0;
	uint8_t *recvBuff = malloc(sizeof(uint8_t));

	UART_putString((UART_Module)uart_module, (uint8_t*)memread_Txt);

	while(1){

		UART_putString((UART_Module)uart_module, (uint8_t*)"\r\nIngrese la direccion de memoria inicial: 0x");
		while(4 > charCounter)
		{
			charAddress[charCounter] = UART_Echo((UART_Module)uart_module);
			charCounter++;
		}
		charCounter = 0;
		subaddress = asciiToHex(charAddress);
		UART_putString((UART_Module)uart_module, (uint8_t*)"\r\nIngrese la cantidad de bytes a leer: ");
		while(2 > charCounter)
		{
			charlen[charCounter] = UART_Echo((UART_Module)uart_module);
			charCounter++;
		}
		charCounter = 0;
		readlen = asciitoDec(charlen);

		xSemaphoreTake(i2cbus_mutex, portMAX_DELAY);
		if(kStatus_Fail == I2C_MEMRead(I2C0, MEM_DEVICE_ADD, subaddress, recvBuff, readlen))
		{
			UART_putString((UART_Module)uart_module, (uint8_t*) "ERROR READING MEMORY");
		}
		else
		{
			UART_putString((UART_Module)uart_module, (uint8_t*)"\r\nDatos obtenidos de memoria: \r\n   -> ");
			UART_putBytes((UART_Module)uart_module, recvBuff, readlen);
		}
		xSemaphoreGive(i2cbus_mutex);
		UART_putString((UART_Module) uart_module, (uint8_t*)"\r\nPresiona cualquier tecla para salir...");
		UART_Echo((UART_Module)uart_module);
		switch((UART_Module)uart_module)
		{
		case UART_0:
			vTaskResume(menu0Task_handle);
			break;
		case UART_3:
			vTaskResume(menu3Task_handle);
			break;
		}
		vTaskDelete(NULL);
	}
}

void memWrite_task(void * uart_module)
{
	uint16_t subaddress = 0;
	uint8_t charAddress[4];
	uint8_t charCounter = 0;

	uint8_t maxWriteLenght = 100;
	uint8_t writeCounter = 0;

	uint8_t dataByteBuffer[maxWriteLenght];
	uint8_t dataByte;

	UART_putString((UART_Module)uart_module, (uint8_t*)memwrite_Txt);

	while(1)
	{

		UART_putString((UART_Module)uart_module, (uint8_t*)"\r\nIngrese la direccion de memoria inicial: 0x");
		while(4 > charCounter)
		{
			charAddress[charCounter] = UART_Echo((UART_Module)uart_module);
			charCounter++;
		}
		charCounter = 0;
		subaddress = asciiToHex(charAddress);
		UART_putString((UART_Module)uart_module, (uint8_t*)"\r\nIngrese los datos a escribir:\r\n");

		while(maxWriteLenght > writeCounter)
		{
			dataByte = UART_Echo((UART_Module)uart_module);
			if(ESC_KEY == dataByte)
			{
				switch((UART_Module)uart_module)
				{
				case UART_0:
					vTaskResume(menu0Task_handle);
					break;
				case UART_3:
					vTaskResume(menu3Task_handle);
					break;
				}
				vTaskDelete(NULL);
			}
			if(ENTER_KEY == dataByte)
			{
				UART_putString((UART_Module)uart_module, (uint8_t*)"\r\nEscribiendo en memoria...\r\n");
				xSemaphoreTake(i2cbus_mutex, portMAX_DELAY);
				if(kStatus_Fail == I2C_MEMWrite(I2C0, MEM_DEVICE_ADD, subaddress, dataByteBuffer, writeCounter))
				{
					UART_putString((UART_Module)uart_module, (uint8_t*) "\r\nError de escritura\r\n");
				}
				else
				{
					UART_putString((UART_Module)uart_module, (uint8_t*) "\r\nTransferencia realizada!\r\n");
				}
				xSemaphoreGive(i2cbus_mutex);
				UART_putString((UART_Module)uart_module, (uint8_t*)"\r\nSaliendo ...\r\n");
				vTaskDelay(pdMS_TO_TICKS(2000));
				switch((UART_Module)uart_module)
				{
				case UART_0:
					vTaskResume(menu0Task_handle);
					break;
				case UART_3:
					vTaskResume(menu3Task_handle);
					break;
				}
				vTaskDelete(NULL);
			}
			dataByteBuffer[writeCounter] = dataByte;
			writeCounter++;
		}
		writeCounter = 0;
	}
}

void setTime_task(void * uart_module)
{
	setTime_handle = xTaskGetCurrentTaskHandle();
	uint8_t charCounter = 0;
	uint8_t newTime[5];
	ascii_time_t *asciiDate;
	uint8_t timeFormat;

	UART_putString((UART_Module) uart_module, (uint8_t*)setTime_Txt);

	while(1)
	{
		do
		{
			xQueueReceive(g_time_queue, &asciiDate, portMAX_DELAY);
		} while (0 != uxQueueMessagesWaiting(g_time_queue));

		UART_putString((UART_Module) uart_module, (uint8_t*)"\r\nIntroduzca la hora en formato HH:MM --> ");
		while(4 > charCounter)
		{
			newTime[charCounter] = UART_Echo((UART_Module) uart_module);
			if(ESC_KEY == newTime[charCounter])
			{
				switch((UART_Module)uart_module)
				{
				case UART_0:
					vTaskResume(menu0Task_handle);
					break;
				case UART_3:
					vTaskResume(menu3Task_handle);
					break;
				}
				vTaskDelete(setTime_handle);
			}
			charCounter++;
		}
		charCounter = 0;
		if (FORMAT_12H == asciiDate->timeformat)
		{
			UART_putString((UART_Module) uart_module,
					(uint8_t*) "\r\nSe esta usando un formato de 12 Horas,\r\nindica si la nueva hora es AM o PM (a/p)");
			switch(UART_Echo((UART_Module) uart_module))
			{
			case 'a':
				timeFormat = FORMAT_AM;
				break;
			case 'p':
				timeFormat = FORMAT_PM;
				break;
			}
		}
		UART_putString((UART_Module) uart_module, (uint8_t*)"\r\nLa hora introducida es ");
		UART_putBytes((UART_Module) uart_module, &newTime[0], 1);
		UART_putBytes((UART_Module) uart_module, &newTime[1], 1);
		UART_putBytes((UART_Module) uart_module, (uint8_t*)":", 1);
		UART_putBytes((UART_Module) uart_module, &newTime[2], 1);
		UART_putBytes((UART_Module) uart_module, &newTime[3], 1);
		if (FORMAT_12H == asciiDate->timeformat)
		{
			switch(timeFormat)
			{
			case FORMAT_AM:
				UART_putString((UART_Module) uart_module, (uint8_t*)" am");
				break;
			case FORMAT_PM:
				UART_putString((UART_Module) uart_module, (uint8_t*)" pm");
			}
		}
		UART_putString((UART_Module) uart_module, (uint8_t*)"\r\nPresiona cualquier tecla para confirmar o ESC para cancelar...");
		if(ESC_KEY == UART_Echo((UART_Module) uart_module))
		{
			switch((UART_Module)uart_module)
			{
			case UART_0:
				vTaskResume(menu0Task_handle);
				break;
			case UART_3:
				vTaskResume(menu3Task_handle);
				break;
			}
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
			switch((UART_Module)uart_module)
			{
			case UART_0:
				vTaskResume(menu0Task_handle);
				break;
			case UART_3:
				vTaskResume(menu3Task_handle);
				break;
			}
			vTaskDelete(setTime_handle);
		}
	}

}

void setDate_task(void * uart_module)
{
	setDate_handle = xTaskGetCurrentTaskHandle();
	uint8_t charCounter = 0;
	uint8_t newDate[7];

	UART_putString((UART_Module) uart_module, (uint8_t*)setDate_Txt);

	while(1)
	{
		UART_putString((UART_Module) uart_module, (uint8_t*)"\r\nIntroduzca la fecha en formato DD/MM/AA --> ");
		while(6 > charCounter)
		{
			newDate[charCounter] = UART_Echo((UART_Module) uart_module);
			if(ESC_KEY == newDate[charCounter])
			{
				switch((UART_Module)uart_module)
				{
				case UART_0:
					vTaskResume(menu0Task_handle);
					break;
				case UART_3:
					vTaskResume(menu3Task_handle);
					break;
				}
				vTaskDelete(setDate_handle);
			}
			charCounter++;
		}
		charCounter = 0;
		UART_putString((UART_Module) uart_module, (uint8_t*)"\r\nLa fecha introducida es ");
		UART_putBytes((UART_Module) uart_module, &newDate[0], 1);
		UART_putBytes((UART_Module) uart_module, &newDate[1], 1);
		UART_putBytes((UART_Module) uart_module, (uint8_t*)"-", 1);
		UART_putBytes((UART_Module) uart_module, &newDate[2], 1);
		UART_putBytes((UART_Module) uart_module, &newDate[3], 1);
		UART_putBytes((UART_Module) uart_module, (uint8_t*)"-20", 1);
		UART_putBytes((UART_Module) uart_module, &newDate[4], 1);
		UART_putBytes((UART_Module) uart_module, &newDate[5], 1);
		UART_putString((UART_Module) uart_module, (uint8_t*)"\r\nPresiona cualquier tecla para confirmar o ESC para cancelar...");
		if(ESC_KEY == UART_Echo((UART_Module) uart_module))
		{
			switch((UART_Module)uart_module)
			{
			case UART_0:
				vTaskResume(menu0Task_handle);
				break;
			case UART_3:
				vTaskResume(menu3Task_handle);
				break;
			}
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
			switch((UART_Module)uart_module)
			{
			case UART_0:
				vTaskResume(menu0Task_handle);
				break;
			case UART_3:
				vTaskResume(menu3Task_handle);
				break;
			}
			vTaskDelete(setDate_handle);
		}
	}
}

void hourFormat_task(void * uart_module)
{
	hourFormat_handle = xTaskGetCurrentTaskHandle();
	ascii_time_t *asciiDate;

	UART_putString((UART_Module) uart_module, (uint8_t*) hourFormat_Txt);

	while (1)
	{
		do
		{
			xQueueReceive(g_time_queue, &asciiDate, portMAX_DELAY);
		} while (0 != uxQueueMessagesWaiting(g_time_queue));

		UART_putString((UART_Module) uart_module, (uint8_t*) "\r\nEl formato de hora actual es: ");
		switch (asciiDate->timeformat)
		{
		case FORMAT_24H:
			UART_putString((UART_Module) uart_module,
					(uint8_t*) "* 24 Horas *\r\n\nDeseas cambiarlo a 12 Horas? (y/n)");
			if ('y' == UART_Echo((UART_Module) uart_module))
			{
				xSemaphoreTake(i2cbus_mutex, portMAX_DELAY);
				I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCSEC, OSCILLATOR_OFF);
				I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCHOUR,
						((FORMAT_12H << 6)
								| (asciiDate->hours_h - ASCII_NUMBER_MASK) << 4
								| (asciiDate->hours_l - ASCII_NUMBER_MASK)));
				I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCSEC, OSCILLATOR_ON);
				xSemaphoreGive(i2cbus_mutex);
				UART_putString((UART_Module) uart_module,
						(uint8_t*) "\r\n\nEl formato ha sido cambiado, presiona una tecla para salir...");
				UART_Echo((UART_Module) uart_module);
				switch ((UART_Module) uart_module)
				{
				case UART_0:
					vTaskResume(menu0Task_handle);
					break;
				case UART_3:
					vTaskResume(menu3Task_handle);
					break;
				}
				vTaskDelete(hourFormat_handle);
			}
			else
			{
				UART_putString((UART_Module) uart_module,
						(uint8_t*) "\r\n\nEl formato permanece igual, presiona una tecla para salir...");
				UART_Echo((UART_Module) uart_module);
				switch((UART_Module)uart_module)
				{
				case UART_0:
					vTaskResume(menu0Task_handle);
					break;
				case UART_3:
					vTaskResume(menu3Task_handle);
					break;
				}
				vTaskDelete(hourFormat_handle);
			}
			break;
		case FORMAT_12H:
			UART_putString((UART_Module) uart_module,
					(uint8_t*) "* 12 Horas *\r\n\nDeseas cambiarlo a 24 Horas? (y/n)");
			if ('y' == UART_Echo((UART_Module) uart_module))
			{
				xSemaphoreTake(i2cbus_mutex, portMAX_DELAY);
				I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCSEC, OSCILLATOR_OFF);
				I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCHOUR,
						((FORMAT_24H << 6)
								| (asciiDate->hours_h - ASCII_NUMBER_MASK) << 4
								| (asciiDate->hours_l - ASCII_NUMBER_MASK)));
				I2C_Write(I2C0, RTC_DEVICE_ADD, REG_RTCSEC, OSCILLATOR_ON);
				xSemaphoreGive(i2cbus_mutex);
				UART_putString((UART_Module) uart_module,
						(uint8_t*) "\r\n\nEl formato ha sido cambiado, presiona una tecla para salir...");
				UART_Echo((UART_Module) uart_module);
				switch((UART_Module)uart_module)
				{
				case UART_0:
					vTaskResume(menu0Task_handle);
					break;
				case UART_3:
					vTaskResume(menu3Task_handle);
					break;
				}
				vTaskDelete(hourFormat_handle);
			}
			else
			{
				UART_putString((UART_Module) uart_module,
						(uint8_t*) "\r\n\nEl formato permanece igual, presiona una tecla para salir...");
				UART_Echo((UART_Module) uart_module);
				switch((UART_Module)uart_module)
				{
				case UART_0:
					vTaskResume(menu0Task_handle);
					break;
				case UART_3:
					vTaskResume(menu3Task_handle);
					break;
				}
				vTaskDelete(hourFormat_handle);
			}
			break;
		}
	}
}

void timeTerminal_task(void * uart_module)
{
	timeTerminal_handle = xTaskGetCurrentTaskHandle();
	ascii_time_t *asciiDate;

	UART_putString((UART_Module) uart_module, (uint8_t*) terminalTime_Txt);

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
			UART_putString((UART_Module) uart_module, (uint8_t*) terminalTime_Txt);
			UART_putString((UART_Module) uart_module,
					(uint8_t*) "\r\n\n --Error de lectura, verificar conexion al bus de I2C--");
		}
		else
		{
			do
			{
				xQueueReceive(g_time_queue, &asciiDate, portMAX_DELAY);
			} while (0 != uxQueueMessagesWaiting(g_time_queue));

			UART_putString((UART_Module) uart_module, (uint8_t*) terminalTime_Txt);
			UART_putString((UART_Module) uart_module, (uint8_t*) "\r\n\nLa hora actual es: \r\n\n"
					"      ");
			UART_putBytes((UART_Module) uart_module, &asciiDate->hours_h, 1);
			UART_putBytes((UART_Module) uart_module, &asciiDate->hours_l, 1);
			UART_putBytes((UART_Module) uart_module, (uint8_t*) ":", 1);
			UART_putBytes((UART_Module) uart_module, &asciiDate->minutes_h, 1);
			UART_putBytes((UART_Module) uart_module, &asciiDate->minutes_l, 1);
			UART_putBytes((UART_Module) uart_module, (uint8_t*) ":", 1);
			UART_putBytes((UART_Module) uart_module, &asciiDate->seconds_h, 1);
			UART_putBytes((UART_Module) uart_module, &asciiDate->seconds_l, 1);
			if (FORMAT_12H == asciiDate->timeformat)
			{
				switch (asciiDate->ampm)
				{
				case FORMAT_AM:
					UART_putString((UART_Module) uart_module,
							(uint8_t*) " am\r\n\n\n\nPresione cualquier tecla para salir");
					break;
				case FORMAT_PM:
					UART_putString((UART_Module) uart_module,
							(uint8_t*) " pm\r\n\n\n\nPresione cualquier tecla para salir");
					break;
				}
			}
			else
			{
				UART_putString((UART_Module) uart_module,
						(uint8_t*) "\r\n\n\n\n -- Presione cualquier tecla para salir --");
			}
			vPortFree(asciiDate);

			switch((UART_Module) uart_module)
			{
			case UART_0:
				xEventGroupSetBits(uart_interrupt_event, UART0_IRQ_ENABLE);
				break;
			case UART_3:
				xEventGroupSetBits(uart_interrupt_event, UART3_IRQ_ENABLE);
				break;
			}

			EventBits_t event = xEventGroupGetBits(uart_interrupt_event);
			if(UART0_RX_INTERRUPT_EVENT == (UART0_RX_INTERRUPT_EVENT & event) && (UART_Module)uart_module == UART_0)
			{
				xEventGroupClearBits(uart_interrupt_event,UART0_RX_INTERRUPT_EVENT);
				xEventGroupClearBits(uart_interrupt_event, UART0_IRQ_ENABLE);
				vTaskResume(menu0Task_handle);
				vTaskDelete(timeTerminal_handle);
			}
			if(UART3_RX_INTERRUPT_EVENT == (UART3_RX_INTERRUPT_EVENT & event) && (UART_Module)uart_module == UART_3)
			{
				xEventGroupClearBits(uart_interrupt_event,UART3_RX_INTERRUPT_EVENT);
				xEventGroupClearBits(uart_interrupt_event, UART3_IRQ_ENABLE);
				vTaskResume(menu3Task_handle);
				vTaskDelete(timeTerminal_handle);
			}
		}
	}
}

void dateTerminal_task(void * uart_module)
{
	dateTerminal_handle = xTaskGetCurrentTaskHandle();
	ascii_time_t *asciiDate;

	UART_putString((UART_Module) uart_module, (uint8_t*) terminalDate_Txt);

	while (1)
	{

		EventBits_t event = xEventGroupGetBits(timeTerminal_eventB);
		xEventGroupClearBits(timeTerminal_eventB,
				EVENT_TIME_ERR);
		if (EVENT_TIME_ERR == (event & EVENT_TIME_ERR))
		{
			UART_putString((UART_Module) uart_module, (uint8_t*) terminalDate_Txt);
			UART_putString((UART_Module) uart_module,
					(uint8_t*) "\r\n\n --Error de lectura, verificar conexion al bus de I2C--");
		}
		else
		{
			do
			{
				xQueueReceive(g_time_queue, &asciiDate, portMAX_DELAY);
			} while (0 != uxQueueMessagesWaiting(g_time_queue));

			UART_putString((UART_Module) uart_module, (uint8_t*) terminalDate_Txt);
			UART_putString((UART_Module) uart_module, (uint8_t*) "\r\n\nLa fecha de hoy es: \r\n\n"
					"      ");
			UART_putBytes((UART_Module) uart_module, &asciiDate->day_h, 1);
			UART_putBytes((UART_Module) uart_module, &asciiDate->day_l, 1);
			UART_putBytes((UART_Module) uart_module, (uint8_t*) "-", 1);
			UART_putBytes((UART_Module) uart_module, &asciiDate->month_h, 1);
			UART_putBytes((UART_Module) uart_module, &asciiDate->month_l, 1);
			UART_putString((UART_Module) uart_module, (uint8_t*) "-20");
			UART_putBytes((UART_Module) uart_module, &asciiDate->year_h, 1);
			UART_putBytes((UART_Module) uart_module, &asciiDate->year_l, 1);
			UART_putString((UART_Module) uart_module,
					(uint8_t*) "\r\n\n\n\n -- Presione cualquier tecla para salir --");
			vPortFree(asciiDate);

			switch((UART_Module) uart_module)
			{
			case UART_0:
				xEventGroupSetBits(uart_interrupt_event, UART0_IRQ_ENABLE);
				break;
			case UART_3:
				xEventGroupSetBits(uart_interrupt_event, UART3_IRQ_ENABLE);
				break;
			}

			EventBits_t event = xEventGroupGetBits(uart_interrupt_event);
			if(UART0_RX_INTERRUPT_EVENT == (UART0_RX_INTERRUPT_EVENT & event) && (UART_Module)uart_module == UART_0)
			{
				xEventGroupClearBits(uart_interrupt_event,UART0_RX_INTERRUPT_EVENT);
				xEventGroupClearBits(uart_interrupt_event, UART0_IRQ_ENABLE);
				vTaskResume(menu0Task_handle);
				vTaskDelete(timeTerminal_handle);
			}
			if(UART3_RX_INTERRUPT_EVENT == (UART3_RX_INTERRUPT_EVENT & event) && (UART_Module)uart_module == UART_3)
			{
				xEventGroupClearBits(uart_interrupt_event,UART3_RX_INTERRUPT_EVENT);
				xEventGroupClearBits(uart_interrupt_event, UART3_IRQ_ENABLE);
				vTaskResume(menu3Task_handle);
				vTaskDelete(timeTerminal_handle);
			}
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
	}

}

void chat_task(void * uart_module)
{
	//chatTask_handle = xTaskGetCurrentTaskHandle();
	chatBuffer_t *chatTxBuffer;
	chatBuffer_t *chatRxBuffer;
	uint8_t txCounter = 0;
	EventBits_t uart_event;

	UART_putString((UART_Module) uart_module, (uint8_t*)chat_Txt);

	chatTxBuffer = pvPortMalloc(sizeof(chatBuffer_t));

	switch((UART_Module)uart_module)
	{
	case UART_0:
		xEventGroupSetBits(uart_interrupt_event, UART0_IRQ_ENABLE);
		break;
	case UART_3:
		xEventGroupSetBits(uart_interrupt_event, UART3_IRQ_ENABLE);
		break;
	}

	while(1)
	{
		switch((UART_Module) uart_module)
		{
		case UART_0:
			;
			uart_event = xEventGroupWaitBits(uart_interrupt_event, UART0_RX_INTERRUPT_EVENT | MAIL_UART3, pdTRUE, pdFALSE, portMAX_DELAY);
			if(MAIL_UART3 == (MAIL_UART3 & uart_event))
			{
				chatRxBuffer = pvPortMalloc(sizeof(chatBuffer_t));
				do
				{
				xQueueReceive(g_uart3_queue,&chatRxBuffer,portMAX_DELAY);
				}while(0 != uxQueueMessagesWaiting(g_uart3_queue));
				UART_putString((UART_Module) uart_module, (uint8_t*)"\033[u\r\nTerminal 2 dice->  ");
				UART_putBytes((UART_Module) uart_module, (uint8_t*)chatRxBuffer->dataBuff, chatRxBuffer->dataLen);
				UART_putString((UART_Module) uart_module, (uint8_t*)"\033[s");
				//vPortFree(chatRxBuffer);
				//vPortFree(chatTxBuffer);
			}
			else
			{
				if(UART0_RX_INTERRUPT_EVENT == (UART0_RX_INTERRUPT_EVENT & uart_event))
				{
					if(0 == txCounter)
					{
						UART_putString((UART_Module) uart_module, (uint8_t*)"\033[22;2H");
					}
					UART_putBytes((UART_Module) uart_module, &uart0_irqData, 1);
					chatTxBuffer->dataBuff[txCounter] = uart0_irqData;
					chatTxBuffer->dataLen = txCounter;
					if(ESC_KEY == chatTxBuffer->dataBuff[txCounter])
					{
						txCounter = 0;
						xEventGroupClearBits(uart_interrupt_event, UART0_IRQ_ENABLE);
						xQueueReset(g_uart0_queue);
						vTaskResume(menu0Task_handle);
						vTaskDelete(NULL);
					}
					if(ENTER_KEY == chatTxBuffer->dataBuff[txCounter])
					{
						txCounter = 0;
						xQueueSend(g_uart0_queue, &chatTxBuffer, portMAX_DELAY);
						UART_putString((UART_Module) uart_module, (uint8_t*)"\033[u\r\nTu dices: ");
						UART_putBytes((UART_Module) uart_module, (uint8_t*)chatTxBuffer->dataBuff, chatTxBuffer->dataLen);
						UART_putString((UART_Module) uart_module, (uint8_t*)"\033[s\033[22;2H\033[2K\033[J");
						xEventGroupSetBits(uart_interrupt_event, MAIL_UART0);
						break;
					}
					txCounter++;
				}
			}
		break;

		case UART_3:
			;
			uart_event = xEventGroupWaitBits(uart_interrupt_event, UART3_RX_INTERRUPT_EVENT | MAIL_UART0, pdTRUE, pdFALSE, portMAX_DELAY);
			if(MAIL_UART0 == (MAIL_UART0 & uart_event))
			{
				chatRxBuffer = pvPortMalloc(sizeof(chatBuffer_t));
				do
				{
					xQueueReceive(g_uart0_queue,&chatRxBuffer,portMAX_DELAY);
				}while(0 != uxQueueMessagesWaiting(g_uart0_queue));
				UART_putString((UART_Module) uart_module, (uint8_t*)"\033[u\r\nTerminal 1 dice->  ");
				UART_putBytes((UART_Module) uart_module, (uint8_t*)chatRxBuffer->dataBuff, chatRxBuffer->dataLen);
				UART_putString((UART_Module) uart_module, (uint8_t*)"\033[s");
				//vPortFree(chatRxBuffer);
				//vPortFree(chatTxBuffer);
			}
			else
			{
				if(UART3_RX_INTERRUPT_EVENT == (UART3_RX_INTERRUPT_EVENT & uart_event))
				{
					if(0 == txCounter)
					{
						UART_putString((UART_Module) uart_module, (uint8_t*)"\033[22;2H");
					}
					UART_putBytes((UART_Module) uart_module, &uart3_irqData, 1);

					chatTxBuffer->dataBuff[txCounter] = uart3_irqData;
					chatTxBuffer->dataLen = txCounter;
					xQueueSend(g_uart3_queue, &chatTxBuffer, portMAX_DELAY);
					if(ESC_KEY == chatTxBuffer->dataBuff[txCounter])
					{
						txCounter = 0;
						xQueueReset(g_uart3_queue);
						xEventGroupClearBits(uart_interrupt_event, UART3_IRQ_ENABLE);
						vTaskResume(menu3Task_handle);
						vTaskDelete(NULL);
					}
					if(ENTER_KEY == chatTxBuffer->dataBuff[txCounter])
					{
						txCounter = 0;
						UART_putString((UART_Module) uart_module, (uint8_t*)"\033[u\r\nTu dices: ");
						UART_putBytes((UART_Module) uart_module, (uint8_t*)chatTxBuffer->dataBuff, chatTxBuffer->dataLen);
						UART_putString((UART_Module) uart_module, (uint8_t*)"\033[s\033[22;2H\033[2K\033[J");
						xEventGroupSetBits(uart_interrupt_event, MAIL_UART3);
						break;
					}
					txCounter++;
				}
			}
			break;
		}
	}
}
