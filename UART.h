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
#define UART0_RX_INTERRUPT_EVENT (1<<0)
#define UART3_RX_INTERRUPT_EVENT (1<<1)
#define UART0_IRQ_ENABLE (1<<2)
#define UART3_IRQ_ENABLE (1<<3)

#define UART0_IRQHandler UART0_RX_TX_IRQHandler
#define UART3_IRQHandler UART3_RX_TX_IRQHandler
#define UART0_IRQn UART0_RX_TX_IRQn
#define UART3_IRQn UART3_RX_TX_IRQn


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
uint8_t UART_Echo(UART_Module module);
void uart_init(void);
void UART_putString(UART_Module module, uint8_t *string);
void UART_putBytes(UART_Module module, uint8_t *data, size_t numBytes);
//void UART0_RX_TX_DriverIRQHandler();


#endif /* SOURCE_UART_H_ */
