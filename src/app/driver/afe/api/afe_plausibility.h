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
 * @file    afe_plausibility.h
 * @author  foxBMS Team
 * @date    2019-01-24 (date of creation)
 * @updated 2021-01-15 (date of last update)
 * @ingroup MODULES
 * @prefix  AFE
 *
 * @brief   plausibility checks for cell voltage and cell temperatures
 *
 */

#ifndef FOXBMS__AFE_PLAUSIBILITY_H_
#define FOXBMS__AFE_PLAUSIBILITY_H_

/*========== Includes =======================================================*/
#include "general.h"

/*========== Macros and Definitions =========================================*/

/**
 * @brief   struct definition for plausibility values of an AFE
 * @details This struct has to be implemented in a variable by each AFE and
 *          passed to #AFE_PlausibilityCheckVoltageMeasurementRange().
 */
typedef struct AFE_PLAUSIBILITY_VALUES {
    /** largest cell voltage value that is considered plausible in mV
     *
     * This value should be the maximum operating limit of the AFE.
    */
    const int16_t maximumPlausibleVoltage_mV;
    /** smallest cell voltage value that is considered plausible in mV
     *
     * This value should be the minimum operating limit of the AFE.
    */
    const int16_t minimumPlausibleVoltage_mV;
} AFE_PLAUSIBILITY_VALUES_s;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief  Cell voltage measurement range plausibility check
 *
 * @param  cellvoltage_mV   cell voltage that is checked in mV
 * @param  plausibleValues  struct of type #AFE_PLAUSIBILITY_VALUES_s with the
 *                          plausible limits of cell voltages
 *
 * @return #STD_OK if no plausibility issue detected, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e AFE_PlausibilityCheckVoltageMeasurementRange(
    const int16_t cellvoltage_mV,
    const AFE_PLAUSIBILITY_VALUES_s plausibleValues);

/**
 * @brief  Cell temperature plausibility check
 *
 * @param  celltemperature_ddegC  cell temperature that is checked in deci &deg;C
 */
extern STD_RETURN_TYPE_e AFE_PlausibilityCheckTempMinMax(const int16_t celltemperature_ddegC);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__AFE_PLAUSIBILITY_H_ */
