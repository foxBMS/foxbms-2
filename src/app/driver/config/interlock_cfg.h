/**
 *
 * @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der
 *  angewandten Forschung e.V. All rights reserved.
 *
 * BSD 3-Clause License
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1.  Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer
 * to foxBMS in your hardware, software, documentation or advertising
 * materials:
 *
 * &Prime;This product uses parts of foxBMS&reg;&Prime;
 *
 * &Prime;This product includes parts of foxBMS&reg;&Prime;
 *
 * &Prime;This product is derived from foxBMS&reg;&Prime;
 *
 */

/**
 * @file    interlock_cfg.h
 * @author  foxBMS Team
 * @date    2020-02-24 (date of creation)
 * @updated 2020-02-24 (date of last update)
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

#include "HL_gio.h"

/*========== Macros and Definitions =========================================*/

/** IO register to which the interlock is connected */
#define ILCK_IO_REG (gioPORTA)

/**
 * The number of defines per contactor must be the same as the length
 *  of the array ilck_contactors_cfg in contactor_cfg.c
 * Every contactor consists of 1 control pin and 1 feedback pin
 * counting together as 1 contactor.
 * E.g. if you have 1 contactor your define has to be:
 *      \verbatim #define ILCK_INTERLOCK_CONTROL       (PIN_MCU_0_INTERLOCK_CONTROL)\endverbatim
 *      \verbatim #define ILCK_INTERLOCK_FEEDBACK      (PIN_MCU_0_INTERLOCK_FEEDBACK)\endverbatim
 */
#define ILCK_INTERLOCK_CONTROL (6U)

/** Defines the pin where interlock feedback pin is connected to the mcu */
#define ILCK_INTERLOCK_FEEDBACK (0U)

/**
 * This define MUST represent the cycle time of the task in which context the
 * functions run, e.g., if the ILCK_Trigger() is running in the 10 ms task
 * then the define must be set to 10.
 *
 * This define also sets the minimum time.
 */

#define ILCK_TASK_CYCLE_CONTEXT_MS (10)

/** ILCK statemachine short time definition in ms */
#define ILCK_STATEMACH_SHORTTIME_MS (ILCK_TASK_CYCLE_CONTEXT_MS)

/** Symbolic names for contactors' possible states */
typedef enum ILCK_ELECTRICAL_STATE_TYPE {
    ILCK_SWITCH_OFF,   /*!< Contactor off         --> Contactor is open           */
    ILCK_SWITCH_ON,    /*!< Contactor on          --> Contactor is closed         */
    ILCK_SWITCH_UNDEF, /*!< Contactor undefined   --> Contactor state not known   */
} ILCK_ELECTRICAL_STATE_TYPE_e;

/** Symbolic names defining the electric behavior of the contactor */
typedef enum ILCK_FEEDBACK_TYPE {
    ILCK_FEEDBACK_NORMALLY_OPEN,   /*!< Feedback line of a contactor is normally open      */
    ILCK_FEEDBACK_NORMALLY_CLOSED, /*!< Feedback line of a contactor is normally closed    */
    ILCK_FEEDBACK_TYPE_DONT_CARE,  /*!< Feedback line of the contactor is not used         */
} ILCK_FEEDBACK_TYPE_e;

/**
 * struct describing the electrical state (expected and measured) of the interlock
 */
typedef struct ILCK_ELECTRICAL_STATE {
    ILCK_ELECTRICAL_STATE_TYPE_e set;
    ILCK_ELECTRICAL_STATE_TYPE_e feedback;
} ILCK_ELECTRICAL_STATE_s;

/** struct decribing the hardware configuration of the interlock */
typedef struct ILCK_CONFIG {
    /* IO_PORTS_e control_pin; */
    /* IO_PORTS_e feedback_pin; */
    uint32_t control_pin;
    uint32_t feedback_pin;
    ILCK_FEEDBACK_TYPE_e feedback_pin_type;
} ILCK_CONFIG_s;

/*========== Extern Constant and Variable Declarations ======================*/
/** configuration of the interlock */
extern ILCK_CONFIG_s ilck_interlock_config;
/** electrical state of the interlock */
extern ILCK_ELECTRICAL_STATE_s ilck_interlock_state;

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__INTERLOCK_CFG_H_ */
