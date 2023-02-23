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
 * @file    spi_cfg.h
 * @author  foxBMS Team
 * @date    2020-03-05 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
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

#include "battery_system_cfg.h"

#include "HL_het.h"
#include "HL_spi.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** Index for the SPI nodes @{ */
#define SPI_SPI1_INDEX (0u)
#define SPI_SPI2_INDEX (1u)
#define SPI_SPI3_INDEX (2u)
#define SPI_SPI4_INDEX (3u)
#define SPI_SPI5_INDEX (4u)
/**@}*/

/* Number of SPI interfaces */
#define SPI_NR_SPI_INTERFACES (5u)

/** Bit in SPIDAT1 register that activates hardware Chip Select hold */
#define SPI_CSHOLD_BIT (0x10000000u)

/** Bit in SPIDAT1 register that activates delay between words */
#define SPI_WDEL_BIT (0x04000000u)

/** Position of CSNR field in SPIDAT1 register */
#define SPI_HARDWARE_CHIP_SELECT_FIELD_POSITION (16u)

/** Position of DFSEL field in SPIDAT1 register */
#define SPI_DATA_FORMAT_FIELD_POSITION (24u)

/** Position of TX buffer empty flag in SPIFLG register */
#define SPI_TX_BUFFER_EMPTY_FLAG_POSITION (9u)

/** Mask used to clear all HW CS bits */
#define SPI_PC0_CLEAR_HW_CS_MASK (0xFFFFFF00u)

/** Time to avoid infinite loop when waiting for Tx empty flag in a while loop */
#define SPI_TX_EMPTY_TIMEOUT_ITERATIONS (6000u)

/** Max number of hardware chip select pins */
#define SPI_MAX_NUMBER_HW_CS (6u)

/** SBC MCU chip select pin */
#define SPI_SBC_MCU_CHIP_SELECT_PIN (0u)

/** enum for spi interface state */
typedef enum {
    SPI_IDLE,
    SPI_BUSY,
} SPI_BUSY_STATE_e;

/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-cs-type-start-include */
/** type of chip select for spi */
typedef enum {
    SPI_CHIP_SELECT_HARDWARE,
    SPI_CHIP_SELECT_SOFTWARE,
    SPI_CHIP_SELECT_MAX,
} SPI_CHIP_SELECT_TYPE_e;
/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-cs-type-stop-include */

/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-configuration-start-include */
/** configuration of the SPI interface */
typedef struct {
    spiDAT1_t *pConfig;
    spiBASE_t *pNode;
    volatile uint32_t *pGioPort;
    uint32_t csPin;
    SPI_CHIP_SELECT_TYPE_e csType;
} SPI_INTERFACE_CONFIG_s;
/* INCLUDE MARKER FOR THE DOCUMENTATION; DO NOT MOVE spi-documentation-configuration-stop-include */

/* -------------- SPI Configurations --------------------------------------- */
/** LTC chip select pin */
#define SPI_LTC_CHIP_SELECT_PIN (1u)

/** Maxim chip select pin */
#define SPI_MAXIM_CHIP_SELECT_PIN (0u)

/** NXP chip select pin @{ */
#define SPI_NXP_TX_CHIP_SELECT_PIN (1u)
#define SPI_NXP_RX_CHIP_SELECT_PIN (0u)
/**@}*/

/** FRAM chip select pin */
#define SPI_FRAM_CHIP_SELECT_PIN (1u)

/** GIO defines for SPI for Smart Power Switches @{ */
#define SPI_SPS_CS_GIOPORT     (hetREG2->DOUT)
#define SPI_SPS_CS_GIOPORT_DIR (hetREG2->DIR)
#define SPI_SPS_CS_PIN         (1u) /* Connected to pin 1 of HET register */
/**@}*/

/*========== Extern Constant and Variable Declarations ======================*/
extern SPI_INTERFACE_CONFIG_s spi_ltcInterface[BS_NR_OF_STRINGS];
extern SPI_INTERFACE_CONFIG_s spi_mxmInterface;
extern SPI_INTERFACE_CONFIG_s spi_nxp775InterfaceTx[BS_NR_OF_STRINGS];
extern SPI_INTERFACE_CONFIG_s spi_nxp775InterfaceRx[BS_NR_OF_STRINGS];
extern SPI_INTERFACE_CONFIG_s spi_framInterface;
extern SPI_INTERFACE_CONFIG_s spi_spsInterface;
extern SPI_INTERFACE_CONFIG_s spi_sbcMcuInterface;

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
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__SPI_CFG_H_ */
