/**
 * @file cvtfloat.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "cvtfloat.h"

/**********************
 *  STATIC VARIABLES
 **********************/

static int8_t _float_str[16] = {0};

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
    int32_t digits)
{
    int32_t part_int = 0;
    int32_t part_fract = 0;
    int32_t base = 1;

    int32_t _part_int = 0;

    for (; digits != 0; base *= 10, digits--);
        part_int = (int32_t)value;

    _part_int = (int32_t)value;

    part_fract = (int32_t)(
        (value - (double)_part_int) * base
    );

    if (part_fract < 0)
        part_fract *= -1;

    printf("%d.%d", part_int, 
        part_fract);
}

/**
 * Separate the integer and decimal parts of floating-point, 
 * floating-point separation of the floating-point 
 * part of the magnification, and into a string output display.
 * @param value The floating-point number to be converted.
 * @param digits The number of floating-point 
 * digits that are retained.
 */
int8_t * log_string_double(double value, 
    int32_t digits)
{
    /*int32_t digits = 3;*/
    int32_t part_fract = 0;
    int32_t intPart = 0;
    int32_t base = 1;

    int32_t _part_int = 0;

    for (; digits != 0; base *= 10, digits--);
        intPart = (int32_t)value;

    _part_int = (int32_t)value;

    part_fract = (int32_t)(
        (value - (double)_part_int) * base
    );

    if (part_fract < 0)
        part_fract *= -1;

    snprintf((int8_t *)_float_str, 
        16, "%d.%d", 
        intPart, 
        part_fract);

    return _float_str;
}
