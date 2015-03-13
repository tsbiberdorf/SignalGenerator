
#include <stdlib.h>
#include <stdio.h>

#include "IO_Map.h"
#include "PDD/UART_PDD.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define SERIAL_BASE_UART_BASE_PTR (UART1_BASE_PTR) /**< which UART to use */
//#define SERIAL_BASE_UART_BASE_PTR (UART3_BASE_PTR) /**< which UART to use */

/*!
 * Semaphore used to protect UART TX FIFO from multithreaded access.
 */
static SemaphoreHandle_t gSerialDebugWriteMutex = NULL;

int32_t NewCharPresent()
{
	int32_t charPresent = 0;
	if ((UART_PDD_ReadInterruptStatusReg(SERIAL_BASE_UART_BASE_PTR) & UART_S1_RDRF_MASK) )
	{
		/* Any data in receiver buffer */
		charPresent = 1;
	}
	return charPresent;
}

/*!
 * Read "count" size_t (read byte) units from the device (UART3)
 * @param fd Unused file descriptor
 * @param buf Buffer to read into
 * @param count number of characters to read from the device.
 * @return
 */
int _read (int fd, const void *buf, size_t count)
{
	size_t CharCnt = 0x00;

	(void)fd;                            /* Parameter is not used, suppress unused argument warning */
	for (;count > 0x00; --count) {
		if ((UART_PDD_ReadInterruptStatusReg(SERIAL_BASE_UART_BASE_PTR) & UART_S1_RDRF_MASK) == 0x00) { /* Any data in receiver buffer */
			if (CharCnt != 0x00) {           /* No, at least one char received? */
				break;                         /* Yes, return received char(s) */
			} else {                         /* Wait until a char is received */
				vTaskDelay(1);
			}
		}
		CharCnt++;                         /* Increase char counter */
		/* Save character received by UARTx device into the receive buffer */
		*(uint8_t*)buf = (unsigned char)UART_PDD_GetChar8(SERIAL_BASE_UART_BASE_PTR);
		/* Stop reading if CR (Ox0D) character is received */
		if (*(uint8_t*)buf == 0x0DU) {     /* New line character (CR) received ? */
			*(uint8_t*)buf = '\n';           /* Yes, convert LF to '\n' char. */
			break;                           /* Stop loop and return received char(s) */
		}
		(uint8_t*)buf++;                   /* Increase buffer pointer */
	}
	return CharCnt;
}

/*!
 * This function is low layer support for printf-like functions.
 * @param fd Unused file descriptor
 * @param buf Buffer of data to write to the device.
 * @param count Number of size_t (read "byte") sized data units to write to
 * the device.
 * @note This was originally a processor expert function. I removed the '\r'
 * before '\n' code. I would rather just have printf send what we ask it to and
 * take care of line endings in the application code. (as far as I can tell,
 * its not proper stdlib functionality anyway..)
 * @return return number of characters written to the device.
 */
int _write (int fd, const void *buf, size_t count)
{
	size_t CharCnt = 0x00;
	volatile int32_t status;
	/*
	 * The file descriptor for stdout is 1, stderr is 2
	 */
	if(fd == 1)
	{
		if(gSerialDebugWriteMutex == NULL)
		{
			/*
			 * A binary semaphore is created in the "taken" state, so once we
			 * create it we will leave it taken until we are done using the UART.
			 * Otherwise, if it has already been created, we need to take it.
			 */
			gSerialDebugWriteMutex = xSemaphoreCreateBinary();
		}
		else
		{
			xSemaphoreTake(gSerialDebugWriteMutex, portMAX_DELAY);
		}
	}
	else if(fd == 2)
	{
		// This is standard error
		// maybe disable interrupts
	}
	else
	{
		// not handled either
		// Could possibly use this for ctx->gio operations??
	}

	for (;count > 0x00; --count) {
		status = UART1_S1;
		while(status & UART_S1_TDRE_MASK == 0)
		{
			;
		}
		/* Wait until UART is ready for saving a next character into the transmit buffer */
		while ((UART_PDD_ReadInterruptStatusReg(SERIAL_BASE_UART_BASE_PTR) & UART_S1_TDRE_MASK) == 0)
		{
			/*
			 * If we are pending on a status register, we may consider lazily
			 * delaying until the uart is ready, it almost depends on the baud.
			 * Another option is implement the interrupt and allow the task to
			 * be woken from the ISR if it is a higher priority the whatever
			 * task is currently running.
			 * For now, spinlock...
			 */
			//vTaskDelay(1);
		}
		/* Save a character into the transmit buffer of the UART0 device */
		UART_PDD_PutChar8(SERIAL_BASE_UART_BASE_PTR, (unsigned char)*(uint8_t*)buf);
		(uint8_t*)buf++;                   /* Increase buffer pointer */
		CharCnt++;                         /* Increase char counter */
	}

	if(fd == 1)
	{
		xSemaphoreGive(gSerialDebugWriteMutex);
	}
	else if(fd == 2)
	{
		// maybe restore interrupts to whatever state they were in previously
		// if we disable them when coming in...
	}

	return CharCnt;
}
