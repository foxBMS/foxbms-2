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
 * @file    bms-slave_cfg.h
 * @author  foxBMS Team
 * @date    2025-01-08 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup BATTERY_SYSTEM_CONFIGURATION
 * @prefix  SLV
 *
 * @brief   Configuration of the foxBMS BMS-Slave (e.g., value of balancing
 *          resistor, input capacitance...)
 * @details This files contains basic macros of the foxBMS BMS-Slave in order to
 *          derive needed inputs in other parts of the software.
 *          These macros are all depended on the hardware.
 */

#ifndef FOXBMS__BMS_SLAVE_CFG_H_
#define FOXBMS__BMS_SLAVE_CFG_H_

/*========== Includes =======================================================*/
#include "general.h"

/*========== Macros and Definitions =========================================*/

/**
 * @def     SLV_NR_OF_GPIOS_PER_MODULE
 * @brief   Defines the number of GPIOs
 */
#define SLV_NR_OF_GPIOS_PER_MODULE (10u)

/**
 * @def     SLV_NR_OF_GPAS_PER_MODULE
 * @brief   Defines the number of GPA inputs
 */
#define SLV_NR_OF_GPAS_PER_MODULE (2u)

/** Value of the balancing resistors on the slave-board */
#define SLV_BALANCING_RESISTANCE_ohm (100.0f)

/** Value of the input capacitors between the cell inputs on the BMS-Slave
 *  board. Set to 0 if no capacitors are used at all */
#define SLV_CELL_INPUT_CAPACITOR_CAPACITANCE_nF (0u)

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__BMS_SLAVE_CFG_H_ */
