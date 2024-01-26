/**
 * @file cvtfloat.h
 *
 */

#ifndef __CVTFLOAT_H__
#define __CVTFLOAT_H__

/*********************
 *      INCLUDES
 *********************/

#include <stdint.h>
#include <stdio.h>

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Separate the integer and decimal parts of floating-point, 
 * floating-point separation of the floating-point 
 * part of the magnification, and into a string output display.
 * @param value The floating-point number to be converted.
 * @param digits The number of floating-point 
 * digits that are retained.
 */
void log_string_double_printf(double value, 
    int32_t digits);

/**
 * Separate the integer and decimal parts of floating-point, 
 * floating-point separation of the floating-point 
 * part of the magnification, and into a string output display.
 * @param value The floating-point number to be converted.
 * @param digits The number of floating-point 
 * digits that are retained.
 */
int8_t * log_string_double(double value, 
    int32_t digits);

#endif /*__CVTFLOAT_H__*/
