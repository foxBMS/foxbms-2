/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    nxp_mc33775a_alarm.c
 * @author  foxBMS Team
 * @date    2025-02-03 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup DRIVERS
 * @prefix  N77X
 *
 * @brief   Alarm line functions of the MC33775A analog front-end driver.
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "diag.h"
#include "nxp_mc3377x-ll.h"
#include "nxp_mc3377x_alarm.h"
#include "nxp_mc3377x_helpers.h"
#include "nxp_mc3377x_reg_def.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
static void N77x_ConfigureAlarmCellOvUv(N77X_STATE_s *pState);

static void N77x_ConfigureAlarmFaults(N77X_STATE_s *pState);

/*========== Static Function Implementations ================================*/
static void N77x_ConfigureAlarmCellOvUv(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);
    /* Enable pins for checking OV & UV */
    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS, MC3377X_PRMM_VC_OV_UV_CFG_OFFSET, N77X_PRMM_VC_OV_UV_CFG, pState->pSpiTxSequence);

    /* Set thresholds according to config */
    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS, MC3377X_PRMM_VC_OV_TH_CFG_OFFSET, N77X_PRMM_VC_OV_TH_CFG, pState->pSpiTxSequence);
    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS, MC3377X_PRMM_VC_UV0_TH_CFG_OFFSET, N77X_PRMM_VC_UV0_TH_CFG, pState->pSpiTxSequence);
    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS, MC3377X_PRMM_VC_UV1_TH_CFG_OFFSET, N77X_PRMM_VC_UV1_TH_CFG, pState->pSpiTxSequence);
}

static void N77x_ConfigureAlarmFaults(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);
    uint16_t alarmOutCfg   = 0;
    uint16_t supplyFltCfg0 = 0;
    uint16_t supplyFltCfg1 = 0;
    uint16_t anaFltCfg     = 0;
    uint16_t comFltCfg     = 0;
    uint16_t measFltCfg    = 0;

    alarmOutCfg |= (1 << MC3377X_FEH_ALARM_OUT_CFG_SYSFLTEVT_POS);
    alarmOutCfg |= (0 << MC3377X_FEH_ALARM_OUT_CFG_BALRDY_POS);
    alarmOutCfg |= (0 << MC3377X_FEH_ALARM_OUT_CFG_WAKEUPIN_POS);
    alarmOutCfg |= (1 << MC3377X_FEH_ALARM_OUT_CFG_ALARMIN_POS);
    alarmOutCfg |= (0 << MC3377X_FEH_ALARM_OUT_CFG_AIN3UV_POS);
    alarmOutCfg |= (0 << MC3377X_FEH_ALARM_OUT_CFG_AIN3OV_POS);
    alarmOutCfg |= (0 << MC3377X_FEH_ALARM_OUT_CFG_AIN2UV_POS);
    alarmOutCfg |= (0 << MC3377X_FEH_ALARM_OUT_CFG_AIN2OV_POS);
    alarmOutCfg |= (0 << MC3377X_FEH_ALARM_OUT_CFG_AIN1UV_POS);
    alarmOutCfg |= (0 << MC3377X_FEH_ALARM_OUT_CFG_AIN1OV_POS);
    alarmOutCfg |= (0 << MC3377X_FEH_ALARM_OUT_CFG_AIN0UV_POS);
    alarmOutCfg |= (0 << MC3377X_FEH_ALARM_OUT_CFG_AIN0OV_POS);
    alarmOutCfg |= (1 << MC3377X_FEH_ALARM_OUT_CFG_VCUV1_POS);
    alarmOutCfg |= (1 << MC3377X_FEH_ALARM_OUT_CFG_VCUV0_POS);
    alarmOutCfg |= (1 << MC3377X_FEH_ALARM_OUT_CFG_VCOV_POS);

    supplyFltCfg0 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG0_VDDCUVEN_POS);
    supplyFltCfg0 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG0_VDDCOVEN_POS);
    supplyFltCfg0 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG0_VAUXUVEN_POS);
    supplyFltCfg0 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG0_VAUXOVEN_POS);
    supplyFltCfg0 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG0_VPREREFSUVEN_POS);
    supplyFltCfg0 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG0_IBIASPERMUCEN_POS);
    supplyFltCfg0 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG0_IBIASPERMOCEN_POS);
    supplyFltCfg0 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG0_AFECPUVEN_POS);
    supplyFltCfg0 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG0_AFECPOVEN_POS);
    supplyFltCfg0 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG0_V2P5AUVEN_POS);
    supplyFltCfg0 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG0_V2P5AOVEN_POS);
    supplyFltCfg0 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG0_VDDAUVEN_POS);
    supplyFltCfg0 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG0_VDDAOVEN_POS);
    supplyFltCfg0 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG0_VBATOVEN_POS);

    supplyFltCfg1 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG1_VPREOVEN_POS);
    supplyFltCfg1 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG1_VDDIOUVEN_POS);
    supplyFltCfg1 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG1_VDDIOOVEN_POS);
    supplyFltCfg1 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG1_VDDCHCEN_POS);
    supplyFltCfg1 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG1_VPREREFSOVEN_POS);
    supplyFltCfg1 |= (1 << MC3377X_FEH_SUPPLY_FLT_EVT_CFG1_VBATLVEN_POS);

    anaFltCfg |= (1 << MC3377X_FEH_ANA_FLT_EVT_CFG_FUSEFLTEN_POS);
    anaFltCfg |= (1 << MC3377X_FEH_ANA_FLT_EVT_CFG_BALFLTEN_POS);
    anaFltCfg |= (1 << MC3377X_FEH_ANA_FLT_EVT_CFG_MONBISTEN_POS);

    comFltCfg |= (1 << MC3377X_FEH_COM_FLT_EVT_CFG_RSPLENERREN_POS);
    comFltCfg |= (1 << MC3377X_FEH_COM_FLT_EVT_CFG_ERRCNTOFEN_POS);
    comFltCfg |= (1 << MC3377X_FEH_COM_FLT_EVT_CFG_CRCERREN_POS);
    comFltCfg |= (1 << MC3377X_FEH_COM_FLT_EVT_CFG_FRAMEERREN_POS);

    measFltCfg |= (1 << MC3377X_FEH_MEAS_FLT_EVT_CFG_SYNCMEASFLTEN_POS);
    measFltCfg |= (1 << MC3377X_FEH_MEAS_FLT_EVT_CFG_SECCALCRCFLTEN_POS);
    measFltCfg |= (1 << MC3377X_FEH_MEAS_FLT_EVT_CFG_PRIMCALCRCFLTEN_POS);

    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS, MC3377X_FEH_SUPPLY_FLT_EVT_CFG0_OFFSET, supplyFltCfg0, pState->pSpiTxSequence);
    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS, MC3377X_FEH_SUPPLY_FLT_EVT_CFG1_OFFSET, supplyFltCfg1, pState->pSpiTxSequence);
    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS, MC3377X_FEH_ANA_FLT_EVT_CFG_OFFSET, anaFltCfg, pState->pSpiTxSequence);
    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS, MC3377X_FEH_COM_FLT_EVT_CFG_OFFSET, comFltCfg, pState->pSpiTxSequence);
    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS, MC3377X_FEH_MEAS_FLT_EVT_CFG_OFFSET, measFltCfg, pState->pSpiTxSequence);
    N77x_CommunicationWrite(
        N77X_BROADCAST_ADDRESS, MC3377X_FEH_ALARM_OUT_CFG_OFFSET, alarmOutCfg, pState->pSpiTxSequence);
}

/*========== Extern Function Implementations ================================*/

extern void N77x_HandleAlarm(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);

    DIAG_Handler(DIAG_ID_AFE_ALARM, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);

    for (uint8_t s = 0; s < BS_NR_OF_STRINGS; s++) {
        for (uint8_t m = 0; m < BS_NR_OF_MODULES_PER_STRING; m++) {
            N77x_CommunicationRead(
                m + 1, MC3377X_FEH_ALARM_OUT_REASON_OFFSET, &pState->alarm.alarmOutReason[s][m], pState);
            N77x_CommunicationRead(
                m + 1, MC3377X_FEH_SUPPLY_FLT_STAT0_OFFSET, &pState->alarm.supplyFaultState0[s][m], pState);
            N77x_CommunicationRead(
                m + 1, MC3377X_FEH_SUPPLY_FLT_STAT1_OFFSET, &pState->alarm.supplyFaultState1[s][m], pState);
            N77x_CommunicationRead(m + 1, MC3377X_FEH_ANA_FLT_STAT_OFFSET, &pState->alarm.anaFaultState[s][m], pState);
            N77x_CommunicationRead(m + 1, MC3377X_FEH_COM_FLT_STAT_OFFSET, &pState->alarm.comFaultState[s][m], pState);
            N77x_CommunicationRead(
                m + 1, MC3377X_FEH_MEAS_FLT_STAT_OFFSET, &pState->alarm.measFaultState[s][m], pState);
        }
    }
}

extern void N77x_CheckAlarm(N77X_STATE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);
    uint8_t input = ((gioPORT_t *)N77X_ALARM_PORT)->DIN;

    /* ALARMIN from interface is mapped to GIOA4 */
    /* Low active */
    if (!((input & (1u << 4u)) >> 4u) && pState->firstMeasurementMade) {
        pState->alarm.alarmOccurred = true;

        N77x_HandleAlarm(pState);
    }
}

extern void N77x_InitializeAlarm(N77X_STATE_s *pState) {
    uint16_t alarmCfg  = 0;
    uint16_t reasonVal = 0;

    /* Configure all slaves via broadcast */
    if (N77X_ALARM_ENABLE_INPUT) {
        alarmCfg |= (MC3377X_FEH_ALARM_CFG_ALARMIN_LOW_ACTIVE_ENUM_VAL << MC3377X_FEH_ALARM_CFG_ALARMIN_POS);
    }
    if (N77X_ALARM_ENABLE_OUTPUT) {
        alarmCfg |= (MC3377X_FEH_ALARM_CFG_ALARMOUT_LOW_ACTIVE_ENUM_VAL << MC3377X_FEH_ALARM_CFG_ALARMOUT_POS);
    }

    N77x_ConfigureAlarmCellOvUv(pState);

    N77x_ConfigureAlarmFaults(pState);

    /* Enable alarm input and output for all */
    N77x_CommunicationWrite(N77X_BROADCAST_ADDRESS, MC3377X_FEH_ALARM_CFG_OFFSET, alarmCfg, pState->pSpiTxSequence);

    /* Disable alarm input for last slave */
    N77x_CommunicationWrite(
        BS_NR_OF_MODULES_PER_STRING,
        MC3377X_FEH_ALARM_CFG_OFFSET,
        (alarmCfg & ~MC3377X_FEH_ALARM_CFG_ALARMIN_MSK),
        pState->pSpiTxSequence);

    /**
     * Read to reset all alarms occurred during initialization
     * At least the last slave triggered an alarm for not having ALARMIN disabled
     * -> Reset from last to first
     */
    for (uint8_t i = BS_NR_OF_MODULES_PER_STRING; i >= 1; i--) {
        reasonVal = 0;
        N77x_CommunicationRead(i, MC3377X_FEH_ALARM_OUT_REASON_OFFSET, &reasonVal, pState);
    }
    pState->alarm.alarmOccurred = false;

    /* Configure Master ALARM input */
    ((gioPORT_t *)N77X_ALARM_PORT)->DIR &= ~0x4u;       /* Data input direction: 0 -> input */
    ((gioPORT_t *)N77X_ALARM_PORT)->PULDIS |= 1u << 4u; /* Disable pullup/down: 1 -> disabled */
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_N77x_ConfigureAlarmCellOvUv(N77X_STATE_s *pState) {
    N77x_ConfigureAlarmCellOvUv(pState);
}

extern void TEST_N77x_ConfigureAlarmFaults(N77X_STATE_s *pState) {
    N77x_ConfigureAlarmFaults(pState);
}
#endif
