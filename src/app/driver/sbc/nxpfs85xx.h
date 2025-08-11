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
 * @file    nxpfs85xx.h
 * @author  foxBMS Team
 * @date    2020-03-18 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  FS85
 *
 * @brief   Header for the driver for the FRAM module
 * @details It must always be used when creating new c header files.
 */

#ifndef FOXBMS__NXPFS85XX_H_
#define FOXBMS__NXPFS85XX_H_

/*========== Includes =======================================================*/
#include "fram_cfg.h"

#include "sbc_fs8x.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** struct for fail-safe registers
 * for register description see data sheet FS84_FS85 - Rev. 3.0 - 9 April 2019 */
typedef struct {
    uint16_t grl_flags;                             /*!< FS8X_FS_GRL_FLAGS_ADDR */
    uint16_t iOvervoltageUndervoltageSafeReaction1; /*!< FS8X_FS_I_OVUV_SAFE_REACTION1_ADDR */
    uint16_t iOvervoltageUndervoltageSafeReaction2; /*!< FS8X_FS_I_OVUV_SAFE_REACTION2_ADDR */
    uint16_t iWatchdogConfiguration;                /*!< FS8X_FS_I_WD_CFG_ADDR */
    uint16_t i_safe_inputs;                         /*!< FS8X_FS_I_SAFE_INPUTS_ADDR */
    uint16_t iFailSafeSateMachine;                  /*!< FS8X_FS_I_FSSM_ADDR */
    uint16_t i_svs;                                 /*!< FS8X_FS_I_SVS_ADDR */
    uint16_t watchdogWindow;                        /*!< FS8X_FS_WD_WINDOW_ADDR */
    uint16_t watchdogSeed;                          /*!< FS8X_FS_WD_SEED_ADDR */
    uint16_t watchdogAnswer;                        /*!< FS8X_FS_WD_ANSWER_ADDR */
    uint16_t overvoltageUndervoltageRegisterStatus; /*!< FS8X_FS_OVUVREG_STATUS_ADDR */
    uint16_t releaseFs0bPin;                        /*!< FS8X_FS_RELEASE_FS0B_ADDR */
    uint16_t safeIos;                               /*!< FS8X_FS_SAFE_IOS_ADDR */
    uint16_t diag_safety;                           /*!< FS8X_FS_DIAG_SAFETY_ADDR */
    uint16_t intb_mask;                             /*!< FS8X_FS_INTB_MASK_ADDR */
    uint16_t states;                                /*!< FS8X_FS_STATES_ADDR */
} FS85_FS_REGISTER_s;

/** struct for main registers
 * for register description see data sheet FS84_FS85 - Rev. 3.0 - 9 April 2019 */
typedef struct {
    uint16_t flag;              /*!< FS8X_M_FLAG_ADDR */
    uint16_t mode;              /*!< FS8X_M_MODE_ADDR */
    uint16_t registerControl1;  /*!< FS8X_M_REG_CTRL1_ADDR */
    uint16_t registerControl2;  /*!< FS8X_M_REG_CTRL2_ADDR */
    uint16_t analogMultiplexer; /*!< FS8X_M_AMUX_ADDR */
    uint16_t clock;             /*!< FS8X_M_CLOCK_ADDR */
    uint16_t int_mask1;         /*!< FS8X_M_INT_MASK1_ADDR */
    uint16_t int_mask2;         /*!< FS8X_M_INT_MASK2_ADDR */
    uint16_t flag1;             /*!< FS8X_M_FLAG1_ADDR */
    uint16_t flag2;             /*!< FS8X_M_FLAG2_ADDR */
    uint16_t vmon_regx;         /*!< FS8X_M_VMON_REGX_ADDR */
    uint16_t lvb1_svs;          /*!< FS8X_M_LVB1_SVS_ADDR */
    uint16_t memory0;           /*!< FS8X_M_MEMORY0_ADDR */
    uint16_t memory1;           /*!< FS8X_M_MEMORY1_ADDR */
    uint16_t deviceId;          /*!< FS8X_M_DEVICEID_ADDR */
} FS85_MAIN_REGISTERS_s;

typedef enum {
    FS85_NORMAL_MODE,
    FS85_DEBUG_MODE,
} FS85_OPERATION_MODE_e;

/** struct for FIN configuration */
typedef struct {
    bool finUsed;                /*!< flag if FIN feature of SBC is used */
    STD_RETURN_TYPE_e finState;  /*!< Is set to STD_NOT_OK if short circuit between FIN and RSTB pin detected */
    volatile uint32_t *pGIOport; /*!< pointer to port where FIN pin of SBC is connected to */
    uint32_t pin;                /*!< pin where FIN pin of SBC is connected to */
} FS85_FIN_CONFIGURATION_s;

/** stores a pointer to the persistent entry in the FRAM */
typedef struct {
    FRAM_BLOCK_ID_e entry;  /*!< FRAM ID of persistent SBC entry in FRAM */
    FRAM_SBC_INIT_s *pData; /*!< pointer to SBC entry in FRAM module */
} FS85_NVRAM_INFO_s;

/** state struct to create FS85xx instance */
typedef struct {
    SPI_INTERFACE_CONFIG_s *pSpiInterface; /*< pointer to used SPI interface configuration */
    fs8x_drv_data_t configValues;          /*!< configuration of used communication interface */
    FS85_FIN_CONFIGURATION_s fin;          /*!< configurations for FIN functionality */
    FS85_MAIN_REGISTERS_s mainRegister;    /*!< FS85xx main registers */
    FS85_FS_REGISTER_s fsRegister;         /*!< FS85xx safety registers */
    FS85_NVRAM_INFO_s nvram;    /*!< configuration and data for persistent memory required for initialization */
    FS85_OPERATION_MODE_e mode; /*!< current operation mode of FS85xx */
} FS85_STATE_s;

/*========== Extern Constant and Variable Declarations ======================*/
extern FS85_STATE_s fs85xx_mcuSupervisor;

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief           Configures SBC during INIT_FS phase
 * @details         Checks first if SBC currently is in INIT_FS phase and if
 *                  not transfers SBC back into INIT_FS. Verifies basic checks,
 *                  configures fail-safe registers and closes INIT_FS
 *                  afterwards.
 * @param[in,out]   pInstance   SBC instance that is initialized
 * @return          #STD_OK if all checks were successful and SBC configured
 *                  correctly, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e FS85_InitializeFsPhase(FS85_STATE_s *pInstance);

/**
 * @brief           Calculates the number of required watchdog refresh to reset
 *                  fault error counter
 * @param[in,out]   pInstance                   SBC instance that is
 *                                              initialized
 * @param[out]      pRequiredWatchdogRefreshes  number of required good
 *                                              watchdog refreshes
 * @return          #STD_OK if required watchdog refreshes were calculated
 *                  successfully, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e FS85_InitializeNumberOfRequiredWatchdogRefreshes(
    FS85_STATE_s *pInstance,
    uint8_t *pRequiredWatchdogRefreshes);

/**
 * @brief           Checks if fault error counter is zero
 * @param[in,out]   pInstance   SBC instance where fault error counter is
 *                              checked
 * @return          #STD_OK if fault error counter equals zero, otherwise
 *                  #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e FS85_CheckFaultErrorCounter(FS85_STATE_s *pInstance);

/**
 * @brief           Performs SBC safety path checks
 * @details         Function perform safety path checks for FIN, FS0B and RSTB
 *                  to ensure that all pins work as expected
 * @param[in,out]   pInstance where the safety paths are checked
 * @return          #STD_OK if safety path check successful, otherwise
 *                  #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e FS85_SafetyPathChecks(FS85_STATE_s *pInstance);

/**
 * @brief           Trigger watchdog
 * @details         Triggers watchdog of passed SBC instance and verify if it
 *                  was good refresh within the configured window
 * @param[in,out]   pInstance   SBC instance where the watchdog is triggered
 * @return          #STD_OK if watchdog has been triggered successfully,
 *                  otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e FS85_TriggerWatchdog(FS85_STATE_s *pInstance);

/**
 * @brief           Check WAKE1 Signal at SBC
 * @details         Checks ignition signal at WAKE1 pin of SBC. If falling
 *                  edge is detected, send SBC to Standby.
 * @param[in,out]   pInstance   SBC instance where WAKE1 is checked
 * @return          true if ignition signal is detected, otherwise false
 */
extern bool FS85_CheckIgnitionSignal(FS85_STATE_s *pInstance);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
extern STD_RETURN_TYPE_e TEST_FS85_CheckRegisterValues(uint32_t registerValue, uint32_t expectedRegisterValue);
extern void TEST_FS85_UpdateRegister(
    FS85_STATE_s *pInstance,
    bool isFailSafe,
    uint32_t registerAddress,
    uint32_t registerValue);
extern void TEST_FS85_UpdateFailSafeRegister(
    FS85_FS_REGISTER_s *pFsRegister,
    uint32_t registerAddress,
    uint32_t registerValue);
extern void TEST_FS85_UpdateMainRegister(
    FS85_MAIN_REGISTERS_s *pMainRegister,
    uint32_t registerAddress,
    uint32_t registerValue);
extern STD_RETURN_TYPE_e TEST_FS85_ReadBackRegister(FS85_STATE_s *pInstance, bool isFailSafe, uint8_t registerAddress);
extern STD_RETURN_TYPE_e TEST_FS85_WriteRegisterFsInit(
    FS85_STATE_s *pInstance,
    uint8_t registerAddress,
    uint16_t registerValue);
extern STD_RETURN_TYPE_e TEST_FS85_WriteBackRegisterFsInit(
    FS85_STATE_s *pInstance,
    uint8_t registerAddress,
    uint16_t registerValue);
extern STD_RETURN_TYPE_e TEST_FS85_ClearRegisterFlags(
    FS85_STATE_s *pInstance,
    uint8_t registerAddress,
    bool isFailSafe,
    uint16_t registerValue);
extern STD_RETURN_TYPE_e TEST_FS85_ReadBackAllRegisters(FS85_STATE_s *pInstance);
extern STD_RETURN_TYPE_e TEST_FS85_PerformPathCheckRstb(FS85_STATE_s *pInstance);
extern STD_RETURN_TYPE_e TEST_FS85_PerformPathCheckFs0b(FS85_STATE_s *pInstance);
extern STD_RETURN_TYPE_e TEST_FS85_GoToStandby(FS85_STATE_s *pInstance);
#endif

#endif /* FOXBMS__NXPFS85XX_H_ */
