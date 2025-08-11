/**
 *
 * @copyright &copy; 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    adc.h
 * @author  foxBMS Team
 * @date    2019-01-07 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  ADC
 *
 * @brief   Headers for the driver for the ADC module.
 * @details TODO
 */

#ifndef FOXBMS__ADC_H_
#define FOXBMS__ADC_H_

/*========== Includes =======================================================*/

#include "battery_system_cfg.h"

#include "HL_adc.h"

#include <stdint.h>

#ifdef UNITY_UNIT_TEST
#include "database.h"

#include <math.h>
#endif

/*========== Macros and Definitions =========================================*/

/** ADC voltage reference, high */
#define ADC_VREFHIGH_mV (5000.0f)
/** ADC voltage reference, low */
#define ADC_VREFLOW_mV (0.0f)
/** ADC conversion factor, 12 bit conversion */
#define ADC_CONVERSION_FACTOR_12BIT (4096.0f)
/** ADC conversion factor, 10 bit conversion */
#define ADC_CONVERSION_FACTOR_10BIT (1024.0f)
/** ADC conversion offset */
#define ADC_CONVERSION_OFFSET (0.5f)

/** End bit position in ADC Groupx Interrupt Flag Register */
#define ADC_CONVERSION_ENDDBIT (8u)

/**
 * State for the ADC conversion
 */
typedef enum {
    ADC_START_CONVERSION,
    ADC_WAIT_CONVERSION_FINISHED,
    ADC_CONVERSION_FINISHED,
} ADC_STATE_e;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   controls ADC measurement sequence.
 *
 */
extern void ADC_Control(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern float_t TEST_ADC_ConvertVoltage(uint16_t adcCounts);
extern void TEST_ADC_SetAdcConversionState(ADC_STATE_e state);
extern ADC_STATE_e TEST_ADC_GetAdcConversionState(void);
extern DATA_BLOCK_ADC_VOLTAGE_s *TEST_ADC_GetAdc1Voltages(void);
#endif

#endif /* FOXBMS__ADC_H_ */
