/*
 * uartInterface.c
 *
 *  Created on: Apr 2, 2015
 *      Author: TBiberdorf
 */


#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "arm_cm4.h"

#include "Cpu.h"
#include "FreeRTOS.h"
#include "task.h"
#include "..\Peripheral\uartInterface.h"


#define UART_BIT_SAMPLE_RATE (172)
const char *gUartPingTaskname = "UART ping Interface Task";
#define PIN0 (1<<0)
#define PIN1 (1<<1)
#define PIN2 (1<<2)
#define PIN3 (1<<3)
#define PIN4 (1<<4)
#define PIT0_PULSE_TIMER (PIT_TCTRL0) /**< timer control register */
#define PIT0_TIMER_DELAY (PIT_LDVAL0) /**< timeout period calculated by (Period * 48Mhz) */
#define BUFFER_SIZE (4*1014)
#define NUMB_READ_SAMPLES_PER_BIT (2)
#define VERIFY_HIGH_BIT (0x03)
#define VERIFY_LOW_BIT (0x00)


enum bitElement_e
{
	eStart,
	eBit0,
	eBit1,
	eBit2,
	eBit3,
	eBit4,
	eBit5,
	eBit6,
	eBit7,
	eStop
};
enum bitStatus_e
{
	eEmpty,
	eData,
	eFull,
};

typedef struct bitBang_s
{
	enum bitElement_e bitElement;
	uint8_t readMask;
	uint8_t readBit;
	uint8_t readCnt;
	char *writePtr;
	uint8_t wrIndex;
	uint8_t rdIndex;
	char currentByte;
	char bufferByte;
	enum bitStatus_e status;
}sbitBang_t;

typedef struct PortSample_s
{
	uint8_t sampleSize;
	sbitBang_t **port;
}sPortSample_t;

#define INPUT_PORT (GPIOB_PDIR)
sPortSample_t gPortB;
sbitBang_t *tmpSample;
static void pushDataByte(sbitBang_t *BitSample)
{
	BitSample->wrIndex++;
	if(BitSample->wrIndex == BUFFER_SIZE)
	{
		BitSample->wrIndex = 0;
	}
	if(BitSample->wrIndex == BitSample->rdIndex)
	{
		/*
		 * if our buffer has been filled, drop
		 * the current byte.
		 */
		BitSample->wrIndex--;
		BitSample->status = eFull;
	}
	else
	{
		BitSample->status = eData;
		BitSample->writePtr[BitSample->wrIndex] = BitSample->currentByte;
	}
}

/**
 * @details Check for a start byte to be received.
 *
 * @param ReadValue GPIO port data
 * @param BitSample pointer to data structure for current port bit.
 * @return void
 */
static void DataStartBit(uint32_t ReadValue,sbitBang_t *BitSample)
{
	if( BitSample->readCnt < NUMB_READ_SAMPLES_PER_BIT )
	{
		if(!(BitSample->readMask & ReadValue))
		{
			GPIOD_PCOR = (PIN4);
			GPIOB_PSOR = (PIN1);
			GPIOD_PSOR = (PIN2);
			/* our start bit must be zero to be
			 * valid
			 */
			BitSample->readCnt++;
		}
		else
		{
			GPIOD_PSOR = (PIN4);
			BitSample->readCnt = 0;
			GPIOD_PCOR = (PIN2);
		}
	}
	else
	{
		GPIOB_PSOR = (PIN1);
		/*
		 * move to the next bit
		 */
		BitSample->readCnt = 0;
		BitSample->readBit  = 0;;
		BitSample->bitElement = eBit0;
	}
	GPIOB_PCOR = (PIN1);
}

/**
 * @details Check for a stop bit to be received.
 *
 * @param ReadValue GPIO port data
 * @param BitSample pointer to data structure for current port bit.
 * @return void
 */
static void DataStopBit(uint32_t ReadValue,sbitBang_t *BitSample)
{
	GPIOB_PSOR = (PIN1);

	if( BitSample->readMask & ReadValue )
	{
		GPIOD_PSOR = (PIN4);
		/* our stop bit must be high to be
		 * valid
		 */
		BitSample->readBit = 0;
		BitSample->readCnt = 0;
		BitSample->bitElement = eStart;

		/*
		 * shift read byte to results queue
		 */
		pushDataByte(BitSample);
		BitSample->bufferByte = BitSample->currentByte;
		tmpSample = BitSample;
		NVICSTIR = (INT_SWI-16); /* trigger a SWI interrrupt */

		BitSample->currentByte = 0x00;
		GPIOD_PCOR = (PIN2);
	}
	else
	{
		BitSample->readBit = 0;
		BitSample->readCnt = 0;
		BitSample->bitElement = eStart;

		BitSample->currentByte = 0x00;
		GPIOD_PCOR = (PIN2);
	}

//	if( BitSample->readCnt < NUMB_READ_SAMPLES_PER_BIT )
//	{
//		if( BitSample->readMask & ReadValue )
//		{
//			/* our stop bit must be high to be
//			 * valid
//			 */
//			BitSample->readCnt++;
//		}
//		else
//		{
//			BitSample->readCnt = 0;
//		}
//	}
//	if(BitSample->readCnt == NUMB_READ_SAMPLES_PER_BIT)
//	{
//		BitSample->readCnt++;
//		/*
//		 * good stop bit
//		 */
//		BitSample->readBit = 0;
//		BitSample->readCnt = 0;
//		BitSample->bitElement = eStart;
//
//		/*
//		 * shift read byte to results queue
//		 */
//		pushDataByte(BitSample);
//		BitSample->currentByte = 0x00;
//		GPIOD_PCOR = (PIN2);
//	}
	GPIOB_PCOR = (PIN1);
}

/**
 * @details Sample the 8 data bits for each UART byte.
 * When all 8 bits are recieved, a stop byte will be checked for.
 *
 * @param ReadValue GPIO port data
 * @param Shift number of bits to shift the current sample data
 * @param BitSample pointer to data structure for current port bit.
 * @return void
 */
static void DataBitSample(uint32_t ReadValue,uint8_t Shift, sbitBang_t *BitSample)
{
	GPIOB_PSOR = (PIN1);

	if( BitSample->readCnt < NUMB_READ_SAMPLES_PER_BIT )
	{
		/*
		 * measure first two readings
		 */
		if((BitSample->readMask & ReadValue))
		{
			GPIOD_PSOR = (PIN4);

			BitSample->readCnt++;
			BitSample->readBit = BitSample->readBit << 1;
			BitSample->readBit |= 1;
		}
		else
		{
			GPIOD_PCOR = (PIN4);
			BitSample->readCnt++;
			BitSample->readBit = BitSample->readBit << 1;
		}
	}
	else
	{
		BitSample->readCnt++;

		/*
		 * verify that our bit that we just read is valid
		 */
		if(BitSample->readBit ==  VERIFY_HIGH_BIT)
		{
			GPIOD_PSOR = (PIN1);
			/*
			 * we just read a high level
			 */
			BitSample->bitElement++;
			BitSample->currentByte |= (0x80>>Shift);
			GPIOD_PCOR = (PIN1);
		}
		else if(BitSample->readBit ==  VERIFY_LOW_BIT)
		{
			GPIOD_PSOR = (PIN0);
			/*
			 * we just read a zero level
			 */
			BitSample->bitElement++;
			GPIOD_PCOR = (PIN0);
		}
		else
		{
			/*
			 * one of our samples does not match
			 * return looking for a start bit.
			 */
			BitSample->bitElement = eStart;
		}

		/*
		 * move to the next bit
		 */
		BitSample->readBit = 0;
		BitSample->readCnt = 0;

	}

	exitMethod:
	GPIOB_PCOR = (PIN1);
	return;
}

static void uartPortBSample(uint32_t ReadValue)
{
	uint8_t shift;
	uint8_t i;
	for(i=0;i< gPortB.sampleSize;i++)
	{
		/*
		 * loop through all possible input bits on Port B
		 */
		shift = 0x7;
		switch(gPortB.port[i]->bitElement)
		{
		case eStart:
			DataStartBit( ReadValue, gPortB.port[i] );
			break;
		case eBit7:
			shift--;
		case eBit6:
			shift--;
		case eBit5:
			shift--;
		case eBit4:
			shift--;
		case eBit3:
			shift--;
		case eBit2:
			shift--;
		case eBit1:
			shift--;
		case eBit0:
			GPIOD_PSOR = (PIN3);
			DataBitSample(ReadValue, shift, (gPortB.port[i]) );
			GPIOD_PCOR = (PIN3);
			break;
		case eStop:
			DataStopBit( ReadValue, (gPortB.port[i]) );
			break;
		}
	}
}

void PIT0_IRQ()
{
	uint32_t gpioRead;

	PIT_TFLG0 = PIT_TFLG_TIF_MASK; // clear IRQ
//	GPIOB_PSOR = (PIN1);
	gpioRead = INPUT_PORT;
	uartPortBSample(gpioRead);
//	GPIOB_PCOR = (PIN1);
}

void SWI_IRQ()
{
	pushDataByte(tmpSample);

//	GPIOB_PTOR = (PIN1);

}

static void initPortBSampleData()
{
	gPortB.sampleSize = 1;
	gPortB.port = pvPortMalloc( (sizeof(sbitBang_t *)) * gPortB.sampleSize);

	gPortB.port[0] = pvPortMalloc(sizeof(sbitBang_t));
//	gPortB.port[1] = pvPortMalloc(sizeof(sbitBang_t));

	gPortB.port[0]->readMask = (1<<0);
	gPortB.port[0]->writePtr = pvPortMalloc(BUFFER_SIZE+1);
//	gPortB.port[2]->readMask = (1<<1);
}

static void initUartRxPins()
{
	int32_t gpioValue;
	SIM_SCGC5 |= (SIM_SCGC5_PORTB_MASK|SIM_SCGC5_PORTD_MASK);

	gpioValue = GPIOB_PDDR;
	gpioValue &= ~(PIN0); /* clear pin0 */
	gpioValue |= (PIN1); /* set pin1 */

	GPIOB_PDDR |= gpioValue;
	GPIOB_PSOR = (PIN1);
//	GPIOB_PCOR = PIN1;
	PORTB_PCR0 = PORT_PCR_MUX(0x1);
	PORTB_PCR1 = PORT_PCR_MUX(0x1);

	GPIOD_PDDR |= (PIN0|PIN1|PIN2|PIN3|PIN4);
	GPIOD_PCOR = (PIN0|PIN1|PIN2|PIN3|PIN4);
	PORTD_PCR0 = PORT_PCR_MUX(0x1);
	PORTD_PCR1 = PORT_PCR_MUX(0x1);
	PORTD_PCR2 = PORT_PCR_MUX(0x1);
	PORTD_PCR3 = PORT_PCR_MUX(0x1);
	PORTD_PCR4 = PORT_PCR_MUX(0x1);

	/*
	 * enable clock for PIT module
	 */
	SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;

	set_irq_priority (INT_PIT0-16, 1); 	/* assign priority of PIT2 irq in NVIC */
	enable_irq(INT_PIT0-16) ;   		/* enable PIT2 interrupt in NVIC */

	PIT0_TIMER_DELAY = UART_BIT_SAMPLE_RATE; //  =  60Mhz * 2.893us = 173.6
	PIT_MCR = 0;
	PIT0_PULSE_TIMER = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;
}

void UartPingTask(void *pvParameters)
{
	uint8_t result;
	uint32_t portValue,portEVal;
	uint32_t value;
	uint32_t uartFlagSwitch = 0;
	int32_t i;
	printf("start UartPingTask\r\n");

	set_irq_priority (INT_SWI-16, 8);
	enable_irq(INT_SWI-16) ;   // enable  switch handler interrupt

	initPortBSampleData();
	initUartRxPins();

	while(1)
	{
		GPIOB_PTOR = (PIN0);
//		NVICSTIR = (INT_SWI-16); /* trigger a SWI interrrupt */

		for(i=0; i< gPortB.sampleSize; i++)
		{
			if( gPortB.port[i]->rdIndex != gPortB.port[i]->wrIndex )
			{
				printf("%c",gPortB.port[i]->writePtr[gPortB.port[i]->rdIndex++]);
			}
		}
		vTaskDelay(10);
	}
}

