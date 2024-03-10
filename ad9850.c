/*======================================
by chu

			VCC ----------- 3.3V
      GND ----------- GND
    RESET ----------- TP_MISO  (PA6)
    W_CLK ----------- TP_SCK   (PA5)
    FQ_UD ----------- NCS      (PG1)
       D7 ----------- TP_MOSI  (PA7)
======================================*/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "ad9850.h"

#define AD9850_SERIAL_EN // serial mode
#define AD9850_OSC_FREQ 125000000u	// external oscillator frequency

// define IO ports
#define DDS_RESET_PORT	GPIO_PORTA_BASE
#define DDS_RESET_PIN	GPIO_PIN_6
#define DDS_RESET_CLK	RCC_APB2Periph_GPIOA

#define DDS_W_CLK_PORT	GPIO_PORTA_BASE
#define DDS_W_CLK_PIN	GPIO_PIN_5
#define DDS_W_CLK_CLK	RCC_APB2Periph_GPIOA

#define DDS_FQ_UD_PORT	GPIO_PORTG_BASE
#define DDS_FQ_UD_PIN	GPIO_PIN_1
#define DDS_FQ_UD_CLK	RCC_APB2Periph_GPIOG

#define DDS_D7_PORT		GPIO_PORTA_BASE
#define DDS_D7_PIN		GPIO_PIN_7
#define DDS_D7_CLK		RCC_APB2Periph_GPIOA

static void AD9850_ResetToSerial(void);

/***********************************************************
function: AD9850_InitHard
usage: initiate AD9850, including GPIO and working mode
***********************************************************/
void AD9850_InitHard(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)){};
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOG)){};
  
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_6|GPIO_PIN_5|GPIO_PIN_7);
	GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_1);
	
	AD9850_ResetToSerial();
}

/*******************************************
subfunction: AD9850_ResetToSerial
used by: function: AD9850_InitHard
usage: reset AD9850 and set it serial mode
*******************************************/
static void AD9850_ResetToSerial(void)
{
	// set three pins to 0
	GPIOPinWrite(DDS_W_CLK_PORT, DDS_W_CLK_PIN, 0);
	GPIOPinWrite(DDS_FQ_UD_PORT, DDS_FQ_UD_PIN, 0);
	GPIOPinWrite(DDS_RESET_PORT, DDS_RESET_PIN, 0);
	
	// pulse on reset
	GPIOPinWrite(DDS_RESET_PORT, DDS_RESET_PIN, DDS_RESET_PIN);
	GPIOPinWrite(DDS_RESET_PORT, DDS_RESET_PIN, 0);

	// pulse on w_clk
	GPIOPinWrite(DDS_W_CLK_PORT, DDS_W_CLK_PIN, DDS_W_CLK_PIN);
	GPIOPinWrite(DDS_W_CLK_PORT, DDS_W_CLK_PIN, 0);

	// pulse on fq_ud
	GPIOPinWrite(DDS_FQ_UD_PORT, DDS_FQ_UD_PIN, DDS_FQ_UD_PIN);
	GPIOPinWrite(DDS_FQ_UD_PORT, DDS_FQ_UD_PIN, 0);
}

/*************************************************************
function: AD9850_WriteCmd
argument: ucPhase: output phase, _dOutFreq: output frequency
usage: send 40 bits command align with serial protocol
*************************************************************/
void AD9850_WriteCmd(uint8_t _ucPhase, double _dOutFreq)
{
	uint32_t ulFreqWord;
	uint8_t i;
	// calculate frequency word from input argument
	ulFreqWord = (uint32_t)((_dOutFreq * 4294967295u) / AD9850_OSC_FREQ);
	// send bits[0:31], from low to high
	for (i = 0; i < 32; i++)
	{
		if (ulFreqWord & 0x00000001)
		{
			GPIOPinWrite(DDS_D7_PORT, DDS_D7_PIN, DDS_D7_PIN);
		}
		else
		{
			GPIOPinWrite(DDS_D7_PORT, DDS_D7_PIN, 0);
		}
		ulFreqWord >>= 1;
		GPIOPinWrite(DDS_W_CLK_PORT, DDS_W_CLK_PIN, DDS_W_CLK_PIN);
		GPIOPinWrite(DDS_W_CLK_PORT, DDS_W_CLK_PIN, 0);
	}
	// send bits[32:39], from low to high
	for (i = 0; i < 8; i++)
	{
		if (_ucPhase & 0x00000001)
		{
			GPIOPinWrite(DDS_D7_PORT, DDS_D7_PIN, DDS_D7_PIN);
		}
		else
		{
			GPIOPinWrite(DDS_D7_PORT, DDS_D7_PIN, 0);
		}
		_ucPhase >>= 1;
		GPIOPinWrite(DDS_W_CLK_PORT, DDS_W_CLK_PIN, DDS_W_CLK_PIN);
		GPIOPinWrite(DDS_W_CLK_PORT, DDS_W_CLK_PIN, 0);
	}
	// flush AD9850's buffer
	GPIOPinWrite(DDS_FQ_UD_PORT, DDS_FQ_UD_PIN, DDS_FQ_UD_PIN);
	GPIOPinWrite(DDS_FQ_UD_PORT, DDS_FQ_UD_PIN, 0);
}
