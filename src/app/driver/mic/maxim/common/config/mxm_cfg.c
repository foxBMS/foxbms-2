/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der
 *  angewandten Forschung e.V. All rights reserved.
 *
 * BSD 3-Clause License
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1.  Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer
 * to foxBMS in your hardware, software, documentation or advertising
 * materials:
 *
 * &Prime;This product uses parts of foxBMS&reg;&Prime;
 *
 * &Prime;This product includes parts of foxBMS&reg;&Prime;
 *
 * &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    mxm_cfg.c
 * @author  foxBMS Team
 * @date    2019-01-09 (date of creation)
 * @updated 2020-06-22 (date of last update)
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  MXM
 *
 * @brief   Configuration for the MAX monitoring chip
 *
 */

/*========== Includes =======================================================*/
#include "mxm_cfg.h"

#include "io.h"
#include "spi.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern void MXM_MonitoringPinInit(void) {
    IO_PinSet((uint32_t *)&MXM_17841B_GIODIR, MXM_17841B_SHTNDL_PIN);
    IO_PinSet((uint32_t *)&MXM_17841B_GIOPORT, MXM_17841B_SHTNDL_PIN);
}

extern STD_RETURN_TYPE_e MXM_GetSPIStateReady(void) {
    /* the current implementation is blocking, therefore the interface is always accesible */
    return STD_OK;
}

extern STD_RETURN_TYPE_e MXM_SendData(uint16_t *txBuffer, uint16_t length) {
    return SPI_TransmitData(&spi_MxmInterface, txBuffer, length);
}

extern STD_RETURN_TYPE_e MXM_ReceiveData(uint16_t *txBuffer, uint16_t *rxBuffer, uint16_t length) {
    return SPI_TransmitReceiveData(&spi_MxmInterface, txBuffer, rxBuffer, length);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
