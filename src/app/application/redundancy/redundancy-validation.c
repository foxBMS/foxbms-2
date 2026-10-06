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
 * @file    redundancy-validation.c
 * @author  foxBMS Team
 * @date    2026-07-22 (date of creation)
 * @updated 2026-10-06 (date of last update)
 * @version v1.12.0
 * @ingroup APPLICATION
 * @prefix  MCR
 *
 * @brief   Redundancy checks for cell voltage and cell temperatures for
 *          redundant measurement values
 * @details This module implements redundancy checks for cell voltage and
 *          cell temperature for redundant measurement values.
 *          The redundancy checks are based on the difference between the
 *          base and redundant measurement values.
 */

/*========== Includes =======================================================*/
#include "redundancy-validation.h"

#include "redundancy_cfg.h"

#include "fassert.h"
#include "fstd_types.h"

#include <stdint.h>
#include <stdlib.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e MCR_CheckCellVoltage(
    int16_t baseCellVoltage,
    int16_t redundancy0CellVoltage,
    int16_t *pCellVoltage) {
    /* AXIVION Routine Generic-MissingParameterAssert: baseCellVoltage: parameter accepts whole range */
    /* AXIVION Routine Generic-MissingParameterAssert: redundancy0CellVoltage: parameter accepts whole range */
    FAS_ASSERT(pCellVoltage != NULL_PTR);

    STD_RETURN_TYPE_e retval = STD_OK;

    if (abs(baseCellVoltage - redundancy0CellVoltage) > MCR_CELL_VOLTAGE_TOLERANCE_mV) {
        retval = STD_NOT_OK;
    }
    /* Take the average value of base and redundant measurement value */
    *pCellVoltage = (baseCellVoltage + redundancy0CellVoltage) / 2;
    return retval;
}

extern STD_RETURN_TYPE_e MCR_CheckCellTemperature(
    int16_t baseCellTemperature,
    int16_t redundancy0CellTemperature,
    int16_t *pCellTemperature) {
    /* AXIVION Routine Generic-MissingParameterAssert: baseCellTemperature: parameter accepts whole range */
    /* AXIVION Routine Generic-MissingParameterAssert: redundancy0CellTemperature: parameter accepts whole range */
    FAS_ASSERT(pCellTemperature != NULL_PTR);

    STD_RETURN_TYPE_e retval = STD_OK;

    if (abs(baseCellTemperature - redundancy0CellTemperature) > MCR_CELL_TEMPERATURE_TOLERANCE_dK) {
        retval = STD_NOT_OK;
    }
    /* Take the average value of base and redundant measurement value */
    *pCellTemperature = (baseCellTemperature + redundancy0CellTemperature) / 2;
    return retval;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
