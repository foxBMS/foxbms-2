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
 * @file    lib-build_main.c
 * @author  foxBMS Team
 * @date    2020-10-06 (date of creation)
 * @updated 2021-07-23 (date of last update)
 * @ingroup GENERAL
 * @prefix  TODO
 *
 * @brief   Main function
 *
 * @details TODO
 */

/*========== Includes =======================================================*/
/* clang-format off */
#include "main.h"
/* clang-format on */

#include "HL_can.h"
#include "HL_gio.h"
#include "HL_spi.h"
#include "HL_sys_core.h"

#include "adc.h"
#include "can.h"
#include "checksum.h"
#include "contactor.h"
#include "diag.h"
#include "dma.h"
#include "libproject-example.h"
#include "masterinfo.h"
#include "meas.h"
#include "os.h"
#include "sps.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
int main(void) {
    SUPER_Function(0, 1);
    MINFO_SetResetSource(getResetSource()); /* Get reset source and clear respective flags */
    _enable_IRQ_interrupt_();
    gioInit();
    canInit();
    spiInit();
    CONT_Initialize();
    SPS_Initialize();
    MEAS_Initialize();
    DMA_Initialize();
    DIAG_Initialize(&diag_device);
    CAN_Initialize();

    OS_InitializeOperatingSystem();
    if (OS_INIT_PRE_OS != os_boot) {
        while (1) {
            /* Could not create Queues, Mutexes, Events and Tasks
               do not boot further from this point on*/
        }
    }

    if (STD_OK != CHK_ValidateChecksum()) {
        if (DIAG_HANDLER_RETURN_OK != DIAG_Handler(DIAG_ID_FLASHCHECKSUM, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u)) {
            while (1) {
                /* Could not validate checksum do not boot further from this point on */
            }
        }
    }

    os_schedulerStartTime = OS_GetTickCount();

    OS_StartScheduler();
    while (1) {
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
