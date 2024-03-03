/**
 * @file mb.h
 *
 */

#ifndef __MB_H__
#define __MB_H__

/*********************
 *      INCLUDES
 *********************/

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

/**********************
 *      TYPEDEFS
 **********************/

/**
 * Describes foreseeable errors in communication using the bus protocol, 
 * and error codes comply with common standards
 */
enum {
    MB_RES_NONE = 0x00,
    MB_RES_ILLEGAL_FUNCTION = 0x01,
    MB_RES_ILLEGAL_DATA_ADDRESS = 0x02,
    MB_RES_ILLEGAL_DATA_VALUE = 0x03,
    MB_RES_SLAVE_DEVICE_FAILURE = 0x04,
    MB_RES_ACKNOWLEDGE = 0x05,
    MB_RES_SLAVE_BUSY = 0x06,
    MB_RES_MEMORY_PARITY_ERROR = 0x08,
    MB_RES_GATEWAY_PATH_FAILED = 0x0A,
    MB_RES_GATEWAY_TGT_FAILED = 0x0B
};

typedef uint8_t mb_res_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

mb_res_t mb_rtu_read_reg_data(
    uint8_t * pdu_data_frame_p, 
    uint16_t * pdu_data_len);
mb_res_t mb_rtu_read_data(uint8_t * pdu_data_frame_p, 
    uint16_t * pdu_data_len, uint16_t address, uint16_t num);
mb_res_t mb_rtu_write_reg_data(uint8_t * pdu_data_frame_p, 
    uint16_t * pdu_data_len);
mb_res_t mb_rtu_write_data(uint8_t * pdu_data_frame_p, 
    uint16_t * pdu_data_len, uint16_t address, 
    uint16_t num);
void mb_rtu_reg_get_range(uint16_t * start, uint16_t * end);
void mb_rtu_reg_clear_range();
bool mb_rtu_reg_range_valid();

#endif /*__MB_H__*/
