/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    can.c
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  CAN
 *
 * @brief   Driver for the CAN module
 *
 * @details Implementation of the CAN Interrupts, initialization, buffers,
 *          receive and transmit interfaces.
 */

/*========== Includes =======================================================*/
#include "general.h"

#include "can.h"

#include "HL_het.h"
#include "HL_reg_system.h"

#include "can_helper.h"
#include "database.h"
#include "diag.h"
#include "ftask.h"
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
 *      object is set (if RmtEn = 1).
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

/** initialize the SPI interface to the CAN transceiver */
static void CAN_InitializeTransceiver(void);

/** checks that the configured message period for Tx messages is valid */
static void CAN_ValidateConfiguredTxMessagePeriod(void);

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
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER2, CAN1_ENABLE_PIN);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER2, CAN1_STANDBY_PIN);
    PEX_SetPin(PEX_PORT_EXPANDER2, CAN1_ENABLE_PIN);
    PEX_SetPin(PEX_PORT_EXPANDER2, CAN1_STANDBY_PIN);

    /** Initialize transceiver for CAN2 */
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER2, CAN2_ENABLE_PIN);
    PEX_SetPinDirectionOutput(PEX_PORT_EXPANDER2, CAN2_STANDBY_PIN);
    PEX_SetPin(PEX_PORT_EXPANDER2, CAN2_ENABLE_PIN);
    PEX_SetPin(PEX_PORT_EXPANDER2, CAN2_STANDBY_PIN);
}

static void CAN_ValidateConfiguredTxMessagePeriod(void) {
    for (uint16_t i = 0u; i < can_txLength; i++) {
        if (can_txMessages[i].timing.period == 0u) {
            FAS_ASSERT(FAS_TRAP);
        }
    }
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

    for (uint16_t i = 0u; i < can_txLength; i++) {
        if (((counterTicks * CAN_TICK_ms) % (can_txMessages[i].timing.period)) == can_txMessages[i].timing.phase) {
            if (can_txMessages[i].callbackFunction != NULL_PTR) {
                can_txMessages[i].callbackFunction(
                    can_txMessages[i].message, data, can_txMessages[i].pMuxId, &can_kShim);
                /* CAN messages are currently discarded if all message boxes
                 * are full. They will not be retransmitted within the next
                 * call of CAN_PeriodicTransmit() */
                CAN_DataSend(
                    can_txMessages[i].canNode, can_txMessages[i].message.id, can_txMessages[i].message.idType, data);
                retVal = STD_OK;
            }
        }
    }

    counterTicks++;
    return retVal;
}

static void CAN_CheckCanTiming(void) {
    uint32_t currentTime;
    DATA_BLOCK_ERROR_STATE_s errorFlagsTab     = {.header.uniqueId = DATA_BLOCK_ID_ERROR_STATE};
    DATA_BLOCK_STATE_REQUEST_s stateRequestTab = {.header.uniqueId = DATA_BLOCK_ID_STATE_REQUEST};

    currentTime = OS_GetTickCount();
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
                DIAG_Handler(DIAG_ID_CAN_CC_RESPONDING, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
            } else {
                DIAG_Handler(DIAG_ID_CAN_CC_RESPONDING, DIAG_EVENT_OK, DIAG_STRING, s);
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
                DIAG_Handler(DIAG_ID_CAN_EC_RESPONDING, DIAG_EVENT_NOT_OK, DIAG_STRING, s);
            } else {
                DIAG_Handler(DIAG_ID_CAN_EC_RESPONDING, DIAG_EVENT_OK, DIAG_STRING, s);
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
    CAN_ValidateConfiguredTxMessagePeriod();
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
            for (uint16_t i = 0u; i < can_rxLength; i++) {
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

/** called in case of CAN interrupt, defined as weak in HAL */
/* 'extern'  is omitted here (as opposed to how the foxBMS project declares
   and defines functions) as the TI HAL convention does omit the keyword here
   and we want to stay consistent with TI's declaration.) */
/* AXIVION Next Codeline Style Linker-Multiple_Definition: TI HAL only provides a weak implementation */
void UNIT_TEST_WEAK_IMPL canMessageNotification(canBASE_t *node, uint32 messageBox) {
    /* AXIVION Routine Generic-MissingParameterAssert: node: unchecked in interrupt */
    /* AXIVION Routine Generic-MissingParameterAssert: messageBox: unchecked in interrupt */

    if (messageBox <= CAN_NR_OF_TX_MESSAGE_BOX) {
        CAN_TxInterrupt(node, messageBox);
    } else {
        CAN_RxInterrupt(node, messageBox);
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern CAN_STATE_s *TEST_CAN_GetCANState(void) {
    return &can_state;
}
#endif
