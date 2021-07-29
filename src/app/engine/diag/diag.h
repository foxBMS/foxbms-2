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
 * @file    diag.h
 * @author  foxBMS Team
 * @date    2019-11-28 (date of creation)
 * @updated 2021-07-29 (date of last update)
 * @ingroup ENGINE
 * @prefix  DIAG
 *
 * @brief   Diagnosis driver header
 *
 */

#ifndef FOXBMS__DIAG_H_
#define FOXBMS__DIAG_H_

/*========== Includes =======================================================*/
#include "diag_cfg.h"

/*========== Macros and Definitions =========================================*/

/** diagnosis handler return types */
typedef enum DIAG_RETURNTYPE {
    DIAG_HANDLER_RETURN_OK,               /*!<  error not occurred or occurred but threshold not reached */
    DIAG_HANDLER_RETURN_ERR_OCCURRED,     /*!<  error occurred and enabled */
    DIAG_HANDLER_RETURN_WARNING_OCCURRED, /*!<  warning occurred (error occurred but not enabled) */
    DIAG_HANDLER_RETURN_WRONG_ID,         /*!<  wrong diagnosis id */
    DIAG_HANDLER_RETURN_UNKNOWN,          /*!<  unknown return type */
    DIAG_HANDLER_INVALID_TYPE,            /*!<  invalid diagnosis type, error in configuration */
    DIAG_HANDLER_INVALID_DATA,            /*!<  invalid data, dependent of the diagHandler */
    DIAG_HANDLER_INVALID_ERR_IMPACT,      /*!<  event neither string nor system level related */
    DIAG_HANDLER_RETURN_NOT_READY,        /*!<  diagnosis handler not ready */
} DIAG_RETURNTYPE_e;

/**
 * state of diagnosis module
 */
typedef enum {
    DIAG_STATE_UNINITIALIZED, /*!< diagnosis module not initialized */
    DIAG_STATE_INITIALIZED,   /*!< diagnosis module initialized (ready for use) */
} DIAG_STATE_e;

/** central state struct of the diag module */
typedef struct DIAG {
    DIAG_STATE_e state;                                        /*!< actual state of diagnosis module */
    uint16_t errcnttotal;                                      /*!< total counts of diagnosis entry records*/
    uint16_t errcntreported;                                   /*!< reported error counts to external tool*/
    uint32_t entry_event[DIAG_ID_MAX];                         /*!< last detected entry event*/
    uint8_t entry_cnt[DIAG_ID_MAX];                            /*!< reported event counter used for limitation  */
    uint16_t occurrenceCounter[BS_NR_OF_STRINGS][DIAG_ID_MAX]; /*!< counter for the occurrence of diag events */
    uint8_t id2ch[DIAG_ID_MAX];                                /*!< diagnosis-id to configuration channel selector*/
    uint8_t nr_of_ch;                                          /*!< number of configured channels*/
    uint32_t errflag[(DIAG_ID_MAX + 31) / 32];                 /*!< detected error flags (bit_nr = diag_id) */
    uint32_t warnflag[(DIAG_ID_MAX + 31) / 32];                /*!< detected warning flags (bit_nr = diag_id) */
    uint32_t err_enableflag[(DIAG_ID_MAX + 31) / 32];          /*!< enabled error flags (bit_nr = diag_id) */
} DIAG_s;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   DIAG_Handler provides generic error handling, based on diagnosis
 *          group.
 * @details This function calls the handler functions depending on the
 *          diagnosis group of call. It needs to get called in every function
 *          which wants to apply some kind of diagnosis handling. According to
 *          its return value further treatment is either left to the calling
 *          module itself, or can be done in the callback function defined in
 *          diag_cfg.c
 * @ingroup API_DIAG
 * @param   diag_id #DIAG_ID_e of the event that has occurred
 * @param   event   event that occurred (OK, NOK, RESET)
 * @param   impact  #DIAG_IMPACT_LEVEL_e of #DIAG_ID_e
 * @param   data    individual information for #DIAG_ID_e e.g. string number,..
 * @return  #DIAG_HANDLER_RETURN_UNKNOWN if invalid #DIAG_EVENT_e, otherwise
 *          return value of #DIAG_RETURNTYPE_e
 */
extern DIAG_RETURNTYPE_e DIAG_Handler(DIAG_ID_e diag_id, DIAG_EVENT_e event, DIAG_IMPACT_LEVEL_e impact, uint32_t data);

/**
 * @brief   DIAG_CheckEvent provides a simple interface to check an event for
 *          #STD_OK
 * @details DIAG_CheckEvent is a wrapper function for #DIAG_Handler(). In simple
 *          cases where a return value that is not #STD_OK (or a 0 casted to
 *          #STD_OK) should increase the error counter in a diagnosis channel,
 *          this function should be used instead of directly calling the
 *          #DIAG_Handler().
 * @param   cond    condition
 * @param   diag_id event ID of the event that has occurred
 * @param   impact  #DIAG_IMPACT_LEVEL_e of #DIAG_ID_e
 * @param   data    individual information for #DIAG_ID_e e.g. string number,..
 * @return  STD_OK if ok, STD_NOT_OK if not ok
 */
extern STD_RETURN_TYPE_e DIAG_CheckEvent(
    STD_RETURN_TYPE_e cond,
    DIAG_ID_e diag_id,
    DIAG_IMPACT_LEVEL_e impact,
    uint32_t data);

/**
 * @brief   DIAG_Init initializes all needed structures/buffers.
 * @details This function provides initialization of the diagnose module. In
 *          case of miss behaviour it calls Reset and adds an entry into
 *          database to ensure data validity/report back malfunction
 * @param   diag_dev_pointer
 * @return  #STD_OK if ok, #STD_NOT_OK if not ok
 */
extern STD_RETURN_TYPE_e DIAG_Initialize(DIAG_DEV_s *diag_dev_pointer);

/**
 * @brief   Checks if passed diagnosis entry has been triggered or not
 * @param   diagnosisEntry event ID of diagnosis entry
 * @return  #STD_OK if diagnosis entry has not surpassed error threshold, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e DIAG_GetDiagnosisEntryState(DIAG_ID_e diagnosisEntry);

/**
 * @brief   Prints contents of the error buffer on user request.
 */
extern void DIAG_PrintErrors(void);

/**
 * @brief   Get configured delay of passed diagnosis entry
 * @param   diagnosisEntry event ID of diagnosis entry
 * @return  configured delay in ms
 */
extern uint32_t DIAG_GetDelay(DIAG_ID_e diagnosisEntry);

/**
 * @brief   Check if any fatal error is set
 * @return  true, if a diagnosis entry with severity #DIAG_FATAL_ERROR is set,
 *          otherwise false
 */
extern bool DIAG_IsAnyFatalErrorSet(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__DIAG_H_ */
