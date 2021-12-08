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
 * @file    mxm_cfg.h
 * @author  foxBMS Team
 * @date    2019-01-09 (date of creation)
 * @updated 2021-12-06 (date of last update)
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  MXM
 *
 * @brief   Header for the configuration for the Maxim monitoring chip.
 *
 */

#ifndef FOXBMS__MXM_CFG_H_
#define FOXBMS__MXM_CFG_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "battery_system_cfg.h"
#include "spi_cfg.h"

#include "HL_reg_het.h"

#include "spi.h"

/*========== Macros and Definitions =========================================*/
/**
 * MAX17841B GPIO configuration
 * @{
 */
#define MXM_17841B_GIOPORT    (hetREG1)
#define MXM_17841B_SHTNDL_PIN (21U)
/**@}*/

/**
 * MAX17841B configuration registers default values
 * @{
 */
#define MXM_41B_RX_INT_ENABLE_DEFAULT_VALUE (0x00u)
#define MXM_41B_TX_INT_ENABLE_DEFAULT_VALUE (0x00u)
#define MXM_41B_RX_INT_FLAG_DEFAULT_VALUE   (0x00u)
#define MXM_41B_TX_INT_FLAG_DEFAULT_VALUE   (0x80u)

/** default configuration for config 1
 *
 * * baudrate 2Mbps
 * * no single ended mode
 * * no device count
 */
#define MXM_41B_CONFIG_1_DEFAULT_VALUE (0x60u)

/** default configuration for config 2
 *
 * * enable tx queue mode
 */
#define MXM_41B_CONFIG_2_DEFAULT_VALUE (0x10u)

/** default config for config 3
 *
 * * no keep-alive (infinite delay)
 */
#define MXM_41B_CONFIG_3_DEFAULT_VALUE (0x0Fu)
/**@}*/

/*========== Extern Constant and Variable Declarations ======================*/
/** if this flag is set, the post init self check will pass on even if the satellite numbers are not correct */
extern const bool mxm_allowSkippingPostInitSelfCheck;

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Initialize the pins connected to the MAX17841B
 * @details Initializes the pins that are needed by MAX17841B
 *          to a known state. The pins are:
 *          * the interrupt pin
 *          * the shutdown pin
 *          * the chip select pin
 *
 *          Only the necessary pins are configured.
 */
extern void MXM_InitializeMonitoringPins(void);

/**
 * @brief   Return whether SPI interface is ready
 * @details In the case of an DMA-based implementation this function
 *          can be used to check the state of the SPI interface and whether
 *          it can be accessed for writing.
 * @return  #STD_OK if the SPI interface is free
 */
extern STD_RETURN_TYPE_e MXM_GetSPIStateReady(void);

/**
 * @brief       Transmit data over SPI
 * @details     Wraps the #SPI_TransmitData API function in order to have
 *              one single point of access for the driver to the SPI interface.
 * @param[in]   txBuffer    pointer to an array that is to be transmitted
 * @param[in]   length      length of array
 * @return      #STD_NOT_OK for unsuccessful transmissions, #STD_OK on success
 */
extern STD_RETURN_TYPE_e MXM_SendData(uint16_t *txBuffer, uint16_t length);

/**
 * @brief       Send and Receive data over SPI
 * @details     Wraps the #SPI_TransmitReceiveData API function in order to
 *              have one single point of access for the driver to the SPI
 *              interface.
 * @param[in]   txBuffer    pointer to an array that is to be transmitted
 * @param[out]  rxBuffer    pointer to an array into which received data shall
 *                          be written
 * @param[in]   length      length of both arrays
 * @return      #STD_NOT_OK for unsuccessful transmissions, #STD_OK on success
 */
extern STD_RETURN_TYPE_e MXM_ReceiveData(uint16_t *txBuffer, uint16_t *rxBuffer, uint16_t length);

/**
 * @brief       Pulls the shutdown of the bridge IC low
 * @details     The bridge IC used in this driver has a shutdown pin that allows
 *              to switch it off. This function tells the bridge IC to shut down.
 */
extern void MXM_ShutDownBridgeIc(void);

/**
 * @brief       Pulls the shutdown of the bridge IC high
 * @details     The bridge IC used in this driver has a shutdown pin that allows
 *              to switch it off. This function tells the bridge IC to enable.
 *              (Pulls the shutdown high, bridge IC will start)
 */
extern void MXM_EnableBridgeIc(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__MXM_CFG_H_ */
