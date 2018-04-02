/*
 * UART.h
 *
 *  Created on: Apr 1, 2018
 *      Author: joab
 */

#ifndef UART_H_
#define UART_H_

#include "fsl_uart.h"

#define UART_BAUDRATE 115200

void uart_ConfigInit();
void uart_txData(UART_Type *base, uint8_t *data, size_t datasize);
static void uart_Callback(UART_Type *base, uart_handle_t *handle, status_t status, void *userData);


#endif /* UART_H_ */
