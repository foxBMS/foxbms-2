/*
* Copyright (C) 2009-2018 Texas Instruments Incorporated - www.ti.com
*
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

/**
 * @file    emac-low-level.h
 * @date    2025-09-15 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS
 * @prefix  EMAC
 *
 * @brief   Implementation of emac driver
 * @details  Implements the low level register access.
 */

#ifndef FOXBMS__EMAC_LOW_LEVEL_H_
#define FOXBMS__EMAC_LOW_LEVEL_H_

/*
 * The origin of this header is the file 'HL_emac.h' created by HALCoGen
 * 04.07.01 when selecting tab 'TMS570LC4357ZWT_FreeRTOS' -> 'DRIVER ENABLE'
 * and then choosing the option 'EMAC'.
 * Fraunhofer IISB changed the file in order to support the DP83869 PHY
 * as follows:
* - Split the driver into low level and high level driver
 * - Change used types to conform to MISRA-C
 * - Use foxBMS style naming conventions (e.g., function names follow the
 *   Verb-Noun-pattern, variable names do **not** use underscore, do not
 *   abbreviate)
 * - Use foxBMS style doxygen comments
 * - Use foxBMS style prefixes
 */

/* cspell:ignore CPPI GMII MACSRCADDRHI MACSRCADDRLO MDIOCONTROL RXHDP */
/* cspell:ignore RXPULSE TXHDP TXPULSE */

/*========== Includes =======================================================*/

#include "HL_sys_common.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/*
 * Macros to indicate EMAC Channel Numbers
 */
/**@{*/
#define EMAC_CHANNEL_0 (0x00000000u)
#define EMAC_CHANNEL_1 (0x00000001u)
#define EMAC_CHANNEL_2 (0x00000002u)
#define EMAC_CHANNEL_3 (0x00000003u)
#define EMAC_CHANNEL_4 (0x00000004u)
#define EMAC_CHANNEL_5 (0x00000005u)
#define EMAC_CHANNEL_6 (0x00000006u)
#define EMAC_CHANNEL_7 (0x00000007u)
/**@}*/

/** Macros for Configuration Value Registers */
/**@{*/
#define EMAC_MACSRCADDRHI_CONFIGVALUE                                                                              \
    ((uint32_t)((uint32_t)0x03u << 24u) | (uint32_t)((uint32_t)0xEEU << 16u) | (uint32_t)((uint32_t)0x08u << 8u) | \
     (uint32_t)((uint32_t)0x0u))
#define EMAC_MACSRCADDRLO_CONFIGVALUE ((uint32_t)((uint32_t)0x6Cu << 8u) | (uint32_t)((uint32_t)0xA6u))
#define EMAC_MDIOCONTROL_CONFIGVALUE  0x4114001Fu
/**@}*/

/** Macros which can be used as duplexMode parameter to the API EMAC_SetDuplexMode */
/**@{*/
#define EMAC_DUPLEX_FULL (0x00000001u)
#define EMAC_DUPLEX_HALF (0x00000000u)
/**@}*/

/** Macros which can be used as matchFilt parameters to the API EMAC_SetMacAddress */
/**@{*/
/* Address not used to match/filter incoming packets */
#define EMAC_MAC_ADDRESS_NO_MATCH_NO_FILTER (0x00000000u)
/* Address will be used to filter incoming packets */
#define EMAC_MAC_ADDRESS_FILTER (0x00100000u)
/* Address will be used to match incoming packets */
#define EMAC_MAC_ADDRESS_MATCH (0x00180000u)
/**@}*/

/** Macros which can be passed as eoiFlag to EMACRxIntAckToClear API */
/**@{*/
#define EMAC_INT_CORE0_RX (0x1u)
#define EMAC_INT_CORE1_RX (0x5u)
#define EMAC_INT_CORE2_RX (0x9u)
/**@}*/

/** Macros which can be passed as eoiFlag to EMACTxIntAckToClear API */
/**@{*/
#define EMAC_INT_CORE0_TX (0x2u)
#define EMAC_INT_CORE1_TX (0x6u)
#define EMAC_INT_CORE2_TX (0xAu)
/**@}*/

/** Structure to hold the values of the EMAC Configuration Registers */
typedef struct {
    /* EMAC Module Register Values */
    uint32_t txControl;            /* Transmit Control Register. */
    uint32_t rxControl;            /* Receive Control Register */
    uint32_t txInterruptMaskSet;   /* Transmit Interrupt Mask Set Register */
    uint32_t txInterruptMaskClear; /* Transmit Interrupt Clear Register */
    uint32_t rxInterruptMaskSet;   /* Receive Interrupt Mask Set Register */
    uint32_t rxInterruptMaskClear; /* Receive Interrupt Mask Clear Register*/
    uint32_t macSourceAddressHigh; /* MAC Source Address High Bytes Register*/
    uint32_t macSourceAddressLow;  /* MAC Source Address Low Bytes Register*/

    /*MDIO Module Registers */
    uint32_t mdioControl; /*MDIO Control Register. */

    /* EMAC Control Module Registers */
    uint32_t c0RxEnable; /*EMAC Control Module Receive Interrupt Enable Register*/
    uint32_t c0TxEnable; /*EMAC Control Module Transmit Interrupt Enable Register*/
} EMAC_CONFIGURATION_REGISTER_s;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Set the duplex mode of operation(full/half) for MAC.
 * @param   emacBase    Base address of the EMAC Module registers.
 * @param   duplexMode  duplex mode of operation duplexMode can take the
 *                      following values.
 *                      - EMAC_DUPLEX_FULL - Full Duplex
 *                      - EMAC_DUPLEX_HALF - Half Duplex.
 **/
extern void EMAC_SetDuplexMode(uint32_t emacBase, uint32_t duplexMode);

/**
 * @brief   Initialize the EMAC and EMAC Control modules.
 * @details The EMAC Control module is reset, the CPPI RAM is cleared.
 *          Also, all the interrupts are disabled. This API does not enable any
 *          interrupt or operation of the EMAC.
 * @param   emacCtrlBase    Base Address of the EMAC Control module
 *                          registers.
 * @param   emacBase        Base address of the EMAC module registers
 **/
extern void EMAC_InitializeRegisters(uint32_t emacCtrlBase, uint32_t emacBase);

/**
 * @brief   Set the MAC Address in MACSRCADDR registers.
 * @param   emacBase    Base Address of the EMAC module registers.
 * @param   macAddr     Start address of a MAC address array.
 *                      The array[0] shall be the LSB of the MAC address
 **/
extern void EMAC_SetMacSourceAddress(uint32_t emacBase, uint8_t macAddr[6]);

/**
 * @brief   Set the MAC Address in MACADDR registers.
 * @param   emacBase    Base Address of the EMAC module registers.
 * @param   channel     Channel Number
 * @param   matchFilt   Match or Filter
 * @param   macAddr     Start address of a MAC address array.
 *                      The array[0] shall be the LSB of the MAC address
 *          matchFilt can take the following values
 *          - EMAC_MAC_ADDRESS_NO_MATCH_NO_FILTER - Address is not used to match
 *              or filter incoming packet.
 *          - EMAC_MAC_ADDRESS_FILTER - Address is used to filter incoming packets
 *          - EMAC_MAC_ADDRESS_MATCH - Address is used to match incoming packets
 **/
extern void EMAC_SetMacAddress(uint32_t emacBase, uint32_t channel, uint8_t macAddr[6], uint32_t matchFilt);

/**
 * @brief   Write the TX Completion Pointer for a specific channel
 * @param   emacBase    Base Address of the EMAC module registers.
 * @param   channel     Channel Number.
 * @param   comPtr      Completion pointer Value to be written
 **/
extern void EMAC_WriteTxCompletionPointer(uint32_t emacBase, uint32_t channel, intptr_t comPtr);

/**
 * @brief   Write the the RX Completion Pointer for a specific channel
 * @param   emacBase    Base Address of the EMAC module registers.
 * @param   channel     Channel Number.
 * @param   comPtr      Completion pointer Value to be written
 **/
extern void EMAC_WriteRxCompletionPointer(uint32_t emacBase, uint32_t channel, uint32_t comPtr);

/**
 * @brief   Acknowledge an interrupt processed to the EMAC Control Core.
 * @param   emacBase    Base Address of the EMAC module registers.
 * @param   eoiFlag     Type of interrupt to acknowledge to the EMAC Control
 *                      module. eoiFlag can take the following values:
 *                      - EMAC_INT_CORE0_TX - Core 0 TX Interrupt
 *                      - EMAC_INT_CORE1_TX - Core 1 TX Interrupt
 *                      - EMAC_INT_CORE2_TX - Core 2 TX Interrupt
 *                      - EMAC_INT_CORE0_RX - Core 0 RX Interrupt
 *                      - EMAC_INT_CORE1_RX - Core 1 RX Interrupt
 *                      - EMAC_INT_CORE2_RX - Core 2 RX Interrupt
 **/
extern void EMAC_AcknowledgeControlCoreInterrupt(uint32_t emacBase, uint32_t eoiFlag);

/**
 * @brief   Write the TX HDP register.
 * @details If transmit is enabled, write to the TX HDP will immediately start
 *          transmission.
 *          The data will be taken from the buffer pointer of the TX buffer
 *          descriptor written to the TX HDP.
 * @param   emacBase    Base Address of the EMAC Module Registers.
 * @param   descHdr     Address of the TX buffer descriptor
 * @param   channel     Channel Number
 **/
extern void EMAC_WriteTxHeaderDescriptorPointer(uint32_t emacBase, intptr_t descHdr, uint32_t channel);

/**
 * @brief   Write the RX HDP register.
 * @details If receive is enabled, write to the RX HDP will enable data
 *          reception to point to the corresponding RX buffer descriptor's
 *          buffer pointer.
 * @param   emacBase    Base Address of the EMAC Module Registers.\n
 * @param   descHdr     Address of the RX buffer descriptor
 * @param   channel     Channel Number
 **/

extern void EMAC_WriteRxHeaderDescriptorPointer(uint32_t emacBase, uint32_t descHdr, uint32_t channel);
/**
 * @brief   Enable the TXPULSE Interrupt Generation.
 * @param   emacBase      Base address of the EMAC Module registers.
 * @param   emacCtrlBase  Base address of the EMAC CONTROL module registers
 * @param   ctrlCore      Channel number for which the interrupt to be enabled
 *                        in EMAC Control module
 * @param   channel       Channel number for which interrupt to be enabled
 **/
extern void EMAC_EnableTxInterruptPulse(uint32_t emacBase, uint32_t emacCtrlBase, uint32_t ctrlCore, uint32_t channel);

/**
 * @brief   Disable the TXPULSE Interrupt Generation.
 * @param   emacBase      Base address of the EMAC Module registers.
 * @param   emacCtrlBase  Base address of the EMAC CONTROL module registers
 * @param   ctrlCore      Channel number for which the interrupt to be enabled
 *                        in EMAC Control module
 * @param   channel       Channel number for which interrupt to be disabled
 **/
extern void EMAC_DisableTxInterruptPulse(uint32_t emacBase, uint32_t emacCtrlBase, uint32_t ctrlCore, uint32_t channel);

/**
 * @brief   Enable the RXPULSE Interrupt Generation.
 * @param   emacBase      Base address of the EMAC Module registers.
 * @param   emacCtrlBase  Base address of the EMAC CONTROL module registers
 * @param   ctrlCore      Control core for which the interrupt to be enabled.
 * @param   channel       Channel number for which interrupt to be enabled
 **/
extern void EMAC_EnableRxInterruptPulse(uint32_t emacBase, uint32_t emacCtrlBase, uint32_t ctrlCore, uint32_t channel);

/**
 * @brief   Disable the RXPULSE Interrupt Generation.
 * @param   emacBase        Base address of the EMAC Module registers.
 * @param   emacCtrlBase    Base address of the EMAC CONTROL module registers
 * @param   ctrlCore        Control core for which the interrupt to be disabled
 * @param   channel         Channel number for which interrupt to be disabled
 **/
extern void EMAC_DisableRxInterruptPulse(uint32_t emacBase, uint32_t emacCtrlBase, uint32_t ctrlCore, uint32_t channel);

/**
 * @brief   Set the GMII bit, RX and TX are enabled for receive and transmit.
 *          Note: This is not the API to enable MII.
 * @param   emacBase    Base address of the EMAC Module registers.
 **/
extern void EMAC_EnableMii(uint32_t emacBase);

/**
 * @brief   Clear the GMII bit, Rx and Tx are held in reset.
 *          Note: This is not the API to disable MII.
 * @param   emacBase    Base address of the EMAC Module registers.
 **/
extern void EMAC_DisableMii(uint32_t emacBase);

/**
 * @brief   Enable the transmit in the TX Control Register
 * @details After the transmit is enabled, any write to TXHDP of a channel will
 *          start transmission.
 * @param   emacBase    Base Address of the EMAC Module Registers.
 **/
extern void EMAC_EnableTx(uint32_t emacBase);

/**
 * @brief   Disable the transmit in the TX Control Register
 * @param   emacBase      Base Address of the EMAC Module Registers.
 **/
extern void EMAC_DisableTx(uint32_t emacBase);

/**
 * @brief   Enable the receive in the RX Control Register
 * @details After the receive is enabled, and write to RXHDP of
 *          a channel, the data can be received in the destination
 *          specified by the corresponding RX buffer descriptor.
 * @param   emacBase    Base Address of the EMAC Module Registers.
 **/
extern void EMAC_EnableRx(uint32_t emacBase);

/**
 * @brief   Disable the receive in the RX Control Register
 * @param   emacBase    Base Address of the EMAC Module Registers.
 **/
extern void EMAC_DisableRx(uint32_t emacBase);

/**
 * @brief   Enable a specific channel to receive broadcast frames
 * @param   emacBase    Base Address of the EMAC module registers.
 * @param   channel     Channel Number.
 **/
extern void EMAC_EnableRxBroadCast(uint32_t emacBase, uint32_t channel);

/**
 * @brief   Disable receive broadcast frames
 * @param   emacBase    Base Address of the EMAC module registers.
 **/

extern void EMAC_DisableRxBroadCast(uint32_t emacBase);

/**
 * @brief   Enable a specific channel to receive multicast frames
 * @param   emacBase    Base Address of the EMAC module registers.
 * @param   channel     Channel Number.
 **/
extern void EMAC_EnableRxMultiCast(uint32_t emacBase, uint32_t channel);

/**
 * @brief   Disable a specific channel to receive multicast frames
 * @param   emacBase    Base Address of the EMAC module registers.
 * @param   channel     Channel Number.
 **/
extern void EMAC_DisableRxMultiCast(uint32_t emacBase, uint32_t channel);

/**
 * @brief   Enable unicast for a specific channel
 * @param   emacBase    Base Address of the EMAC module registers.
 * @param   channel     Channel Number.
 **/
extern void EMAC_SetRxUnicast(uint32_t emacBase, uint32_t channel);

/**
 * @brief   Disable unicast for a specific channel
 * @param   emacBase    Base Address of the EMAC module registers.
 * @param   channel     Channel Number.
 **/
extern void EMAC_ClearRxUnicast(uint32_t emacBase, uint32_t channel);

/**
 * @brief   Set the free buffers for a specific channel
 * @param   emacBase    Base Address of the EMAC module registers.
 * @param   channel     Channel Number.
 * @param   nBuf        Number of free buffers
 **/
extern void EMAC_SetNumberFreeBuffer(uint32_t emacBase, uint32_t channel, uint32_t nBuf);

/**
 * @brief   Perform a transmit queue teardown, that is, transmission is aborted.
 * @param   emacBase    Base Address of the EMAC module registers.
 * @param   channel     Channel Number.
 **/
extern void EMAC_TeardownTx(uint32_t emacBase, uint32_t channel);

/**
 * @brief   Perform a receive queue teardown, that is, reception is aborted.
 * @param   emacBase    Base Address of the EMAC module registers.
 * @param   channel     Channel Number.
 **/
extern void EMAC_TeardownRx(uint32_t emacBase, uint32_t channel);

/**
 * @brief   Perform a soft reset of the EMAC and EMAC Control Modules.
 * @param   emacCtrlBase  Base address of the EMAC CONTROL module registers
 * @param   emacBase      Base Address of the EMAC module registers.
 **/
extern void EMAC_SoftReset(uint32_t emacCtrlBase, uint32_t emacBase);

/**
 * @brief   Enable Idle State of the EMAC Module.
 * @param   emacBase    Base Address of the EMAC module registers.
 **/
extern void EMAC_EnableIdleState(uint32_t emacBase);

/**
 * @brief   Disable Idle State of the EMAC Module.
 * @param   emacBase      Base Address of the EMAC module registers.
 **/
extern void EMAC_DisableIdleState(uint32_t emacBase);

/**
 * @brief   Enables Loopback Mode.
 * @param   emacBase      Base Address of the EMAC module registers.
 **/
extern void EMAC_EnableLoopback(uint32_t emacBase);

/**
 * @brief   Disables Loopback Mode.
 * @param   emacBase      Base Address of the EMAC module registers.
 **/
extern void EMAC_DisableLoopback(uint32_t emacBase);

/**
* @brief    Get the initial or current values of the configuration registers
* @details  Copy the initial or current value (depending on the parameter
*           'type') of the configuration registers to the struct pointed by
*           config_reg
* @param   emacBase      Base Address of the EMAC module registers.
* @param   emacCtrlBase  Base address of the EMAC CONTROL module registers
* @param[in] config_reg pointer to the struct to which the initial or
*                       current value of the configuration registers need to be
*                       stored
* @param[in] type       whether initial or current value of the configuration
*                       registers need to be stored
*                       - InitialValue: initial value of the configuration
*                                       registers will be stored in the struct
*                                       pointed by config_reg
*                       - CurrentValue: initial value of the configuration
*                                       registers will be stored in the struct
*                                       pointed by config_reg
*/
extern void EMAC_GetConfigValue(
    uint32_t emacBase,
    uint32_t emacCtrlBase,
    EMAC_CONFIGURATION_REGISTER_s *config_reg,
    config_value_type_t type);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__EMAC_LOW_LEVEL_H_ */
