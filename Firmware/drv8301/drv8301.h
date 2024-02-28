/**
 * @file drv8301.h
 *
 */

#ifndef __DRV8301_H__
#define __DRV8301_H__

/*********************
 *      INCLUDES
 *********************/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**
 * Contains fault information for status registers 1 and status 2.
 * where FAULT_GVDD_OV is the failure of status register 2.
 */
enum {
    FAULT_NOFAULT  = (0 <<  0), /**< No fault*/
    FAULT_FETLC_OC = (1 <<  0), /**< FET low side, phase C over current fault*/
    FAULT_FETHC_OC = (1 <<  1), /**< FET high side, phase C over current fault*/
    FAULT_FETLB_OC = (1 <<  2), /**< FET low side, phase B over current fault*/
    FAULT_FETHB_OC = (1 <<  3), /**< FET high side, phase B over current fault*/
    FAULT_FETLA_OC = (1 <<  4), /**< FET low side, phase A over current fault*/
    FAULT_FETHA_OC = (1 <<  5), /**< FET high side, phase A over current fault*/
    FAULT_OTW      = (1 <<  6), /**< Over temperature warning fault*/
    FAULT_OTSD     = (1 <<  7), /**< Over temperature shut down fault*/
    FAULT_PVDD_UV  = (1 <<  8), /**< Power supply Vdd under voltage fault*/
    FAULT_GVDD_UV  = (1 <<  9), /**< DRV8301 Vdd under voltage fault*/
    FAULT_FAULT    = (1 << 10), /**< DRV8301 Vdd over voltage fault*/
    FAULT_GVDD_OV  = (1 << 23)  /**< DRV8301 Vdd over voltage fault*/
};

/*The fault type of the MOS driver IC*/
typedef uint16_t md_fault_t;

/**
 * DRV8301 register address, DRV8301 There are 
 * four registers, two of which are control registers and 
 * two of which are status registers.
 */
enum {
    ADDR_REG_STA1 = 0x00 << 11, /**< DRV8301 Status Register 1*/
    ADDR_REG_STA2 = 0x01 << 11, /**< DRV8301 Status Register 2*/
    ADDR_REG_CTL1 = 0x02 << 11, /**< DRV8301 Control Register 1*/
    ADDR_REG_CTL2 = 0x03 << 11  /**< DRV8301 Control Register 2*/
};

typedef uint16_t md_addr_t;

/**
 * DRV8301 Register configuration status
 * and is available via a API.
 */
enum {
    STATE_UNINITED = 0,
    STATE_CHECKS,
    STATE_READY,
};

typedef uint8_t md_state_t;

/**
 * for the drv8301's SPI R/W modes.
 */
enum {
    READ  = 1 << 15, /**< Read Mode*/
    WRITE = 0 << 15  /**< Write Mode*/
};

typedef uint16_t md_RW_t;

/*Dev Select the I/O operation portal*/
typedef void (*_setval_t)(bool val);
/*Dev Select the I/O operation portal*/
typedef bool (*_readval_t)();

/**
 * Multiple DRV8301 involve a common pin definition that 
 * makes it easy to drive the pins of a given object.
 */
typedef struct {
    _setval_t setval;
    _readval_t readval;
} md_pin_t;

/**
 * Used to store the configuration values 
 * of the DRV8301 control registers.
 */
typedef struct {
    uint16_t reg1data;
    uint16_t reg2data;
} md_regctl_t;

/**
 * Object-oriented design, construct a MOSFET-Driven 
 * device descriptor to store device parameters
 */
typedef struct {
    uint8_t init_state;
    md_regctl_t regctl;
    md_pin_t pin_nfalt; /**< Multiple DRV8301common pin*/
    md_pin_t pin_cs;    /**< Multiple DRV8301 common pin*/
    md_pin_t pin_en;    /**< Multiple DRV8301 common pin*/
} md_drv8301_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Transmit and Receive an amount of data in non-blocking mode with DMA.
 * @param  tx_buf pointer to transmission data buffer
 * @param  rx_buf pointer to reception data buffer
 * @param  Size amount of data to be sent
 * @return HAL status.
 */
void md_drv8301_spi_transfer_async(md_drv8301_t * drv8301_p, 
    const uint8_t * tx_buf, uint8_t * rx_buf, 
    size_t length);

/**
 * Transmit and Receive an amount of data in non-blocking mode with DMA.
 * @param  tx_buf pointer to transmission data buffer
 * @param  rx_buf pointer to reception data buffer
 * @param  Size amount of data to be sent
 * @return HAL status.
 */
bool md_drv8301_spi_transfer(md_drv8301_t * drv8301_p, 
    const uint8_t* tx_buf, uint8_t* rx_buf, 
    size_t length, uint32_t timeout_ms);

/**
 * SPI Transmit and Receive completed callback.
 * act_drv_p pointer to a The currently active drive object.
 */
void md_drv8301_spi_transfer_end();

bool md_drv8301_register_config(md_drv8301_t * drv8301_p, 
    float requested_gain, float * actual_gain);

/**
 * Apply the changed configuration parameters to the actual DRV8301 device.
 * @param drv8301_p pointer to DRV8301 that are currently 
 * in the configuration phase.
 * @return Configuration result.
 */
bool md_drv8301_register_init(md_drv8301_t * drv8301_p);

/**
 * Writes data to a DRV8301 register. There is no check if the write succeeded.
 * @param drv8301_p pointer to DRV8301 that are currently 
 * in the configuration phase.
 * @return Configuration result.
 */
bool md_drv8301_write_reg(md_drv8301_t * drv8301_p, 
    uint16_t reg_addr, uint16_t data);

/**
 * Reads data from a DRV8301 register.
 * @param drv8301_p pointer to DRV8301 that are currently 
 * in the configuration phase.
 * @return Configuration result.
 */
bool md_drv8301_read_reg(md_drv8301_t * drv8301_p, 
    uint16_t reg_addr, uint16_t * data_p);

/**
 * Assemble data fields, build data.
 * @param drv8301_p pointer to DRV8301 that are currently 
 * in the configuration phase.
 * @return Build the field.
 */
uint16_t build_spi_data(uint16_t _mode, uint16_t reg_addr, 
    uint16_t data);

/**
 * Transmit and Receive an amount of data in non-blocking mode with DMA.
 * @param drv8301_p pointer to DRV8301 that are currently 
 * in the configuration phase.
 * @return Failure field.
 */
md_fault_t md_drv8301_get_error(md_drv8301_t * drv8301_p);

/**
 * Monitors the nFAULT pin. This must be run at an interval of <8ms from the moment the init()
 * functions starts to run, otherwise it's possible that a temporary power
 * loss is missed, leading to unwanted register values.
 * In case of power loss the nFAULT pin can be low for as little as 8ms.
 * @param drv8301_p pointer to DRV8301 that are currently 
 * in the configuration phase.
 */
void md_drv8301_checks(md_drv8301_t * drv8301_p);

/**
 * Returns true if and only if the DRV8301 chip is in an initialized
 * state and ready to do switching and current sensor opamp operation.
 * @param drv8301_p pointer to DRV8301 that are currently 
 * in the configuration phase.
 * @return is ready.
 */
bool md_drv8301_ready(md_drv8301_t * drv8301_p);

#endif /*__DRV8301_H__*/
