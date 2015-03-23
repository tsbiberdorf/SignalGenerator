/*
 * FXOS8700CQ.c
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
#include "FXOS8700CQ.h"


typedef enum _FXOSCmds_e
{
	eFXOSidle,
	eFXOSStatus,
	eFXOSWhoAmI,
	eFXOSx,
	eFXOSy,
	eFXOSz,
}eFXOSCmds_t;

const char *gFXOS8700CQTaskname = "FXOS8700CQ Interface Task";
#define FXOS8700_I2C_ADDRESS                         0x1C

uint32_t gFXOScmd = 0;

void readFXOSStatus()
{
	gFXOScmd = eFXOSStatus;
}

void readFXOSWhoAmI()
{
	gFXOScmd = eFXOSWhoAmI;
}

void readFXOSx()
{
	gFXOScmd = eFXOSx;
}

void readFXOSy()
{
	gFXOScmd = eFXOSy;
}

void readFXOSz()
{
	gFXOScmd = eFXOSz;
}

void FXOS8700CQTask(void *pvParameters)
{
	uint8_t result;

	printf("start FXOS8700CQTask\r\n");
	init_I2C();
	gFXOScmd = 0;

	while(1)
	{
		if(gFXOScmd)
		{
			switch(gFXOScmd)
			{
			case eFXOSStatus:
				result = i2cReadRegister(FXOS8700_I2C_ADDRESS,0x00);
				break;
			case eFXOSWhoAmI:
				result = i2cReadRegister(FXOS8700_I2C_ADDRESS,0x0d);
				break;
			case eFXOSx:
				result = i2cReadRegister(FXOS8700_I2C_ADDRESS,0x0d);
				break;
			case eFXOSy:
				result = i2cReadRegister(FXOS8700_I2C_ADDRESS,0x0d);
				break;
			case eFXOSz:
				result = i2cReadRegister(FXOS8700_I2C_ADDRESS,0x0d);
				break;
			}
			printf("result: %x\r\n",result);
			gFXOScmd = 0;
		}
		vTaskDelay(10);
	}
}


