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
 * @file    dma.h
 * @author  foxBMS Team
 * @date    2019-12-12 (date of creation)
 * @updated 2019-12-12 (date of last update)
 * @ingroup DRIVERS
 * @prefix  DMA
 *
 * @brief   Headers for the driver for the DMA module.
 *
 */

#ifndef FOXBMS__DMA_H_
#define FOXBMS__DMA_H_

/*========== Includes =======================================================*/
#include "dma_cfg.h"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/
/** tx control packet for DMA */
extern g_dmaCTRL g_dmaCTRLPKT_TX;
/** rx control packet for DMA */
extern g_dmaCTRL g_dmaCTRLPKT_RX;

/** tx control packet for N775 DMA */
extern g_dmaCTRL dma_n775DmaControlPacketTx;
/** rx control packet for N775 DMA */
extern g_dmaCTRL dma_n775DmaControlPacketRx;

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   Enables the DMA module.
 * @details This function enables the DMA streams.
 */
extern void DMA_Initialize(void);

/**
 * @brief   Re-enables the DMA module.
 * @details This function re-enables the DMA streams. Called in DMA complete
 *          interrupt.
 */
extern void DMA_ReInit(void);

/**
 * @brief   Sends and receives data over SPI with DMA.
 */
extern STD_RETURN_TYPE_e DMA_TransmitReceive_SPIwithDMA(void);

/**
 * @brief   gets the SPI transmit status.
 * @return  retval  true if transmission still ongoing, false otherwise
 */
extern bool DMA_IsTransmitOngoing(void);

/**
 * @brief   sets the SPI transmit status.
 */
extern void DMA_SetTransmitOngoing(void);

/**
 * @brief   gets the SPI transmit status.
 * @return  retval  true if transmission still ongoing, false otherwise
 */
extern bool DMA_IsTxTransmitOngoingN775(void);

/**
 * @brief   gets the SPI transmit status.
 * @return  retval  true if transmission still ongoing, false otherwise
 */
extern bool DMA_IsRxTransmitOngoingN775(void);

/**
 * @brief   sets the SPI transmit status.
 */
extern void DMA_SetTxTransmitOngoingN775(void);

/**
 * @brief   sets the SPI transmit status.
 */
extern void DMA_SetRxTransmitOngoingN775(void);

/**
 * @brief   enables the DMA module.
 * This function enables the DMA streams.
 */
void DMA_InitN775(void);

/**
 * @brief   Re-enables the DMA module.
 * This function re-enables the DMA streams.
 * Called in DMA complete interrupt.
 */
void DMA_ReInitN775Tx(void);

/**
 * @brief   Re-enables the DMA module.
 * This function re-enables the DMA streams.
 * Called in DMA complete interrupt.
 */
void DMA_ReInitN775Rx(void);

/**
 * @brief   Sends data over SPI with DMA.
 */
STD_RETURN_TYPE_e DMA_TransmitSPiWithDmaN775(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__DMA_H_ */
