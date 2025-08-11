/**
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
 * @file    app-hl_notification.c
 * @date    11-Dec-2018
 * @updated 2025-08-05 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  IGNOR
 *
 * @brief   User Notification Definition File
 * @details This file is created by TI HALCoGen 04.07.01 and adapted to foxBMS
 *          specific needs.
 *          This file defines empty notification routines for notifications
 *          that are unused in foxBMS.
 *          Changes are:
 *          - add 'section'-markers
 *          - add a prefix
 *          - add code justifications for SPA (Axivion)
 *          - make code unit testable ('UNITY_UNIT_TEST')
 *
 */

/*========== Includes =======================================================*/
/* clang-format off */
#include "HL_esm.h"
#include "HL_adc.h"
#include "HL_can.h"
#include "HL_gio.h"
#include "HL_lin.h"
#include "HL_sci.h"
#include "HL_spi.h"
#include "HL_het.h"
#include "HL_dcc.h"
#include "HL_i2c.h"
#include "HL_crc.h"
#include "HL_etpwm.h"
#include "HL_eqep.h"
#include "HL_ecap.h"
#include "HL_epc.h"
#include "HL_emac.h"
#include "HL_sys_dma.h"
/* clang-format on */

#include "HL_hal_stdtypes.h"

#include "infinite-loop-helper.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

void esmGroup1Notification(esmBASE_t *esm, uint32 channel) {
}

void esmGroup2Notification(esmBASE_t *esm, uint32 channel) {
}

void esmGroup3Notification(esmBASE_t *esm, uint32 channel) {
    while (FOREVER()) { /* Wait */
    }
}

void adcNotification(adcBASE_t *adc, uint32 group) {
}

void canErrorNotification(canBASE_t *node, uint32 notification) {
}

void canStatusChangeNotification(canBASE_t *node, uint32 notification) {
}

void dccNotification(dccBASE_t *dcc, uint32 flags) {
}

void gioNotification(gioPORT_t *port, uint32 bit) {
}

void i2cNotification(i2cBASE_t *i2c, uint32 flags) {
}

void linNotification(linBASE_t *lin, uint32 flags) {
}

void sciNotification(sciBASE_t *sci, uint32 flags) {
}

void spiEndNotification(spiBASE_t *spi) {
}

void pwmNotification(hetBASE_t *hetREG, uint32 pwm, uint32 notification) {
}

void edgeNotification(hetBASE_t *hetREG, uint32 edge) {
}

void hetNotification(hetBASE_t *het, uint32 offset) {
}

void crcNotification(crcBASE_t *crc, uint32 flags) {
}

void etpwmNotification(etpwmBASE_t *node) {
}

void etpwmTripNotification(etpwmBASE_t *node, uint16 flags) {
}

void eqepNotification(eqepBASE_t *eqep, uint16 flags) {
}

void epcCAMFullNotification(void) {
}

void epcFIFOFullNotification(uint32 epcFIFOStatus) {
}

void emacTxNotification(hdkif_t *hdkif) {
}

void emacRxNotification(hdkif_t *hdkif) {
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
