/*
 * I2C.c
 *
 *  Created on: Mar 5, 2018
 *      Author: joab
 */

#include "I2C.h"

i2c_master_handle_t i2c_handle;
SemaphoreHandle_t i2cbus_mutex_t;

volatile bool completionFlag = false;
volatile bool nackFlag = false;

static void i2c_master_callback(I2C_Type *base, i2c_master_handle_t *handle,
        status_t status, void * userData)
{
	if (status == kStatus_I2C_Addr_Nak)
	{
		nackFlag = true;
	}
	if (status == kStatus_Success)
	{
		completionFlag = true;
	}
}

void i2c_release_bus_delay(void)
{
	uint32_t i = 0;
	for (i = 0; i < 100; i++)
	{
		__NOP();
	}
}

void i2c_ReleaseBus()
{
	uint8_t i = 0;
	gpio_pin_config_t pin_config;
	port_pin_config_t i2c_pin_config =
	{ 0 };

	/* Config pin mux as gpio */
	i2c_pin_config.pullSelect = kPORT_PullUp;
	i2c_pin_config.mux = kPORT_MuxAsGpio;

	pin_config.pinDirection = kGPIO_DigitalOutput;
	pin_config.outputLogic = 1U;
	CLOCK_EnableClock(kCLOCK_PortE);
	PORT_SetPinConfig(PORTE, 24, &i2c_pin_config);
	PORT_SetPinConfig(PORTE, 25, &i2c_pin_config);

	GPIO_PinInit(GPIOE, 24, &pin_config);
	GPIO_PinInit(GPIOE, 25, &pin_config);

	GPIO_PinWrite(GPIOE, 25, 0U);
	i2c_release_bus_delay();

	for (i = 0; i < 9; i++)
	{
		GPIO_PinWrite(GPIOE, 24, 0U);
		i2c_release_bus_delay();

		GPIO_PinWrite(GPIOE, 25, 1U);
		i2c_release_bus_delay();

		GPIO_PinWrite(GPIOE, 24, 1U);
		i2c_release_bus_delay();
		i2c_release_bus_delay();
	}

	GPIO_PinWrite(GPIOE, 24, 0U);
	i2c_release_bus_delay();

	GPIO_PinWrite(GPIOE, 25, 0U);
	i2c_release_bus_delay();

	GPIO_PinWrite(GPIOE, 24, 1U);
	i2c_release_bus_delay();

	GPIO_PinWrite(GPIOE, 25, 1U);
	i2c_release_bus_delay();
}

void i2c_init()
{

	i2c_master_config_t masterConfig;
	I2C_MasterGetDefaultConfig(&masterConfig);
	masterConfig.baudRate_Bps = I2C_BAUDRATE;
	I2C_MasterInit(I2C0, &masterConfig, CLOCK_GetFreq(kCLOCK_BusClk));
	I2C_MasterTransferCreateHandle(I2C0, &i2c_handle, i2c_master_callback,
			NULL);
	i2cbus_mutex_t = xSemaphoreCreateMutex();
}

status_t I2C_MEMRead(I2C_Type *base, uint8_t device_addr, uint16_t reg_addr,
		uint8_t *rxBuff, uint32_t rxSize)
{
	i2c_master_transfer_t masterXfer;
	memset(&masterXfer, 0, sizeof(masterXfer));
	masterXfer.slaveAddress = device_addr;
	masterXfer.direction = kI2C_Read;
	masterXfer.subaddress = reg_addr;
	masterXfer.subaddressSize = 2;
	masterXfer.data = rxBuff;
	masterXfer.dataSize = rxSize;
	masterXfer.flags = kI2C_TransferDefaultFlag;

	xSemaphoreTake(i2cbus_mutex_t, portMAX_DELAY);
	I2C_MasterTransferNonBlocking(I2C0, &i2c_handle, &masterXfer);
	while (!completionFlag)
	{
		if (nackFlag)
		{
			nackFlag = false;
			return kStatus_Fail;
		}
	}
	completionFlag = false;
	xSemaphoreGive(i2cbus_mutex_t);
	return kStatus_Success;
}

status_t I2C_MEMWrite(I2C_Type *base, uint8_t device_addr, uint16_t reg_addr,
		uint8_t *txBuff, uint32_t txSize)
{
	i2c_master_transfer_t masterXfer;
	memset(&masterXfer, 0, sizeof(masterXfer));
	masterXfer.slaveAddress = device_addr;
	masterXfer.direction = kI2C_Write;
	masterXfer.subaddress = reg_addr;
	masterXfer.subaddressSize = 2;
	masterXfer.data = txBuff;
	masterXfer.dataSize = txSize;
	masterXfer.flags = kI2C_TransferDefaultFlag;

	xSemaphoreTake(i2cbus_mutex_t, portMAX_DELAY);
	I2C_MasterTransferNonBlocking(I2C0, &i2c_handle, &masterXfer);
	while (!completionFlag)
	{
		if (nackFlag)
		{
			nackFlag = false;
			return kStatus_Fail;
		}
	}
	completionFlag = false;
	xSemaphoreGive(i2cbus_mutex_t);
	return kStatus_Success;
}

status_t I2C_Read(I2C_Type *base, uint8_t device_addr, uint8_t reg_addr,
		uint8_t *rxBuff, uint32_t rxSize)
{
	i2c_master_transfer_t masterXfer;
	memset(&masterXfer, 0, sizeof(masterXfer));
	masterXfer.slaveAddress = device_addr;
	masterXfer.direction = kI2C_Read;
	masterXfer.subaddress = reg_addr;
	masterXfer.subaddressSize = 1;
	masterXfer.data = rxBuff;
	masterXfer.dataSize = rxSize;
	masterXfer.flags = kI2C_TransferDefaultFlag;

	xSemaphoreTake(i2cbus_mutex_t, portMAX_DELAY);
	I2C_MasterTransferNonBlocking(I2C0, &i2c_handle, &masterXfer);
	while (!completionFlag)
	{
		if (nackFlag)
		{
			nackFlag = false;
			return kStatus_Fail;
		}
	}
	completionFlag = false;
	xSemaphoreGive(i2cbus_mutex_t);
	return kStatus_Success;
}

status_t I2C_Write(I2C_Type *base, uint8_t device_addr, uint8_t reg_addr,
		uint8_t value)
{
	i2c_master_transfer_t masterXfer;

	masterXfer.slaveAddress = device_addr;
	masterXfer.direction = kI2C_Write;
	masterXfer.subaddress = reg_addr;
	masterXfer.subaddressSize = 1;
	masterXfer.data = &value;
	masterXfer.dataSize = 1;
	masterXfer.flags = kI2C_TransferDefaultFlag;

	xSemaphoreTake(i2cbus_mutex_t, portMAX_DELAY);
	I2C_MasterTransferNonBlocking(I2C0, &i2c_handle, &masterXfer);
	while (!completionFlag)
	{
		if (nackFlag)
		{
			nackFlag = false;
			return kStatus_Fail;
		}
	}
	completionFlag = false;
	xSemaphoreGive(i2cbus_mutex_t);
	return kStatus_Success;
}

