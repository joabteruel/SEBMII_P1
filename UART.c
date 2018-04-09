/*
 * UART.c
 *
 *  Created on: Feb 27, 2017
 *      Author: joab
 */

#include "UART.h"

//size_t n;

uint8_t uart0_background_buffer[32];
uint8_t uart3_background_buffer[32];
uint8_t uart0_inBuffer[1];
uint8_t uart3_inBuffer[1];

//EventGroupHandle_t uart_interrupt_event;

uart_rtos_handle_t uart0_handler;
uart_rtos_handle_t uart3_handler;



struct _uart_handle t_handle;
struct _uart_handle t_handle_b;



uart_rtos_config_t uart_config =
{ .base = UART0, .baudrate = UART0_BAUDRATE, .parity = kUART_ParityDisabled,
		.stopbits = kUART_OneStopBit, .buffer = uart0_background_buffer,
		.buffer_size = sizeof(uart0_background_buffer), };

uart_rtos_config_t uart_config_b =
{ .base = UART3, .baudrate = UART3_BAUDRATE, .parity = kUART_ParityDisabled,
		.stopbits = kUART_OneStopBit, .buffer = uart3_background_buffer,
		.buffer_size = sizeof(uart3_background_buffer), };

void uart_init(void)
{
	NVIC_SetPriority(UART0_RX_TX_IRQn, 5);
	NVIC_SetPriority(UART3_RX_TX_IRQn, 5);
	uart_config.srcclk = CLOCK_GetFreq(UART0_CLK_SRC);
	UART_RTOS_Init(&uart0_handler, &t_handle, &uart_config);
	uart_config_b.srcclk = CLOCK_GetFreq(UART3_CLK_SRC);
	UART_RTOS_Init(&uart3_handler, &t_handle_b, &uart_config_b);
	//uart_interrupt_event = xEventGroupCreate();

	UART_EnableInterrupts(UART0, kUART_RxDataRegFullInterruptEnable | kUART_RxOverrunInterruptEnable);
	EnableIRQ(UART0_IRQn);
	UART_EnableInterrupts(UART3, kUART_RxDataRegFullInterruptEnable | kUART_RxOverrunInterruptEnable);
	EnableIRQ(UART3_IRQn);
}

void UART_putBytes(UART_Module module, uint8_t *data, size_t numBytes){
	if(UART_0 == module)
	{
		UART_RTOS_Send(&uart0_handler, data, numBytes);
	}
	if(UART_3 == module)
	{
		UART_RTOS_Send(&uart3_handler, data, numBytes);
	}
}

void UART_putString(UART_Module module, uint8_t *string)
{
	if (UART_0 == module)
	{
		while (*string)
		{
			UART_RTOS_Send(&uart0_handler, string++, 1);
		}
	}
	if (UART_3 == module)
	{
		while (*string)
		{
			UART_RTOS_Send(&uart3_handler, string++, 1);
		}
	}

}


uint8_t UART_Echo(UART_Module module){
	size_t dataSize;
	volatile uint8_t dataInBuffer;
	uart_rtos_handle_t handle;
	uint8_t recv_buffer[1];

	if(UART_0 == module)
	{
		handle = uart0_handler;
	}
	if(UART_3 == module)
	{
		handle = uart3_handler;
	}

	/* Send data */
	UART_RTOS_Receive(&handle, recv_buffer, sizeof(recv_buffer), &dataSize);
	if (dataSize > 0)
	{
		/* Echo the received data */
		UART_RTOS_Send(&handle, (uint8_t *) recv_buffer, dataSize);
		dataInBuffer = recv_buffer[0];
		return dataInBuffer;
	}
	else
		return 1; //Warning avoidance
}


