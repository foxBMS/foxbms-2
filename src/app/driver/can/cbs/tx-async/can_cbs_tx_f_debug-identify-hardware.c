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
 * @file    can_cbs_tx_f_debug-identify-hardware.c
 * @author  foxBMS Team
 * @date    2023-05-31 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   CAN driver Tx callback implementation
 * @details CAN Tx callback for sending the hardware identification
 */

/*========== Includes =======================================================*/
#include "can_cbs_tx_f_debug-identify-hardware.h"

#include "can.h"
#include "can_cfg_tx-async-message-definitions.h"
#include "can_helper.h"
#include "foxmath.h"

#include <math.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** @{
 * configuration of the multiplexer
 */
#define CANTX_MUX_START_BIT (7u)
#define CANTX_MUX_LENGTH    (1u)
#define CANTX_MUX_MASTER    (0x00u)
#define CANTX_MUX_SLAVE     (0x01u)
/** @} */

/** @{
 * configuration of the slave multiplexer message
 */
#define CANTX_MUX_AFE_NUMBER_START_BIT (6u)
#define CANTX_MUX_AFE_NUMBER_LENGTH    (15u)
#define CANTX_MUX_SERIAL_ID_START_BIT  (23u)
#define CANTX_MUX_SERIAL_ID_LENGTH     (48u)
/** @} */

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/
uint64_t *serialIds = NULL_PTR;

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Sends the application configuration message
 * @param   pMessageData message data of the CAN message
 * @param   muxValue multiplexer value to be set
 */
static void CANTX_DebugIdentifyHardwareMux(uint64_t *pMessageData, uint8_t muxValue);

/**
 * @brief   Sends the master identification message
 */
static STD_RETURN_TYPE_e CANTX_SendMasterIdentification(void);

/**
 * @brief   Sends the slave identification message
 * @param   afeNumber number of afe slave message to send
 */
static STD_RETURN_TYPE_e CANTX_SendSlaveIdentification(uint16_t afeNumber);

/**
 * @brief   Sets the slave serial Id in message
 * @param   pMessageData message data of the CAN message
 * @param   afeNumber afe number value to be set
 */
static void CANTX_SetSlaveSerialIdMessageData(uint64_t *pMessageData, uint16_t afeNumber);

/*========== Static Function Implementations ================================*/
static void CANTX_DebugIdentifyHardwareMux(uint64_t *pMessageData, uint8_t muxValue) {
    FAS_ASSERT(pMessageData != NULL_PTR);
    FAS_ASSERT((muxValue == CANTX_MUX_MASTER) || (muxValue == CANTX_MUX_SLAVE));

    CAN_TxSetMessageDataWithSignalData(
        pMessageData, CANTX_MUX_START_BIT, CANTX_MUX_LENGTH, muxValue, CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

static STD_RETURN_TYPE_e CANTX_SendMasterIdentification(void) {
    uint64_t messageData         = 0u;
    uint8_t canData[CAN_MAX_DLC] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    /* Set multiplexer value */
    CANTX_DebugIdentifyHardwareMux(&messageData, CANTX_MUX_MASTER);

    /* Set the can data */
    CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_IDENTIFY_HARDWARE_ENDIANNESS);

    /* Send the message */
    STD_RETURN_TYPE_e queuedSuccessfully = CAN_DataSend(
        CAN_NODE_DEBUG_MESSAGE, CANTX_DEBUG_IDENTIFY_HARDWARE_ID, CANTX_DEBUG_IDENTIFY_HARDWARE_ID_TYPE, canData);

    return queuedSuccessfully;
}

static STD_RETURN_TYPE_e CANTX_SendSlaveIdentification(uint16_t afeNumber) {
    uint64_t messageData         = 0u;
    uint8_t canData[CAN_MAX_DLC] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    /* Set slave mux message data */
    CANTX_DebugIdentifyHardwareMux(&messageData, CANTX_MUX_SLAVE);

    /* Set afe number message data*/
    CAN_TxSetMessageDataWithSignalData(
        &messageData,
        CANTX_MUX_AFE_NUMBER_START_BIT,
        CANTX_MUX_AFE_NUMBER_LENGTH,
        afeNumber,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
    /* Set serial id message data*/
    CANTX_SetSlaveSerialIdMessageData(&messageData, afeNumber);

    /* Send slave identification message*/
    CAN_TxSetCanDataWithMessageData(messageData, canData, CANTX_DEBUG_IDENTIFY_HARDWARE_ENDIANNESS);
    STD_RETURN_TYPE_e queuedSuccessfully = CAN_DataSend(
        CAN_NODE_DEBUG_MESSAGE, CANTX_DEBUG_IDENTIFY_HARDWARE_ID, CANTX_DEBUG_IDENTIFY_HARDWARE_ID_TYPE, canData);

    return queuedSuccessfully;
}

static void CANTX_SetSlaveSerialIdMessageData(uint64_t *pMessageData, uint16_t afeNumber) {
    FAS_ASSERT(serialIds != NULL_PTR);

    /* Select the serialId element for the message from the pointer to the array */
    uint64_t serialId = serialIds[afeNumber];

    /* Set afe serial id message data*/
    CAN_TxSetMessageDataWithSignalData(
        pMessageData,
        CANTX_MUX_SERIAL_ID_START_BIT,
        CANTX_MUX_SERIAL_ID_LENGTH,
        serialId,
        CANTX_DEBUG_BUILD_CONFIGURATION_ENDIANNESS);
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e CANTX_DebugIdentifyHardware(void) {
    /* Trigger the application configuration message */
    STD_RETURN_TYPE_e queuedSuccessfully = CANTX_SendMasterIdentification();
    if (queuedSuccessfully == STD_OK) {
        /* Trigger the slave identification message for every Afe */
        serialIds = AFE_IdentifyAfes();
        for (uint16_t i = 0; i < (BS_NR_OF_STRINGS * BS_NR_OF_MODULES_PER_STRING); i++) {
            queuedSuccessfully = CANTX_SendSlaveIdentification(i);
        }
    }
    return queuedSuccessfully;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_CANTX_DebugIdentifyHardwareMux(uint64_t *pMessageData, uint8_t muxValue) {
    CANTX_DebugIdentifyHardwareMux(pMessageData, muxValue);
}

extern STD_RETURN_TYPE_e TEST_CANTX_SendMasterIdentification(void) {
    return CANTX_SendMasterIdentification();
}

extern STD_RETURN_TYPE_e TEST_CANTX_SendSlaveIdentification(uint16_t afeNumber) {
    return CANTX_SendSlaveIdentification(afeNumber);
}
#endif
