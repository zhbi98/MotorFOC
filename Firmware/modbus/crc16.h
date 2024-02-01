/**
 * @file crc16.h
 *
 */

#ifndef __CRC16_H__
#define __CRC16_H__

/*********************
 *      INCLUDES
 *********************/

#include <stdint.h>

/**********************
 * GLOBAL PROTOTYPES
 **********************/

uint16_t crc16(uint8_t *buffer, 
	uint16_t buffer_length);

#endif /*__CRC16_H__*/
