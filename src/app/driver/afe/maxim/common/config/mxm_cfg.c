/**
 *
 * @copyright &copy; 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * - "This product uses parts of foxBMS&reg;"
 * - "This product includes parts of foxBMS&reg;"
 * - "This product is derived from foxBMS&reg;"
 *
 */

/**
 * @file    mxm_cfg.c
 * @author  foxBMS Team
 * @date    2019-01-09 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  MXM
 *
 * @brief   Configuration for the MAX analog front-end
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "mxm_cfg.h"

#include "diag.h"
#include "io.h"
#include "spi.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/
const bool mxm_allowSkippingPostInitSelfCheck = true;

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern void MXM_InitializeMonitoringPins(void) {
    IO_SetPinDirectionToOutput(&MXM_17841B_GIOPORT->DIR, MXM_17841B_SHTNDL_PIN);
    IO_PinReset(&MXM_17841B_GIOPORT->DOUT, MXM_17841B_SHTNDL_PIN);

    /* configure functional of SIMO and SOMI pin */
    SPI_SetFunctional(spi_mxmInterface.pNode, SPI_PIN_SIMO, true);
    SPI_SetFunctional(spi_mxmInterface.pNode, SPI_PIN_SOMI, true);
}

extern STD_RETURN_TYPE_e MXM_GetSPIStateReady(void) {
    return SPI_CheckInterfaceAvailable(spi_mxmInterface.pNode);
}

extern STD_RETURN_TYPE_e MXM_SendData(uint16_t *txBuffer, uint16_t length) {
    FAS_ASSERT(txBuffer != NULL_PTR);
    FAS_ASSERT(length != 0u);
    const STD_RETURN_TYPE_e spiReturnValue = SPI_TransmitData(&spi_mxmInterface, txBuffer, length);
    /* this driver currently only handles one physical string, therefore reporting to string 0 */
    (void)DIAG_CheckEvent(spiReturnValue, DIAG_ID_AFE_SPI, DIAG_STRING, 0u);
    return spiReturnValue;
}

extern STD_RETURN_TYPE_e MXM_ReceiveData(uint16_t *txBuffer, uint16_t *rxBuffer, uint16_t length) {
    FAS_ASSERT(txBuffer != NULL_PTR);
    FAS_ASSERT(rxBuffer != NULL_PTR);
    FAS_ASSERT(length != 0u);
    const STD_RETURN_TYPE_e spiReturnValue = SPI_TransmitReceiveData(&spi_mxmInterface, txBuffer, rxBuffer, length);
    /* this driver currently only handles one physical string, therefore reporting to string 0 */
    (void)DIAG_CheckEvent(spiReturnValue, DIAG_ID_AFE_SPI, DIAG_STRING, 0u);
    return spiReturnValue;
}

extern void MXM_ShutDownBridgeIc(void) {
    IO_PinReset(&MXM_17841B_GIOPORT->DOUT, MXM_17841B_SHTNDL_PIN);
}

extern void MXM_EnableBridgeIc(void) {
    IO_PinSet(&MXM_17841B_GIOPORT->DOUT, MXM_17841B_SHTNDL_PIN);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
