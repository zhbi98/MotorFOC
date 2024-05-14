/**
 * @file devdesc.h
 *
 */

#ifndef __DEVDESC_H__
#define __DEVDESC_H__

/*********************
 *      INCLUDES
 *********************/

#include "gd32f30x.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/*********************
 *      DEFINES
 *********************/

#define REG_SN_START   0U
#define REG_SN_END     8U

#define REG_VER_START  8U
#define REG_VER_END   12U

#define REG_BD_START  12U
#define REG_BD_END    16U

#define REG_DTE_START 16U
#define REG_DTE_END   24U

#define REG_ID_START  24U
#define REG_ID_END    25U

/**********************
 *      TYPEDEFS
 **********************/

/**
 * Construct a device descriptor that contains all the information
 * Flash Is written by 4 bytes at a time, 
 * and if it is a structure, make sure each member 
 * is 4-byte aligned
 */
typedef struct {
	uint16_t sn[8];     /**< SN:RC2020010101*/
	uint16_t version[4]; /**< 1.0.1.0.0*/
	uint16_t build[4];   /**< 1024*/
	uint16_t date[8];   /**< 2020/05/01*/
	uint16_t slaveid __attribute__((aligned(4)));   /**< 0x2A*/
} xt_devdesc_t;

extern xt_devdesc_t devdesc;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initializes the device's core clock in preparation for startup.
 * The initialization frequency is 168 MHZ.
 */
void xt_devdesc_write_data();

/**
 * Initializes the device's core clock in preparation for startup.
 * The initialization frequency is 168 MHZ.
 */
void xt_devdesc_read_data();

/**
 * Initializes the device's core clock in preparation for startup.
 * The initialization frequency is 168 MHZ.
 */
bool verify(void * p1, void * p2, uint32_t n);

#endif /*__DEVDESC_H__*/
