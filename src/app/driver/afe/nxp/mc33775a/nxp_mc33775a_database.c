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
 * @file    nxp_mc33775a_database.c
 * @author  foxBMS Team
 * @date    2025-03-21 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup DRIVERS
 * @prefix  N775
 *
 * @brief   Database functions of the MC33775A analog front-end driver.
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "nxp_mc33775a_database.h"

#include "database.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern void N775_InitializeDatabase(N775_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        pState->n775Data.cellVoltage->state                    = 0u;
        pState->n775Data.minMax->minimumCellVoltage_mV[s]      = 0;
        pState->n775Data.minMax->maximumCellVoltage_mV[s]      = 0;
        pState->n775Data.minMax->nrModuleMinimumCellVoltage[s] = 0u;
        pState->n775Data.minMax->nrModuleMaximumCellVoltage[s] = 0u;
        pState->n775Data.minMax->nrCellMinimumCellVoltage[s]   = 0u;
        pState->n775Data.minMax->nrCellMaximumCellVoltage[s]   = 0u;
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                pState->n775Data.cellVoltage->cellVoltage_mV[s][m][cb]     = 0;
                pState->n775Data.cellVoltage->invalidCellVoltage[s][m][cb] = true;
            }
        }

        pState->n775Data.cellTemperature->state                = 0u;
        pState->n775Data.minMax->minimumTemperature_ddegC[s]   = 0;
        pState->n775Data.minMax->maximumTemperature_ddegC[s]   = 0;
        pState->n775Data.minMax->nrModuleMinimumTemperature[s] = 0u;
        pState->n775Data.minMax->nrModuleMaximumTemperature[s] = 0u;
        pState->n775Data.minMax->nrSensorMinimumTemperature[s] = 0u;
        pState->n775Data.minMax->nrSensorMaximumTemperature[s] = 0u;

        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                pState->n775Data.cellTemperature->cellTemperature_ddegC[s][m][ts]  = 0;
                pState->n775Data.cellTemperature->invalidCellTemperature[s][m][ts] = true;
            }
        }

        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                pState->n775Data.balancingControl->activateBalancing[s][m][cb] = false;
            }
        }
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            pState->n775Data.errorTable->communicationOk[s][m]        = false;
            pState->n775Data.errorTable->noCommunicationTimeout[s][m] = false;
            pState->n775Data.errorTable->crcIsValid[s][m]             = false;
            pState->n775Data.errorTable->mux0IsOk[s][m]               = false;
            pState->n775Data.errorTable->mux1IsOK[s][m]               = false;
            pState->n775Data.errorTable->mux2IsOK[s][m]               = false;
            pState->n775Data.errorTable->mux3IsOK[s][m]               = false;
        }
        for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            pState->n775Data.uid[s][m] = 0u;
        }
    }

    DATA_WRITE_DATA(
        pState->n775Data.cellVoltage,
        pState->n775Data.cellTemperature,
        pState->n775Data.minMax,
        pState->n775Data.balancingControl);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_N775_InitializeDatabase(N775_STATE_s *pState) {
    N775_InitializeDatabase(pState);
}
#endif
