/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    adc.h
 * @author  foxBMS Team
 * @date    2019-01-07 (date of creation)
 * @updated 2021-03-24 (date of last update)
 * @ingroup DRIVERS
 * @prefix  ADC
 *
 * @brief   Headers for the driver for the ADC module.
 *
 */

#ifndef FOXBMS__ADC_H_
#define FOXBMS__ADC_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "battery_system_cfg.h"

#include "HL_adc.h"

/*========== Macros and Definitions =========================================*/

/** ADC voltage reference, high */
#define ADC_VREFHIGH_mV (5000.0f)
/** ADC voltage reference, low */
#define ADC_VREFLOW_mV (0.0f)
/** ADC conversion factor, 12 bit conversion */
#define ADC_CONV_FACTOR_12BIT (4096.0f)
/** ADC conversion factor, 10 bit conversion */
#define ADC_CONV_FACTOR_10BIT (1024.0f)

/** End bit position in ADC Groupx Interrupt Flag Register */
#define ADC_CONVERSION_ENDDBIT (8U)

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

#endif /* FOXBMS__ADC_H_ */
