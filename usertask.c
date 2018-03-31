/*
 * usertask.c
 *
 *  Created on: Mar 13, 2018
 *      Author: Joab
 */

#include "usertask.h"

/* Global handlers definitions*/
uart_handle_t uartHandle;

/* Global callbacks definitions */
uart_transfer_callback_t uartCallback;

/* Global peripherals initialization definitions */
uint8_t uart_configInit()
{
	uart_config_t config;

	CLOCK_EnableClock(kCLOCK_PortB);
	PORT_SetPinMux(PORTB, 16, kPORT_MuxAlt3);
	PORT_SetPinMux(PORTB, 17, kPORT_MuxAlt3);
	UART_GetDefaultConfig(&config);

	config.baudRate_Bps = 115200;
	config.enableTx = true;
	config.enableRx = true;

	UART_Init(UART0, &config, CLOCK_GetFreq(UART0_CLK_SRC));
	UART_TransferCreateHandle(UART0, &uartHandle, uartCallback, NULL);
	NVIC_SetPriority(UART0_RX_TX_IRQn, 5);
	return 0;
}
