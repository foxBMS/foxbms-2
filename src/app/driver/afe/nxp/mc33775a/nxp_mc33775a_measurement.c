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
 * @file    nxp_mc33775a_measurement.c
 * @author  foxBMS Team
 * @date    2025-04-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  N77X
 *
 * @brief   Measurement functions of the MC33775A analog front-end driver.
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "nxp_mc3377x_measurement.h"
/* clang-format off */
#include "nxp_mc3377x-ll.h"
/* clang-format on */
#include "afe.h"
#include "database.h"
#include "foxmath.h"
#include "nxp_mc3377x.h"
#include "nxp_mc3377x_helpers.h"
#include "nxp_mc3377x_reg_def.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/**
 * @brief   initializes the measurement.
 * @param   pState state of the NXP MC33775A driver
 */
static void N775_InitMeasurement(N77X_STATE_s *pState);

/**
 * @brief   retrieves the results of voltage measurements.
 * @param   pState state of the NXP MC33775A driver
 * @param   m module index
 * @param   primaryRawValues primary raw values
 * @param   gpio03Error gpio error 03
 */
static void N775_RetrieveVoltages(N77X_STATE_s *pState, uint8_t m, uint16_t *primaryRawValues, bool *gpio03Error);

/**
 * @brief   retrieves the results of temperature measurements.
 * @param   pState state of the NXP MC33775A driver
 * @param   m module index
 * @param   gpio03Error gpio error 03
 * @param   gpio47Error gpio error 47
 */
static void N775_RetrieveTemperatures(N77X_STATE_s *pState, uint8_t m, bool *gpio03Error, bool *gpio47Error);

/**
 * @brief   retrieves the measurement results.
 * @param   pState state of the NXP MC33775A driver
 */
static void N775_RetrieveMeasurement(N77X_STATE_s *pState);

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

static void N775_InitMeasurement(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    /* Send capture command. This ends the last cycle and starts a new one */
    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS,
        MC3377X_ALLM_APP_CTRL_OFFSET,
        (1u << MC3377X_ALLM_APP_CTRL_PAUSEBAL_POS) | (0x3FFu << MC3377X_PRMM_APP_CTRL_CAPVC_POS) |
            (MC3377X_PRMM_APP_CTRL_VCOLNUM_DISABLED_ENUM_VAL << MC3377X_PRMM_APP_CTRL_VCOLNUM_POS),
        pState->pSpiTxSequence);
    /* Wait for measurements to take place */
    N77x_Wait(N77X_MEASUREMENT_CAPTURE_TIME_MS);
    /* Send capture command. This ends the last cycle and starts a new one */
    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS,
        MC3377X_ALLM_APP_CTRL_OFFSET,
        (0u << MC3377X_ALLM_APP_CTRL_PAUSEBAL_POS) | (0x3FFu << MC3377X_PRMM_APP_CTRL_CAPVC_POS) |
            (MC3377X_PRMM_APP_CTRL_VCOLNUM_DISABLED_ENUM_VAL << MC3377X_PRMM_APP_CTRL_VCOLNUM_POS),
        pState->pSpiTxSequence);
    /* Wait for measurements to be ready */
    N77x_Wait(N77X_MEASUREMENT_READY_TIME_MS);
}

static void N775_RetrieveVoltages(N77X_STATE_s *pState, uint8_t m, uint16_t *primaryRawValues, bool *gpio03Error) {
    int16_t primaryValues[20] = {0u};

    for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
        /* Store cell voltages */
        if (N77X_INVALID_REGISTER_VALUE != primaryRawValues[cb + 1u]) {
            primaryValues[cb + 1u] = (int16_t)primaryRawValues[cb + 1u];
            pState->n77xData.cellVoltage->cellVoltage_mV[pState->currentString][m][cb] =
                (((float_t)primaryValues[cb + 1u]) * 154.0e-6f * UNIT_CONVERSION_FACTOR_1000_FLOAT);
            /* String voltage measurement is calculated as sum of individual cell voltage measurement */
            pState->n77xData.cellVoltage->stringVoltage_mV[pState->currentString] +=
                pState->n77xData.cellVoltage->cellVoltage_mV[pState->currentString][m][cb];
            pState->n77xData.cellVoltage->invalidCellVoltage[pState->currentString][m][cb] = false;
            pState->n77xData.cellVoltage->nrValidCellVoltages[pState->currentString]++;
        } else {
            pState->n77xData.cellVoltage->cellVoltage_mV[pState->currentString][m][cb] =
                AFE_DEFAULT_CELL_VOLTAGE_INVALID_VALUE;
            pState->n77xData.cellVoltage->invalidCellVoltage[pState->currentString][m][cb] = true;
        }
    }
    for (uint8_t g = 0u; g < 4u; g++) {
        /* Store GPIO voltages */
        if (N77X_INVALID_REGISTER_VALUE != primaryRawValues[g + 16u]) {
            primaryValues[g + 16u] = (int16_t)primaryRawValues[g + 16u];
            pState->n77xData.allGpioVoltage
                ->gpioVoltages_mV[pState->currentString][g + (m * SLV_NR_OF_GPIOS_PER_MODULE)] =
                (((float_t)primaryValues[g + 16u]) * 154.0e-6f * 1000.0f);
        } else {
            *gpio03Error = true;
        }
    }
    /* Store module voltage */
    if (N77X_INVALID_REGISTER_VALUE != primaryRawValues[15u]) {
        primaryValues[15u] = (int16_t)primaryRawValues[15u];
        pState->n77xData.cellVoltage->moduleVoltage_mV[pState->currentString][m] =
            (((float_t)primaryValues[15u]) * 2.58e-3f * 1000.0f);
    }
}

static void N775_RetrieveTemperatures(N77X_STATE_s *pState, uint8_t m, bool *gpio03Error, bool *gpio47Error) {
    /* Set temperature values */
    if (N77X_USE_MUX_FOR_TEMP == true) {
        /* Mux case */
        if (*gpio03Error == false) {
            pState->n77xData.cellTemperature
                ->cellTemperature_ddegC[pState->currentString][m][pState->currentMux[pState->currentString]] =
                N77x_ConvertVoltagesToTemperatures(
                    pState->n77xData.allGpioVoltage
                        ->gpioVoltages_mV[pState->currentString]
                                         [N77X_MULTIPLEXER_TEMP_GPIO_POSITION + (m * SLV_NR_OF_GPIOS_PER_MODULE)]);
            pState->n77xData.cellTemperature
                ->invalidCellTemperature[pState->currentString][m][pState->currentMux[pState->currentString]] = false;
        } else {
            pState->n77xData.cellTemperature
                ->cellTemperature_ddegC[pState->currentString][m][pState->currentMux[pState->currentString]] = 0;
            pState->n77xData.cellTemperature
                ->invalidCellTemperature[pState->currentString][m][pState->currentMux[pState->currentString]] = true;
        }
    } else if (N77X_USE_MUX_FOR_TEMP == false) {
        /* No  mux case */
        if ((*gpio03Error == false) && (*gpio47Error == false)) {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                pState->n77xData.cellTemperature->cellTemperature_ddegC[pState->currentString][m][ts] =
                    N77x_ConvertVoltagesToTemperatures(
                        pState->n77xData.allGpioVoltage
                            ->gpioVoltages_mV[pState->currentString][ts + (m * SLV_NR_OF_GPIOS_PER_MODULE)]);
                pState->n77xData.cellTemperature->invalidCellTemperature[pState->currentString][m][ts] = false;
            }
        } else {
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                pState->n77xData.cellTemperature->cellTemperature_ddegC[pState->currentString][m][ts]  = 0;
                pState->n77xData.cellTemperature->invalidCellTemperature[pState->currentString][m][ts] = true;
            }
        }
    } else {
        /* Invalid value for switch case */
        FAS_ASSERT(FAS_TRAP);
    }
}

static void N775_RetrieveMeasurement(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    uint16_t primaryRawValues[20]                   = {0u};
    uint16_t secondaryRawValues[4]                  = {0u};
    uint16_t currentRawValue                        = 0u;
    int16_t secondaryValues[4]                      = {0u};
    int16_t currentValue                            = 0u;
    N77X_COMMUNICATION_STATUS_e retValPrimary       = N77X_COMMUNICATION_OK;
    N77X_COMMUNICATION_STATUS_e retValSecondary     = N77X_COMMUNICATION_OK;
    N77X_COMMUNICATION_STATUS_e retValSupplyCurrent = N77X_COMMUNICATION_OK;
    bool gpio03Error                                = false;
    bool gpio47Error                                = false;

    /* Reset number previous measurement results on string-level */
    pState->n77xData.cellVoltage->stringVoltage_mV[pState->currentString]        = 0u;
    pState->n77xData.cellVoltage->nrValidCellVoltages[pState->currentString]     = 0u;
    pState->n77xData.cellTemperature->nrValidTemperatures[pState->currentString] = 0u;

    for (uint8_t m = 0u; m < BS_NR_OF_MODULES_PER_STRING; m++) {
        uint8_t deviceAddress = m + 1u;
        retValPrimary         = N77x_CommunicationReadMultiple(
            deviceAddress, 20, 4u, MC3377X_PRMM_APP_VC_CNT_OFFSET, primaryRawValues, pState);
        retValSecondary = N77x_CommunicationReadMultiple(
            deviceAddress, 4, 4u, MC3377X_SECM_APP_AIN4_OFFSET, secondaryRawValues, pState);
        if (N77X_CHECK_SUPPLY_CURRENT == true) {
            retValSupplyCurrent =
                N77x_CommunicationRead(deviceAddress, MC3377X_SECM_PER_NPNISENSE_OFFSET, &currentRawValue, pState);
        }

        N77x_ErrorHandling(pState, retValPrimary, m);
        if (retValPrimary == N77X_COMMUNICATION_OK) {
            N775_RetrieveVoltages(pState, m, primaryRawValues, &gpio03Error);
        } else {
            /* Reset voltage values and the relevant invalid flag if the communication is not ok */
            for (uint8_t cb = 0u; cb < BS_NR_OF_CELL_BLOCKS_PER_MODULE; cb++) {
                pState->n77xData.cellVoltage->cellVoltage_mV[pState->currentString][m][cb]     = 0;
                pState->n77xData.cellVoltage->invalidCellVoltage[pState->currentString][m][cb] = true;
            }
        }

        N77x_ErrorHandling(pState, retValSecondary, m);
        if (retValSecondary == N77X_COMMUNICATION_OK) {
            for (uint8_t g = 4u; g < 8u; g++) {
                if (N77X_INVALID_REGISTER_VALUE != secondaryRawValues[g - 4u]) {
                    secondaryValues[g - 4u] = (int16_t)secondaryRawValues[g - 4u];
                    pState->n77xData.allGpioVoltage
                        ->gpioVoltages_mV[pState->currentString][g + (m * SLV_NR_OF_GPIOS_PER_MODULE)] =
                        (((float_t)secondaryValues[g - 4u]) * 154.0e-6f * 1000.0f);
                } else {
                    gpio47Error = true;
                }
            }
        }

        N77x_ErrorHandling(pState, retValPrimary, m);
        if (retValPrimary == N77X_COMMUNICATION_OK) {
            N775_RetrieveTemperatures(pState, m, &gpio03Error, &gpio47Error);
        } else {
            /* Reset temperature values the relevant invalid flag if the communication is not ok */
            for (uint8_t ts = 0u; ts < BS_NR_OF_TEMP_SENSORS_PER_MODULE; ts++) {
                pState->n77xData.cellTemperature->cellTemperature_ddegC[pState->currentString][m][ts]  = 0;
                pState->n77xData.cellTemperature->invalidCellTemperature[pState->currentString][m][ts] = true;
            }
        }

        if (N77X_CHECK_SUPPLY_CURRENT == true) {
            N77x_ErrorHandling(pState, retValSupplyCurrent, m);
            if (retValSupplyCurrent == N77X_COMMUNICATION_OK) {
                if (N77X_INVALID_REGISTER_VALUE != currentRawValue) {
                    currentValue = (int16_t)currentRawValue;
                    pState->n77xData.supplyCurrent->current[pState->currentString][m] =
                        (((float_t)currentValue) * 7.69e-6f * 1000.0f);
                }
            }
        }
    }

    DATA_WRITE_DATA(pState->n77xData.cellVoltage, pState->n77xData.cellTemperature, pState->n77xData.allGpioVoltage);
}

/*========== Extern Function Implementations ================================*/
extern void N77x_CaptureMeasurement(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    N775_InitMeasurement(pState);
    N775_RetrieveMeasurement(pState);
}

extern void N77x_StartMeasurement(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    /* Enable cell voltage measurements */
    N77x_CommunicationWrite(N77X_BROADCAST_ADDRESS, MC3377X_ALLM_VCVB_CFG_OFFSET, 0x3FFF, pState->pSpiTxSequence);
    N77x_Wait(N77X_T_WAIT_CYC_SOC_MS);
    /* Enable analog inputs 0-3 and module voltage measurement */
    N77x_CommunicationWrite(N77X_BROADCAST_ADDRESS, MC3377X_PRMM_AIN_CFG_OFFSET, 0x1F, pState->pSpiTxSequence);
    N77x_Wait(N77X_T_WAIT_CYC_SOC_MS);
    /* Enable analog inputs 4-7 measurement */
    N77x_CommunicationWrite(N77X_BROADCAST_ADDRESS, MC3377X_SECM_AIN_CFG_OFFSET, 0x0F, pState->pSpiTxSequence);
    N77x_Wait(N77X_T_WAIT_CYC_SOC_MS);
    /* Set pause of balancing before measurement start, enable the measurement units simultaneously */
    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS,
        MC3377X_ALLM_CFG_OFFSET,
        (N77X_BALPAUSELEN_10US << MC3377X_ALLM_CFG_BALPAUSELEN_POS) | (1 << MC3377X_ALLM_CFG_MEASEN_POS),
        pState->pSpiTxSequence);
    N77x_Wait(N77X_T_WAIT_CYC_SOC_MS);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_N77x_InitMeasurement(N77X_STATE_s *pState) {
    N775_InitMeasurement(pState);
}
extern void TEST_N77x_RetrieveVoltages(N77X_STATE_s *pState, uint8_t m, uint16_t *primaryRawValues, bool *gpio03Error) {
    N775_RetrieveVoltages(pState, m, primaryRawValues, gpio03Error);
}
extern void TEST_N77x_RetrieveTemperatures(N77X_STATE_s *pState, uint8_t m, bool *gpio03Error, bool *gpio47Error) {
    N775_RetrieveTemperatures(pState, m, gpio03Error, gpio47Error);
}
extern void TEST_N77x_RetrieveMeasurement(N77X_STATE_s *pState) {
    N775_RetrieveMeasurement(pState);
}
#endif
