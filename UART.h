/*
 * UART.h
 *
 *  Created on: Feb 27, 2017
 *      Author: joab
 */

#ifndef SOURCE_UART_H_
#define SOURCE_UART_H_

#include "fsl_uart_freertos.h"
#include "fsl_uart.h"
#include "pin_mux.h"
#include "clock_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define UART0_BAUDRATE 115200U
#define UART3_BAUDRATE 115200U

/*******************************************************************************
 * Data Types
 ******************************************************************************/
typedef enum
{
	UART_0, UART_3
} UART_Module;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
uint8_t UART_Echo(void);
void uart_init(void);
void UART_putString(UART_Module module, uint8_t *string);
void UART_putBytes(UART_Module module, uint8_t *data, size_t numBytes);
uart_rtos_handle_t *getHandleUART0();
uart_rtos_handle_t* getHandleUART3();

#endif /* SOURCE_UART_H_ */
