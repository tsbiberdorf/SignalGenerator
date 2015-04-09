/**
 * @file UserInterface.c
 * @author Terry biberdorf
 * @date Mar 13, 2015
 *
 * @brief Serial User Interface
 *
 * @details This module will contain the Serial debug port handler
 *
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "Cpu.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "..\Peripheral\FXOS8700CQ.h"
#include "..\Peripheral\MAX5825eval.h"
#include "UserInterface.h"

const char *gUserInterfaceTaskname = "User Interface Task";
#define MAX_UI_MSG_SIZE (128)
static int8_t gUIMsgBuffer[MAX_UI_MSG_SIZE +1 ];
static uint16_t gUIMsgBufferIndex = 0;

extern int32_t NewCharPresent();

static void menu()
{
	printf("0 - status\r\n");
	printf("w - Read who am i \r\n");
	printf("A - write Accelerometer cntl Register \r\n");
	printf("M - write Magnetometer cntl Register1 \r\n");
	printf("N - write Magnetometer cntl Register1 \r\n");
	printf("I - initialize the inertia sensor \r\n");
	printf("r - read Inertial Sensor Values \r\n");
}

/*!
 * This task is made to stress the printf functionality
 * @param pvParameters
 */
void UserInterfaceTask(void *pvParameters)
{
	gUIMsgBufferIndex = 0;
	char newChar;
	for(;;)
	{
		if(NewCharPresent())
		{
			newChar = getchar();
			gUIMsgBuffer[gUIMsgBufferIndex++] = newChar;
		}
		if( (newChar == '\n') || (gUIMsgBufferIndex ==MAX_UI_MSG_SIZE) )
		{
			gUIMsgBufferIndex = 0;
			switch( gUIMsgBuffer[0] )
			{
			case '0':
				readFXOSStatus();
				break;
			case 'd':
			case 'D':
				parseDACCmd(gUIMsgBuffer);
				break;
			case 'w':
				readFXOSWhoAmI();
				break;
			case 'A':
				writeAccelCntlReg(gUIMsgBuffer+1);
				break;
			case 'M':
				writeMagCntlReg1(gUIMsgBuffer+1);
				break;
			case 'N':
				writeMagCntlReg2(gUIMsgBuffer+1);
				break;
			case 'I':
				initFXOS();
				break;
			case 'J':
				initMAX5825();
				break;
			case 'r':
				readInertiaValues();
				break;
			case 'R':
				readInertiaValuesMulti();
				break;
			case '?':
				menu();
				break;
			default:
				printf("UI: %s\r\n",gUIMsgBuffer);
			}
			newChar = 0;
		}
		vTaskDelay(1);
	}
}
