/**
 * @file stm32.h
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "stm32.h"

#include "gpio.h"
#include "spi.h"
#include "dma.h"
#include "time.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initializes the device's core clock in preparation for startup.
 * The initialization frequency is 168 MHZ.
 */
void stm32_init()
{
    /*Initialize all configured peripherals*/
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_SPI3_Init();

    /*Reset both DRV chips. The enable pin also controls the SPI interface, not*/
    /*only the driver stages.*/
    /*drv_enable_gpio.write(false);*/
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
    sleep_ms(1); /*Mimumum pull-down time for full reset: 20us*/

    /*drv_enable_gpio.write(true);*/
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
    sleep_ms(20); /*Mimumum pull-down time for full reset: 20us*/
}
