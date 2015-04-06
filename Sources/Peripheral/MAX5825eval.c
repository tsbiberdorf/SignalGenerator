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


const char *gMAX5825evalTaskname = "MAX5825 Interface Task";
#define MAX5825_I2C_ADDRESS                         0x10

eMAX5825Cmds_t gMAX5825cmd = eMAX5825idle;
uint8_t gMAX5825Data = 0;




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




