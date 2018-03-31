/*
 * I2C.h
 *
 *  Created on: Mar 5, 2018
 *      Author: joab
 */

#ifndef I2C_H_
#define I2C_H_

#include "fsl_i2c.h"

#define I2C_BAUDRATE 100000U


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
uint8_t i2c_init(void);
void i2c_master_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData);
bool I2C_Read(I2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t *rxBuff, uint32_t rxSize);
bool I2C_Write(I2C_Type *base, uint8_t device_addr, uint8_t reg_addr, uint8_t value);
bool I2C_MEMRead(I2C_Type *base, uint8_t device_addr, uint16_t reg_addr, uint8_t *rxBuff, uint32_t rxSize);
bool I2C_MEMWrite(I2C_Type *base, uint8_t device_addr, uint16_t reg_addr, uint8_t *txBuff, uint32_t txSize);





#endif /* I2C_H_ */
