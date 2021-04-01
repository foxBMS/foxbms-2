/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der
 *  angewandten Forschung e.V. All rights reserved.
 *
 * BSD 3-Clause License
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1.  Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer
 * to foxBMS in your hardware, software, documentation or advertising
 * materials:
 *
 * &Prime;This product uses parts of foxBMS&reg;&Prime;
 *
 * &Prime;This product includes parts of foxBMS&reg;&Prime;
 *
 * &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    can_cfg.c
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2021-03-24 (date of last update)
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  CAN
 *
 * @brief   Configuration for the CAN module
 *
 * The CAN bus settings and the received messages and their
 * reception handling are to be specified here.
 *
 *
 */

/*========== Includes =======================================================*/
#include "can_cfg.h"

#include "database.h"
#include "diag.h"
#include "foxmath.h"
#include "imd.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   Puts CAN signal data in a 64-bit variable.
 * This function is used to compose a 64-bit CAN message.
 * It takes signal data, signal bit start, signal bit length
 * and puts the data in the 64-bit variable.
 * @param   pMessage   64-bit variable containing the CAN data
 * @param   bitStart   starting bit were the signal data must be put
 * @param   bitLength  length of the signal data to be put in CAN message
 * @param   signal     signal data to be put in message
 * @param   byteOrder  big or little endianess of data
 */
static void CAN_TxSetMessageDataWithSignalData(
    uint64_t *pMessage,
    uint64_t bitStart,
    uint8_t bitLength,
    uint64_t signal,
    CAN_byteOrder_e byteOrder);

/**
 * @brief   Copy CAN data from a 64-bit variable to 8 bytes.
 * This function is used to copy a 64-bit CAN message to 8 bytes.
 * @param   pMessage   64-bit variable containing the CAN data
 * @param   pCanData   8 bytes where the data is copied
 */
static void CAN_TxSetCanDataWithMessageData(uint64_t *pMessage, uint8_t *pCanData);

/**
 * @brief   Gets CAN signal data from a 64-bit variable.
 * This function is used to get signal data from  a 64-bit
 * CAN message.
 * It takes signal bit start, signal bit length and extract
 * signal data from the the 64-bit variable.
 * @param   message    64-bit variable containing the CAN data
 * @param   bitStart   starting bit were the signal data must be put
 * @param   bitLength  length of the signal data to be put in CAN message
 * @param   pSignal    signal data to be retrieved from message
 * @param   byteOrder  big or little endianess of data
 */
static void CAN_RxGetSignalDataFromMessageData(
    uint64_t message,
    uint64_t bitStart,
    uint8_t bitLength,
    uint64_t *pSignal,
    CAN_byteOrder_e byteOrder);

/**
 * @brief   Copy CAN data from 8 bytes to a 64-bit variable.
 * @details This function is used to copy data from a 64-bit variable to 8
 *          bytes.
 * @param   pCanData   8 bytes containing the data
 * @param   pMessage   64-bit where the data is copied
 */
static void CAN_RxGetMessageDataFromCanData(uint8_t *pCanData, uint64_t *pMessage);

/** TX callback functions @{ */
static uint32_t CAN_TxVoltage(uint32_t id, uint8_t dlc, CAN_byteOrder_e byteOrder, uint8_t *canData, uint32_t *pMuxId);
static uint32_t CAN_TxPcbTemperature(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *canData,
    uint32_t *pMuxId);
static uint32_t CAN_TxExternalTemperature(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *canData,
    uint32_t *pMuxId);
static uint32_t CAN_TxVoltageMinMax(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *canData,
    uint32_t *pMuxId);
/** @} */

/** RX callback functions @{ */
static uint32_t CAN_RxImdInfo(uint32_t id, uint8_t dlc, CAN_byteOrder_e byteOrder, uint8_t *canData, uint32_t *pMuxId);
static uint32_t CAN_RxImdResponse(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *canData,
    uint32_t *pMuxId);
static uint32_t CAN_RxRequest(uint32_t id, uint8_t dlc, CAN_byteOrder_e byteOrder, uint8_t *canData, uint32_t *pMuxId);
static uint32_t CAN_RxSwReset(uint32_t id, uint8_t dlc, CAN_byteOrder_e byteOrder, uint8_t *canData, uint32_t *pMuxId);
static uint32_t CAN_RxCurrentSensor(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *canData,
    uint32_t *pMuxId);
static uint32_t CAN_RxDebug(uint32_t id, uint8_t dlc, CAN_byteOrder_e byteOrder, uint8_t *canData, uint32_t *pMuxId);
static uint32_t CAN_RxSwVersion(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *canData,
    uint32_t *pMuxId);
/** @} */

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/* ***************************************
 *  Configure TX messages here
 ****************************************/

/** registry of CAN TX messages */
const CAN_MSG_TX_TYPE_s can_txMessages[] = {
    {0x100, 8, 100, 0, littleEndian, &CAN_TxVoltageMinMax, NULL_PTR},   /*!< Min/Max voltages */
    {0x101, 8, 1000, 0, littleEndian, &CAN_TxPcbTemperature, NULL_PTR}, /*!< PCB temperature and open wire channels */
    {0x102, 8, 100, 0, littleEndian, &CAN_TxExternalTemperature, NULL_PTR}, /*!< External temperature */

    {0x110, 8, 100, 0, littleEndian, &CAN_TxVoltage, NULL_PTR},  /*!< Cell voltages 0-5*/
    {0x111, 8, 100, 0, littleEndian, &CAN_TxVoltage, NULL_PTR},  /*!< Cell voltages 6-11*/
    {0x112, 8, 100, 0, littleEndian, &CAN_TxVoltage, NULL_PTR},  /*!< Cell voltages 12-17*/
    {0x113, 8, 100, 10, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 18-23*/
    {0x114, 8, 100, 10, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 24-29*/
    {0x115, 8, 100, 10, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 30-35*/
    {0x116, 8, 100, 20, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 36-41*/
    {0x117, 8, 100, 20, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 42-47*/
    {0x118, 8, 100, 20, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 48-53*/
    {0x119, 8, 100, 30, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 54-59*/
    {0x11A, 8, 100, 30, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 60-65*/
    {0x11B, 8, 100, 30, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 66-71*/
    {0x11C, 8, 100, 40, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 72-77*/
    {0x11D, 8, 100, 40, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 78-83*/
    {0x11E, 8, 100, 40, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 84-89*/
    {0x11F, 8, 100, 50, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 90-95*/
    {0x120, 8, 100, 50, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 96-101*/
    {0x121, 8, 100, 50, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 102-107*/
    {0x122, 8, 100, 60, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 108-113*/
    {0x123, 8, 100, 60, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 114-119*/
    {0x124, 8, 100, 60, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 120-125*/
    {0x125, 8, 100, 70, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 126-131*/
    {0x126, 8, 100, 70, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 132-137*/
    {0x127, 8, 100, 70, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 138-143*/
    {0x128, 8, 100, 80, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 144-149*/
    {0x129, 8, 100, 80, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 150-155*/
    {0x12A, 8, 100, 80, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 156-161*/
    {0x12B, 8, 100, 90, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 162-167*/
    {0x12C, 8, 100, 90, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 168-173*/
    {0x12D, 8, 100, 90, littleEndian, &CAN_TxVoltage, NULL_PTR}, /*!< Cell voltages 174-179*/
};

/* ***************************************
 *  Configure RX messages here
 ****************************************/

/** registry of CAN RX messages */
const CAN_MSG_RX_TYPE_s can_rxMessages[] = {
    {0x37, 8, 0, littleEndian, &CAN_RxImdInfo},     /*!< request SW version */
    {0x23, 8, 0, littleEndian, &CAN_RxImdResponse}, /*!< request SW version */

    {0x120, 8, 0, littleEndian, &CAN_RxRequest},                     /*!< state request      */
    {CAN_ID_SOFTWARE_RESET_MSG, 8, 0, littleEndian, &CAN_RxSwReset}, /*!< software reset     */

    {0x521u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor I in cyclic mode   */
    {0x522u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor U1 in cyclic mode  */
    {0x523u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor U2 in cyclic mode  */
    {0x524u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor U3 in cyclic mode  */
    {0x525u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor T in cyclic mode  */
    {0x526u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor Power in cyclic mode  */
    {0x527u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor C-C in cyclic mode  */
    {0x528u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor E-C in cyclic mode  */

#if BS_NR_OF_STRINGS > 1u
    {0x621u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor I in cyclic mode   */
    {0x622u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor U1 in cyclic mode  */
    {0x623u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor U2 in cyclic mode  */
    {0x624u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor U3 in cyclic mode  */
    {0x625u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor T in cyclic mode  */
    {0x626u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor Power in cyclic mode  */
    {0x627u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor C-C in cyclic mode  */
    {0x628u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor E-C in cyclic mode  */

#if BS_NR_OF_STRINGS > 2u
    {0x721u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor I in cyclic mode   */
    {0x722u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor U1 in cyclic mode  */
    {0x723u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor U2 in cyclic mode  */
    {0x724u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor U3 in cyclic mode  */
    {0x725u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor T in cyclic mode  */
    {0x726u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor Power in cyclic mode  */
    {0x727u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor C-C in cyclic mode  */
    {0x728u, 8, 0, bigEndian, &CAN_RxCurrentSensor}, /*!< current sensor E-C in cyclic mode  */

#endif /* BS_NR_OF_STRINGS > 1 */
#endif /* BS_NR_OF_STRINGS > 2 */

    {0x100, 8, 0, littleEndian, &CAN_RxDebug},     /*!< debug message      */
    {0x777, 8, 0, littleEndian, &CAN_RxSwVersion}, /*!< request SW version */
};

/** length of CAN message arrays @{*/
const uint8_t can_txLength = sizeof(can_txMessages) / sizeof(can_txMessages[0]);
const uint8_t can_rxLength = sizeof(can_rxMessages) / sizeof(can_rxMessages[0]);
/**@}*/

/** local copies of database tables */
/**@{*/
static DATA_BLOCK_CELL_VOLTAGE_s can_tableCellVoltages       = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE};
static DATA_BLOCK_ADC_TEMPERATURE_s can_tableAdcTemperatures = {.header.uniqueId = DATA_BLOCK_ID_ADC_TEMPERATURE};
static DATA_BLOCK_MIN_MAX_s can_tableMinimumMaximumValues    = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_CURRENT_SENSOR_s can_tableCurrentSensor    = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR};
static DATA_BLOCK_OPEN_WIRE_s can_tableOpenWire              = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
static DATA_BLOCK_STATEREQUEST_s can_tableStateRequest       = {.header.uniqueId = DATA_BLOCK_ID_STATEREQUEST};
/**@}*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/
#pragma diag_push
#pragma diag_suppress 880
static uint32_t CAN_TxVoltage(uint32_t id, uint8_t dlc, CAN_byteOrder_e byteOrder, uint8_t *canData, uint32_t *pMuxId) {
    FAS_ASSERT(canData != NULL_PTR);
    uint32_t index    = 0;
    uint64_t message  = 0;
    float tmpVal      = 0.0f;
    float offset      = 0.0f;
    float factor      = 0.0f;
    uint64_t bitStart = 0;
    uint8_t bitLength = 0;

    /* first signal to transmit cell voltages */
    if (id == 0x110u) {
        DATA_READ_DATA(&can_tableCellVoltages);
    }

    index     = id - 0x110u;
    bitLength = 10;
    /* each voltage frame contains 6 voltages */
    for (uint8_t i = 0u; i < 6u; i++) {
        /* conversion from mV to V */
        tmpVal = (can_tableCellVoltages.cellVoltage_mV[0][(index * 6u) + i]) /
                 1000.0; /* TODO: string index to be added */
        /* apply offset and factor */
        offset = 2.5f;
        factor = 200.0f;
        tmpVal = (tmpVal + offset) * factor;
        /* set data in CAN frame */
        bitStart = 10u * i;
        CAN_TxSetMessageDataWithSignalData(&message, bitStart, bitLength, (uint32_t)tmpVal, byteOrder);
    }

    /* now copy data in the buffer that will be use to send data */
    CAN_TxSetCanDataWithMessageData(&message, canData);

    return 0;
}
#pragma diag_pop

#pragma diag_push
#pragma diag_suppress 880
uint32_t CAN_TxPcbTemperature(uint32_t id, uint8_t dlc, CAN_byteOrder_e byteOrder, uint8_t *canData, uint32_t *pMuxId) {
    FAS_ASSERT(canData != NULL_PTR);
    uint64_t message  = 0;
    float tmpVal      = 0.0f;
    float offset      = 0.0f;
    float factor      = 0.0f;
    uint64_t bitStart = 0;
    uint8_t bitLength = 0;

    DATA_READ_DATA(&can_tableAdcTemperatures, &can_tableOpenWire);

    /* Three PCB temperatures to send */
    bitLength = 16;
    for (int i = 0; i < 3; i++) {
        /* apply offset and factor */
        tmpVal = (float)can_tableAdcTemperatures.temperatureAdc0_ddegC[i];
        offset = 0.0f;
        factor = 0.1f;
        tmpVal = (tmpVal + offset) * factor;
        /* set data in CAN frame */
        bitStart = 16 * i;
        CAN_TxSetMessageDataWithSignalData(&message, bitStart, bitLength, (uint32_t)tmpVal, byteOrder);
    }

    /* Number of open wires */
    bitLength = 16;
    bitStart  = 48;
    tmpVal    = (float)can_tableOpenWire.nrOpenWires[0]; /* TODO: string index to be added */
    offset    = 0.0f;
    factor    = 1.0f;
    tmpVal    = (tmpVal + offset) * factor;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, bitStart, bitLength, (uint32_t)tmpVal, byteOrder);

    /* now copy data in the buffer that will be use to send data */
    CAN_TxSetCanDataWithMessageData(&message, canData);

    return 0;
}
#pragma diag_pop

#pragma diag_push
#pragma diag_suppress 880
uint32_t CAN_TxExternalTemperature(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *canData,
    uint32_t *pMuxId) {
    FAS_ASSERT(canData != NULL_PTR);
    uint64_t message  = 0;
    float tmpVal      = 0.0f;
    float offset      = 0.0f;
    float factor      = 0.0f;
    uint64_t bitStart = 0;
    uint8_t bitLength = 0;

    DATA_READ_DATA(&can_tableAdcTemperatures, &can_tableCurrentSensor);

    /* Three external temperatures to send */
    bitLength = 16;
    for (int i = 0; i < 3; i++) {
        /* apply offset and factor */
        tmpVal = (float)can_tableAdcTemperatures.temperatureAdc1_ddegC[i];
        offset = 0.0f;
        factor = 0.1f;
        tmpVal = (tmpVal + offset) * factor;
        /* set data in CAN frame */
        bitStart = 16 * i;
        CAN_TxSetMessageDataWithSignalData(&message, bitStart, bitLength, (uint32_t)tmpVal, byteOrder);
    }

    /* current measurement */
    bitLength = 16;
    bitStart  = 48;
    tmpVal    = (float)can_tableCurrentSensor.current_mA[0]; /* TODO: string index to be added */
    offset    = 0.0f;
    factor    = 1.0f;
    tmpVal    = (tmpVal + offset) * factor;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, bitStart, bitLength, (uint32_t)tmpVal, byteOrder);

    /* now copy data in the buffer that will be use to send data */
    CAN_TxSetCanDataWithMessageData(&message, canData);

    return 0;
}
#pragma diag_pop

#pragma diag_push
#pragma diag_suppress 880
static uint32_t CAN_TxVoltageMinMax(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *canData,
    uint32_t *pMuxId) {
    FAS_ASSERT(canData != NULL_PTR);
    uint64_t message  = 0;
    float tmpVal      = 0.0f;
    float offset      = 0.0f;
    float factor      = 0.0f;
    uint64_t bitStart = 0;
    uint8_t bitLength = 0;

    DATA_READ_DATA(&can_tableMinimumMaximumValues);

    /* min voltage */
    bitStart  = 0;
    bitLength = 16;
    tmpVal    = (float)can_tableMinimumMaximumValues.minimumCellVoltage_mV[0]; /* TODO: string index to be added */
    offset    = 0.0f;
    factor    = 1.0f;
    tmpVal    = (tmpVal + offset) * factor;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, bitStart, bitLength, (uint32_t)tmpVal, byteOrder);

    /* apply offset and factor */
    bitStart  = 16;
    bitLength = 8;
    tmpVal    = (float)can_tableMinimumMaximumValues.nrCellMinimumCellVoltage[0]; /* TODO: string index to be added */
    offset    = 0.0f;
    factor    = 1.0f;
    tmpVal    = (tmpVal + offset) * factor;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, bitStart, bitLength, (uint32_t)tmpVal, byteOrder);

    /* apply offset and factor */
    bitStart  = 24;
    bitLength = 16;
    tmpVal    = (float)can_tableMinimumMaximumValues.maximumCellVoltage_mV[0]; /* TODO: string index to be added */
    offset    = 0.0f;
    factor    = 1.0f;
    tmpVal    = (tmpVal + offset) * factor;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, bitStart, bitLength, (uint32_t)tmpVal, byteOrder);

    /* apply offset and factor */
    bitStart  = 40;
    bitLength = 8;
    tmpVal    = (float)can_tableMinimumMaximumValues.nrCellMaximumCellVoltage[0]; /* TODO: string index to be added */
    offset    = 0.0f;
    factor    = 1.0f;
    tmpVal    = (tmpVal + offset) * factor;
    /* set data in CAN frame */
    CAN_TxSetMessageDataWithSignalData(&message, bitStart, bitLength, (uint32_t)tmpVal, byteOrder);

    /* now copy data in the buffer that will be use to send data */
    CAN_TxSetCanDataWithMessageData(&message, canData);

    return 0;
}

#pragma diag_pop

#pragma diag_push
#pragma diag_suppress 880
static uint32_t CAN_RxImdInfo(uint32_t id, uint8_t dlc, CAN_byteOrder_e byteOrder, uint8_t *canData, uint32_t *pMuxId) {
    FAS_ASSERT(canData != NULL_PTR);
    CAN_BUFFERELEMENT_s canMessage = {0u};
    uint32_t retVal                = 1u;

    canMessage.id = id;
    for (uint8_t i = 0; i < dlc; i++) {
        canMessage.data[i] = canData[i];
    }
    if (pdPASS == xQueueSend(imd_canDataQueue, (void *)&canMessage, 0u)) {
        retVal = 0u;
    }
    return retVal;
}

#pragma diag_pop

#pragma diag_push
#pragma diag_suppress 880
static uint32_t CAN_RxImdResponse(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *canData,
    uint32_t *pMuxId) {
    FAS_ASSERT(canData != NULL_PTR);
    CAN_BUFFERELEMENT_s canMessage = {0u};
    uint32_t retVal                = 1u;

    canMessage.id = id;
    for (uint8_t i = 0; i < dlc; i++) {
        canMessage.data[i] = canData[i];
    }
    if (pdPASS == xQueueSend(imd_canDataQueue, (void *)&canMessage, 0u)) {
        retVal = 0u;
    }
    return retVal;
}

#pragma diag_pop

#pragma diag_push
#pragma diag_suppress 880
static uint32_t CAN_RxRequest(uint32_t id, uint8_t dlc, CAN_byteOrder_e byteOrder, uint8_t *canData, uint32_t *pMuxId) {
    FAS_ASSERT(canData != NULL_PTR);
    uint64_t message     = 0;
    uint64_t signal      = 0;
    uint64_t bitStart    = 0;
    uint8_t bitLength    = 0;
    uint8_t stateRequest = 0;

    DATA_READ_DATA(&can_tableStateRequest);

    bitStart  = 8;
    bitLength = 8;
    CAN_RxGetMessageDataFromCanData(canData, &message);
    CAN_RxGetSignalDataFromMessageData(message, bitStart, bitLength, &signal, byteOrder);

    stateRequest                                     = (uint8_t)signal;
    can_tableStateRequest.previousStateRequestViaCan = can_tableStateRequest.stateRequestViaCan;
    can_tableStateRequest.stateRequestViaCan         = stateRequest;
    if ((can_tableStateRequest.stateRequestViaCan != can_tableStateRequest.previousStateRequestViaCan) ||
        ((OS_GetTickCount() - can_tableStateRequest.header.timestamp) > 3000)) {
        can_tableStateRequest.stateRequestViaCanPending = stateRequest;
    }
    can_tableStateRequest.state++;

    DATA_WRITE_DATA(&can_tableStateRequest);

    return 0;
}

#pragma diag_pop

#pragma diag_push
#pragma diag_suppress 880
static uint32_t CAN_RxSwReset(uint32_t id, uint8_t dlc, CAN_byteOrder_e byteOrder, uint8_t *canData, uint32_t *pMuxId) {
    FAS_ASSERT(canData != NULL_PTR);
    return 0;
}

#pragma diag_pop

#pragma diag_push
#pragma diag_suppress 880
static uint32_t CAN_RxCurrentSensor(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *canData,
    uint32_t *pMuxId) {
    FAS_ASSERT(canData != NULL_PTR);
    uint64_t message  = 0u;
    uint64_t signal   = 0u;
    uint64_t bitStart = 0u;
    uint8_t bitLength = 0u;

    int32_t tempValue    = 0;
    uint8_t dummy        = 0u;
    uint8_t stringNumber = 0;

    /* IDs are used as follows:
       - String 0:  0x521 - 0x528
       - String 1:  0x621 - 0x628
       - String 2:  0x721 - 0x728 */
    if (id <= 0x528u) {
        stringNumber = 0u;
    } else if (id <= 0x628u) {
        stringNumber = 1u;
    } else {
        stringNumber = 2u;
    }

    CAN_RxGetMessageDataFromCanData(canData, &message);

    /* Get status*/
    bitStart  = 0;
    bitLength = 8;
    CAN_RxGetSignalDataFromMessageData(message, bitStart, bitLength, &signal, byteOrder);

    dummy = (uint32_t)signal;
    dummy &= 0xF0u; /* only high nibble contains diag info */
    if ((dummy & 0x10u) == 0x10u) {
        /* Overcurrent detected. This feature is currently not supported. */
    }
    if ((dummy & 0x20u) == 0x20u) {
        switch (id) {
            case 0x521u: /* Current status */
                can_tableCurrentSensor.invalidCurrentMeasurement[stringNumber] = 1;
                break;
            case 0x522u: /* Voltage status */
                can_tableCurrentSensor.invalidHighVoltageMeasurement[stringNumber][0] = 1;
                break;
            case 0x523u:
                can_tableCurrentSensor.invalidHighVoltageMeasurement[stringNumber][1] = 1;
                break;
            case 0x524u:
                can_tableCurrentSensor.invalidHighVoltageMeasurement[stringNumber][2] = 1;
                break;
            case 0x525u: /* Temperature status */
                can_tableCurrentSensor.invalidSensorTemperatureMeasurement[stringNumber] = 1;
                break;
            case 0x526u: /* Power status */
                can_tableCurrentSensor.invalidPowerMeasurement[stringNumber] = 1;
                break;
            case 0x527u: /* CC status */
                can_tableCurrentSensor.invalidCurrentCountingMeasurement[stringNumber] = 1;
                break;
            case 0x528u: /* EC status */
                can_tableCurrentSensor.invalidEnergyCountingMeasurement[stringNumber] = 1;
                break;
            default:
                break;
        }
    } else {
        can_tableCurrentSensor.invalidCurrentMeasurement[stringNumber]           = 0;
        can_tableCurrentSensor.invalidHighVoltageMeasurement[stringNumber][0]    = 0;
        can_tableCurrentSensor.invalidHighVoltageMeasurement[stringNumber][1]    = 0;
        can_tableCurrentSensor.invalidHighVoltageMeasurement[stringNumber][2]    = 0;
        can_tableCurrentSensor.invalidSensorTemperatureMeasurement[stringNumber] = 0;
        can_tableCurrentSensor.invalidPowerMeasurement[stringNumber]             = 0;
        can_tableCurrentSensor.invalidCurrentCountingMeasurement[stringNumber]   = 0;
        can_tableCurrentSensor.invalidEnergyCountingMeasurement[stringNumber]    = 0;
    }

    if (((dummy & 0x40u) == 0x40u) || ((dummy & 0x80u) == 0x80u)) {
        can_tableCurrentSensor.invalidCurrentMeasurement[stringNumber]           = 1;
        can_tableCurrentSensor.invalidHighVoltageMeasurement[stringNumber][0]    = 1;
        can_tableCurrentSensor.invalidHighVoltageMeasurement[stringNumber][1]    = 1;
        can_tableCurrentSensor.invalidHighVoltageMeasurement[stringNumber][2]    = 1;
        can_tableCurrentSensor.invalidSensorTemperatureMeasurement[stringNumber] = 1;
        can_tableCurrentSensor.invalidPowerMeasurement[stringNumber]             = 1;
        can_tableCurrentSensor.invalidCurrentCountingMeasurement[stringNumber]   = 1;
        can_tableCurrentSensor.invalidEnergyCountingMeasurement[stringNumber]    = 1;
    }

    /* Get measurement */
    bitStart  = 16;
    bitLength = 32;
    CAN_RxGetSignalDataFromMessageData(message, bitStart, bitLength, &signal, byteOrder);
    switch (id) {
        /* Current measurement */
        case 0x521u:
        case 0x621u:
        case 0x721u:
            tempValue                                       = (int32_t)signal;
            can_tableCurrentSensor.current_mA[stringNumber] = tempValue;
            can_tableCurrentSensor.newCurrent++;
            can_tableCurrentSensor.previousTimestampCurrent[stringNumber] =
                can_tableCurrentSensor.timestampCurrent[stringNumber];
            can_tableCurrentSensor.timestampCurrent[stringNumber] = OS_GetTickCount();
            break;
        /* Voltage measurement U1 */
        case 0x522u:
        case 0x622u:
        case 0x722u:
            tempValue                                              = (int32_t)signal;
            can_tableCurrentSensor.highVoltage_mV[stringNumber][0] = tempValue;
            can_tableCurrentSensor.previousTimestampHighVoltage[stringNumber][0] =
                can_tableCurrentSensor.timestampHighVoltage[stringNumber][0];
            can_tableCurrentSensor.timestampHighVoltage[stringNumber][0] = OS_GetTickCount();
            break;
        /* Voltage measurement U2 */
        case 0x523u:
        case 0x623u:
        case 0x723u:
            tempValue                                              = (int32_t)signal;
            can_tableCurrentSensor.highVoltage_mV[stringNumber][1] = tempValue;
            can_tableCurrentSensor.previousTimestampHighVoltage[stringNumber][1] =
                can_tableCurrentSensor.timestampHighVoltage[stringNumber][1];
            can_tableCurrentSensor.timestampHighVoltage[stringNumber][1] = OS_GetTickCount();
            break;
        /* Voltage measurement U3 */
        case 0x524u:
        case 0x624u:
        case 0x724u:
            tempValue                                              = (int32_t)signal;
            can_tableCurrentSensor.highVoltage_mV[stringNumber][2] = tempValue;
            can_tableCurrentSensor.previousTimestampHighVoltage[stringNumber][2] =
                can_tableCurrentSensor.timestampHighVoltage[stringNumber][2];
            can_tableCurrentSensor.timestampHighVoltage[stringNumber][2] = OS_GetTickCount();
            break;
        /* Temperature measurement */
        case 0x525u:
        case 0x625u:
        case 0x725u:
            tempValue                                                    = (int32_t)signal;
            can_tableCurrentSensor.sensorTemperature_ddegC[stringNumber] = tempValue;
            break;
        /* Power measurement */
        case 0x526u:
        case 0x626u:
        case 0x726u:
            tempValue                                    = (int32_t)signal;
            can_tableCurrentSensor.power_W[stringNumber] = tempValue;
            can_tableCurrentSensor.newPower++;
            can_tableCurrentSensor.previousTimestampPower[stringNumber] =
                can_tableCurrentSensor.timestampPower[stringNumber];
            can_tableCurrentSensor.timestampPower[stringNumber] = OS_GetTickCount();
            break;
        /* CC measurement */
        case 0x527u:
        case 0x627u:
        case 0x727u:
            tempValue = (int32_t)signal;
            can_tableCurrentSensor.previousTimestampCurrentCounting[stringNumber] =
                can_tableCurrentSensor.timestampCurrentCounting[stringNumber];
            can_tableCurrentSensor.timestampCurrentCounting[stringNumber] = OS_GetTickCount();
            can_tableCurrentSensor.currentCounter_As[stringNumber]        = tempValue;
            break;
        /* EC measurement */
        case 0x528u:
        case 0x628u:
        case 0x728u:
            tempValue                                             = (int32_t)signal;
            can_tableCurrentSensor.energyCounter_Wh[stringNumber] = tempValue;
            can_tableCurrentSensor.previousTimestampEnergyCounting[stringNumber] =
                can_tableCurrentSensor.timestampEnergyCounting[stringNumber];
            can_tableCurrentSensor.timestampEnergyCounting[stringNumber] = OS_GetTickCount();
            break;

        default:
            FAS_ASSERT(FAS_TRAP);
            break;
    }

    DATA_WRITE_DATA(&can_tableCurrentSensor);
    return 0;
}
#pragma diag_pop

#pragma diag_push
#pragma diag_suppress 880
static uint32_t CAN_RxDebug(uint32_t id, uint8_t dlc, CAN_byteOrder_e byteOrder, uint8_t *canData, uint32_t *pMuxId) {
    FAS_ASSERT(canData != NULL_PTR);

    uint64_t message  = 0;
    uint64_t signal   = 0;
    uint64_t bitStart = 0;
    uint8_t bitLength = 0;

    CAN_RxGetMessageDataFromCanData(canData, &message);

    bitStart  = 0;
    bitLength = 8;
    CAN_RxGetSignalDataFromMessageData(message, bitStart, bitLength, &signal, byteOrder);

    switch ((uint8_t)signal) {
        case 0xAA:
            for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
                DIAG_Handler(DIAG_ID_DEEP_DISCHARGE_DETECTED, DIAG_EVENT_OK, DIAG_STRING, stringNumber);
            }
            break;

        default:
            break;
    }
    return 0;
}
#pragma diag_pop

#pragma diag_push
#pragma diag_suppress 880
static uint32_t CAN_RxSwVersion(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *canData,
    uint32_t *pMuxId) {
    FAS_ASSERT(canData != NULL_PTR);
    return 0;
}
#pragma diag_pop

static void CAN_TxSetMessageDataWithSignalData(
    uint64_t *pMessage,
    uint64_t bitStart,
    uint8_t bitLength,
    uint64_t signal,
    CAN_byteOrder_e byteOrder) {
    FAS_ASSERT(pMessage != NULL_PTR);
    uint64_t mask = 0xFFFFFFFFFFFFFFFFu;

    /* Swap byte order if necessary */
    if (byteOrder == littleEndian) {
        /* No need to switch byte order as native MCU endianness is little-endian (intel) */
    } else if (byteOrder == bigEndian) {
        if (bitLength <= 8u) {
            /* No need to switch byte order as signal length is smaller than one byte */
        } else if (bitLength <= 16u) {
            /* Swap byte order */
            signal = (uint64_t)MATH_swapBytes_uint16_t((uint16_t)signal);
        } else if (bitLength <= 32u) {
            /* Swap byte order */
            signal = (uint64_t)MATH_swapBytes_uint32_t((uint32_t)signal);
        } else { /* (signal.bit_length <= 64) */
            /* Swap byte order */
            signal = MATH_swapBytes_uint64_t(signal);
        }
    }

    if ((bitLength < 64u) && (bitStart < 64u)) {
        mask <<= bitLength;
        mask = ~mask;

        *pMessage |= (signal & mask) << bitStart;
    } else {
        /* unexpectedly large bit-shift */
        FAS_ASSERT(FAS_TRAP);
    }
}

static void CAN_TxSetCanDataWithMessageData(uint64_t *pMessage, uint8_t *pCanData) {
    FAS_ASSERT(pMessage != NULL_PTR);
    FAS_ASSERT(pCanData != NULL_PTR);
    pCanData[0] = (uint8_t)(((*pMessage) >> 0u) & 0xFFu);
    pCanData[1] = (uint8_t)(((*pMessage) >> 8u) & 0xFFu);
    pCanData[2] = (uint8_t)(((*pMessage) >> 16u) & 0xFFu);
    pCanData[3] = (uint8_t)(((*pMessage) >> 24u) & 0xFFu);
    pCanData[4] = (uint8_t)(((*pMessage) >> 32u) & 0xFFu);
    pCanData[5] = (uint8_t)(((*pMessage) >> 40u) & 0xFFu);
    pCanData[6] = (uint8_t)(((*pMessage) >> 48u) & 0xFFu);
    pCanData[7] = (uint8_t)(((*pMessage) >> 56u) & 0xFFu);
}

static void CAN_RxGetSignalDataFromMessageData(
    uint64_t message,
    uint64_t bitStart,
    uint8_t bitLength,
    uint64_t *pSignal,
    CAN_byteOrder_e byteOrder) {
    FAS_ASSERT(pSignal != NULL_PTR);
    uint64_t mask = 0xFFFFFFFFFFFFFFFFu;

    if ((bitLength < 64u) && (bitStart < 64u)) {
        mask <<= bitLength;
        mask = ~mask;

        *pSignal = (message >> bitStart) & mask;
    } else {
        /* unexpectedly large bit-shift */
        FAS_ASSERT(FAS_TRAP);
    }

    /* Swap byte order if necessary */
    if (byteOrder == littleEndian) {
        /* No need to switch byte order as native MCU endianness is little-endian (intel) */
    } else if (byteOrder == bigEndian) {
        if (bitLength <= 8u) {
            /* No need to switch byte order as signal length is smaller than one byte */
        } else if (bitLength <= 16u) {
            /* Swap byte order */
            *pSignal = (uint64_t)MATH_swapBytes_uint16_t((uint16_t)*pSignal);
        } else if (bitLength <= 32u) {
            /* Swap byte order */
            *pSignal = (uint64_t)MATH_swapBytes_uint32_t((uint32_t)*pSignal);
        } else { /* (signal.bit_length <= 64) */
            /* Swap byte order */
            *pSignal = MATH_swapBytes_uint64_t(*pSignal);
        }
    }
}

static void CAN_RxGetMessageDataFromCanData(uint8_t *pCanData, uint64_t *pMessage) {
    FAS_ASSERT(pMessage != NULL_PTR);
    FAS_ASSERT(pCanData != NULL_PTR);
    *pMessage = ((((uint64_t)pCanData[0]) & 0xFFu) << 0u) | ((((uint64_t)pCanData[1]) & 0xFFu) << 8u) |
                ((((uint64_t)pCanData[2]) & 0xFFu) << 16u) | ((((uint64_t)pCanData[3]) & 0xFFu) << 24u) |
                ((((uint64_t)pCanData[4]) & 0xFFu) << 32u) | ((((uint64_t)pCanData[5]) & 0xFFu) << 40u) |
                ((((uint64_t)pCanData[6]) & 0xFFu) << 48u) | ((((uint64_t)pCanData[7]) & 0xFFu) << 56u);
}

/*========== Extern Function Implementations ================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern DATA_BLOCK_CELL_VOLTAGE_s *TEST_CAN_GetCellvoltageTab(void) {
    return &can_tableCellVoltages;
}

/* TX callback functions */
extern uint32_t TEST_CAN_TxVoltage(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *pCanData,
    uint32_t *pMuxId) {
    return CAN_TxVoltage(id, dlc, byteOrder, pCanData, pMuxId);
}
extern uint32_t TEST_CAN_TxPcbTemperature(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *pCanData,
    uint32_t *pMuxId) {
    return CAN_TxPcbTemperature(id, dlc, byteOrder, pCanData, pMuxId);
}
extern uint32_t TEST_CAN_TxExternalTemperature(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *pCanData,
    uint32_t *pMuxId) {
    return CAN_TxExternalTemperature(id, dlc, byteOrder, pCanData, pMuxId);
}
extern uint32_t TEST_CAN_TxVoltageMinMax(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *pCanData,
    uint32_t *pMuxId) {
    return CAN_TxVoltageMinMax(id, dlc, byteOrder, pCanData, pMuxId);
}

/* RX callback functions */
extern uint32_t TEST_CAN_RxRequest(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *pCanData,
    uint32_t *pMuxId) {
    return CAN_RxRequest(id, dlc, byteOrder, pCanData, pMuxId);
}
extern uint32_t TEST_CAN_RxSwReset(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *pCanData,
    uint32_t *pMuxId) {
    return CAN_RxSwReset(id, dlc, byteOrder, pCanData, pMuxId);
}
extern uint32_t TEST_CAN_RxCurrentSensor(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *pCanData,
    uint32_t *pMuxId) {
    return CAN_RxCurrentSensor(id, dlc, byteOrder, pCanData, pMuxId);
}
extern uint32_t TEST_CAN_RxDebug(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *pCanData,
    uint32_t *pMuxId) {
    return CAN_RxDebug(id, dlc, byteOrder, pCanData, pMuxId);
}
extern uint32_t TEST_CAN_RxSwVersion(
    uint32_t id,
    uint8_t dlc,
    CAN_byteOrder_e byteOrder,
    uint8_t *pCanData,
    uint32_t *pMuxId) {
    return CAN_RxSwVersion(id, dlc, byteOrder, pCanData, pMuxId);
}
#endif
