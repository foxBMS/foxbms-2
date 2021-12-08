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
 * @file    mxm_1785x.c
 * @author  foxBMS Team
 * @date    2019-01-15 (date of creation)
 * @updated 2021-12-06 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MXM
 *
 * @brief   Driver for the MAX17841B ASCI and MAX1785x monitoring chip
 *
 * @details This file contains the main-state-machine that drives the
 *          monitoring ICs of the MAX1785x family by Maxim Integrated.
 *
 */

/*========== Includes =======================================================*/
#include "mxm_1785x.h"

#include "afe_plausibility.h"
#include "database.h"
#include "mxm_1785x_tools.h"
#include "mxm_battery_management.h"
#include "mxm_registry.h"
#include "os.h"
#include "tsi.h"

/*========== Macros and Definitions =========================================*/

#define MXM_OFFSET_CELL_1  (0u)  /*!< storage offset for cell 1 */
#define MXM_OFFSET_CELL_2  (1u)  /*!< storage offset for cell 2 */
#define MXM_OFFSET_CELL_3  (2u)  /*!< storage offset for cell 3 */
#define MXM_OFFSET_CELL_4  (3u)  /*!< storage offset for cell 4 */
#define MXM_OFFSET_CELL_5  (4u)  /*!< storage offset for cell 5 */
#define MXM_OFFSET_CELL_6  (5u)  /*!< storage offset for cell 6 */
#define MXM_OFFSET_CELL_7  (6u)  /*!< storage offset for cell 7 */
#define MXM_OFFSET_CELL_8  (7u)  /*!< storage offset for cell 8 */
#define MXM_OFFSET_CELL_9  (8u)  /*!< storage offset for cell 9 */
#define MXM_OFFSET_CELL_10 (9u)  /*!< storage offset for cell 10 */
#define MXM_OFFSET_CELL_11 (10u) /*!< storage offset for cell 11 */
#define MXM_OFFSET_CELL_12 (11u) /*!< storage offset for cell 12 */
#define MXM_OFFSET_CELL_13 (12u) /*!< storage offset for cell 13 */
#define MXM_OFFSET_CELL_14 (13u) /*!< storage offset for cell 14 */
#define MXM_OFFSET_AUX0    (0u)  /*!< storage offset for AUX0 */
#define MXM_OFFSET_AUX2    (2u)  /*!< storage offset for AUX2 */
#define MXM_OFFSET_AUX3    (3u)  /*!< storage offset for AUX3 */
#define MXM_OFFSET_BLOCK   (0u)  /*!< storage offset for block voltage */

/** maximum number of errors that may occur before the next error triggers a reset */
#define MXM_MAXIMUM_ERROR_COUNT (10u)

/** timeout after which the error counter will be reset */
#define MXM_TIMEOUT_RESET_ERROR_COUNTER_ms (500)

/** maximum length of a test buffer */
#define MXM_TEST_BUFFER_MAXIMUM_LENGTH (120u)

/** local definition of plausible cell voltage values */
static const AFE_PLAUSIBILITY_VALUES_s mxm_plausibleCellVoltages = {
    .maximumPlausibleVoltage_mV = 5000,
    .minimumPlausibleVoltage_mV = -2500,
};

/*========== Static Constant and Variable Definitions =======================*/
/** @brief VAA reference voltage (3.3V) */
#define MXM_REF_VAA_mV (3300u)

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/** @brief Retrieves data from lower statemachine and writes it to the rx buffer. */
static void MXM_GetDataFrom5XStateMachine(MXM_MONITORING_INSTANCE_s *pInstance);

/**
 * @brief       Parse voltage values from a READALL receive buffer into an
 *              array.
 * @details     This function expects a preprocessed RX buffer with an answer
 *              to a READALL command. It will parse the data bytes from this
 *              message into an array structured like mxm_local_cellvoltages.
 *              The offset of the measured cell has to be defined. For example
 *              cell 1 has an offset of 0 and cell 4 has an offset of 3. The
 *              offset for the READALL command is always defined in reference
 *              to module 0.
 *
 *              If is_temperature_measurement is set to true, the function
 *              expects an auxiliary measurement (e.g. temperatures).
 * @param[in]   kpkVoltRxBuffer         array-pointer to the RX buffer
 * @param[in]   voltRxBufferLength      length of the RX buffer
 * @param[in]   measurementOffset       offset of the data in the cell voltage
 *                                      array (target)
 * @param[in]   conversionType          type of conversion that has been used
 *                                      for the measured data
 * @param[out]  pVoltagesTarget         array-pointer to the cell voltages
 *                                      array
 * @param[in]   measurementType         whether the measurement is temperature
 *                                      or cell
 * @param[in]   fullScaleReference_mV   reference voltage of full scale
 */
static void MXM_ParseVoltageLineReadall(
    const uint8_t *const kpkVoltRxBuffer,
    uint16_t voltRxBufferLength,
    uint8_t measurementOffset,
    MXM_CONVERSION_TYPE_e conversionType,
    uint16_t *pVoltagesTarget,
    MXM_MEASURE_TYPE_e measurementType,
    uint32_t fullScaleReference_mV);

/**
 * @brief       Parse a RX buffer containing voltage values.
 * @details     This function parses a RX buffer containing the answer to a
 *              READALL command. It will check whether the message contains a
 *              READALL command and whether a supported register has been
 *              queried.
 *              Depending on the queried register, it will pass the proper
 *              parameters to #MXM_ParseVoltageLineReadall(). The contained
 *              data from this message will be parsed into a struct structured
 *              like #MXM_MONITORING_INSTANCE_s::localVoltages.
 * @param[in]   kpkVoltageRxBuffer      array-pointer to the RX buffer
 * @param[in]   voltageRxBufferLength   length of the RX buffer
 * @param[out]  datastorage             contains all measured voltages for local
 *                                      consumption in the module
 * @param[in]   conversionType          type of conversion that has been used for
 *                                      the measured data
 * @return      #STD_NOT_OK in the case that the RX buffer does not contain a
 *              valid message or the conversion-type is unknown,
 *              otherwise #STD_OK
 */
static STD_RETURN_TYPE_e MXM_ParseVoltageReadall(
    const uint8_t *const kpkVoltageRxBuffer,
    uint16_t voltageRxBufferLength,
    MXM_DATA_STORAGE_s *datastorage,
    MXM_CONVERSION_TYPE_e conversionType);

/**
 * @brief   Test the #MXM_ParseVoltageReadall()-function
 * @details Test the function #MXM_ParseVoltageReadall() by passing predefined
 *          RX buffer to it and checking the outcome. This function writes to
 *          the variable #MXM_MONITORING_INSTANCE_s::localVoltages and nulls it
 *          completely after execution. It is intended as a self-check that can
 *          be performed during startup of the driver.
 * @return  #STD_OK if the self-check has been performed successfully,
 *          otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e must_check_return MXM_ParseVoltageReadallTest(MXM_MONITORING_INSTANCE_s *pInstance);

/**
 * @brief           Execute all preinit selfchecks.
 * @details         Executes the following self-checks:
 *                      - #MXM_CRC8SelfTest()
 *                      - #MXM_ConvertTest()
 *                      - #MXM_FirstSetBitTest()
 *                      - #MXM_ExtractValueFromRegisterTest()
 *                      - #MXM_ParseVoltageReadallTest()
 *                      - #MXM_5XUserAccessibleAddressSpaceCheckerSelfCheck()
 *
 *                  These self-checks do not need an initialized daisy-chain
 *                  and can therefore be executed at startup of the
 *                  state-machine.
 *
 *                  After execution of each test, the return value is stored in
 *                  the supplied state-struct. The function returns whether the
 *                  self-check has successfully passed.
 * @param[in,out]   pState  pointer to the state-struct, will write status into
 * @return          #STD_OK on success, #STD_NOT_OK on failure,
 *                  return value has to be used
 */
static STD_RETURN_TYPE_e must_check_return MXM_PreInitSelfCheck(MXM_MONITORING_INSTANCE_s *pState);

/*========== Static Function Implementations ================================*/

static void MXM_GetDataFrom5XStateMachine(MXM_MONITORING_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    STD_RETURN_TYPE_e getBufferResult =
        MXM_5XGetRXBuffer(pInstance->pInstance5X, pInstance->rxBuffer, MXM_RX_BUFFER_LENGTH);
    FAS_ASSERT(getBufferResult == STD_OK);
    pInstance->dcByte = MXM_5XGetLastDCByte(pInstance->pInstance5X);
}

static void MXM_ParseVoltageLineReadall(
    const uint8_t *const kpkVoltRxBuffer,
    uint16_t voltRxBufferLength,
    uint8_t measurementOffset,
    MXM_CONVERSION_TYPE_e conversionType,
    uint16_t *pVoltagesTarget,
    MXM_MEASURE_TYPE_e measurementType,
    uint32_t fullScaleReference_mV) {
    FAS_ASSERT(kpkVoltRxBuffer != NULL_PTR);
    FAS_ASSERT(pVoltagesTarget != NULL_PTR);

    /* assert that assumptions behind computation of numberOfConnectedDevices
    are correct */
    FAS_ASSERT(voltRxBufferLength >= (2u + 2u));
    FAS_ASSERT(((voltRxBufferLength - 2u - 2u) / 2u) <= (uint16_t)UINT8_MAX);
    /* buffer-length - length of start - length of end divided by two (LSB and MSB) */
    const uint8_t numberOfConnectedDevices = (uint8_t)(((voltRxBufferLength - 2u - 2u) / 2u) & (uint8_t)UINT8_MAX);
    /* TODO impact of alive counter on rxBufferLength
     * otherwise offset at the end of message is currently 2 (DATACHECKBYTE and CRC) */
    for (uint8_t i = 2u; i < (voltRxBufferLength - 2u); i = i + 2u) {
        const uint8_t calculatedModuleNumber = numberOfConnectedDevices - ((i / 2u) - 1u) - 1u;
        uint16_t calculatedModulePosition    = 0u;
        switch (measurementType) {
            case MXM_MEASURE_TEMP:
                calculatedModulePosition = (uint16_t)calculatedModuleNumber * MXM_MAXIMUM_NR_OF_AUX_PER_MODULE;
                break;
            case MXM_MEASURE_CELL_VOLTAGE:
                calculatedModulePosition = (uint16_t)calculatedModuleNumber * MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE;
                break;
            case MXM_MEASURE_BLOCK_VOLTAGE:
                calculatedModulePosition = calculatedModuleNumber;
                break;
            default:
                FAS_ASSERT(FAS_TRAP);
                break;
        }
        FAS_ASSERT((calculatedModulePosition + measurementOffset) <= (uint16_t)UINT16_MAX);
        uint16_t calculatedArrayPosition = calculatedModulePosition + measurementOffset;

        /* check calculated array position */
        switch (measurementType) {
            case MXM_MEASURE_TEMP:
                FAS_ASSERT(calculatedArrayPosition < (MXM_MAXIMUM_NR_OF_MODULES * MXM_MAXIMUM_NR_OF_AUX_PER_MODULE));
                break;
            case MXM_MEASURE_CELL_VOLTAGE:
                FAS_ASSERT(calculatedArrayPosition < (MXM_MAXIMUM_NR_OF_MODULES * MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE));
                break;
            case MXM_MEASURE_BLOCK_VOLTAGE:
                FAS_ASSERT(calculatedArrayPosition < (MXM_MAXIMUM_NR_OF_MODULES));
                break;
            default:
                FAS_ASSERT(FAS_TRAP);
                break;
        }

        FAS_ASSERT(i <= voltRxBufferLength);
        FAS_ASSERT((i + (uint16_t)1u) <= voltRxBufferLength);
        MXM_Convert(
            kpkVoltRxBuffer[i],
            kpkVoltRxBuffer[i + 1u],
            &pVoltagesTarget[calculatedArrayPosition],
            conversionType,
            fullScaleReference_mV);
    }
}

static STD_RETURN_TYPE_e MXM_ParseVoltageReadall(
    const uint8_t *const kpkVoltageRxBuffer,
    uint16_t voltageRxBufferLength,
    MXM_DATA_STORAGE_s *datastorage,
    MXM_CONVERSION_TYPE_e conversionType) {
    STD_RETURN_TYPE_e retval = STD_OK;
    FAS_ASSERT(kpkVoltageRxBuffer != NULL_PTR);
    FAS_ASSERT(datastorage != NULL_PTR);

    if (kpkVoltageRxBuffer[0] != BATTERY_MANAGEMENT_READALL) {
        /* rxBuffer does not contain a READALL command */
        retval = STD_NOT_OK;
    } else if ((voltageRxBufferLength % 2u) != 0u) {
        /* without alive counter rx-buffer always should be of even length */
        /* TODO impact of alive-counter-byte */
        retval = STD_NOT_OK;
    } else if (
        (conversionType != MXM_CONVERSION_UNIPOLAR) && (conversionType != MXM_CONVERSION_BIPOLAR) &&
        (conversionType != MXM_CONVERSION_BLOCK_VOLTAGE)) {
        /* conversion type is not supported */
        retval = STD_NOT_OK;
    } else {
        switch (kpkVoltageRxBuffer[1]) {
            case (uint8_t)MXM_REG_CELL1:
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    MXM_OFFSET_CELL_1,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    MXM_REF_UNIPOLAR_CELL_mV);
                break;
            case (uint8_t)MXM_REG_CELL2:
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    MXM_OFFSET_CELL_2,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    MXM_REF_UNIPOLAR_CELL_mV);
                break;
            case (uint8_t)MXM_REG_CELL3:
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    MXM_OFFSET_CELL_3,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    MXM_REF_UNIPOLAR_CELL_mV);
                break;
            case (uint8_t)MXM_REG_CELL4:
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    MXM_OFFSET_CELL_4,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    MXM_REF_UNIPOLAR_CELL_mV);
                break;
            case (uint8_t)MXM_REG_CELL5:
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    MXM_OFFSET_CELL_5,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    MXM_REF_UNIPOLAR_CELL_mV);
                break;
            case (uint8_t)MXM_REG_CELL6:
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    MXM_OFFSET_CELL_6,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    MXM_REF_UNIPOLAR_CELL_mV);
                break;
            case (uint8_t)MXM_REG_CELL7:
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    MXM_OFFSET_CELL_7,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    MXM_REF_UNIPOLAR_CELL_mV);
                break;
            case (uint8_t)MXM_REG_CELL8:
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    MXM_OFFSET_CELL_8,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    MXM_REF_UNIPOLAR_CELL_mV);
                break;
            case (uint8_t)MXM_REG_CELL9:
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    MXM_OFFSET_CELL_9,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    MXM_REF_UNIPOLAR_CELL_mV);
                break;
            case (uint8_t)MXM_REG_CELL10:
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    MXM_OFFSET_CELL_10,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    MXM_REF_UNIPOLAR_CELL_mV);
                break;
            case (uint8_t)MXM_REG_CELL11:
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    MXM_OFFSET_CELL_11,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    MXM_REF_UNIPOLAR_CELL_mV);
                break;
            case (uint8_t)MXM_REG_CELL12:
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    MXM_OFFSET_CELL_12,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    MXM_REF_UNIPOLAR_CELL_mV);
                break;
            case (uint8_t)MXM_REG_CELL13:
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    MXM_OFFSET_CELL_13,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    MXM_REF_UNIPOLAR_CELL_mV);
                break;
            case (uint8_t)MXM_REG_CELL14:
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    MXM_OFFSET_CELL_14,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    MXM_REF_UNIPOLAR_CELL_mV);
                break;
            case (uint8_t)MXM_REG_AUX0:
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    MXM_OFFSET_AUX0,
                    conversionType,
                    datastorage->auxVoltages_mV,
                    MXM_MEASURE_TEMP,
                    MXM_REF_VAA_mV);
                break;
            case (uint8_t)MXM_REG_AUX2:
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    MXM_OFFSET_AUX2,
                    conversionType,
                    datastorage->auxVoltages_mV,
                    MXM_MEASURE_TEMP,
                    MXM_REF_VAA_mV);
                break;
            case (uint8_t)MXM_REG_AUX3:
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    MXM_OFFSET_AUX3,
                    conversionType,
                    datastorage->auxVoltages_mV,
                    MXM_MEASURE_TEMP,
                    MXM_REF_VAA_mV);
                break;
            case (uint8_t)MXM_REG_BLOCK:
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    MXM_OFFSET_BLOCK,
                    conversionType,
                    datastorage->blockVoltages,
                    MXM_MEASURE_BLOCK_VOLTAGE,
                    65000u);
                /* TODO scaling and variable size  (65000)*/
                break;
            default:
                /* the read register is not a valid cell register */
                retval = STD_NOT_OK;
                break;
        }
    }
    return retval;
}

static STD_RETURN_TYPE_e must_check_return MXM_ParseVoltageReadallTest(MXM_MONITORING_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    STD_RETURN_TYPE_e retval = STD_OK;

    /* AXIVION Disable Style Generic-NoMagicNumbers: This test function uses magic numbers to test predefined values. */

    uint8_t testBuffer[MXM_TEST_BUFFER_MAXIMUM_LENGTH] = {0};
    uint16_t testBufferLength                          = MXM_TEST_BUFFER_MAXIMUM_LENGTH;
    /* init test buffer */
    for (uint8_t i = 0u; i < MXM_TEST_BUFFER_MAXIMUM_LENGTH; i++) {
        testBuffer[i] = 0u;
    }

    /* not a readall buffer */
    testBuffer[0] = BATTERY_MANAGEMENT_HELLOALL;
    if (MXM_ParseVoltageReadall(testBuffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_NOT_OK) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
        retval = STD_NOT_OK;
    }

    /* not a cell voltage register */
    testBuffer[0] = BATTERY_MANAGEMENT_READALL;
    testBuffer[1] = (uint8_t)MXM_REG_VERSION;
    if (MXM_ParseVoltageReadall(testBuffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_NOT_OK) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
        retval = STD_NOT_OK;
    }

    /* bogus conversion type */
    testBuffer[0] = BATTERY_MANAGEMENT_READALL;
    testBuffer[1] = (uint8_t)MXM_REG_CELL1;
    /* AXIVION Next Line Style MisraC2012Directive-4.1 MisraC2012-10.5: explicitely invalid value in order to provoke error response */
    if (MXM_ParseVoltageReadall(testBuffer, testBufferLength, &pInstance->localVoltages, (MXM_CONVERSION_TYPE_e)42) !=
        STD_NOT_OK) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
        retval = STD_NOT_OK;
    }

    /* not an even length of rxBuffer */
    testBuffer[0]    = BATTERY_MANAGEMENT_READALL;
    testBuffer[1]    = (uint8_t)MXM_REG_CELL1;
    testBufferLength = 5;
    if (MXM_ParseVoltageReadall(testBuffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_NOT_OK) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
        retval = STD_NOT_OK;
    }

    /* test data for CELL1REG */
    testBuffer[0]    = BATTERY_MANAGEMENT_READALL;
    testBuffer[1]    = (uint8_t)MXM_REG_CELL1;
    testBuffer[2]    = 0xFCu;
    testBuffer[3]    = 0xFFu;
    testBuffer[4]    = 0x00u;
    testBuffer[5]    = 0x00u;
    testBuffer[6]    = 0xFCu;
    testBuffer[7]    = 0xFFu;
    testBuffer[8]    = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    testBuffer[9]    = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 10u;
    if (MXM_ParseVoltageReadall(testBuffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
        retval = STD_NOT_OK;
    } else {
        if ((pInstance->localVoltages.cellVoltages_mV[0] != MXM_REF_UNIPOLAR_CELL_mV) ||
            (pInstance->localVoltages.cellVoltages_mV[14] != 0u) ||
            (pInstance->localVoltages.cellVoltages_mV[28] != MXM_REF_UNIPOLAR_CELL_mV)) {
            /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_0 = 0; i_0 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_0++) {
        pInstance->localVoltages.cellVoltages_mV[i_0] = 0;
    }

    /* test data for CELL2REG */
    testBuffer[0]    = BATTERY_MANAGEMENT_READALL;
    testBuffer[1]    = (uint8_t)MXM_REG_CELL2;
    testBuffer[2]    = 0xFCu;
    testBuffer[3]    = 0xFFu;
    testBuffer[4]    = 0x00u;
    testBuffer[5]    = 0x00u;
    testBuffer[6]    = 0xFCu;
    testBuffer[7]    = 0xFFu;
    testBuffer[8]    = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    testBuffer[9]    = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 10;
    if (MXM_ParseVoltageReadall(testBuffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
        retval = STD_NOT_OK;
    } else {
        if ((pInstance->localVoltages.cellVoltages_mV[1] != MXM_REF_UNIPOLAR_CELL_mV) ||
            (pInstance->localVoltages.cellVoltages_mV[15] != 0u) ||
            (pInstance->localVoltages.cellVoltages_mV[29] != MXM_REF_UNIPOLAR_CELL_mV)) {
            /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_1 = 0; i_1 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_1++) {
        pInstance->localVoltages.cellVoltages_mV[i_1] = 0;
    }

    /* test data for CELL3REG */
    testBuffer[0]    = BATTERY_MANAGEMENT_READALL;
    testBuffer[1]    = (uint8_t)MXM_REG_CELL3;
    testBuffer[2]    = 0xFCu;
    testBuffer[3]    = 0xFFu;
    testBuffer[4]    = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    testBuffer[5]    = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(testBuffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[2] != MXM_REF_UNIPOLAR_CELL_mV) {
            /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_2 = 0; i_2 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_2++) {
        pInstance->localVoltages.cellVoltages_mV[i_2] = 0;
    }

    /* test data for CELL4REG */
    testBuffer[0]    = BATTERY_MANAGEMENT_READALL;
    testBuffer[1]    = (uint8_t)MXM_REG_CELL4;
    testBuffer[2]    = 0xFCu;
    testBuffer[3]    = 0xFFu;
    testBuffer[4]    = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    testBuffer[5]    = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(testBuffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[3] != MXM_REF_UNIPOLAR_CELL_mV) {
            /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_3 = 0; i_3 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_3++) {
        pInstance->localVoltages.cellVoltages_mV[i_3] = 0;
    }

    /* test data for CELL5REG */
    testBuffer[0]    = BATTERY_MANAGEMENT_READALL;
    testBuffer[1]    = (uint8_t)MXM_REG_CELL5;
    testBuffer[2]    = 0xFCu;
    testBuffer[3]    = 0xFFu;
    testBuffer[4]    = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    testBuffer[5]    = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(testBuffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[4] != MXM_REF_UNIPOLAR_CELL_mV) {
            /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_4 = 0; i_4 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_4++) {
        pInstance->localVoltages.cellVoltages_mV[i_4] = 0;
    }

    /* test data for CELL6REG */
    testBuffer[0]    = BATTERY_MANAGEMENT_READALL;
    testBuffer[1]    = (uint8_t)MXM_REG_CELL6;
    testBuffer[2]    = 0xFCu;
    testBuffer[3]    = 0xFFu;
    testBuffer[4]    = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    testBuffer[5]    = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(testBuffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[5] != MXM_REF_UNIPOLAR_CELL_mV) {
            /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_5 = 0; i_5 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_5++) {
        pInstance->localVoltages.cellVoltages_mV[i_5] = 0;
    }

    /* test data for CELL7REG */
    testBuffer[0]    = BATTERY_MANAGEMENT_READALL;
    testBuffer[1]    = (uint8_t)MXM_REG_CELL7;
    testBuffer[2]    = 0xFCu;
    testBuffer[3]    = 0xFFu;
    testBuffer[4]    = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    testBuffer[5]    = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(testBuffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[6] != MXM_REF_UNIPOLAR_CELL_mV) {
            /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_6 = 0; i_6 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_6++) {
        pInstance->localVoltages.cellVoltages_mV[i_6] = 0;
    }

    /* test data for CELL8REG */
    testBuffer[0]    = BATTERY_MANAGEMENT_READALL;
    testBuffer[1]    = (uint8_t)MXM_REG_CELL8;
    testBuffer[2]    = 0xFCu;
    testBuffer[3]    = 0xFFu;
    testBuffer[4]    = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    testBuffer[5]    = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(testBuffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[7] != MXM_REF_UNIPOLAR_CELL_mV) {
            /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_7 = 0; i_7 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_7++) {
        pInstance->localVoltages.cellVoltages_mV[i_7] = 0;
    }

    /* test data for CELL9REG */
    testBuffer[0]    = BATTERY_MANAGEMENT_READALL;
    testBuffer[1]    = (uint8_t)MXM_REG_CELL9;
    testBuffer[2]    = 0xFCu;
    testBuffer[3]    = 0xFFu;
    testBuffer[4]    = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    testBuffer[5]    = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(testBuffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[8] != MXM_REF_UNIPOLAR_CELL_mV) {
            /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_8 = 0; i_8 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_8++) {
        pInstance->localVoltages.cellVoltages_mV[i_8] = 0;
    }

    /* test data for CELL10REG */
    testBuffer[0]    = BATTERY_MANAGEMENT_READALL;
    testBuffer[1]    = (uint8_t)MXM_REG_CELL10;
    testBuffer[2]    = 0xFCu;
    testBuffer[3]    = 0xFFu;
    testBuffer[4]    = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    testBuffer[5]    = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(testBuffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[9] != MXM_REF_UNIPOLAR_CELL_mV) {
            /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_9 = 0; i_9 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_9++) {
        pInstance->localVoltages.cellVoltages_mV[i_9] = 0;
    }

    /* test data for CELL11REG */
    testBuffer[0]    = BATTERY_MANAGEMENT_READALL;
    testBuffer[1]    = (uint8_t)MXM_REG_CELL11;
    testBuffer[2]    = 0xFCu;
    testBuffer[3]    = 0xFFu;
    testBuffer[4]    = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    testBuffer[5]    = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(testBuffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[10] != MXM_REF_UNIPOLAR_CELL_mV) {
            /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_10 = 0; i_10 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_10++) {
        pInstance->localVoltages.cellVoltages_mV[i_10] = 0;
    }

    /* test data for CELL12REG */
    testBuffer[0]    = BATTERY_MANAGEMENT_READALL;
    testBuffer[1]    = (uint8_t)MXM_REG_CELL12;
    testBuffer[2]    = 0xFCu;
    testBuffer[3]    = 0xFFu;
    testBuffer[4]    = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    testBuffer[5]    = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(testBuffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[11] != MXM_REF_UNIPOLAR_CELL_mV) {
            /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_11 = 0; i_11 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_11++) {
        pInstance->localVoltages.cellVoltages_mV[i_11] = 0;
    }

    /* test data for CELL13REG */
    testBuffer[0]    = BATTERY_MANAGEMENT_READALL;
    testBuffer[1]    = (uint8_t)MXM_REG_CELL13;
    testBuffer[2]    = 0xFCu;
    testBuffer[3]    = 0xFFu;
    testBuffer[4]    = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    testBuffer[5]    = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(testBuffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[12] != MXM_REF_UNIPOLAR_CELL_mV) {
            /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_12 = 0; i_12 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_12++) {
        pInstance->localVoltages.cellVoltages_mV[i_12] = 0;
    }

    /* test data for CELL14REG */
    testBuffer[0]    = BATTERY_MANAGEMENT_READALL;
    testBuffer[1]    = (uint8_t)MXM_REG_CELL14;
    testBuffer[2]    = 0xFCu;
    testBuffer[3]    = 0xFFu;
    testBuffer[4]    = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    testBuffer[5]    = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(testBuffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: return value collects all negative results. */
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[13] != MXM_REF_UNIPOLAR_CELL_mV) {
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_13 = 0; i_13 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_13++) {
        pInstance->localVoltages.cellVoltages_mV[i_13] = 0;
    }

    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    return retval;
}

static STD_RETURN_TYPE_e must_check_return MXM_PreInitSelfCheck(MXM_MONITORING_INSTANCE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);
    STD_RETURN_TYPE_e retval                   = STD_OK;
    pState->selfCheck.crc                      = MXM_CRC8SelfTest();
    pState->selfCheck.conv                     = MXM_ConvertTest();
    pState->selfCheck.firstSetBit              = MXM_FirstSetBitTest();
    pState->selfCheck.extractValueFromRegister = MXM_ExtractValueFromRegisterTest();
    pState->selfCheck.parseVoltageReadall      = MXM_ParseVoltageReadallTest(pState);
    pState->selfCheck.addressSpaceChecker      = MXM_5XUserAccessibleAddressSpaceCheckerSelfCheck();

    if ((pState->selfCheck.crc == STD_OK) && (pState->selfCheck.addressSpaceChecker == STD_OK) &&
        (pState->selfCheck.conv == STD_OK) && (pState->selfCheck.firstSetBit == STD_OK) &&
        (pState->selfCheck.extractValueFromRegister == STD_OK) && (pState->selfCheck.parseVoltageReadall == STD_OK)) {
        /* check has passed, return value already set */
    } else {
        retval = STD_NOT_OK;
    }

    return retval;
}

/*========== Extern Function Implementations ================================*/
extern void MXM_CheckIfErrorCounterCanBeReset(MXM_MONITORING_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);

    if (pInstance->errorCounter != 0u) {
        const bool counterCanBeReset =
            OS_CheckTimeHasPassed(pInstance->timestampLastError, MXM_TIMEOUT_RESET_ERROR_COUNTER_ms);
        if (counterCanBeReset == true) {
            pInstance->errorCounter       = 0u;
            pInstance->timestampLastError = 0u;
        }
    }
}

extern void MXM_ErrorHandlerReset(MXM_MONITORING_INSTANCE_s *pInstance, bool immediateReset) {
    FAS_ASSERT(pInstance != NULL_PTR);

    pInstance->timestampLastError = OS_GetTickCount();
    pInstance->errorCounter++;

    if ((pInstance->errorCounter > MXM_MAXIMUM_ERROR_COUNT) || (immediateReset == true)) {
        pInstance->resetNecessary = true;
    }
}

extern void MXM_HandleStateWriteall(
    MXM_MONITORING_INSTANCE_s *pInstance,
    MXM_STATEMACHINE_OPERATION_STATES_e nextState) {
    FAS_ASSERT(pInstance != NULL_PTR);

    if (pInstance->requestStatus5x == MXM_5X_STATE_UNSENT) {
        const STD_RETURN_TYPE_e setStateRequestReturn = MXM_5XSetStateRequest(
            pInstance->pInstance5X,
            MXM_STATEMACH_5X_WRITEALL,
            pInstance->batteryCmdBuffer,
            &pInstance->requestStatus5x);
        FAS_ASSERT(setStateRequestReturn == STD_OK);
    } else if (pInstance->requestStatus5x == MXM_5X_STATE_UNPROCESSED) {
        /* wait for processing */
    } else if (pInstance->requestStatus5x == MXM_5X_STATE_PROCESSED) {
        /* continue to the substate specified by the request */
        pInstance->operationSubstate = nextState;
        pInstance->requestStatus5x   = MXM_5X_STATE_UNSENT;
    } else if (pInstance->requestStatus5x == MXM_5X_STATE_ERROR) {
        /* default-behavior: retry */
        pInstance->requestStatus5x = MXM_5X_STATE_UNSENT;
        MXM_ErrorHandlerReset(pInstance, false);
    } else {
        /* invalid value, trap */
        FAS_ASSERT(FAS_TRAP);
    }
}

extern bool must_check_return MXM_HandleStateReadall(
    MXM_MONITORING_INSTANCE_s *pInstance,
    MXM_REG_NAME_e registerName,
    MXM_STATEMACHINE_OPERATION_STATES_e nextState) {
    FAS_ASSERT(pInstance != NULL_PTR);

    bool retval = false;
    if (pInstance->requestStatus5x == MXM_5X_STATE_UNSENT) {
        pInstance->batteryCmdBuffer.regAddress        = registerName;
        const STD_RETURN_TYPE_e setStateRequestReturn = MXM_5XSetStateRequest(
            pInstance->pInstance5X, MXM_STATEMACH_5X_READALL, pInstance->batteryCmdBuffer, &pInstance->requestStatus5x);
        FAS_ASSERT(setStateRequestReturn == STD_OK);
    } else if (pInstance->requestStatus5x == MXM_5X_STATE_PROCESSED) {
        MXM_GetDataFrom5XStateMachine(pInstance);
        pInstance->operationSubstate = nextState;
        pInstance->requestStatus5x   = MXM_5X_STATE_UNSENT;
        retval                       = true;
    } else if (pInstance->requestStatus5x == MXM_5X_STATE_UNPROCESSED) {
        /* wait for processing */
    } else if (pInstance->requestStatus5x == MXM_5X_STATE_ERROR) {
        /* try to reset state */
        pInstance->requestStatus5x = MXM_5X_STATE_UNSENT;
        MXM_ErrorHandlerReset(pInstance, false);
    } else {
        /* invalid state */
        FAS_ASSERT(FAS_TRAP);
    }

    return retval;
}

extern STD_RETURN_TYPE_e MXM_ProcessOpenWire(const MXM_MONITORING_INSTANCE_s *const kpkInstance) {
    FAS_ASSERT(kpkInstance != NULL_PTR);
    FAS_ASSERT(kpkInstance->pOpenwire_table != NULL_PTR);

    const uint8_t numberOfSatellites = MXM_5XGetNumberOfSatellites(kpkInstance->pInstance5X);
    static_assert(
        (((uint16_t)BATTERY_MANAGEMENT_TX_LENGTH_READALL + (2u * MXM_MAXIMUM_NR_OF_MODULES)) <= (uint8_t)UINT8_MAX),
        "please check assumptions: code cannot handle number of modules");
    const uint8_t messageLength = BATTERY_MANAGEMENT_TX_LENGTH_READALL + (2u * numberOfSatellites);
    /* step over every byte-tuple in the RX-buffer */
    for (uint8_t i = 2u; i < (messageLength - 2u); i = i + 2u) {
        /* calculate current module number (first in RX-buffer is last in line) */
        const uint8_t calculatedModuleNumberInDaisyChain = numberOfSatellites - ((i / 2u) - 1u) - 1u;
        uint8_t stringNumber                             = 0u;
        uint16_t moduleNumber                            = 0u;
        MXM_ConvertModuleToString(calculatedModuleNumberInDaisyChain, &stringNumber, &moduleNumber);
        const uint16_t calculatedModulePosition = moduleNumber * MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE;

        /* step over every cell in the module and update the openwire struct accordingly */
        for (uint8_t c = 0u; c < MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE; c++) {
            if (c < MXM_CELLS_IN_LSB) {
                /* cell numbers under 8 can be found in the LSB */
                const uint8_t mask = 1u << c;
                if ((uint8_t)(mask & kpkInstance->rxBuffer[i]) > 0u) {
                    kpkInstance->pOpenwire_table->openwire[stringNumber][calculatedModulePosition + c] = 1;
                } else {
                    kpkInstance->pOpenwire_table->openwire[stringNumber][calculatedModulePosition + c] = 0;
                }
            } else {
                /* cell numbers over or equal 8 can be found in the MSB */
                const uint8_t mask = 1u << (c - MXM_CELLS_IN_LSB);
                if ((uint8_t)(mask & kpkInstance->rxBuffer[i + 1u]) > 0u) {
                    kpkInstance->pOpenwire_table->openwire[stringNumber][calculatedModulePosition + c] = 1;
                } else {
                    kpkInstance->pOpenwire_table->openwire[stringNumber][calculatedModulePosition + c] = 0;
                }
            }
        }
    }

    /* write database block */
    const STD_RETURN_TYPE_e dataReturn = DATA_WRITE_DATA(kpkInstance->pOpenwire_table);
    return dataReturn;
}

extern STD_RETURN_TYPE_e MXM_ConstructBalancingBuffer(MXM_BALANCING_STATE_s *pBalancingInstance) {
    FAS_ASSERT(pBalancingInstance != NULL_PTR);
    FAS_ASSERT(pBalancingInstance->pBalancingControl_table != NULL_PTR);
    STD_RETURN_TYPE_e retval = STD_OK;

    /* Re-Initialize the cells to balance at each iteration */
    pBalancingInstance->cellsToBalance = 0u;

    /*  /!\ WARNING
        In this function, EVEN cells refer to the even cells IN THE REAL MODULE, and NOT in the database index.
        --> EVEN cells in a module = ODD database index
        --> ODD cells in a module = EVEN database index
        e.g. :  Cell 2 in the module corresponds to index 1 in the database
                the cell index is even, but the database index is odd. */

    static_assert((MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE <= BS_NR_OF_CELLS_PER_MODULE), "impossible configuration");
    /* Iterate over all the cells of the module 'moduleBalancingIndex' in a daisy-chain */
    if (pBalancingInstance->moduleBalancingIndex < MXM_MAXIMUM_NR_OF_MODULES) {
        for (uint8_t c = 0; c < BS_NR_OF_CELLS_PER_MODULE; c++) {
            /* Determine the position of the cell 'c' of module 'moduleBalancingIndex' in the DB */
            uint8_t stringNumber  = 0u;
            uint16_t moduleNumber = 0u;
            MXM_ConvertModuleToString(pBalancingInstance->moduleBalancingIndex, &stringNumber, &moduleNumber);
            const uint16_t dBIndex = (moduleNumber * BS_NR_OF_CELLS_PER_MODULE) + c;
            if (pBalancingInstance->pBalancingControl_table->balancingState[stringNumber][dBIndex] == 1u) {
                /* Cell 'c' of module '::moduleBalancingIndex' needs to be balanced.
                       Need to determine the balancing order --> even or odd cells?
                       If the balancing order has not been determined before, need to do it. */
                if ((pBalancingInstance->evenCellsNeedBalancing == false) &&
                    (pBalancingInstance->oddCellsNeedBalancing == false)) {
                    if (((c % 2u) != 0u) && (pBalancingInstance->evenCellsBalancingProcessed == false)) {
                        pBalancingInstance->evenCellsNeedBalancing = true;
                    } else if (((c % 2u) == 0u) && (pBalancingInstance->oddCellsBalancingProcessed == false)) {
                        pBalancingInstance->oddCellsNeedBalancing = true;
                    } else {
                        /* nothing to do */
                    }
                }
                if ((pBalancingInstance->evenCellsNeedBalancing == true) && ((c % 2u) != 0u) &&
                    /* If the even cells need to be balanced */
                    (pBalancingInstance->evenCellsBalancingProcessed == false)) {
                    pBalancingInstance->cellsToBalance |= ((uint16_t)0x01u << c);
                } else if (
                    /* If the odd cells need to be balanced */
                    (pBalancingInstance->oddCellsNeedBalancing == true) && ((c % 2u) == 0u) &&
                    (pBalancingInstance->oddCellsBalancingProcessed == false)) {
                    pBalancingInstance->cellsToBalance |= ((uint16_t)0x01u << c);
                } else {
                    /* nothing to do */
                }
            }
        }
    } else {
        retval = STD_NOT_OK;
    }
    return retval;
}

extern STD_RETURN_TYPE_e MXM_ParseVoltagesIntoDB(const MXM_MONITORING_INSTANCE_s *const kpkInstance) {
    FAS_ASSERT(kpkInstance != NULL_PTR);
    FAS_ASSERT(kpkInstance->pCellVoltages_table != NULL_PTR);
    FAS_ASSERT(kpkInstance->pCellTemperatures_table != NULL_PTR);
    FAS_ASSERT((BS_NR_OF_MODULES * BS_NR_OF_STRINGS) <= MXM_MAXIMUM_NR_OF_MODULES);

    uint16_t numberValidVoltageMeasurements[BS_NR_OF_STRINGS] = {0};
    /* voltages */
    for (uint8_t i_mod = 0; i_mod < (BS_NR_OF_MODULES * BS_NR_OF_STRINGS); i_mod++) {
        const bool moduleIsConnected = MXM_CheckIfADeviceIsConnected(kpkInstance, i_mod);
        uint8_t stringNumber         = 0u;
        uint16_t moduleNumber        = 0u;
        MXM_ConvertModuleToString(i_mod, &stringNumber, &moduleNumber);
        FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
        FAS_ASSERT(moduleNumber < BS_NR_OF_MODULES);
        kpkInstance->pCellVoltages_table->moduleVoltage_mV[stringNumber][moduleNumber] =
            kpkInstance->localVoltages.blockVoltages[i_mod];
        /* every iteration that we hit a string first (module 0), we reset the packvoltage counter */
        if (moduleNumber == 0u) {
            kpkInstance->pCellVoltages_table->packVoltage_mV[stringNumber] = 0;
        }
        kpkInstance->pCellVoltages_table->packVoltage_mV[stringNumber] +=
            (int32_t)kpkInstance->localVoltages.blockVoltages[i_mod];
        FAS_ASSERT(BS_NR_OF_CELLS_PER_MODULE <= MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE); /*!< invalid configuration! */
        for (uint8_t i_bat = 0; i_bat < BS_NR_OF_CELLS_PER_MODULE; i_bat++) {
            uint16_t cell_counter_db           = (moduleNumber * BS_NR_OF_CELLS_PER_MODULE) + i_bat;
            uint16_t cell_counter_max          = ((uint16_t)i_mod * MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE) + i_bat;
            const uint16_t cellVoltageLocal_mV = kpkInstance->localVoltages.cellVoltages_mV[cell_counter_max];
            if ((int32_t)cellVoltageLocal_mV > INT16_MAX) {
                kpkInstance->pCellVoltages_table->cellVoltage_mV[stringNumber][cell_counter_db] = INT16_MAX;
            } else {
                kpkInstance->pCellVoltages_table->cellVoltage_mV[stringNumber][cell_counter_db] =
                    (int16_t)cellVoltageLocal_mV;
            }
            const STD_RETURN_TYPE_e valueIsPlausible = AFE_PlausibilityCheckVoltageMeasurementRange(
                kpkInstance->pCellVoltages_table->cellVoltage_mV[stringNumber][cell_counter_db],
                mxm_plausibleCellVoltages);
            if ((valueIsPlausible == STD_OK) && moduleIsConnected) {
                numberValidVoltageMeasurements[stringNumber]++;
            } else {
                /* Invalidate cell voltage measurement */
                kpkInstance->pCellVoltages_table->invalidCellVoltage[stringNumber][moduleNumber] |=
                    ((uint64_t)1u << i_bat);
            }
        }
    }

    /* temperatures */
    uint16_t numberValidTemperatureMeasurements[BS_NR_OF_STRINGS] = {0};
    for (uint8_t i_mod = 0; i_mod < (BS_NR_OF_MODULES * BS_NR_OF_STRINGS); i_mod++) {
        const bool moduleIsConnected = MXM_CheckIfADeviceIsConnected(kpkInstance, i_mod);
        uint8_t stringNumber         = 0u;
        uint16_t moduleNumber        = 0u;
        MXM_ConvertModuleToString(i_mod, &stringNumber, &moduleNumber);
        FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
        FAS_ASSERT(moduleNumber < BS_NR_OF_MODULES);

        /* store aux measurement from AUX2 (MUX0) */
        if (kpkInstance->muxCounter < BS_NR_OF_TEMP_SENSORS_PER_MODULE) {
            const uint16_t temperatureIndexDb = (moduleNumber * BS_NR_OF_TEMP_SENSORS_PER_MODULE) +
                                                kpkInstance->muxCounter;
            const uint16_t temperatureIndexMxm = ((uint16_t)i_mod * MXM_MAXIMUM_NR_OF_AUX_PER_MODULE) + 2u;
            const uint16_t auxVoltage_mV       = kpkInstance->localVoltages.auxVoltages_mV[temperatureIndexMxm];
            /* const uint16_t temporaryVoltage    = (auxVoltage_mV / ((float)3300 - auxVoltage_mV)) * 1000; */
            const int16_t temperature_ddegC = TSI_GetTemperature(auxVoltage_mV);
            kpkInstance->pCellTemperatures_table->cellTemperature_ddegC[stringNumber][temperatureIndexDb] =
                temperature_ddegC;
            const STD_RETURN_TYPE_e valueIsPlausible = AFE_PlausibilityCheckTempMinMax(temperature_ddegC);
            if ((valueIsPlausible == STD_OK) && moduleIsConnected) {
                numberValidTemperatureMeasurements[stringNumber]++;
            } else {
                /* Invalidate temperature measurement */
                kpkInstance->pCellTemperatures_table->invalidCellTemperature[stringNumber][moduleNumber] |=
                    ((uint16_t)1u << kpkInstance->muxCounter);
            }
        }
    }

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        kpkInstance->pCellVoltages_table->nrValidCellVoltages[s]     = numberValidVoltageMeasurements[s];
        kpkInstance->pCellTemperatures_table->nrValidTemperatures[s] = numberValidTemperatureMeasurements[s];
    }

    STD_RETURN_TYPE_e retval = DATA_WRITE_DATA(kpkInstance->pCellVoltages_table, kpkInstance->pCellTemperatures_table);

    return retval;
}

extern void MXM_InitializeStateStruct(
    MXM_BALANCING_STATE_s *pBalancingInstance,
    MXM_MONITORING_INSTANCE_s *pMonitoringInstance) {
    FAS_ASSERT(pBalancingInstance != NULL_PTR);
    FAS_ASSERT(pMonitoringInstance != NULL_PTR);

    pBalancingInstance->moduleBalancingIndex        = 0u;
    pBalancingInstance->evenCellsNeedBalancing      = false;
    pBalancingInstance->oddCellsNeedBalancing       = false;
    pBalancingInstance->evenCellsBalancingProcessed = false;
    pBalancingInstance->oddCellsBalancingProcessed  = false;
    pBalancingInstance->cellsToBalance              = 0u;
    pBalancingInstance->previousTimeStamp           = 0u;
    pBalancingInstance->currentTimeStamp            = 0u;

    pMonitoringInstance->resetNecessary                     = false;
    pMonitoringInstance->errorCounter                       = 0u;
    pMonitoringInstance->timestampLastError                 = 0u;
    pMonitoringInstance->state                              = MXM_STATEMACHINE_STATES_UNINITIALIZED;
    pMonitoringInstance->operationSubstate                  = MXM_INIT_ENTRY;
    pMonitoringInstance->allowStartup                       = false;
    pMonitoringInstance->operationRequested                 = false;
    pMonitoringInstance->firstMeasurementDone               = false;
    pMonitoringInstance->stopRequested                      = false;
    pMonitoringInstance->openwireRequested                  = false;
    pMonitoringInstance->undervoltageAlert                  = false;
    pMonitoringInstance->muxCounter                         = 0u;
    pMonitoringInstance->diagnosticCounter                  = MXM_THRESHOLD_DIAGNOSTIC_AFTER_CYCLES;
    pMonitoringInstance->dcByte                             = MXM_DC_EMPTY;
    pMonitoringInstance->mxmVoltageCellCounter              = 0u;
    pMonitoringInstance->highest5xDevice                    = 0u;
    pMonitoringInstance->requestStatus5x                    = MXM_5X_STATE_UNSENT;
    pMonitoringInstance->batteryCmdBuffer.regAddress        = MXM_REG_VERSION;
    pMonitoringInstance->batteryCmdBuffer.lsb               = 0u;
    pMonitoringInstance->batteryCmdBuffer.msb               = 0u;
    pMonitoringInstance->batteryCmdBuffer.blocksize         = 0u;
    pMonitoringInstance->batteryCmdBuffer.deviceAddress     = 0u;
    pMonitoringInstance->batteryCmdBuffer.model             = MXM_GetModelIdOfDaisyChain();
    pMonitoringInstance->resultSelfCheck                    = STD_NOT_OK;
    pMonitoringInstance->selfCheck.crc                      = STD_NOT_OK;
    pMonitoringInstance->selfCheck.conv                     = STD_NOT_OK;
    pMonitoringInstance->selfCheck.firstSetBit              = STD_NOT_OK;
    pMonitoringInstance->selfCheck.extractValueFromRegister = STD_NOT_OK;
    pMonitoringInstance->selfCheck.parseVoltageReadall      = STD_NOT_OK;
    pMonitoringInstance->selfCheck.addressSpaceChecker      = STD_NOT_OK;
    pMonitoringInstance->selfCheck.fmeaStatusASCI           = STD_NOT_OK;

    for (uint32_t i = 0u; i < (MXM_MAXIMUM_NR_OF_MODULES * MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE); i++) {
        pMonitoringInstance->localVoltages.cellVoltages_mV[i] = 0u;
    }

    for (uint32_t i = 0u; i < (MXM_MAXIMUM_NR_OF_MODULES * MXM_MAXIMUM_NR_OF_AUX_PER_MODULE); i++) {
        pMonitoringInstance->localVoltages.auxVoltages_mV[i] = 0u;
    }

    for (uint32_t i = 0u; i < MXM_MAXIMUM_NR_OF_MODULES; i++) {
        pMonitoringInstance->localVoltages.blockVoltages[i] = 0u;
    }

    for (uint32_t i = 0u; i < MXM_MAXIMUM_NR_OF_MODULES; i++) {
        pMonitoringInstance->registry[i].connected       = false;
        pMonitoringInstance->registry[i].deviceAddress   = 0u;
        pMonitoringInstance->registry[i].model           = MXM_MODEL_ID_NONE;
        pMonitoringInstance->registry[i].siliconVersion  = MXM_siliconVersion_invalid;
        pMonitoringInstance->registry[i].deviceID        = 0u;
        pMonitoringInstance->registry[i].registerStatus1 = 0u;
        pMonitoringInstance->registry[i].registerStatus2 = 0u;
        pMonitoringInstance->registry[i].registerStatus3 = 0u;
        pMonitoringInstance->registry[i].registerFmea1   = 0u;
        pMonitoringInstance->registry[i].registerFmea2   = 0u;
    }

    for (uint32_t i = 0u; i < MXM_RX_BUFFER_LENGTH; i++) {
        pMonitoringInstance->rxBuffer[i] = 0u;
    }
}

extern MXM_MONITORING_STATE_e must_check_return
    MXM_MonGetVoltages(MXM_MONITORING_INSTANCE_s *pState, MXM_REG_NAME_e regAddress) {
    FAS_ASSERT(pState != NULL_PTR);
    MXM_MONITORING_STATE_e retval = MXM_MONITORING_STATE_PENDING;
    if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
        pState->batteryCmdBuffer.regAddress           = regAddress;
        const STD_RETURN_TYPE_e setStateRequestReturn = MXM_5XSetStateRequest(
            pState->pInstance5X, MXM_STATEMACH_5X_READALL, pState->batteryCmdBuffer, &pState->requestStatus5x);
        FAS_ASSERT(setStateRequestReturn == STD_OK);
    } else if (pState->requestStatus5x == MXM_5X_STATE_PROCESSED) {
        MXM_GetDataFrom5XStateMachine(pState);
        uint8_t temp_len = BATTERY_MANAGEMENT_TX_LENGTH_READALL +
                           (2u * MXM_5XGetNumberOfSatellites(pState->pInstance5X));
        /* AXIVION Next Line Style FaultDetection-UnusedAssignments: default value is set as a safety measure */
        STD_RETURN_TYPE_e readallReturn = STD_NOT_OK;
        if (regAddress == MXM_REG_BLOCK) {
            readallReturn = MXM_ParseVoltageReadall(
                pState->rxBuffer, temp_len, &pState->localVoltages, MXM_CONVERSION_BLOCK_VOLTAGE);
        } else {
            readallReturn =
                MXM_ParseVoltageReadall(pState->rxBuffer, temp_len, &pState->localVoltages, MXM_CONVERSION_UNIPOLAR);
        }
        if (readallReturn == STD_NOT_OK) {
            MXM_ErrorHandlerReset(pState, false);
        }
        pState->requestStatus5x = MXM_5X_STATE_UNSENT;
        retval                  = MXM_MONITORING_STATE_PASS;
    } else if (pState->requestStatus5x == MXM_5X_STATE_UNPROCESSED) {
        /* do nothing */
    } else if (pState->requestStatus5x == MXM_5X_STATE_ERROR) {
        /* try to reset state */
        pState->requestStatus5x = MXM_5X_STATE_UNSENT;
        MXM_ErrorHandlerReset(pState, false);
    } else {
        /* invalid state */
        FAS_ASSERT(FAS_TRAP);
    }
    return retval;
}

extern void MXM_StateMachine(MXM_MONITORING_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);

    if (pInstance->state == MXM_STATEMACHINE_STATES_UNINITIALIZED) {
        pInstance->requestStatus5x = MXM_5X_STATE_UNSENT;
        MXM_MonRegistryInit(pInstance);
        pInstance->state = MXM_STATEMACHINE_STATES_SELFCHECK_PRE_INIT;
    }

    switch (pInstance->state) {
        case MXM_STATEMACHINE_STATES_SELFCHECK_PRE_INIT:
            /* TODO proper selfchecks, that are only executed during IBIT, CBIT and PBIT */
            pInstance->resultSelfCheck = MXM_PreInitSelfCheck(pInstance);
            if (pInstance->resultSelfCheck == STD_OK) {
                pInstance->requestStatus5x = MXM_5X_STATE_UNSENT;
                pInstance->state           = MXM_STATEMACHINE_STATES_INIT;
            } else {
                /* software is broken by configuration/design */
                FAS_ASSERT(FAS_TRAP);
            }
            break;
        case MXM_STATEMACHINE_STATES_INIT:
            if (pInstance->requestStatus5x == MXM_5X_STATE_UNSENT) {
                const STD_RETURN_TYPE_e setStateRequestReturn = MXM_5XSetStateRequest(
                    pInstance->pInstance5X,
                    MXM_STATEMACH_5X_INIT,
                    pInstance->batteryCmdBuffer,
                    &pInstance->requestStatus5x);
                FAS_ASSERT(setStateRequestReturn == STD_OK);
            } else if (pInstance->requestStatus5x == MXM_5X_STATE_UNPROCESSED) {
                /* just wait */
            } else if (pInstance->requestStatus5x == MXM_5X_STATE_ERROR) {
                /* do not reset state machine, but count error */
                MXM_ErrorHandlerReset(pInstance, false);
            } else if (pInstance->allowStartup == false) {
                /* do not start yet, just wait */

            } else if ((pInstance->requestStatus5x == MXM_5X_STATE_PROCESSED) && (pInstance->allowStartup == true)) {
                /* update registry */
                STD_RETURN_TYPE_e ConnectResult =
                    MXM_MonRegistryConnectDevices(pInstance, MXM_5XGetNumberOfSatellites(pInstance->pInstance5X));
                FAS_ASSERT(ConnectResult == STD_OK);
                pInstance->requestStatus5x = MXM_5X_STATE_UNSENT;
                pInstance->state           = MXM_STATEMACHINE_STATES_SELFCHECK_POST_INIT;
            } else {
                FAS_ASSERT(FAS_TRAP);
            }

            break;
        case MXM_STATEMACHINE_STATES_SELFCHECK_POST_INIT:
            /* TODO more checks */
            pInstance->selfCheck.fmeaStatusASCI = STD_NOT_OK;
            if (pInstance->requestStatus5x == MXM_5X_STATE_UNSENT) {
                const STD_RETURN_TYPE_e stateRequestReturn = MXM_5XSetStateRequest(
                    pInstance->pInstance5X,
                    MXM_STATEMACH_5X_41B_FMEA_CHECK,
                    pInstance->batteryCmdBuffer,
                    &pInstance->requestStatus5x);
                FAS_ASSERT(stateRequestReturn == STD_OK);
            } else if (pInstance->requestStatus5x == MXM_5X_STATE_UNPROCESSED) {
                /* just wait */
            } else if (pInstance->requestStatus5x == MXM_5X_STATE_ERROR) {
                /* this is a configuration or hardware failure */
                pInstance->selfCheck.fmeaStatusASCI = STD_NOT_OK;
                pInstance->resultSelfCheck          = STD_NOT_OK;
                MXM_ErrorHandlerReset(pInstance, false);
            } else if (pInstance->requestStatus5x == MXM_5X_STATE_PROCESSED) {
                pInstance->selfCheck.fmeaStatusASCI = STD_OK;
                const STD_RETURN_TYPE_e resultNumberOfSatellitesGood =
                    MXM_5XGetNumberOfSatellitesGood(pInstance->pInstance5X);
                if ((pInstance->resultSelfCheck == STD_OK) && (pInstance->selfCheck.fmeaStatusASCI == STD_OK) &&
                    (resultNumberOfSatellitesGood == STD_OK)) {
                    pInstance->resultSelfCheck = STD_OK;
                    pInstance->requestStatus5x = MXM_5X_STATE_UNSENT;
                    pInstance->state           = MXM_STATEMACHINE_STATES_IDLE;
                } else if (mxm_allowSkippingPostInitSelfCheck == true) {
                    pInstance->resultSelfCheck = STD_NOT_OK;
                    pInstance->requestStatus5x = MXM_5X_STATE_UNSENT;
                    pInstance->state           = MXM_STATEMACHINE_STATES_IDLE;
                } else {
                    /* skipping the self-check is not allowed and it failed, try to reset */
                    pInstance->resetNecessary = true;
                }
            } else {
                FAS_ASSERT(FAS_TRAP);
            }
            break;
        case MXM_STATEMACHINE_STATES_IDLE:
            pInstance->stopRequested = false;
            if (pInstance->operationRequested == true) {
                pInstance->state             = MXM_STATEMACHINE_STATES_OPERATION;
                pInstance->operationSubstate = MXM_INIT_ENTRY;
            } else {
                /* Do nothing */
            }
            break;
        case MXM_STATEMACHINE_STATES_OPERATION:
            MXM_StateMachineOperation(pInstance);
            break;
        default:
            FAS_ASSERT(FAS_TRAP);
            break;
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern STD_RETURN_TYPE_e TEST_MXM_ParseVoltageReadallTest(MXM_MONITORING_INSTANCE_s *pInstance) {
    return MXM_ParseVoltageReadallTest(pInstance);
}
extern STD_RETURN_TYPE_e TEST_MXM_ParseVoltagesIntoDB(MXM_MONITORING_INSTANCE_s *pInstance) {
    return MXM_ParseVoltagesIntoDB(pInstance);
}
extern STD_RETURN_TYPE_e TEST_MXM_ParseVoltageReadall(
    uint8_t *voltRxBuffer,
    uint16_t voltRxBufferLength,
    MXM_DATA_STORAGE_s *datastorage,
    MXM_CONVERSION_TYPE_e conversionType) {
    return MXM_ParseVoltageReadall(voltRxBuffer, voltRxBufferLength, datastorage, conversionType);
}
#endif
