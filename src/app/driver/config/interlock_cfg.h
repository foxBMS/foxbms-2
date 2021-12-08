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
 * @file    interlock_cfg.h
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2021-12-08 (date of last update)
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  ILCK
 *
 * @brief   Header for the configuration for the driver for the interlock
 *
 */

#ifndef FOXBMS__INTERLOCK_CFG_H_
#define FOXBMS__INTERLOCK_CFG_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "HL_het.h"

/*========== Macros and Definitions =========================================*/

/** IO register to which the interlock is connected */
#define ILCK_IO_REG_DIR (hetREG1->DIR)
/** IO port for interlock related pins */
#define ILCK_IO_REG_PORT (hetREG1)

/** Enable pin for diagnostic power supply */
#define ILCK_INTERLOCK_CONTROL_PIN_IL_HS_ENABLE (30u)

/** Defines the pin where interlock feedback pin is connected to the mcu */
#define ILCK_INTERLOCK_FEEDBACK_PIN_IL_STATE (29u)

/* Defines to which ADC input the interlock circuitry is connected to */
/** High-side voltage sense signal connected to ADC input 2 (IL_HS_VS) */
#define ILCK_ADC_INPUT_HIGH_SIDE_VOLTAGE_SENSE (2u)
/** Low-side voltage sense signal connected to ADC input 3 (IL_LS_VS) */
#define ILCK_ADC_INPUT_LOW_SIDE_VOLTAGE_SENSE (3u)
/** High-side current sense signal connected to ADC input 4 (IL_HS_CS) */
#define ILCK_ADC_INPUT_HIGH_SIDE_CURRENT_SENSE (4u)
/** Low-side current sense signal connected to ADC input 5 (IL_LS_CS) */
#define ILCK_ADC_INPUT_LOW_SIDE_CURRENT_SENSE (5u)

/** Factor of resistor divider used for measuring the low- and high-side
 *  interlock voltages. Used resistor values are 5k6 ohm and 3k9 ohm. */
#define ILCK_VOLTAGE_DIVIDER_FACTOR ((5.6f + 3.9f) / 3.9f)

/**
 * Conversion factor for interlock current IL_HS_CS
 * 2k49 ohm is the resistance through which sense current flows
 * 80 is the (output current / sense current) ratio of high-side power switch
 * TPS2H000-Q1 data sheet SLVSD72D - DECEMBER 2015 - REVISED DECEMBER 2019 p.8
 */
#define ILCK_FACTOR_IL_HS_CS_1_ohm (80.0f / 2049.0f)

/** Linear conversion factor for low-side interlock current measurement
 *  (IL_LS_CS). Measurement performed via 2 ohm shunt with a gain of 20V/V */
#define ILCK_FACTOR_IL_LS_CS_1_ohm (0.025f)

/**
 * @brief   ILCK statemachine short time definition in #ILCK_Trigger() calls
 *          until next state/substate is processed
 */
#define ILCK_STATEMACH_SHORTTIME (1u)

/** Symbolic names for contactors' possible states */
typedef enum ILCK_ELECTRICAL_STATE_TYPE {
    ILCK_SWITCH_OFF,   /*!< Contactor off         --> Contactor is open           */
    ILCK_SWITCH_ON,    /*!< Contactor on          --> Contactor is closed         */
    ILCK_SWITCH_UNDEF, /*!< Contactor undefined   --> Contactor state not known   */
} ILCK_ELECTRICAL_STATE_TYPE_e;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__INTERLOCK_CFG_H_ */
