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
 * @file    can_cbs_rx.h
 * @author  foxBMS Team
 * @date    2021-04-20 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup DRIVERS
 * @prefix  CANRX
 *
 * @brief   CAN callbacks header
 * @details TODO
 */

#ifndef FOXBMS__CAN_CBS_RX_H_
#define FOXBMS__CAN_CBS_RX_H_

/*========== Includes =======================================================*/

#include "can_cfg.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/** RX callback functions @{ */
/**
 * @brief   CAN Rx callback function for IMD info messages
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] kpkCanData  payload of can frame
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANRX_ImdInfo(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   CAN Rx callback function for IMD response messages
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] kpkCanData  payload of can frame
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANRX_ImdResponse(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   CAN Rx callback function for state requests
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] kpkCanData  payload of can frame
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANRX_BmsStateRequest(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   CAN Rx callback function for current sensor measurements
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] kpkCanData  payload of can frame
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANRX_CurrentSensor(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   CAN Rx callback function for debug messages
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] kpkCanData  payload of can frame
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANRX_Debug(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   CAN Rx callback function for aerosol sensor messages
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] kpkCanData  payload of can frame
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANRX_AerosolSensor(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   CAN Rx callback function to handle cell-temperatures message
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] kpkCanData  payload of can frame
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANRX_CellTemperatures(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim);

/**
 * @brief   CAN Rx callback function to handle cell-voltages message
 * @param[in] message     contains the message ID, DLC and endianness
 * @param[in] kpkCanData  payload of can frame
 * @param[in] kpkCanShim  shim to the database entries
 */
extern uint32_t CANRX_CellVoltages(
    CAN_MESSAGE_PROPERTIES_s message,
    const uint8_t *const kpkCanData,
    const CAN_SHIM_s *const kpkCanShim);
/**@}*/

/**
 * @brief   Gets data from IMD Info message and stores it in data table
 * @param   kpkCanData Can data from the IMD Info message
 * @param   pTableInsulationMonitoring Pointer to the data table where data will be written
 */
void CANRX_ImdInfoGetDataFromMessage(
    const uint8_t *const kpkCanData,
    DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring);

/**
 * @brief   Checks the current measurement mode
 * @param   kpkCanData Can data from the IMD Info message
 * @param   mode Mode that the actual value is compared to
 * @return  True if mode from can data and mode are equal
 */
bool CANRX_ImdInfoCheckMeasurementMode(const uint8_t *const kpkCanData, uint8_t mode);

/**
 * @brief   Checks if self test has been executed
 * @param   kpkCanData Can data from the IMD Info message
 * @return  true if self test has been executed
 */
bool CANRX_ImdInfoHasSelfTestBeenExecuted(const uint8_t *const kpkCanData);

/**
 * @brief   Checks if self test has finished
 * @param   kpkCanData Can data from the IMD Info message
 * @return  true if self test has finished
 */
bool CANRX_ImdInfoIsSelfTestFinished(const uint8_t *const kpkCanData);

/**
 * @brief   Gets insulation resistance from IMD response message and stores it in the data table
 * @param   kpkCanData Can data from the IMD Info message
 * @param   pTableInsulationMonitoring Pointer to the data table where data will be written
 */
void CANRX_ImdResponseReadInsulationResistance(
    const uint8_t *const kpkCanData,
    DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring);

/**
 * @brief   Gets tendency of the insulation fault from IMD response message and stores it in the data table
 * @param   kpkCanData Can data from the IMD Info message
 * @param   pTableInsulationMonitoring Pointer to the data table where data will be written
 */
void CANRX_ImdResponseCheckInsulationFaultTendency(
    const uint8_t *const kpkCanData,
    DATA_BLOCK_INSULATION_MONITORING_s *pTableInsulationMonitoring);

/**
 * @brief   Checks the current relay state of one relay
 * @param   kpkCanData Can data from the IMD Info message
 * @param   relay Relay that the actual value is compared to
 * @param   relayState Relay state that the actual value is compared to
 * @return  True if relay state from can data and relay are equal
 */
bool CANRX_ImdResponseCheckRelayState(const uint8_t *const kpkCanData, uint8_t relay, uint8_t relayState);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
/* externalized functions from src/app/driver/can/cbs/rx/can_cbs_rx_current-sensor.c */
extern void TEST_CANRX_HandleChannelError(const CAN_SHIM_s *const kpkCanShim, uint32_t messageId, uint8_t stringNumber);
extern void TEST_CANRX_ResetError(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber);
extern void TEST_CANRX_HandleMeasurementError(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber);
extern void TEST_CANRX_HandleSystemError(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber);
extern void TEST_CANRX_HandleSensorData(
    const CAN_SHIM_s *const kpkCanShim,
    uint32_t messageId,
    uint8_t stringNumber,
    int32_t signalData);
extern void TEST_CANRX_SetCurrent(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData);
extern void TEST_CANRX_SetVoltageU1(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData);
extern void TEST_CANRX_SetVoltageU2(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData);
extern void TEST_CANRX_SetVoltageU3(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData);
extern void TEST_CANRX_SetTemperature(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData);
extern void TEST_CANRX_SetPower(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData);
extern void TEST_CANRX_SetCoulombCounting(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData);
extern void TEST_CANRX_SetEnergyCounting(const CAN_SHIM_s *const kpkCanShim, uint8_t stringNumber, int32_t signalData);
extern uint8_t TEST_CANRX_SetStringNumberBasedOnCanMessageId(uint32_t messageId);

/* externalized functions from src/app/driver/can/cbs/rx/can_cbs_rx_debug.c */
extern uint8_t TEST_CANRX_GetHundredthOfSeconds(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern uint8_t TEST_CANRX_GetSeconds(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern uint8_t TEST_CANRX_GetMinutes(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern uint8_t TEST_CANRX_GetHours(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern uint8_t TEST_CANRX_GetWeekday(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern uint8_t TEST_CANRX_GetDay(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern uint8_t TEST_CANRX_GetMonth(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern uint8_t TEST_CANRX_GetYear(uint64_t messageData, CAN_ENDIANNESS_e endianness);

extern void TEST_CANRX_TriggerBmsSoftwareVersionMessage(void);
extern void TEST_CANRX_TriggerMcuUniqueDieIdMessage(void);
extern void TEST_CANRX_TriggerMcuLotNumberMessage(void);
extern void TEST_CANRX_TriggerMcuWaferInformationMessage(void);
extern void TEST_CANRX_TriggerTimeInfoMessage(void);
extern void TEST_CANRX_TriggerUptimeInfoMessage(void);
extern void TEST_CANRX_TriggerBootTimestampMessage(void);
extern void TEST_CANRX_TriggerCommitHashMessage(void);
extern void TEST_CANRX_TriggerBuildConfigurationMessage(void);

extern bool TEST_CANRX_CheckIfBmsSoftwareVersionIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern bool TEST_CANRX_CheckIfMcuUniqueDieIdIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern bool TEST_CANRX_CheckIfMcuLotNumberIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern bool TEST_CANRX_CheckIfMcuWaferInformationIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern bool TEST_CANRX_CheckIfSoftwareResetIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern bool TEST_CANRX_CheckIfFramInitializationIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern bool TEST_CANRX_CheckIfTimeInfoIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern bool TEST_CANRX_CheckIfUptimeInfoIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern bool TEST_CANRX_CheckIfBootTimestampIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern bool TEST_CANRX_CheckIfCommitHashIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern bool TEST_CANRX_CheckIfBuildConfigurationIsRequested(uint64_t messageData, CAN_ENDIANNESS_e endianness);

extern void TEST_CANRX_ProcessVersionInformationMux(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern void TEST_CANRX_ProcessRtcMux(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern void TEST_CANRX_ProcessSoftwareResetMux(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern void TEST_CANRX_ProcessFramInitializationMux(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern void TEST_CANRX_ProcessTimeInfoMux(uint64_t messageData, CAN_ENDIANNESS_e endianness);
extern void TEST_CANRX_ProcessUptimeInfoMux(uint64_t messageData, CAN_ENDIANNESS_e endianness);

extern void TEST_CANRX_HandleAerosolSensorErrors(const CAN_SHIM_s *const kpkCanShim, uint16_t signalData);
extern void TEST_CANRX_HandleAerosolSensorStatus(const CAN_SHIM_s *const kpkCanShim, uint16_t signalData);
extern void TEST_CANRX_SetParticulateMatterConcentration(const CAN_SHIM_s *const kpkCanShim, uint16_t signalData);
extern void TEST_CANRX_SetAerosolSensorCrcCheckCode(const CAN_SHIM_s *const kpkCanShim, uint16_t signalData);

/* externalized functions from src/app/driver/can/cbs/rx/can_cbs_rx_bms-state-request.c */
extern void TEST_CANRX_ClearAllPersistentFlags(uint64_t messageData);
extern void TEST_CANRX_HandleModeRequest(uint64_t messageData, const CAN_SHIM_s *const kpkCanShim);
extern void TEST_CANRX_HandleBalancingRequest(uint64_t messageData);
extern void TEST_CANRX_SetBalancingThreshold(uint64_t messageData);

/* externalized functions from src/app/driver/can/cbs/rx/can_cbs_rx_cell-temperatures.c */
extern void TEST_CANRX_GetCanAfeCellTemperaturesFromMessage(
    CAN_CAN2AFE_CELL_TEMPERATURES_QUEUE_s *pCellTemperatures,
    uint64_t messageData);
/* externalized functions from src/app/driver/can/cbs/rx/can_cbs_rx_cell-voltages.c */
extern void TEST_CANRX_GetCanAfeCellVoltagesFromMessage(
    CAN_CAN2AFE_CELL_VOLTAGES_QUEUE_s *pCellVoltages,
    uint64_t messageData);

/* externalized functions from src/app/driver/can/cbs/rx/can_cbs_rx_imd-info.c*/
extern void TEST_CANRX_TransferImdInfoMessageToCanBuffer(
    uint8_t messageDlc,
    const uint8_t *const kpkCanData,
    CAN_BUFFER_ELEMENT_s *canBuffer);

/* externalized functions from src/app/driver/can/cbs/rx/can_cbs_rx_imd-response.c*/
extern void TEST_CANRX_TransferImdResponseMessageToCanBuffer(
    uint8_t messageDlc,
    const uint8_t *const kpkCanData,
    CAN_BUFFER_ELEMENT_s *canBuffer);
#endif

#endif /* FOXBMS__CAN_CBS_RX_H_ */
