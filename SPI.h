/*
 * SPI.h
 *
 *  Created on: Feb 27, 2017
 *      Author: joab
 */

#ifndef SOURCE_SPI_H_
#define SOURCE_SPI_H_

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_dspi.h"
#include "board.h"

#include "pin_mux.h"
#include "clock_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define TRANSFER_BAUDRATE 5000000U /*! Transfer baudrate - 1MHz */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void spi_task(void *pvParameters);
uint8_t spi_init(void);
void DSPI_MasterUserCallback(SPI_Type *base, dspi_master_handle_t *handle, status_t status, void *userData);
void spiSendByte(uint8_t data);


#endif /* SOURCE_SPI_H_ */
