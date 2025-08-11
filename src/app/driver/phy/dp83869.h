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
 * @file    dp83869.h
 * @date    2025-04-01 (date of creation)
 * @updated 2025-04-01 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  PHY
 *
 * @brief   Implementation of physical layer driver
 * @details Implements the driver for the DP83869HM PHY
 */

#ifndef FOXBMS__DP83869_H_
#define FOXBMS__DP83869_H_

/*
 * The origin of this header is the file 'HL_phy_dp83640.h' created by HALCoGen
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

#include "fstd_types.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/* PHY ID. The LSB nibble will vary between different PHY revisions */
#define PHY_PHY_ID (0x2000A0F1)

/* 100 Base TX Full Duplex capability */
#define PHY_100BTX_HD (0x0000u)
#define PHY_100BTX_FD (0x0100u)

/* 100 Base TX capability */
#define PHY_NO_100BTX (0x0000u)
#define PHY_100BTX    (0x0080u)

/* 10 BaseT duplex capabilities */
#define PHY_10BT_HD (0x0000u)
#define PHY_10BT_FD (0x0040u)

/* 10 BaseT ability*/
#define PHY_NO_10BT (0x0000u)
#define PHY_10BT    (0x0020u)

/* TODO: This define belongs to the EMAC and will moved as the corresponding driver is added */
#define PHY_MDIO_BASE 0xFCF78900u

/* Structure to hold the values of the PHY Opmode Register. */
typedef struct {

    bool rgmii2miiEnable;       /*!< RGMII to MII Enable Strap: 0x0 = RGMII mode, 0x1 = MII Mode */
    bool autoNegotiationEnable; /*!< Auto Negotiation Enable Strap: 0x0 = Enable, 0x1 = Disable*/
    uint8_t phyAddressess;      /*!< PHY Address Strap */
    uint8_t operationMode;      /*!< OPMODE Strap 0x0 = RGMII To Copper
                                                  0x1 = RGMII to 1000Base-X
                                                  0x2 = RGMII to 100Base-FX
                                                  0x3 = RGMII-SGMII Bridge
                                                  0x4 = 1000Base-T to 1000Base-X
                                                  0x5 = 100Base-T to 100Base-FX
                                                  0x6 = SGMII to Copper
                                                  0x7 = JTAG for Boundary Scan */
    bool mirrorEnable;          /*!< Mirror Mode Enable StraP. Refer to strap configuration section as this strap
                                     also decides MAC interface in Bridge Mode applications: 0x0 = Disable, 0x1 = Enable*/
    bool linkLossPassEnable;    /*!< Link Loss Pass Through Enable Strap: 0x0 = Enable, 0x1 = Disable */

} PHY_OPERATION_MODE_s;

typedef enum {
    mbps10   = 0x0,
    mbps100  = 0x1,
    mbps1000 = 0x2
} PHY_SPEED_SEL_e;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
extern STD_RETURN_TYPE_e PHY_Initialize(uint32_t mdioBaseAddressess);

/**
 * @brief   Read the PHY ID.
 * @param   mdioBaseAddress Base Address of the MDIO Module Registers.
 * @param   phyAddress      PHY Address.
 * @return  32 bit PHY ID
 **/
extern uint32_t PHY_GetId(uint32_t mdioBaseAddress, uint32_t phyAddress);

/**
 * @brief   Perform a software reset of the PHY.
 * @details This will reset the PHY and return registers to their default
 *          values. Registers controlled via strap pins will return back to
 *          their last strapped values.
 * @param   mdioBaseAddress Base Address of the MDIO Module Registers.
 * @param   phyAddress   PHY Address.
 *

 **/
extern void PHY_ResetSoftware(uint32_t mdioBaseAddress, uint32_t phyAddress);

/**
 * @brief   Perform a hardware reset of the PHY.
 * @details This will reset the PHY and return registers to their default
 *          values. Registers controlled via strap pins will return back to
 *          their last strapped values.
 **/
extern void PHY_ResetHardware(void);

/**
 * @brief   Autonegotiate with the EMAC device connected to the PHY.
 * @param   mdioBaseAddress Base Address of the MDIO Module Registers.
 * @param   phyAddress      PHY Address.
 * @param   advVal          Autonegotiation advertisement value
 *          advVal can take any of the following OR any combination of the
 *          following values:
 *          - DP83869_100BTX - 100BaseTX
 *          - DP83869_100BTX_FD - Full duplex capability for 100BaseTX
 *          - DP83869_10BT - 10BaseT
 *          - DP83869_10BT_FD - Full duplex capability for 10BaseT
 * @return  status after autonegotiation
 **/
extern STD_RETURN_TYPE_e PHY_AutoNegotiate(uint32_t mdioBaseAddress, uint32_t phyAddress, uint16_t advVal);

/**
 * @brief   Get the advertised abilities of the Link Partner as received during
 *          autonegotiation.
 * @details The content of the register changes after the successful
 *          autonegotiation if Next pages are supported.
 * @param   mdioBaseAddress Base Address of the MDIO Module Registers.
 * @param   phyAddress      PHY Address.
 * @param   pPartnerAbility  Advertised abilities of the Link Partner
 * @return  true if reading was successful, false otherwise
 */

extern bool PHY_GetPartnerAbility(uint32_t mdioBaseAddress, uint32_t phyAddress, uint16_t *pPartnerAbility);

/**
 * @brief   Reads the link status of the PHY.
 * @param   mdioBaseAddress Base Address of the MDIO Module Registers.
 * @param   phyAddress      PHY Address.
 * @param   retries         The number of retries before indicating down status
 * @return  link status after reading
 **/
extern STD_RETURN_TYPE_e PHY_GetLinkStatus(uint32_t mdioBaseAddress, uint32_t phyAddress, volatile uint32_t retries);

/**
 * @brief   Sets the PHY in MII mode.
 * @details The PHY is by default in RGMII mode.
 *          This Function Sets the OP_MODE_DECODE register to MII mode.
 * @param   mdioBaseAddress Address of the MDIO Module Registers.
 * @param   phyAddress      PHY Address.
 * @return  true if setting MII mode was successful, otherwise false
 */
extern bool PHY_SetMiiMode(uint32_t mdioBaseAddress, uint32_t phyAddress);

/**
 * @brief   Resets the MII Bus.
 * @param   mdioBaseAddress Base Address of the MDIO Module Registers.
 * @param   phyAddress      PHY Address.
 **/
extern void PHY_ResetMii(uint32_t mdioBaseAddress, uint32_t phyAddress);

/**
 * @brief   Enables PHY Loopback.
 * @param   mdioBaseAddress Base Address of the MDIO Module Registers.
 * @param   phyAddress      PHY Address.
 * @return  true if enable was successful, otherwise false
 **/
extern bool PHY_EnableLoopback(uint32_t mdioBaseAddress, uint32_t phyAddress);

/**
 * @brief   Returns the operation mode indicated in the Strap Status Register
 * @param   pOperationMode  Operation mode
 * @param   mdioBaseAddress Base Address of the MDIO Module Registers.
 * @param   phyAddress      PHY Address.
 */
extern void PHY_GetOperationMode(uint32_t mdioBaseAddress, uint32_t phyAddress, PHY_OPERATION_MODE_s *pOperationMode);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
void TEST_PHY_WriteExtendedAddressSpaceRegister(
    uint32_t mdioBaseAddress,
    uint32_t phyAddress,
    uint32_t regNum,
    uint16_t RegVal);
bool TEST_PHY_ReadExtendedAddressSpaceRegister(
    uint32_t mdioBaseAddress,
    uint32_t phyAddress,
    uint32_t regNum,
    uint16_t *pData);
void TEST_PHY_SelectSpeed(uint32_t mdioBaseAddress, uint32_t phyAddress, PHY_SPEED_SEL_e speed);
void TEST_PHY_RestartSoftware(uint32_t mdioBaseAddress, uint32_t phyAddress);
#endif

#endif /* FOXBMS__DP83869_H_ */
