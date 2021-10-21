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
 * @file    can.h
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2021-07-23 (date of last update)
 * @ingroup DRIVERS
 * @prefix  CAN
 *
 * @brief   Header for the driver for the CAN module
 *
 * Provides the interfaces for initialization, receive
 * and transmit handling
 *
 */

#ifndef FOXBMS__CAN_H_
#define FOXBMS__CAN_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "can_cfg.h"

/*========== Macros and Definitions =========================================*/

/** Half of the 64 messageboxes are defined for TX
 * This is used to determined in the CAN interrupt routine if TX or RX case
 */
#define CAN_NR_OF_TX_MESSAGE_BOX (32U)

/** Task time slot where the CAN TX function is called. Repetition time of
 * periodic CAN messages must be multiple of this (e.g., 1u, 10u or 100u)
 */
#define CAN_TICK_MS (10U)

/** This structure contains variables relevant for the CAN signal module. */
typedef struct CAN_STATE {
    bool periodicEnable;                           /*!< defines if periodic transmit and receive should run */
    bool currentSensorPresent[BS_NR_OF_STRINGS];   /*!< defines if a current sensor is detected */
    bool currentSensorCCPresent[BS_NR_OF_STRINGS]; /*!< defines if a CC info is being sent */
    bool currentSensorECPresent[BS_NR_OF_STRINGS]; /*!< defines if a EC info is being sent */
} CAN_STATE_s;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   Sends over CAN the data passed in parameters.
 * This function goes over the messageboxes and marks the ones that should
 * be sent.
 * @param[in,out]   pNode   CAN interface to use
 * @param[in]       id      ID of message to send
 * @param[out]      pData   data to send (8 bytes)
 * @return  #STD_OK if a message box was free to send, #STD_NOT_OK otherwise
 */
extern STD_RETURN_TYPE_e CAN_DataSend(canBASE_t *pNode, uint32_t id, uint8 *pData);

/**
 * @brief   Calls the functions to drive the CAN interface.
 * Makes the CAN timing checks and sends the periodic messages.
 */
extern void CAN_MainFunction(void);

/**
 * @brief   Checks the data received per CAN.
 * A receive buffer is used because CAN frames are received in an interrupt routine.
 * The TMS570LC4357 does not allow nested interrupts, so interrupts are deactivated during receive.
 * Calls to the database do not work when interrupts are disabled.
 * Receive callbacks are made within this function: as it is not called during an interrupt routine,
 * calls to the database can be made.
 */
extern void CAN_ReadRxBuffer(void);

/**
 * @brief   Enables the CAN transceiver..
 * This function sets th pins to enable the CAN transceiver.
 * It must be called before using the CAN interface.
 */
extern void CAN_Initialize(void);

/**
 * @brief   Enables periodic sending per CAN.
 * This is used to prevent sending uninitialized data per CAN
 * (e.g., before the first LTC measurement cycle was completed).
 */
extern void CAN_EnablePeriodic(bool command);

/**
 * @brief   set flag for presence of current sensor.
 * @return  retval  true if a current sensor is present, false otherwise
 */
extern bool CAN_IsCurrentSensorPresent(uint8_t stringNumber);

/**
 * @brief   get flag if CC message from current sensor is received.
 * @param   stringNumber   addressed string
 * @return  true if CC message is being received, false otherwise
 */
extern bool CAN_IsCurrentSensorCcPresent(uint8_t stringNumber);

/**
 * @brief   get flag if EC message from current sensor is received
 * @param   stringNumber   addressed string
 * @return  true if EC message is being received, false otherwise
 */
extern bool CAN_IsCurrentSensorEcPresent(uint8_t stringNumber);

/**
 * @brief   Transmit startup boot message
 * @return  #STD_OK if transmission successful, otherweise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e CAN_TransmitBootMessage(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern CAN_STATE_s *TEST_CAN_GetCANState(void);
#endif

#endif /* FOXBMS__CAN_H_ */
