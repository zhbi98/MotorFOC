/**
 * @file drv8301.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "drv8301.h"
#include "time.h"
#include "spi.h"

/**********************
 *  STATIC VARIABLES
 **********************/

volatile uint16_t tx_buf = 0xFFFF, rx_buf = 0xFFFF;
volatile static uint8_t spi_tran_ret = 0xFF;
static md_drv8301_t * act_drv_p = NULL;

/**********************
 *  STATIC PROTOTYPES
 **********************/

static bool md_drv8301_spi_start(md_drv8301_t * drv8301_p, 
    const uint8_t * tx_buf, uint8_t * rx_buf, 
    size_t length);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Transmit and Receive an amount of data in non-blocking mode with DMA.
 * @param  tx_buf pointer to transmission data buffer
 * @param  rx_buf pointer to reception data buffer
 * @param  Size amount of data to be sent
 * @return HAL status.
 */
static bool md_drv8301_spi_start(md_drv8301_t * drv8301_p, 
    const uint8_t * tx_buf, uint8_t * rx_buf, 
    size_t length)
{
    HAL_DMA_StateTypeDef rdma = hspi3.hdmarx->State;
    HAL_DMA_StateTypeDef tdma = hspi3.hdmatx->State;
    HAL_StatusTypeDef status = HAL_ERROR;

    drv8301_p->pin_cs.setval(false);

    /*This can happen if the DMA or interrupt 
    priorities are not configured properly.*/
    if (rdma != HAL_DMA_STATE_READY || 
        tdma != HAL_DMA_STATE_READY) {
        status = HAL_BUSY;
    } else if (tx_buf && rx_buf) {
        status = HAL_SPI_TransmitReceive_DMA(
            &hspi3, (uint8_t *)tx_buf, 
            rx_buf, length);
    } else if (tx_buf) {
        status = HAL_SPI_Transmit_DMA(&hspi3, 
            (uint8_t *)tx_buf, 
            length);
    } else if (rx_buf) {
        status = HAL_SPI_Receive_DMA(&hspi3, 
            rx_buf, length);
    }

    if (status != HAL_OK) {
        drv8301_p->pin_cs.setval(true);
    }

    return (status != HAL_OK) ? 0 : 1;
}

/**
 * Transmit and Receive an amount of data in non-blocking mode with DMA.
 * @param  tx_buf pointer to transmission data buffer
 * @param  rx_buf pointer to reception data buffer
 * @param  Size amount of data to be sent
 * @return HAL status.
 */
void md_drv8301_spi_transfer_async(md_drv8301_t * drv8301_p, 
    const uint8_t * tx_buf, uint8_t * rx_buf, 
    size_t length)
{
    bool res = md_drv8301_spi_start(drv8301_p, 
        tx_buf, rx_buf, length);

    if (!res) spi_tran_ret = false;
    /*Note that the complete interrupt must be 
    sent by the SPI to set the state to true*/
    /*else spi_tran_ret = true;*/
}

/**
 * Transmit and Receive an amount of data in non-blocking mode with DMA.
 * @param  tx_buf pointer to transmission data buffer
 * @param  rx_buf pointer to reception data buffer
 * @param  Size amount of data to be sent
 * @return HAL status.
 */
bool md_drv8301_spi_transfer(md_drv8301_t * drv8301_p, 
    const uint8_t* tx_buf, uint8_t* rx_buf, 
    size_t length, uint32_t timeout_ms)
{
    spi_tran_ret = 0xFF;
    act_drv_p = drv8301_p;

    md_drv8301_spi_transfer_async(drv8301_p, 
        tx_buf, rx_buf, length);

    /*Wait for the conversion to complete*/
    while (spi_tran_ret == 0xFF) {sleep_us(1);}

    return spi_tran_ret;
}

/**
 * SPI Transmit and Receive completed callback.
 * act_drv_p pointer to a The currently active drive object.
 */
void md_drv8301_spi_transfer_end()
{
    /*The data is sent, cancel the DRV8301 chip selection*/
    act_drv_p->pin_cs.setval(true);
    /*The data is sent and the state is set to true*/
    spi_tran_ret = true;
}

/**
 * Based on the actual combination of configuration parameters, 
 * a frame of configuration data is constructed and stored, 
 * where the data is not immediately updated to the device. 
 * If the gate driver was in ready state and the new 
 * configuration is different from the old one then 
 * the gate driver will exit ready state.
 * @param drv8301_p pointer to DRV8301 that are currently 
 * in the configuration phase.
 * @param requested_gain The value of the gain requested.
 * @param actual_gain Match to a close gain value.
 * @return Configuration result.
 */
bool md_drv8301_register_config(md_drv8301_t * drv8301_p, 
    float requested_gain, float * actual_gain)
{
    /*Calculate gain setting: Snap down to have equal or 
      larger range as requested or largest possible 
      range otherwise*/

    const float kgain_all[4] = {10.0f, 20.0f, 40.0f, 80.0f};
    uint16_t kgain_idx = 3;

    /**
     * For reference:
     * 20V/V on 500uOhm gives a range of +/- 150A
     * 40V/V on 500uOhm gives a range of +/- 75A
     * 20V/V on 666uOhm gives a range of +/- 110A
     * 40V/V on 666uOhm gives a range of +/- 55A
     */

    while (kgain_idx && (kgain_all[kgain_idx] > requested_gain)) {kgain_idx--;}
    if (actual_gain != NULL) *actual_gain = kgain_all[kgain_idx];

    md_regctl_t _regctl = {0};

    _regctl.reg1data = 
        /**< Overcurrent set to approximately 
        150A at 100 degC, this may need tweaking.*/
        (21 << 6) 
        /**< OCP_MODE: latch shut down*/
        | (0b01 << 4)
        /**< Driver mode 6x PWM mode*/  
        | (0b0 << 3)
        /**< Don't reset latched faults*/ 
        | (0b0 << 2)  
        /**< Gate-drive peak current: 1.7A*/ 
        | (0b00 << 0);

    _regctl.reg2data =
        /**< OC_TOFF: cycle by cycle*/
        (0b0 << 6)
        /**< Calibration off (normal operation)*/
        | (0b00 << 4)
        /**< Select shunt current amp gain*/
        | (kgain_idx << 2)
        /**< report both over temperature 
        and over current on nOCTW pin*/
        | (0b00 << 0);

    md_regctl_t regctl = drv8301_p->regctl;

    /*Check if register values have changed*/
    bool regs_equal = (
        regctl.reg1data == _regctl.reg1data
    ) && (
        regctl.reg2data == _regctl.reg2data
    );

    /*The new configuration is different 
    from the original configuration and requires 
    the configuration value to be updated*/
    if (!regs_equal) {
        drv8301_p->regctl = _regctl;
        drv8301_p->init_state = \
            STATE_UNINITED;
        drv8301_p->pin_en.setval(false);
    }

    return true;
}

/**
 * Apply the changed configuration parameters to the actual DRV8301 device.
 * @param drv8301_p pointer to DRV8301 that are currently 
 * in the configuration phase.
 * @return Configuration result.
 */
bool md_drv8301_register_init(md_drv8301_t * drv8301_p)
{
    uint16_t val = 0;

    if (drv8301_p->init_state == STATE_READY) return true;

    /*Reset DRV chip. The enable pin also controls the SPI interface, not only the driver stages.*/
    drv8301_p->pin_en.setval(false);
    sleep_us(40); /*mimumum pull-down time for full reset: 20us*/

    /*make is_ready() ignore transient errors 
    before registers are set up*/
    drv8301_p->init_state = STATE_UNINITED;

    drv8301_p->pin_en.setval(true);
    sleep_us(20); /*t_spi_ready, max = 10ms*/

    /*the write operation tends to be ignored if only done once (not sure why)*/
    bool wrote_regs = \
           md_drv8301_write_reg(drv8301_p, ADDR_REG_CTL1, 
            drv8301_p->regctl.reg1data) 
        && md_drv8301_write_reg(drv8301_p, ADDR_REG_CTL1, 
            drv8301_p->regctl.reg1data) 
        && md_drv8301_write_reg(drv8301_p, ADDR_REG_CTL1, 
            drv8301_p->regctl.reg1data) 
        && md_drv8301_write_reg(drv8301_p, ADDR_REG_CTL1, 
            drv8301_p->regctl.reg1data) 
        && md_drv8301_write_reg(drv8301_p, ADDR_REG_CTL1, 
            drv8301_p->regctl.reg1data) 
        && md_drv8301_write_reg(drv8301_p, ADDR_REG_CTL2, 
            drv8301_p->regctl.reg2data);

    /**
     * Of particular note is that kRegNameControl1 and regs_.control_register_1 
     * are repeated five times.This may mean that the code has repeatedly 
     * performed the write operation to ensure that the write operation 
     * succeeds. The note mentions: "If you only perform a write 
     * operation once, then the operation tends to be ignored (not sure why)".
     * This may be a specific behavior of the hardware or device and requires 
     * multiple writes to ensure that the configuration is valid.
     */

    if (!wrote_regs) return false;
    sleep_us(100); /*Wait for configuration to be applied*/
    drv8301_p->init_state = STATE_CHECKS;

    bool is_read_regs = \
        md_drv8301_read_reg(drv8301_p, ADDR_REG_CTL1, &val) && 
        (val == drv8301_p->regctl.reg1data) && 
        md_drv8301_read_reg(drv8301_p, ADDR_REG_CTL2, &val) && 
        (val == drv8301_p->regctl.reg2data);

    if (!is_read_regs) return false;

    md_fault_t fault = \
        md_drv8301_get_error(drv8301_p);

    if (fault != FAULT_NOFAULT) return false;

    /*There could have been an nFAULT edge meanwhile. 
    In this case we shouldn't consider the driver ready.*/
    if (drv8301_p->init_state == STATE_CHECKS)
        drv8301_p->init_state = STATE_READY;

    return drv8301_p->init_state == STATE_READY;
}

/**
 * Writes data to a DRV8301 register. There is no check if the write succeeded.
 * @param drv8301_p pointer to DRV8301 that are currently 
 * in the configuration phase.
 * @return Configuration result.
 */
bool md_drv8301_write_reg(md_drv8301_t * drv8301_p, 
    uint16_t reg_addr, uint16_t data)
{
    bool tran_res = 0;

    /*Do blocking write*/
    tx_buf = build_spi_data(WRITE, 
        reg_addr, data);

    tran_res = md_drv8301_spi_transfer(
        drv8301_p, (uint8_t *)(&tx_buf), 
        NULL, 1, 1000);

    if (!tran_res) return false;
    sleep_us(1);

    return true;
}

/**
 * Reads data from a DRV8301 register.
 * @param drv8301_p pointer to DRV8301 that are currently 
 * in the configuration phase.
 * @return Configuration result.
 */
bool md_drv8301_read_reg(md_drv8301_t * drv8301_p, 
    uint16_t reg_addr, uint16_t * data_p) 
{
    bool tran_res = 0;

    tx_buf = build_spi_data(READ, reg_addr, 0);

    tran_res = md_drv8301_spi_transfer(drv8301_p, 
        (uint8_t *)(&tx_buf), NULL, 1, 1000);

    if (!tran_res) return false;

    sleep_us(1);

    tx_buf = build_spi_data(READ, reg_addr, 0);
    rx_buf = 0xFFFF;

    tran_res = md_drv8301_spi_transfer(drv8301_p, 
        (uint8_t *)(&tx_buf), (uint8_t *)(&rx_buf), 
        1, 1000);

    if (!tran_res) return false;

    sleep_us(1);

    if (rx_buf == 0xBEEF) return false;

    if (data_p != NULL) *data_p = rx_buf & 0x07FF;

    return true;
}

/**
 * Assemble data fields, build data.
 * @param drv8301_p pointer to DRV8301 that are currently 
 * in the configuration phase.
 * @return Build the field.
 */
uint16_t build_spi_data(uint16_t _mode, uint16_t reg_addr, 
    uint16_t data)
{
    return _mode | reg_addr | (data & 0x07FF);
}

/**
 * Transmit and Receive an amount of data in non-blocking mode with DMA.
 * @param drv8301_p pointer to DRV8301 that are currently 
 * in the configuration phase.
 * @return Failure field.
 */
md_fault_t md_drv8301_get_error(md_drv8301_t * drv8301_p)
{
    uint16_t _flt = 0, flt = 0;

    bool res = md_drv8301_read_reg(
        drv8301_p, ADDR_REG_STA1, 
        &_flt);

    if (!res) return (md_fault_t)0xFFFFFFFF;

    res = md_drv8301_read_reg(
        drv8301_p, ADDR_REG_STA1, 
        &flt);

    if (!res) return (md_fault_t)0xFFFFFFFF;

    return (md_fault_t)((uint32_t)_flt | 
        ((uint32_t)(flt & 0x0080) << 16));
}

/**
 * Monitors the nFAULT pin. This must be run at an interval of <8ms from the moment the init()
 * functions starts to run, otherwise it's possible that a temporary power
 * loss is missed, leading to unwanted register values.
 * In case of power loss the nFAULT pin can be low for as little as 8ms.
 * @param drv8301_p pointer to DRV8301 that are currently 
 * in the configuration phase.
 */
void md_drv8301_checks(md_drv8301_t * drv8301_p)
{
    uint8_t init_state = drv8301_p->init_state;
    bool pin = drv8301_p->pin_nfalt.readval();

    if (init_state != STATE_UNINITED && !pin)
        init_state = STATE_UNINITED;
    drv8301_p->init_state = init_state;
}

/**
 * Returns true if and only if the DRV8301 chip is in an initialized
 * state and ready to do switching and current sensor opamp operation.
 * @param drv8301_p pointer to DRV8301 that are currently 
 * in the configuration phase.
 * @return is ready.
 */
bool md_drv8301_ready(md_drv8301_t * drv8301_p)
{
    uint8_t init_state = drv8301_p->init_state;
    return init_state == STATE_READY;
}
