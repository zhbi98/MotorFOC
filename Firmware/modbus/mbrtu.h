
#ifndef __MBRTU_H__
#define __MBRTU_H__

#include <stdbool.h>
#include <string.h>
#include "crc16.h"
#include "mbtimer.h"
#include "mb.h"
/*#include "usart.h"*/
/*#include "rs485.h"*/

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

#define RTU_BUF_MIN 4
#define RTU_BUF_MAX 256

#define BROADCAST_ADDRESS 0
#define ADDRESS_MIN 1
#define ADDRESS_MAX 247

#define SLAVE_ADDR_BYTE_SIZE 1
#define FUNCODE_BYTE_SIZE 1
#define CRC_BYTE_SIZE 2

#define SLAVE_ADDR_INDEX 0
#define FUN_CODE_INDEX 1
#define PDU_DATA_INDEX 2

typedef enum {
    MB_STATE_INIT,
    MB_STATE_IDLE,
    MB_STATE_RECV,
    MB_STATE_RECEIVEED,
} MB_StateTypeDef;

extern MB_StateTypeDef mbState;

extern void MB_RtuModeInit(unsigned char slaveAddr, unsigned int baudRate);
extern void MB_SetSlaveAddr(unsigned char slaveAddr);
extern void MB_RtuSendBytes(unsigned char * buf, unsigned short len);
extern void MB_RtuRecvBytes(unsigned char byte);
extern unsigned char MB_RtuFrameValid();
extern unsigned char MB_RtuSlaveAddrValid();
extern unsigned char MB_RtuReadSlaveAddr();
extern void MB_RtuReadPduDataFrame();
extern unsigned char MB_RtuReadPduFunCode();
extern void MB_RtuPduFieldDeal();
extern void MB_RtuFunHandlers(unsigned char funCode, unsigned char * pduDataFrame, unsigned short * pduDataLen);
extern void MB_RtuBuildSendFrames(unsigned char * pduDataFrame, unsigned short pduDataLen);

#endif
