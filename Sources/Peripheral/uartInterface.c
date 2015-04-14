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
#define PIN0 (1<<0)
#define PIN1 (1<<1)
#define PIT0_PULSE_TIMER (PIT_TCTRL0) /**< timer control register */
#define PIT0_TIMER_DELAY (PIT_LDVAL0) /**< timeout period calculated by (Period * 48Mhz) */

void PIT0_IRQ()
{
	PIT_TFLG0 = PIT_TFLG_TIF_MASK; // clear IRQ
	GPIOB_PTOR = (PIN1);

}

static void initUartRxPins()
{
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;

	GPIOB_PDDR |= (PIN0|PIN1);
	GPIOB_PSOR = (PIN0|PIN1);
//	GPIOB_PCOR = PIN1;
	PORTB_PCR0 = PORT_PCR_MUX(0x1);
	PORTB_PCR1 = PORT_PCR_MUX(0x1);

	enable_irq();
	/*
	 * enable clock for PIT module
	 */
	SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;

	set_irq_priority (INT_PIT0-16, 6); 	/* assign priority of PIT2 irq in NVIC */
	enable_irq(INT_PIT0-16) ;   		/* enable PIT2 interrupt in NVIC */

	PIT0_TIMER_DELAY = 50; //  =  50Mhz * 1us = 50
	PIT0_PULSE_TIMER = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;
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
		GPIOB_PTOR = (PIN0);
		vTaskDelay(1);
	}
}

