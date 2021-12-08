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
 * @file    sys_mon_cfg.h
 * @author  foxBMS Team
 * @date    2019-11-28 (date of creation)
 * @updated 2021-11-10 (date of last update)
 * @ingroup ENGINE_CONFIGURATION
 * @prefix  SYSM
 *
 * @brief   TODO
 */

#ifndef FOXBMS__SYS_MON_CFG_H_
#define FOXBMS__SYS_MON_CFG_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "diag_cfg.h"

/*========== Macros and Definitions =========================================*/
/** diagnosis handling type for system monitoring */
typedef enum SYSM_HANDLING_TYPE {
    SYSM_HANDLING_DONOTHING,         /**< */
    SYSM_HANDLING_SWITCHOFFCONTACTOR /**< */
} SYSM_HANDLING_TYPE_e;

/**
 * listing of system-relevant tasks or functions which are checked by
 * the system monitoring
 */
typedef enum SYSM_TASK_ID {
    SYSM_TASK_ID_ENGINE,                 /**< diag entry for engine task                 */
    SYSM_TASK_ID_CYCLIC_1ms,             /**< diag entry for engine cyclic 1ms task      */
    SYSM_TASK_ID_CYCLIC_10ms,            /**< diag entry for engine cyclic 10ms task     */
    SYSM_TASK_ID_CYCLIC_100ms,           /**< diag entry for engine cyclic 100ms task    */
    SYSM_TASK_ID_CYCLIC_ALGORITHM_100ms, /**< diag entry for algorithm cyclic 100ms task */
    SYSM_TASK_ID_MAX                     /**< end marker do not delete               */
} SYSM_TASK_ID_e;

/** recording activation */
typedef enum SYSM_RECORDING {
    SYSM_RECORDING_ENABLED,  /*!< enable event recording  */
    SYSM_RECORDING_DISABLED, /*!< disable event recording */
} SYSM_RECORDING_e;

/** enable diagnosis entry for system monitoring */
typedef enum SYSM_ACTIVATE {
    SYSM_ENABLED,  /**< TODO */
    SYSM_DISABLED, /**< TODO */
} SYSM_ACTIVATE_e;

/** Channel configuration of one system monitoring channel */
typedef struct SYSM_CH_CFG {
    SYSM_TASK_ID_e id;                           /**< the task id by its symbolic name                           */
    SYSM_ACTIVATE_e enable;                      /**< enable or disable system monitoring                        */
    uint8_t cycleTime;                           /**< max. delay time in ms until #SYSM_Notify has to be called  */
    uint8_t maxJitter;                           /**< max. jitter in ms                                          */
    SYSM_RECORDING_e enableRecording;            /**< enabled if set to DIAG_RECORDING_ENABLED                   */
    SYSM_HANDLING_TYPE_e handlingType;           /**< type of handling of system monitoring errors               */
    void (*callbackfunc)(SYSM_TASK_ID_e taskID); /**< callback in case of error                                  */
} SYSM_MONITORING_CFG_s;

/*========== Extern Constant and Variable Declarations ======================*/
/** diag system monitoring struct */
extern SYSM_MONITORING_CFG_s sysm_ch_cfg[];

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   initialization function for system monitoring
 * @details Has to be called once after startup to initialize and check system
 *          monitoring related functionality
 * @return  #STD_OK if no configuration error detected, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e SYSM_Init(void);

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_SYSM_DummyCallback(SYSM_TASK_ID_e taskId);
#endif

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__SYS_MON_CFG_H_ */
