/*
 * uartInterface.c
 *
 *  Created on: Apr 2, 2015
 *      Author: TBiberdorf
 */


#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "Cpu.h"
#include "FreeRTOS.h"
#include "task.h"
#include "..\Peripheral\uartInterface.h"



const char *gUartPingTaskname = "UART ping Interface Task";

static void initUartRxPins()
{

}

void UartPingTask(void *pvParameters)
{
	uint8_t result;
	uint32_t portValue,portEVal;
	uint32_t value;
	uint32_t uartFlagSwitch = 0;
	printf("start UartPingTask\r\n");


	initUartRxPins();

	while(1)
	{
		if( uartFlagSwitch == 1)
		{
			uartFlagSwitch = 0;
			PORTE_PCR1 = PORT_PCR_MUX(0x03);
		}

		if( uartFlagSwitch == 2)
		{
			uartFlagSwitch = 0;
			PORTE_PCR1 = PORT_PCR_MUX(0x01);
			GPIOE_PDDR = ~(1<<1);
		}
		portEVal = GPIOE_PDIR & (1<<1);
		printf("%d",portEVal);
		vTaskDelay(10);
	}
}

