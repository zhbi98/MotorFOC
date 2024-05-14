/**
 * @file main.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "gd32f30x.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "systick.h"
#include "time.h"
#include "log.h"
#include "key.h"
#include "led.h"

#include "xt_usart0.h"
#include "xt_usart2.h"

#include "mb.h"
#include "mbrtu.h"
#include "mbtimer.h"
#include "nt_minitask.h"
#include "devdesc.h"
#include "xt_flash.h"
#include "tran.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void sys_clock_config(void);
static void _timer_init(uint32_t _prescaler, 
    uint32_t _period);

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initializes the device's core clock in preparation for startup.
 * The initialization frequency is 168 MHZ.
 */
void remap_gpio_init()
{
    rcu_periph_clock_enable(RCU_AF);
    /*gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);*/
    gpio_pin_remap_config(
        GPIO_SWJ_DISABLE_REMAP, 
        ENABLE);
}

/**
 * Initializes the device's core clock in preparation for startup.
 * The initialization frequency is 168 MHZ.
 */
void test_gpio_init()
{
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, 
        GPIO_OSPEED_2MHZ, GPIO_PIN_14);
}

/**
 * Initializes the device's core clock in preparation for startup.
 * The initialization frequency is 168 MHZ.
 */
uint32_t apply()
{
    xt_key_refer();

    /*Key event handling for relay interaction*/
    if (xt_key_get_act() == KEY1_EVT) xt_tran_dir_update();

    /*Key event handling for relay interaction*/
    bool valid = mb_rtu_reg_range_valid();

    if (valid) {
        uint16_t start = 0, end = 0;

        mb_rtu_reg_get_range(&start, &end);

        if (start >= REG_SN_START && 
            end <= REG_ID_END
        ) {
            uint32_t ofs = sizeof(xt_devdesc_t);
            xt_devdesc_t _devdesc = {0};

            uint32_t * _desc_p = \
                (uint32_t *)(&_devdesc);

            /*The validation failed and the storage needs to be updated*/
            memset(&_devdesc, '\0', 
                sizeof(_devdesc));

            /*The validation failed and the storage needs to be updated*/
            xt_flash_read(FLASH_START_ADDR, 
                FLASH_START_ADDR + ofs, 
                _desc_p);

            bool res = verify(&devdesc, 
                &_devdesc, ofs);

            /*The validation failed and the storage needs to be updated*/
            if (!res) {
                xt_devdesc_write_data();

                uint8_t id = \
                    (uint8_t)devdesc.slaveid;

                mb_rtu_set_slave_addr(id);
            }
        }

        else
        if (start >= REG_DIR_START && 
            end <= REG_DIR_END
        ) {
            tran_dir = tran_dir & 0x0001;

            tran_dir = (
                tran_dir == XT_IDLE
            ) ? XT_ACT : XT_IDLE;

            xt_tran_dir_update();
        }

        else
        if (start >= REG_LED3_START && 
            end <= REG_LED3_END
        ) {
            _led3 = _led3 & 0x0001;
            if (_led3) gpio_bit_set(GPIOB, GPIO_PIN_9);
            else gpio_bit_reset(GPIOB, GPIO_PIN_9);

            sleep_ms(50);
        }

        /*Data has been applied to the driver 
        to clear the event*/
        mb_rtu_reg_clear_range();
    }
}

/**
 * Initializes the device's core clock in preparation for startup.
 * The initialization frequency is 168 MHZ.
 */
uint32_t mb()
{
    /*Process the protocol fields*/
    mb_rtu_pdu_field_deal();
    led_light_work(&led1);
    led_light_work(&led2);
    /*led_light_work(&led3);*/
}

/**
 * Initializes the device's core clock in preparation for startup.
 * The initialization frequency is 168 MHZ.
 */
int32_t main()
{
    sys_clock_config(); /*SystemInit();*/
    remap_gpio_init();
    xt_usart0_dev_init();
    xt_usart2_dev_init();
    systick_config();
    xt_key_gpio_init();
    xt_tran_gpio_init();
    led_gpio_init();

    mb_rtu_mode_init(42, 115200);
    _timer_init(960, 100); /*Timer tick 1ms*/

    _mb_rtu_xcall_register(0x03, 
        mb_rtu_read_reg_data);
    _mb_rtu_xcall_register(0x10, 
        mb_rtu_write_reg_data);

    xt_devdesc_read_data();

    if (devdesc.slaveid == 0xFFFF) {
        strcpy((uint8_t *)devdesc.sn, "XT0000000001");
        strcpy((uint8_t *)devdesc.version, "1.0.0");
        strcpy((uint8_t *)devdesc.build, "1024");
        strcpy((uint8_t *)devdesc.date, "2020/10/24");
        devdesc.slaveid = 0x42;
        info("%s", (uint8_t *)devdesc.sn);
        info("%s", (uint8_t *)devdesc.version);
        info("%s", (uint8_t *)devdesc.build);
        info("%s", (uint8_t *)devdesc.date);
        info("%x", devdesc.slaveid);
    }

    nt_task_add(apply, 100, 100);
    nt_task_add(mb, 10, 10);

    for (;;) {
        nt_task_handler();
    }

    return 0;
}

/**
 * Initializes the device's core clock in preparation for startup.
 * The initialization frequency is 108 MHZ.
 */
static void sys_clock_config(void)
{
    /**使能内外部晶振*/
    rcu_osci_on(RCU_IRC8M);
    /**等待晶振稳定*/
    while (rcu_osci_stab_wait(RCU_IRC8M) != SUCCESS);

    /**设置PREDV0*/
    rcu_predv0_config(RCU_PREDV0_DIV1);

    /**PLL 设置*/
    rcu_pll_config(RCU_PLLSRC_IRC8M_DIV2, RCU_PLL_MUL24);
    /**使能 PLL*/
    rcu_osci_on(RCU_PLL_CK);
    /**等待 PLL 稳定*/
    while (rcu_osci_stab_wait(RCU_IRC8M) != SUCCESS);

    /**总线时钟配置*/
    rcu_ahb_clock_config(RCU_AHB_CKSYS_DIV1);
    rcu_apb1_clock_config(RCU_APB1_CKAHB_DIV2);
    rcu_apb2_clock_config(RCU_APB2_CKAHB_DIV1);

    /**系统时钟源选择*/
    rcu_system_clock_source_config(RCU_CKSYSSRC_PLL);
    while(rcu_system_clock_source_get() != RCU_SCSS_PLL);

    /**更新SystemCoreClock全局变量*/
    SystemCoreClockUpdate();
}

/*
-----------------------------------------
    Timing 0.1s

    Prescaler = AHB Clock / Timing Clock
              = 84(MHz) / 50(KHz)
              = 84000000(hz) / 50000(hz)
              = 1680

    T = 1 / f
      = 1(s) / f(Hz)
      = 1000000(us) / 50000(hz)
      = 20(us/times)

    Period = Timing / T
           = 0.1(s) / 20(us/times)
           = 100000(us) / 20(us/times)
           = 5000(times)

-----------------------------------------
    Timing 0.5s
    
    Prescaler = AHB Clock / Timing Clock
              = 84(MHz) / 10(KHz)
              = 84000000(hz) / 10000(hz)
              = 8400

    T = 1 / f
      = 1(s) / f(Hz)
      = 1000000(us) / 10000(hz)
      = 100(us/times)

    Period = Timing / T
           = 0.5(s) / 100(us/times)
           = 500000(us) / 100(us/times)
           = 5000(times)
-----------------------------------------
*/

static void _timer_init(uint32_t _prescaler, uint32_t _period)
{
    /**
     * TIMER configuration: generate PWM signals
     * with different duty cycles:
     * TIMERCLK = SystemCoreClock / 120 = 1MHz
     */
    timer_oc_parameter_struct timer_out_init;
    timer_parameter_struct time_init;

    rcu_periph_clock_enable(RCU_TIMER1);
    timer_deinit(TIMER1);

    time_init.prescaler         = _prescaler - 1;
    time_init.alignedmode       = TIMER_COUNTER_EDGE;
    time_init.counterdirection  = TIMER_COUNTER_UP;
    time_init.period            = _period;
    time_init.clockdivision     = TIMER_CKDIV_DIV1;
    time_init.repetitioncounter = 0;
    timer_init(TIMER1, &time_init);

#if 0
    CH0 config in pwm mode
    timer_out_init.outputstate  = TIMER_CCX_ENABLE;
    timer_out_init.outputnstate = TIMER_CCXN_DISABLE;
    timer_out_init.ocpolarity   = TIMER_OC_POLARITY_HIGH;
    timer_out_init.ocnpolarity  = TIMER_OCN_POLARITY_HIGH;
    timer_out_init.ocidlestate  = TIMER_OC_IDLE_STATE_LOW;
    timer_out_init.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;
    timer_channel_output_config(TIMER1,
        SUART_TIMER_CH, &timer_out_init);

    timer_channel_output_pulse_value_config(
        TIMER1, SUART_TIMER_CH, 250);
    timer_channel_output_mode_config(
        TIMER1, SUART_TIMER_CH, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(
        TIMER1, SUART_TIMER_CH, TIMER_OC_SHADOW_DISABLE);
    timer_primary_output_config(TIMER1, ENABLE);
    timer_auto_reload_shadow_enable(TIMER1);
#endif

    nvic_irq_enable(TIMER1_IRQn, 1, 1);
    timer_interrupt_enable(
        TIMER1, TIMER_INT_UP);
    timer_enable(TIMER1);
}

/**
 * Initializes the device's core clock in preparation for startup.
 * The initialization frequency is 108 MHZ.
 */
void TIMER1_IRQHandler()
{
    if (timer_interrupt_flag_get(TIMER1, 
            TIMER_INT_FLAG_UP) == SET) {
        timer_interrupt_flag_clear(TIMER1, 
            TIMER_INT_FLAG_UP);

        mb_timer_tick_callback();
        nt_task_tick_inc(1);
    }
}
