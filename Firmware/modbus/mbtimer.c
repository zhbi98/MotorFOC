/**
 * @file mbtimer.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "mbtimer.h"

/*********************
 *      DEFINES
 *********************/

#define MB_TIMEOUT_INVALID (65535U)

/**********************
 *  STATIC VARIABLES
 **********************/

static __IO uint32_t tick50us_inited = 0;
static __IO uint32_t tick50us_val = 0;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize timer to set 3.5 character time according to
 * the communication baud rate and slave device baud rate.
 * @param baud_rate slave device baud rate.
 */
void mb_timer_init(uint32_t baud_rate)
{
    /* The timer reload value for a character is given by:
     *
     * ChTimeValue = Ticks_per_1s / ( Baudrate / 11 )
     *             = 11 * Ticks_per_1s / Baudrate
     *             = 220000 / Baudrate
     * The reload for t3.5 is 1.5 times this value 
     * and similary for t3.5.
     */
    uint32_t val = (
        7UL * 220000UL
    ) / (2UL * baud_rate);

    /**
     * If baudrate > 19200 then we should use the fixed 
     * timer values t35 = 1750us. Otherwise t35 must be 
     * 3.5 times the character time.
     */
    uint32_t _val = (
        baud_rate > 19200
    ) ? 35 : val;

    tick50us_inited = _val;
}

/**
 * The frame interval timer updates the function, placing
 * the function intoa timed interrupt function to execute.
 */
void mb_timer_tick_callback()
{
    if (tick50us_val == 
        MB_TIMEOUT_INVALID)
        return;

    /*A frame of data received, off the timer, 
    ready to process the data*/
    if (tick50us_val > 0)
        tick50us_val--;
    /*If the counter is an unsigned number, 
    you need to use judgement, otherwise it is 
    easy to have an abnormal negative value*/
    
    if (tick50us_val > 0) return;

    /*tick50us_val = MB_TIMEOUT_INVALID;*/
    mb_timer_disable();

    /*The packet frame interval is up 
    and the bus status update begins*/
    mb_rtu_T35_expired();
}

/**
 * Enable a 3.5 character frame interval timer, ready 
 * to receive data frame.
 */
void mb_timer_enable()
{
    uint32_t val = tick50us_inited;
    tick50us_val = val;
}

/**
 * Overloading the 3.5 character time to restart the timer,
 * for next frame interval control.
 */
void mb_timer_reload()
{
    uint32_t val = tick50us_inited;
    tick50us_val = val;
}

/**
 * Disable a 3.5 character frame interval timer.
 * A frame of data received, off the timer, 
 * ready to process the data.
 */
void mb_timer_disable()
{
    tick50us_val = \
        MB_TIMEOUT_INVALID;
}
