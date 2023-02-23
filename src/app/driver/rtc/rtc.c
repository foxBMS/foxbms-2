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
 * @file    rtc.c
 * @author  foxBMS Team
 * @date    2021-02-22 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  RTC
 *
 * @brief   Implementation of the RTC driver
 * @details Uses the NXP PCF2131 RTC over I2C
 *
 */

/*========== Includes =======================================================*/
#include "rtc.h"

#include "database.h"
#include "diag.h"
#include "foxmath.h"
#include "fstd_types.h"
#include "ftask.h"
#include "i2c.h"

#include <stdint.h>
/* AXIVION Disable Style MisraC2012-21.10: Time implementation is suitable fpr the application */
#include <time.h>

/*========== Macros and Definitions =========================================*/

/** Time data of the RTC */
typedef struct {
    time_t secondsSinceEpoch;
    uint16_t milliseconds;
} RTC_SYSTEM_TIMER_EPOCH_s;

/*========== Static Constant and Variable Definitions =======================*/

/* AXIVION Disable Style MisraC2012-1.2: i2c buffer must be put in shared RAM section if used with DMA and cache */
#pragma SET_DATA_SECTION(".sharedRAM")
/** I2C buffer for write transactions with RTC */
static uint8_t rtc_i2cWriteBuffer[RTC_MAX_I2C_TRANSACTION_SIZE_IN_BYTES] = {0};
/** I2C buffer for read transactions with RTC */
static uint8_t rtc_i2cReadBuffer[RTC_MAX_I2C_TRANSACTION_SIZE_IN_BYTES] = {0};
#pragma SET_DATA_SECTION()
/* AXIVION Enable Style MisraC2012-1.2: only i2c buffer needs to be in the shared RAM section */

/** Variable containing the RTC system time */
static RTC_SYSTEM_TIMER_EPOCH_s rtc_SystemTime = {0};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   Reads the time from the RTC IC.
 * @details This function reads the time from the RTC IC over I2C.
 * @return  RTC time read from the RTC IC
 */
static RTC_TIME_DATA_s RTC_ReadTime(void);

static void RTC_WriteTime(RTC_TIME_DATA_s rtcTime);
static void RTC_SetOverCanMessage(void);
static void RTC_AdjustTime(void);
static void RTC_CheckBatteryLowVoltageAlert(void);

static void RTC_SetSystemTimeEpochFormat(time_t timeEpochFormat, uint16_t milliseconds);
static RTC_SYSTEM_TIMER_EPOCH_s RTC_GetSystemTimeEpochFormat(void);

static struct tm RTC_rtcFormatToTmFormat(RTC_TIME_DATA_s timeRtcFormat);
static RTC_TIME_DATA_s RTC_tmFormatToRtcFormat(struct tm timeTmFormat);

/*========== Static Function Implementations ================================*/

/**
 * @brief   Write the time to the RTC IC.
 * @details This function writes the time to the RTC IC over I2C.
 * @param   rtcTime RTC time to write to the IC
 */
static void RTC_WriteTime(RTC_TIME_DATA_s rtcTime) {
    /* AXIVION Routine Generic-MissingParameterAssert: time: parameter accepts whole range */
    STD_RETURN_TYPE_e retValI2c = STD_OK;
    STD_RETURN_TYPE_e retVal    = STD_OK;

    /* Write control_1 register to stop RTC */
    rtc_i2cWriteBuffer[0u] = RTC_REG_CONTROL_1_ADDR;
    /* Set stop bit, clear 12/24hour mode bit */
    rtc_i2cWriteBuffer[1u] = 0u;
    rtc_i2cWriteBuffer[1u] |= RTC_CTRL1_STOP_BIT_MASK;
    rtc_i2cWriteBuffer[1u] &= ~((uint8_t)RTC_CTRL1_12_24HOUR_MODE_BIT_MASK);
    retValI2c = I2C_WriteDma(
        RTC_I2C_INTERFACE, RTC_I2C_ADDRESS, RTC_WRITE_REGISTER_I2C_TRANSACTION_SIZE_IN_BYTES, rtc_i2cWriteBuffer);
    if (retValI2c == STD_NOT_OK) {
        retVal = STD_NOT_OK;
    }
    /* Write 0xA4 to software reset register */
    rtc_i2cWriteBuffer[0u] = RTC_REG_SOFTWARE_RESET_ADDR;
    rtc_i2cWriteBuffer[1u] = RTC_SWRESET_CLEARPRESCALER;
    /* Address counters rolls to time data */
    rtc_i2cWriteBuffer[RTC_WRITE_REGISTER_I2C_TRANSACTION_SIZE_IN_BYTES + RTC_100TH_OF_SECONDS_OFFSET] =
        ((rtcTime.hundredthOfSeconds / RTC_TENS_PLACE_FACTOR) << RTC_TENS_PLACE_OFFSET) |
        (rtcTime.hundredthOfSeconds % RTC_TENS_PLACE_FACTOR);
    rtc_i2cWriteBuffer[RTC_WRITE_REGISTER_I2C_TRANSACTION_SIZE_IN_BYTES + RTC_SECONDS_OFFSET] =
        ((rtcTime.seconds / RTC_TENS_PLACE_FACTOR) << RTC_TENS_PLACE_OFFSET) |
        (rtcTime.seconds % RTC_TENS_PLACE_FACTOR);
    rtc_i2cWriteBuffer[RTC_WRITE_REGISTER_I2C_TRANSACTION_SIZE_IN_BYTES + RTC_MINUTES_OFFSET] =
        ((rtcTime.minutes / RTC_TENS_PLACE_FACTOR) << RTC_TENS_PLACE_OFFSET) |
        (rtcTime.minutes % RTC_TENS_PLACE_FACTOR);
    rtc_i2cWriteBuffer[RTC_WRITE_REGISTER_I2C_TRANSACTION_SIZE_IN_BYTES + RTC_HOURS_OFFSET] =
        ((rtcTime.hours / RTC_TENS_PLACE_FACTOR) << RTC_TENS_PLACE_OFFSET) | (rtcTime.hours % RTC_TENS_PLACE_FACTOR);
    rtc_i2cWriteBuffer[RTC_WRITE_REGISTER_I2C_TRANSACTION_SIZE_IN_BYTES + RTC_DAYS_OFFSET] =
        ((rtcTime.day / RTC_TENS_PLACE_FACTOR) << RTC_TENS_PLACE_OFFSET) | (rtcTime.day % RTC_TENS_PLACE_FACTOR);
    rtc_i2cWriteBuffer[RTC_WRITE_REGISTER_I2C_TRANSACTION_SIZE_IN_BYTES + RTC_WEEKDAYS_OFFSET] =
        ((rtcTime.weekday / RTC_TENS_PLACE_FACTOR) << RTC_TENS_PLACE_OFFSET) |
        (rtcTime.weekday % RTC_TENS_PLACE_FACTOR);
    rtc_i2cWriteBuffer[RTC_WRITE_REGISTER_I2C_TRANSACTION_SIZE_IN_BYTES + RTC_MONTHS_OFFSET] =
        ((rtcTime.month / RTC_TENS_PLACE_FACTOR) << RTC_TENS_PLACE_OFFSET) | (rtcTime.month % RTC_TENS_PLACE_FACTOR);
    rtc_i2cWriteBuffer[RTC_WRITE_REGISTER_I2C_TRANSACTION_SIZE_IN_BYTES + RTC_YEARS_OFFSET] =
        ((rtcTime.year / RTC_TENS_PLACE_FACTOR) << RTC_TENS_PLACE_OFFSET) | (rtcTime.year % RTC_TENS_PLACE_FACTOR);
    /* Write time data */
    retValI2c =
        I2C_WriteDma(RTC_I2C_INTERFACE, RTC_I2C_ADDRESS, RTC_MAX_I2C_TRANSACTION_SIZE_IN_BYTES, rtc_i2cWriteBuffer);
    if (retValI2c == STD_NOT_OK) {
        retVal = STD_NOT_OK;
    }
    /* Wait external reference */
    /* Now write control_1 register again to start RTC */
    rtc_i2cWriteBuffer[0u] = RTC_REG_CONTROL_1_ADDR;
    rtc_i2cWriteBuffer[1u] = 0u;
    rtc_i2cWriteBuffer[1u] &= ~((uint8_t)RTC_CTRL1_STOP_BIT_MASK);
    rtc_i2cWriteBuffer[1u] &= ~((uint8_t)RTC_CTRL1_12_24HOUR_MODE_BIT_MASK);
    retValI2c = I2C_WriteDma(
        RTC_I2C_INTERFACE, RTC_I2C_ADDRESS, RTC_WRITE_REGISTER_I2C_TRANSACTION_SIZE_IN_BYTES, rtc_i2cWriteBuffer);
    if (retValI2c == STD_NOT_OK) {
        retVal = STD_NOT_OK;
    }

    if (retVal == STD_NOT_OK) {
        DIAG_Handler(DIAG_ID_I2C_RTC_ERROR, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
    } else {
        DIAG_Handler(DIAG_ID_I2C_RTC_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
    }
}

/**
 * @brief   Check if a CAN message was received to set the RTC time.
 *
 * When a CAN message to set the RTC time is received, it is written
 * in the dedicated queue. This function check if an element is present
 * in the queue. If yes, the time is first written to the RTC IC over
 * I2C, then to the RTC system timer.
 *
 */
static void RTC_SetOverCanMessage(void) {
    RTC_TIME_DATA_s rtc_time = {0};

    if (OS_ReceiveFromQueue(ftsk_rtcSetTimeQueue, (void *)&rtc_time, 0u) == OS_SUCCESS) {
        /* Write time data to RTC */
        RTC_WriteTime(rtc_time);
        /* Set system timer */
        RTC_SetSystemTimeRtcFormat(rtc_time);
    }
}

/**
 * @brief   Adjust RTC system timer with time from RTC IC.
 *
 * The time is read from the RTC IC and compared to the
 * RTC system timer. If the difference is greater than the
 * limit, the RTC system timer is adjusted.
 */
static void RTC_AdjustTime(void) {
    RTC_TIME_DATA_s rtcTimeFromTimerRtcFormat = {0};
    RTC_TIME_DATA_s rtcTimeFromIcRtcFormat    = {0};
    time_t rtcTimeFromTimerEpochFormat        = 0;
    time_t rtcTimeFromIcEpochFormat           = 0;
    struct tm rtcTimeFromTimerTmFormat        = {0};
    struct tm rtcTimeFromIcTmFormat           = {0};

    rtcTimeFromIcRtcFormat = RTC_ReadTime();
    /* Convert time and date from RTC to tm struct */
    rtcTimeFromIcTmFormat = RTC_rtcFormatToTmFormat(rtcTimeFromIcRtcFormat);
    /* Convert tm struct to timer in seconds since epoch */
    rtcTimeFromIcEpochFormat = mktime(&rtcTimeFromIcTmFormat);

    rtcTimeFromTimerRtcFormat = RTC_GetSystemTimeRtcFormat();
    /* Convert time and date from RTC to tm struct */
    rtcTimeFromTimerTmFormat = RTC_rtcFormatToTmFormat(rtcTimeFromTimerRtcFormat);
    /* Convert tm struct to timer in seconds since epoch */
    rtcTimeFromTimerEpochFormat = mktime(&rtcTimeFromTimerTmFormat);

    if (abs(rtcTimeFromIcEpochFormat - rtcTimeFromTimerEpochFormat) > RTC_MAX_DIFFERENCE_BETWEEN_TIMER_AND_IC_s) {
        /* Difference  between RTC timer and RTC IC higher than limit: adjust RTC timer */
        RTC_InitializeSystemTimeWithRtc();
    }
}

/**
 * @brief   Read bit for battery voltage low flag.
 */
static void RTC_CheckBatteryLowVoltageAlert(void) {
    STD_RETURN_TYPE_e retValI2c = STD_OK;
    STD_RETURN_TYPE_e retVal    = STD_OK;
    uint8_t blfBit              = 0u; /* Battery Low Flag (BLF) */

    /* Set address to read from */
    rtc_i2cWriteBuffer[0u] = RTC_REG_CONTROL_3_ADDR;
    retValI2c              = I2C_Write(RTC_I2C_INTERFACE, RTC_I2C_ADDRESS, 1u, rtc_i2cWriteBuffer);
    if (retValI2c == STD_NOT_OK) {
        retVal = STD_NOT_OK;
    }
    /* Address set, read register data */
    retValI2c = I2C_Read(RTC_I2C_INTERFACE, RTC_I2C_ADDRESS, 1u, rtc_i2cReadBuffer);
    if (retValI2c == STD_NOT_OK) {
        retVal = STD_NOT_OK;
    }

    /* OSF bit is stored at position 7 in seconds register */
    blfBit = (rtc_i2cReadBuffer[0u] & RTC_CTRL3_BATTERY_LOW_FLAG_BIT_MASK) >> RTC_CTRL3_BATTERY_LOW_FLAG_BIT_POSITION;

    if (retVal == STD_NOT_OK) {
        DIAG_Handler(DIAG_ID_I2C_RTC_ERROR, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
    } else {
        DIAG_Handler(DIAG_ID_I2C_RTC_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
        /* If I2C communication successful, check BLF bit */
        if (blfBit != 0u) {
            DIAG_Handler(DIAG_ID_RTC_BATTERY_LOW_ERROR, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
        } else {
            DIAG_Handler(DIAG_ID_RTC_BATTERY_LOW_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
        }
    }
}

/**
 * @brief   Set the RTC system timer value.
 *
 * This function sets the timer value directly.
 * It is called by RTC_SetSystemTimeRtcFormat() which
 * takes a RTC_TIME_DATA_s parameter as input.
 *
 * @param    timeEpochFormat   RTC system timer value to set, main value in seconds
 * @param    milliseconds      RTC system timer value to set, secondary value in milliseconds
 */
static void RTC_SetSystemTimeEpochFormat(time_t timeEpochFormat, uint16_t milliseconds) {
    OS_EnterTaskCritical();
    rtc_SystemTime.secondsSinceEpoch = timeEpochFormat;
    rtc_SystemTime.milliseconds      = milliseconds;
    OS_ExitTaskCritical();
}

/**
 * @brief   Get the RTC system timer value.
 *
 * This function gets the timer value directly.
 * It is called by RTC_GetSystemTimeRtcFormat() which
 * returns a RTC_TIME_DATA_s parameter.
 *
 * @return      RTC system timer value in seconds since epoch format
 */
static RTC_SYSTEM_TIMER_EPOCH_s RTC_GetSystemTimeEpochFormat(void) {
    RTC_SYSTEM_TIMER_EPOCH_s systemTimerValueEpoch = {0};
    OS_EnterTaskCritical();
    systemTimerValueEpoch = rtc_SystemTime;
    OS_ExitTaskCritical();
    return systemTimerValueEpoch;
}

/**
 * @brief   Convert time from RTC_TIME_DATA_s to struct tm format.
 *
 * @param   timeRtcFormat  time in RTC_TIME_DATA_s format
 * @return  time in struct tm format
 */
static struct tm RTC_rtcFormatToTmFormat(RTC_TIME_DATA_s timeRtcFormat) {
    struct tm timeTmFormat = {0};

    timeTmFormat.tm_year = (RTC_START_YEAR + timeRtcFormat.year) - RTC_CTIME_YEAR_START;
    timeTmFormat.tm_mon  = timeRtcFormat.month - RTC_CTIME_MONTH_START;
    timeTmFormat.tm_mday = timeRtcFormat.day;
    timeTmFormat.tm_hour = timeRtcFormat.hours;
    timeTmFormat.tm_min  = timeRtcFormat.minutes;
    timeTmFormat.tm_sec  = timeRtcFormat.seconds;
    timeTmFormat.tm_wday = timeRtcFormat.weekday;

    return timeTmFormat;
}

/**
 * @brief   Convert time from struct tm to RTC_TIME_DATA_s format.
 *
 * @param   timeTmFormat  time in struct tm format
 * @return  time in RTC_TIME_DATA_s format
 */
static RTC_TIME_DATA_s RTC_tmFormatToRtcFormat(struct tm timeTmFormat) {
    RTC_TIME_DATA_s timeRtcFormat = {0};

    timeRtcFormat.year    = timeTmFormat.tm_year + RTC_CTIME_YEAR_START - RTC_START_YEAR;
    timeRtcFormat.month   = timeTmFormat.tm_mon + RTC_CTIME_MONTH_START;
    timeRtcFormat.day     = timeTmFormat.tm_mday;
    timeRtcFormat.hours   = timeTmFormat.tm_hour;
    timeRtcFormat.minutes = timeTmFormat.tm_min;
    timeRtcFormat.seconds = timeTmFormat.tm_sec;
    timeRtcFormat.weekday = timeTmFormat.tm_wday;

    return timeRtcFormat;
}

static RTC_TIME_DATA_s RTC_ReadTime(void) {
    RTC_TIME_DATA_s rtcTime     = {0};
    STD_RETURN_TYPE_e retValI2c = STD_OK;
    STD_RETURN_TYPE_e retVal    = STD_OK;

    uint8_t osfBit = 0u;

    /* Set address to read from */
    rtc_i2cWriteBuffer[0u] = RTC_REG_TIME_START_ADDR;
    retValI2c              = I2C_Write(RTC_I2C_INTERFACE, RTC_I2C_ADDRESS, 1u, rtc_i2cWriteBuffer);
    if (retValI2c == STD_NOT_OK) {
        retVal = STD_NOT_OK;
    }
    /* Address set, read time data */
    retValI2c = I2C_ReadDma(RTC_I2C_INTERFACE, RTC_I2C_ADDRESS, RTC_NUMBER_OF_TIMEDATA_BYTES, rtc_i2cReadBuffer);
    if (retValI2c == STD_NOT_OK) {
        retVal = STD_NOT_OK;
    }

    rtcTime.hundredthOfSeconds =
        (((rtc_i2cReadBuffer[RTC_100TH_OF_SECONDS_OFFSET] & RTC_HUNDREDTH_OF_SECONDS_TENS_PLACE_MASK) >>
          RTC_TENS_PLACE_OFFSET) *
         RTC_TENS_PLACE_FACTOR) +
        (rtc_i2cReadBuffer[RTC_100TH_OF_SECONDS_OFFSET] & RTC_UNITS_PLACE_MASK);
    rtcTime.seconds =
        (((rtc_i2cReadBuffer[RTC_SECONDS_OFFSET] & RTC_SECONDS_TENS_PLACE_MASK) >> RTC_TENS_PLACE_OFFSET) *
         RTC_TENS_PLACE_FACTOR) +
        (rtc_i2cReadBuffer[RTC_SECONDS_OFFSET] & RTC_UNITS_PLACE_MASK);
    /* OSF bit is stored at position 7 in seconds register */
    osfBit = (rtc_i2cReadBuffer[RTC_SECONDS_OFFSET] & RTC_SECONDS_OSF_BIT_MASK) >> RTC_SECONDS_OSF_BIT_POSITION;
    rtcTime.minutes =
        (((rtc_i2cReadBuffer[RTC_MINUTES_OFFSET] & RTC_MINUTES_TENS_PLACE_MASK) >> RTC_TENS_PLACE_OFFSET) *
         RTC_TENS_PLACE_FACTOR) +
        (rtc_i2cReadBuffer[RTC_MINUTES_OFFSET] & RTC_UNITS_PLACE_MASK);
    rtcTime.hours = (((rtc_i2cReadBuffer[RTC_HOURS_OFFSET] & RTC_HOURS_TENS_PLACE_MASK) >> RTC_TENS_PLACE_OFFSET) *
                     RTC_TENS_PLACE_FACTOR) +
                    (rtc_i2cReadBuffer[RTC_HOURS_OFFSET] & RTC_UNITS_PLACE_MASK);
    rtcTime.day = (((rtc_i2cReadBuffer[RTC_DAYS_OFFSET] & RTC_DAYS_TENS_PLACE_MASK) >> RTC_TENS_PLACE_OFFSET) *
                   RTC_TENS_PLACE_FACTOR) +
                  (rtc_i2cReadBuffer[RTC_DAYS_OFFSET] & RTC_UNITS_PLACE_MASK);
    rtcTime.weekday = (rtc_i2cReadBuffer[RTC_WEEKDAYS_OFFSET] & RTC_WEEKDAYS_UNITS_PLACE_MASK);
    rtcTime.month   = (((rtc_i2cReadBuffer[RTC_MONTHS_OFFSET] & RTC_MONTHS_TENS_PLACE_MASK) >> RTC_TENS_PLACE_OFFSET) *
                     RTC_TENS_PLACE_FACTOR) +
                    (rtc_i2cReadBuffer[RTC_MONTHS_OFFSET] & RTC_UNITS_PLACE_MASK);
    rtcTime.year = (((rtc_i2cReadBuffer[RTC_YEARS_OFFSET] & RTC_YEARS_TENS_PLACE_MASK) >> RTC_TENS_PLACE_OFFSET) *
                    RTC_TENS_PLACE_FACTOR) +
                   (rtc_i2cReadBuffer[RTC_YEARS_OFFSET] & RTC_UNITS_PLACE_MASK);

    if (retVal == STD_NOT_OK) {
        DIAG_Handler(DIAG_ID_I2C_RTC_ERROR, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
    } else {
        DIAG_Handler(DIAG_ID_I2C_RTC_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
        /* If I2C communication successful, check OSF bit */
        if (osfBit != 0u) {
            DIAG_Handler(DIAG_ID_RTC_CLOCK_INTEGRITY_ERROR, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
        } else {
            DIAG_Handler(DIAG_ID_RTC_CLOCK_INTEGRITY_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
        }
    }

    return rtcTime;
}

/*========== Extern Function Implementations ================================*/

extern void RTC_Trigger(void) {
    static uint32_t initialTimeBatteryLowCheck = 0u;
    static uint32_t initialTimeAdjustCheck     = 0u;
    uint32_t currentTime                       = OS_GetTickCount();

    STD_RETURN_TYPE_e initFinished = STD_NOT_OK;

    initFinished = RTC_Initialize();

    if (initFinished == STD_OK) {
        /* Check if an RTC CAN message came */
        RTC_SetOverCanMessage();
        /* Check battery low */
        if ((currentTime - initialTimeBatteryLowCheck) >=
            (RTC_TIME_BETWEEN_BLF_FLAG_CHECK_min * RTC_FACTOR_MIN_TO_MS)) {
            RTC_CheckBatteryLowVoltageAlert();
            initialTimeBatteryLowCheck = OS_GetTickCount();
        }
        /* Adjust time if necessary */
        if ((currentTime - initialTimeAdjustCheck) >=
            (RTC_TIME_BETWEEN_RTC_TIMER_ADJUSTMENT_min * RTC_FACTOR_MIN_TO_MS)) {
            RTC_AdjustTime();
            initialTimeAdjustCheck = OS_GetTickCount();
        }
    }
}

extern STD_RETURN_TYPE_e RTC_Initialize(void) {
    STD_RETURN_TYPE_e returnValue         = STD_NOT_OK;
    static RTC_INIT_STATES_e rtcInitState = RTC_SET_SYSTEM_TIMER;
    STD_RETURN_TYPE_e retValI2c           = STD_OK;
    STD_RETURN_TYPE_e retVal              = STD_OK;
    uint8_t otprBit                       = 1u;
    uint8_t otprTimeout                   = RTC_OTPR_BIT_WAIT_TIMEOUT_ms;
    static uint32_t initTime              = 0u;
    uint32_t currentTime                  = OS_GetTickCount();

    switch (rtcInitState) {
        case RTC_SET_SYSTEM_TIMER:
            RTC_InitializeSystemTimeWithRtc();
            rtcInitState = RTC_SET_BLF;
            OS_DelayTaskUntil(&currentTime, 2u);
            break;
        case RTC_SET_BLF:
            /* Write PWRMNG bits */
            /* Set address to write to */
            rtc_i2cWriteBuffer[0u] = RTC_REG_CONTROL_3_ADDR;
            /* Set data to write, to activate direct switching mode and battery low detection */
            rtc_i2cWriteBuffer[1u] = 0u;
            rtc_i2cWriteBuffer[1u] |= ((uint8_t)RTC_CTRL3_PWRMNG_DIRECTSWITCH_LOWDETECTIONENABLE_MODE)
                                      << RTC_CTRL3_BATTERY_PWRMNG_BITS_POSITION;
            retValI2c = I2C_WriteDma(
                RTC_I2C_INTERFACE,
                RTC_I2C_ADDRESS,
                RTC_WRITE_REGISTER_I2C_TRANSACTION_SIZE_IN_BYTES,
                rtc_i2cWriteBuffer);
            if (retValI2c == STD_NOT_OK) {
                retVal = STD_NOT_OK;
            }
            rtcInitState = RTC_CLEAR_OTPR;
            OS_DelayTaskUntil(&currentTime, 2u);
            break;

        case RTC_CLEAR_OTPR:
            /* OTP refresh begin */
            /* Clear OTPR bit */
            /* Set address to write to */
            rtc_i2cWriteBuffer[0u] = RTC_REG_CLKOUT_CTL_ADDR;
            /* Set data to write, to clear OTPR to 0 */
            rtc_i2cWriteBuffer[1u] = 0u;
            rtc_i2cWriteBuffer[1u] &= ~((uint8_t)RTC_CLKOUT_CTL_OTPR_BIT_MASK);
            retValI2c = I2C_WriteDma(
                RTC_I2C_INTERFACE,
                RTC_I2C_ADDRESS,
                RTC_WRITE_REGISTER_I2C_TRANSACTION_SIZE_IN_BYTES,
                rtc_i2cWriteBuffer);
            if (retValI2c == STD_NOT_OK) {
                retVal = STD_NOT_OK;
            }
            initTime     = OS_GetTickCount();
            rtcInitState = RTC_WAIT_CLEAR_OTPR;
            OS_DelayTaskUntil(&currentTime, 2u);
            break;

        case RTC_WAIT_CLEAR_OTPR:
            /* Read OTPR bit until cleared to 0 */

            /* Set address to read from */
            rtc_i2cWriteBuffer[0u] = RTC_REG_CLKOUT_CTL_ADDR;
            retValI2c              = I2C_WriteDma(RTC_I2C_INTERFACE, RTC_I2C_ADDRESS, 1u, rtc_i2cWriteBuffer);
            if (retValI2c == STD_NOT_OK) {
                retVal = STD_NOT_OK;
            }
            /* Address set, read data */
            retValI2c = I2C_Read(RTC_I2C_INTERFACE, RTC_I2C_ADDRESS, 1u, rtc_i2cReadBuffer);
            if (retValI2c == STD_NOT_OK) {
                retVal = STD_NOT_OK;
            }
            otprBit = (rtc_i2cReadBuffer[0u] & RTC_CLKOUT_CTL_OTPR_BIT_MASK) >> RTC_CLKOUT_CTL_OTPR_BIT_POSITION;
            if (((currentTime - initTime) > RTC_OTPR_BIT_WAIT_TIMEOUT_ms) || (otprTimeout == 0u)) {
                rtcInitState = RTC_SET_OTPR;
            }
            OS_DelayTaskUntil(&currentTime, 2u);
            break;

        case RTC_SET_OTPR:
            /* Set OTPR bit */
            /* Set address to write to */
            rtc_i2cWriteBuffer[0u] = RTC_REG_CLKOUT_CTL_ADDR;
            /* Set data to write, to set OTPR to 1 */
            rtc_i2cWriteBuffer[1u] = 0u;
            rtc_i2cWriteBuffer[1u] |= RTC_CLKOUT_CTL_OTPR_BIT_MASK;
            retValI2c = I2C_WriteDma(
                RTC_I2C_INTERFACE,
                RTC_I2C_ADDRESS,
                RTC_WRITE_REGISTER_I2C_TRANSACTION_SIZE_IN_BYTES,
                rtc_i2cWriteBuffer);
            if (retValI2c == STD_NOT_OK) {
                retVal = STD_NOT_OK;
            }
            initTime     = OS_GetTickCount();
            rtcInitState = RTC_WAIT_SET_OTPR;
            OS_DelayTaskUntil(&currentTime, 2u);
            break;

        case RTC_WAIT_SET_OTPR:
            /* Read OTPR bit until set to 1 */
            /* Set address to read from */
            rtc_i2cWriteBuffer[0u] = RTC_REG_CLKOUT_CTL_ADDR;
            retValI2c              = I2C_Write(RTC_I2C_INTERFACE, RTC_I2C_ADDRESS, 1u, rtc_i2cWriteBuffer);
            if (retValI2c == STD_NOT_OK) {
                retVal = STD_NOT_OK;
            }
            /* Address set, read data */
            retValI2c = I2C_Read(RTC_I2C_INTERFACE, RTC_I2C_ADDRESS, 1u, rtc_i2cReadBuffer);
            if (retValI2c == STD_NOT_OK) {
                retVal = STD_NOT_OK;
            }
            otprBit = (rtc_i2cReadBuffer[0u] & RTC_CLKOUT_CTL_OTPR_BIT_MASK) >> RTC_CLKOUT_CTL_OTPR_BIT_POSITION;
            if (((currentTime - initTime) > RTC_OTPR_BIT_WAIT_TIMEOUT_ms) || (otprBit == 1u)) {
                /* OTP refresh end */
                rtcInitState = RTC_INIT_SET_FINISHED;
            }
            OS_DelayTaskUntil(&currentTime, 2u);
            break;

        case RTC_INIT_SET_FINISHED:
            /* Init finished */
            returnValue = STD_OK;
            break;
        default:
            break;
    }

    if (retVal == STD_NOT_OK) {
        DIAG_Handler(DIAG_ID_I2C_RTC_ERROR, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
    } else {
        DIAG_Handler(DIAG_ID_I2C_RTC_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
    }

    return returnValue;
}

extern void RTC_InitializeSystemTimeWithRtc(void) {
    RTC_TIME_DATA_s rtc_initTime = {0};

    /* Get time and date from RTC */
    rtc_initTime = RTC_ReadTime();
    /* Set system timer */
    RTC_SetSystemTimeRtcFormat(rtc_initTime);
}

extern void RTC_IncrementSystemTime(void) {
    if (rtc_SystemTime.milliseconds < 999u) {
        rtc_SystemTime.milliseconds++;
    } else {
        rtc_SystemTime.milliseconds = 0u;
        rtc_SystemTime.secondsSinceEpoch++;
    }
}

extern void RTC_SetSystemTimeRtcFormat(RTC_TIME_DATA_s timeRtcFormat) {
    time_t systemTimeEpochFormat = 0;
    struct tm systemTimeTmFormat = {0};

    /* Convert time and date from RTC to tm struct */
    systemTimeTmFormat = RTC_rtcFormatToTmFormat(timeRtcFormat);
    /* Convert tm struct to timer in seconds since epoch */
    systemTimeEpochFormat = mktime(&systemTimeTmFormat);
    /* Set system timer */
    RTC_SetSystemTimeEpochFormat(systemTimeEpochFormat, timeRtcFormat.hundredthOfSeconds * 10u);
}

RTC_TIME_DATA_s RTC_GetSystemTimeRtcFormat(void) {
    RTC_SYSTEM_TIMER_EPOCH_s systemTimeEpochFormat = {0};
    struct tm systemTimeTmFormat                   = {0};
    RTC_TIME_DATA_s systemTimeRtcFormat            = {0};

    /* Get system timer */
    systemTimeEpochFormat = RTC_GetSystemTimeEpochFormat();
    /* Convert timer in seconds since epoch to tm struct */
    systemTimeTmFormat = *(localtime(&(systemTimeEpochFormat.secondsSinceEpoch)));
    /* Convert tm struct to RTC */
    systemTimeRtcFormat                    = RTC_tmFormatToRtcFormat(systemTimeTmFormat);
    systemTimeRtcFormat.hundredthOfSeconds = (systemTimeEpochFormat.milliseconds / 10u);

    return systemTimeRtcFormat;
}
/* AXIVION Enable Style MisraC2012-21.10: Time library only used in this module */

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
