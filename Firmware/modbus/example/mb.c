/**
 * @file mb.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "mb.h"
#include "devdesc.h"
#include "tran.h"
#include "log.h"

/*********************
 *      DEFINES
 *********************/

#define REG_ADDR_START 40001U
#define REG_COUNT      40U

/**********************
 *  STATIC VARIABLES
 **********************/

/**
 * Note that the Modbus RTU protocol uses 16-bit data transmission 
 * (whether it is a read-write coil, read discrete input, 
 * read input register, read-write hold register) , therefore, 
 * when using Modbus protocol to transfer multiple single-byte data needs to be combined 
 * into 16-bit (two-byte) data for transmission or storage, 
 * while paying attention to the combined byte order (size-side mode) .
 */

static uint16_t * const data_p[REG_COUNT] = {
    /*0*/
    &devdesc.sn[0], &devdesc.sn[1], &devdesc.sn[2], &devdesc.sn[3], &devdesc.sn[4], 
    &devdesc.sn[5], &devdesc.sn[6], &devdesc.sn[7], 

    /*8*/
    &devdesc.version[0], &devdesc.version[1], &devdesc.version[2], &devdesc.version[3], 

    &devdesc.build[0], &devdesc.build[1], &devdesc.build[2], &devdesc.build[3], 

    /*16*/
    &devdesc.date[0], &devdesc.date[1], &devdesc.date[2], &devdesc.date[3], &devdesc.date[4], 
    &devdesc.date[5], &devdesc.date[6], &devdesc.date[7], 

    /*24*/
    &devdesc.slaveid, 

    /*25*/
    &tran_dir, &_led3, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL, 
    NULL, NULL, NULL, NULL, NULL,
};

static uint16_t register_start_nr = 0xFFFF;
static uint16_t register_end_nr = 0xFFFF;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * After the Modbus master device accesses the slave device register, 
 * we can use this function to know the range of register addresses 
 * that the Modbus master device accesses.
 * The master device writes data into a specified register range, 
 * through which we can turn the data into events that 
 * perform data-binding actions
 * @param start Points to The initial register index.
 * @param end The Ending Register Address Index.
 */
void mb_rtu_reg_get_range(uint16_t * start, uint16_t * end)
{
    *start = register_start_nr;
    *end = register_end_nr;
}

/**
 * Clears events that have been handled within the address range of this register.
 * @param start Points to The initial register index.
 * @param end The Ending Register Address Index.
 */
void mb_rtu_reg_clear_range()
{
    register_start_nr = 0xFFFF;
    register_end_nr = 0xFFFF;
}

/**
 * Is the address range that produced the execution event valid, 
 * and whether the event generated in the register address range needs to be processed.
 * @param start Points to The initial register index.
 * @param end The Ending Register Address Index.
 * @return valid.
 */
bool mb_rtu_reg_range_valid()
{
    uint16_t start = register_start_nr;
    uint16_t end = register_end_nr;

    if (start == 0xFFFF || 
        end == 0xFFFF) return false;
    if (start > end ) return false;

    return true;
}

/**
 * Reads the data contained in the specified register from the device, 
 * and return to the main device.
 * @param pdu_data_frame_p Points to an area of the cached data frame 
 * that belongs to the receiving and sending shared space.
 * @param pdu_data_len This parameter describes the number of 
 * bytes in the frame of data received or to be sent.
 * @return Get the results from the data.
 */
mb_res_t mb_rtu_read_reg_data(
    uint8_t * pdu_data_frame_p, 
    uint16_t * pdu_data_len)
{
    uint16_t read_addr = 0;
    uint16_t read_num = 0;

    read_addr =  (pdu_data_frame_p[0] << 8);
    read_addr |= (pdu_data_frame_p[1] << 0);

    read_num =  (pdu_data_frame_p[2] << 8);
    read_num |= (pdu_data_frame_p[3] << 0);

    (*pdu_data_len) = 0;

    /*1data = 2bytes*/
    pdu_data_frame_p[*pdu_data_len] = read_num * 2;
    (*pdu_data_len) += 1; /*Offsset 1 byte*/

    mb_res_t res = mb_rtu_read_data(
        pdu_data_frame_p, 
        pdu_data_len, 
        read_addr, 
        read_num
    );

    if (res == MB_RES_NONE)
        *pdu_data_len = read_num * 2 + 1; /*data byte num + data num * 2*/

    return res;
}

/**
 * Reads the data contained in the specified register from the device, 
 * and return to the main device.
 * @param pdu_data_frame_p Points to an area of the cached data frame 
 * that belongs to the receiving and sending shared space.
 * @param pdu_data_len This parameter describes the number of 
 * bytes in the frame of data received or to be sent.
 * @param address The starting address of the register in which the data resides.
 * @param num Read the number of data, not the number of bytes.
 * @return Get the results from the data.
 */
mb_res_t mb_rtu_read_data(uint8_t * pdu_data_frame_p, 
    uint16_t * pdu_data_len, uint16_t address, 
    uint16_t num)
{
    uint32_t start = REG_ADDR_START;
    uint32_t end = REG_ADDR_START + REG_COUNT;

    /*Read the number of data, not the number of bytes*/

    if ((address < start) || ((address + num) > end))
        return MB_RES_ILLEGAL_DATA_ADDRESS;

    uint16_t index = (uint16_t)(
        address - REG_ADDR_START);

    for (uint16_t i = 0; i < num; i++) {
        pdu_data_frame_p[*pdu_data_len] = \
            (*data_p[index + i] >> 8) & 0xFF;

        (*pdu_data_len) += 1;

        pdu_data_frame_p[*pdu_data_len] = \
            (*data_p[index + i] >> 0) & 0xFF;

        (*pdu_data_len) += 1;
    }

    return MB_RES_NONE;
}

/**
 * Writes data to the specified slave device register.
 * @param pdu_data_frame_p Points to an area of the cached data frame 
 * that belongs to the receiving and sending shared space.
 * @param pdu_data_len This parameter describes the number of 
 * bytes in the frame of data received or to be sent.
 * @return Get the results from the data.
 */
mb_res_t mb_rtu_write_reg_data(uint8_t * pdu_data_frame_p, 
    uint16_t * pdu_data_len)
{
    uint16_t write_addr = 0;
    uint16_t write_num = 0;
    uint8_t byte_cnt = 0;

    write_addr =  (pdu_data_frame_p[0] << 8);
    write_addr |= (pdu_data_frame_p[1] << 0);

    write_num = (uint16_t)(pdu_data_frame_p[2] << 8);
    write_num |= (uint16_t)(pdu_data_frame_p[3]);

    byte_cnt = (uint8_t)(pdu_data_frame_p[4]); /*write_num * 2*/

    mb_res_t res = mb_rtu_write_data(&pdu_data_frame_p[5], 
        pdu_data_len, write_addr, 
        write_num);

    if (res == MB_RES_NONE)
        *pdu_data_len = 4; /*Address + write num*/

    return res;
}

/**
 * Writes data to the specified slave device register.
 * @param pdu_data_frame_p Points to an area of the cached data frame 
 * that belongs to the receiving and sending shared space.
 * @param pdu_data_len This parameter describes the number of 
 * bytes in the frame of data received or to be sent.
 * @param address The starting address of the register in which the data resides.
 * @param num Read the number of data, not the number of bytes.
 * @return Get the results from the data.
 */
mb_res_t mb_rtu_write_data(uint8_t * pdu_data_frame_p, 
    uint16_t * pdu_data_len, uint16_t address, 
    uint16_t num)
{
    uint32_t start = REG_ADDR_START;
    uint32_t end = REG_ADDR_START + REG_COUNT;

    if ((address < start) || ((address + num) > end))
        return MB_RES_ILLEGAL_DATA_ADDRESS;

    uint16_t index = (uint16_t)(
        address - REG_ADDR_START);

    for (uint16_t i = 0; i < num; i++) {
        *data_p[index + i]  = \
            pdu_data_frame_p[i * 2 + 0] << 8;
        *data_p[index + i] |= \
            pdu_data_frame_p[i * 2 + 1] << 0;
    }

    register_start_nr = index;
    register_end_nr = index + num;

    info("start:%d, end:%d", 
        register_start_nr, 
        register_end_nr
    );

    return MB_RES_NONE;
}
