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
 * @file    database_helper.c
 * @author  foxBMS Team
 * @date    2021-05-05 (date of creation)
 * @updated 2021-06-09 (date of last update)
 * @ingroup ENGINE
 * @prefix  DATA
 *
 * @brief   Database helper implementation
 *
 * @details Implementation of database helper function
 */

/*========== Includes =======================================================*/
#include "database_helper.h"

#include "battery_system_cfg.h"

#include "os.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
extern bool DATA_DatabaseEntryUpdatedAtLeastOnce(DATA_BLOCK_HEADER_s dataBlockHeader) {
    bool retval = false;
    if (!((dataBlockHeader.timestamp == 0u) && (dataBlockHeader.previousTimestamp == 0u))) {
        /* Only possibility for timestamp AND previous timestamp to be 0 is, if
           the database entry has never been updated. Thus if this is not the
           case the database entry must have been updated */
        retval = true;
    }
    return retval;
}

extern bool DATA_EntryUpdatedWithinInterval(DATA_BLOCK_HEADER_s dataBlockHeader, uint32_t timeInterval) {
    bool retval               = false;
    uint32_t currentTimestamp = OS_GetTickCount();

    /* Unsigned integer arithmetic also works correctly if currentTimestap is
       larger than pHeader->timestamp (timer overflow), thus no need to use abs() */
    const uint32_t timeDifferenceLastCall = currentTimestamp - dataBlockHeader.timestamp;
    const bool updatedAtLeastOnce         = DATA_DatabaseEntryUpdatedAtLeastOnce(dataBlockHeader);
    if ((timeDifferenceLastCall <= timeInterval) && (updatedAtLeastOnce == true)) {
        /* Difference between current timestamp and last update timestamp is
           smaller than passed time interval */
        retval = true;
    }
    return retval;
}

extern bool DATA_EntryUpdatedPeriodicallyWithinInterval(DATA_BLOCK_HEADER_s dataBlockHeader, uint32_t timeInterval) {
    bool retval                     = false;
    const uint32_t currentTimestamp = OS_GetTickCount();

    /* Unsigned integer arithmetic also works correctly if currentTimestap is
       smaller than dataBlockHeader.timestamp (timer overflow), thus no need to use abs() */
    const uint32_t timeDifferenceLastCall     = currentTimestamp - dataBlockHeader.timestamp;
    const uint32_t timeDifferenceBetweenCalls = dataBlockHeader.timestamp - dataBlockHeader.previousTimestamp;

    if ((timeDifferenceLastCall <= timeInterval) && (timeDifferenceBetweenCalls <= timeInterval) &&
        (DATA_DatabaseEntryUpdatedAtLeastOnce(dataBlockHeader) == true)) {
        /* Difference between timestamps is smaller than passed time interval */
        retval = true;
    }
    return retval;
}

extern uint8_t DATA_GetStringNumberFromVoltageIndex(uint16_t cellIndex) {
    FAS_ASSERT(cellIndex < (BS_NR_OF_BAT_CELLS * BS_NR_OF_STRINGS));
    return (uint8_t)(cellIndex / BS_NR_OF_BAT_CELLS);
}

extern uint8_t DATA_GetModuleNumberFromVoltageIndex(uint16_t cellIndex) {
    FAS_ASSERT(cellIndex < (BS_NR_OF_BAT_CELLS * BS_NR_OF_STRINGS));
    uint8_t stringNumber = DATA_GetStringNumberFromVoltageIndex(cellIndex);
    uint8_t moduleNumber = (uint8_t)((cellIndex / BS_NR_OF_CELLS_PER_MODULE) - (stringNumber * BS_NR_OF_MODULES));
    return moduleNumber;
}

extern uint8_t DATA_GetCellNumberFromVoltageIndex(uint16_t cellIndex) {
    FAS_ASSERT(cellIndex < (BS_NR_OF_BAT_CELLS * BS_NR_OF_STRINGS));
    return (uint8_t)(cellIndex % BS_NR_OF_CELLS_PER_MODULE);
}

extern uint8_t DATA_GetStringNumberFromTemperatureIndex(uint16_t sensorIndex) {
    FAS_ASSERT(sensorIndex < BS_NR_OF_TEMP_SENSORS);
    return (uint8_t)(sensorIndex / BS_NR_OF_TEMP_SENSORS_PER_STRING);
}

extern uint8_t DATA_GetModuleNumberFromTemperatureIndex(uint16_t sensorIndex) {
    FAS_ASSERT(sensorIndex < BS_NR_OF_TEMP_SENSORS);
    uint8_t stringNumber = DATA_GetStringNumberFromTemperatureIndex(sensorIndex);
    uint8_t moduleNumber =
        (uint8_t)((sensorIndex / BS_NR_OF_TEMP_SENSORS_PER_MODULE) - (stringNumber * BS_NR_OF_MODULES));
    return moduleNumber;
}

extern uint8_t DATA_GetSensorNumberFromTemperatureIndex(uint16_t sensorIndex) {
    FAS_ASSERT(sensorIndex < BS_NR_OF_TEMP_SENSORS);
    return (uint8_t)(sensorIndex % BS_NR_OF_TEMP_SENSORS_PER_MODULE);
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
