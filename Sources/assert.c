/*!
 * @file assert.c
 *
 * Implements cstdlib's assert behavior.
 *
 */

#include <stdio.h>
#include <assert.h>

#include "FreeRTOS.h"

/*!
 * Assert delay Counter
 */
#define ASSERT_COUNTER (2e6)

// Exclude implementation of function if NDEBUG is defined
#ifndef NDEGUB
/*!
 * Implements the behavior of the cstdlib assert function.
 *
 * When this function is referenced by the assert macro, it will print the text
 * of the assertion it failed on, the file, the line number and the function.
 * It repeats printing this data to the UART port at a period determined by
 * ASSERT_COUNTER. For me, it seems to repeat once every 20 seconds.
 *
 * It WILL increase code size dramatically since assert creates a string from
 * every expression put inside of it's brackets.
 *
 * newlibc-nano (along with other newlib implementations) have their own
 * implementation of __assert_func. I believe these are weak aliases, or it
 * behaves like a weak alias, which allows this function to "override" the
 * default assert functionality.
 *
 * @param file Name string of the file.
 * @param line Line number causing the assert.
 * @param function Function name the assert occurred in.
 * @param expression Expression that evaluated to 0 that caused the assert.
 * @todo Possibly have this print out the assert information repetitively.
 *
 */
void  __attribute__((__noreturn__)) __assert_func(const char *file, int line,
		const char *function, const char *expression)
{
	int counter = 0;

	// Disable interrupts
	portDISABLE_INTERRUPTS();

	while(1)
	{
		fprintf(stderr, "assertion \"%s\" failed: file \"%s\", line %d, function: "
					"%s\r\n", expression, file, line, function);
		for(counter = 0; counter < ASSERT_COUNTER; counter++)
		{
			__asm("nop");
		}
	}
}
#endif
