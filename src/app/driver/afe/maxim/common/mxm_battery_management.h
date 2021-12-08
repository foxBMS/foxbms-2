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
 * @file    mxm_battery_management.h
 * @author  foxBMS Team
 * @date    2019-01-14 (date of creation)
 * @updated 2021-12-06 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MXM
 *
 * @brief   Headers for the driver for the MAX17841B ASCI and
 *          MAX1785x monitoring chip
 *
 * @details def
 *
 */

#ifndef FOXBMS__MXM_BATTERY_MANAGEMENT_H_
#define FOXBMS__MXM_BATTERY_MANAGEMENT_H_

/*========== Includes =======================================================*/
#include "mxm_cfg.h"

#include "mxm_17841b.h"
#include "mxm_basic_defines.h"
#include "mxm_crc8.h"
#include "mxm_register_map.h"

/*========== Macros and Definitions =========================================*/

/** length of the command buffer */
#define COMMAND_BUFFER_LENGTH (10u)

/** length of the rx buffer */
#define MXM_5X_RX_BUFFER_LEN 100u

/** seed for helloall, should be 0x00u except for special edge cases */
#define HELLOALL_START_SEED 0x00u
/** seed for data check, should be 0x00u */
#define DATA_CHECK_BYTE_SEED 0x00u

/**
 * @brief   Type for Battery Management Protocol commands.
 * @details This type describes the hex-numbers of all Maxim Battery
 *          Management Protocol commands. For details see the
 *          data sheet of MAX1785x devices.
 */
typedef uint8_t MXM_BATTERY_MANAGEMENT_COMMAND_t;

/**
 * @brief HELLOALL message
 */
#define BATTERY_MANAGEMENT_HELLOALL ((MXM_BATTERY_MANAGEMENT_COMMAND_t)0x57u)

/**
 * @brief ALERTPACKET message
 */
#define BATTERY_MANAGEMENT_ALERTPACKET ((MXM_BATTERY_MANAGEMENT_COMMAND_t)0x21u)

/**
 * @brief WRITEDEVICE message (write single register of a single device)
 */
#define BATTERY_MANAGEMENT_WRITEDEVICE ((MXM_BATTERY_MANAGEMENT_COMMAND_t)0x04u)

/**
 * @brief WRITEALL message (write single register of all daisy-chain devices)
 */
#define BATTERY_MANAGEMENT_WRITEALL ((MXM_BATTERY_MANAGEMENT_COMMAND_t)0x02u)

/**
 * @brief READDEVICE message (read single register of a single device)
 */
#define BATTERY_MANAGEMENT_READDEVICE ((MXM_BATTERY_MANAGEMENT_COMMAND_t)0x05u)

/**
 * @brief READALL message (read single register of all daisy-chain devices)
 */
#define BATTERY_MANAGEMENT_READALL ((MXM_BATTERY_MANAGEMENT_COMMAND_t)0x03u)

/**
 * @brief READBLOCK message (read block of registers of a single device)
 */
#define BATTERY_MANAGEMENT_READBLOCK ((MXM_BATTERY_MANAGEMENT_COMMAND_t)0x06u)

/**
 * @brief   DOWNHOST message (make the downhost writing)
 * @details This feature is only useable on downhost line.
 */
#define BATTERY_MANAGEMENT_DOWNHOST ((MXM_BATTERY_MANAGEMENT_COMMAND_t)0x09u)

/**
 * @brief UPHOST message (make the uphost writing, only useable on uphost line)
 */
#define BATTERY_MANAGEMENT_UPHOST ((MXM_BATTERY_MANAGEMENT_COMMAND_t)0x08u)

/**
 * @brief   Battery Management Protocol lengths of TX buffer
 * @details Reference values for the lengths of the respective TX buffers
 *          for Battery Management Protocol messages.
 */

/**
 * @brief length of a READALL command without stuffing bytes
 */
#define BATTERY_MANAGEMENT_TX_LENGTH_READALL ((uint8_t)4u)

/**
 * @brief States of the Battery Management Protocol state-machine
 */
typedef enum {
    MXM_STATEMACH_5X_UNINITIALIZED,  /*!< Uninitialized state that the state-machine starts in */
    MXM_STATEMACH_5X_INIT,           /*!< Initialization state */
    MXM_STATEMACH_5X_41B_FMEA_CHECK, /*!< Requests a FMEA-check from the 41B-state-machine */
    MXM_STATEMACH_5X_IDLE,           /*!< Idle state, transitions are possible in this state */
    MXM_STATEMACH_5X_WRITEALL,       /*!< WRITEALL state, sends a WRITEALL to the daisy-chain */
    MXM_STATEMACH_5X_WRITE_DEVICE,   /*!< WRITEDEVICE state, sends a WRITEDEVICE to a specific address */
    MXM_STATEMACH_5X_READALL,        /*!< READALL state, sends a READALL to the daisy-chain */
    MXM_STATEMACH_5X_MAXSTATE,       /*!< Highest state */
} MXM_STATEMACHINE_5X_e;

/**
 * @brief Sub-states of the Battery Management Protocol state-machine
 */
typedef enum {
    MXM_5X_INIT_41B_INIT,        /*!< initialization of the MAX17841 bridge IC */
    MXM_5X_INIT_41B_GET_VERSION, /*!< retrieves the version of the bridge IC */
    /** waits for the slave devices to be reset (due to disabled preambles) */
    MXM_5X_INIT_WAIT_FOR_RESET,
    /** substate for the initialization, enable RX interrupt flags */
    MXM_5X_INIT_ENABLE_RX_INTERRUPT_FLAGS,
    /** substate for the wake-up routine, clear the receive buffer in the
     *  bridge IC */
    MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_CLEAR_RECEIVE_BUFFER,
    /** substate for the wake-up routine, enable the sending of preambles in
     *  order to wake up the daisy-chain devices */
    MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_EN_PREAMBLES,
    MXM_5X_INIT_ENABLE_KEEP_ALIVE, /*!< enables keep alive mode (which will take over after en preambles is off) */
    /** substate for the wake-up routine, wait for the daisy-chain devices to
     *  return an answer, resulting in status RX busy of the bridge IC */
    MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_WAIT_FOR_RX_STATUS_BUSY,
    /** substate for the wake-up routine, disable the sending of preambles */
    MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_DIS_PREAMBLES,
    /** substate for the wake-up routine, wait for the RX transmission to
     *  finish */
    MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_WAIT_FOR_RX_STATUS_EMPTY,
    /** substate for the wake-up routine, clear the transmit buffer */
    MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_CLEAR_TRANSMIT_BUFFER,
    /** substate for the wake-up routine, clear the receive buffer */
    MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_CLEAR_RECEIVE_BUFFER_2,
    /** substate for the wake-up routine, sending helloall to the daisy-chain */
    MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_HELLOALL,
    /** substate for the wake-up routine, verifying the result of helloall */
    MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_HELLOALL_VERIFY_MSG_AND_COUNT,
    /** substate for requesting an FMEA check on the bridge IC */
    MXM_5X_41B_FMEA_REQUEST,
    /** substate for the verification of the FMEA check done in
     *  #MXM_5X_41B_FMEA_REQUEST */
    MXM_5X_41B_FMEA_CHECK,
    /** substate for the initiation of a uart transaction for the WRITEALL
     *  command */
    MXM_5X_WRITEALL_UART_TRANSACTION,
    /** substate for the verifying the buffer of a WRITEALL transaction */
    MXM_5X_WRITEALL_UART_VERIFY_BUFFER,
    /** substate for the initiation of a uart transaction for the WRITEALL
     *  command */
    MXM_5X_WRITE_DEVICE_UART_TRANSACTION,
    /** substate for checking the received CRC in a WRITEDEVICE transaction */
    MXM_5X_WRITE_DEVICE_CHECK_CRC,
    /** substate for the initiation of a uart transaction for the READALL
     *  command */
    MXM_5X_READALL_UART_TRANSACTION,
    /** substate for checking the received CRC in a READALL transaction */
    MXM_5X_READALL_CHECK_CRC,
    /** substate for retrieving the data check byte from a READALL transaction */
    MXM_5X_READALL_GET_DC,
    /** default value indicating no selected substate; has to be last substate in enum */
    MXM_5X_ENTRY_SUBSTATE,
} MXM_5X_SUBSTATES_e;

/**
 * @brief   Request status of Battery Management Protocol states.
 * @details This enum describes the states that a state in the
 *          #MXM_5XStateMachine() can have.
 *
 *          Alternating bit patterns are used for better distinction
 *          and to make sure that there is no accidental overlap.
 */
typedef enum {
    MXM_5X_STATE_UNSENT = 0x56, /*!< The request has not been sent to the state-machine yet. */
    MXM_5X_STATE_UNPROCESSED =
        0x67, /*!< The request has been received by the state-machine, but not been processed yet. */
    MXM_5X_STATE_PROCESSED = 0x9A, /*!< The request has been process successfully */
    MXM_5X_STATE_ERROR     = 0xAB, /*!< An error has occurred during processing of the request. */
} MXM_5X_STATE_REQUEST_STATUS_e;

/**
 * @brief Payload command
 */
typedef struct {
    MXM_REG_NAME_e regAddress; /*!< register address that shall be written */
    uint8_t lsb;               /*!< least significant bit */
    uint8_t msb;               /*!< most significant bit */
    uint8_t blocksize;         /*!< blocksize for the READBLOCK command */
    uint8_t deviceAddress;     /*!< device address for commands that address specific devices */
    MXM_MODEL_ID_e model;      /*!< model of device that shall be addressed */
} MXM_5X_COMMAND_PAYLOAD_s;

/**
 * @brief 5x statemachine structure
 **/
typedef struct {
    MXM_STATEMACHINE_5X_e state;              /*!< state of Driver State Machine */
    MXM_5X_SUBSTATES_e substate;              /*!< substate of current Driver State */
    MXM_5X_COMMAND_PAYLOAD_s commandPayload;  /*!< command payload of the Battery Management Protocol */
    MXM_5X_STATE_REQUEST_STATUS_e *processed; /*!< status-indicator of the underlying state-machine */
    MXM_41B_STATE_REQUEST_STATUS_e status41b; /*!< for tracking of 41B state-machine */
    /**
     * @brief   Number of satellites
     * @details This variable tracks the number of monitoring ICs that have been
     *          found during the enumeration of all connected monitoring ICs.
     *          Therefore this variable is initialized with 0. This value will be
     *          compared with the expected number of monitoring ICs which is set in
     *          #BS_NR_OF_MODULES.
     */
    uint8_t numberOfSatellites;
    /**
     * @brief   Number of monitoring ICs matches the expected number.
     * @details This status variable indicates whether the number of found
     *          monitoring ICs in #MXM_5X_INSTANCE_s::numberOfSatellites
     *          matches with the configured number of monitoring ICs in
     *          #BS_NR_OF_MODULES. This variable is updated during execution of
     *          the substate
     *          #MXM_5X_INIT_WAKE_UP_SATELLITE_DEVICES_HELLOALL_VERIFY_MSG_AND_COUNT
     *          in the state #MXM_STATEMACH_5X_INIT in #MXM_5XStateMachine().
     *
     *          In case the numbers are the same, the variable will read #STD_OK,
     *          otherwise it will be set to #STD_NOT_OK.
     */
    STD_RETURN_TYPE_e numberOfSatellitesIsGood;
    /**
     * @brief   Tracks the last received DC byte.
     */
    uint8_t lastDCByte;
    uint8_t errorCounter;        /*!< counts the number of errors that the underlying state-machine has encountered. */
    uint32_t resetWaitTimestamp; /*!< timestamp of the moment when waiting for the devices to be reset has started */
    /**
     * @brief   Length of Command Buffer
     * @details Length of the array #MXM_5X_INSTANCE_s::commandBuffer.
     */
    uint8_t commandBufferCurrentLength;
    /**
     * @brief   Command Buffer
     * @details This variable contains a buffer for Battery Management Protocol
     *          commands. The content is constructed by calling functions like
     *          #MXM_5XConstructCommandBufferHelloall() and similar.
     *
     *          Afterwards this buffer can be passed on to the lower state-machine
     *          as payload. The length of this buffer is described in
     *          #MXM_5X_INSTANCE_s::commandBufferCurrentLength.
     */
    uint16_t commandBuffer[COMMAND_BUFFER_LENGTH];
    uint16_t rxBuffer[MXM_5X_RX_BUFFER_LEN]; /*!< array containing the buffer for received data */
} MXM_5X_INSTANCE_s;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   Execute state-machine for Battery Management Protocol.
 * @details This function executes the state-machine that exposes an interface
 *          for the Maxim Battery Management Protocol for monitoring ICs
 *          of the MAX1785x family.
 */
extern void MXM_5XStateMachine(MXM_41B_INSTANCE_s *pInstance41b, MXM_5X_INSTANCE_s *pInstance5x);

/**
 * @brief Returns the last received DC byte.
 * @param[in]   kpkInstance pointer to the instance struct
 */
extern MXM_DC_BYTE_e MXM_5XGetLastDCByte(const MXM_5X_INSTANCE_s *const kpkInstance);

/**
 * @brief Get the value of #MXM_5X_INSTANCE_s::numberOfSatellitesIsGood
 * @param[in]   kpkInstance pointer to the instance struct
 *
 * @return #MXM_5X_INSTANCE_s::numberOfSatellitesIsGood
 */
extern STD_RETURN_TYPE_e MXM_5XGetNumberOfSatellitesGood(const MXM_5X_INSTANCE_s *const kpkInstance);

/**
 * @brief   Copy RX buffer into variable.
 * @details This function copies the RX buffer which is locally available in
 *          the #MXM_5XStateMachine() into a supplied array pointer.
 *          It will make sure that both the length of the local as also the
 *          supplied length of the RX buffer are not violated.
 *          In case of the supplied RX buffer being longer than the locally
 *          available one, the remaining entries of the buffer will be
 *          filled with 0.
 * @param[in,out]   kpkInstance pointer to the state-struct
 * @param[in]       rxBuffer    array-pointer to a RX buffer that shall be filled
 * @param[in]       rxBufferLength   length of the supplied array
 * @return      #STD_NOT_OK for rxBuffer being a #NULL_PTR or rxBufferLength
 *              having length 0
 */
extern STD_RETURN_TYPE_e MXM_5XGetRXBuffer(
    const MXM_5X_INSTANCE_s *const kpkInstance,
    uint8_t *rxBuffer,
    uint16_t rxBufferLength);

/**
 * @brief   Get number of satellites
 * @details Getter-function for the number of satellites
 *          (variable #MXM_5X_INSTANCE_s::numberOfSatellites).
 * @param[in]   kpkInstance pointer to the state struct
 * @return  value of #MXM_5X_INSTANCE_s::numberOfSatellites
 */
extern uint8_t MXM_5XGetNumberOfSatellites(const MXM_5X_INSTANCE_s *const kpkInstance);

/**
 * @brief   Set state request for the Battery Management Statemachine
 * @details This function sets the state requests for the #MXM_5XStateMachine().
 * @param[in,out]   pInstance5x      pointer to the 5x state
 * @param[in]       state       State that is requested
 * @param[in]       commandPayload additional payload that is handled by the state
 * @param[in,out]   processed   pointer to the status of the request
 * @return          #STD_OK if the request has been successfully placed,
 *                  #STD_NOT_OK if not
 */
extern STD_RETURN_TYPE_e MXM_5XSetStateRequest(
    MXM_5X_INSTANCE_s *pInstance5x,
    MXM_STATEMACHINE_5X_e state,
    MXM_5X_COMMAND_PAYLOAD_s commandPayload,
    MXM_5X_STATE_REQUEST_STATUS_e *processed);

/**
 * @brief   runs a selfcheck for the address space check
 * @details Runs a selfcheck for the function which is checking if a register
 *          address is inside the user accessible address space of the
 *          monitoring IC.
 * @return  #STD_OK if the selfcheck has been successful
 */
extern STD_RETURN_TYPE_e must_check_return MXM_5XUserAccessibleAddressSpaceCheckerSelfCheck(void);

/**
 * @brief   Initializes the state struct with default values
 * @details This function is called through the startup of the driver in order
 *          to ensure proper default values.
 * @param[out]  pInstance   instance of the state struct that shall be initialized
 */
extern void MXM_5X_InitializeStateStruct(MXM_5X_INSTANCE_s *pInstance);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__MXM_BATTERY_MANAGEMENT_H_ */
