/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * - "This product uses parts of foxBMS&reg;"
 * - "This product includes parts of foxBMS&reg;"
 * - "This product is derived from foxBMS&reg;"
 *
 */

/**
 * @file    fram_helper.h
 * @author  foxBMS Team
 * @date    2026-05-17 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup DRIVERS
 * @prefix  FRAM
 *
 * @brief   Helper for adc value calibration
 * @details TODO
 */

#ifndef FOXBMS__FRAM_HELPER_H_
#define FOXBMS__FRAM_HELPER_H_

/*========== Includes =======================================================*/

#include "fstd_types.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
typedef enum {
    FRAM_CALIBRATION_CHANNEL_0,
    FRAM_CALIBRATION_CHANNEL_1,
    FRAM_CALIBRATION_CHANNEL_2,
    FRAM_CALIBRATION_CHANNEL_3,
    FRAM_CALIBRATION_CHANNEL_4,
    FRAM_CALIBRATION_CHANNEL_5,
    FRAM_CALIBRATION_CHANNEL_6,
    FRAM_CALIBRATION_CHANNEL_7,
    FRAM_CALIBRATION_CHANNEL_8,
    FRAM_CALIBRATION_CHANNEL_9,
    FRAM_CALIBRATION_CHANNEL_MAX, /**< DO NOT CHANGE, MUST BE THE LAST ENTRY */
} FRAM_CALIBRATION_VALUE_CHANNELS_e;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   calibrates new adc value from raw value and calibration values.
 * @param  uncalibratedValue raw measurement data
 * @param  calibrationChannel for the adc calibration
 * @return calibrated value calculated from raw value and calibrations values from the calibrationChannel
 */
extern float_t FRAM_GetCalibratedValue(float_t uncalibratedValue, FRAM_CALIBRATION_VALUE_CHANNELS_e calibrationChannel);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif
#endif /* FOXBMS__FRAM_HELPER_H_ */
