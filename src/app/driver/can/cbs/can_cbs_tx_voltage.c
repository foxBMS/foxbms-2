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
 * @file    can_cbs_tx_voltage.c
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2021-07-29 (date of last update)
 * @ingroup DRIVER
 * @prefix  CAN
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for cell voltages
 */

/*========== Includes =======================================================*/
#include "can_cbs.h"
#include "can_helper.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/** the number of voltages per message-frame */
#define NUMBER_OF_MUX_VOLTAGES_PER_MESSAGE (4u)

/**
 * CAN signals used in this message
 * Parameters:
 * bit start, bit length, factor, offset, minimum value, maximum value
 */
static const CAN_SIGNAL_TYPE_s cellVoltageMultiplexer  = {7u, 8u, 1.0f, 0.0f, 0.0f, 1.0f};
static const CAN_SIGNAL_TYPE_s cellVoltage0InvalidFlag = {12u, 1u, 1.0f, 0.0f, 0.0f, 1.0f};
static const CAN_SIGNAL_TYPE_s cellVoltage1InvalidFlag = {13u, 1u, 1.0f, 0.0f, 0.0f, 1.0f};
static const CAN_SIGNAL_TYPE_s cellVoltage2InvalidFlag = {14u, 1u, 1.0f, 0.0f, 0.0f, 1.0f};
static const CAN_SIGNAL_TYPE_s cellVoltage3InvalidFlag = {15u, 1u, 1.0f, 0.0f, 0.0f, 1.0f};
static const CAN_SIGNAL_TYPE_s cellVoltage0_mV         = {11u, 13u, 1.0f, 0.0f, 0.0f, 8192.0f};
static const CAN_SIGNAL_TYPE_s cellVoltage1_mV         = {30u, 13u, 1.0f, 0.0f, 0.0f, 8192.0f};
static const CAN_SIGNAL_TYPE_s cellVoltage2_mV         = {33u, 13u, 1.0f, 0.0f, 0.0f, 8192.0f};
static const CAN_SIGNAL_TYPE_s cellVoltage3_mV         = {52u, 13u, 1.0f, 0.0f, 0.0f, 8192.0f};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief Helper function for CAN_TxVoltage()
 *
 * Used in the CAN_TxVoltage() callback to set
 * invalid flag data and voltage data
 * in the CAN frame.
 *
 * @param[in] muxId                          multiplexer value
 * @param[in] pMessage                       pointer to CAN frame data
 * @param[in] cellVoltageSignal              signal characteristics for voltage data
 * @param[in] cellVoltageInvalidFlagSignal   signal characteristics for invalid flag data
 * @param[in] endianness                     big or little endianness of data
 * @param[in] kpkCanShim                     shim to the database entries
 */
static void CAN_TxVoltageSetData(
    uint8_t muxId,
    uint64_t *pMessage,
    CAN_SIGNAL_TYPE_s cellVoltageSignal,
    CAN_SIGNAL_TYPE_s cellVoltageInvalidFlagSignal,
    CAN_ENDIANNESS_e endianness,
    const CAN_SHIM_s *const kpkCanShim);

/*========== Static Function Implementations ================================*/

static void CAN_TxVoltageSetData(
    uint8_t muxId,
    uint64_t *pMessage,
    CAN_SIGNAL_TYPE_s cellVoltageSignal,
    CAN_SIGNAL_TYPE_s cellVoltageInvalidFlagSignal,
    CAN_ENDIANNESS_e endianness,
    const CAN_SHIM_s *const kpkCanShim) {
    /* cell index must not be greater than the number of cells */
    if (muxId < (BS_NR_OF_BAT_CELLS * BS_NR_OF_STRINGS)) {
        /* Get string, module and cell number */
        const uint8_t stringNumber = DATA_GetStringNumberFromVoltageIndex(muxId);
        const uint8_t moduleNumber = DATA_GetModuleNumberFromVoltageIndex(muxId);
        const uint8_t cellNumber   = DATA_GetCellNumberFromVoltageIndex(muxId);

        uint32_t signalData_valid = 0u;
        /* Valid bits data */
        if ((kpkCanShim->pTableCellVoltage->invalidCellVoltage[stringNumber][moduleNumber] & (0x01u << cellNumber)) ==
            0u) {
            signalData_valid = 0u;
        } else {
            signalData_valid = 1u;
        }
        /* Set valid bit data in CAN frame */
        CAN_TxSetMessageDataWithSignalData(
            pMessage,
            cellVoltageInvalidFlagSignal.bitStart,
            cellVoltageInvalidFlagSignal.bitLength,
            signalData_valid,
            endianness);

        /*Voltage data */
        float signalData_mV =
            (float)(kpkCanShim->pTableCellVoltage
                        ->cellVoltage_mV[stringNumber][(moduleNumber * BS_NR_OF_CELLS_PER_MODULE) + cellNumber]);
        /* Apply offset and factor */
        CAN_TxPrepareSignalData(&signalData_mV, cellVoltageSignal);
        /* Set voltage data in CAN frame */
        CAN_TxSetMessageDataWithSignalData(
            pMessage, cellVoltageSignal.bitStart, cellVoltageSignal.bitLength, (uint32_t)signalData_mV, endianness);
    }
}

/*========== Extern Function Implementations ================================*/
extern uint32_t CAN_TxVoltage(
    uint32_t id,
    uint8_t dlc,
    CAN_ENDIANNESS_e endianness,
    uint8_t *pCanData,
    uint8_t *pMuxId,
    const CAN_SHIM_s *const kpkCanShim) {
    FAS_ASSERT(id < CAN_MAX_11BIT_ID); /* Currently standard ID, 11 bit */
    FAS_ASSERT(dlc <= CAN_MAX_DLC);    /* Currently max 8 bytes in a CAN frame */
    FAS_ASSERT(pCanData != NULL_PTR);
    FAS_ASSERT(pMuxId != NULL_PTR);
    FAS_ASSERT(kpkCanShim != NULL_PTR);
    uint64_t message = 0;

    /* Reset mux if maximum was reached */
    if (*pMuxId >= (BS_NR_OF_STRINGS * BS_NR_OF_BAT_CELLS)) {
        *pMuxId = 0u;
    }
    /* First signal to transmit cell voltages: get database values */
    if (*pMuxId == 0u) {
        DATA_READ_DATA(kpkCanShim->pTableCellVoltage);
    }

    /* Set mux signal in CAN frame */
    uint32_t signalData = *pMuxId / NUMBER_OF_MUX_VOLTAGES_PER_MESSAGE;
    CAN_TxSetMessageDataWithSignalData(
        &message, cellVoltageMultiplexer.bitStart, cellVoltageMultiplexer.bitLength, (uint32_t)signalData, endianness);

    /* Set other signals in CAN frame */
    CAN_TxVoltageSetData(*pMuxId, &message, cellVoltage0_mV, cellVoltage0InvalidFlag, endianness, kpkCanShim);
    /* Increment multiplexer for next cell */
    (*pMuxId)++;
    CAN_TxVoltageSetData(*pMuxId, &message, cellVoltage1_mV, cellVoltage1InvalidFlag, endianness, kpkCanShim);
    /* Increment multiplexer for next cell */
    (*pMuxId)++;
    CAN_TxVoltageSetData(*pMuxId, &message, cellVoltage2_mV, cellVoltage2InvalidFlag, endianness, kpkCanShim);
    /* Increment multiplexer for next cell */
    (*pMuxId)++;
    CAN_TxVoltageSetData(*pMuxId, &message, cellVoltage3_mV, cellVoltage3InvalidFlag, endianness, kpkCanShim);
    /* Increment multiplexer for next cell */
    (*pMuxId)++;

    /* All signal data copied in CAN frame, now copy data in the buffer that will be use to send the frame */
    CAN_TxSetCanDataWithMessageData(message, pCanData, endianness);

    return 0;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST

#endif
