/* ###################################################################
**     This component module is generated by Processor Expert. Do not modify it.
**     Filename    : pin_mux.h
**     Project     : SignalGenerator
**     Processor   : MK22FN512VLH12
**     Component   : PinSettings
**     Version     : Component 01.007, Driver 1.1, CPU db: 3.00.000
**     Compiler    : GNU C Compiler
**     Date/Time   : 2015-03-11, 15:56, # CodeGen: 0
**     Abstract    :
**
**     Settings    :
**
**     Contents    :
**         hardware_init       - void hardware_init(void);
**         configure_ftm_pins  - void configure_ftm_pins(uint32_t instance);
**         configure_gpio_pins - void configure_gpio_pins(uint32_t instance);
**         configure_i2c_pins  - void configure_i2c_pins(uint32_t instance);
**         configure_i2s_pins  - void configure_i2s_pins(uint32_t instance);
**         configure_jtag_pins - void configure_jtag_pins(uint32_t instance);
**         configure_llwu_pins - void configure_llwu_pins(uint32_t instance);
**         configure_osc_pins  - void configure_osc_pins(uint32_t instance);
**         configure_rtc_pins  - void configure_rtc_pins(uint32_t instance);
**         configure_spi_pins  - void configure_spi_pins(uint32_t instance);
**         configure_uart_pins - void configure_uart_pins(uint32_t instance);
**
**     Copyright : 1997 - 2014 Freescale Semiconductor, Inc. 
**     All Rights Reserved.
**     
**     Redistribution and use in source and binary forms, with or without modification,
**     are permitted provided that the following conditions are met:
**     
**     o Redistributions of source code must retain the above copyright notice, this list
**       of conditions and the following disclaimer.
**     
**     o Redistributions in binary form must reproduce the above copyright notice, this
**       list of conditions and the following disclaimer in the documentation and/or
**       other materials provided with the distribution.
**     
**     o Neither the name of Freescale Semiconductor, Inc. nor the names of its
**       contributors may be used to endorse or promote products derived from this
**       software without specific prior written permission.
**     
**     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
**     ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
**     WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
**     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
**     ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
**     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
**     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
**     ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
**     (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
**     SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**     
**     http: www.freescale.com
**     mail: support@freescale.com
** ###################################################################*/
/*!
** @file pin_mux.h
** @version 1.1
** @brief
**
*/         
/*!
**  @addtogroup pin_mux_module pin_mux module documentation
**  @{
*/         

#ifndef pin_mux_H_
#define pin_mux_H_

/* MODULE pin_mux. */

/* Including shared modules, which are used in the whole project */
#include "Init_Config.h"


void pin_mux_hardware_init(void);
/*
** ===================================================================
**     Method      :  pin_mux_hardware_init (component PinSettings)
*/
/*!
**     @brief
**         This method sets registers according routing settings. Call
**         this method in user code to route desired pins into
**         peripherals. The method is useful for reinitialization HW
**         after some routing changes.
*/
/* ===================================================================*/


/* END pin_mux. */
#endif /* #ifndef __pin_mux_H_ */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.4 [05.11]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
