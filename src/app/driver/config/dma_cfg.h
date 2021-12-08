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
 * @file    dma_cfg.h
 * @author  foxBMS Team
 * @date    2020-03-05 (date of creation)
 * @updated 2021-09-28 (date of last update)
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  DMA
 *
 * @brief   Headers for the configuration for the DMA module
 *
 *
 */

#ifndef FOXBMS__DMA_CFG_H_
#define FOXBMS__DMA_CFG_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "battery_system_cfg.h"

#include "HL_spi.h"
#include "HL_sys_dma.h"

/*========== Macros and Definitions =========================================*/

/** defines for the DMA channels */
/**@{*/
#define DMA_CHANNEL_SPI1_TX (DMA_CH0)
#define DMA_CHANNEL_SPI1_RX (DMA_CH1)
#define DMA_CHANNEL_SPI2_TX (DMA_CH2)
#define DMA_CHANNEL_SPI2_RX (DMA_CH3)
#define DMA_CHANNEL_SPI3_TX (DMA_CH4)
#define DMA_CHANNEL_SPI3_RX (DMA_CH5)
#define DMA_CHANNEL_SPI4_TX (DMA_CH6)
#define DMA_CHANNEL_SPI4_RX (DMA_CH7)
#define DMA_CHANNEL_SPI5_TX (DMA_CH8)
#define DMA_CHANNEL_SPI5_RX (DMA_CH9)
#define DMA_CHANNEL_I2C_TX  (DMA_CH10)
#define DMA_CHANNEL_I2C_RX  (DMA_CH11)
/**@}*/

/** defines for the DMA request lines */
/**@{*/
#define DMA_REQ_LINE_SPI1_TX (DMA_REQ1)
#define DMA_REQ_LINE_SPI1_RX (DMA_REQ0)
#define DMA_REQ_LINE_SPI2_TX (DMA_REQ3)
#define DMA_REQ_LINE_SPI2_RX (DMA_REQ2)
#define DMA_REQ_LINE_SPI3_TX (DMA_REQ15)
#define DMA_REQ_LINE_SPI3_RX (DMA_REQ14)
#define DMA_REQ_LINE_SPI4_TX (DMA_REQ25)
#define DMA_REQ_LINE_SPI4_RX (DMA_REQ24)
#define DMA_REQ_LINE_SPI5_TX (DMA_REQ31)
#define DMA_REQ_LINE_SPI5_RX (DMA_REQ30)
#define DMA_REQ_LINE_I2C_TX  (DMA_REQ11)
#define DMA_REQ_LINE_I2C_RX  (DMA_REQ10)
/**@}*/

/** define for the shift of an address for big endian 8bit */
#define DMA_BIG_ENDIAN_ADDRESS_8BIT (3u)
/** define for the shift of an address for big endian 16bit */
#define DMA_BIG_ENDIAN_ADDRESS_16BIT (2u)

/** number of spi interfaces for DMA */
#define DMA_NUMBER_SPI_INTERFACES 5u
/** DMA Request enable bit */
#define DMAREQEN_BIT (0x10000U)
/** SPI enable bit */
#define SPIEN_BIT (0x1000000U)

/** DMA request line for LTC */
/**@{*/
#define DMA_REQ_LINE_LTC_TX (DMA_REQ_LINE_SPI1_TX)
#define DMA_REQ_LINE_LTC_RX (DMA_REQ_LINE_SPI1_RX)
/**@}*/

/** DMA request line for N775 */
/**@{**/
#define DMA_REQ_LINE_N775_TX (DMA_REQ_LINE_SPI1_TX)
#define DMA_REQ_LINE_N775_RX (DMA_REQ_LINE_SPI5_RX)
/**@}*/

/** describes the RX and TX DMA channels used for an SPI interface */
typedef struct DMA_CHANNEL_CONFIG {
    dmaChannel_t txChannel;
    dmaChannel_t rxChannel;
} DMA_CHANNEL_CONFIG_s;

/** describes the DMA requests corresponding to the DMA channel used for an SPI interface */
typedef struct DMA_REQUEST_CONFIG {
    dmaRequest_t txRequest;
    dmaRequest_t rxRequest;
} DMA_REQUEST_CONFIG_s;

/*========== Extern Constant and Variable Declarations ======================*/

extern DMA_CHANNEL_CONFIG_s dma_spiDmaChannels[DMA_NUMBER_SPI_INTERFACES];
extern DMA_REQUEST_CONFIG_s dma_spiDmaRequests[DMA_NUMBER_SPI_INTERFACES];
extern spiBASE_t *dma_spiInterfaces[DMA_NUMBER_SPI_INTERFACES];

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__DMA_CFG_H_ */
