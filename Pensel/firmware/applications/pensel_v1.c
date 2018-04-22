/*!
 * @file    main.c
 * @author  Tyler Holmes
 * @version 0.1.0
 * @date    20-May-2017
 * @brief   Main project logic.
 */

#include <stdint.h>
#include <string.h>
#include "common.h"

// Sensors
#include "modules/LSM303DLHC/LSM303DLHC.h"
#include "peripherals/ADC/ADC.h"        // for thumbwheel
#include "peripherals/hardware/hardware.h"   // HW support and button / switch functions

// Communications and such
#include "peripherals/I2C/I2C.h"
#include "peripherals/UART/UART.h"
#include "infrastructure/UART-reports/reports.h"

// Algssss
#include "modules/orientation/orientation.h"
#include "modules/calibration/cal.h"

// STM Drivers
#include "peripherals/stm32f3/stm32f3xx_hal_def.h"
#include "peripherals/stm32f3/stm32f3xx_hal.h"
#include "peripherals/stm32f3-configuration/stm32f3xx_hal_conf.h"


//! HAL millisecond tick
extern __IO uint32_t uwTick;
// Global variables to influence state

//! Global toggle to enable/disable streaming mag data
bool gEnableRawMagStream = false;
//! Global toggle to enable/disable streaming accel data
bool gEnableRawAccelStream = false;
//! Global toggle to enable/disable streaming filtered mag data
bool gEnableFilteredMagStream = false;
//! Global toggle to enable/disable streaming filtered accel data
bool gEnableFilteredAccelStream = false;

critical_errors_t gCriticalErrors;


#ifdef WATCHDOG_ENABLE
    //! Global indicating whether or not to pet the watchdog
    static bool gPetWdg = false;


    void wdg_captureAlert(void) {
        // If we want, we can catch if a watchdog has ocurred
        uint32_t subcount = 0;

        LED_set(LED_0, 0);
        LED_set(LED_1, 1);
        while (1) {
            subcount++;
            if (subcount > 1000000) {
                subcount = 0;
                LED_toggle(LED_0);
                LED_toggle(LED_1);
            }
        }
    }
#endif

static inline void check_retval_fatal(char * filename, uint32_t lineno, ret_t retval) {
    if (retval != RET_OK) {
        fatal_error_handler(filename, lineno, (int8_t)retval);
    }
}

void clear_critical_errors(void) {
    #ifdef WATCHDOG_ENABLE
        gCriticalErrors.wdg_reset = 0;
    #endif
}


/*! Main function code. Does the following:
 *      1. Initializes all sub-modules
 *      2. Loops forever and behaves as such given switch state:
 *          Switch 0: Runs report parsing only
 *          Switch 1: Runs debug print output only
 */
int main(void)
{
    ret_t retval;
    mag_norm_t mag_pkt;
    accel_norm_t accel_pkt;
    uint32_t subcount = 0;

    // system configuration...
    HAL_Init();
    SystemClock_Config();
    configure_pins();
    clear_critical_errors();

    retval = UART_init(250000);
    check_retval_fatal(__FILE__, __LINE__, retval);

    #ifdef WATCHDOG_ENABLE
    if ( wdg_isSet() ) {
        // set a report variable in critical errors
        gCriticalErrors.wdg_reset = 1;
        #ifdef WATCHDOG_CAPTURE
            wdg_captureAlert();
        #endif
    }
    retval = wdg_init();
    check_retval_fatal(__FILE__, __LINE__, retval);
    gPetWdg = true;
    #endif

    // peripheral configuration
    retval = I2C_init();
    check_retval_fatal(__FILE__, __LINE__, retval);

    retval = ADC_init();
    check_retval_fatal(__FILE__, __LINE__, retval);

    retval = LSM303DLHC_init(kAccelODR_200_Hz, kOne_mg_per_LSB,
                             kMagODR_220_Hz, kXY_450_Z_400_LSB_per_g);
    check_retval_fatal(__FILE__, __LINE__, retval);

    retval = rpt_init(&UART_sendChar, &UART_getChar);
    check_retval_fatal(__FILE__, __LINE__, retval);

    retval = orient_init();
    check_retval_fatal(__FILE__, __LINE__, retval);

    // Load Calibration
    cal_loadFromFlash();
    if (cal_checkValidity() != RET_OK) {
        cal_loadDefaults();
    }

    LED_set(LED_0, 0);
    LED_set(LED_1, 0);

    while (true) {
        // Parse reports and such
        rpt_run();

        // get packets and such
        LSM303DLHC_run();

        // let the user know roughly how many workloops are ocurring
        if (subcount == 100000) {
            LED_toggle(LED_1);
            subcount = 0;
        } else {
            subcount++;
        }

        // --- Check Mag Data ----

        if( LSM303DLHC_mag_dataAvailable() ) {
            if ( LSM303DLHC_mag_getPacket(&mag_pkt, false) == RET_OK ) {
                // Transmit the packet if we're streaming it
                if (gEnableRawMagStream == true) {
                    retval = rpt_sendStreamReport(RMAG_STREAM_REPORT_ID,
                        sizeof(mag_norm_t), (uint8_t *)&mag_pkt);
                    if (retval != RET_OK) {
                        // Dropped input report
                    }
                }
                // ingest it into the interested parties
                orient_calcMagOrientation(mag_pkt);
                if (gEnableFilteredMagStream == true) {
                    cartesian_vect_t vect;
                    vect = orient_getMagOrientation();
                    mag_pkt.x = vect.x;
                    mag_pkt.y = vect.y;
                    mag_pkt.z = vect.z;
                    retval = rpt_sendStreamReport(FMAG_STREAM_REPORT_ID,
                        sizeof(mag_norm_t), (uint8_t *)&mag_pkt);
                    if (retval != RET_OK) {
                        // Dropped input report
                    }
                }
            }
        }

        // --- Check Accel Data ----

        if( LSM303DLHC_accel_dataAvailable() ) {
            if ( LSM303DLHC_accel_getPacket(&accel_pkt, false) == RET_OK ) {
                // Transmit the packet if we're streaming it
                if (gEnableRawAccelStream == true) {
                    retval = rpt_sendStreamReport(RACCEL_STREAM_REPORT_ID,
                        sizeof(accel_norm_t), (uint8_t *)&accel_pkt);
                    if (retval != RET_OK) {
                        // Dropped input report
                    }
                }
                // ingest it into the interested parties
                orient_calcAccelOrientation(accel_pkt);
                if (gEnableFilteredAccelStream == true) {
                    cartesian_vect_t vect;
                    vect = orient_getAccelOrientation();
                    accel_pkt.x = vect.x;
                    accel_pkt.y = vect.y;
                    accel_pkt.z = vect.z;
                    retval = rpt_sendStreamReport(FACCEL_STREAM_REPORT_ID,
                        sizeof(accel_norm_t), (uint8_t *)&accel_pkt);
                    if (retval != RET_OK) {
                        // Dropped input report
                    }
                }
            }
        }

        // If we're in debug mode, don't run nothin'
        if ( switch_getval() == kSwitch_0) {
            // do nothing for now...
        }
        // If we're in debug output mode, print packets to UART
        else if ( switch_getval() == kSwitch_1 || switch_getval() == kSwitch_2 ) {
            // do nothing for now...
        }
    } /* while (true) */
} /* main() */

/*! millisecond ISR that increments the global time as well as calls some functions
 *  that need to be periodically serviced. We do a few things:
 *
 *     1. Take care of button and switch debouncing every 10 ms
 *        - If we've changed states in interrupt context and enough time has elapsed (``)
 *          we will change the button / switch state
 *     2. Pet the watchdog every 5 ms. Must be pet within 10 ms!
 *     3. Toggle the heartbeat LED every 1 second
 */
void HAL_IncTick(void)
{
    // sub counter to take care of some tasks every N ms
    static uint8_t sub_count = 0;
    static uint16_t second_count = 0;

    // increment the ms timer
    uwTick++;

    // take care of some things that need to be called periodically every ~10 ms
    if (sub_count >= 9) {
        sub_count = 0;
        button_periodic_handler(uwTick);
        switch_periodic_handler(uwTick);
    } else {
        sub_count++;
    }

    // every 5 ms, pet the watchdog. Need to pet the watchdog every 10 ms!
    #ifdef WATCHDOG_ENABLE
    if (sub_count == 5 && gPetWdg) {
        wdg_pet();
    }
    #endif

    // Heartbeat LED flash
    if (second_count >= 1000) {
        second_count = 0;
        LED_toggle(LED_0);
    } else {
        second_count++;
    }
}


/*! Error handler that is called when fatal exceptions are found.
 *
 * @param file (char *): File in which the error comes from. Use the __FILE__ macro.
 * @param line (uint32_t): Line number in which the error comes from. Use the __LINE__ macro.
 * @param err_code (int8_t): Error code that was thrown. Usually the ret_t value
    that caused the fail.
 */
void fatal_error_handler(char file[], uint32_t line, int8_t err_code)
{
    // FREAK OUT
    #ifdef DEBUG
        uint32_t timer_count, i = 0;
        LED_set(LED_0, 0);
        LED_set(LED_1, 1);

        // Can't rely on HAL tick as we maybe in the ISR context...
        while (1) {
            UART_sendString(file);
            UART_sendint((int64_t)line);
            UART_sendint((int64_t)err_code);
            UART_sendString("\r\n");

            for (i = 0; i < 25; i++) {
                while (timer_count < 100000) {
                    timer_count++;
                }
                timer_count = 0;
                // If we're in debug mode, keep the watchdog kickin
                #if defined(DEBUG) && defined(WATCHDOG_ENABLE)
                    if (gPetWdg) {
                        wdg_pet();
                    }
                #endif
            }
            LED_toggle(LED_0);
            LED_toggle(LED_1);
        }
    #else
        // TODO: Reset everything
        // TODO: Log failure
        // for now, just let the watchdog happen
        while (1);
    #endif
}

// HAL uses this function. Call our error function.
void assert_failed(uint8_t* file, uint32_t line)
{
    fatal_error_handler((char *)file, line, -1);
}