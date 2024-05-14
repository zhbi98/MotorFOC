/**
 * @file devdesc.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "devdesc.h"
#include "xt_flash.h"
#include "log.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/

xt_devdesc_t devdesc = {0};

/*.sn = "AB0000000001",*/
/*.version = "1.0.0",*/
/*.build = "1024",*/
/*.date = "2020/10/24",*/
/*.slaveid = 0x2A,*/


/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initializes the device's core clock in preparation for startup.
 * The initialization frequency is 168 MHZ.
 */
void _devdesc()
{
    info("%s", (uint8_t *)devdesc.sn);
    info("%s", (uint8_t *)devdesc.version);
    info("%s", (uint8_t *)devdesc.build);
    info("%s", (uint8_t *)devdesc.date);
    info("%x", devdesc.slaveid);
}

/**
 * Initializes the device's core clock in preparation for startup.
 * The initialization frequency is 168 MHZ.
 */
void xt_devdesc_write_data()
{
	uint32_t ofs = sizeof(xt_devdesc_t);
	uint32_t * desc_p = (uint32_t *)(&devdesc);

    /*Operational memory erases sectors of 
    data to be written.*/
    xt_flash_erase_pages(FLASH_START_ADDR, 
    	FLASH_END_ADDR);

    /*Operational memory erases sectors of 
    data to be written.*/
    xt_flash_write(FLASH_START_ADDR, 
    	FLASH_START_ADDR + ofs, 
    	desc_p);

    _devdesc();
}

/**
 * Initializes the device's core clock in preparation for startup.
 * The initialization frequency is 168 MHZ.
 */
void xt_devdesc_read_data()
{
	uint32_t ofs = sizeof(xt_devdesc_t);
	uint32_t * desc_p = (uint32_t *)(&devdesc);

    /*Operational memory erases sectors of 
    data to be written.*/
    memset(&devdesc, '\0', sizeof(devdesc));

    /*Operational memory erases sectors of 
    data to be written.*/
    xt_flash_read(FLASH_START_ADDR, 
    	FLASH_START_ADDR + ofs, 
    	desc_p);

    _devdesc();
}

/**
 * A content observer to see if the data has changed, 
 * returning an error if the content is different 
 * from the original, or the correct result.
 * @param p1 pointer to the source data.
 * @param p2 pointer to the verify data.
 * @param n data size.
 * @return ture or false.
 */
bool verify(void * p1, void * p2, uint32_t n)
{
    uint8_t * data_p1 = (uint8_t *)p1;
    uint8_t * data_p2 = (uint8_t *)p2;

    for (uint32_t i = 0; i < n; i++)
        if (data_p1[i] != data_p2[i])
            return false;
    return true;
}
