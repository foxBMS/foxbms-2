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
 * @file    can.c
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CAN
 *
 * @brief   Driver for the CAN module
 * @details Implementation of the CAN Interrupts, initialization, buffers,
 *          receive and transmit interfaces.
 */

/*========== Includes =======================================================*/
#include "general.h"

#include "can.h"

#include "HL_can.h"
#include "HL_het.h"
#include "HL_reg_can.h"
#include "HL_reg_system.h"

#include "can_helper.h"
#include "database.h"
#include "diag.h"
#include "ftask.h"
#include "io.h"
#include "pex.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** lower limit of timestamp counts for a valid CAN timing */
#define CAN_TIMING_LOWER_LIMIT_COUNTS (95u)

/** upper limit of timestamp counts for a valid CAN timing */
#define CAN_TIMING_UPPER_LIMIT_COUNTS (105u)

/** return value of function canGetData if no data was lost during reception */
#define CAN_HAL_RETVAL_NO_DATA_LOST (1u)

/**
 * IF2ARB register configuration
 *
 * Bits 28-0 ID: Message identifier
 *    ID[28:0] 29-bit Identifier ("Extended Frame").
 *    ID[28:18] 11-bit Identifier ("Standard Frame").
 */
/** ID shift for standard identifier */
#define CAN_IF2ARB_STANDARD_IDENTIFIER_SHIFT (18u)
/** ID shift for extended identifier */
#define CAN_IF2ARB_EXTENDED_IDENTIFIER_SHIFT (0u)

/* Bit 29 Dir: Message direction
 *    0 Direction = Receive: On Tx Request, a Remote Frame with the identifier of this message object is
 *      transmitted. On receiving a Data Frame with a matching identifier, this message is stored in this
 *      message object.
 *    1 Direction = Transmit: On TxRequest, the respective message object is transmitted as a Data
 *      Frame. On receiving a Remote Frame with a matching identifier, the TxRequest bit of this message
 *      object is set (if RemoteEntry = 1).
 */
/** IF2ARB set TX direction */
#define CAN_IF2ARB_SET_TX_DIRECTION ((uint32)1u << 29u)
/* Bit 30 - Xtd: Extended identifier
 *    0 The 11-bit ("standard") identifier is used for this message object
 *    1 The 29-bit ("extended") identifier is used for this message object
 */
/** IF2ARB use standard identifier */
#define CAN_IF2ARB_USE_STANDARD_IDENTIFIER ((uint32)0u << 30u)
/** IF2ARB use extended identifier */
#define CAN_IF2ARB_USE_EXTENDED_IDENTIFIER ((uint32)1u << 30u)

/** Range of mailboxes that are explicitly configured for the reception of
 *  messages with extended identifiers. */ /**@{*/
#define CAN_LOWEST_MAILBOX_FOR_EXTENDED_IDENTIFIERS  (61u)
#define CAN_HIGHEST_MAILBOX_FOR_EXTENDED_IDENTIFIERS (64u)
/**@}*/

FAS_STATIC_ASSERT(
    (CAN_LOWEST_MAILBOX_FOR_EXTENDED_IDENTIFIERS <= CAN_HIGHEST_MAILBOX_FOR_EXTENDED_IDENTIFIERS),
    "Lower mailbox number must not be greater than higher mailbox number");
FAS_STATIC_ASSERT(
    (CAN_HIGHEST_MAILBOX_FOR_EXTENDED_IDENTIFIERS <= CAN_TOTAL_NUMBER_OF_MESSAGE_BOXES),
    "Highest mailbox number must not be larger than total number of mailboxes");

/*========== Static Constant and Variable Definitions =======================*/

/** tracks the local state of the can module */
static CAN_STATE_s can_state = {
    .periodicEnable         = false,
    .currentSensorPresent   = {GEN_REPEAT_U(false, GEN_STRIP(BS_NR_OF_STRINGS))},
    .currentSensorCCPresent = {GEN_REPEAT_U(false, GEN_STRIP(BS_NR_OF_STRINGS))},
    .currentSensorECPresent = {GEN_REPEAT_U(false, GEN_STRIP(BS_NR_OF_STRINGS))},
};

/** stores the number of CAN_periodicTransmit calls at which the internal
 *  counter will reset to prevent overflow in CAN_IsPeriodElapsed */
static uint32_t can_counterResetValue = 0u;

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
 * @brief   Handles the processing of messages that are meant to be
 *          transmitted.
 * @details This function looks for the repetition times and the repetition
 *          phase of messages that are intended to be sent periodically.
 *          If a comparison with an internal counter (i.e., the counter how
 *          often this function has been called) states that a transmit is
 *          pending, the message is composed by call of CANS_ComposeMessage
 *          and transferred to the buffer of the CAN module.
 *          If a callback function is declared in configuration, this callback
 *          is called after successful transmission.
 * @return  #STD_OK if a CAN transfer was made, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e CAN_PeriodicTransmit(void);

/**
 * @brief   Checks if a configured period CAN message should be transmitted depending
 *          on the configured message period and message phase.
 * @param   ticksSinceStart internal counter of a periodically called function
 * @param   messageIndex    index of the message to check in the tx message array
 * @return  true if phase matches and message should be transmitted, false otherwise
 */

static bool CAN_IsMessagePeriodElapsed(uint32_t ticksSinceStart, uint16_t messageIndex);

/**
 * @brief   Checks if the CAN messages come in the specified time window
 * @details The time delta where a message is expected to be received needs to
 *          be between the configured limits #CAN_TIMING_LOWER_LIMIT_COUNTS and
 *          #CAN_TIMING_UPPER_LIMIT_COUNTS,
 *          If the message is received within the time frame the check is true,
 *          false otherwise.
 *          The result is then reported via flags in the DIAG module.
 */
static void CAN_CheckCanTiming(void);

#if BS_CURRENT_SENSOR_PRESENT == true
/**
 * @brief   Sets flag to indicate current sensor is present.
 * @param   command         true if current sensor present, otherwise false
 * @param   stringNumber    string addressed
 */
static void CAN_SetCurrentSensorPresent(bool command, uint8_t stringNumber);

/**
 * @brief   Sets flag to indicate current sensor sends C-C values.
 * @param   command         true if coulomb counting message detected,
 *                          otherwise false
 * @param   stringNumber    string addressed
 */
static void CAN_SetCurrentSensorCcPresent(bool command, uint8_t stringNumber);

/**
 * @brief   Sets flag to indicate current sensor sends C-C values.
 * @param   command         true if energy counting message detected, otherwise
 *                          false
 * @param   stringNumber    string addressed
 */
static void CAN_SetCurrentSensorEcPresent(bool command, uint8_t stringNumber);

static void CAN_CheckCanTimingOfCurrentSensor(void);
#endif /* BS_CURRENT_SENSOR_PRESENT == true */

/**
 * @brief    Initialize RX mailboxes for usage with extended identifiers
 * @details  The first 32 mailboxes are configured via HALCoGen as RX mailboxes
 *           to be used with standard 11-bit identifiers. As the configuration
 *           if standard or extended identifiers should be used can only be
 *           configured for all mailboxes and not individually for each mailbox
 *           this configuration is done here.
 *
 *           The HALCoGen configuration for the mailboxes 61-64 is overwritten
 *           here and they are configured to received all CAN messages with an
 *           extended identifier.
 *
 *           This configuration is only done for CAN nodes canReg1 and canReg2!
 */
static void CAN_ConfigureRxMailboxesForExtendedIdentifiers(void);

/** Initialize transceiver standby/enable pins */
static void CAN_InitializeTransceiver(void);

/**
 * @brief   Calculate Reset Value for internal counter in CAN_PeriodicTransmit
 * @details Calculate the least common multiply of message periods in
 *          can_TxMessages array. All periods will elapse at this time, so the
 *          counter can safely be reset.
 * @return  Reset Value for internal counter in CAN_PeriodicTransmit
 */
static uint32_t CAN_CalculateCounterResetValue(void);

/** checks that the configured message period for Tx messages is valid */
static void CAN_ValidateConfiguredTxMessagePeriod(void);

/** checks that the configured message phase for Tx messages is valid */
static void CAN_ValidateConfiguredTxMessagePhase(void);

/** checks the  struct block for storing and passing on the local
 * database table handle for containing no null pointers*/
static void CAN_CheckDatabaseNullPointer(CAN_SHIM_s canShim);

/**
 * @brief  get pointer CAN node configuration struct from register address
 * @param  pNodeRegister pointer to CAN node hardware register address
 * @return pointer to node configuration struct
 */
static CAN_NODE_s *CAN_GetNodeConfigurationStructFromRegisterAddress(canBASE_t *pNodeRegister);

/*========== Static Function Implementations ================================*/

static void CAN_ConfigureRxMailboxesForExtendedIdentifiers(void) {
    /* Content copied from HALCoGen generated configuration file HL_can.c
     * Date: 11-Dec-2018
     * Version: 04.07.01
     */

    /* AXIVION Disable Style Generic-NoMagicNumbers: Content copied from HALCoGen generated configuration file */
    /* AXIVION Disable Style MisraC2012-2.2: Content copied from HALCoGen generated configuration file */
    /* AXIVION Disable Style IISB-LiteralSuffixesCheck: Content copied from HALCoGen generated configuration file */
    /* AXIVION Disable Style Generic-NoEmptyLoops: Content copied from HALCoGen generated configuration file */

    /* Reconfigure CAN1 mailboxes 42, 61, 62, 63 and 64 */

    /** - Setup control register
     *     - Disable automatic wakeup on bus activity
     *     - Local power down mode disabled
     *     - Disable DMA request lines
     *     - Enable global Interrupt Line 0 and 1
     *     - Disable debug mode
     *     - Release from software reset
     *     - Enable/Disable parity or ECC
     *     - Enable/Disable auto bus on timer
     *     - Setup message completion before entering debug state
     *     - Setup normal operation mode
     *     - Request write access to the configuration registers
     *     - Setup automatic retransmission of messages
     *     - Disable error interrupts
     *     - Disable status interrupts
     *     - Enter initialization mode
     */
    canREG1->CTL = (uint32)0x00000000U | (uint32)0x00000000U | (uint32)((uint32)0x0000000AU << 10U) |
                   (uint32)0x00020043U;

    /** - Clear all pending error flags and reset current status */
    canREG1->ES |= 0xFFFFFFFFU;

    /** - Setup auto bus on timer period */
    canREG1->ABOTR = (uint32)0U;

    /** - Initialize message 42
     *     - Wait until IF1 is ready for use
     *     - Set message mask
     *     - Set message control word
     *     - Set message arbitration
     *     - Set IF1 control byte
     *     - Set IF1 message number
     */
    /* SAFETYMCUSW 28 D MR:NA <APPROVED> "Potentially infinite loop found -
     * Hardware Status check for execution sequence" */
    while ((canREG1->IF1STAT & 0x80U) == 0x80U) {
    } /* Wait */

    canREG1->IF1MSK = 0xC0000000U | (uint32)((uint32)((uint32)0x00000000U & (uint32)0x000007FFU) << (uint32)18U);
    canREG1->IF1ARB = (uint32)0x80000000U | (uint32)0x00000000U | (uint32)0x00000000U |
                      (uint32)((uint32)((uint32)0U & (uint32)0x000007FFU) << (uint32)18U);
    canREG1->IF1MCTL = 0x00001000U | (uint32)0x00000400U | (uint32)0x00000000U | (uint32)0x00000000U | (uint32)8U;
    canREG1->IF1CMD  = (uint8)0xF8U;
    canREG1->IF1NO   = 42U;

    /** - Initialize message 61
     *     - Wait until IF1 is ready for use
     *     - Set message mask
     *     - Set message control word
     *     - Set message arbitration
     *     - Set IF1 control byte
     *     - Set IF1 message number
     */
    /* SAFETYMCUSW 28 D MR:NA <APPROVED> "Potentially infinite loop found -
     * Hardware Status check for execution sequence" */
    while ((canREG1->IF1STAT & 0x80U) == 0x80U) {
    } /* Wait */

    canREG1->IF1MSK = 0xC0000000U | (uint32)((uint32)((uint32)0x00000000U & (uint32)0x1FFFFFFFU) << (uint32)0U);
    canREG1->IF1ARB = (uint32)0x80000000U | (uint32)0x40000000U | (uint32)0x00000000U |
                      (uint32)((uint32)((uint32)0U & (uint32)0x1FFFFFFFU) << (uint32)0U);
    canREG1->IF1MCTL = 0x00001000U | (uint32)0x00000400U | (uint32)0x00000000U | (uint32)0x00000000U | (uint32)8U;
    canREG1->IF1CMD  = (uint8)0xF8U;
    canREG1->IF1NO   = 61U;

    /** - Initialize message 62
     *     - Wait until IF2 is ready for use
     *     - Set message mask
     *     - Set message control word
     *     - Set message arbitration
     *     - Set IF2 control byte
     *     - Set IF2 message number
     */
    /* SAFETYMCUSW 28 D MR:NA <APPROVED> "Potentially infinite loop found -
     * Hardware Status check for execution sequence" */
    while ((canREG1->IF2STAT & 0x80U) == 0x80U) {
    } /* Wait */

    canREG1->IF2MSK = 0xC0000000U | (uint32)((uint32)((uint32)0x00000000U & (uint32)0x1FFFFFFFU) << (uint32)0U);
    canREG1->IF2ARB = (uint32)0x80000000U | (uint32)0x40000000U | (uint32)0x00000000U |
                      (uint32)((uint32)((uint32)0U & (uint32)0x1FFFFFFFU) << (uint32)0U);
    canREG1->IF2MCTL = 0x00001000U | (uint32)0x00000400U | (uint32)0x00000000U | (uint32)0x00000000U | (uint32)8U;
    canREG1->IF2CMD  = (uint8)0xF8U;
    canREG1->IF2NO   = 62U;

    /** - Initialize message 63
     *     - Wait until IF1 is ready for use
     *     - Set message mask
     *     - Set message control word
     *     - Set message arbitration
     *     - Set IF1 control byte
     *     - Set IF1 message number
     */
    /* SAFETYMCUSW 28 D MR:NA <APPROVED> "Potentially infinite loop found -
     * Hardware Status check for execution sequence" */
    while ((canREG1->IF1STAT & 0x80U) == 0x80U) {
    } /* Wait */

    canREG1->IF1MSK = 0xC0000000U | (uint32)((uint32)((uint32)0x00000000U & (uint32)0x1FFFFFFFU) << (uint32)0U);
    canREG1->IF1ARB = (uint32)0x80000000U | (uint32)0x40000000U | (uint32)0x00000000U |
                      (uint32)((uint32)((uint32)0U & (uint32)0x1FFFFFFFU) << (uint32)0U);
    canREG1->IF1MCTL = 0x00001000U | (uint32)0x00000400U | (uint32)0x00000000U | (uint32)0x00000000U | (uint32)8U;
    canREG1->IF1CMD  = (uint8)0xF8U;
    canREG1->IF1NO   = 63U;

    /** - Initialize message 64
     *     - Wait until IF2 is ready for use
     *     - Set message mask
     *     - Set message control word
     *     - Set message arbitration
     *     - Set IF2 control byte
     *     - Set IF2 message number
     */
    /* SAFETYMCUSW 28 D MR:NA <APPROVED> "Potentially infinite loop found -
     * Hardware Status check for execution sequence" */
    while ((canREG1->IF2STAT & 0x80U) == 0x80U) {
    } /* Wait */

    canREG1->IF2MSK = 0xC0000000U | (uint32)((uint32)((uint32)0x00000000U & (uint32)0x1FFFFFFFU) << (uint32)0U);
    canREG1->IF2ARB = (uint32)0x80000000U | (uint32)0x40000000U | (uint32)0x00000000U |
                      (uint32)((uint32)((uint32)0U & (uint32)0x1FFFFFFFU) << (uint32)0U);
    canREG1->IF2MCTL = 0x00001000U | (uint32)0x00000400U | (uint32)0x00000000U | (uint32)0x00000000U | (uint32)8U;
    canREG1->IF2CMD  = (uint8)0xF8U;
    canREG1->IF2NO   = 64U;

    /** - Setup IF1 for data transmission
     *     - Wait until IF1 is ready for use
     *     - Set IF1 control byte
     */
    /* SAFETYMCUSW 28 D MR:NA <APPROVED> "Potentially infinite loop found -
     * Hardware Status check for execution sequence" */
    while ((canREG1->IF1STAT & 0x80U) == 0x80U) {
    } /* Wait */
    canREG1->IF1CMD = 0x87U;

    /** - Setup IF2 for reading data
     *     - Wait until IF1 is ready for use
     *     - Set IF1 control byte
     */
    /* SAFETYMCUSW 28 D MR:NA <APPROVED> "Potentially infinite loop found -
     * Hardware Status check for execution sequence" */
    while ((canREG1->IF2STAT & 0x80U) == 0x80U) {
    } /* Wait */
    canREG1->IF2CMD = 0x17U;

    /** - Leave configuration and initialization mode  */
    canREG1->CTL &= ~(uint32)(0x00000041U);

    /** Reconfigure CAN2 mailboxes 61 - 64 */

    /** - Setup control register
     *     - Disable automatic wakeup on bus activity
     *     - Local power down mode disabled
     *     - Disable DMA request lines
     *     - Enable global Interrupt Line 0 and 1
     *     - Disable debug mode
     *     - Release from software reset
     *     - Enable/Disable parity or ECC
     *     - Enable/Disable auto bus on timer
     *     - Setup message completion before entering debug state
     *     - Setup normal operation mode
     *     - Request write access to the configuration registers
     *     - Setup automatic retransmission of messages
     *     - Disable error interrupts
     *     - Disable status interrupts
     *     - Enter initialization mode
     */
    canREG2->CTL = (uint32)0x00000000U | (uint32)0x00000000U | (uint32)((uint32)0x0000000AU << 10U) | 0x00020043U;

    /** - Clear all pending error flags and reset current status */
    canREG2->ES |= 0xFFFFFFFFU;

    /** - Initialize message 61
     *     - Wait until IF1 is ready for use
     *     - Set message mask
     *     - Set message control word
     *     - Set message arbitration
     *     - Set IF1 control byte
     *     - Set IF1 message number
     */
    /* SAFETYMCUSW 28 D MR:NA <APPROVED> "Potentially infinite loop found -
     * Hardware Status check for execution sequence" */
    while ((canREG2->IF1STAT & 0x80U) == 0x80U) {
    } /* Wait */

    canREG2->IF1MSK = 0xC0000000U | (uint32)((uint32)((uint32)0x00000000U & (uint32)0x1FFFFFFFU) << (uint32)0U);
    canREG2->IF1ARB = (uint32)0x80000000U | (uint32)0x40000000U | (uint32)0x00000000U |
                      (uint32)((uint32)((uint32)0U & (uint32)0x1FFFFFFFU) << (uint32)0U);
    canREG2->IF1MCTL = 0x00001000U | (uint32)0x00000400U | (uint32)0x00000000U | (uint32)0x00000000U | (uint32)8U;
    canREG2->IF1CMD  = (uint8)0xF8U;
    canREG2->IF1NO   = 61U;

    /** - Initialize message 62
     *     - Wait until IF2 is ready for use
     *     - Set message mask
     *     - Set message control word
     *     - Set message arbitration
     *     - Set IF2 control byte
     *     - Set IF2 message number
     */
    /* SAFETYMCUSW 28 D MR:NA <APPROVED> "Potentially infinite loop found -
     * Hardware Status check for execution sequence" */
    while ((canREG2->IF2STAT & 0x80U) == 0x80U) {
    } /* Wait */

    canREG2->IF2MSK = 0xC0000000U | (uint32)((uint32)((uint32)0x00000000U & (uint32)0x1FFFFFFFU) << (uint32)0U);
    canREG2->IF2ARB = (uint32)0x80000000U | (uint32)0x40000000U | (uint32)0x00000000U |
                      (uint32)((uint32)((uint32)0U & (uint32)0x1FFFFFFFU) << (uint32)0U);
    canREG2->IF2MCTL = 0x00001000U | (uint32)0x00000400U | (uint32)0x00000000U | (uint32)0x00000000U | (uint32)8U;
    canREG2->IF2CMD  = (uint8)0xF8U;
    canREG2->IF2NO   = 62U;

    /** - Initialize message 63
     *     - Wait until IF1 is ready for use
     *     - Set message mask
     *     - Set message control word
     *     - Set message arbitration
     *     - Set IF1 control byte
     *     - Set IF1 message number
     */
    /* SAFETYMCUSW 28 D MR:NA <APPROVED> "Potentially infinite loop found -
     * Hardware Status check for execution sequence" */
    while ((canREG2->IF1STAT & 0x80U) == 0x80U) {
    } /* Wait */

    canREG2->IF1MSK = 0xC0000000U | (uint32)((uint32)((uint32)0x00000000U & (uint32)0x1FFFFFFFU) << (uint32)0U);
    canREG2->IF1ARB = (uint32)0x80000000U | (uint32)0x40000000U | (uint32)0x00000000U |
                      (uint32)((uint32)((uint32)0U & (uint32)0x1FFFFFFFU) << (uint32)0U);
    canREG2->IF1MCTL = 0x00001000U | (uint32)0x00000400U | (uint32)0x00000000U | (uint32)0x00000000U | (uint32)8U;
    canREG2->IF1CMD  = (uint8)0xF8U;
    canREG2->IF1NO   = 63U;

    /** - Initialize message 64
     *     - Wait until IF2 is ready for use
     *     - Set message mask
     *     - Set message control word
     *     - Set message arbitration
     *     - Set IF2 control byte
     *     - Set IF2 message number
     */
    /* SAFETYMCUSW 28 D MR:NA <APPROVED> "Potentially infinite loop found -
     * Hardware Status check for execution sequence" */
    while ((canREG2->IF2STAT & 0x80U) == 0x80U) {
    } /* Wait */

    canREG2->IF2MSK = 0xC0000000U | (uint32)((uint32)((uint32)0x00000000U & (uint32)0x1FFFFFFFU) << (uint32)0U);
    canREG2->IF2ARB = (uint32)0x80000000U | (uint32)0x40000000U | (uint32)0x00000000U |
                      (uint32)((uint32)((uint32)0U & (uint32)0x1FFFFFFFU) << (uint32)0U);
    canREG2->IF2MCTL = 0x00001000U | (uint32)0x00000400U | (uint32)0x00000000U | (uint32)0x00000000U | (uint32)8U;
    canREG2->IF2CMD  = (uint8)0xF8U;
    canREG2->IF2NO   = 64U;

    /** - Setup IF1 for data transmission
     *     - Wait until IF1 is ready for use
     *     - Set IF1 control byte
     */
    /* SAFETYMCUSW 28 D MR:NA <APPROVED> "Potentially infinite loop found -
     * Hardware Status check for execution sequence" */
    while ((canREG2->IF1STAT & 0x80U) == 0x80U) {
    } /* Wait */
    canREG2->IF1CMD = 0x87U;

    /** - Setup IF2 for reading data
     *     - Wait until IF1 is ready for use
     *     - Set IF1 control byte
     */
    /* SAFETYMCUSW 28 D MR:NA <APPROVED> "Potentially infinite loop found -
     * Hardware Status check for execution sequence" */
    while ((canREG2->IF2STAT & 0x80U) == 0x80U) {
    } /* Wait */
    canREG2->IF2CMD = 0x17U;

    /** - Leave configuration and initialization mode  */
    canREG2->CTL &= ~(uint32)(0x00000041U);

    /* AXIVION Enable Style Generic-NoMagicNumbers: */
    /* AXIVION Enable Style MisraC2012-2.2: */
    /* AXIVION Enable Style IISB-LiteralSuffixesCheck: */
    /* AXIVION Enable Style Generic-NoEmptyLoops: */
}

static void CAN_InitializeTransceiver(void) {
    /** Initialize transceiver for CAN1 */
    IO_SetPinDirectionToOutput(&CAN_CAN1_IO_REG_DIR, CAN_CAN1_ENABLE_PIN);
    IO_SetPinDirectionToOutput(&CAN_CAN1_IO_REG_DIR, CAN_CAN1_STANDBY_PIN);
    IO_PinSet(&CAN_CAN1_IO_REG_DOUT, CAN_CAN1_ENABLE_PIN);
    IO_PinSet(&CAN_CAN1_IO_REG_DOUT, CAN_CAN1_STANDBY_PIN);

    /** Initialize transceiver for CAN2 */
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER2, CAN_CAN2_ENABLE_PIN);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER2, CAN_CAN2_STANDBY_PIN);
    PEX_SetPin(PEX_PORT_EXPANDER2, CAN_CAN2_ENABLE_PIN);
    PEX_SetPin(PEX_PORT_EXPANDER2, CAN_CAN2_STANDBY_PIN);
}

static void CAN_ValidateConfiguredTxMessagePeriod(void) {
    for (uint16_t i = 0u; i < can_txMessagesLength; i++) {
        if ((can_txMessages[i].timing.period == 0u) || ((can_txMessages[i].timing.period % CAN_TICK_ms) != 0u)) {
            FAS_ASSERT(FAS_TRAP);
        }
    }
}

static void CAN_ValidateConfiguredTxMessagePhase(void) {
    for (uint16_t i = 0u; i < can_txMessagesLength; i++) {
        if ((can_txMessages[i].timing.phase >= can_txMessages[i].timing.period) ||
            ((can_txMessages[i].timing.phase % CAN_TICK_ms) != 0u)) {
            FAS_ASSERT(FAS_TRAP);
        }
    }
}

static void CAN_CheckDatabaseNullPointer(CAN_SHIM_s canShim) {
    FAS_ASSERT(canShim.pQueueImd != NULL_PTR);
    FAS_ASSERT(canShim.pTableCellVoltage != NULL_PTR);     /*!< pointer database table with cell voltages */
    FAS_ASSERT(canShim.pTableCellTemperature != NULL_PTR); /*!< pointer database table with cell temperatures */
    FAS_ASSERT(canShim.pTableCurrentSensor != NULL_PTR); /*!< pointer database table with current sensor measurements */
    FAS_ASSERT(canShim.pTableErrorState != NULL_PTR);    /*!< pointer database table with error state variables */
    FAS_ASSERT(canShim.pTableInsulation != NULL_PTR);    /*!< pointer database table with insulation monitoring info */
    FAS_ASSERT(canShim.pTableMinMax != NULL_PTR);        /*!< pointer database table with min/max values */
    FAS_ASSERT(canShim.pTableMol != NULL_PTR);           /*!< pointer database table with MOL flags */
    FAS_ASSERT(canShim.pTableMsl != NULL_PTR);           /*!< pointer database table with MSL flags */
    FAS_ASSERT(canShim.pTableOpenWire != NULL_PTR);      /*!< pointer database table with open wire status */
    FAS_ASSERT(canShim.pTablePackValues != NULL_PTR);    /*!< pointer database table with pack values */
    FAS_ASSERT(canShim.pTableRsl != NULL_PTR);           /*!< pointer database table with RSL flags */
    FAS_ASSERT(canShim.pTableSoc != NULL_PTR);           /*!< pointer database table with SOC values */
    FAS_ASSERT(canShim.pTableSoe != NULL_PTR);           /*!< pointer database table with SOE values */
    FAS_ASSERT(canShim.pTableSof != NULL_PTR);           /*!< pointer database table with SOF values */
    FAS_ASSERT(canShim.pTableSoh != NULL_PTR);           /*!< pointer database table with SOH values */
    FAS_ASSERT(canShim.pTableStateRequest != NULL_PTR);  /*!< pointer database table with state requests */
}

static CAN_NODE_s *CAN_GetNodeConfigurationStructFromRegisterAddress(canBASE_t *pNodeRegister) {
    FAS_ASSERT(pNodeRegister != NULL_PTR);
    CAN_NODE_s *node = NULL_PTR;
    /* Find correct CAN node configuration struct */
    if (pNodeRegister == can_node1.canNodeRegister) {
        node = (CAN_NODE_s *)&can_node1;
    } else if (pNodeRegister == can_node2Isolated.canNodeRegister) {
        node = (CAN_NODE_s *)&can_node2Isolated;
    } else {
        /* Invalid address. This should not have happened */
        FAS_ASSERT(FAS_TRAP);
    }
    return node;
}

static void CAN_RxInterrupt(canBASE_t *pNode, uint32 messageBox) {
    FAS_ASSERT(pNode != NULL_PTR);
    FAS_ASSERT(messageBox <= CAN_TOTAL_NUMBER_OF_MESSAGE_BOXES); /* hardware starts counting at 1 -> use <= */

    uint8_t messageData[CAN_DEFAULT_DLC] = {0u};
    /**
     *  Read even if queues are not created, otherwise message boxes get full.
     *  Possible return values:
     *   - 0: no new data
     *   - 1: no data lost
     *   - 3: data lost */
    uint32_t retval = canGetData(pNode, messageBox, (uint8 *)&messageData[0]); /* copy to RAM */

    /* Check that CAN RX queue is started before using it and data is valid */
    if ((ftsk_allQueuesCreated == true) && (retval == CAN_HAL_RETVAL_NO_DATA_LOST)) {
        CAN_BUFFER_ELEMENT_s can_rxBuffer = {NULL_PTR, 0u, CAN_INVALID_TYPE, {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}};
        /* Find configured CAN node from register address */
        can_rxBuffer.canNode = CAN_GetNodeConfigurationStructFromRegisterAddress(pNode);

        /* Check message box number if it is a mailbox reserved for extended identifiers or not */
        if (!((messageBox >= CAN_LOWEST_MAILBOX_FOR_EXTENDED_IDENTIFIERS) &&
              (messageBox <= CAN_HIGHEST_MAILBOX_FOR_EXTENDED_IDENTIFIERS))) {
            /* Extract standard identifier from IF2ARB register*/
            can_rxBuffer.id     = canGetID(pNode, messageBox) >> CAN_IF2ARB_STANDARD_IDENTIFIER_SHIFT;
            can_rxBuffer.idType = CAN_STANDARD_IDENTIFIER_11_BIT;
        } else {
            /* Extract extended identifier from IF2ARB register*/
            can_rxBuffer.id     = canGetID(pNode, messageBox) >> CAN_IF2ARB_EXTENDED_IDENTIFIER_SHIFT;
            can_rxBuffer.idType = CAN_EXTENDED_IDENTIFIER_29_BIT;
        }

        /* Write data into buffer */
        can_rxBuffer.data[CAN_BYTE_0_POSITION] = messageData[CAN_BYTE_0_POSITION];
        can_rxBuffer.data[CAN_BYTE_1_POSITION] = messageData[CAN_BYTE_1_POSITION];
        can_rxBuffer.data[CAN_BYTE_2_POSITION] = messageData[CAN_BYTE_2_POSITION];
        can_rxBuffer.data[CAN_BYTE_3_POSITION] = messageData[CAN_BYTE_3_POSITION];
        can_rxBuffer.data[CAN_BYTE_4_POSITION] = messageData[CAN_BYTE_4_POSITION];
        can_rxBuffer.data[CAN_BYTE_5_POSITION] = messageData[CAN_BYTE_5_POSITION];
        can_rxBuffer.data[CAN_BYTE_6_POSITION] = messageData[CAN_BYTE_6_POSITION];
        can_rxBuffer.data[CAN_BYTE_7_POSITION] = messageData[CAN_BYTE_7_POSITION];

        if (OS_SendToBackOfQueueFromIsr(ftsk_canRxQueue, (void *)&can_rxBuffer, NULL_PTR) == OS_SUCCESS) {
            /* queue is not full */
            (void)DIAG_Handler(DIAG_ID_CAN_RX_QUEUE_FULL, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
        } else {
            /* queue is full */
            (void)DIAG_Handler(DIAG_ID_CAN_RX_QUEUE_FULL, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
        }
    }
}

static STD_RETURN_TYPE_e CAN_PeriodicTransmit(void) {
    STD_RETURN_TYPE_e retVal     = STD_NOT_OK;
    static uint32_t counterTicks = 0;
    uint8_t data[CAN_MAX_DLC]    = {0};

    CAN_SendMessagesFromQueue();

    for (uint16_t i = 0u; i < can_txMessagesLength; i++) {
        if (CAN_IsMessagePeriodElapsed(counterTicks, i) == true) {
            if (can_txMessages[i].callbackFunction != NULL_PTR) {
                can_txMessages[i].callbackFunction(
                    can_txMessages[i].message, data, can_txMessages[i].pMuxId, &can_kShim);
                if (CAN_DataSend(
                        can_txMessages[i].canNode,
                        can_txMessages[i].message.id,
                        can_txMessages[i].message.idType,
                        data) != STD_OK) {
                    /* message was not sent */
                    /* store the message */
                    CAN_BUFFER_ELEMENT_s unsentMessage = {
                        .canNode = can_txMessages[i].canNode,
                        .id      = can_txMessages[i].message.id,
                        .idType  = can_txMessages[i].message.idType,
                        .data    = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}};

                    for (uint8_t j = 0; j < can_txMessages[i].message.dlc; j++) {
                        unsentMessage.data[j] = data[j];
                    }

                    /* add message to queue */
                    if (OS_SendToBackOfQueue(ftsk_canTxUnsentMessagesQueue, (void *)&unsentMessage, 0u) == OS_SUCCESS) {
                        /* Queue is not full */
                        (void)DIAG_Handler(DIAG_ID_CAN_TX_QUEUE_FULL, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
                    } else {
                        /* Queue is full */
                        (void)DIAG_Handler(DIAG_ID_CAN_TX_QUEUE_FULL, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
                    }
                }
                retVal = STD_OK;
            }
        }
    }

    /* check if counter needs to be reset */
    counterTicks++;
    if (counterTicks == can_counterResetValue) {
        counterTicks = 0;
    }

    return retVal;
}

static uint32_t CAN_CalculateCounterResetValue(void) {
    /* initialize with first array entry for first iteration */
    uint32_t resetValue = can_txMessages[0].timing.period;

    /*  calculate least common multiple (LCM) of the first and second can_TxMessage period.
        After that, calculate LCM of the next can_TxMessage period and last iterations LCM
        until no more can_TxMessages remain. */
    for (uint8_t i = 1u; i < can_txMessagesLength; i++) {
        const uint32_t value1 = resetValue;
        const uint32_t value2 = can_txMessages[i].timing.period;

        /* calculate greatest common divisor using Euclid's algorithm */
        /* the algorithm is designed for starting with bigger value, but also
           works when starting with smaller value, so just start with value1 */

        uint32_t result    = value1;
        uint32_t divisor   = value2;
        uint32_t tempValue = 0u;

        while (divisor != 0u) {
            tempValue = result % divisor;
            result    = divisor;
            divisor   = tempValue;
        }

        /* calculate least common multiple using the greatest common divisor */
        resetValue = (value1 / result) * value2;
        /* AXIVION Routine FaultDetection-DivisionByZero: /: message periods cannot be 0, so result is never 0 */
        /* AXIVION Routine MisraC2012Directive-4.1: /: message periods cannot be 0, so result is never 0 */
        /* AXIVION Routine MisraC2012Directive-4.1: *: only wraps around if periods are too big */
    }

    /* scale down to match internal counter of CAN_PeriodicTransmit */
    resetValue /= CAN_TICK_ms;
    return resetValue;
}

static bool CAN_IsMessagePeriodElapsed(uint32_t ticksSinceStart, uint16_t messageIndex) {
    /* AXIVION Routine Generic-MissingParameterAssert: ticksSinceStart: parameter accepts whole range */
    FAS_ASSERT(messageIndex < can_txMessagesLength);

    bool retVal = false;
    if (((ticksSinceStart * CAN_TICK_ms) % (can_txMessages[messageIndex].timing.period)) ==
        can_txMessages[messageIndex].timing.phase) {
        /* AXIVION Routine MisraC2012Directive-4.1: *: counter gets reset in periodicTransmit, no wrap around */
        /* AXIVION Routine FaultDetection-DivisionByZero: %: message period is never zero, checked by config */
        /* AXIVION Routine MisraC2012Directive-4.1: %: message period is never zero, checked by config */
        retVal = true;
    }
    return retVal;
}

static void CAN_CheckCanTiming(void) {
    uint32_t currentTime                       = OS_GetTickCount();
    DATA_BLOCK_ERROR_STATE_s errorFlagsTab     = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};
    DATA_BLOCK_STATE_REQUEST_s stateRequestTab = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};

    DATA_READ_DATA(&stateRequestTab, &errorFlagsTab);

    /* Is the BMS still getting CAN messages? */
    if ((currentTime - stateRequestTab.header.timestamp) <= CAN_TIMING_UPPER_LIMIT_COUNTS) {
        if (((stateRequestTab.header.timestamp - stateRequestTab.header.previousTimestamp) >=
             CAN_TIMING_LOWER_LIMIT_COUNTS) &&
            ((stateRequestTab.header.timestamp - stateRequestTab.header.previousTimestamp) <=
             CAN_TIMING_UPPER_LIMIT_COUNTS)) {
            DIAG_Handler(DIAG_ID_CAN_TIMING, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
        } else {
            DIAG_Handler(DIAG_ID_CAN_TIMING, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
        }
    } else {
        DIAG_Handler(DIAG_ID_CAN_TIMING, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
    }

#if BS_CURRENT_SENSOR_PRESENT == true
    CAN_CheckCanTimingOfCurrentSensor();
#endif /* BS_CURRENT_SENSOR_PRESENT == true */
}

#if BS_CURRENT_SENSOR_PRESENT == true
static void CAN_CheckCanTimingOfCurrentSensor(void) {
    uint32_t currentTime                   = OS_GetTickCount();
    DATA_BLOCK_CURRENT_SENSOR_s currentTab = {.header.uniqueId = DATA_BLOCK_ID_CURRENT_SENSOR};
    /* check time stamps of current measurements */
    DATA_READ_DATA(&currentTab);

    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        /* Current has been measured at least once */
        if (currentTab.timestampCurrent[s] != 0u) {
            /* Check time since last received string current data */
            if ((currentTime - currentTab.timestampCurrent[s]) > BS_CURRENT_MEASUREMENT_RESPONSE_TIMEOUT_ms) {
                DIAG_Handler(DIAG_ID_CURRENT_SENSOR_RESPONDING, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
            } else {
                DIAG_Handler(DIAG_ID_CURRENT_SENSOR_RESPONDING, DIAG_EVENT_OK, DIAG_STRING, s);
                if (can_state.currentSensorPresent[s] == false) {
                    CAN_SetCurrentSensorPresent(true, s);
                }
            }
        }

        /* check time stamps of CC measurements */
        /* if timestamp_cc != 0, this means current sensor cc message has been received at least once */
        if (currentTab.timestampCurrentCounting[s] != 0) {
            if ((currentTime - currentTab.timestampCurrentCounting[s]) >
                BS_COULOMB_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms) {
                DIAG_Handler(DIAG_ID_CURRENT_SENSOR_CC_RESPONDING, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
            } else {
                DIAG_Handler(DIAG_ID_CURRENT_SENSOR_CC_RESPONDING, DIAG_EVENT_OK, DIAG_STRING, s);
                if (can_state.currentSensorCCPresent[s] == false) {
                    CAN_SetCurrentSensorCcPresent(true, s);
                }
            }
        }

        /* check time stamps of EC measurements */
        /* if timestamp_ec != 0, this means current sensor ec message has been received at least once */
        if (currentTab.timestampEnergyCounting[s] != 0) {
            if ((currentTime - currentTab.timestampEnergyCounting[s]) >
                BS_ENERGY_COUNTING_MEASUREMENT_RESPONSE_TIMEOUT_ms) {
                DIAG_Handler(DIAG_ID_CURRENT_SENSOR_EC_RESPONDING, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
            } else {
                DIAG_Handler(DIAG_ID_CURRENT_SENSOR_EC_RESPONDING, DIAG_EVENT_OK, DIAG_STRING, s);
                if (can_state.currentSensorECPresent[s] == false) {
                    CAN_SetCurrentSensorEcPresent(true, s);
                }
            }
        }
    }
}

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
#endif /* BS_CURRENT_SENSOR_PRESENT == true */

static void CAN_TxInterrupt(canBASE_t *pNode, uint32 messageBox) {
    /* AXIVION Routine Generic-MissingParameterAssert: pNode: unused parameter */
    /* AXIVION Routine Generic-MissingParameterAssert: messageBox: unused parameter */
    (void)pNode;
    (void)messageBox;
}

/*========== Extern Function Implementations ================================*/

extern void CAN_Initialize(void) {
    canInit();
    /* This function overwrites HALCoGen configuration mailbox configuration 61 - 64 for CAN1 and CAN2. */
    CAN_ConfigureRxMailboxesForExtendedIdentifiers();
    /* PEX pins are used for transceiver configuration -> I2C and port expander
     * needs to be initialized previously for a successful initialization. */
    CAN_InitializeTransceiver();
    can_counterResetValue = CAN_CalculateCounterResetValue();
    CAN_ValidateConfiguredTxMessagePeriod();
    CAN_ValidateConfiguredTxMessagePhase();
    CAN_CheckDatabaseNullPointer(can_kShim);
}

extern STD_RETURN_TYPE_e CAN_DataSend(CAN_NODE_s *pNode, uint32_t id, CAN_IDENTIFIER_TYPE_e idType, uint8 *pData) {
    FAS_ASSERT(pNode != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: id: parameter accepts whole range */
    FAS_ASSERT((idType == CAN_STANDARD_IDENTIFIER_11_BIT) || (idType == CAN_EXTENDED_IDENTIFIER_29_BIT));
    FAS_ASSERT(pData != NULL_PTR);
    FAS_ASSERT((pNode == CAN_NODE_1) || (pNode == CAN_NODE_2));

    STD_RETURN_TYPE_e result = STD_NOT_OK;

    /**
     *  Parse all TX message boxes until we find a free one,
     *  then use it to send the CAN message.
     *  In the HAL, message box numbers start from 1, not 0.
     */
    for (uint8_t messageBox = 1u; messageBox <= CAN_NR_OF_TX_MESSAGE_BOX; messageBox++) {
        if (canIsTxMessagePending(pNode->canNodeRegister, messageBox) == 0u) {
            /* id shifted by 18 to use standard frame */
            /* standard frame: bits [28:18] */
            /* extended frame: bits [28:0] */
            /* bit 29 set to 1: to set direction Tx in IF2ARB register */
            /* bit 30 set to 1: 29-bit ("extended") identifier is used for this message object */
            if (idType == CAN_STANDARD_IDENTIFIER_11_BIT) {
                canUpdateID(
                    pNode->canNodeRegister,
                    messageBox,
                    ((id << CAN_IF2ARB_STANDARD_IDENTIFIER_SHIFT) | CAN_IF2ARB_SET_TX_DIRECTION |
                     CAN_IF2ARB_USE_STANDARD_IDENTIFIER));
            } else {
                /* Extended 29-bit identifier is used for this node */
                canUpdateID(
                    pNode->canNodeRegister,
                    messageBox,
                    ((id << CAN_IF2ARB_EXTENDED_IDENTIFIER_SHIFT) | CAN_IF2ARB_SET_TX_DIRECTION |
                     CAN_IF2ARB_USE_EXTENDED_IDENTIFIER));
            }
            canTransmit(pNode->canNodeRegister, messageBox, pData);
            result = STD_OK;
            break;
        }
    }
    return result;
}

#if !defined(UNITY_UNIT_TEST) || defined(COMPILE_FOR_UNIT_TEST)
/* in the unit test case we mock 'HL_can.h', so we have an implementation */
extern void canMessageNotification(canBASE_t *node, uint32 messageBox) {
    /* AXIVION Routine Generic-MissingParameterAssert: node: unchecked in interrupt */
    /* AXIVION Routine Generic-MissingParameterAssert: messageBox: unchecked in interrupt */

    if (messageBox <= CAN_NR_OF_TX_MESSAGE_BOX) {
        CAN_TxInterrupt(node, messageBox);
    } else {
        CAN_RxInterrupt(node, messageBox);
    }
}
#endif

extern void CAN_SendMessagesFromQueue(void) {
    CAN_BUFFER_ELEMENT_s message = {NULL_PTR, 0u, CAN_INVALID_TYPE, {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}};

    while (OS_ReceiveFromQueue(ftsk_canTxUnsentMessagesQueue, (void *)&message, 0u) == OS_SUCCESS) {
        /* Queue was not empty */
        if (CAN_DataSend(message.canNode, message.id, message.idType, message.data) == STD_NOT_OK) {
            /* Message was not sent */
            if (OS_SendToBackOfQueue(ftsk_canTxUnsentMessagesQueue, (void *)&message, 0u) == OS_FAIL) {
                /* Queue is full. */
            }
            break;
        }
    }
}

extern void CAN_MainFunction(void) {
    CAN_CheckCanTiming();
    if (can_state.periodicEnable == true) {
        CAN_PeriodicTransmit();
    }
}

extern void CAN_ReadRxBuffer(void) {
    if (ftsk_allQueuesCreated == true) {
        CAN_BUFFER_ELEMENT_s can_rxBuffer = {NULL_PTR, 0u, CAN_INVALID_TYPE, {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u}};
        while (OS_ReceiveFromQueue(ftsk_canRxQueue, (void *)&can_rxBuffer, 0u) == OS_SUCCESS) {
            /* data queue was not empty */
            for (uint16_t i = 0u; i < can_rxMessagesLength; i++) {
                if ((can_rxBuffer.canNode == can_rxMessages[i].canNode) &&
                    (can_rxBuffer.id == can_rxMessages[i].message.id) &&
                    (can_rxBuffer.idType == can_rxMessages[i].message.idType)) {
                    if (can_rxMessages[i].callbackFunction != NULL_PTR) {
                        can_rxMessages[i].callbackFunction(can_rxMessages[i].message, can_rxBuffer.data, &can_kShim);
                    }
                }
            }
        }
    }
}

extern void CAN_EnablePeriodic(bool command) {
    if (command == true) {
        can_state.periodicEnable = true;
    } else {
        can_state.periodicEnable = false;
    }
}

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

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern CAN_STATE_s *TEST_CAN_GetCANState(void) {
    return &can_state;
}
extern void TEST_CAN_ValidateConfiguredTxMessagePeriod(void) {
    CAN_ValidateConfiguredTxMessagePeriod();
}
extern void TEST_CAN_ValidateConfiguredTxMessagePhase(void) {
    CAN_ValidateConfiguredTxMessagePhase();
}
extern void TEST_CAN_CheckDatabaseNullPointer(CAN_SHIM_s canShim) {
    CAN_CheckDatabaseNullPointer(canShim);
}
extern void TEST_CAN_TxInterrupt(canBASE_t *pNode, uint32 messageBox) {
    CAN_TxInterrupt(pNode, messageBox);
}
extern void TEST_CAN_RxInterrupt(canBASE_t *pNode, uint32 messageBox) {
    CAN_RxInterrupt(pNode, messageBox);
}
extern STD_RETURN_TYPE_e TEST_CAN_PeriodicTransmit(void) {
    return CAN_PeriodicTransmit();
}
extern uint32_t TEST_CAN_CalculateCounterResetValue(void) {
    return CAN_CalculateCounterResetValue();
}
extern void TEST_CAN_CheckCanTiming(void) {
    CAN_CheckCanTiming();
}
extern bool TEST_CAN_IsMessagePeriodElapsed(uint32_t ticksSinceStart, uint16_t messageIndex) {
    return CAN_IsMessagePeriodElapsed(ticksSinceStart, messageIndex);
}
#if BS_CURRENT_SENSOR_PRESENT == true
extern void TEST_CAN_SetCurrentSensorPresent(bool command, uint8_t stringNumber) {
    CAN_SetCurrentSensorPresent(command, stringNumber);
}
extern void TEST_CAN_SetCurrentSensorCcPresent(bool command, uint8_t stringNumber) {
    CAN_SetCurrentSensorCcPresent(command, stringNumber);
}
extern void TEST_CAN_SetCurrentSensorEcPresent(bool command, uint8_t stringNumber) {
    CAN_SetCurrentSensorEcPresent(command, stringNumber);
}
extern void TEST_CAN_CheckCanTimingOfCurrentSensor(void) {
    CAN_CheckCanTimingOfCurrentSensor();
}
#endif /* BS_CURRENT_SENSOR_PRESENT == true */
extern void TEST_CAN_ConfigureRxMailboxesForExtendedIdentifiers(void) {
    CAN_ConfigureRxMailboxesForExtendedIdentifiers();
}
extern void TEST_CAN_InitializeTransceiver(void) {
    CAN_InitializeTransceiver();
}
extern CAN_NODE_s *TEST_CAN_GetNodeConfigurationStructFromRegisterAddress(canBASE_t *pNodeRegister) {
    return CAN_GetNodeConfigurationStructFromRegisterAddress(pNodeRegister);
}
#endif
