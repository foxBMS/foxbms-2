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
 * @file    can.c
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2021-03-29 (date of last update)
 * @ingroup DRIVERS
 * @prefix  CAN
 *
 * @brief   Driver for the CAN module
 *
 * @details Implementation of the CAN Interrupts, initialization, buffers,
 *          receive and transmit interfaces.
 */

/*========== Includes =======================================================*/
#include "can.h"

#include "HL_het.h"

#include "bender_iso165c.h"
#include "database.h"
#include "diag.h"
#include "io.h"
#include "mcu.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/** tracks the local state of the can module */
static CAN_STATE_s can_state = {
    .periodicEnable         = false,
    .currentSensorPresent   = false,
    .currentSensorCCPresent = false,
    .currentSensorECPresent = false,
};

/** local buffer for receiving data from CAN */
static CAN_BUFFERELEMENT_s can_rxBufferData[CAN0_RX_BUFFER_LENGTH];

/** interface object for #can_rxBufferData */
static CAN_RX_BUFFER_s can_rxBuffer = {
    .pRead  = &can_rxBufferData[0],
    .pWrite = &can_rxBufferData[0],
    .length = CAN0_RX_BUFFER_LENGTH,
};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   Called in case of CAN TX interrupt.
 * @param   pNode        CAN interface on which message was sent
 * @param   messageBox   message box on which message was sent
 */
static void CAN_TxInterrupt(canBASE_t *pNode, uint32 messageBox);

/**
 * @brief   Called in case of CAN RX interrupt.
 * @param   pNode        CAN interface on which message was received
 * @param   messageBox   message box on which message was received
 */
static void CAN_RxInterrupt(canBASE_t *pNode, uint32 messageBox);

/**
 * @brief    Handles the processing of messages that are meant to be transmitted.
 * This function looks for the repetition times and the repetition phase of
 * messages that are intended to be sent periodically. If a comparison with
 * an internal counter (i.e., the counter how often this function has been called)
 * states that a transmit is pending, the message is composed by call of CANS_ComposeMessage
 * and transferred to the buffer of the CAN module. If a callback function
 * is declared in configuration, this callback is called after successful transmission.
 * @return   #STD_OK if a CAN transfer was made, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e CAN_PeriodicTransmit(void);

/**
 * @brief   Checks if the CAN messages come in the specified time window
 * If the (current time stamp) - (previous time stamp) > 96ms and < 104ms,
 * the check is true, false otherwise.
 * @return  true if timing is in tolerance range, false otherwise
 */
static bool CAN_CheckCanTiming(void);

#if CURRENT_SENSOR_PRESENT == true
/**
 * @brief   Sets flag to indicate current sensor is present.
 *
 * @param command        true if current sensor present, otherwise false
 * @param stringNumber   string addressed
 *
 * @return  none
 */
static void CAN_SetCurrentSensorPresent(bool command, uint8_t stringNumber);

/**
 * @brief   Sets flag to indicate current sensor sends C-C values.
 *
 * @param command        true if coulomb counting message detected, otherwise false
 * @param stringNumber   string addressed
 *
 * @return  none
 */
static void CAN_SetCurrentSensorCcPresent(bool command, uint8_t stringNumber);

/**
 * @brief   Sets flag to indicate current sensor sends C-C values.
 *
 * @param command        true if energy counting message detected, otherwise false
 * @param stringNumber   string addressed
 *
 * @return  none
 */
static void CAN_SetCurrentSensorEcPresent(bool command, uint8_t stringNumber);
#endif /* CURRENT_SENSOR_PRESENT == true */

/** initialize the SPI interface to the CAN transceiver */
static void CAN_InitializeTransceiver(void);

/*========== Static Function Implementations ================================*/

static void CAN_InitializeTransceiver(void) {
    /* set EN and STB pins to output */
    SETBIT(CAN_HET1_GIO->DIR, CAN_HET1_EN_PIN);
    SETBIT(CAN_HET1_GIO->DIR, CAN_HET1_STB_PIN);

    /* first set EN and STB pins to 0 */
    IO_PinReset((uint32_t *)&CAN_HET1_GIO->DOUT, CAN_HET1_EN_PIN);
    IO_PinReset((uint32_t *)&CAN_HET1_GIO->DOUT, CAN_HET1_STB_PIN);
    /* wait after pin toggle */
    MCU_delay_us(CAN_PIN_TOGGLE_DELAY_US);

    /* set EN pin to 1 */
    IO_PinSet((uint32_t *)&CAN_HET1_GIO->DOUT, CAN_HET1_EN_PIN);
    /* wait after pin toggle */
    MCU_delay_us(CAN_PIN_TOGGLE_DELAY_US);

    /* set STB pin to 1 */
    IO_PinSet((uint32_t *)&CAN_HET1_GIO->DOUT, CAN_HET1_STB_PIN);
    /* wait after pin toggle */
    MCU_delay_us(CAN_PIN_TOGGLE_DELAY_US);
}

/*========== Extern Function Implementations ================================*/

extern void CAN_Initialize(void) {
    CAN_InitializeTransceiver();
}

extern STD_RETURN_TYPE_e CAN_DataSend(canBASE_t *pNode, uint32_t id, uint8 *pData) {
    FAS_ASSERT(pNode != NULL_PTR);
    FAS_ASSERT(pData != NULL_PTR);

    for (uint8_t messageBox = 1u; messageBox < CAN_NR_OF_TX_MESSAGEBOX; messageBox++) {
        if (canIsTxMessagePending(pNode, messageBox) == 0) {
            /* id shifted by 18 to use standard frame */
            /* standard frame: bits [28:18] */
            /* extended frame: bits [28:0] */
            /* bit 29 set to 1: to set direction Tx in IF2ARB register */
            canUpdateID(pNode, messageBox, ((id << 18) | (1U << 29)));
            canTransmit(pNode, messageBox, pData);
            return STD_OK;
        }
    }
    return STD_NOT_OK;
}

extern void CAN_MainFunction(void) {
    CAN_CheckCanTiming();
    if (true == can_state.periodicEnable) {
        CAN_PeriodicTransmit();
    }
}

static STD_RETURN_TYPE_e CAN_PeriodicTransmit(void) {
    STD_RETURN_TYPE_e retVal     = STD_NOT_OK;
    static uint32_t counterTicks = 0;
    uint8_t data[8]              = {0};

    for (uint16_t i = 0u; i < can_txLength; i++) {
        if (((counterTicks * CAN_TICK_MS) % (can_txMessages[i].repetitionTime)) == can_txMessages[i].repetitionPhase) {
            if (can_txMessages[i].callbackFunction != NULL_PTR) {
                OS_EnterTaskCritical();
                can_txMessages[i].callbackFunction(
                    can_txMessages[i].id,
                    can_txMessages[i].dlc,
                    can_txMessages[i].byteOrder,
                    data,
                    can_txMessages[i].pMuxId);
                OS_ExitTaskCritical();
                CAN_DataSend(CAN0_NODE, can_txMessages[i].id, data);
                retVal = STD_OK;
            }
        }
    }

    counterTicks++;
    return retVal;
}

static bool CAN_CheckCanTiming(void) {
    bool retVal = false;

    uint32_t current_time;
    DATA_BLOCK_ERRORSTATE_s errorFlagsTab     = {.header.uniqueId = DATA_BLOCK_ID_ERRORSTATE};
    DATA_BLOCK_STATEREQUEST_s staterequestTab = {.header.uniqueId = DATA_BLOCK_ID_STATEREQUEST};
#if CURRENT_SENSOR_PRESENT == true
    DATA_BLOCK_CURRENT_SENSOR_s currentTab = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR};
#endif /* CURRENT_SENSOR_PRESENT == true */

    current_time = OS_GetTickCount();
    DATA_READ_DATA(&staterequestTab, &errorFlagsTab);

    /* Is the BMS still getting CAN messages? */
    if ((current_time - staterequestTab.header.timestamp) <= 105) {
        if (((staterequestTab.header.timestamp - staterequestTab.header.previousTimestamp) >= 95) &&
            ((staterequestTab.header.timestamp - staterequestTab.header.previousTimestamp) <= 105)) {
            retVal = true;
            DIAG_Handler(DIAG_ID_CAN_TIMING, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
        } else {
            retVal = false;
            DIAG_Handler(DIAG_ID_CAN_TIMING, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
        }
    } else {
        retVal = false;
        DIAG_Handler(DIAG_ID_CAN_TIMING, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
    }

#if CURRENT_SENSOR_PRESENT == true
    /* check time stamps of current measurements */
    DATA_READ_DATA(&currentTab);

    for (uint8_t stringNumber = 0u; stringNumber < BS_NR_OF_STRINGS; stringNumber++) {
        /* Current has been measured at least once */
        if (currentTab.timestampCurrent[stringNumber] != 0u) {
            /* Check time since last received string current data */
            if ((current_time - currentTab.timestampCurrent[stringNumber]) >
                BS_CURRENT_MEASUREMENT_RESPONSE_TIMEOUT_MS) {
                DIAG_Handler(DIAG_ID_CURRENT_SENSOR_RESPONDING, DIAG_EVENT_NOT_OK, DIAG_STRING, stringNumber);
            } else {
                DIAG_Handler(DIAG_ID_CURRENT_SENSOR_RESPONDING, DIAG_EVENT_OK, DIAG_STRING, stringNumber);
                if (can_state.currentSensorPresent[stringNumber] == false) {
                    CAN_SetCurrentSensorPresent(true, stringNumber);
                }
            }
        }

        /* check time stamps of CC measurements */
        /* if timestamp_cc != 0, this means current sensor cc message has been received at least once */
        if (currentTab.timestampCurrentCounting[stringNumber] != 0) {
            if ((current_time - currentTab.timestampCurrentCounting[stringNumber]) >
                BS_COULOMB_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_MS) {
                DIAG_Handler(DIAG_ID_CAN_CC_RESPONDING, DIAG_EVENT_NOT_OK, DIAG_STRING, stringNumber);
            } else {
                DIAG_Handler(DIAG_ID_CAN_CC_RESPONDING, DIAG_EVENT_OK, DIAG_STRING, stringNumber);
                if (can_state.currentSensorCCPresent[stringNumber] == false) {
                    CAN_SetCurrentSensorCcPresent(true, stringNumber);
                }
            }
        }

        /* check time stamps of EC measurements */
        /* if timestamp_ec != 0, this means current sensor ec message has been received at least once */
        if (currentTab.timestampEnergyCounting[stringNumber] != 0) {
            if ((current_time - currentTab.timestampEnergyCounting[stringNumber]) >
                BS_ENERGY_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_MS) {
                DIAG_Handler(DIAG_ID_CAN_EC_RESPONDING, DIAG_EVENT_NOT_OK, DIAG_STRING, stringNumber);
            } else {
                DIAG_Handler(DIAG_ID_CAN_EC_RESPONDING, DIAG_EVENT_OK, DIAG_STRING, stringNumber);
                if (can_state.currentSensorECPresent[stringNumber] == false) {
                    CAN_SetCurrentSensorEcPresent(true, stringNumber);
                }
            }
        }
    }
#endif /* CURRENT_SENSOR_PRESENT == true */

    return retVal;
}

extern void CAN_ReadRxBuffer(void) {
    while (can_rxBuffer.pRead != can_rxBuffer.pWrite) {
        for (int i = 0; i < can_rxLength; i++) {
            if (can_rxBuffer.pRead->id == can_rxMessages[i].id) {
                if (can_rxMessages[i].callbackFunction != NULL_PTR) {
                    can_rxMessages[i].callbackFunction(
                        can_rxMessages[i].id,
                        can_rxMessages[i].dlc,
                        can_rxMessages[i].byteOrder,
                        can_rxBuffer.pRead->data,
                        NULL_PTR);
                }
            }
        }
        can_rxBuffer.pRead++;
        if (can_rxBuffer.pRead > &can_rxBufferData[CAN0_RX_BUFFER_LENGTH - 1u]) {
            can_rxBuffer.pRead = &can_rxBufferData[0];
        }
    }
}

/**
 * @brief   enable/disable the periodic transmit/receive.
 */
extern void CAN_EnablePeriodic(bool command) {
    if (command == true) {
        can_state.periodicEnable = true;
    } else {
        can_state.periodicEnable = false;
    }
}

#if CURRENT_SENSOR_PRESENT == true
static void CAN_SetCurrentSensorPresent(bool command, uint8_t stringNumber) {
    if (command == true) {
        OS_EnterTaskCritical();
        can_state.currentSensorPresent[stringNumber] = true;
        OS_ExitTaskCritical();
    } else {
        OS_EnterTaskCritical();
        can_state.currentSensorPresent[stringNumber] = false;
        OS_ExitTaskCritical();
    }
}

static void CAN_SetCurrentSensorCcPresent(bool command, uint8_t stringNumber) {
    if (command == true) {
        OS_EnterTaskCritical();
        can_state.currentSensorCCPresent[stringNumber] = true;
        OS_ExitTaskCritical();
    } else {
        OS_EnterTaskCritical();
        can_state.currentSensorCCPresent[stringNumber] = false;
        OS_ExitTaskCritical();
    }
}

static void CAN_SetCurrentSensorEcPresent(bool command, uint8_t stringNumber) {
    if (command == true) {
        OS_EnterTaskCritical();
        can_state.currentSensorECPresent[stringNumber] = true;
        OS_ExitTaskCritical();
    } else {
        OS_EnterTaskCritical();
        can_state.currentSensorECPresent[stringNumber] = false;
        OS_ExitTaskCritical();
    }
}
#endif /* CURRENT_SENSOR_PRESENT == true */

extern bool CAN_IsCurrentSensorPresent(uint8_t stringNumber) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    return can_state.currentSensorPresent[stringNumber];
}

extern bool CAN_IsCurrentSensorCcPresent(uint8_t stringNumber) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    return can_state.currentSensorCCPresent[stringNumber];
}

extern bool CAN_IsCurrentSensorEcPresent(uint8_t stringNumber) {
    FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS);
    return can_state.currentSensorECPresent[stringNumber];
}

static void CAN_TxInterrupt(canBASE_t *pNode, uint32 messageBox) {
}

static void CAN_RxInterrupt(canBASE_t *pNode, uint32 messageBox) {
    FAS_ASSERT(pNode != NULL_PTR);
    uint32_t id     = 0;
    uint8_t data[8] = {0};
    if (pNode == CAN0_NODE) {
        canGetData(pNode, messageBox, (uint8 *)&data[0]); /* copy to RAM */
        /* id shifted by 18 to use standard frame from IF2ARB register*/
        /* standard frame: bits [28:18] */
        /* extended frame: bits [28:0] */
        id = canGetID(pNode, messageBox) >> 18;

        can_rxBuffer.pWrite->id      = id;
        can_rxBuffer.pWrite->data[0] = data[0];
        can_rxBuffer.pWrite->data[1] = data[1];
        can_rxBuffer.pWrite->data[2] = data[2];
        can_rxBuffer.pWrite->data[3] = data[3];
        can_rxBuffer.pWrite->data[4] = data[4];
        can_rxBuffer.pWrite->data[5] = data[5];
        can_rxBuffer.pWrite->data[6] = data[6];
        can_rxBuffer.pWrite->data[7] = data[7];

        can_rxBuffer.pWrite++;
        if (can_rxBuffer.pWrite > &can_rxBufferData[CAN0_RX_BUFFER_LENGTH - 1u]) {
            can_rxBuffer.pWrite = &can_rxBufferData[0];
        }
    }
}

/** called in case of CAN interrupt, defined as weak in HAL */
void UNIT_TEST_WEAK_IMPL canMessageNotification(canBASE_t *node, uint32 messageBox) {
    if (messageBox <= CAN_NR_OF_TX_MESSAGEBOX) {
        CAN_TxInterrupt(node, messageBox);
    } else {
        CAN_RxInterrupt(node, messageBox);
    }
}

/*========== Getter for static Variables (Unit Test) ========================*/
#ifdef UNITY_UNIT_TEST
extern CAN_STATE_s *TEST_CAN_GetCANState(void) {
    return &can_state;
}
#endif

/*========== Externalized Static Function Implementations (Unit Test) =======*/
