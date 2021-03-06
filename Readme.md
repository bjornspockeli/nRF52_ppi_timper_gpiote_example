# Simple PPI/Timer/GPIOTE Example

### Brief:

Short tutorial that shows how to configure a Timer to toogle a GPIO pin, in software and by using PPI and GPIOTE to bypass the CPU.


### Requirements
- nRF5x DK
- SDK v13.0.0
- Template Project found in nRF5_SDK_13.0.0_04a0bfd\examples\peripheral\template_project

## Tasks

In all the tasks we'll be using the SDK driver for the peripherals, i.e. nrf_drv_xxx.c, which can be found in nRF5_SDK_13.0.0_04a0bfd\components\drivers_nrf\. It is possible to use the peripherals at the register level using the Hardware Access Layer(HAL) for said peripheral, found in nRF5_SDK_13.0.0_04a0bfd\components\drivers_nrf\hal. THis will, however, not be covered in this tutorial. 

### Warm-up

The template project includes all the peripheral libraries and drivers from the SDK, but we're only going to use a few, so to reduce the compile time and size of our project we'll temporarily remove them from the project. 

1. Find the *template_project* folder in nRF5_SDK_13.0.0_04a0bfd\examples\peripheral\

2. Create a copy of the  template_project folder and rename it to *gpiote_timer_ppi_handson*

3. Open the *template_pca10040.uvprojx* Keil project found in *gpiote_timer_ppi_handson\pca10040\blank\arm5_no_packs*

4. Select the folders *Board Support*, *nRF_Drivers*, *nRF_Libaries*, *nRF_Log* and *nRF_Segger_RTT* one by one in the Keil Project Explorer, left-click the selected folder, select *Options for Group xxxx* and uncheck *Include in Target Build*. 

5. Open the *sdk_config.h* file in the *None* folder in the Keil Project Explorer and click the *Configuration Wizard* tab. Uncheck all *nRF_Drivers*, *nRF_Libraries* and *nRF_Log*.

Remove files from Target Build | Uncheck modules in skd_config.h  | 
------------ |------------ |
<img src="https://github.com/bjornspockeli/nRF52_ppi_timper_gpiote_example/blob/master/images/warmup_uninclude_files.JPG" width="400"> | <img src="https://github.com/bjornspockeli/nRF52_ppi_timper_gpiote_example/blob/master/images/skd_config_uncheck.JPG" width="400"> |

### 1. GPIOTE - GPIO Tasks and Events

**Module description:** The GPIO tasks and events (GPIOTE) module provides functionality for accessing GPIO pins using tasks
and events. Each GPIOTE channel can be assigned to one pin. The GPIOTE block enables GPIOs to generate events on pin state change which can be used to carry out
tasks through the PPI system.

The GPIOTE driver API (nrf_drv_gpiote.c) is documented [here](http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v13.0.0/hardware_driver_gpiote.html?cp=4_0_0_2_3).

#### Steps

1.  Include the *nrf_drv_gpiote.h* header in the *main.c* file and create the function gpiote_init(), it should be *static void* and takes no arguments. Also, remember to enable the driver in *sdk_config.h*.

2. Initialize the GPIOTE driver.

3. Configure pin connected to LED1 on the nRF52 DK so that it is set as an an output and that the OUT task will toogle the pin. 
    (Hint 1: See the backside of the nRF52 Dk for the pinout).
    (Hint 2: Use the *GPIOTE_CONFIG_OUT_TASK_TOGGLE(init_high)* macro when configuring the pin). 

4. Enable the OUT task for the pin connected to LED1.

5. Add gpiote_init() in main() before the infinite while-loop. 

### 2. TIMER - Timer/counter

**Module description:** The TIMER can operate in two modes: timer and counter. Both run on the high-frequency clock source (HFCLK) and includes a four-bit (1/2X) prescaler that can divide the TIMER input clock from the HFCLK controller. The PPI system allows a TIMER event to trigger a task of any other system peripheral of the device.The PPI system also enables the TIMER task/event features to generate periodic output and PWM signals to any GPIO. The number of input/outputs used at the same time is limited by the number of GPIOTE channels.

The TIMER driver API (nrf_drv_timer.c) is documented [here](http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v13.0.0/hardware_driver_timer.html?cp=4_0_0_2_15).

#### Steps

1.  Include the *nrf_drv_timer.h* header in the *main.c* file and create the function timer_init(), it should be *static void* and takes no arguments. Also, remember to enable the driver in *sdk_config.h*.

2. Configure the timer to use the default config(NRF_DRV_TIMER_DEFAULT_CONFIG) that is given in *sdk_config.h*.

3. Open *sdk_config.h* in the *Configuration Wizard* view and check TIMER_ENABLED under nRF_Drivers. Change the settings so that the Timer frequency is set to 1MHz, the bit width is set to 32 and TIMER0 is the only enabled instance. 

4. Initialize the *nrf_drv_timer* driver. 

5. Use the *nrf_drv_timer_extended_compare()* function to set the compare value so that a Compare Event is generated every 0.5 seconds, enable the COMPARE0_CLEAR shortcut so that the timer is cleared for every Compare Event(creating a repeating timer) and enable interrupts on the COMPARE0 event.
    - (Hint 1: Use the *NRF_TIMER_CC_CHANNEL0* macro for the CC channel number).
    - (Hint 2: Use the *nrf_drv_timer_ms_to_ticks()* function to set the CC value).   
    - (Hint 3: Use *NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK* as the timer short mask) 

6. Turn on the timer by enabling it.

7. In the Timer callback function, that was passed as an argument during the initialization, you should call the *nrf_drv_gpiote_out_task_trigger()* function to trigger the OUT task of the LED1 pin manually. 

8. Add timer_init() in main() before the infinite while-loop.

9. Compile the template project and download it to the nRF52 DK. LED_1 shold now blink with a frequency of 0.5 Hz.

### 3. PPI - Programmable Peripheral Interconnect

**Module description:** The Programmable peripheral interconnect (PPI) enables peripherals to interact autonomously with each
other using tasks and events independent of the CPU. The PPI allows precise synchronization between
peripherals when real-time application constraints exist and eliminates the need for CPU activity to
implement behavior which can be predefined using PPI.


The PPI driver API (nrf_drv_ppi.c) is documented [here](http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v13.0.0/hardware_driver_ppi.html?cp=4_0_0_2_7).

#### Steps

1.  Include the *nrf_drv_ppi.h* header in the *main.c* file and create the function ppi_init(), it should be *static void* and takes no arguments. Also, remember to enable the driver in *sdk_config.h*.

2. Initialize the nrf_drv_ppi driver in the newley created ppi_init().

3. Allocate the first available PPI channel.

4. Assign the task and event endpoints of the allocated PPI channel so that a TIMER0 COMPARE[0] event triggers the OUT task of the LED_1 GPIO pin.
    - (Hint 1: Use the *nrf_drv_timer_event_address_get()* function to get the EEP(**E**vent **E**nd**P**oint).
    - (Hint 2: Use the *nrf_drv_gpiote_out_task_addr_get()* function to get the TEP(**T**ask **E**nd**P**oint). 

5. Enable the PPI channel. 

6. Disable the interrupt on the COMPARE0 event by setting the last parameter of *nrf_drv_timer_extended_compare()* to false in *timer_init()*.

7. Add *ppi_init()* in *main()* before the infinite while-loop, but after *gpiote_init()*.

8. Compile the template project and download it to the nRF52 DK. LED1 should now blink with the same frequency as before.

9. Start a debug session, press *Run(F5)* and then *Stop* to halt the CPU of the nRF52832. LED1 should continue to blink with the same frequency since we're bypassing the CPU using the PPI peripheral. 

10. Use the FORK feature of the PPI peripheral to assign a second Task Endpoint to the COMPARE0 event, specifically the OUT task of the pin connected to LED2 on the nRF52 DK.
    - (Hint 1: Use the *nrf_drv_ppi_channel_fork_assign()* to assign the second TEP).
    - (Hint 2: Remember to configure the pin connected to LED2 as an output and set the OUT task to toogle).   

11. Compile the template project and download it to the nRF52 DK. LED1 and LED2 should now blink with the same frequency as before.

12. Start a debug session, press *Run(F5)* and then *Stop* to halt the CPU of the nRF52832. LED1 and LED2 should continue to blink with the same frequency with the CPU halted. 

### 4. Logging - Adding the logging module

**Module description:** The logger module provides logging capability for your application. It is used by SDK modules and can be also utilized in application code.

The Logging module API (nrf_log.c) is documented [here](http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v13.0.0/nrf_log.html?cp=4_0_0_3_20).

#### Steps

1. Include *nrf_log_ctrl.h* and *nrf_log.h*  Initialize the logging module by calling *NRF_LOG_INIT(NULL)* in *main()*;

2. Include the .c files in the *nRF_Log* and *nRF_Segger_RTT* in the Keil Project explorer. You also need to add *#define NRF_LOG_MODULE_NAME "APP"* to the top of *main()*, but it must be defined before the *nrf_log_ctrl.h* and *nrf_log.h* include-statements.

3. Open *sdk_config.h* in the *Configuration Wizard* view and check *NRF_LOG_ENABLED*, expand to display the logging settings and uncheck *NRF_LOG_DEFFERED*. Under *nrf_log_backend* uncheck *NRF_LOG_BACKEND_SERIAL_USES_UART* and check *NRF_LOG_BACKEND_SERIAL_USES_RTT*. 

4. Add *NRF_LOG_INFO("GPIOTE/TIMER/PPI/TWI Handson \r\n");* after calling *NRF_LOG_INIT()*, compile the project and flash the project to your nRF52 DK. 

5. Open J-Link RTT Viewer, use the configuration shown below and press *OK*. The string passed with the *NRF_LOG_INFO()* macro should show up in the RTT terminal.   

J-Link RTT Viewer Configuration  | 
------------ |
<img src="https://github.com/bjornspockeli/nRF52_ppi_timper_gpiote_example/blob/master/images/rtt_viewer_config.JPG" width="250"> |

### 5. TWI - Two-Wire Interface

**Module description:** TWI master with EasyDMA (TWIM) or without(TWI) is a two-wire half-duplex master which can communicate with multiple slave devices connected to the same bus. The GPIOs used for each two-wire interface line can be chosen from any GPIO on the device and are independently configurable.

The TWI driver API (nrf_drv_twi.c) is documented [here](
http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.sdk5.v13.0.0/hardware_driver_twi.html?cp=4_0_0_2_16).

#### Steps

1. Include the *nrf_drv_twi.h* header in the *main.c* file and create the function *twi_init()*. Also, remember to enable the driver in *sdk_config.h*.

2. Initialize and enable the *nrf_drv_twi* driver within the *twi_init()* function. 
    - (Hint 1: The SCL and SDA pins of the mBed Application corresponds to pin 27 and 26 on the nRF52 DK).
    - (Hint 2: Use *TWI_DEFAULT_CONFIG_IRQ_PRIORITY* as the interrupt priority).   

3. Create the function *LM75B_set_mode()* where you put the LM75B sensor in normal mode by writing *0x00* to the *LM75B_REG_CONF* register of the LM75B.
    - (Hint 1: Use the *NRF_LOG_INFO()* macro to print the data ).
    - *LM75B_ADDR* (0x90U >> 1), *LM75B_REG_TEMP* 0x00U and *LM75B_REG_CONF* 0x01U

4. Create the function *LM75B_read_temp_data()* where you read the *LM75B_REG_TEMP* register on the LM75B and print the read temperature data using the Logging interface of the nRF5x SDK.
    - (Hint 1: Use the *NRF_LOG_INFO()* macro to print the data ).


5. Call *LM75B_read_temp_data()* from the *timer_event_handler()*

6. Compile the project and start a debug session and run the code. Halting the CPU you'll see that execution will loop forever at the *while(m_xfer_done == false)* statement in *LM75B_read_temp_data()*, i.e.the *m_xfer_done* flag is never set to *true*. Why is this?
    - (Hint 1: Remember that the m_xfer_done flag is set in the twi_event_handler function).
    - (Hint 2: It might have something to do with interrupt priorities).

7. Correct the code, compile the project, download it to the nRF52 DK and start J-Link RTT viewer as described in Step 4 - Adding the log module. You should now see the temperature being printed in the RTT terminal. 

Temperature printed in J-Link RTT Viewer | 
------------ |
<img src="https://github.com/bjornspockeli/nRF52_ppi_timper_gpiote_example/blob/master/images/segger_print_temp.JPG" width="500"> |
