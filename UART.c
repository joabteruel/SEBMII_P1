/*
 * UART.c
 *
 *  Created on: Apr 1, 2018
 *      Author: joab
 */

#include "UART.h"

/* Global handlers definitions*/
uart_handle_t uart0_handle, uart3_handle;
volatile bool txFinished = false;
volatile bool rxFinished = false;
uart_transfer_t txBuffer;
uart_transfer_t rxBuffer;


/* Global callback definition */
static void uart_Callback(UART_Type *base, uart_handle_t *handle, status_t status,
		void *userData)
{
	if (kStatus_UART_TxIdle == status)
	{
		txFinished = true;
	}
	if (kStatus_UART_RxIdle == status)
	{
		rxFinished = true;
	}
}

/* Global peripherals initialization definitions */
void uart_ConfigInit()
{
	uart_config_t uart_config;

	UART_GetDefaultConfig(&uart_config);
	uart_config.baudRate_Bps = UART_BAUDRATE;
	uart_config.enableTx = true;
	uart_config.enableRx = true;

	UART_Init(UART0, &uart_config, CLOCK_GetFreq(kCLOCK_BusClk));
	UART_TransferCreateHandle(UART0, &uart0_handle, uart_Callback, NULL);
	NVIC_SetPriority(UART0_RX_TX_IRQn, 5);

	UART_Init(UART3, &uart_config, CLOCK_GetFreq(kCLOCK_BusClk));
	UART_TransferCreateHandle(UART3, &uart3_handle, uart_Callback, NULL);
	NVIC_SetPriority(UART3_RX_TX_IRQn, 5);
}

void uart_txData(UART_Type *base, uint8_t *data, size_t datasize)
{
	txBuffer.data = data;
	txBuffer.dataSize = datasize;
	if(base == UART0)
	{
		UART_TransferSendNonBlocking(base, &uart0_handle, &txBuffer);
	}
	if(base == UART3)
	{
		UART_TransferSendNonBlocking(base, &uart3_handle, &txBuffer);
	}
	while (!txFinished)
	{
	}
	txFinished = false;

}
