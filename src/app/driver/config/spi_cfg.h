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
 * @file    spi_cfg.h
 * @author  foxBMS Team
 * @date    2020-03-05 (date of creation)
 * @updated 2020-09-30 (date of last update)
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  SPI
 *
 * @brief   Headers for the configuration for the SPI module
 *
 *
 */

#ifndef FOXBMS__SPI_CFG_H_
#define FOXBMS__SPI_CFG_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "battery_system_cfg.h"

#include "HL_het.h"
#include "HL_spi.h"

/*========== Macros and Definitions =========================================*/

/* TODO: check definition of SPI nodes depending on target hardware */
/** SPI defines for LTC */
#define LTC_SPI_NODE (spiREG1)

/** GIO defines for SPI for Smart Power Switches @{ */
#define SPS_SPI_CS_GIOPORT     (hetREG2->DOUT)
#define SPS_SPI_CS_GIOPORT_DIR (hetREG2->DIR)
#define SPS_SPI_CS_PIN         (1u)
/**@}*/

/**
 * SPI defines for NXP MC33775A
 * @{
 */
#define N775_SPI_TX_NODE    (spiREG1)
#define N775_SPI_TX_GIOPORT (N775_SPI_TX_NODE->PC3)
#define N775_SPI_TX_CS_PIN  (2U)

#define N775_SPI_RX_NODE    (spiREG5)
#define N775_SPI_RX_GIOPORT (N775_SPI_TX_NODE->PC3)
#define N775_SPI_RX_CS_PIN  (2U)
/**@}*/

/** enum for spi interface state */
typedef enum SPI_BUSY_STATE {
    SPI_IDLE,
    SPI_BUSY,
} SPI_BUSY_STATE_e;

/** spi block identification numbers */
typedef enum SPI_INTERFACE {
    SPI_Interface1,
    SPI_Interface2,
    SPI_Interface3,
    SPI_Interface4,
    SPI_Interface5,
} SPI_INTERFACE_e;

/** configuration of the SPI interface */
typedef struct SPI_INTERFACE_CONFIG {
    SPI_INTERFACE_e channel;
    const spiDAT1_t *pConfig;
    spiBASE_t *pNode;
    volatile uint32_t *pGioPort;
    uint32_t csPin;
} SPI_INTERFACE_CONFIG_s;

/*========== Extern Constant and Variable Declarations ======================*/
extern SPI_INTERFACE_CONFIG_s spi_ltcInterface[BS_NR_OF_STRINGS];
extern SPI_INTERFACE_CONFIG_s spi_MxmInterface;
extern SPI_INTERFACE_CONFIG_s spi_nxp775Interface;
extern SPI_INTERFACE_CONFIG_s spi_framInterface;
extern SPI_INTERFACE_CONFIG_s spi_spsInterface;
extern SPI_INTERFACE_CONFIG_s spi_adc0Interface;
extern SPI_INTERFACE_CONFIG_s spi_adc1Interface;
extern SPI_INTERFACE_CONFIG_s spi_kSbcMcuInterface;
extern SPI_INTERFACE_CONFIG_s spi_dmaTransmission[];

extern uint32_t spi_saveFmt0[];

extern SPI_BUSY_STATE_e spi_busyFlags[];

extern const uint8_t spi_nrBusyFlags;

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief Switch the SPS spi handle to high speed
 * @param[out]  pSpiSpsInterface    handle of the SPS spi config
 */
extern void SPI_SpsInterfaceSwitchToHighSpeed(SPI_INTERFACE_CONFIG_s *pSpiSpsInterface);

/**
 * @brief Switch the SPS spi handle to low speed
 * @param[out]  pSpiSpsInterface    handle of the SPS spi config
 */
extern void SPI_SpsInterfaceSwitchToLowSpeed(SPI_INTERFACE_CONFIG_s *pSpiSpsInterface);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__SPI_CFG_H_ */
