/*
 * main.h
 *
 *  Created on: Mar 6, 2015
 *      Author: AMcguire
 */

#ifndef MAIN_H_
#define MAIN_H_

#define MAJOR_VERSION (0)
#define MINOR_VERSION (0)
#define VERSION_VERSION (0)
#define REVISION_VERSION (1)

#define PORTD_DEBUG_IO (1)
#ifdef PORTD_DEBUG_IO
#define BIT0 (1<<0)
#define BIT1 (1<<1)
#define BIT2 (1<<2)
#define BIT3 (1<<3)

#define SETD_BIT0() (GPIOD_PSOR |= BIT0)
#define SETD_BIT1() (GPIOD_PSOR |= BIT1)
#define SETD_BIT2() (GPIOD_PSOR |= BIT2)
#define SETD_BIT3() (GPIOD_PSOR |= BIT3)

#define CLRD_BIT0() (GPIOD_PCOR |= BIT0)
#define CLRD_BIT1() (GPIOD_PCOR |= BIT1)
#define CLRD_BIT2() (GPIOD_PCOR |= BIT2)
#define CLRD_BIT3() (GPIOD_PCOR |= BIT3)

#define TGLD_BIT0() (GPIOD_PTOR |= BIT0)
#define TGLD_BIT1() (GPIOD_PTOR |= BIT1)
#define TGLD_BIT2() (GPIOD_PTOR |= BIT2)
#define TGLD_BIT3() (GPIOD_PTOR |= BIT3)

#else
#define SETD_BIT0()
#define SETD_BIT1()
#define SETD_BIT2()
#define SETD_BIT3()

#define CLRD_BIT0()
#define CLRD_BIT1()
#define CLRD_BIT2()
#define CLRD_BIT3()

#define TGLD_BIT0()
#define TGLD_BIT1()
#define TGLD_BIT2()
#define TGLD_BIT3()

#endif

#endif /* MAIN_H_ */
