/**
 * @file stm32.h
 *
 */

#ifndef __STM32_H__
#define __STM32_H__

/*********************
 *      INCLUDES
 *********************/

#include "stm32f4xx_hal.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initializes the device's core clock in preparation for startup.
 * The initialization frequency is 168 MHZ.
 */
void stm32_init();

#endif /*__STM32_H__*/
