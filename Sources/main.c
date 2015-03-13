/*!
 * Main is the entry for the application code once the low level start sequence
 * has occurred.
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "Cpu.h"

#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "UserInterface\UserInterface.h"

char *task1name = "Task1";
char *task2name = "Task2";

/*!
 * This task is made to stress the printf functionality
 * @param pvParameters
 */
void TestPrint(void *pvParameters)
{
	int32_t delay=100;
	int32_t getFlag = 0;
	if( strcmp((char*)pvParameters,task1name) == 0)
	{
		delay = 1000;
		getFlag = 1;
	}
	if( strcmp((char*)pvParameters,task2name) == 0)
	{
		delay = 5000;
	}

	for(;;)
	{
		printf("%s\r\n", (char*)pvParameters);
		vTaskDelay(delay);

//		if(getFlag)
//		{
//			printf("%c\r\n",getchar());
//		}
	}
}

static void CreateSigGenTasks()
{
	xTaskCreate(TestPrint, task1name, 512, task1name, 3, NULL);
	xTaskCreate(TestPrint, task2name, 512, task2name, 2, NULL);

	xTaskCreate(UserInterfaceTask, gUserInterfaceTaskname, 512, gUserInterfaceTaskname, 3, NULL);
}

/*!
 * Entry point after booting the microcontroller.
 * @return Never returns
 */
int main(void)
{
	int value = 0;
	PE_low_level_init();

	fprintf(stderr, "\n\n\n\n\r\nMP8000\r\n");
	fprintf(stderr, "FW Version %d.%d.%d.%d\r\n\r\n", MAJOR_VERSION,
			MINOR_VERSION, VERSION_VERSION, REVISION_VERSION);

	fprintf(stderr, "Creating Tasks\r\n");
	CreateSigGenTasks();
	fprintf(stderr, "Starting Scheduler\r\n");

	// I think I need to add vPortSVCHandler, vPortPendSVHandler and vPortTickHandler to the IVT
	vTaskStartScheduler();

	assert(0);
}

