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
 * @file    diag.c
 * @author  foxBMS Team
 * @date    2019-11-28 (date of creation)
 * @updated 2021-11-09 (date of last update)
 * @ingroup ENGINE
 * @prefix  DIAG
 *
 * @brief   Diagnosis driver implementation
 *
 * This diagnose module is responsible for error handling and reporting.
 * Reported errors are logged into the global database and can be reviewed
 * on user request.
 */

/*========== Includes =======================================================*/
#include "diag.h"

#include "os.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
/** state-variable of the diag module */
static DIAG_s diag;

/** pointer to the device configuration of the diag module */
static DIAG_DEV_s *diag_devptr;

/** superb implementation of a mutex for the diag module */
static uint8_t diag_locked = 0;

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
static void DIAG_Reset(void);

/**
 * @brief   DIAG_EntryWrite adds an error entry.
 * @details This function adds an entry to the error buffer. It provides some
 *          functionality to prevent duplicates from being logged. Multiple
 *          occurring error doesn't get logged anymore after they reached a
 *          pre-defined error count.
 * @param  eventID  ID of entry
 * @param  event    OK, NOK or RESET
 * @param  data     individual information for #DIAG_ID_e e.g. string number,..
 * @return 0xFF if event is logged, otherwise 0
 */
static uint8_t DIAG_EntryWrite(uint8_t eventID, DIAG_EVENT_e event, uint32_t data);

/*========== Static Function Implementations ================================*/
/**
 * @brief   DIAG_Reset resetsall needed structures
 * @details This function gets called during initialization of the diagnose
 *          module. It clears memory and counters used by diag later on.
 */

static void DIAG_Reset(void) {
    diag_locked = 1;

    /* Reset counter */
    for (uint32_t i = 0u; i < sizeof(diag.entry_cnt); i++) {
        diag.entry_cnt[i] = 0;
    }
    diag.errcnttotal = 0;
    diag_locked      = 0;
}

/*========== Extern Function Implementations ================================*/
STD_RETURN_TYPE_e DIAG_Initialize(DIAG_DEV_s *diag_dev_pointer) {
    FAS_ASSERT(diag_dev_pointer != NULL_PTR);

    STD_RETURN_TYPE_e retval = STD_OK;
    uint8_t id_nr            = (uint8_t)DIAG_ID_MAX;
    /* take assumptions on the value of DIAG_ID_MAX */
    FAS_ASSERT((uint16_t)DIAG_ID_MAX < UINT8_MAX);
    uint32_t tmperr_Check[((uint16_t)DIAG_ID_MAX + 31u) / 32u] = {0};

    diag_devptr = diag_dev_pointer;

    diag.state         = DIAG_STATE_UNINITIALIZED;
    uint16_t checkfail = 0u;

    /* TODO this will always evaluate to true?! */
    if (checkfail > 0u) {
        DIAG_Reset();
    }

    /* Fill lookup table id2ch */
    for (uint8_t c = 0; c < diag_dev_pointer->nr_of_ch; c++) {
        id_nr = diag_dev_pointer->ch_cfg[c].id;
        if (id_nr < (uint16_t)DIAG_ID_MAX) {
            diag.id2ch[id_nr] = c; /* e.g. diag.id2ch[DIAG_ID_90] = configured channel index */
        } else {
            /* Configuration error -> set retval to #STD_NOT_OK */
            checkfail |= 0x20u;
            retval = STD_NOT_OK;
        }
    }

    for (uint8_t i = 0; i < (uint8_t)(((uint16_t)DIAG_ID_MAX + 31u) / 32u); i++) {
        tmperr_Check[i] = 0u;
    }

    /* Fill enable array err_enableflag */
    for (uint8_t i = 0; i < diag_dev_pointer->nr_of_ch; i++) {
        if (diag_dev_pointer->ch_cfg[i].enable_evaluate == DIAG_EVALUATION_DISABLED) {
            /* Disable diagnosis entry */
            tmperr_Check[diag_dev_pointer->ch_cfg[i].id / 32] |= (1 << (diag_dev_pointer->ch_cfg[i].id % 32));
        }
    }

    /* take over configured error enable masks*/
    for (uint8_t c = 0; c < (uint8_t)(((uint16_t)DIAG_ID_MAX + 31u) / 32u); c++) {
        diag.err_enableflag[c] = ~tmperr_Check[c];
    }

    /* Reset counter in case init function is called multiple times */
    diag_dev_pointer->numberOfFatalErrors = 0u;
    /* Fill pointer array with links to all diagnosis entries with a fatal error */
    for (uint16_t entry = 0u; entry < (uint16_t)DIAG_ID_MAX; entry++) {
        if (DIAG_ID_cfg[entry].severity == DIAG_FATAL_ERROR) {
            diag_dev_pointer->pFatalErrorLinkTable[diag_dev_pointer->numberOfFatalErrors] = &DIAG_ID_cfg[entry];
            diag_dev_pointer->numberOfFatalErrors++;
        }
    }

    diag.state = DIAG_STATE_INITIALIZED;
    return retval;
}

STD_RETURN_TYPE_e DIAG_GetDiagnosisEntryState(DIAG_ID_e diagnosisEntry) {
    STD_RETURN_TYPE_e retval      = STD_OK;
    const uint16_t errorThreshold = diag_devptr->ch_cfg[diag.id2ch[(uint16_t)diagnosisEntry]].threshold;

    /* Error if active if threshold counter is larger than configured error threshold */
    for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++) {
        uint16_t thresholdCounter = diag.occurrenceCounter[s][(uint16_t)diagnosisEntry];
        if (thresholdCounter > errorThreshold) {
            /* error-threshold exceeded -> error is active */
            retval = STD_NOT_OK;
        }
    }
    return retval;
}

void DIAG_PrintErrors(void) {
}

static uint8_t DIAG_EntryWrite(uint8_t eventID, DIAG_EVENT_e event, uint32_t data) {
    uint8_t ret_val = 0;
    if (diag_locked > 0u) {
        return ret_val; /* only locked when clearing the diagnosis memory */
    }

    if (diag.entry_event[eventID] == event) {
        /* same event of same error type already recorded before -> ignore until event toggles */
        return ret_val;
    }
    if ((diag.entry_event[eventID] == DIAG_EVENT_OK) && (event == DIAG_EVENT_RESET)) {
        /* do record DIAG_EVENT_RESET-event only if last event was an error (re-initialization) */
        /* meaning: DIAG_EVENT_RESET-event at first time call or after DIAG_EVENT_OK-event will not be recorded */
        return ret_val;
    }

    if (++diag.entry_cnt[eventID] > DIAG_MAX_ENTRIES_OF_ERROR) {
        /* this type of error has been recorded too many times -> ignore to avoid filling buffer with same failurecodes */
        diag.entry_cnt[eventID] = DIAG_MAX_ENTRIES_OF_ERROR;
        return ret_val;
    }

    /* now record failurecode */
    ret_val = 0xFF;

    /* counts of (new) diagnosis entry records which is still not been read by
     * external Tool which will reset this value to 0 after having read all
     * new entries which means <acknowledged by user> */
    ++diag.errcntreported;
    ++diag.errcnttotal; /* total counts of diagnosis entry records */
    diag.entry_event[eventID] = event;

    return ret_val;
}

DIAG_RETURNTYPE_e DIAG_Handler(DIAG_ID_e diag_id, DIAG_EVENT_e event, DIAG_IMPACT_LEVEL_e impact, uint32_t data) {
    DIAG_RETURNTYPE_e ret_val      = DIAG_HANDLER_RETURN_UNKNOWN;
    uint32_t *u32ptr_errCodemsk    = NULL_PTR;
    uint32_t *u32ptr_warnCodemsk   = NULL_PTR;
    uint16_t *u16ptr_threshcounter = NULL_PTR;
    uint16_t cfg_threshold         = 0;
    uint16_t err_enable_idx        = 0;
    uint32_t err_enable_bitmask    = 0;

    DIAG_RECORDING_e recording_enabled;
    DIAG_EVALUATE_e evaluate_enabled;

    if (diag.state == DIAG_STATE_UNINITIALIZED) {
        return DIAG_HANDLER_RETURN_NOT_READY;
    }

    if (diag_id >= DIAG_ID_MAX) {
        return DIAG_HANDLER_RETURN_WRONG_ID;
    }

    if (!((impact == DIAG_SYSTEM) || (DIAG_STRING))) {
        return DIAG_HANDLER_INVALID_ERR_IMPACT;
    }

    if ((impact == DIAG_STRING) && (data >= BS_NR_OF_STRINGS)) {
        return DIAG_HANDLER_INVALID_DATA;
    }

    /*  Determine a stringID, for impact level #DIAG_SYSTEM this is
        always 0. This stringID is used to access the #DIAG_s::occurrenceCounter
        2D-array.
    */
    uint8_t stringID = 0u;
    if (impact == DIAG_STRING) {
        stringID = data;
    }

    err_enable_idx     = diag_id / 32;        /* array index of diag.err_enableflag[..] */
    err_enable_bitmask = 1 << (diag_id % 32); /* bit number (mask) of diag.err_enableflag[idx] */

    u32ptr_errCodemsk    = &diag.errflag[err_enable_idx];
    u32ptr_warnCodemsk   = &diag.warnflag[err_enable_idx];
    u16ptr_threshcounter = &diag.occurrenceCounter[stringID][diag_id];
    cfg_threshold        = diag_devptr->ch_cfg[diag.id2ch[diag_id]].threshold;
    recording_enabled    = diag_devptr->ch_cfg[diag.id2ch[diag_id]].enable_recording;
    evaluate_enabled     = diag_devptr->ch_cfg[diag.id2ch[diag_id]].enable_evaluate;

    if (event == DIAG_EVENT_OK) {
        if ((diag.err_enableflag[err_enable_idx] & err_enable_bitmask) > 0u) {
            /* if (((*u16ptr_threshcounter) == 0) && (*u32ptr_errCodemsk == 0)) */
            if (((*u16ptr_threshcounter) == 0)) {
                /* everything ok, nothing to be handled */
            } else if ((*u16ptr_threshcounter) > 1) {
                (*u16ptr_threshcounter)--; /* Error did not occur, decrement Error-Counter */
            } else if ((*u16ptr_threshcounter) == 1) {
                /* else if ((*u16ptr_threshcounter) <= 1) */
                /* Error did not occur, now decrement to zero and clear Error- or Warning-Flag and make recording if enabled */
                *u32ptr_errCodemsk &= ~err_enable_bitmask;  /* ERROR:   clear corresponding bit in errflag[idx] */
                *u32ptr_warnCodemsk &= ~err_enable_bitmask; /* WARNING: clear corresponding bit in warnflag[idx] */
                (*u16ptr_threshcounter) = 0;
                /* Make entry in error-memory (error disappeared) */
                if (recording_enabled == DIAG_RECORDING_ENABLED) {
                    DIAG_EntryWrite(diag_id, event, data);
                }

                if (evaluate_enabled == DIAG_EVALUATION_ENABLED) {
                    /* Call callback function and reset error */
                    DIAG_ID_cfg[diag.id2ch[diag_id]].fpCallback(diag_id, DIAG_EVENT_RESET, &diag_kDatabaseShim, data);
                }
            }
        }
        ret_val = DIAG_HANDLER_RETURN_OK; /* Function does not return an error-message! */
    } else if (event == DIAG_EVENT_NOT_OK) {
        if ((diag.err_enableflag[err_enable_idx] & err_enable_bitmask) > 0u) {
            if ((*u16ptr_threshcounter) < cfg_threshold) {
                (*u16ptr_threshcounter)++;        /* error-threshold not exceeded yet, increment Error-Counter */
                ret_val = DIAG_HANDLER_RETURN_OK; /* Function does not return an error-message! */
            } else if ((*u16ptr_threshcounter) == cfg_threshold) {
                /* Error occured AND error-threshold exceeded */
                (*u16ptr_threshcounter)++;
                *u32ptr_errCodemsk |= err_enable_bitmask;   /* ERROR:   set corresponding bit in errflag[idx] */
                *u32ptr_warnCodemsk &= ~err_enable_bitmask; /* WARNING: clear corresponding bit in warnflag[idx] */

                /* Make entry in error-memory (error occurred) */
                if (recording_enabled == DIAG_RECORDING_ENABLED) {
                    DIAG_EntryWrite(diag_id, event, data);
                }

                if (evaluate_enabled == DIAG_EVALUATION_ENABLED) {
                    /* Call callback function and set error */
                    DIAG_ID_cfg[diag.id2ch[diag_id]].fpCallback(diag_id, DIAG_EVENT_NOT_OK, &diag_kDatabaseShim, data);
                }
                /* Function returns an error-message! */
                ret_val = DIAG_HANDLER_RETURN_ERR_OCCURRED;
            } else if (((*u16ptr_threshcounter) > cfg_threshold)) {
                /* error-threshold already exceeded, nothing to be handled */
                ret_val = DIAG_HANDLER_RETURN_ERR_OCCURRED;
            }
        } else {
            /* Error occured BUT NOT enabled by mask */
            *u32ptr_errCodemsk &= ~err_enable_bitmask;      /* ERROR:   clear corresponding bit in errflag[idx] */
            *u32ptr_warnCodemsk |= err_enable_bitmask;      /* WARNING: set corresponding bit in warnflag[idx] */
            ret_val = DIAG_HANDLER_RETURN_WARNING_OCCURRED; /* Function returns an error-message! */
        }
    } else if (event == DIAG_EVENT_RESET) {
        if ((diag.err_enableflag[err_enable_idx] & err_enable_bitmask) > 0u) {
            /* clear counter, Error-, Warning-Flag and make recording if enabled */
            *u32ptr_errCodemsk &= ~err_enable_bitmask;  /* ERROR:   clear corresponding bit in errflag[idx] */
            *u32ptr_warnCodemsk &= ~err_enable_bitmask; /* WARNING: clear corresponding bit in warnflag[idx] */
            (*u16ptr_threshcounter) = 0;
            if (recording_enabled == DIAG_RECORDING_ENABLED) {
                DIAG_EntryWrite(
                    diag_id,
                    event,
                    data); /* Make entry in error-memory (error disappeared) if error was recorded before */
            }
            if (evaluate_enabled == DIAG_EVALUATION_ENABLED) {
                /* Call callback function and reset error */
                DIAG_ID_cfg[diag.id2ch[diag_id]].fpCallback(diag_id, DIAG_EVENT_RESET, &diag_kDatabaseShim, data);
            }
        }
        ret_val = DIAG_HANDLER_RETURN_OK; /* Function does not return an error-message! */
    }

    return ret_val;
}

STD_RETURN_TYPE_e DIAG_CheckEvent(
    STD_RETURN_TYPE_e cond,
    DIAG_ID_e diag_id,
    DIAG_IMPACT_LEVEL_e impact,
    uint32_t data) {
    STD_RETURN_TYPE_e retVal = STD_NOT_OK;

    if (cond == STD_OK) {
        DIAG_Handler(diag_id, DIAG_EVENT_OK, impact, data);
    } else {
        DIAG_Handler(diag_id, DIAG_EVENT_NOT_OK, impact, data);
    }

    return retVal;
}

uint32_t DIAG_GetDelay(DIAG_ID_e diagnosisEntry) {
    FAS_ASSERT(diagnosisEntry < DIAG_ID_MAX);
    return DIAG_ID_cfg[diag.id2ch[(uint16_t)diagnosisEntry]].delay_ms;
}

bool DIAG_IsAnyFatalErrorSet(void) {
    bool fatalErrorActive = false;
    for (uint16_t entry = 0u; entry < diag_device.numberOfFatalErrors; entry++) {
        const STD_RETURN_TYPE_e diagnosisState =
            DIAG_GetDiagnosisEntryState(diag_device.pFatalErrorLinkTable[entry]->id);
        if (STD_NOT_OK == diagnosisState) {
            fatalErrorActive = true;
        }
    }
    return fatalErrorActive;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
