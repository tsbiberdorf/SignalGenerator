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
#include "UserInterface.h"

const char *gUserInterfaceTaskname = "User Interface Task";
#define MAX_UI_MSG_SIZE (128)
static char gUIMsgBuffer[MAX_UI_MSG_SIZE +1 ];
static uint16_t gUIMsgBufferIndex = 0;

extern int32_t NewCharPresent();

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
				readFXOSWhoAmI();
				break;
			case '1':
				readFXOSx();
				break;
			case '2':
				readFXOSy();
				break;
			case '3':
				readFXOSz();
				break;
			default:
				printf("UI: %s\r\n",gUIMsgBuffer);
			}
			newChar = 0;
		}
		vTaskDelay(1);
	}
}
