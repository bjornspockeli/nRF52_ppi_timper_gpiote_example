/**
 * Copyright (c) 2009 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/** @file
* @brief Example template project.
* @defgroup nrf_templates_example Example Template
*
*/

#include <stdbool.h>
#include <stdint.h>

#include "nrf.h"
#include "nordic_common.h"
#include "boards.h"

// Header file that needs to be included for PPI/GPIOTE/TIMER/TWI example.
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_twi.h"

// Headers and defines needed by the logging interface
#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

// Map LED pins on the nRF52 DK. 
#define LED1 17
#define LED2 20
#define LED3 19

/* TWI instance ID. */
#define TWI_INSTANCE_ID     0

/* Common addresses definition for temperature sensor. */
#define LM75B_ADDR          (0x90U >> 1)
#define LM75B_REG_TEMP      0x00U
#define LM75B_REG_CONF      0x01U

/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

/* Indicates if operation on TWI has ended. */
static volatile bool m_xfer_done = false;

/* Buffer for samples read from temperature sensor. */
static uint8_t m_sample;


// Timer instance
const nrf_drv_timer_t timer0 = NRF_DRV_TIMER_INSTANCE(0);

// PPI channel
nrf_ppi_channel_t ppi_channel;

// Forward decleration LM75B_read_temp_data
static void LM75B_read_temp_data(void);


// Dummy timer event handler that will be used for step 2 and 4, but not step 3.
void timer_event_handler(nrf_timer_event_t event_type, void * p_context)
{
    switch(event_type)
    {
        case NRF_TIMER_EVENT_COMPARE0:
            nrf_drv_gpiote_out_task_trigger(LED3);
            LM75B_read_temp_data();
            break;
        default:
            // Do nothing.
            break;
    }
    
}

/** @brief Function for initializing the Timer peripheral.
*/
static void timer_init(void) {
    
    ret_code_t err_code;
    
    // Configure the timer to use the default configuration set in sdk_config.h
    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    
    //Initializing the Timer driver
    err_code = nrf_drv_timer_init(&timer0, &timer_cfg, timer_event_handler);
    APP_ERROR_CHECK(err_code);
    
    /*Configure the timer to generate the COMPARE event after 200*1000UL ticks and enable the shortcut that triggers the CLEAR task on every COMPARE event.
        This will */
    nrf_drv_timer_extended_compare(&timer0, NRF_TIMER_CC_CHANNEL0, nrf_drv_timer_ms_to_ticks(&timer0, 500), NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true); // Set last argument to false for Task 3
    
    // Turning on the Timer. 
    nrf_drv_timer_enable(&timer0);
}


/** @brief Function for initializing the GPIOTE peripheral.
*/
static void gpiote_init(void) {
    
    ret_code_t err_code;
    
    // Initialize the GPIOTE driver
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    // Configure the GPIO pin so that its toggled every time the OUT task is triggerd
    nrf_drv_gpiote_out_config_t config = GPIOTE_CONFIG_OUT_TASK_TOGGLE(false); 
    
    // Apply the configuration above to the LED_1 GPIO pin.  
    err_code = nrf_drv_gpiote_out_init(LED1, &config);
    APP_ERROR_CHECK(err_code);
    
    // Apply the configuration above to the LED_2 GPIO pin.  
    err_code = nrf_drv_gpiote_out_init(LED2, &config);
    APP_ERROR_CHECK(err_code);
    
       // Apply the configuration above to the LED_2 GPIO pin.  
    err_code = nrf_drv_gpiote_out_init(LED3, &config);
    APP_ERROR_CHECK(err_code);
   
    // Enabling the OUT task for the LED_1 GPIO pin.
    nrf_drv_gpiote_out_task_enable(LED1);
    
    // Enabling the OUT task for the LED_2 GPIO pin.
    nrf_drv_gpiote_out_task_enable(LED2);
    
    // Enabling the OUT task for the LED_3 GPIO pin.
    nrf_drv_gpiote_out_task_enable(LED3);
}

/** @brief Function for initializing the PPI peripheral.
*/
static void ppi_init(void)
{
    uint32_t err_code = NRF_SUCCESS;

    //Initialize the PPI driver
    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);
    
   // Allocate the first unused PPI channel 
    err_code = nrf_drv_ppi_channel_alloc(&ppi_channel);
    APP_ERROR_CHECK(err_code);
    
    
    // Assigning task and event endpoints so that the PPI channel triggers the OUT task of the LED_1 GPIO pin on TIMER0 COMPARE[0] match.
    err_code = nrf_drv_ppi_channel_assign(ppi_channel,
                                          nrf_drv_timer_event_address_get(&timer0, NRF_TIMER_EVENT_COMPARE0),
                                          nrf_drv_gpiote_out_task_addr_get(LED1)); 
    APP_ERROR_CHECK(err_code);
      
    // Fork the PPI channel, so that OUT task of the LED_2 GPIO pin is also triggered by the TIMER compare event. 
    err_code = nrf_drv_ppi_channel_fork_assign(ppi_channel, nrf_drv_gpiote_out_task_addr_get(LED2));
    APP_ERROR_CHECK(err_code);
    
    // Enable the PPI channel 
    err_code = nrf_drv_ppi_channel_enable(ppi_channel);
    APP_ERROR_CHECK(err_code);
   
}   

/**
 * @brief TWI events handler.
 */
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    switch (p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
            if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_RX)
            {
                NRF_LOG_INFO("Temperature: %d Celsius degrees.\r\n", m_sample);
            }
            m_xfer_done = true;
            break;
        default:
            break;
    }
}

/**
 * @brief TWI initialization.
 */
void twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_lm75b_config = {
       .scl                = ARDUINO_SCL_PIN, //27
       .sda                = ARDUINO_SDA_PIN, //
       .frequency          = NRF_TWI_FREQ_100K,
       .interrupt_priority = TWI_DEFAULT_CONFIG_IRQ_PRIORITY,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_lm75b_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

/**
 * @brief Function for setting the LM75B temperature sensor in Normal mode.
 */
void LM75B_set_mode(void)
{
    ret_code_t err_code;

    /* Writing to LM75B_REG_CONF "0" set temperature sensor in NORMAL mode. */
    uint8_t reg[2] = {LM75B_REG_CONF, 0U};
    err_code = nrf_drv_twi_tx(&m_twi, LM75B_ADDR, reg, sizeof(reg), false);
    APP_ERROR_CHECK(err_code);
    while (m_xfer_done == false);
}

/**
 * @brief Function for reading data from temperature sensor.
 */
static void LM75B_read_temp_data()
{
    ret_code_t err_code;
    
    // Writing to pointer byte. 
    uint8_t reg = LM75B_REG_TEMP;
    m_xfer_done = false;
    err_code = nrf_drv_twi_tx(&m_twi, LM75B_ADDR, &reg, 1, false);
    APP_ERROR_CHECK(err_code);
    while (m_xfer_done == false);
    
    m_xfer_done = false;

    /* Read 1 byte from the specified address - skip 3 bits dedicated for fractional part of temperature. */
    err_code = nrf_drv_twi_rx(&m_twi, LM75B_ADDR, &m_sample, sizeof(m_sample));
    APP_ERROR_CHECK(err_code);
}

static void power_manage()
{ 
  /* WFE - If the Event Register is not set, WFE suspends execution until one of the following events occurs:

            * an IRQ interrupt, unless masked by the CPSR I-bit
            * an FIQ interrupt, unless masked by the CPSR F-bit
            * an Imprecise Data abort, unless masked by the CPSR A-bit
            * a Debug Entry request, if Debug is enabled
            * an Event signaled by another processor using the SEV instruction. 
    
    If the Event Register is set, WFE clears it and returns immediately. If WFE is implemented, SEV must also be implemented.    
    
    SEV - SEV causes an event to be signaled to all cores within a multiprocessor system. If SEV is implemented, WFE must also be implemented.
   */

    // Wait for an event.
    __WFE();
    // Clear any pending events.
    __SEV();
    __WFE();
}

/**
 * @brief Function for application main entry.
 */
int main(void)
{ 
    ret_code_t err_code;
    
    err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);
    
    NRF_LOG_INFO("GPIOTE/TIMER/PPI/TWI Handson \r\n");
    
    // The GPIOTE peripheral must be initialized first, so that the correct Task Endpoint addresses are returned by nrf_drv_gpiote_xxx_task_addr_get()
    gpiote_init();
    ppi_init();
    timer_init();
    twi_init();
    LM75B_set_mode();

    while (true)
    {
        
        power_manage();
        // Do nothing.
    }
}
/** @} */
