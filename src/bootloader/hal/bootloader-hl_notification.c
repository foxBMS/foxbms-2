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
 * @file    bootloader-hl_notification.c
 * @date    11-Dec-2018
 * @updated 2025-08-05 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  IGNOR
 *
 * @brief   User Notification Definition File
 * @details This file is created by TI HALCoGen and adapted to foxBMS specific
 *          needs.
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
#include "HL_can.h"
#include "HL_gio.h"
#include "HL_rti.h"
#include "HL_epc.h"
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

void dmaGroupANotification(dmaInterrupt_t inttype, uint32 channel) {
}

void rtiNotification(rtiBASE_t *rtiREG, uint32 notification) {
}

void canErrorNotification(canBASE_t *node, uint32 notification) {
}

void canStatusChangeNotification(canBASE_t *node, uint32 notification) {
}

void gioNotification(gioPORT_t *port, uint32 bit) {
}

void epcCAMFullNotification(void) {
}

void epcFIFOFullNotification(uint32 epcFIFOStatus) {
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
