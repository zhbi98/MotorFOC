/**
 * @file time.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "time.h"

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void sleep_us(uint32_t us)
{
    volatile uint32_t i;

    for (; us > 0; us--)
        for (i = 12; 
            i > 0; i--);
}

void sleep_ms(uint32_t ms)
{
    volatile uint32_t i;

    for (; ms > 0; ms--)
        for (i = 12000; 
            i > 0; i--);
}
