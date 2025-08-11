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
 * @file    dp83869.c
 * @date    2025-04-01 (date of creation)
 * @updated 2025-04-01 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  PHY
 *
 * @brief   Implementation of physical layer driver
 * @details Implements the driver for the DP83869HM PHY
 */

/*
 * The origin of this header is the file 'HL_phy_dp83640.c' created by HALCoGen
 * 04.07.01 when selecting tab 'TMS570LC4357ZWT_FreeRTOS' -> 'EMAC' -> 'PHY'
 * and then choosing the option 'DP83640'.
 * Fraunhofer IISB changed the file in order to support the DP83869 PHY
 * as follows:
 * - Change used types to conform to MISRA-C
 * - Use foxBMS style naming conventions (e.g., function names follow the
 *   Verb-Noun-pattern, variable names do **not** use underscore, do not
 *   abbreviate)
 * - Use foxBMS style doxygen comments
 * - Use foxBMS style prefixes
 */

/*========== Includes =======================================================*/

#include "dp83869.h"

#include "phy_cfg.h"

#include "HL_mdio.h"
#include "HL_sys_common.h"

#include "database.h"
#include "fassert.h"
#include "fstd_types.h"
#include "io.h"
#include "os.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/* PHY register offset definitions */
#define PHY_BMCR       (0x0u)
#define PHY_BMSR       (0x1u)
#define PHY_PHYIDR1    (0x2u)
#define PHY_PHYIDR2    (0x3u)
#define PHY_ANAR       (0x4u)
#define PHY_ALNPAR     (0x5u)
#define PHY_STATUS     (0x11u)
#define PHY_BIST       (0x16u)
#define PHY_REGCR      (0x0Du)
#define PHY_ADDAR      (0x0Eu)
#define PHY_GEN_CTRL   (0x1Fu)
#define PHY_STRAP_STS  (0x6Eu)
#define PHY_OP_MODE    (0x1DFu)
#define PHY_FX_INT_STS (0xC19u)

/* PHY status definitions */
#define PHY_ID_SHIFT                         (16u)
#define PHY_MII_RESET                        (0x8000u)
#define PHY_SW_RESET                         (0x8000u)
#define PHY_AUTONEG_ENABLE                   (0x1000u)
#define PHY_AUTONEG_RESTART                  (0x0200u)
#define PHY_AUTONEG_COMPLETE                 (0x0020u)
#define PHY_AUTONEG_INCOMPLETE               (0x0000u)
#define PHY_AUTONEG_STATUS                   (0x0020u)
#define PHY_AUTONEG_ABLE                     (0x0008u)
#define PHY_LPBK_ENABLE                      (0x4000u)
#define PHY_LINK_STATUS                      (0x0004u)
#define PHY_INVALID_TYPE                     (0x0u)
#define PHY_EXTERNAL_LOOPBACK_EN             (0x04u)
#define PHY_MII_MODE_SET                     (0x60u)
#define PHY_SWRESTART                        (0x4000u)
#define PHY_DEVAD_EXTENDED                   (0x1Fu)
#define PHY_DEVAD_EXTENDED_NO_POST_INCREMENT (0x401Fu)

/*========== Static Constant and Variable Definitions =======================*/
static DATA_BLOCK_PHY_s phy_tablePhy = {.header.uniqueId = DATA_BLOCK_ID_PHY};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Writes register from the Extended Address Space.
 * @param   mdioBaseAddress Base Address of the MDIO Module Registers.
 * @param   phyAddress      PHY Address.
 * @param   regNum          Register Number to be written.
 * @param   RegVal          Value to write in register
 **/
static void PHY_WriteExtendedAddressSpaceRegister(
    uint32_t mdioBaseAddress,
    uint32_t phyAddress,
    uint32_t regNum,
    uint16_t RegVal);

/**
 * @brief   Reads register from the Extended Address Space.
 * @details The DP83869HM SMI function supports read or write access to the
 *          extended register set using registers REGCR (0x000Dh) and ADDAR
 *          (0x000Eh) and the MDIO Manageable Device (MMD) indirect method
 *          defined in IEEE 802.3ah Draft for clause 22 for accessing the
 *          clause 45 extended register set.
 *          The standard register set, MDIO registers 0 to 31, is accessed
 *          using the normal direct-MDIO access or the indirect method, except
 *          for register REGCR (0x000Dh) and ADDAR (0x000Eh) which is accessed
 *          only using the normal MDIO transaction. The SMI function ignores
 *          indirect accesses to these registers.
 * @param   mdioBaseAddress Base Address of the MDIO Module Registers.
 * @param   phyAddress      PHY Address.
 * @param   regNum          Register Number to be read.
 * @param   pData           Pointer where the read value shall be written.
 * @return true if reading was successful, false otherwise
 **/
static bool PHY_ReadExtendedAddressSpaceRegister(
    uint32_t mdioBaseAddress,
    uint32_t phyAddress,
    uint32_t regNum,
    uint16_t *pData);
/**
 * @brief   Select the data transmission rate.
 * @param   mdioBaseAddress Base Address of the MDIO Module Registers.
 * @param   phyAddress      PHY Address.
 * @param   speed           Data tranmission rates
 */
static void PHY_SelectSpeed(uint32_t mdioBaseAddress, uint32_t phyAddress, PHY_SPEED_SEL_e speed);

/**
 * @brief   Triggers a software restart on the PHY.
 * @param   mdioBaseAddress Base Address of the MDIO Module Registers.
 * @param   phyAddress      PHY Address.
 **/
static void PHY_RestartSoftware(uint32_t mdioBaseAddress, uint32_t phyAddress);

/*========== Static Function Implementations ================================*/
static void PHY_WriteExtendedAddressSpaceRegister(
    uint32_t mdioBaseAddress,
    uint32_t phyAddress,
    uint32_t regNum,
    uint16_t RegVal) {
    /* AXIVION Routine Generic-MissingParameterAssert: mdioBaseAddress: parameter accepts whole range */
    FAS_ASSERT(phyAddress < 32u);
    FAS_ASSERT((regNum != PHY_REGCR) && (regNum != PHY_ADDAR) && (regNum <= PHY_FX_INT_STS));
    /* AXIVION Routine Generic-MissingParameterAssert: RegVal: parameter accepts whole range */

    MDIOPhyRegWrite(mdioBaseAddress, phyAddress, PHY_REGCR, PHY_DEVAD_EXTENDED);
    MDIOPhyRegWrite(mdioBaseAddress, phyAddress, PHY_ADDAR, regNum);
    MDIOPhyRegWrite(mdioBaseAddress, phyAddress, PHY_REGCR, PHY_DEVAD_EXTENDED_NO_POST_INCREMENT);
    MDIOPhyRegWrite(mdioBaseAddress, phyAddress, PHY_ADDAR, RegVal);
}

static bool PHY_ReadExtendedAddressSpaceRegister(
    uint32_t mdioBaseAddress,
    uint32_t phyAddress,
    uint32_t regNum,
    uint16_t *dataPtr) {
    /* AXIVION Routine Generic-MissingParameterAssert: mdioBaseAddress: parameter accepts whole range */
    FAS_ASSERT(phyAddress < 32u);
    FAS_ASSERT((regNum != PHY_REGCR) && (regNum != PHY_ADDAR) && (regNum <= PHY_FX_INT_STS));
    FAS_ASSERT(dataPtr != NULL_PTR);

    MDIOPhyRegWrite(mdioBaseAddress, phyAddress, PHY_REGCR, PHY_DEVAD_EXTENDED);
    MDIOPhyRegWrite(mdioBaseAddress, phyAddress, PHY_ADDAR, regNum);
    MDIOPhyRegWrite(mdioBaseAddress, phyAddress, PHY_REGCR, PHY_DEVAD_EXTENDED_NO_POST_INCREMENT);
    return (MDIOPhyRegRead(mdioBaseAddress, phyAddress, PHY_ADDAR, dataPtr));
}

static void PHY_SelectSpeed(uint32_t mdioBaseAddress, uint32_t phyAddress, PHY_SPEED_SEL_e speed) {
    /* AXIVION Routine Generic-MissingParameterAssert: mdioBaseAddress: parameter accepts whole range */
    FAS_ASSERT(phyAddress < 32u);
    /* AXIVION Routine Generic-MissingParameterAssert: speed: parameter accepts whole range */

    uint16_t regVal  = 0x0000u;
    uint16_t *regPtr = &regVal;

    MDIOPhyRegRead(mdioBaseAddress, phyAddress, (uint32_t)PHY_BMCR, regPtr);
    regVal &= ~(1 << 13);
    regVal |= (speed & 2) << 12; /* Set SPEED_SEL_LSB */
    regVal &= ~(1 << 6);
    regVal |= (speed & 1) << 6; /* Set SPEED_SEL_MSB */
    MDIOPhyRegWrite(mdioBaseAddress, phyAddress, (uint32_t)PHY_BMCR, regVal);
}

static void PHY_RestartSoftware(uint32_t mdioBaseAddress, uint32_t phyAddress) {
    /* AXIVION Routine Generic-MissingParameterAssert: mdioBaseAddress: parameter accepts whole range */
    FAS_ASSERT(phyAddress < 32u);

    uint16_t regVal  = 0u;
    uint16_t *regPtr = &regVal;

    MDIOPhyRegWrite(mdioBaseAddress, phyAddress, PHY_GEN_CTRL, PHY_SWRESTART);

    (void)MDIOPhyRegRead(mdioBaseAddress, phyAddress, PHY_GEN_CTRL, regPtr);
    /* : This bit is self-clearing and returns 1 until the reset process is complete. */
    while ((regVal & PHY_SWRESTART) != 0u) {
        (void)MDIOPhyRegRead(mdioBaseAddress, phyAddress, PHY_GEN_CTRL, regPtr);
    }
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e PHY_Initialize(uint32_t mdioBaseAddressess) {
    /* AXIVION Routine Generic-MissingParameterAssert: mdioBaseAddress: parameter accepts whole range */

    STD_RETURN_TYPE_e result = STD_OK;
    uint32_t phyID           = 0u;
    uint32_t phyIdReadCount  = 0xFu;
    uint32_t phyLinkRetries  = 0xFu;

    /* Reset all registers before start */
    PHY_ResetHardware();

    /* Get PHY ID and check MDIO connection */
    while ((phyID == 0u) && (phyIdReadCount > 0u)) {
        phyID = PHY_GetId(mdioBaseAddressess, PHY_ADDRESS);
        phyIdReadCount--;
        /* Don't block other tasks to much */
        OS_DelayTask(10u);
    }

    phyID = phyID >> 4; /* Don't compare the Revision Number */
    if (0u == phyID) {
        result = STD_NOT_OK;
    } else if (PHY_PHY_ID >> 4 != phyID) {
        result = STD_NOT_OK; /* Wrong PHY */
    }

    /* Check PHY alive status */
    if (result == STD_OK) {
        if (((MDIOPhyAliveStatusGet(mdioBaseAddressess) >> PHY_ADDRESS) & (uint32_t)0x01u) == (uint32_t)0u) {
            result = STD_NOT_OK;
        } else {
            phy_tablePhy.aliveStatus = true;
        }
    }

    /* Set PHY to MII-mode */
    if ((result == STD_OK) && (PHY_SetMiiMode(mdioBaseAddressess, PHY_ADDRESS) == false)) {
        result = STD_NOT_OK;
    }

    /* Check for network link */
    if ((result == STD_OK) &&
        (PHY_GetLinkStatus(mdioBaseAddressess, PHY_ADDRESS, (uint32_t)phyLinkRetries) != STD_OK)) {
        result = STD_NOT_OK;
    }

    DATA_WRITE_DATA(&phy_tablePhy);
    return result;
}

extern STD_RETURN_TYPE_e PHY_GetLinkStatus(uint32_t mdioBaseAddress, uint32_t phyAddress, volatile uint32_t retries) {
    /* AXIVION Routine Generic-MissingParameterAssert: mdioBaseAddress: parameter accepts whole range */
    FAS_ASSERT(phyAddress < 32u);
    /* AXIVION Routine Generic-MissingParameterAssert: retries: parameter accepts whole range */

    volatile uint16_t linkStatus = 0u;
    STD_RETURN_TYPE_e retVal     = STD_NOT_OK;
    bool readSuccess             = false;

    while (retries > 0u) {
        /* First try to read the BMSR of the PHY*/
        readSuccess = MDIOPhyRegRead(mdioBaseAddress, phyAddress, (uint32_t)PHY_BMSR, &linkStatus);
        if (readSuccess == true) {
            if ((linkStatus & PHY_LINK_STATUS) != 0u) {
                /* Link status okay */
                retVal                  = STD_OK;
                phy_tablePhy.linkStatus = true;
                break;
            } else {
                retries--;
                /* Don't block other tasks to much */
                OS_DelayTask(100u);
            }
        } else {
            retries--;
            /* Don't block other tasks to much */
            OS_DelayTask(100u);
        }
    }
    if (retVal == STD_NOT_OK) {
        phy_tablePhy.linkStatus = false;
    }
    DATA_WRITE_DATA(&phy_tablePhy);
    return retVal;
}

extern uint32_t PHY_GetId(uint32_t mdioBaseAddress, uint32_t phyAddress) {
    /* AXIVION Routine Generic-MissingParameterAssert: mdioBaseAddress: parameter accepts whole range */
    FAS_ASSERT(phyAddress < 32u);

    uint32_t id   = 0u;
    uint16_t data = 0u;

    /* read the ID1 register */
    (void)MDIOPhyRegRead(mdioBaseAddress, phyAddress, (uint32_t)PHY_PHYIDR1, &data);

    /* update the ID1 value */
    id = (uint32_t)data;
    id = (uint32_t)((uint32_t)id << PHY_ID_SHIFT);

    /* read the ID2 register */
    (void)MDIOPhyRegRead(mdioBaseAddress, phyAddress, (uint32_t)PHY_PHYIDR2, &data);

    /* update the ID2 value */
    id |= data;

    /* return the ID in ID1:ID2 format */
    return id;
}

extern STD_RETURN_TYPE_e PHY_AutoNegotiate(uint32_t mdioBaseAddress, uint32_t phyAddress, uint16_t advVal) {
    /* AXIVION Routine Generic-MissingParameterAssert: mdioBaseAddress: parameter accepts whole range */
    FAS_ASSERT(phyAddress < 32u);
    /* AXIVION Routine Generic-MissingParameterAssert: advVal: parameter accepts whole range */

    volatile uint16_t data   = 0u;
    volatile uint16_t anar   = 0u;
    STD_RETURN_TYPE_e retVal = STD_OK;
    uint32_t phyNegTries     = 100u;

    if (MDIOPhyRegRead(mdioBaseAddress, phyAddress, (uint32_t)PHY_BMCR, &data) != true) {
        retVal = STD_NOT_OK;
    }

    if (retVal == STD_OK) {
        /* Enable Auto Negotiation */
        data |= PHY_AUTONEG_ENABLE;
        MDIOPhyRegWrite(mdioBaseAddress, phyAddress, (uint32_t)PHY_BMCR, data);
        if (MDIOPhyRegRead(mdioBaseAddress, phyAddress, (uint32_t)PHY_BMCR, &data) != true) {
            retVal = STD_NOT_OK;
        }
    }

    if (retVal == STD_OK) {
        /* Write Auto Negotiation capabilities */
        (void)MDIOPhyRegRead(mdioBaseAddress, phyAddress, (uint32_t)PHY_ANAR, &anar);
        anar &= (uint16_t)(~0xFF10u);
        MDIOPhyRegWrite(mdioBaseAddress, phyAddress, (uint32_t)PHY_ANAR, (anar | advVal));
        /* Start Auto Negotiation */
        data |= PHY_AUTONEG_RESTART;
        MDIOPhyRegWrite(mdioBaseAddress, phyAddress, (uint32_t)PHY_BMCR, data);
        data = 0u;
        /* Get the auto negotiation status*/
        if (MDIOPhyRegRead(mdioBaseAddress, phyAddress, (uint32_t)PHY_BMSR, &data) != true) {
            retVal = STD_NOT_OK;
        }
    }

    if (retVal == STD_OK) {
        /* Wait till auto negotiation is complete */
        while ((((uint16_t)(PHY_AUTONEG_INCOMPLETE)) == (data & (uint16_t)(PHY_AUTONEG_STATUS))) &&
               (phyNegTries > 0u)) {
            (void)MDIOPhyRegRead(mdioBaseAddress, phyAddress, (uint32_t)PHY_BMSR, &data);
            phyNegTries--;
            OS_DelayTask(PHY_AUTO_NEG_DELAY_TIME);
        }

        /* Check if the PHY is able to perform auto negotiation */
        if ((data & PHY_AUTONEG_ABLE) == 0u) {
            retVal = STD_NOT_OK;
        }
    }

    return retVal;
}

extern bool PHY_GetPartnerAbility(uint32_t mdioBaseAddress, uint32_t phyAddress, uint16_t *pPartnerAbility) {
    /* AXIVION Routine Generic-MissingParameterAssert: mdioBaseAddress: parameter accepts whole range */
    FAS_ASSERT(phyAddress < 32u);
    FAS_ASSERT(pPartnerAbility != NULL_PTR);

    return (MDIOPhyRegRead(mdioBaseAddress, phyAddress, PHY_ALNPAR, pPartnerAbility));
}

extern bool PHY_SetMiiMode(uint32_t mdioBaseAddress, uint32_t phyAddress) {
    /* AXIVION Routine Generic-MissingParameterAssert: mdioBaseAddress: parameter accepts whole range */
    FAS_ASSERT(phyAddress < 32u);

    bool xReturn     = false;
    uint16_t regVal  = 0x0000u;
    uint16_t *regPtr = &regVal;

    /* Reset PHY*/
    PHY_ResetSoftware(mdioBaseAddress, phyAddress);

    PHY_ReadExtendedAddressSpaceRegister(mdioBaseAddress, phyAddress, (uint32_t)PHY_OP_MODE, regPtr);

    /* Setting MII mode. */
    regVal = PHY_MII_MODE_SET;
    PHY_WriteExtendedAddressSpaceRegister(mdioBaseAddress, phyAddress, (uint32_t)PHY_OP_MODE, regVal);

    PHY_RestartSoftware(mdioBaseAddress, phyAddress);
    regVal = 0x0000u;
    PHY_ReadExtendedAddressSpaceRegister(mdioBaseAddress, phyAddress, (uint32_t)PHY_OP_MODE, regPtr);

    if (PHY_MII_MODE_SET == regVal) {
        xReturn = true;
    }
    return xReturn;
}

extern void PHY_ResetMii(uint32_t mdioBaseAddress, uint32_t phyAddress) {
    /* AXIVION Routine Generic-MissingParameterAssert: mdioBaseAddress: parameter accepts whole range */
    FAS_ASSERT(phyAddress < 32u);

    uint16_t regVal  = 0u;
    uint16_t *regPtr = &regVal;
    MDIOPhyRegWrite(mdioBaseAddress, phyAddress, PHY_BMCR, PHY_MII_RESET);

    (void)MDIOPhyRegRead(mdioBaseAddress, phyAddress, PHY_BMCR, regPtr);
    /* : This bit is self-clearing and returns 1 until the reset process is complete. */
    while ((regVal & PHY_MII_RESET) != 0u) {
        (void)MDIOPhyRegRead(mdioBaseAddress, phyAddress, PHY_BMCR, regPtr);
    }
}

extern bool PHY_EnableLoopback(uint32_t mdioBaseAddress, uint32_t phyAddress) {
    /* AXIVION Routine Generic-MissingParameterAssert: mdioBaseAddress: parameter accepts whole range */
    FAS_ASSERT(phyAddress < 32u);

    uint16_t regVal  = 0x0000u;
    uint16_t *regPtr = &regVal;
    boolean retVal   = true;
    uint8_t speed;

    /* Disabling Auto Negotiate. */
    (void)MDIOPhyRegRead(mdioBaseAddress, phyAddress, (uint32_t)PHY_BMCR, regPtr);
    regVal &= (uint16_t)(~((uint16_t)PHY_AUTONEG_ENABLE));
    MDIOPhyRegWrite(mdioBaseAddress, phyAddress, (uint32_t)PHY_BMCR, regVal);

    /* Force 10 Mbps mode */
    PHY_SelectSpeed(mdioBaseAddress, phyAddress, mbps10);

    /* Enabling Loopback. */
    regVal |= PHY_LPBK_ENABLE;

    MDIOPhyRegWrite(mdioBaseAddress, phyAddress, (uint32_t)PHY_BMCR, regVal);

    /* In 100Base-TX mode after MII loopback is enabled through register 0x00, it is necessary
    to write 0x0004 to register 0x16 for proper operation of MII Loopback*/
    regVal = 0u;
    MDIOPhyRegRead(mdioBaseAddress, phyAddress, (uint32_t)PHY_STATUS, regPtr);
    speed = regVal >> 13u;
    if (speed == mbps100) {
        regVal = 0u;
        MDIOPhyRegRead(mdioBaseAddress, phyAddress, (uint32_t)PHY_BIST, regPtr);

        /* Enable Digital Loopback*/
        regVal |= 0x4u;
        MDIOPhyRegWrite(mdioBaseAddress, phyAddress, (uint32_t)PHY_BIST, regVal);

        regVal = 0u;
        if (MDIOPhyRegRead(mdioBaseAddress, phyAddress, (uint32_t)PHY_BIST, regPtr) != true) {
            retVal = FALSE;
        }
    }

    /* When configuring loopback modes, the Loopback Configuration Register (LOOPCR), address 0x00FE, should be
    set to 0xE720. */
    PHY_WriteExtendedAddressSpaceRegister(mdioBaseAddress, phyAddress, 0x00FE, 0xE720);

    /* Force 10 Mbps mode */
    PHY_SelectSpeed(mdioBaseAddress, phyAddress, mbps10);

    return retVal;
}

extern void PHY_ResetSoftware(uint32_t mdioBaseAddress, uint32_t phyAddress) {
    /* AXIVION Routine Generic-MissingParameterAssert: mdioBaseAddress: parameter accepts whole range */
    FAS_ASSERT(phyAddress < 32u);

    uint16_t regVal  = 0u;
    uint16_t *regPtr = &regVal;

    MDIOPhyRegWrite(mdioBaseAddress, phyAddress, PHY_GEN_CTRL, PHY_SW_RESET);

    (void)MDIOPhyRegRead(mdioBaseAddress, phyAddress, PHY_GEN_CTRL, regPtr);
    /* : This bit is self-clearing and returns 1 until the reset process is complete. */
    while ((regVal & PHY_SW_RESET) != 0u) {
        (void)MDIOPhyRegRead(mdioBaseAddress, phyAddress, PHY_GEN_CTRL, regPtr);
    }
}

extern void PHY_ResetHardware(void) {
    IO_PinReset(&PHY_IO_HW_RESET_REG_DOUT, PHY_HW_RESET_PIN);
    OS_DelayTask(PHY_HW_RESET_TIME);
    IO_PinSet(&PHY_IO_HW_RESET_REG_DOUT, PHY_HW_RESET_PIN);
    OS_DelayTask(PHY_HW_POST_RESET_STABILIZATION_TIME);
}

extern void PHY_GetOperationMode(uint32_t mdioBaseAddress, uint32_t phyAddress, PHY_OPERATION_MODE_s *pOperationMode) {
    /* AXIVION Routine Generic-MissingParameterAssert: mdioBaseAddress: parameter accepts whole range */
    FAS_ASSERT(phyAddress < 32u);
    FAS_ASSERT(pOperationMode != NULL_PTR);

    uint16_t regVal  = 0x0000u;
    uint16_t *regPtr = &regVal;

    PHY_ReadExtendedAddressSpaceRegister(mdioBaseAddress, phyAddress, PHY_STRAP_STS, regPtr);

    pOperationMode->rgmii2miiEnable       = regVal & 0x1;
    pOperationMode->autoNegotiationEnable = ~(regVal & 0x2);
    pOperationMode->phyAddressess         = (regVal >> 4) & 0x0F;
    pOperationMode->operationMode         = (regVal >> 9) & 0x07;
    pOperationMode->mirrorEnable          = (regVal >> 12);
    pOperationMode->linkLossPassEnable    = (uint8_t)~(regVal >> 13);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
void TEST_PHY_WriteExtendedAddressSpaceRegister(
    uint32_t mdioBaseAddress,
    uint32_t phyAddress,
    uint32_t regNum,
    uint16_t RegVal) {
    PHY_WriteExtendedAddressSpaceRegister(mdioBaseAddress, phyAddress, regNum, RegVal);
}
bool TEST_PHY_ReadExtendedAddressSpaceRegister(
    uint32_t mdioBaseAddress,
    uint32_t phyAddress,
    uint32_t regNum,
    uint16_t *pData) {
    return PHY_ReadExtendedAddressSpaceRegister(mdioBaseAddress, phyAddress, regNum, pData);
}
void TEST_PHY_SelectSpeed(uint32_t mdioBaseAddress, uint32_t phyAddress, PHY_SPEED_SEL_e speed) {
    PHY_SelectSpeed(mdioBaseAddress, phyAddress, speed);
}
void TEST_PHY_RestartSoftware(uint32_t mdioBaseAddress, uint32_t phyAddress) {
    PHY_RestartSoftware(mdioBaseAddress, phyAddress);
}
#endif
