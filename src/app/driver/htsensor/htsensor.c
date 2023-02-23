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
 * @file    htsensor.c
 * @author  foxBMS Team
 * @date    2021-08-05 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  HTSEN
 *
 * @brief   Driver for the Sensirion SHT35-DIS I2C humidity/temperature sensor
 *
 */

/*========== Includes =======================================================*/
#include "htsensor.h"

#include "database.h"
#include "i2c.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** Sensor I2C interface */
#define HTSEN_I2C_INTERFACE (i2cREG1)
/** Sensor I2C address */
#define HTSEN_I2C_ADDRESS (0x44u)

/** Number of read tries before restart, to avoid infinite waiting for results */
#define HTSEN_READ_TRIES (5u)

/** Conversion coefficients to get measurement from raw temperature value @{ */
#define HTSEN_TEMP_SCALING     (175.0f)
#define HTSEN_TEMP_OFFSET      (-45.0f)
#define HTSEN_TEMP_DEG_TO_DDEG (10.0f)
/**@}*/

/** Conversion coefficients to get measurement from raw humidity value @{ */
#define HTSEN_HUMIDITY_SCALING (100.0f)
#define HTSEN_FULL_SCALE       (65535.0f)
/**@}*/

/** Defines for byte positions for data handling @{ */
#define HTSEN_TEMPERATURE_LSB             (1u)
#define HTSEN_TEMPERATURE_MSB             (0u)
#define HTSEN_TEMPERATURE_BYTE_CRC        (2u)
#define HTSEN_HUMIDITY_LSB                (4u)
#define HTSEN_HUMIDITY_MSB                (3u)
#define HTSEN_HUMIDITY_BYTE_CRC           (5u)
#define HTSEN_BYTE_SHIFT                  (8u)
#define HTSEN_MEASUREMENT_LENGTH_IN_BYTES (2u)
#define HTSEN_TOTAL_DATA_LENGTH_IN_BYTES  (6u)
/**@}*/

/** Defines for sensor CRC computation @{ */
#define HTSEN_CRC_POLYNOMIAL (0x31u)
#define HTSEN_CRC_SEED       (0xFF)
#define HTSEN_CRC_BYTE_SHIFT (0x8u)
#define HTSEN_CRC_MSB_MASK   (0x80u)
#define HTSEN_CRC_8BIT_MASK  (0xFFu)
/**@}*/

/**
 * Definition of single measurement command
 * data sheet February 2019 - Version 6, table 9, page 10
 *
 * Clock stretching can be used to let sensor tell that the measurement
 * is not finished by pulling the clock low
 * Not recommended: if system is stopped during clock stretching,
 * sensor could be blocked in this mode and render I2C bus unusable.
 * !!!---WARNING---!!!: if clock stretching is used, the timeout in the
 * receive loop of the function #I2C_Read() must be adapted,
 * the current value is too small compared to the time the slave will
 * make the master wait in the clock stretching mode.
 *
 * Depending on the clock stretching setting, the code for the instructions
 * changes:
 * -> With stretching
 *  - 1 MSB
 *    - 1 LSB for high repeatability
 *    - 1 LSB for medium repeatability
 *    - 1 LSB for low repeatability
 * -> Without stretching
 *  - 1 MSB
 *    - 1 LSB for high repeatability
 *    - 1 LSB for medium repeatability
 *    - 1 LSB for low repeatability
 * This results in 6 possible configurations
 * Durability changes the measurement duration.
 * @{
 */
#define HTSEN_CLOCK_STRETCHING (false)
#if (HTSEN_CLOCK_STRETCHING == false)
#define HTSEN_SINGLE_MEAS_MSB      (0x24u)
#define HTSEN_HIGH_REPEATABILITY   (0x00u)
#define HTSEN_MEDIUM_REPEATABILITY (0x0Bu)
#define HTSEN_LOW_REPEATABILITY    (0x16u)
#else
#define HTSEN_SINGLE_MEAS_MSB      (0x2Cu)
#define HTSEN_HIGH_REPEATABILITY   (0x06u)
#define HTSEN_MEDIUM_REPEATABILITY (0x0Du)
#define HTSEN_LOW_REPEATABILITY    (0x10u)
#endif

#define HTSEN_SINGLE_MEAS_LSB (HTSEN_HIGH_REPEATABILITY)
/**@}*/

/*========== Static Constant and Variable Definitions =======================*/
#pragma SET_DATA_SECTION(".sharedRAM")
uint8_t i2cReadBuffer[HTSEN_TOTAL_DATA_LENGTH_IN_BYTES] = {0u, 0u, 0u, 0u, 0u, 0u};
uint8_t i2cWriteBuffer[2u]                              = {HTSEN_SINGLE_MEAS_MSB, HTSEN_SINGLE_MEAS_LSB};
#pragma SET_DATA_SECTION()

/** variable to store the measurement results */
static DATA_BLOCK_HTSEN_s htsen_data = {.header.uniqueId = DATA_BLOCK_ID_HTSEN};
/** number of tries to read the sensor data before requested a new measurement */
static uint8_t htsen_readTries = 0u;

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   computes CRC8.
 * @param[in]   data:    data to use to compute CRC
 * @param[in]   length:  length of data
 * @return  8-bit CRC value
 */
static uint8_t HTSEN_CalculateCrc8(const uint8_t *data, uint32_t length);

/**
 * @brief   computes temperature measurement from raw value.
 * @param[in]   data:    raw temperature value
 * @return  temperature measurement in deci &deg;C
 */
static int16_t HTSEN_ConvertRawTemperature(uint16_t data);

/**
 * @brief   computes humidity measurement from raw value.
 * @param[in]   data:    raw humidity value
 * @return  humidity measurement in %
 */
static uint8_t HTSEN_ConvertRawHumidity(uint16_t data);

/*========== Static Function Implementations ================================*/

static uint8_t HTSEN_CalculateCrc8(const uint8_t *data, uint32_t length) {
    FAS_ASSERT(data != NULL_PTR);
    FAS_ASSERT(length <= UINT32_MAX);

    uint16_t crc = HTSEN_CRC_SEED;

    for (uint8_t i = 0u; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0u; j < HTSEN_CRC_BYTE_SHIFT; j++) {
            if ((crc & HTSEN_CRC_MSB_MASK) != 0u) {
                crc = ((crc << 1u) & HTSEN_CRC_8BIT_MASK) ^ HTSEN_CRC_POLYNOMIAL;
            } else {
                crc = (crc & HTSEN_CRC_8BIT_MASK) << 1u;
            }
        }
    }
    return (uint8_t)(crc & HTSEN_CRC_8BIT_MASK);
}

static int16_t HTSEN_ConvertRawTemperature(uint16_t data) {
    /* AXIVION Routine Generic-MissingParameterAssert: data: parameter accepts whole range */
    float_t temperature_ddeg = HTSEN_TEMP_DEG_TO_DDEG *
                               (HTSEN_TEMP_OFFSET + (HTSEN_TEMP_SCALING * (((float_t)data) / HTSEN_FULL_SCALE)));
    return (int16_t)temperature_ddeg;
}

static uint8_t HTSEN_ConvertRawHumidity(uint16_t data) {
    float_t humidity_perc = (HTSEN_HUMIDITY_SCALING * (((float_t)data) / HTSEN_FULL_SCALE));
    return (uint8_t)humidity_perc;
}

/*========== Extern Function Implementations ================================*/

extern void HTSEN_Trigger(void) {
    static HTSEN_STATE_e htsenState    = HTSEN_START_MEAS;
    STD_RETURN_TYPE_e htsenReturnValue = STD_OK;
    uint32_t current_time              = OS_GetTickCount();

    switch (htsenState) {
        case HTSEN_START_MEAS:
            /* Trigger a measurement */
            htsenReturnValue = I2C_WriteDma(HTSEN_I2C_INTERFACE, HTSEN_I2C_ADDRESS, 2u, i2cWriteBuffer);
            OS_DelayTaskUntil(&current_time, 2u);
            if (htsenReturnValue == STD_OK) {
                htsenState = HTSEN_READ_RESULTS;
            }
            htsen_readTries = HTSEN_READ_TRIES;
            break;
        case HTSEN_READ_RESULTS:
            /* Try to read values */
            htsenReturnValue =
                I2C_ReadDma(HTSEN_I2C_INTERFACE, HTSEN_I2C_ADDRESS, HTSEN_TOTAL_DATA_LENGTH_IN_BYTES, i2cReadBuffer);
            OS_DelayTaskUntil(&current_time, 2u);
            if (htsenReturnValue == STD_OK) {
                /* If sensor acknowledges on I2C bus, results are available */
                /* Check if CRC valid */
                /* Only take temperature value if CRC valid */
                if (i2cReadBuffer[HTSEN_TEMPERATURE_BYTE_CRC] ==
                    HTSEN_CalculateCrc8(&i2cReadBuffer[HTSEN_TEMPERATURE_MSB], HTSEN_MEASUREMENT_LENGTH_IN_BYTES)) {
                    htsen_data.temperature_ddegC = HTSEN_ConvertRawTemperature(
                        (((uint16_t)i2cReadBuffer[HTSEN_TEMPERATURE_MSB]) << HTSEN_BYTE_SHIFT) |
                        (uint16_t)i2cReadBuffer[HTSEN_TEMPERATURE_LSB]);
                }
                /* Only take humidity value if CRC valid */
                if (i2cReadBuffer[HTSEN_HUMIDITY_BYTE_CRC] ==
                    HTSEN_CalculateCrc8(&i2cReadBuffer[HTSEN_HUMIDITY_MSB], HTSEN_MEASUREMENT_LENGTH_IN_BYTES)) {
                    htsen_data.humidity_perc = HTSEN_ConvertRawHumidity(
                        (((uint16_t)i2cReadBuffer[HTSEN_HUMIDITY_MSB]) << HTSEN_BYTE_SHIFT) |
                        (uint16_t)i2cReadBuffer[HTSEN_HUMIDITY_LSB]);
                }
                DATA_WRITE_DATA(&htsen_data);
                htsenState = HTSEN_START_MEAS;
            } else {
                /* If sensor does not acknowledge on I2C bus, results are not available ye */
                if (htsen_readTries > 0u) {
                    htsen_readTries--;
                } else {
                    htsenState = HTSEN_START_MEAS;
                }
            }
            break;
        default:
            /* invalid state */
            FAS_ASSERT(FAS_TRAP);
            break;
    }
}

/*========== Getter for static Variables (Unit Test) ========================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern uint8_t TEST_HTSEN_TestCalculateCrc8(uint8_t *data, uint32_t length) {
    return HTSEN_CalculateCrc8(data, 2u);
}
#endif
