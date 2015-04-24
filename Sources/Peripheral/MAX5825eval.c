/*
 * MAX5825eval.c
 *
 *  Created on: Apr 5, 2015
 *      Author: ThreeBoys
 */


/*
 * MAX5825eval.c
 *
 *  Created on: Mar 13, 2015
 *      Author: TBiberdorf
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "Cpu.h"
#include "FreeRTOS.h"
#include "task.h"
#include "i2cInterface.h"
#include "MAX5825eval.h"


/******************************************************************************
 * MACROs/variables
 */
#define DAC_WR_SAMPLE_RATE (172)
#define PIN0 (1<<0)
#define PIN1 (1<<1)
#define PIN2 (1<<2)
#define PIN3 (1<<3)
#define PIN4 (1<<4)
#define PIN11 (1<<11)
#define PIT0_PULSE_TIMER (PIT_TCTRL0) /**< timer control register */
#define PIT0_TIMER_DELAY (PIT_LDVAL0) /**< timeout period calculated by (Period * 48Mhz) */

typedef enum _MAX5825Cmds_e
{
	eMAX5825idle,
	eResetMAX5825,
	eSWClrMAX5825,
	eLoadCODEMAX5825,
	eCode2DACMAX5825,
	eMAX5825GenWave,
	eMAX5825StopGenWave,
}eMAX5825Cmds_t;

#define MAX5825_RESET_REG		(0x35)
#define MAX5825_SW_CLR_REG		(0x34)
#define MAX5825_SET_CODE_REG	(0x80)
#define MAX5825_LOAD_DAC_REG	(0x90)
#define MAX5825_LOAD_ALLDAC_REG	(0xC1)


const char *gMAX5825evalTaskname = "MAX5825 Interface Task";
#define MAX5825_I2C_ADDRESS                         0x10
#define MAX5825_CMD_BLOCK_SIZE (3)
eMAX5825Cmds_t gMAX5825cmd = eMAX5825idle;
uint8_t gMAX5825Data = 0;

typedef struct sDAC
{
	uint8_t channel;
	uint16_t value;
}sDAC_t;

sDAC_t gDACCmd;

/**
 * ============================================================================
 * Private APIs
 *=============================================================================
 */

/**
 * @details reset the DAC module
 */
static void resetMAX5825()
{
	uint8_t cmdBlock[MAX5825_CMD_BLOCK_SIZE] = {MAX5825_RESET_REG,0x96,0x30};
	printf("resetMAX5825\r\n");

	i2cWriteData(MAX5825_I2C_ADDRESS,cmdBlock,MAX5825_CMD_BLOCK_SIZE);
}

/**
 * @details perform a SW clear of DAC module registers
 */
static void swClrMAX5825()
{
	uint8_t cmdBlock[MAX5825_CMD_BLOCK_SIZE] = {MAX5825_SW_CLR_REG,0x96,0x30};
	printf("swClrMAX5825\r\n");

	i2cWriteData(MAX5825_I2C_ADDRESS,cmdBlock,MAX5825_CMD_BLOCK_SIZE);
}

/**
 * @details write to the CODE register of the targeted DAC channel
 */
static void setCodeMAX5825(uint8_t DACAddr,uint16_t Data)
{
	uint8_t cmdBlock[MAX5825_CMD_BLOCK_SIZE] = {MAX5825_SET_CODE_REG,0x0,0x0};

	cmdBlock[0] |= DACAddr & 0xF;/* mask lower nibble of address to select
	 	 	 	 	 	 	 	 	 * the DAC to write to */

	/* MSB must be shifted down Data[12..4] */
	cmdBlock[1] = (uint8_t)((Data & 0x0FF0) >> 4);
	 /* LSnibble Data[3..0] must be shifted to upper nibble */
	cmdBlock[2] = (uint8_t)((Data & 0x000F) << 4);

	i2cWriteData(MAX5825_I2C_ADDRESS,cmdBlock,MAX5825_CMD_BLOCK_SIZE);
}

/**
 * @details have the DAC channel shift CODE to its DAC register
 */
static void setCode2DACRegMAX5825(uint8_t DACAddr)
{
	uint8_t cmdBlock[MAX5825_CMD_BLOCK_SIZE] = {MAX5825_LOAD_DAC_REG,0x0,0x0};

	cmdBlock[0] |= DACAddr & 0xF;/* mask lower nibble of address to select
	 	 	 	 	 	 	 	 	 * the DAC to write to */

	i2cWriteData(MAX5825_I2C_ADDRESS,cmdBlock,MAX5825_CMD_BLOCK_SIZE);
}

/**
 * menu for MAX5825 DAC
 */
static void menuMAX5825()
{
	printf("MAX5825 menu options\r\n");
	printf("d1: reset\r\n");
	printf("d2: s/w clear\r\n");
	printf("d3 <dac#> <value> : write 12bit value to DAC# CODE registers\r\n");
	printf("d4 <dac#> : write DAC# CODE to DAC register\r\n");
	printf("d5 <dac#> : generate wave on DAC# \r\n");
	printf("d6 : stop all wave generation \r\n");
}

/**
 *
 */
void setStartGenWave(uint8_t DACAddr)
{
	printf("start wave %d\r\n",DACAddr);
}

/**
 *
 */
void setStopGenWave()
{
	printf("stop wave\r\n");
}

/*
 * @details store the DAC channel to send the command to.
 *
 */
static void setChannel(uint8_t Channel)
{
	/**
	 * @note need to add error checking here to make sure the channel is correct
	 */
	gDACCmd.channel = Channel;
}

/**
 * @details store the value to write to the DAC channel
 */
static void setValue(uint16_t Value)
{
	/**
	 * @note need to add error checking here to make sure the channel is correct
	 */
	gDACCmd.value = Value;
}

void PIT0_IRQ()
{

	PIT_TFLG0 = PIT_TFLG_TIF_MASK; // clear IRQ
//	GPIOB_PSOR = (PIN1);
//	GPIOB_PCOR = (PIN1);
}


/**
 * ============================================================================
 * Public APIs
 *=============================================================================
 */

void initMAX5825()
{
	gMAX5825cmd = eMAX5825idle;

	/*
	 * enable clock for PIT module
	 */
	SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;

	set_irq_priority (INT_PIT0-16, 1); 	/* assign priority of PIT2 irq in NVIC */
	enable_irq(INT_PIT0-16) ;   		/* enable PIT2 interrupt in NVIC */

	PIT0_TIMER_DELAY = DAC_WR_SAMPLE_RATE; //  =  60Mhz * 2.893us = 173.6
	PIT_MCR = 0;
	PIT0_PULSE_TIMER = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;

}

/**
 * @details Perform MAX5825 DAC communication.
 * The MAXIM5825PMB module should be connected as follows to the FRDM-K22F
 * board.
 *
 * MAX5825       FRDM-K22F
 * J1 connector
 * Pin
 * 1 _LDAC_     - J25:8 P3V3
 * 2 _IRQ_      - n/c
 * 3 SCL        - J24:12 PTB2 (I2C0_SCL)
 * 4 SDA        - J24:10 PTB3 (I2C0_SDA)
 * 5 GND        - J25:14 Gnd
 * 6 VDD        - J25:4  P3V3
 */
void MAX5825evalTask(void *pvParameters)
{
	uint8_t result;
	printf("start MAX5825evalTask\r\n");

	gMAX5825cmd = eMAX5825idle;

	while(1)
	{
		if(gMAX5825cmd)
		{
			switch(gMAX5825cmd)
			{
			case eResetMAX5825:
				resetMAX5825();
				break;
			case eSWClrMAX5825:
				swClrMAX5825();
				break;
			case eLoadCODEMAX5825:
				setCodeMAX5825(gDACCmd.channel,gDACCmd.value);
				break;
			case eCode2DACMAX5825:
				setCode2DACRegMAX5825(gDACCmd.channel);
				break;
			case eMAX5825GenWave:
				setStartGenWave(gDACCmd.channel);
				break;
			case eMAX5825StopGenWave:
				setStopGenWave();
				break;
			}
			gMAX5825cmd = 0;
		}
		vTaskDelay(10);
	}
}

/**
 * @details parse the DAC commands
 */
void parseDACCmd(const int8_t *Cmd)
{
	uint16_t dacCh;
	uint16_t dacValue;
	int32_t retCode;
	char *nextValue,*ptrCheck;
	printf("MAX5825 cmd:%s\r\n",Cmd);

	switch(Cmd[1])
	{
	case '1':
		gMAX5825cmd = eResetMAX5825;
		break;
	case '2':
		gMAX5825cmd = eSWClrMAX5825;
		swClrMAX5825();
		break;
	case '3':
		dacCh = 0;
		dacCh = strtol( (Cmd+2),&nextValue,10);
		if(dacCh != 0)
		{
			dacValue = strtol( nextValue,&ptrCheck,10);
			if(ptrCheck == nextValue)
			{
				retCode = -1;
			}
		}
		if(retCode < 0)
		{
			printf("invalid parameters to set DAC value\r\n");
		}
		else
		{
			printf("eLoadCODEMAX5825 %d %d\r\n",dacCh,dacValue);
			setChannel(dacCh);
			setValue(dacValue);
			gMAX5825cmd = eLoadCODEMAX5825;
		}
		break;
	case '4':
		dacCh = 0;
		dacCh = strtol( (Cmd+2),NULL,10);
		if(dacCh != 0)
		{
			printf("eCode2DACMAX5825 %d\r\n",dacCh,dacValue);
			setChannel(dacCh);
			gMAX5825cmd = eCode2DACMAX5825;
		}
		else
		{
			printf("invalid parameters to set DAC value\r\n");
		}
		break;
	case '5':
		dacCh = 0;
		dacCh = strtol( (Cmd+2),NULL,10);
		if(dacCh != 0)
		{
			printf("eCode2DACMAX5825 %d\r\n",dacCh,dacValue);
			setChannel(dacCh);
			gMAX5825cmd = eMAX5825GenWave;
		}
		else
		{
			printf("invalid parameters to set DAC value\r\n");
		}
		break;
	case '6':
		gMAX5825cmd = eMAX5825StopGenWave;
		break;
	default:
		menuMAX5825();
	}
}




