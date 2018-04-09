/*
 * The Clear BSD License
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 * that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
 !!GlobalInfo
 product: Pins v3.0
 processor: MK64FN1M0xxx12
 package_id: MK64FN1M0VLL12
 mcu_data: ksdk2_0
 processor_version: 2.0.0
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

#include "fsl_common.h"
#include "fsl_port.h"
#include "pin_mux.h"
#include "fsl_gpio.h"

#define PIN0_IDX                         0u   /*!< Pin number for pin 0 in a port */
#define PIN1_IDX                         1u   /*!< Pin number for pin 1 in a port */
#define PIN2_IDX                         2u   /*!< Pin number for pin 2 in a port */
#define PIN3_IDX                         3u   /*!< Pin number for pin 3 in a port */
#define PIN4_IDX                         4u   /*!< Pin number for pin 4 in a port */
#define PIN5_IDX                         5u   /*!< Pin number for pin 5 in a port */
#define PIN6_IDX                         6u   /*!< Pin number for pin 6 in a port */
#define PIN7_IDX                         7u   /*!< Pin number for pin 7 in a port */
#define PIN10_IDX 						10u /*!< Pin number for pin 10 in a port */
#define PIN11_IDX 						11u /*!< Pin number for pin 11 in a port */
#define PIN16_IDX                       16u   /*!< Pin number for pin 16 in a port */
#define PIN17_IDX                       17u   /*!< Pin number for pin 17 in a port */
#define PIN24_IDX 						24u /*!< Pin number for pin 24 in a port */
#define PIN25_IDX 						25u /*!< Pin number for pin 25 in a port */
#define SOPT5_UART0TXSRC_UART_TX      0x00u   /*!< UART 0 transmit data source select: UART0_TX pin */

/*FUNCTION**********************************************************************
 * 
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 * 
 *END**************************************************************************/
void BOARD_InitBootPins(void)
{
	BOARD_InitPins();
}

/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
 BOARD_InitPins:
 - options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}
 - pin_list:
 - {pin_num: '62', peripheral: UART0, signal: RX, pin_signal: PTB16/SPI1_SOUT/UART0_RX/FTM_CLKIN0/FB_AD17/EWM_IN}
 - {pin_num: '63', peripheral: UART0, signal: TX, pin_signal: PTB17/SPI1_SIN/UART0_TX/FTM_CLKIN1/FB_AD16/EWM_OUT_b}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */

/*FUNCTION**********************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 *END**************************************************************************/
void BOARD_InitPins(void)
{
	gpio_pin_config_t port_config =
	{ kGPIO_DigitalOutput, 0, };

	CLOCK_EnableClock(kCLOCK_PortB); /* Port B Clock Gate Control: Clock enabled */
	CLOCK_EnableClock(kCLOCK_PortD); /* Port D Clock Gate Control: Clock enabled */
	CLOCK_EnableClock(kCLOCK_PortE); /* Port E Clock Gate Control: Clock enabled */
	CLOCK_EnableClock(kCLOCK_PortC); /* Port E Clock Gate Control: Clock enabled */
	CLOCK_EnableClock(kCLOCK_I2c0);


	GPIO_PinInit(GPIOD, PIN0_IDX, &port_config);
	GPIO_PinInit(GPIOD, PIN3_IDX, &port_config);

	port_pin_config_t config_i2c =
	{ kPORT_PullDisable, kPORT_SlowSlewRate, kPORT_PassiveFilterDisable,
	        kPORT_OpenDrainDisable, kPORT_LowDriveStrength, kPORT_MuxAlt5,
	        kPORT_UnlockRegister, };

	PORT_SetPinConfig(PORTE, PIN24_IDX, &config_i2c);
	PORT_SetPinConfig(PORTE, PIN25_IDX, &config_i2c);

	PORT_SetPinMux(PORTB, PIN16_IDX, kPORT_MuxAlt3); /* PORTB16 (pin E10) is configured as UART0_RX */
	PORT_SetPinMux(PORTB, PIN17_IDX, kPORT_MuxAlt3); /* PORTB17 (pin E9) is configured as UART0_TX */
	PORT_SetPinMux(PORTC, PIN16_IDX, kPORT_MuxAlt3); /* PORTB10 (pin 62) is configured as UART3_RX */
	PORT_SetPinMux(PORTC, PIN17_IDX, kPORT_MuxAlt3); /* PORTB11 (pin 63) is configured as UART3_TX */
	PORT_SetPinMux(PORTD, PIN0_IDX, kPORT_MuxAsGpio); /* PORTD0 (pin A5) is configured as SPI0_PCS0 */
	PORT_SetPinMux(PORTD, PIN1_IDX, kPORT_MuxAlt2); /* PORTD1 (pin D4) is configured as SPI0_SCK */
	PORT_SetPinMux(PORTD, PIN2_IDX, kPORT_MuxAlt2); /* PORTD2 (pin C4) is configured as SPI0_SOUT */
	PORT_SetPinMux(PORTD, PIN3_IDX, kPORT_MuxAsGpio); /* PORTD3 (pin B4) is configured as SPI0_SIN */
	PORT_SetPinMux(PORTE, PIN24_IDX, kPORT_MuxAlt5); /* PORTE24 (pin 31) is configured as I2C0_SCL */
	PORT_SetPinMux(PORTE, PIN25_IDX, kPORT_MuxAlt5); /* PORTE25 (pin 32) is configured as I2C0_SDA */
	SIM->SOPT5 = ((SIM->SOPT5 & (~(SIM_SOPT5_UART0TXSRC_MASK))) /* Mask bits to zero which are setting */
	| SIM_SOPT5_UART0TXSRC(SOPT5_UART0TXSRC_UART_TX) /* UART 0 transmit data source select: UART0_TX pin */
	);

	  port_pin_config_t config_led =
	{ kPORT_PullDisable, kPORT_SlowSlewRate, kPORT_PassiveFilterDisable,
	kPORT_OpenDrainDisable, kPORT_LowDriveStrength, kPORT_MuxAsGpio,
	kPORT_UnlockRegister, };

	gpio_pin_config_t led_config_gpio =
	{ kGPIO_DigitalOutput, 1 };

	PORT_SetPinConfig(PORTB, 21, &config_led);
	GPIO_PinInit(GPIOB, 21, &led_config_gpio);

}

/*******************************************************************************
 * EOF
 ******************************************************************************/
