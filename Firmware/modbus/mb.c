
#include "mb.h"

#define REG_ADDR_START 40001
#define REG_COUNT      40

unsigned short dataBuf[REG_COUNT] = {
    1900, 1901, 1902, 1903, 1904, 1905, 1906, 1907, 1908, 1909, 
    1910, 1911, 1912, 1913, 1914, 1915, 1916, 1917, 1918, 1919,
    1920, 1921, 1922, 1923, 1924, 1925, 1926, 1927, 1928, 1929, 
    1930, 1931, 1932, 1933, 1934, 1935, 1936, 1937, 1938, 1939,
};

MB_ExceptionTypeDef MB_RtuReadRegData(unsigned char * pduDataFrame, unsigned short * pduDataLen)
{
    unsigned short readAddr = 0;
    unsigned short readNum = 0;

    readAddr =  (pduDataFrame[0] << 8);
    readAddr |= (pduDataFrame[1] << 0);

    readNum =  (pduDataFrame[2] << 8);
    readNum |= (pduDataFrame[3] << 0);

    (*pduDataLen) = 0;

    // 1data = 2bytes
    pduDataFrame[*pduDataLen] = readNum * 2;
    (*pduDataLen) += 1;

    return MB_RtuReadData(pduDataFrame, pduDataLen, readAddr, readNum);
}

MB_ExceptionTypeDef MB_RtuReadData(unsigned char * pduDataFrame, unsigned short * pduDataLen, unsigned short address, unsigned short num)
{
    if ((address < REG_ADDR_START) || ((address + num) > (REG_ADDR_START + REG_COUNT)))
        return MB_EX_ILLEGAL_DATA_ADDRESS;

    unsigned short index = (unsigned short)(address - REG_ADDR_START);
    for (unsigned short i = 0; i < num; i++) {
        pduDataFrame[*pduDataLen] = (dataBuf[index + i] >> 8) & 0xFF;
        (*pduDataLen) += 1;
        pduDataFrame[*pduDataLen] = (dataBuf[index + i] >> 0) & 0xFF;
        (*pduDataLen) += 1;
    }

    return MB_EX_NONE;
}

MB_ExceptionTypeDef MB_RtuWriteRegData(unsigned char * pduDataFrame, unsigned short * pduDataLen)
{
    unsigned short writeAddr = 0;
    unsigned short readNum = 1;

    writeAddr =  (pduDataFrame[0] << 8);
    writeAddr |= (pduDataFrame[1] << 0);

    return MB_RtuWriteData(&pduDataFrame[2], pduDataLen, writeAddr, readNum);
}

MB_ExceptionTypeDef MB_RtuWriteData(unsigned char * pduDataFrame, unsigned short * pduDataLen, unsigned short address, unsigned short num)
{
    if ((address < REG_ADDR_START) || ((address + num) > (REG_ADDR_START + REG_COUNT)))
        return MB_EX_ILLEGAL_DATA_ADDRESS;

    unsigned short index = (unsigned short)(address - REG_ADDR_START);
    for (unsigned short i = 0; i < num; i++) {
        dataBuf[index]  = pduDataFrame[i + 0] << 8;
        dataBuf[index] |= pduDataFrame[i + 1] << 0;
    }
#if 0
    motor_control.mode_run = Motor_Mode_Digital_Speed;
    Motor_Control_Write_Goal_Speed(51200 * dataBuf[index]); /*0.1 Cycle/s*/
#endif
    return MB_EX_NONE;
}
