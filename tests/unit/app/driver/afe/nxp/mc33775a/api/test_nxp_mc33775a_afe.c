/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    test_nxp_mc33775a_afe.c
 * @author  foxBMS Team
 * @date    2020-06-10 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of some module
 *
 */

/*========== Includes =======================================================*/

#include "unity.h"
#include "Mockafe_dma.h"
#include "Mockdma.h"
#include "Mocknxp_mc33775a.h"
#include "Mockos.h"
#include "Mockpex.h"

#include <stdbool.h>
#include <stdint.h>

TEST_FILE("nxp_mc33775a_afe.c")

/*========== Definitions and Implementations for Unit Test ==================*/

static DATA_BLOCK_CELL_VOLTAGE_s n775_cellVoltage           = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_TEMPERATURE_s n775_cellTemperature   = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};
static DATA_BLOCK_MIN_MAX_s n775_minMax                     = {.header.uniqueId = DATA_BLOCK_ID_MIN_MAX};
static DATA_BLOCK_BALANCING_CONTROL_s n775_balancingControl = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};
static DATA_BLOCK_ALL_GPIO_VOLTAGES_s n775_allGpioVoltage   = {.header.uniqueId = DATA_BLOCK_ID_ALL_GPIO_VOLTAGES_BASE};
static DATA_BLOCK_BALANCING_FEEDBACK_s n775_balancingFeedback = {
    .header.uniqueId = DATA_BLOCK_ID_BALANCING_FEEDBACK_BASE};
static DATA_BLOCK_SLAVE_CONTROL_s n775_slaveControl = {.header.uniqueId = DATA_BLOCK_ID_SLAVE_CONTROL};
static DATA_BLOCK_OPEN_WIRE_s n775_openWire         = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};
static N775_SUPPLY_CURRENT_s n775_supplyCurrent     = {0};
static N775_ERRORTABLE_s n775_errorTable            = {0};

N775_STATE_s n775_stateBase = {
    .firstMeasurementMade       = false,
    .currentString              = 0u,
    .pSpiTxSequenceStart        = NULL_PTR,
    .pSpiTxSequence             = NULL_PTR,
    .pSpiRxSequenceStart        = NULL_PTR,
    .pSpiRxSequence             = NULL_PTR,
    .currentMux                 = {0u},
    .pMuxSequenceStart          = NULL_PTR,
    .pMuxSequence               = NULL_PTR,
    .n775Data.cellVoltage       = &n775_cellVoltage,
    .n775Data.cellTemperature   = &n775_cellTemperature,
    .n775Data.allGpioVoltage    = &n775_allGpioVoltage,
    .n775Data.minMax            = &n775_minMax,
    .n775Data.balancingFeedback = &n775_balancingFeedback,
    .n775Data.balancingControl  = &n775_balancingControl,
    .n775Data.slaveControl      = &n775_slaveControl,
    .n775Data.openWire          = &n775_openWire,
    .n775Data.supplyCurrent     = &n775_supplyCurrent,
    .n775Data.errorTable        = &n775_errorTable,
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
