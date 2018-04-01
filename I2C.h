/*
 * I2C.h
 *
 *  Created on: Mar 5, 2018
 *      Author: joab
 */

#ifndef I2C_H_
#define I2C_H_

#include "fsl_i2c.h"
#include "fsl_port.h"
#include "fsl_gpio.h"

#define I2C_BAUDRATE 100000U


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void i2c_release_bus_delay(void);
void i2c_ReleaseBus();
void i2c_init(void);
static void i2c_master_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void * userData);
void I2C_Read(I2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t *rxBuff, uint32_t rxSize);
void I2C_Write(I2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t value);
void I2C_MEMRead(I2C_Type *base, uint8_t device_addr, uint16_t reg_addr, uint8_t *rxBuff, uint32_t rxSize);
void I2C_MEMWrite(I2C_Type *base, uint8_t device_addr, uint16_t reg_addr, uint8_t *txBuff, uint32_t txSize);





#endif /* I2C_H_ */
