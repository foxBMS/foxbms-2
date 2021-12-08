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
 * @file    test_mxm_17852.c
 * @author  foxBMS Team
 * @date    2021-11-30 (date of creation)
 * @updated 2021-12-06 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  MXM
 *
 * @brief   Test for the Maxim 17852 driver.
 *
 * @details def
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockafe_plausibility.h"
#include "Mockdatabase.h"
#include "Mockfassert.h"
#include "Mockmxm_17841b.h"
#include "Mockmxm_41b_register_map.h"
#include "Mockmxm_battery_management.h"
#include "Mockmxm_cfg.h"
#include "Mockmxm_crc8.h"
#include "Mockmxm_registry.h"
#include "Mockos.h"
#include "Mocktsi.h"

#include "database_cfg.h"

#include "mxm_1785x.h"
#include "mxm_1785x_tools.h"
#include "test_assert_helper.h"

TEST_FILE("mxm_17852.c")

/*========== Definitions and Implementations for Unit Test ==================*/

static DATA_BLOCK_OPEN_WIRE_s mxm_tableOpenWire = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};

static MXM_MONITORING_INSTANCE_s mxm_instance = {
    .state                 = MXM_STATEMACHINE_STATES_UNINITIALIZED,
    .operationSubstate     = MXM_INIT_ENTRY,
    .allowStartup          = false,
    .operationRequested    = false,
    .firstMeasurementDone  = false,
    .stopRequested         = false,
    .openwireRequested     = false,
    .undervoltageAlert     = false,
    .dcByte                = MXM_DC_EMPTY,
    .mxmVoltageCellCounter = 0,
    .highest5xDevice       = 0,
    .requestStatus5x       = MXM_5X_STATE_UNSENT,
    .batteryCmdBuffer =
        {
            .regAddress    = (MXM_REG_NAME_e)0x00,
            .lsb           = 0x00,
            .msb           = 0x00,
            .deviceAddress = 0x00,
            .blocksize     = 0,
            .model         = MXM_MODEL_ID_MAX17852,
        },
    .resultSelfCheck = STD_NOT_OK,
    .selfCheck =
        {
            .crc                      = STD_NOT_OK,
            .conv                     = STD_NOT_OK,
            .firstSetBit              = STD_NOT_OK,
            .extractValueFromRegister = STD_NOT_OK,
            .parseVoltageReadall      = STD_NOT_OK,
            .addressSpaceChecker      = STD_NOT_OK,
            .fmeaStatusASCI           = STD_NOT_OK,
        },
    .pOpenwire_table = &mxm_tableOpenWire,
};

const bool mxm_allowSkippingPostInitSelfCheck = true;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    mxm_instance.state             = MXM_STATEMACHINE_STATES_UNINITIALIZED;
    mxm_instance.operationSubstate = MXM_INIT_ENTRY;
    mxm_instance.requestStatus5x   = MXM_5X_STATE_UNSENT;

    /* delete rx buffer */
    for (uint16_t i = 0u; i < MXM_RX_BUFFER_LENGTH; i++) {
        mxm_instance.rxBuffer[i] = 0u;
    }
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/** checks the model ID of the implementation */
void testGetModelId(void) {
    TEST_ASSERT_EQUAL(MXM_MODEL_ID_MAX17852, MXM_GetModelIdOfDaisyChain());
}
