
#include "mbrtu.h"

unsigned char  rtuBuf[RTU_BUF_MAX] = {0};
unsigned short rtuLen = 0;

unsigned char  initSlaveAddr = 0x01;
unsigned char  recvSlaveAddr = 0x00;
unsigned short recvFunCode   = 0x00;

unsigned char  pduData[RTU_BUF_MAX - 4];
unsigned short dataLen;

MB_StateTypeDef mbState = MB_STATE_INIT;

void MB_RtuModeInit(unsigned char slaveAddr, unsigned int baudRate)
{
    if ((slaveAddr < ADDRESS_MIN) || (slaveAddr > ADDRESS_MAX))
        return;

    initSlaveAddr = slaveAddr;
    MB_TimerInit(baudRate);

    mbState = MB_STATE_INIT;
    MB_TimerEnable();
}

void MB_SetSlaveAddr(unsigned char slaveAddr)
{
    if ((slaveAddr >= ADDRESS_MIN) && (slaveAddr <= ADDRESS_MAX))
        initSlaveAddr = slaveAddr;
}

void MB_RtuSendBytes(unsigned char * buf, unsigned short len)
{
#if 0
    rs485_receive_disable();
    while (
        HAL_UART_Transmit(
            &Uart1Handle, buf, len, 0XFFFF
        ) != HAL_OK
    );
    rs485_receive_enable();
#endif
}

void MB_RtuRecvBytes(unsigned char byte)
{
    if ((mbState == MB_STATE_IDLE)) {
        mbState = MB_STATE_RECV;
        rtuLen = 0;
        if (rtuLen < RTU_BUF_MAX) {
            rtuBuf[rtuLen] = byte;
            rtuLen++;
            MB_TimerReload();
        }
    } else if (mbState == MB_STATE_RECV) {
        if (rtuLen < RTU_BUF_MAX) {
            rtuBuf[rtuLen] = byte;
            rtuLen++;
            MB_TimerReload();
        }
    }
}

unsigned char MB_RtuFrameValid()
{
    unsigned short _crc16 = 0xFFFF;

    if (rtuLen >= RTU_BUF_MIN) {
        _crc16 = crc16(rtuBuf, rtuLen);
        if (_crc16 == 0x0000)
            return true;
    }
    return false;
}

unsigned char MB_RtuSlaveAddrValid()
{
    unsigned char addr = rtuBuf[SLAVE_ADDR_INDEX];

    if ((addr == initSlaveAddr) || (addr == BROADCAST_ADDRESS)) {
        recvSlaveAddr = addr;
        return true;
    }
    return false;
}

unsigned char MB_RtuReadSlaveAddr()
{
    return recvSlaveAddr;
}

void MB_RtuReadPduDataFrame()
{
    dataLen = rtuLen - SLAVE_ADDR_BYTE_SIZE - CRC_BYTE_SIZE - FUNCODE_BYTE_SIZE;
    recvFunCode = rtuBuf[FUN_CODE_INDEX];
    memcpy(pduData, &rtuBuf[PDU_DATA_INDEX], dataLen);
}

unsigned char MB_RtuReadPduFunCode()
{
    return recvFunCode;
}

void MB_RtuPduFieldDeal()
{
    if (mbState != MB_STATE_RECEIVEED)
        return;

    if ((MB_RtuFrameValid()) && (MB_RtuSlaveAddrValid())) {

        MB_RtuReadPduDataFrame();

        MB_RtuFunHandlers(MB_RtuReadPduFunCode(), pduData, &dataLen);
        MB_RtuBuildSendFrames(pduData, dataLen);
        MB_RtuSendBytes(rtuBuf, rtuLen);

        mbState = MB_STATE_IDLE;
        rtuLen = 0;
    } else {
        mbState = MB_STATE_IDLE;
        rtuLen = 0;
    }
}

#define FUN_HANDLER_MAX 5

typedef MB_ExceptionTypeDef (*pduDatahandle)(unsigned char * pduDataFrame, unsigned short * pduDataLen);

typedef struct {
    unsigned char funCode;
    pduDatahandle handle;
} MB_PduDataHandleTypeDef;

MB_PduDataHandleTypeDef pduDataHandles[FUN_HANDLER_MAX] = {
    {0x03, MB_RtuReadRegData},
    {0x06, MB_RtuWriteRegData},
};

void MB_RtuFunHandlers(unsigned char funCode, unsigned char * pduDataFrame, unsigned short * pduDataLen)
{
    MB_ExceptionTypeDef mbExcept = MB_EX_NONE;
    for (unsigned char i = 0; i < FUN_HANDLER_MAX; i++) {
        if (funCode == pduDataHandles[i].funCode) {
            mbExcept = pduDataHandles[i].handle(pduDataFrame, pduDataLen);
            break;
        }
    }

    if (mbExcept != MB_EX_NONE) {
        recvFunCode = recvFunCode | (0x01 << 7);
        (*pduDataLen) = 0;
        pduDataFrame[*pduDataLen] = mbExcept;
        (*pduDataLen) = 1;
    }
}

void MB_RtuBuildSendFrames(unsigned char * pduDataFrame, unsigned short pduDataLen)
{
    unsigned short _crc16 = 0x0000;

    rtuLen = 0;

    rtuBuf[rtuLen] = MB_RtuReadSlaveAddr();
    rtuLen += 1;
    rtuBuf[rtuLen] = MB_RtuReadPduFunCode();
    rtuLen += 1;

    if ((pduDataLen + 4) < RTU_BUF_MAX) {
        memcpy(&rtuBuf[rtuLen], pduDataFrame, pduDataLen);
        rtuLen += pduDataLen;
    }

    _crc16 = crc16(rtuBuf, rtuLen);
    rtuBuf[rtuLen] = (unsigned char)(_crc16 & 0xFF);
    rtuLen += 1;
    rtuBuf[rtuLen] = (unsigned char)(_crc16 >> 8);
    rtuLen += 1;
}
