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
 * @file    database_helper.h
 * @author  foxBMS Team
 * @date    2021-05-05 (date of creation)
 * @updated 2021-05-05 (date of last update)
 * @ingroup ENGINE
 * @prefix  DATA
 *
 * @brief   Database module header
 *
 * @details Provides helper function to handle database entries
 *
 */

#ifndef FOXBMS__DATABASE_HELPER_H_
#define FOXBMS__DATABASE_HELPER_H_

/*========== Includes =======================================================*/
#include "database_cfg.h"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Checks if passed database entry has been updated at least once.
 * @param[in]  dataBlockHeader header of database entry
 * @return true if database entry has been updated at least once, otherwise false
 */
extern bool DATA_DatabaseEntryUpdatedAtLeastOnce(DATA_BLOCK_HEADER_s dataBlockHeader);

/**
 * @brief   Checks if passed database entry has been updated within the last
 *          time interval
 * @param[in]  dataBlockHeader header of database entry
 * @param[in]  timeInterval in systicks (type: uint32_t)
 * @return true if database entry has been updated within the time interval,
 *         otherwise false
 */
extern bool DATA_EntryUpdatedWithinInterval(DATA_BLOCK_HEADER_s dataBlockHeader, uint32_t timeInterval);

/**
 * @brief   Checks if passed database entry has been periodically updated
 *          within the time interval
 * @details Checks if the last update timestamp is not older than time interval
 *          and if the difference between previous timestamp and timestamp is
 *          smaller than time interval
 * @param[in]  dataBlockHeader header of database entry
 * @param[in]  timeInterval in systicks (type: uint32_t)
 * @return true if database entry has been periodically updated within the time
 *         interval, otherwise false
 */
extern bool DATA_EntryUpdatedPeriodicallyWithinInterval(DATA_BLOCK_HEADER_s dataBlockHeader, uint32_t timeInterval);

/**
 * @brief   Returns string number of passed cell index
 * @param[in]  cellIndex   index of cell starting by 0
 * @return  string number of passed cell index
 */
extern uint8_t DATA_GetStringNumberFromVoltageIndex(uint16_t cellIndex);

/**
 * @brief   Returns module number of passed cell index
 * @param[in]  cellIndex   index of cell starting by 0
 * @return  module number of passed cell index
 */
extern uint8_t DATA_GetModuleNumberFromVoltageIndex(uint16_t cellIndex);

/**
 * @brief   Returns cell number of passed cell index
 * @param[in]  cellIndex   index of cell in starting by 0
 * @return  cell number of passed cell index
 */
extern uint8_t DATA_GetCellNumberFromVoltageIndex(uint16_t cellIndex);

/**
 * @brief   Returns string number of passed temperature sensor index
 * @param[in]  sensorIndex   index of sensor starting by 0
 * @return  string number of passed temperature sensor index
 */
extern uint8_t DATA_GetStringNumberFromTemperatureIndex(uint16_t sensorIndex);

/**
 * @brief   Returns module number of passed temperature sensor index
 * @param[in]  sensorIndex   index of sensor starting by 0
 * @return  module number of passed temperature sensor index
 */
extern uint8_t DATA_GetModuleNumberFromTemperatureIndex(uint16_t sensorIndex);

/**
 * @brief   Returns sensor number of passed temperature sensor index
 * @param[in]  sensorIndex   index of sensorstarting by 0
 * @return  temperature sensor number of passed sensor index
 */
extern uint8_t DATA_GetSensorNumberFromTemperatureIndex(uint16_t sensorIndex);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__DATABASE_HELPER_H_ */
