/**
 * @file mbtimer.h
 *
 */

#ifndef __MBTIMER_H__
#define __MBTIMER_H__

/*********************
 *      INCLUDES
 *********************/

#include <stdint.h>
#include "mbrtu.h"

/*********************
 *      DEFINES
 *********************/

#ifndef __IO 
#define __IO volatile
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize timer to set 3.5 character time 
 * according to the communication baud rate 
 * and slave device baud rate.
 * @param baud_rate slave device baud rate.
 */
void mb_timer_init(uint32_t baud_rate);

/**
 * Overloading the 3.5 character time to restart 
 * the timer, For next frame interval control.
 */
void mb_timer_reload();

/**
 * Enable a 3.5 character frame interval timer.
 * Ready to receive data frame.
 */
void mb_timer_enable();

/**
 * Disable a 3.5 character frame interval timer.
 * A frame of data received, off the timer, 
 * ready to process the data.
 */
void mb_timer_disable();

/**
 * The frame interval timer updates the function, 
 * placing the function into a timed interrupt 
 * function to execute.
 */
void mb_timer_tick_callback();

#endif /*__MBTIMER_H__*/
