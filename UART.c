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

uart_rtos_handle_t uart0_handler;
struct _uart_handle t_handle;

uart_rtos_handle_t uart3_handler;
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

	uart_config.srcclk = CLOCK_GetFreq(UART0_CLK_SRC);
	UART_RTOS_Init(&uart0_handler, &t_handle, &uart_config);

	uart_config_b.srcclk = CLOCK_GetFreq(UART3_CLK_SRC);
	UART_RTOS_Init(&uart3_handler, &t_handle_b, &uart_config_b);
}

void UART_userSend(uint8_t *data, size_t dataSize)
{
	UART_RTOS_Send(&uart0_handler, data, dataSize);
}

uint8_t UART_Echo(void)
{
	size_t dataSize;
	volatile uint8_t dataInBuffer;

	/* Send data */
	UART_RTOS_Receive(&uart0_handler, uart0_inBuffer, sizeof(uart0_inBuffer),
			&dataSize);
	if (dataSize > 0)
	{
		/* Echo the received data */
		UART_RTOS_Send(&uart0_handler, (uint8_t *) uart0_inBuffer, dataSize);
		dataInBuffer = uart0_inBuffer[0];
		return dataInBuffer;
	}
}

uart_rtos_handle_t *getHandleUART0()
{
	return &uart0_handler;
}

uart_rtos_handle_t* getHandleUART3()
{
	return &uart3_handler;
}

