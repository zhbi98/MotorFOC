
#ifndef __MB_H__
#define __MB_H__

#include "mbrtu.h"

typedef enum {
    MB_EX_NONE = 0x00,
    MB_EX_ILLEGAL_FUNCTION = 0x01,
    MB_EX_ILLEGAL_DATA_ADDRESS = 0x02,
    MB_EX_ILLEGAL_DATA_VALUE = 0x03,
    MB_EX_SLAVE_DEVICE_FAILURE = 0x04,
    MB_EX_ACKNOWLEDGE = 0x05,
    MB_EX_SLAVE_BUSY = 0x06,
    MB_EX_MEMORY_PARITY_ERROR = 0x08,
    MB_EX_GATEWAY_PATH_FAILED = 0x0A,
    MB_EX_GATEWAY_TGT_FAILED = 0x0B
} MB_ExceptionTypeDef;

extern MB_ExceptionTypeDef MB_RtuReadRegData(unsigned char * pduDataFrame, unsigned short * pduDataLen);
extern MB_ExceptionTypeDef MB_RtuReadData(unsigned char * pduDataFrame, unsigned short * pduDataLen, unsigned short address, unsigned short num);

extern MB_ExceptionTypeDef MB_RtuWriteRegData(unsigned char * pduDataFrame, unsigned short * pduDataLen);
extern MB_ExceptionTypeDef MB_RtuWriteData(unsigned char * pduDataFrame, unsigned short * pduDataLen, unsigned short address, unsigned short num);

#endif
