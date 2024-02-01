/**
 * @file mb.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "mb.h"

/*********************
 *      DEFINES
 *********************/

#define REG_ADDR_START 40001U
#define REG_COUNT      40U

/**********************
 *  STATIC VARIABLES
 **********************/

static uint16_t data_buf[REG_COUNT] = {
    1900, 1901, 1902, 1903, 1904, 
    1905, 1906, 1907, 1908, 1909, 
    1910, 1911, 1912, 1913, 1914, 
    1915, 1916, 1917, 1918, 1919,
    1920, 1921, 1922, 1923, 1924, 
    1925, 1926, 1927, 1928, 1929, 
    1930, 1931, 1932, 1933, 1934, 
    1935, 1936, 1937, 1938, 1939,
};

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

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
    (*pdu_data_len) += 1;

    return mb_rtu_read_data(
        pdu_data_frame_p, 
        pdu_data_len, 
        read_addr, 
        read_num
    );
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
            (data_buf[index + i] >> 8) & 0xFF;

        (*pdu_data_len) += 1;

        pdu_data_frame_p[*pdu_data_len] = \
            (data_buf[index + i] >> 0) & 0xFF;

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
    uint16_t write_num = 1;

    write_addr =  (pdu_data_frame_p[0] << 8);
    write_addr |= (pdu_data_frame_p[1] << 0);

    return mb_rtu_write_data(&pdu_data_frame_p[2], 
        pdu_data_len, write_addr, 
        write_num);
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
        data_buf[index]  = \
            pdu_data_frame_p[i + 0] << 8;
        data_buf[index] |= \
            pdu_data_frame_p[i + 1] << 0;
    }

#if 0 /*Example*/
    motor_control.mode_run = \
        motor_mode_digital_speed;
    motor_control_write_goal_speed(
        51200 * data_buf[index]); /*0.1 Cycle/s*/
#endif /*Example*/

    return MB_RES_NONE;
}
