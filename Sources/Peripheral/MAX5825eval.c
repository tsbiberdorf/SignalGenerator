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
#include <assert.h>

#include "Cpu.h"
#include "FreeRTOS.h"
#include "task.h"
#include "i2cInterface.h"
#include "MAX5825eval.h"



typedef enum _MAX5825Cmds_e
{
	eMAX5825idle,
	eInitMAX5825,
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


/**
 * @details reset the DAC module
 */
static void resetMAX5825()
{
	uint8_t cmdBlock[MAX5825_CMD_BLOCK_SIZE] = {MAX5825_RESET_REG,0x96,0x30};

	i2cWriteData(MAX5825_I2C_ADDRESS,cmdBlock,MAX5825_CMD_BLOCK_SIZE);
}

/**
 * @details perform a SW clear of DAC module registers
 */
static void swClrMAX5825()
{
	uint8_t cmdBlock[MAX5825_CMD_BLOCK_SIZE] = {MAX5825_SW_CLR_REG,0x96,0x30};

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
static void setCode2DACRegMAX5825(uint8_t DACAddr,uint16_t Data)
{
	uint8_t cmdBlock[MAX5825_CMD_BLOCK_SIZE] = {MAX5825_LOAD_DAC_REG,0x0,0x0};

	cmdBlock[0] |= DACAddr & 0xF;/* mask lower nibble of address to select
	 	 	 	 	 	 	 	 	 * the DAC to write to */

	i2cWriteData(MAX5825_I2C_ADDRESS,cmdBlock,MAX5825_CMD_BLOCK_SIZE);
}


/******************************************************************************
 * public
 *****************************************************************************/

void initMAX5825()
{
	gMAX5825cmd = eInitMAX5825;
}


void MAX5825evalTask(void *pvParameters)
{
	uint8_t result;

	printf("start MAX5825evalTask\r\n");

	gMAX5825cmd = eMAX5825idle;

	while(1)
	{
		if(gMAX5825cmd)
		{
			printf("result: %x\r\n",result);
			gMAX5825cmd = 0;
		}
		vTaskDelay(10);
	}
}




