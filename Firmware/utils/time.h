/**
 * @file time.h
 *
 */

#ifndef __TIME_H__
#define __TIME_H__

/*********************
 *      INCLUDES
 *********************/

#include <stdint.h>

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void sleep_us(uint32_t us);
void sleep_ms(uint32_t ms);

#endif /*__TIME_H__*/
