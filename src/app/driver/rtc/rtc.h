/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer to
 * foxBMS in your hardware, software, documentation or advertising materials:
 *
 * - &Prime;This product uses parts of foxBMS&reg;&Prime;
 * - &Prime;This product includes parts of foxBMS&reg;&Prime;
 * - &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    rtc.h
 * @author  foxBMS Team
 * @date    2021-02-22 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  RTC
 *
 * @brief   Header file of the RTC driver
 * @details Uses the NXP PCF2131 RTC over I2C
 *
 */

#ifndef FOXBMS__RTC_H_
#define FOXBMS__RTC_H_

/*========== Includes =======================================================*/

#include "fstd_types.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** RTC I2C interface */
#define RTC_I2C_INTERFACE (i2cREG1)
/* I2C address of the RTC IC */
#define RTC_I2C_ADDRESS (0x53u)

/** Time data of the RTC */
typedef struct {
    uint8_t hundredthOfSeconds; /* [0-99] */
    uint8_t seconds;            /* [0-59] */
    uint8_t minutes;            /* [0-59] */
    uint8_t hours;              /* [0-23], 24 hour mode is used */
    uint8_t weekday;            /* [0-6], 0 is Sunday, 1 is Monday */
    uint8_t day;                /* [1-31] */
    uint8_t month;              /* [1-12] */
    uint8_t year;               /* [0-99] */
} RTC_TIME_DATA_s;

/* Defines for the I2C communication */
/**@{*/
#define RTC_MAX_I2C_TRANSACTION_SIZE_IN_BYTES            (10u)
#define RTC_WRITE_REGISTER_I2C_TRANSACTION_SIZE_IN_BYTES (2u)
/**@}*/

/* Defines for the RTC control_1 register */
/**@{*/
#define RTC_REG_CONTROL_1_ADDR            (0x0u)
#define RTC_CTRL1_STOP_BIT_MASK           (0x20u)
#define RTC_CTRL1_12_24HOUR_MODE_BIT_MASK (0x04u)
/**@}*/

/* Defines for the RTC control_3 register */
/**@{*/
#define RTC_REG_CONTROL_3_ADDR                  (0x02u)
#define RTC_CTRL3_BATTERY_PWRMNG_BITS_MASK      (0xE0u)
#define RTC_CTRL3_BATTERY_PWRMNG_BITS_POSITION  (5u)
#define RTC_CTRL3_BATTERY_LOW_FLAG_BIT_MASK     (0x04u)
#define RTC_CTRL3_BATTERY_LOW_FLAG_BIT_POSITION (2u)
/**@}*/

/* Power management configuration value to switch to battery voltage
   and monitor battery voltage */
#define RTC_CTRL3_PWRMNG_DIRECTSWITCH_LOWDETECTIONENABLE_MODE (0b011u)

/* Defines for the RTC CLKOUT_ctl register */
/**@{*/
#define RTC_REG_CLKOUT_CTL_ADDR          (0x13u)
#define RTC_CLKOUT_CTL_OTPR_BIT_MASK     (0x20u)
#define RTC_CLKOUT_CTL_OTPR_BIT_POSITION (5u)
/**@}*/

/* Timeout in 10ms to wait for OTPR bit to be cleared or set */
#define RTC_OTPR_BIT_WAIT_TIMEOUT_ms (150u)

/* Mask for the OSF bit */
#define RTC_SECONDS_OSF_BIT_MASK (0x80u)
/* Position of the OSF bit */
#define RTC_SECONDS_OSF_BIT_POSITION (7u)

/* Defines for the RTC software reset register */
/**@{*/
#define RTC_REG_SOFTWARE_RESET_ADDR (0x5u)
#define RTC_SWRESET_CLEARPRESCALER  (0xA4u)
/**@}*/

/* Defines for the RTC time data registers */
/**@{*/
#define RTC_REG_TIME_START_ADDR     (0x06u)
#define RTC_100TH_OF_SECONDS_OFFSET (0x0u)
#define RTC_SECONDS_OFFSET          (0x1u)
#define RTC_MINUTES_OFFSET          (0x2u)
#define RTC_HOURS_OFFSET            (0x3u)
#define RTC_DAYS_OFFSET             (0x4u)
#define RTC_WEEKDAYS_OFFSET         (0x5u)
#define RTC_MONTHS_OFFSET           (0x6u)
#define RTC_YEARS_OFFSET            (0x7u)
/**@}*/

/* Number of bytes needed to store all the RTC time data */
#define RTC_NUMBER_OF_TIMEDATA_BYTES (8u)

/* Defines to access time data for ten's place */
/**@{*/
#define RTC_TENS_PLACE_FACTOR                    (10u)
#define RTC_TENS_PLACE_OFFSET                    (4u)
#define RTC_HUNDREDTH_OF_SECONDS_TENS_PLACE_MASK (0xF0u)
#define RTC_SECONDS_TENS_PLACE_MASK              (0x70u)
#define RTC_MINUTES_TENS_PLACE_MASK              (0x70u)
#define RTC_HOURS_TENS_PLACE_MASK                (0x30u)
#define RTC_DAYS_TENS_PLACE_MASK                 (0x30u)
#define RTC_MONTHS_TENS_PLACE_MASK               (0x10u)
#define RTC_YEARS_TENS_PLACE_MASK                (0xF0u)
/**@}*/

/* Defines to access time data for unit's place */
/**@{*/
#define RTC_UNITS_PLACE_MASK          (0x0Fu)
#define RTC_WEEKDAYS_UNITS_PLACE_MASK (0x07u)
/**@}*/

/* Offset for the RTC year, because it is encoded with two digits */
#define RTC_START_YEAR (2000)

/* Defines for the C library time */
/**@{*/
#define RTC_CTIME_YEAR_START  (1900)
#define RTC_CTIME_MONTH_START (1)
/**@}*/

/* Max allowed difference between timer form RTC timer and IC time  */
#define RTC_MAX_DIFFERENCE_BETWEEN_TIMER_AND_IC_s (1)
/* Factor to convert minutes to milliseconds */
#define RTC_FACTOR_MIN_TO_MS (60000u)
/* Time interval between two comparisons between RTC timer and RTC IC */
#define RTC_TIME_BETWEEN_RTC_TIMER_ADJUSTMENT_min (60u)
/* Time interval between two check of the battery low flag */
#define RTC_TIME_BETWEEN_BLF_FLAG_CHECK_min (10u)

/** States for the OTPR procedure */
typedef enum {
    RTC_SET_SYSTEM_TIMER,
    RTC_SET_BLF,
    RTC_CLEAR_OTPR,
    RTC_WAIT_CLEAR_OTPR,
    RTC_OTPR_CLEARED,
    RTC_SET_OTPR,
    RTC_WAIT_SET_OTPR,
    RTC_INIT_SET_FINISHED,
} RTC_INIT_STATES_e;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   get the RTC system timer.
 * @details This function is used to get the RTC system timer value.
 * @return  value of the RTC system timer
 */
extern RTC_TIME_DATA_s RTC_GetSystemTimeRtcFormat(void);

/**
 * @brief   increment the RTC system timer.
 * @details This function must be called every millisecond and so
 *          must be called in the 1ms task.
 */
extern void RTC_IncrementSystemTime(void);

/**
 * @brief   initialization of the RTC IC.
 * @return  STD_OK if initialization is finished, STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e RTC_Initialize(void);

/**
 * @brief   initialize system time with RTC time.
 * @details This function is called once at the startup of the system.
 *          It reads the time from the RTC IC over I2C and sets the RTC
 *          system timer with this value. It must be called once the
 *          1ms task is already running, otherwise the RTC system timer
 *          will not be incremented and there will be a discrepancy between
 *          the RTC IC time and the RTC system timer.
 */
extern void RTC_InitializeSystemTimeWithRtc(void);

/**
 * @brief   set the RTC system timer.
 * @details This function is used to set the RTC system timer with
 *          RTC time data. It is called at startup by
 *          RTC_InitializeSystemTimeWithRtc().
 * @param   timeRtcFormat   time data to set the RTC system timer
 */
extern void RTC_SetSystemTimeRtcFormat(RTC_TIME_DATA_s timeRtcFormat);

/**
 * @brief   trigger function for the RTC driver.
 * @details This function uses RTC_SetOverCanMessage() to check in the RTC
 *          queue for requests to set the RTC time.
 *          If a request was made, the time of the RTC IC is set over I2C and
 *          the RTC system timer is set.
 *
 */
extern void RTC_Trigger(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__RTC_H_ */
