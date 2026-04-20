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
 * @file    emac-low-level.c
 * @date    2025-09-15 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS
 * @prefix  EMAC
 *
 * @brief   Implementation of emac driver
 * @details Implements the low level register access.
 */

/*
 * The origin of this source file is the file 'HL_emac.c' created by HALCoGen
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

/* cspell:ignore CPPI FULLDUPLEX GMIIE GMIIEN HWREG MACADDRHI MACADDRLO */
/* cspell:ignore MACEOIVECTOR MACSRCADDRHI MACSRCADDRLO MDIOCONTROL */
/* cspell:ignore RXBROADCH RXBROADEN RXBUFFEROFFSET RXCONTROL RXCP RXDIS */
/* cspell:ignore RXEN RXFREEBUFFER RXHDP RXINTMASKCLEAR RXINTMASKSET */
/* cspell:ignore RXMBPENABLE RXMULTCH RXMULTEN RXTEARDOWN RXUNICASTCLEAR */
/* cspell:ignore RXUNICASTSET TXCP TXDIS TXEN TXHDP TXTEARDOWN */

/*========== Includes =======================================================*/
#include "emac-low-level.h"

/* AXIVION Disable Generic-LocalInclude: HL_emac.c is removed */
#include "HL_hw_emac.h"
#include "HL_hw_emac_ctrl.h"
#include "HL_hw_reg_access.h"
#include "HL_mdio.h"
/* AXIVION Enable Generic-LocalInclude: */

#include "fassert.h"

/*========== Macros and Definitions =========================================*/

#define EMAC_MAX_HEADER_DESC (8u)

/** defines for EMAC control register */
/**@{*/
#define EMAC_CONTROL_RESET     (0x01u)
#define EMAC_CONTROL_SOFTRESET (0x4u)
/**@}*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern void EMAC_SetDuplexMode(uint32_t emacBase, uint32_t duplexMode) {
    /* Assertions */
    FAS_ASSERT((emacBase != 0x0u) || ((emacBase < (UINT32_MAX - EMAC_MACCONTROL))));
    FAS_ASSERT((duplexMode == EMAC_DUPLEX_HALF) || (duplexMode == EMAC_DUPLEX_FULL));

    /* Clear relevant register entries */
    HWREG(emacBase + EMAC_MACCONTROL) &= (~(uint32_t)EMAC_MACCONTROL_FULLDUPLEX);
    /* Set mode */
    HWREG(emacBase + EMAC_MACCONTROL) |= duplexMode;
}

extern void EMAC_InitializeRegisters(uint32_t emacCtrlBase, uint32_t emacBase) {
    /* Assertions */
    FAS_ASSERT(emacCtrlBase != 0x0u);
    FAS_ASSERT((emacBase != 0x0u) || (emacBase < (UINT32_MAX - EMAC_CONTROL_SOFTRESET)));

    uint32_t cnt;
    /* Reset the EMAC Control Module. This clears the internal RAM also */
    HWREG(emacCtrlBase + EMAC_CONTROL_SOFTRESET) = EMAC_CONTROL_RESET;

    while ((HWREG(emacCtrlBase + EMAC_CONTROL_SOFTRESET) & EMAC_CONTROL_RESET) == EMAC_CONTROL_RESET) {
    } /* Wait */

    /* Reset the EMAC Module. This clears the internal RAM also */
    HWREG(emacBase + EMAC_SOFTRESET) = EMAC_SOFTRESET_SOFTRESET;

    while ((HWREG(emacBase + EMAC_SOFTRESET) & EMAC_SOFTRESET_SOFTRESET) == EMAC_SOFTRESET_SOFTRESET) {
    } /* Wait */

    HWREG(emacBase + EMAC_MACCONTROL) = 0u;
    HWREG(emacBase + EMAC_RXCONTROL)  = 0u;
    HWREG(emacBase + EMAC_TXCONTROL)  = 0u;

    /* Initialize all the header descriptor pointer registers */
    for (cnt = 0u; cnt < EMAC_MAX_HEADER_DESC; cnt++) {
        HWREG(emacBase + EMAC_RXHDP(cnt))        = 0u;
        HWREG(emacBase + EMAC_TXHDP(cnt))        = 0u;
        HWREG(emacBase + EMAC_RXCP(cnt))         = 0u;
        HWREG(emacBase + EMAC_TXCP(cnt))         = 0u;
        HWREG(emacBase + EMAC_RXFREEBUFFER(cnt)) = 0xFFu;
    }
    /* Clear the interrupt enable for all the channels */
    HWREG(emacBase + EMAC_TXINTMASKCLEAR) = 0xFFu;
    HWREG(emacBase + EMAC_RXINTMASKCLEAR) = 0xFFu;

    HWREG(emacBase + EMAC_MACHASH1) = 0u;
    HWREG(emacBase + EMAC_MACHASH2) = 0u;

    HWREG(emacBase + EMAC_RXBUFFEROFFSET) = 0u;
}

extern void EMAC_SetMacSourceAddress(uint32_t emacBase, uint8_t macAddr[6u]) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    FAS_ASSERT(macAddr != NULL_PTR);
    /* The MAC source address is stored in two registers: high and low. */
    /* AXIVION Disable Style Generic-NoMagicNumbers: Indexed directly by their position */
    HWREG(emacBase + EMAC_MACSRCADDRHI) =
        ((uint32_t)macAddr[5u] | ((uint32_t)macAddr[4u] << EMAC_MACADDRHI_MACADDR4_SHIFT) |
         ((uint32_t)macAddr[3u] << EMAC_MACADDRHI_MACADDR3_SHIFT) |
         ((uint32_t)macAddr[2u] << EMAC_MACADDRHI_MACADDR2_SHIFT));
    HWREG(emacBase + EMAC_MACSRCADDRLO) =
        ((uint32_t)macAddr[1u] | ((uint32_t)macAddr[0u] << EMAC_MACADDRLO_MACADDR0_SHIFT));
    /* AXIVION Enable Style Generic-NoMagicNumbers: */
}

extern void EMAC_SetMacAddress(uint32_t emacBase, uint32_t channel, uint8_t macAddr[6], uint32_t matchFilt) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    FAS_ASSERT(channel <= EMAC_CHANNEL_7);
    FAS_ASSERT(macAddr != NULL_PTR);
    FAS_ASSERT(
        (matchFilt == EMAC_MAC_ADDRESS_FILTER) || (matchFilt == EMAC_MAC_ADDRESS_MATCH) ||
        (matchFilt == EMAC_MAC_ADDRESS_NO_MATCH_NO_FILTER));

    /* Set the mac address for the individual channel */
    HWREG(emacBase + EMAC_MACINDEX) = channel;
    /* The MAC source address is stored in two registers: high and low. */
    /* AXIVION Disable Style Generic-NoMagicNumbers: Indexed directly by their position */
    HWREG(emacBase + EMAC_MACADDRHI) =
        ((uint32_t)macAddr[5u] | ((uint32_t)macAddr[4u] << EMAC_MACADDRHI_MACADDR4_SHIFT) |
         ((uint32_t)macAddr[3u] << EMAC_MACADDRHI_MACADDR3_SHIFT) |
         ((uint32_t)macAddr[2u] << EMAC_MACADDRHI_MACADDR2_SHIFT));
    HWREG(emacBase + EMAC_MACADDRLO) =
        ((uint32_t)macAddr[1u] | ((uint32_t)macAddr[0u] << EMAC_MACADDRLO_MACADDR0_SHIFT) | matchFilt |
         (channel << EMAC_MACADDRLO_CHANNEL_SHIFT));
    /* AXIVION Enable Style Generic-NoMagicNumbers: */
}

extern void EMAC_WriteTxCompletionPointer(uint32_t emacBase, uint32_t channel, intptr_t comPtr) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    FAS_ASSERT(channel <= EMAC_CHANNEL_7);
    FAS_ASSERT(comPtr != 0x0u);
    /* Set the corresponding register value. */
    HWREG(emacBase + EMAC_TXCP(channel)) = comPtr;
}

extern void EMAC_WriteRxCompletionPointer(uint32_t emacBase, uint32_t channel, uint32_t comPtr) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    FAS_ASSERT(channel <= EMAC_CHANNEL_7);
    FAS_ASSERT(comPtr != 0x0u);
    /* Set the corresponding register value. */
    HWREG(emacBase + EMAC_RXCP(channel)) = comPtr;
}

extern void EMAC_AcknowledgeControlCoreInterrupt(uint32_t emacBase, uint32_t eoiFlag) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    FAS_ASSERT(eoiFlag != 0x0u);
    /* Acknowledge the EMAC Control Core */
    HWREG(emacBase + EMAC_MACEOIVECTOR) = eoiFlag;
}

extern void EMAC_WriteTxHeaderDescriptorPointer(uint32_t emacBase, intptr_t descHdr, uint32_t channel) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    FAS_ASSERT(descHdr != 0x0u);
    FAS_ASSERT(channel <= EMAC_CHANNEL_7);
    /* Set the corresponding register value. */
    HWREG(emacBase + EMAC_TXHDP(channel)) = descHdr;
}

extern void EMAC_WriteRxHeaderDescriptorPointer(uint32_t emacBase, uint32_t descHdr, uint32_t channel) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    /* AXIVION Routine Generic-MissingParameterAssert: descHdr: parameter accepts whole range */
    FAS_ASSERT(channel <= EMAC_CHANNEL_7);
    /* Set the corresponding register value. */
    HWREG(emacBase + EMAC_RXHDP(channel)) = descHdr;
}

extern void EMAC_EnableTxInterruptPulse(uint32_t emacBase, uint32_t emacCtrlBase, uint32_t ctrlCore, uint32_t channel) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    FAS_ASSERT(emacCtrlBase != 0x0u);
    /* AXIVION Routine Generic-MissingParameterAssert: ctrlCore: parameter accepts whole range */
    FAS_ASSERT(channel <= EMAC_CHANNEL_7);

    /* Set the mask for the specific channel*/
    HWREG(emacBase + EMAC_TXINTMASKSET) |= ((uint32_t)1u << channel);
    /* Set the corresponding register value. */
    HWREG(emacCtrlBase + EMAC_CTRL_CnTXEN(ctrlCore)) |= ((uint32_t)1u << channel);
}

extern void EMAC_DisableTxInterruptPulse(
    uint32_t emacBase,
    uint32_t emacCtrlBase,
    uint32_t ctrlCore,
    uint32_t channel) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    FAS_ASSERT(emacCtrlBase != 0x0u);
    /* AXIVION Routine Generic-MissingParameterAssert: ctrlCore: parameter accepts whole range */
    FAS_ASSERT(channel <= EMAC_CHANNEL_7);

    /* Set the mask for the specific channel*/
    HWREG(emacBase + EMAC_TXINTMASKCLEAR) |= ((uint32_t)1u << channel);
    /* Set the corresponding register value. */
    HWREG(emacCtrlBase + EMAC_CTRL_CnTXEN(ctrlCore)) &= (~((uint32_t)1u << channel));
}

extern void EMAC_EnableRxInterruptPulse(uint32_t emacBase, uint32_t emacCtrlBase, uint32_t ctrlCore, uint32_t channel) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    FAS_ASSERT(emacCtrlBase != 0x0u);
    /* AXIVION Routine Generic-MissingParameterAssert: ctrlCore: parameter accepts whole range */
    FAS_ASSERT(channel <= EMAC_CHANNEL_7);

    /* Set the mask for the specific channel*/
    HWREG(emacBase + EMAC_RXINTMASKSET) |= ((uint32_t)1u << channel);
    /* Set the corresponding register value. */
    HWREG(emacCtrlBase + EMAC_CTRL_CnRXEN(ctrlCore)) |= ((uint32_t)1u << channel);
}

extern void EMAC_DisableRxInterruptPulse(
    uint32_t emacBase,
    uint32_t emacCtrlBase,
    uint32_t ctrlCore,
    uint32_t channel) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    FAS_ASSERT(emacCtrlBase != 0x0u);
    /* AXIVION Routine Generic-MissingParameterAssert: ctrlCore: parameter accepts whole range */
    FAS_ASSERT(channel <= EMAC_CHANNEL_7);

    /* Set the mask for the specific channel*/
    HWREG(emacBase + EMAC_RXINTMASKCLEAR) |= ((uint32_t)1u << channel);
    /* Set the corresponding register value. */
    HWREG(emacCtrlBase + EMAC_CTRL_CnRXEN(ctrlCore)) &= (~((uint32_t)1u << channel));
}

extern void EMAC_EnableMii(uint32_t emacBase) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);

    /* Set the corresponding register value. */
    HWREG(emacBase + EMAC_MACCONTROL) |= EMAC_MACCONTROL_GMIIEN;
}

extern void EMAC_DisableMii(uint32_t emacBase) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);

    /* Set the corresponding register value. */
    HWREG(emacBase + EMAC_MACCONTROL) &= (~(uint32_t)EMAC_MACCONTROL_GMIIEN);
}

extern void EMAC_EnableTx(uint32_t emacBase) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);

    /* Set the corresponding register value. */
    HWREG(emacBase + EMAC_TXCONTROL) = EMAC_TXCONTROL_TXEN;
}

extern void EMAC_DisableTx(uint32_t emacBase) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);

    /* Set the corresponding register value. */
    HWREG(emacBase + EMAC_TXCONTROL) = EMAC_TXCONTROL_TXDIS;
}

extern void EMAC_EnableRx(uint32_t emacBase) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);

    /* Set the corresponding register value. */
    HWREG(emacBase + EMAC_RXCONTROL) = EMAC_RXCONTROL_RXEN;
}

extern void EMAC_DisableRx(uint32_t emacBase) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);

    /* Set the corresponding register value. */
    HWREG(emacBase + EMAC_RXCONTROL) = EMAC_RXCONTROL_RXDIS;
}

extern void EMAC_EnableRxBroadCast(uint32_t emacBase, uint32_t channel) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    FAS_ASSERT(channel <= EMAC_CHANNEL_7);

    /* Enable Channel */
    HWREG(emacBase + EMAC_RXMBPENABLE) &= (~(uint32_t)EMAC_RXMBPENABLE_RXBROADCH);
    /* Set the corresponding register value. */
    HWREG(emacBase + EMAC_RXMBPENABLE) |=
        ((uint32_t)EMAC_RXMBPENABLE_RXBROADEN | ((uint32_t)channel << (uint32_t)EMAC_RXMBPENABLE_RXBROADCH_SHIFT));
}

extern void EMAC_DisableRxBroadCast(uint32_t emacBase) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);

    HWREG(emacBase + EMAC_RXMBPENABLE) &= (~(uint32_t)EMAC_RXMBPENABLE_RXBROADCH);
    /* Broadcast Frames are filtered. */
    HWREG(emacBase + EMAC_RXMBPENABLE) &= (~(uint32_t)EMAC_RXMBPENABLE_RXBROADEN);
}

extern void EMAC_EnableRxMultiCast(uint32_t emacBase, uint32_t channel) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    FAS_ASSERT(channel <= EMAC_CHANNEL_7);

    /* Enable a specific channel to receive multicast frames */
    HWREG(emacBase + EMAC_RXMBPENABLE) &= (~(uint32_t)EMAC_RXMBPENABLE_RXMULTCH);
    HWREG(emacBase + EMAC_RXMBPENABLE) |= ((uint32_t)EMAC_RXMBPENABLE_RXMULTEN | (channel));
}

extern void EMAC_DisableRxMultiCast(uint32_t emacBase, uint32_t channel) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    FAS_ASSERT(channel <= EMAC_CHANNEL_7);

    /* Enable a specific channel to receive multicast frames */
    HWREG(emacBase + EMAC_RXMBPENABLE) &= (~(uint32_t)EMAC_RXMBPENABLE_RXMULTCH);
    HWREG(emacBase + EMAC_RXMBPENABLE) &= (~(uint32_t)EMAC_RXMBPENABLE_RXMULTEN);
}

extern void EMAC_SetRxUnicast(uint32_t emacBase, uint32_t channel) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    FAS_ASSERT(channel <= EMAC_CHANNEL_7);

    /* Set the corresponding register value. */
    HWREG(emacBase + EMAC_RXUNICASTSET) |= ((uint32_t)1u << channel);
}

extern void EMAC_ClearRxUnicast(uint32_t emacBase, uint32_t channel) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    FAS_ASSERT(channel <= EMAC_CHANNEL_7);

    /* Set the corresponding register value. */
    HWREG(emacBase + EMAC_RXUNICASTCLEAR) |= ((uint32_t)1u << channel);
}

extern void EMAC_SetNumberFreeBuffer(uint32_t emacBase, uint32_t channel, uint32_t nBuf) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    FAS_ASSERT(channel <= EMAC_CHANNEL_7);
    /* AXIVION Routine Generic-MissingParameterAssert: nBuf: parameter accepts whole range */

    /* Set the corresponding register value. */
    HWREG(emacBase + EMAC_RXFREEBUFFER(channel)) = nBuf;
}

extern void EMAC_TeardownTx(uint32_t emacBase, uint32_t channel) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    FAS_ASSERT(channel <= EMAC_CHANNEL_7);

    /* Set the corresponding register value. */
    HWREG(emacBase + EMAC_TXTEARDOWN) &= (channel);
}

extern void EMAC_TeardownRx(uint32_t emacBase, uint32_t channel) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    FAS_ASSERT(channel <= EMAC_CHANNEL_7);

    /* Set the corresponding register value. */
    HWREG(emacBase + EMAC_RXTEARDOWN) &= (channel);
}

extern void EMAC_SoftReset(uint32_t emacCtrlBase, uint32_t emacBase) {
    /* Assertions */
    FAS_ASSERT(emacCtrlBase != 0x0u);
    FAS_ASSERT(emacBase != 0x0u);

    /* Reset the EMAC Control Module. This clears the CPPI RAM also */
    HWREG(emacCtrlBase + EMAC_CTRL_SOFTRESET) = EMAC_SOFTRESET_SOFTRESET;

    /* Hardware status bit read check */
    while ((HWREG(emacCtrlBase + EMAC_CTRL_SOFTRESET) & EMAC_SOFTRESET_SOFTRESET) == EMAC_SOFTRESET_SOFTRESET) {
        /* Wait for the reset to complete */
    }
    /* Reset the EMAC Module. */
    HWREG(emacBase + EMAC_SOFTRESET) = EMAC_SOFTRESET_SOFTRESET;
    /* Hardware status bit read check */
    while ((HWREG(emacBase + EMAC_SOFTRESET) & EMAC_SOFTRESET_SOFTRESET) == EMAC_SOFTRESET_SOFTRESET) {
        /* Wait for the Reset to complete */
    }
}

extern void EMAC_EnableIdleState(uint32_t emacBase) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    /* Set the corresponding register value. */
    HWREG(emacBase + EMAC_MACCONTROL) |= EMAC_MACCONTROL_CMDIDLE;
}

extern void EMAC_DisableIdleState(uint32_t emacBase) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    /* Set the corresponding register value. */
    HWREG(emacBase + EMAC_MACCONTROL) &= (~(uint32_t)(EMAC_MACCONTROL_CMDIDLE));
}

extern void EMAC_EnableLoopback(uint32_t emacBase) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);

    uint32_t GMIIENval = 0u;
    /* Store the value of GMIIEN bit before de-asserting it */
    GMIIENval = HWREG(emacBase + EMAC_MACCONTROL) & EMAC_MACCONTROL_GMIIEN;
    HWREG(emacBase + EMAC_MACCONTROL) &= (~(uint32_t)EMAC_MACCONTROL_GMIIEN);

    /* Enable Loopback */
    HWREG(emacBase + EMAC_MACCONTROL) |= EMAC_MACCONTROL_LOOPBACK;

    /* Restore the value of GMIIEN bit */
    HWREG(emacBase + EMAC_MACCONTROL) |= GMIIENval;
}

extern void EMAC_DisableLoopback(uint32_t emacBase) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);

    uint32_t GMIIENval = 0u;

    /* Store the value of GMIIEN bit before de-asserting it */
    GMIIENval = HWREG(emacBase + EMAC_MACCONTROL) & EMAC_MACCONTROL_GMIIEN;
    HWREG(emacBase + EMAC_MACCONTROL) &= (~(uint32_t)EMAC_MACCONTROL_GMIIEN);

    /* Disable Loopback */
    HWREG(emacBase + EMAC_MACCONTROL) &= (~(uint32_t)EMAC_MACCONTROL_LOOPBACK);

    /* Restore the value of GMIIEN bit */
    HWREG(emacBase + EMAC_MACCONTROL) |= GMIIENval;
}

extern void EMAC_GetConfigValue(
    uint32_t emacBase,
    uint32_t emacCtrlBase,
    EMAC_CONFIGURATION_REGISTER_s *config_reg,
    config_value_type_t type) {
    /* Assertions */
    FAS_ASSERT(emacBase != 0x0u);
    FAS_ASSERT(emacCtrlBase != 0x0u);
    FAS_ASSERT(config_reg != NULL_PTR);
    FAS_ASSERT((type == InitialValue) || (type == CurrentValue));

    if (type == InitialValue) {
        /* Store the initial values. */
        config_reg->txControl            = EMAC_TXCONTROL_TXEN;
        config_reg->rxControl            = EMAC_RXCONTROL_RXEN;
        config_reg->txInterruptMaskSet   = EMAC_TXINTMASKSET_TX0MASK;
        config_reg->txInterruptMaskClear = EMAC_TXINTMASKCLEAR_TX0MASK;
        config_reg->rxInterruptMaskSet   = EMAC_RXINTMASKSET_RX0MASK;
        config_reg->rxInterruptMaskClear = EMAC_RXINTMASKCLEAR_RX0MASK;
        config_reg->macSourceAddressHigh = EMAC_MACSRCADDRHI_CONFIGVALUE;
        config_reg->macSourceAddressLow  = EMAC_MACSRCADDRLO_CONFIGVALUE;
        config_reg->mdioControl          = EMAC_MDIOCONTROL_CONFIGVALUE;
        config_reg->c0RxEnable           = EMAC_RXCONTROL_RXEN;
        config_reg->c0TxEnable           = EMAC_TXCONTROL_TXEN;
    } else {
        /* Read out the current values */
        config_reg->txControl            = HWREG(emacBase + EMAC_TXCONTROL);
        config_reg->rxControl            = HWREG(emacBase + EMAC_RXCONTROL);
        config_reg->txInterruptMaskSet   = HWREG(emacBase + EMAC_TXINTMASKSET);
        config_reg->txInterruptMaskClear = HWREG(emacBase + EMAC_TXINTMASKCLEAR);
        config_reg->rxInterruptMaskSet   = HWREG(emacBase + EMAC_RXINTMASKSET);
        config_reg->rxInterruptMaskClear = HWREG(emacBase + EMAC_RXINTMASKCLEAR);
        config_reg->macSourceAddressHigh = HWREG(emacBase + EMAC_MACSRCADDRHI);
        config_reg->macSourceAddressLow  = HWREG(emacBase + EMAC_MACSRCADDRLO);
        config_reg->mdioControl          = HWREG(emacBase + MDIO_CONTROL);
        config_reg->c0RxEnable           = HWREG(emacCtrlBase + EMAC_CTRL_CnRXEN(EMAC_CHANNEL_0));
        config_reg->c0TxEnable           = HWREG(emacCtrlBase + EMAC_CTRL_CnTXEN(EMAC_CHANNEL_0));
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
