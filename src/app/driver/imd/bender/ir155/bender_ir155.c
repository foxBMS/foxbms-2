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
 * @file    bender_ir155.c
 * @author  foxBMS Team
 * @date    2014-02-11 (date of creation)
 * @updated 2021-09-08 (date of last update)
 * @ingroup DRIVERS
 * @prefix  IR155
 *
 * @brief   Driver for the insulation monitoring
 */

/*========== Includes =======================================================*/
#include "bender_ir155.h"

#include "database.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/** internal handle for the database table of the insulation monitoring driver */
static DATA_BLOCK_INSULATION_MONITORING_s ir155_insulationMeasurement = {
    .header.uniqueId = DATA_BLOCK_ID_INSULATION_MONITORING};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
static STD_RETURN_TYPE_e IR155_MeasureInsulation(void);

/*========== Static Function Implementations ================================*/
static STD_RETURN_TYPE_e IR155_MeasureInsulation(void) {
    ir155_insulationMeasurement.valid                     = 0;
    ir155_insulationMeasurement.state                     = 0;
    ir155_insulationMeasurement.insulationResistance_kOhm = 10000000;
    ir155_insulationMeasurement.insulationFault           = 0;
    ir155_insulationMeasurement.chassisFault              = 0;
    ir155_insulationMeasurement.systemFailure             = 0;
    ir155_insulationMeasurement.insulationWarning         = 0;
    DATA_WRITE_DATA(&ir155_insulationMeasurement);
    return STD_OK;
}

/*========== Extern Function Implementations ================================*/
extern void IMD_Trigger(void) {
    IR155_MeasureInsulation();
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
