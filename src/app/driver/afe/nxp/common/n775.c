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
 * @file    n775.c
 * @author  foxBMS Team
 * @date    2020-05-08 (date of creation)
 * @updated 2021-06-09 (date of last update)
 * @ingroup DRIVERS
 * @prefix  N775
 *
 * @brief   Driver for the MC33775A monitoring chip.
 *
 */

/*========== Includes =======================================================*/
#include "n775.h"

#include "HL_system.h"

#include "MC33775A.h"
#include "afe_dma.h"
#include "database.h"
#include "diag.h"
#include "os.h"

/*========== Macros and Definitions =========================================*/

/** maximum number of supported cells per slave-module */
#define N775_MAX_SUPPORTED_CELLS (12u)

/**
 * Saves the last state and the last substate
 */
#define N775_SAVELASTSTATES()                   \
    n775_state.lastState    = n775_state.state; \
    n775_state.lastSubState = n775_state.subState

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/
#pragma SET_DATA_SECTION(".sharedRAM")
uint16_t n775_RXbuffer[N775_MAX_N_BYTES_FOR_DATA_RECEPTION] = {0};
uint16_t n775_TXbuffer[N775_TX_MESSAGE_LENGTH]              = {0};
#pragma SET_DATA_SECTION()

N775_MESSAGE_s n775_sentData     = {0};
N775_MESSAGE_s n775_receivedData = {0};

uint8_t n775_enumerateAddress = 1u;

/* static int16_t n775_voltage_min = INT16_MAX;
static int16_t n775_voltage_max = INT16_MIN;
static int32_t n775_voltage_sum = 0;

static uint8_t n775_voltage_module_number_min = 0;
static uint8_t n775_voltage_module_number_max = 0;
static uint8_t n775_voltage_cell_number_min = 0;
static uint8_t n775_voltage_cell_number_max = 0;
static uint16_t n775_voltage_nrValidCellVoltages = 0; */

/** local copies of database tables */
/**@{*/
static DATA_BLOCK_CELL_VOLTAGE_s n775_cellVoltage           = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_TEMPERATURE_s n775_cellTemperature   = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
static DATA_BLOCK_MIN_MAX_s n775_minMax                     = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_BALANCING_CONTROL_s n775_balancingControl = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};
/**@}*/

N775_STATE_s n775_state = {
    .timer                = 0,
    .stateReq             = N775_STATE_NO_REQUEST,
    .state                = N775_STATEMACH_UNINITIALIZED,
    .subState             = 0,
    .lastState            = N775_STATEMACH_UNINITIALIZED,
    .lastSubState         = 0,
    .errRequestCounter    = 0,
    .triggerEntry         = 0,
    .firstMeasurementMade = STD_NOT_OK,
    .checkSpiFlag         = STD_NOT_OK,
    .balanceControlDone   = STD_NOT_OK,
    .txTransmitOngoing    = false,
    .rxTransmitOngoing    = false,
    .totalMessages        = 0u,
    .remainingMessages    = 0u,
};

/*========== Static Function Prototypes =====================================*/
static void N775_SetFirstMeasurementCycleFinished(N775_STATE_s *n775_state);
static void N775_CopyStructToTxBuffer(N775_MESSAGE_s *message, uint16_t *buffer);
static void N775_WakeUp(uint16_t daisyChainAddress, uint16_t deviceAddress, uint16_t registerAddress);
static void N775_Write(uint16_t daisyChainAddress, uint16_t deviceAddress, uint16_t registerAddress, uint16_t data);
static void N775_Read(
    uint16_t daisyChainAddress,
    uint16_t deviceAddress,
    uint16_t registerAddress,
    uint16_t totalNumberOfRequestedRegister);
static void N775_Initialize_Database(void);
/* static void N775_Get_BalancingControlValues(void); */
static void N775_StateTransition(N775_STATEMACH_e state, uint8_t substate, uint16_t timer_ms);
/* static void N775_CondBasedStateTransition(STD_RETURN_TYPE_e retVal, DIAG_ID_e diagCode,
                                         N775_STATEMACH_e state_ok, uint8_t substate_ok, uint16_t timer_ms_ok,
                                         N775_STATEMACH_e state_nok,  uint8_t substate_nok,  uint16_t timer_ms_nok); */

uint16_t n775_CrcAddItem(uint16_t remainder, uint16_t item);

static N775_RETURN_TYPE_e N775_CheckStateRequest(N775_STATE_REQUEST_e statereq);
uint16_t n775_CalcCrc(const N775_MESSAGE_s *msg);

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

/* TODO: use own function */
/**
 * @brief   Called to calculate the CRC of a message. NXP function.
 *
 * @param   remainder
 * @param   item
 *
 * @return  remainder
 *
 */
uint16_t n775_CrcAddItem(uint16_t remainder, uint16_t item) {
    int i;
    uint16_t local_remainder = remainder;

    local_remainder ^= item;

    for (i = 0; i < 16; i++) {
        /*
         * Try to divide the current data bit.
         */
        if ((local_remainder & 0x8000u) > 0u) {
            local_remainder = (local_remainder << 1u) ^ ((0x9eb2u << 1u) + 0x1u);
        } else {
            local_remainder = (local_remainder << 1u);
        }
    }

    return (local_remainder);
}

/* TODO: use own function */
/**
 * @brief   Calculate the CRC of a message. NXP function.
 *
 * @param   msg
 *
 * @return  crc
 *
 */
uint16_t n775_CalcCrc(const N775_MESSAGE_s *msg) {
    uint16_t remainder = 0;
    int i;

    remainder = n775_CrcAddItem(remainder, msg->head);
    remainder = n775_CrcAddItem(remainder, msg->dataHead);

    for (i = 0; i < (msg->dataLength - 3); i++) {
        remainder = n775_CrcAddItem(remainder, msg->data[i]);
    }

    return (remainder);
}

/**
 * @brief   in the database, initializes the fields related to the N775 driver.
 *
 * This function loops through all the N775-related data fields in the database
 * and sets them to 0. It should be called in the initialization or re-initialization
 * routine of the N775 driver.
 */
static void N775_Initialize_Database(void) {
    uint16_t i = 0;

    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        n775_cellVoltage.state                               = 0;
        n775_minMax.minimumCellVoltage_mV[stringNumber]      = 0;
        n775_minMax.maximumCellVoltage_mV[stringNumber]      = 0;
        n775_minMax.nrModuleMinimumCellVoltage[stringNumber] = 0;
        n775_minMax.nrModuleMaximumCellVoltage[stringNumber] = 0;
        n775_minMax.nrCellMinimumCellVoltage[stringNumber]   = 0;
        n775_minMax.nrCellMaximumCellVoltage[stringNumber]   = 0;
        for (i = 0; i < BS_NR_OF_BAT_CELLS; i++) {
            n775_cellVoltage.cellVoltage_mV[stringNumber][i] = 0;
        }

        n775_cellTemperature.state                           = 0;
        n775_minMax.minimumTemperature_ddegC[stringNumber]   = 0;
        n775_minMax.maximumTemperature_ddegC[stringNumber]   = 0;
        n775_minMax.nrModuleMinimumTemperature[stringNumber] = 0;
        n775_minMax.nrModuleMaximumTemperature[stringNumber] = 0;
        n775_minMax.nrSensorMinimumTemperature[stringNumber] = 0;
        n775_minMax.nrSensorMaximumTemperature[stringNumber] = 0;
        for (i = 0; i < BS_NR_OF_TEMP_SENSORS_PER_STRING; i++) {
            n775_cellTemperature.cellTemperature_ddegC[stringNumber][i] = 0;
        }

        for (i = 0; i < BS_NR_OF_BAT_CELLS; i++) {
            n775_balancingControl.balancingState[stringNumber][i] = 0;
        }
    }

    DATA_WRITE_DATA(&n775_cellVoltage);
    DATA_WRITE_DATA(&n775_cellTemperature);
    DATA_WRITE_DATA(&n775_minMax);
    DATA_WRITE_DATA(&n775_balancingControl);
}

/**
 * @brief   function for setting N775_Trigger state transitions
 *
 * @param  state:    state to transition into
 * @param  subState: substate to transition into
 * @param  timer_ms: transition into state, substate after timer elapsed
 */
static void N775_StateTransition(N775_STATEMACH_e state, uint8_t subState, uint16_t timer_ms) {
    n775_state.state    = state;
    n775_state.subState = subState;
    n775_state.timer    = timer_ms;
}

/**
 * @brief   re-entrance check of N775 state machine trigger function
 *
 * This function is not re-entrant and should only be called time- or event-triggered.
 * It increments the triggerentry counter from the state variable n775_state.
 * It should never be called by two different processes, so if it is the case, triggerentry
 * should never be higher than 0 when this function is called.
 *
 *
 * @return  retval  0 if no further instance of the function is active, 0xff else
 *
 */
uint8_t N775_CheckReEntrance(void) {
    uint8_t retval = 0;

    OS_EnterTaskCritical();
    if (!n775_state.triggerEntry) {
        n775_state.triggerEntry++;
    } else {
        retval = 0xFF; /* multiple calls of function */
    }
    OS_ExitTaskCritical();

    return (retval);
}
uint8_t TEST_N775_CheckReEntrance(void) {
    return N775_CheckReEntrance();
}

/**
 * @brief   gets the current state request.
 *
 * This function is used in the functioning of the N775 state machine.
 *
 * @return  retval  current state request, taken from N775_STATE_REQUEST_e
 */
extern N775_STATE_REQUEST_e N775_GetStateRequest(void) {
    N775_STATE_REQUEST_e retval = N775_STATE_NO_REQUEST;

    OS_EnterTaskCritical();
    retval = n775_state.stateReq;
    OS_ExitTaskCritical();

    return (retval);
}

/**
 * @brief   gets the current state.
 *
 * This function is used in the functioning of the N775 state machine.
 *
 * @return  current state, taken from N775_STATEMACH_e
 */
extern N775_STATEMACH_e N775_GetState(void) {
    return (n775_state.state);
}

/**
 * @brief   transfers the current state request to the state machine.
 *
 * This function takes the current state request from n775_state and transfers it to the state machine.
 * It resets the value from n775_state to N775_STATE_NO_REQUEST
 *
 * @return  retVal          current state request, taken from N775_STATE_REQUEST_e
 *
 */
N775_STATE_REQUEST_e N775_TransferStateRequest() {
    N775_STATE_REQUEST_e retval = N775_STATE_NO_REQUEST;

    OS_EnterTaskCritical();
    retval              = n775_state.stateReq;
    n775_state.stateReq = N775_STATE_NO_REQUEST;
    OS_ExitTaskCritical();

    return (retval);
}

N775_RETURN_TYPE_e N775_SetStateRequest(N775_STATE_REQUEST_e statereq) {
    N775_RETURN_TYPE_e retVal = N775_ERROR;

    OS_EnterTaskCritical();
    retVal = N775_CheckStateRequest(statereq);

    if ((retVal == N775_OK) || (retVal == N775_BUSY_OK) || (retVal == N775_OK_FROM_ERROR)) {
        n775_state.stateReq = statereq;
    }
    OS_ExitTaskCritical();

    return (retVal);
}

void N775_Trigger(void) {
    N775_STATE_REQUEST_e n775_stateReq = N775_STATE_NO_REQUEST;
    bool n775_goToTrigger              = true;

    N775_Write(0, 0, 0, 0); /* TODO: just to compile, remove */
    N775_Read(0, 0, 0, 0);  /* TODO: just to compile, remove */

    /* Check re-entrance of function */
    if (N775_CheckReEntrance() > 0u) {
        n775_goToTrigger = false;
    }

    if (n775_state.checkSpiFlag == STD_NOT_OK) {
        if (n775_state.timer > 0u) {
            if ((--n775_state.timer) > 0u) {
                n775_state.triggerEntry--;
                n775_goToTrigger = false;
                ; /* handle state machine only if timer has elapsed */
            }
        }
    } else {
        if (AFE_IsTxTransmitOngoing() == true) {
            if (n775_state.timer > 0u) {
                if ((--n775_state.timer) > 0u) {
                    n775_state.triggerEntry--;
                    n775_goToTrigger = false;
                    ; /* handle state machine only if timer has elapsed */
                }
            }
        }
        if (AFE_IsRxTransmitOngoing() == true) {
            if (n775_state.timer > 0u) {
                if ((--n775_state.timer) > 0u) {
                    n775_state.triggerEntry--;
                    n775_goToTrigger = false;
                    ; /* handle state machine only if timer has elapsed */
                }
            }
        }
    }

    if (n775_goToTrigger == true) {
        switch (n775_state.state) {
            /****************************UNINITIALIZED***********************************/
            case N775_STATEMACH_UNINITIALIZED:
                /* waiting for Initialization Request */
                n775_stateReq = N775_TransferStateRequest();
                if (n775_stateReq == N775_STATE_INIT_REQUEST) {
                    N775_SAVELASTSTATES();
                    N775_StateTransition(
                        N775_STATEMACH_INITIALIZATION, N775_ENTRY_UNINITIALIZED, N775_STATEMACH_SHORTTIME);
                } else if (n775_stateReq == N775_STATE_NO_REQUEST) {
                    /* no actual request pending */
                } else {
                    n775_state.errRequestCounter++; /* illegal request pending */
                }
                break;

            /****************************INITIALIZATION**********************************/
            case N775_STATEMACH_INITIALIZATION:
                N775_Initialize_Database();
                N775_StateTransition(N775_STATEMACH_INITIALIZED, N775_ENTRY, N775_STATEMACH_SHORTTIME);
                break;

            /****************************INITIALIZED*************************************/
            case N775_STATEMACH_INITIALIZED:
                N775_SAVELASTSTATES();
                N775_StateTransition(N775_STATEMACH_WAKEUP, N775_ENTRY, N775_STATEMACH_SHORTTIME);
                break;

            /****************************START MEASUREMENT*******************************/
            case N775_STATEMACH_WAKEUP:
                if (n775_state.subState == N775_ENTRY) {
                    N775_SAVELASTSTATES();
                    /* Send first wake-up command */
                    N775_WakeUp(0u, 0u, 0u);
                    N775_StateTransition(N775_STATEMACH_WAKEUP, N775_SECOND_WAKEUP, N775_TWAKECOM_MS);
                } else if (n775_state.subState == N775_SECOND_WAKEUP) {
                    N775_SAVELASTSTATES();
                    /* Send second wake-up command */
                    N775_WakeUp(0u, 0u, 0u);
                    N775_StateTransition(N775_STATEMACH_ENUMERATE, N775_ENTRY, N775_TIME_DAISYCHAIN_WAKEUP_MS);
                }
                break;

            /****************************START MEASUREMENT*******************************/
            case N775_STATEMACH_ENUMERATE:
                if (n775_state.subState == N775_ENTRY) {
                    N775_SAVELASTSTATES();
                    /* Enumerate first slave; first address is 1 and not 0; 0 means unenumerated */
                    N775_Write(
                        0u,
                        n775_enumerateAddress,
                        MC33775_SYS_COM_CFG_OFFSET,
                        (n775_enumerateAddress << MC33775_SYS_COM_CFG_DADD_POS) |
                            (MC33775_SYS_COM_CFG_BUSFW_ENABLED_ENUM_VAL << MC33775_SYS_COM_CFG_BUSFW_POS));
                    N775_StateTransition(
                        N775_STATEMACH_ENUMERATE, N775_CHECK_ENUMERATION, N775_TIME_AFTER_ENUMERATION_MS);
                    n775_enumerateAddress++;
                } else if (n775_state.subState == N775_CHECK_ENUMERATION) {
                    if (n775_enumerateAddress <= N775_N_N775) {
                        /* Enumerate next slave */
                        N775_Write(
                            0u,
                            n775_enumerateAddress,
                            MC33775_SYS_COM_CFG_OFFSET,
                            (n775_enumerateAddress << MC33775_SYS_COM_CFG_DADD_POS) |
                                (MC33775_SYS_COM_CFG_BUSFW_ENABLED_ENUM_VAL << MC33775_SYS_COM_CFG_BUSFW_POS));
                        N775_StateTransition(
                            N775_STATEMACH_ENUMERATE, N775_CHECK_ENUMERATION, N775_TIME_AFTER_ENUMERATION_MS);
                        n775_enumerateAddress++;
                    } else {
                        /* All slave enumerated */
                        N775_StateTransition(N775_STATEMACH_STARTMEAS, N775_ENTRY, N775_STATEMACH_SHORTTIME);
                    }
                }
                break;

            /****************************START MEASUREMENT*******************************/
            case N775_STATEMACH_STARTMEAS:

                break;

            /****************************READ VOLTAGE************************************/
            case N775_STATEMACH_READVOLTAGE:

                break;

            /****************************BALANCE CONTROL*********************************/
            case N775_STATEMACH_BALANCECONTROL:

                break;

            /****************************DEFAULT**************************/
            default:
                /* invalid state */
                FAS_ASSERT(FAS_TRAP);
                break;
        }
    }

    n775_state.triggerEntry--; /* reentrance counter */
}

/**
 * @brief   checks the state requests that are made.
 *
 * This function checks the validity of the state requests.
 * The results of the checked is returned immediately.
 *
 * @param   statereq    state request to be checked
 *
 * @return              result of the state request that was made, taken from N775_RETURN_TYPE_e
 */
static N775_RETURN_TYPE_e N775_CheckStateRequest(N775_STATE_REQUEST_e statereq) {
    N775_RETURN_TYPE_e retVal = N775_OK; /* Default: request is OK */

    if (n775_state.stateReq == N775_STATE_NO_REQUEST) {
        /* init only allowed from the uninitialized state */
        if (statereq == N775_STATE_INIT_REQUEST) {
            if (n775_state.state == N775_STATEMACH_UNINITIALIZED) {
                retVal = N775_OK;
            } else {
                retVal = N775_ALREADY_INITIALIZED;
            }
        }

    } else {
        retVal = N775_REQUEST_PENDING;
    }
    return retVal;
}

/**
 * @brief   gets the measurement initialization status.
 *
 * @return  retval  true if a first measurement cycle was made, false otherwise
 *
 */
extern bool N775_IsFirstMeasurementCycleFinished(void) {
    bool retval = false;

    OS_EnterTaskCritical();
    retval = n775_state.firstMeasurementMade;
    OS_ExitTaskCritical();

    return (retval);
}

/**
 * @brief   sets the measurement initialization status.
 */
static void N775_SetFirstMeasurementCycleFinished(N775_STATE_s *n775_state) {
    OS_EnterTaskCritical();
    n775_state->firstMeasurementMade = STD_OK;
    OS_ExitTaskCritical();
}

extern void TEST_N775_SetFirstMeasurementCycleFinished(N775_STATE_s *n775_state) {
    N775_SetFirstMeasurementCycleFinished(n775_state);
}

/**
 * @brief   sends a write command to the daisy-chain.
 * @param   message message to be sent
 * @param   buffer  buffer used for SPI
 */
static void N775_CopyStructToTxBuffer(N775_MESSAGE_s *message, uint16_t *buffer) {
    uint8_t i = 0;

    buffer[0] = message->head;
    buffer[1] = message->dataHead;
    if (((message->dataLength) >= 1u) && ((message->dataLength) <= 4u)) {
        for (i = 0; i < (message->dataLength); i++) {
            buffer[i + 2u] = message->data[i];
        }
        buffer[i + 3u] = message->crc;
    } else {
        /* this should not happen, stay here */
        FAS_ASSERT(FAS_TRAP);
    }
}

/**
 * @brief   sends a wake-up command to the daisy-chain.
 *
 * @param   daisyChainAddress   parameter CADD in the message format
 * @param   deviceAddress       parameter DADD in the message format lies
 *                              between 1 and 62, 63 means all devices
 * @param   registerAddress     address of register to be written to
 */
static void N775_WakeUp(uint16_t daisyChainAddress, uint16_t deviceAddress, uint16_t registerAddress) {
    uint16_t messageCounter = 0u;

    /**
     * Set Head part or WRITE message
     */
    /* Set command field of WRITE message */
    n775_sentData.head = ((N775_CMD_WAKEUP_NOP & 0x3u) << 14u) | ((n775_sentData.head) & 0x3FFFu);
    /* Set MADD field of WRITE message */
    n775_sentData.head = ((N775_MADD_MASTER0 & 0x1u) << 13u) | ((n775_sentData.head) & 0xDFFFu);
    /* Set CADD field of WRITE message */
    n775_sentData.head = ((daisyChainAddress & 0x7u) << 10u) | ((n775_sentData.head) & 0xE3FFu);
    /* Set DADD field of WRITE message; device addresses start at 1 */
    n775_sentData.head = ((deviceAddress & 0x3Fu) << 4u) | ((n775_sentData.head) & 0xFC0Fu);
    /* Set message counter; not used for WRITE messages, set to 0 */
    (n775_sentData.head) = (messageCounter & 0xFu) | ((n775_sentData.head) & 0xFFF0u);

    /**
     * Set Data head part or WRITE message
     */
    /* Set DATLEN field of WRITE message */
    /* Number of registers addressed per message: 1, which is coded by DATLEN as 0 in the frame */
    n775_sentData.dataHead = ((0u & 0x3u) << 14u) | ((n775_sentData.dataHead) & 0x3FFFu);
    /* Set REGADD field of WRITE message (address of register to be addressed)) */
    n775_sentData.dataHead = (registerAddress & 0x3FFFu) | ((n775_sentData.dataHead) & 0xC000u);

    /**
     * Set Data part or WRITE message
     */
    /* Set data fields; only first field is used. */
    /* Set all data fields to 0. */
    n775_sentData.data[0] = 0u;
    n775_sentData.data[1] = 0u;
    n775_sentData.data[2] = 0u;
    n775_sentData.data[3] = 0u;
    /* Set data length, used internally in the strcut */
    n775_sentData.dataLength = 4u;

    /**
     * Set CRC part or WRITE message
     */
    n775_sentData.crc = n775_CalcCrc(&n775_sentData);

    /* Copy strcut data to SPI buffer */
    N775_CopyStructToTxBuffer(&n775_sentData, n775_TXbuffer);

    /* Send WRITE command to daisy-chain */
    N775_SendData(n775_TXbuffer, n775_RXbuffer, N775_TX_MESSAGE_LENGTH);
}

/**
 * @brief   sends a write command to the daisy-chain.
 *
 * @param   daisyChainAddress   parameter CADD in the message format
 * @param   deviceAddress       parameter DADD in the message format lies
 *                              between 1 and 62, 63 means all devices
 * @param   registerAddress     address of register to be written to
 * @param   data                data to be written in the device register
 */
static void N775_Write(uint16_t daisyChainAddress, uint16_t deviceAddress, uint16_t registerAddress, uint16_t data) {
    uint16_t messageCounter = 0u;

    /**
     * Set Head part or WRITE message
     */
    /* Set command field of WRITE message */
    n775_sentData.head = ((N775_CMD_WRITE & 0x3u) << 14u) | ((n775_sentData.head) & 0x3FFFu);
    /* Set MADD field of WRITE message */
    n775_sentData.head = ((N775_MADD_MASTER0 & 0x1u) << 13u) | ((n775_sentData.head) & 0xDFFFu);
    /* Set CADD field of WRITE message */
    n775_sentData.head = ((daisyChainAddress & 0x7u) << 10u) | ((n775_sentData.head) & 0xE3FFu);
    /* Set DADD field of WRITE message; device addresses start at 1 */
    n775_sentData.head = ((deviceAddress & 0x3Fu) << 4u) | ((n775_sentData.head) & 0xFC0Fu);
    /* Set message counter; not used for WRITE messages, set to 0 */
    (n775_sentData.head) = (messageCounter & 0xFu) | ((n775_sentData.head) & 0xFFF0u);

    /**
     * Set Data head part or WRITE message
     */
    /* Set DATLEN field of WRITE message */
    /* Number of registers addressed per message: 1, which is coded by DATLEN as 0 in the frame */
    n775_sentData.dataHead = ((0u & 0x3u) << 14u) | ((n775_sentData.dataHead) & 0x3FFFu);
    /* Set REGADD field of WRITE message (address of register to be addressed)) */
    n775_sentData.dataHead = (registerAddress & 0x3FFFu) | ((n775_sentData.dataHead) & 0xC000u);

    /**
     * Set Data part or WRITE message
     */
    /* Set data fields; only first field is used. */
    n775_sentData.data[0] = data;
    /* Set other data fields to 0. */
    n775_sentData.data[1] = 0u;
    n775_sentData.data[2] = 0u;
    n775_sentData.data[3] = 0u;
    /* Set data length, used internally in the strcut */
    n775_sentData.dataLength = 4u;

    /**
     * Set CRC part or WRITE message
     */
    n775_sentData.crc = n775_CalcCrc(&n775_sentData);

    /* Copy strcut data to SPI buffer */
    N775_CopyStructToTxBuffer(&n775_sentData, n775_TXbuffer);

    /* Send WRITE command to daisy-chain */
    N775_SendData(n775_TXbuffer, n775_RXbuffer, N775_TX_MESSAGE_LENGTH);
}

/**
 * @brief   sends a read command to the daisy-chain.
 * @param   daisyChainAddress               parameter CADD in the message
 *                                          format
 * @param   deviceAddress                   parameter DADD in the message
 *                                          format lies between 1 and 62
 * @param   registerAddress                 address of first consecutive
 *                                          register to be read from
 * @param   totalNumberOfRequestedRegister  total number of registers values
 *                                          sent by daisy-chain lies between 1
 *                                          and 256
 */
static void N775_Read(
    uint16_t daisyChainAddress,
    uint16_t deviceAddress,
    uint16_t registerAddress,
    uint16_t totalNumberOfRequestedRegister) {
    uint16_t messageCounter = 0u;
    uint16_t dataLen        = 0u;
    uint16_t readParameters = 0u;

    /**
     * Set Head part or WRITE message
     */
    /* Set command field of WRITE message */
    n775_sentData.head = ((N775_CMD_READ & 0x3u) << 14u) | ((n775_sentData.head) & 0x3FFFu);
    /* Set MADD field of WRITE message */
    n775_sentData.head = ((N775_MADD_MASTER0 & 0x1u) << 13u) | ((n775_sentData.head) & 0xDFFFu);
    /* Set CADD field of WRITE message */
    n775_sentData.head = ((daisyChainAddress & 0x7u) << 10u) | ((n775_sentData.head) & 0xE3FFu);
    /* Set DADD field of WRITE message; device addresses start at 1 */
    n775_sentData.head = ((deviceAddress & 0x3Fu) << 4u) | ((n775_sentData.head) & 0xFC0Fu);
    /* Set message counter; not used for READ messages, set to 0 */
    (n775_sentData.head) = (messageCounter & 0xFu) | ((n775_sentData.head) & 0xFFF0u);

    /**
     * Set Data head part or WRITE message
     */
    /* Set DATALEN field of WRITE message; number of registers addressed per mssage: 1 */
    n775_sentData.dataHead = ((dataLen & 0x3u) << 14u) | ((n775_sentData.dataHead) & 0x3FFFu);
    /* Set REGADD field of WRITE message (address of register t be addressed)) */
    n775_sentData.dataHead = (registerAddress & 0x3FFFu) | ((n775_sentData.dataHead) & 0xC000u);

    /**
     * Set Data part or WRITE message
     */
    readParameters = (0u << 11u) +                                 /* First five bits must be written with 0 */
                     (0u << 10u) +                                 /* PAD = 0: no padding */
                     (0u << 8u) +                                  /* RESPLEN = 0: oneregister per frame */
                     ((totalNumberOfRequestedRegister - 1u) << 0); /* NUMREG: number of consecutive registers to read */
                                                                   /* 0 means 1 register, therefore 1u is subtracted */
    /* Set data field; only first of the 4 fields is used */
    n775_sentData.data[0] = readParameters;
    /* Set other data fields to 0. */
    n775_sentData.data[1] = 0u;
    n775_sentData.data[2] = 0u;
    n775_sentData.data[3] = 0u;
    /* Set data length, used internally in the struct */
    n775_sentData.dataLength = 4u;

    /**
     * Set CRC part or WRITE message
     */
    n775_sentData.crc = n775_CalcCrc(&n775_sentData);

    /* Copy strcut data to SPI buffer */
    N775_CopyStructToTxBuffer(&n775_sentData, n775_TXbuffer);

    n775_state.totalMessages     = totalNumberOfRequestedRegister;
    n775_state.remainingMessages = totalNumberOfRequestedRegister;

    /* Send WRITE command to daisy-chain */
    N775_SendData(n775_TXbuffer, n775_RXbuffer, N775_TX_MESSAGE_LENGTH);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
