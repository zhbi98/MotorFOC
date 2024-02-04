/**
 * @file mbrtu.h
 *
 */

#ifndef __MBRTU_H__
#define __MBRTU_H__

/*********************
 *      INCLUDES
 *********************/

#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "crc16.h"
#include "mbtimer.h"
#include "mb.h"

/*********************
 *      DEFINES
 *********************/

/**
 * Constants which defines the format of a modbus frame. The example is
 * shown for a Modbus RTU/ASCII frame. Note that the Modbus PDU is not
 * dependent on the underlying transport.
 *
 *  |<----------------------- MODBUS RECV BUF  (1) ----------------------->|
 *  |               |<------- MODBUS PDU FIELD (2) --------->|             |
 *  +---------------+---------------+------------------------+-------------+
 *  | Slave Address | Function Code |   DataLength+Data      |   CRC/LRC   |
 *  +---------------+---------------+------------------------+-------------+
 *  (3)             (4)             (5)                      (6)
 * 
 * (1)  ... RECV_BUF_MAX  = 256
 *
 * (2)  ... PDU_FIELD_MAX = 253 (256 - SLAVE ADDR - CRC16)
 * 
 * (3)  ... SLAVE_ADDRESS_MAX = 1
 * (4)  ... FUNCTION_CODE = 1
 * (6)  ... CRC/LRC = 2
 */

#define RTU_BUF_MIN 4U
#define RTU_BUF_MAX 256U

#define BROADCAST_ADDRESS 0U
#define ADDRESS_MIN 1U
#define ADDRESS_MAX 247U

#define SLAVE_ADDR_BYTE_SIZE 1U
#define FUNCODE_BYTE_SIZE 1U
#define CRC_BYTE_SIZE 2U

#define SLAVE_ADDR_INDEX 0U
#define FUN_CODE_INDEX 1U
#define PDU_DATA_INDEX 2U

/**********************
 *      TYPEDEFS
 **********************/

typedef mb_res_t (*req_opi_t)(uint8_t * pdu_data_frame_p, 
    uint16_t * pdu_data_len);

/**Protocol Stack Controller receive state*/
enum {
    MB_RX_INIT = 0,
    MB_RX_IDLE,
    MB_RX_RCV,
    MB_RX_END,
    MB_RX_ERR,
};

typedef uint8_t mb_rx_state_t;

/**Protocol Stack Controller transfer state*/
enum {
    MB_TX_IDLE = 0, /**< Transmitter is in idle state*/
    MB_TX_XMIT /**< Transmitter is in transfer state*/
};

typedef uint8_t mb_tx_state_t;

/**Protocol Stack Controller task type*/
enum {
    MB_READY = 0,      /**< Startup finished.*/
    MB_FRAME_RECEIVED, /**< Frame received.*/
    MB_EXECUTE,        /**< Execute function.*/
    MB_FRAME_SENT,     /**< Frame sent.*/
    MB_END             /**< Work end.*/
};

typedef uint8_t mb_task_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void mb_rtu_mode_init(uint8_t slave_addr, uint32_t baud_rate);
void mb_rtu_start();
void mb_rtu_stop();
bool mb_rtu_set_slave_addr(uint8_t slave_addr);
void mb_rtu_send_bytes(uint8_t * data_p, uint16_t len);
void mb_rtu_recv_bytes(uint8_t byte);
uint8_t mb_rtu_frame_valid();
uint8_t mb_rtu_slave_addr_valid();
uint8_t mb_rtu_read_slave_addr();
void mb_rtu_read_pdu_data_frame();
uint8_t mb_rtu_read_pdu_fun_code();
void mb_rtu_pdu_field_deal();
bool _mb_rtu_xcall_register(const uint8_t _code, req_opi_t req_p);
void mb_rtu_fun_handlers(uint8_t fun_code, uint8_t * pdu_data_frame_p, uint16_t * pdu_data_len);
void mb_rtu_build_send_frames(uint8_t * pdu_data_frame, uint16_t pdu_data_len);
void mb_rtu_T35_expired();

#endif /*__MBRTU_H__*/
