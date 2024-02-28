/**
 * @file mbrtu.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include <assert.h>
#include "mbrtu.h"

/*********************
 *      DEFINES
 *********************/

#define MB_SEND_BYTE(buf, len)

/**
 * rs485_receive_disable(); \
 * while (HAL_UART_Transmit( \
 *         &Uart1Handle, \
 *         buf, len, \
 *         0XFFFF \
 *     ) != HAL_OK \
 * ); \
 * rs485_receive_enable();
 */

/**********************
 *  STATIC VARIABLES
 **********************/

static uint8_t  rtu_buf[RTU_BUF_MAX] = {0};
static uint16_t rtu_len = 0;

static uint8_t  init_slave_addr = 0x01;
static uint8_t  recv_slave_addr = 0x00;
static uint16_t recv_fun_code   = 0x00;

static uint8_t  pdu_data[RTU_BUF_MAX - 4];
static uint16_t data_len = 0;

static mb_rx_state_t mb_rx_state = MB_RX_INIT;
static mb_tx_state_t mb_tx_state = MB_TX_IDLE;
static mb_task_t mb_task = MB_READY;

/**********************
 *      TYPEDEFS
 **********************/

/**
 * The construct protocol request actually handles the interface object.
 * Function code and operational interface.
 */
typedef struct {
    uint8_t fun_code;
    req_opi_t req_opi;
} mb_req_ll_t;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initializes the slave device, sets the unique slave device address to which the 
 * slave device belongs, and the device communication baud rate.
 * @param slave_addr Unique slave device address.
 * @param baud_rate Device communication baud rate.
 */
void mb_rtu_mode_init(uint8_t slave_addr, 
    uint32_t baud_rate)
{
    if ((slave_addr < ADDRESS_MIN) || 
        (slave_addr > ADDRESS_MAX))
        return;

    init_slave_addr = slave_addr;
    mb_timer_init(baud_rate);

    mb_rx_state = MB_RX_INIT;
    mb_timer_enable();

    /*Modbus RTU uses 8 Databits.*/
}

/**
 * Initially the receiver is in the state STATE_RX_INIT. we start
 * the timer and if no character is received within t3.5 we change
 * to STATE_RX_IDLE. This makes sure that we delay startup of the
 * modbus protocol stack until the bus is free.
 */
void mb_rtu_start()
{
    mb_rx_state = MB_RX_INIT;
    mb_tx_state = MB_TX_IDLE;
    mb_timer_enable();
}

/**
 * Initially the receiver is in the state MB_RX_IDLE. we start
 * the timer and if no character is received within t3.5 we change
 * to STATE_RX_IDLE. This makes sure that we delay startup of the
 * modbus protocol stack until the bus is free.
 */
void mb_rtu_stop()
{
    mb_timer_disable();
    mb_rx_state = MB_RX_IDLE;
    mb_tx_state = MB_TX_IDLE;
    mb_task = MB_END;
}

/**
 * Set the device from the device address, 
 * generally used to temporarily change the address of the device.
 * @param slave_addr Unique slave device address.
 * @return Whether the settings take effect.
 */
bool mb_rtu_set_slave_addr(uint8_t slave_addr)
{
    if ((slave_addr >= ADDRESS_MIN) && 
        (slave_addr <= ADDRESS_MAX)
    ) {
        init_slave_addr = slave_addr;
        return true;
    }
    return false;
}

/**
 * The realization of Modbus protocol data transmission.
 * @param data_p Points to an area of the cached data.
 * @param len Number of bytes in the frame of data.
 */
void mb_rtu_send_bytes(uint8_t * data_p, uint16_t len)
{
    MB_SEND_BYTE(data_p, len);
}

/**
 * The underlying interface of Modbus protocol data receiving is realized, 
 * which is used in the interrupt of serial interface data receiving,
 * always read the character.
 * @param byte Padding data byte by byte into a data frame buffer.
 */
void mb_rtu_recv_bytes(uint8_t byte)
{
    /*A new frame of data is received when the controller is idle*/
    assert(mb_tx_state == MB_TX_IDLE);
    
    switch (mb_rx_state) {
    /**
     * If we have received a character in the init state we have to
     * wait until the frame is finished.
     */
    case MB_RX_INIT:
        mb_timer_enable();
        break;

    /**
     * In the error state we wait until all characters in the
     * damaged frame are transmitted.
     */
    case MB_RX_ERR:
        mb_timer_enable();
        break;

    /**
     * In the idle state we wait for a new character. If a character
     * is received the t1.5 and t3.5 timers are started and the
     * receiver is in the state STATE_RX_RECEIVCE.
     */
    case MB_RX_IDLE:
        rtu_len = 0; /*Empty the byte count in preparation for the next frame of data*/
        mb_rx_state = MB_RX_RCV;
        /*Padding data byte by byte into a data frame buffer*/
        if (rtu_len < RTU_BUF_MAX) {
            rtu_buf[rtu_len] = byte;
            rtu_len++;
            mb_timer_reload();
        }
        break;

    /**
     * We are currently receiving a frame. Reset the timer after
     * every character received. If more than the maximum possible
     * number of bytes in a modbus frame is received the frame is
     * ignored.
     */
    case MB_RX_RCV:
        /*Padding data byte by byte into a data frame buffer*/
        if (rtu_len < RTU_BUF_MAX) {
            rtu_buf[rtu_len] = byte;
            rtu_len++;
            mb_timer_reload();
        } else {
            mb_rx_state = MB_RX_ERR;
            mb_timer_reload();
        }
        break;
    }
}

/**
 * Get the redundancy verify value in the data, 
 * judge whether the received data frame is incorrect (valid).
 * @return Data frame verification result.
 */
uint8_t mb_rtu_frame_valid()
{
    uint16_t _crc16 = 0xFFFF;

    assert(rtu_len < RTU_BUF_MAX);

    if (rtu_len >= RTU_BUF_MIN) {
        _crc16 = crc16(rtu_buf, rtu_len);
        if (_crc16 == 0x0000)
            return true;
    }
    return false;
}

/**
 * Whether the slave device address specified by the main device is the address of this device, 
 * If the device address is processed for the data frame request.
 * @return The result of a match.
 */
uint8_t mb_rtu_slave_addr_valid()
{
    uint8_t addr = rtu_buf[SLAVE_ADDR_INDEX];

    if ((addr == init_slave_addr) || 
        (addr == BROADCAST_ADDRESS)
    ) {
        /* Save the address field. All frames are passed to the upper layed
         * and the decision if a frame is used is done there.
         */
        recv_slave_addr = addr;
        return true;
    }
    return false;
}

/**
 * Extracts the slave device address contained in 
 * the data frame sent by the master device.
 * @return slave device address.
 */
uint8_t mb_rtu_read_slave_addr()
{
    return recv_slave_addr;
}

/**
 * Extract the PDU message and function code to be executed 
 * from the complete data frame sent by the main device.
 */
void mb_rtu_read_pdu_data_frame()
{
    data_len = rtu_len;

    /*Total length of Modbus-PDU 'Data field' 
    is Modbus-Serial-Line-PDU minus size of address field 
    and CRC checksum, fun code.*/
    data_len -= SLAVE_ADDR_BYTE_SIZE;
    data_len -= CRC_BYTE_SIZE;
    data_len -= FUNCODE_BYTE_SIZE;

    recv_fun_code = \
        rtu_buf[FUN_CODE_INDEX];

    memcpy(pdu_data, 
        &rtu_buf[PDU_DATA_INDEX], 
        data_len);
}

/**
 * Extract the function code to be executed.
 */
uint8_t mb_rtu_read_pdu_fun_code()
{
    return recv_fun_code;
}

/**
 * The complete parsing of the data frame sent by the main device, 
 * and processing of the request of the main device, 
 * the function placed in a main loop repeated execution.
 */
void mb_rtu_pdu_field_deal()
{
    /*Check if the protocol stack is ready.*/
    if (mb_task == MB_END) return;

    uint8_t valid = 0;
    uint8_t _valid = 0;
    uint8_t code = 0;

    switch (mb_task) {
    case MB_READY:
        break;

    case MB_FRAME_RECEIVED:

        /*Length and CRC check*/
        valid = mb_rtu_frame_valid();

        /*Check if the frame is for us. If not ignore the frame.*/
        _valid = mb_rtu_slave_addr_valid();

        if ((valid) && (_valid)) {
            mb_rtu_read_pdu_data_frame();
            mb_task = MB_EXECUTE;
        } else {
            rtu_len = 0;
            mb_task = MB_END;
            mb_rx_state = MB_RX_IDLE;
        }
        break;

    case MB_EXECUTE:
        code = mb_rtu_read_pdu_fun_code();

        mb_rtu_fun_handlers(
            code, pdu_data, 
            &data_len);

        mb_rtu_build_send_frames(
            pdu_data, data_len);

        mb_task = MB_FRAME_SENT;
        break;

    case MB_FRAME_SENT:
        mb_rtu_send_bytes(
            rtu_buf, rtu_len);

        mb_tx_state = MB_TX_IDLE;

        rtu_len = 0;
        mb_rx_state = MB_RX_IDLE;

        mb_task = MB_END;
        break;

    case MB_END:
        break;
    }
}

/*Defines the number of operational interfaces*/
#define FUN_HANDLER_MAX 5U

/**
 * The actual function processing/operation interface is added here, 
 * pay attention to the proper use of Modbus standard function code, 
 * comply with the industry-wide function code allocation standards.
 */
static mb_req_ll_t req_lls[FUN_HANDLER_MAX] = {0};
static uint8_t req_cnt = 0;

/**
 * Add a actual operation callback function to the callback list.
 * @param _code The interval between callback functions being executed.
 * @param req_p pointer to a task callback function.
 * @return The result of a match.
 */
bool _mb_rtu_xcall_register(const uint8_t _code, req_opi_t req_p)
{
    while ((req_lls[req_cnt].req_opi != NULL) && 
        (req_cnt < FUN_HANDLER_MAX)) req_cnt++;

    if (req_cnt > FUN_HANDLER_MAX) return false;

    req_lls[req_cnt].req_opi = req_p;
    req_lls[req_cnt].fun_code = _code;

    return true;
}

/*_mb_rtu_xcall_register(0x03, mb_rtu_read_reg_data);*/
/*_mb_rtu_xcall_register(0x10, mb_rtu_write_reg_data);*/

/**
 * The slave device performs the corresponding request according 
 * to the function code specified by the master device.
 * @param pdu_data_frame_p Points to an area of the cached data frame 
 * that belongs to the receiving and sending shared space.
 * @param pdu_data_len This parameter describes the number of 
 * bytes in the frame of data received or to be sent.
 * @param fun_code function code specified by the master device.
 */
void mb_rtu_fun_handlers(uint8_t fun_code, 
    uint8_t * pdu_data_frame_p, 
    uint16_t * pdu_data_len)
{
    mb_res_t mb_res = MB_RES_NONE;

    for (uint8_t i = 0; i < FUN_HANDLER_MAX; i++) {

        uint8_t _fun_code = \
            req_lls[i].fun_code;

        if (_fun_code == fun_code) {
            mb_res = req_lls[i]\
                .req_opi(pdu_data_frame_p, 
                    pdu_data_len);
            break;
        } 
        else 
        /*No more function handlers registered. Abort.*/
        if (!_fun_code) {
            break;
        }
    }

    /* If the request was not sent to the broadcast address we
     * return a reply.*/
    if (recv_slave_addr != BROADCAST_ADDRESS)
    {
        /*An exception occured. Build an error frame.*/
        if (mb_res != MB_RES_NONE) {
            recv_fun_code = \
                recv_fun_code | (0x01 << 7);
            (*pdu_data_len) = 0;

            pdu_data_frame_p[*pdu_data_len] = mb_res;
            (*pdu_data_len) = 1;
        }
    }
}

/**
 * After the request is processed from the equipment, 
 * a standard message is constructed according to the corresponding data 
 * required by the main equipment to reply to the request of the main equipment.
 * @param pdu_data_frame_p Points to an area of the cached data frame 
 * that belongs to the receiving and sending shared space.
 * @param pdu_data_len This parameter describes the number of 
 * bytes in the frame of data received or to be sent.
 */
void mb_rtu_build_send_frames(uint8_t * pdu_data_frame_p, 
    uint16_t pdu_data_len)
{
    uint16_t _crc16 = 0x0000;

    /**
     * Check if the receiver is still in idle state. If not we where to
     * slow with processing the received frame and the master sent another
     * frame on the network. We have to abort sending the frame.
     */
    if (mb_rx_state != MB_RX_IDLE) return;

    rtu_len = 0;

    /*First byte before the Modbus-PDU is the slave address.*/
    rtu_buf[rtu_len] = \
        mb_rtu_read_slave_addr();
    rtu_len += 1;

    rtu_buf[rtu_len] = \
        mb_rtu_read_pdu_fun_code();
    rtu_len += 1;

    if ((pdu_data_len + 4) < RTU_BUF_MAX) {
        memcpy(&rtu_buf[rtu_len], 
            pdu_data_frame_p, 
            pdu_data_len);
        rtu_len += pdu_data_len;
    }

    /*Calculate CRC16 checksum for Modbus-Serial-Line-PDU.*/
    _crc16 = crc16(rtu_buf, rtu_len);

    rtu_buf[rtu_len] = \
        (uint8_t)(_crc16 & 0xFF);
    rtu_len += 1;

    rtu_buf[rtu_len] = \
        (uint8_t)(_crc16 >> 8);
    rtu_len += 1;

    /*Activate the transmitter.*/
    mb_tx_state = MB_TX_XMIT;
}

/**
 * Modbus protocol 3.5 character frame interval time realization, 
 * through the frame interval time break frame.
 * This function is placed in a timed interrupt to execute.
 */
void mb_rtu_T35_expired()
{
    /*The packet frame interval is up 
    and the bus status update begins*/

    switch (mb_rx_state) {
    /* Timer t35 expired. Startup phase is finished. */
    case MB_RX_INIT:
        mb_rx_state = MB_RX_IDLE;
        mb_task = MB_READY;
        break;
    /* A frame was received and t35 expired. Notify the listener that
     * a new frame was received.*/
    case MB_RX_RCV:
        mb_rx_state = MB_RX_END;
        mb_task = MB_FRAME_RECEIVED;
        break;
    /* An error occured while receiving the frame. */
    case MB_RX_ERR:
        break;
    }

    /*The execution here shows that the 3.5 frame interval time is up, 
    indicating the end of one frame of data*/
    mb_rx_state = MB_RX_IDLE;
}
