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
 * @file    test_app-hl_notification.c
 * @author  foxBMS Team
 * @date    2025-08-05 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   TODO
 * @details TODO
 */

/*========== Includes =======================================================*/

#include "unity.h"

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
#include "HL_hal_stdtypes.h"
/* clang-format on */

#include "Mockinfinite-loop-helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("app-hl_notification.c")

/*========== Definitions and Implementations for Unit Test ==================*/

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
void testesmGroup1Notification(void) {
    esmGroup1Notification(NULL_PTR, 0u); /* arguments do not matter as unused */
}

void testesmGroup2Notification(void) {
    esmGroup2Notification(NULL_PTR, 0u); /* arguments do not matter as unused */
}

void testesmGroup3Notification(void) {
    FOREVER_ExpectAndReturn(0);
    esmGroup3Notification(NULL_PTR, 0u); /* arguments do not matter as unused */

    FOREVER_ExpectAndReturn(1);
    FOREVER_ExpectAndReturn(0);
    esmGroup3Notification(NULL_PTR, 0u); /* arguments do not matter as unused */
}

void testadcNotification(void) {
    adcNotification(NULL_PTR, 0u); /* arguments do not matter as unused */
}

void testcanErrorNotification(void) {
    canErrorNotification(NULL_PTR, 0u); /* arguments do not matter as unused */
}

void testcanStatusChangeNotification(void) {
    canStatusChangeNotification(NULL_PTR, 0u); /* arguments do not matter as unused */
}

void testdccNotification(void) {
    dccNotification(NULL_PTR, 0u); /* arguments do not matter as unused */
}

void testgioNotification(void) {
    gioNotification(NULL_PTR, 0u); /* arguments do not matter as unused */
}

void testi2cNotification(void) {
    i2cNotification(NULL_PTR, 0u); /* arguments do not matter as unused */
}

void testlinNotification(void) {
    linNotification(NULL_PTR, 0u); /* arguments do not matter as unused */
}

void testsciNotification(void) {
    sciNotification(NULL_PTR, 0u); /* arguments do not matter as unused */
}

void testspiEndNotification(void) {
    spiEndNotification(NULL_PTR); /* arguments do not matter as unused */
}

void testpwmNotification(void) {
    pwmNotification(NULL_PTR, 0u, 0u); /* arguments do not matter as unused */
}

void testedgeNotification(void) {
    edgeNotification(NULL_PTR, 0u); /* arguments do not matter as unused */
}

void testhetNotification(void) {
    hetNotification(NULL_PTR, 0u); /* arguments do not matter as unused */
}

void testcrcNotification(void) {
    crcNotification(NULL_PTR, 0u); /* arguments do not matter as unused */
}

void testetpwmNotification(void) {
    etpwmNotification(NULL_PTR); /* arguments do not matter as unused */
}

void testetpwmTripNotification(void) {
    etpwmTripNotification(NULL_PTR, 0u); /* arguments do not matter as unused */
}

void testeqepNotification(void) {
    eqepNotification(NULL_PTR, 0u); /* arguments do not matter as unused */
}

void testepcCAMFullNotification(void) {
    epcCAMFullNotification(); /* arguments do not matter as unused */
}

void testepcFIFOFullNotification(void) {
    epcFIFOFullNotification(0u); /* arguments do not matter as unused */
}

void testemacTxNotification(void) {
    emacTxNotification(NULL_PTR); /* arguments do not matter as unused */
}

void testemacRxNotification(void) {
    emacRxNotification(NULL_PTR); /* arguments do not matter as unused */
}
