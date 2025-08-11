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
 * @file    nxp_mc3377x_reg_def.h
 * @author  foxBMS Team
 * @date    2025-06-25 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  N77X
 *
 * @brief   Wrapper file for mapping to the MC3377X vendor headers
 * @details TODO
 */

#ifndef FOXBMS__NXP_MC3377X_REG_DEF_H_
#define FOXBMS__NXP_MC3377X_REG_DEF_H_

/*========== Includes =======================================================*/
#if defined(FOXBMS_AFE_DRIVER_NXP_MC33775A)
#include "MC33775A.h"
#endif

/*========== Macros and Definitions =========================================*/
#if defined(FOXBMS_AFE_DRIVER_NXP_MC33775A)

/* Balancing */
#define MC3377X_BAL_CH_CFG_OFFSET                   (MC33775_BAL_CH_CFG_OFFSET)
#define MC3377X_BAL_GLOB_TO_TMR_OFFSET              (MC33775_BAL_GLOB_TO_TMR_OFFSET)
#define MC3377X_BAL_PRE_TMR_OFFSET                  (MC33775_BAL_PRE_TMR_OFFSET)
#define MC3377X_BAL_TMR_CH_ALL_OFFSET               (MC33775_BAL_TMR_CH_ALL_OFFSET)
#define MC3377X_BAL_TMR_CH_ALL_PWM_PWM100_ENUM_VAL  (MC33775_BAL_TMR_CH_ALL_PWM_PWM100_ENUM_VAL)
#define MC3377X_BAL_TMR_CH_ALL_PWM_POS              (MC33775_BAL_TMR_CH_ALL_PWM_POS)
#define MC3377X_BAL_TMR_CH_ALL_BALTIME_POS          (MC33775_BAL_TMR_CH_ALL_BALTIME_POS)
#define MC3377X_BAL_GLOB_CFG_OFFSET                 (MC33775_BAL_GLOB_CFG_OFFSET)
#define MC3377X_BAL_GLOB_CFG_BALEN_ENABLED_ENUM_VAL (MC33775_BAL_GLOB_CFG_BALEN_ENABLED_ENUM_VAL)
#define MC3377X_BAL_GLOB_CFG_BALEN_POS              (MC33775_BAL_GLOB_CFG_BALEN_POS)
#define MC3377X_BAL_GLOB_CFG_TMRBALEN_STOP_ENUM_VAL (MC33775_BAL_GLOB_CFG_TMRBALEN_STOP_ENUM_VAL)
#define MC3377X_BAL_GLOB_CFG_TMRBALEN_POS           (MC33775_BAL_GLOB_CFG_TMRBALEN_POS)

/* I2C / Mux */
#define MC3377X_I2C_DATA0_OFFSET                (MC33775_I2C_DATA0_OFFSET)
#define MC3377X_I2C_DATA0_BYTE0_POS             (MC33775_I2C_DATA0_BYTE0_POS)
#define MC3377X_I2C_DATA0_BYTE1_POS             (MC33775_I2C_DATA0_BYTE1_POS)
#define MC3377X_I2C_DATA1_OFFSET                (MC33775_I2C_DATA1_OFFSET)
#define MC3377X_I2C_DATA1_BYTE2_POS             (MC33775_I2C_DATA1_BYTE2_POS)
#define MC3377X_I2C_DATA1_BYTE3_POS             (MC33775_I2C_DATA1_BYTE3_POS)
#define MC3377X_I2C_CTRL_OFFSET                 (MC33775_I2C_CTRL_OFFSET)
#define MC3377X_I2C_CTRL_START_POS              (MC33775_I2C_CTRL_START_POS)
#define MC3377X_I2C_CTRL_STPAFTER_STOP_ENUM_VAL (MC33775_I2C_CTRL_STPAFTER_STOP_ENUM_VAL)
#define MC3377X_I2C_CTRL_STPAFTER_POS           (MC33775_I2C_CTRL_STPAFTER_POS)
#define MC3377X_I2C_CTRL_RDAFTER_POS            (MC33775_I2C_CTRL_RDAFTER_POS)
#define MC3377X_I2C_STAT_OFFSET                 (MC33775_I2C_STAT_OFFSET)
#define MC3377X_I2C_STAT_PENDING_MSK            (MC33775_I2C_STAT_PENDING_MSK)
#define MC3377X_I2C_STAT_NACKRCV_MSK            (MC33775_I2C_STAT_NACKRCV_MSK)
#define MC3377X_I2C_CFG_OFFSET                  (MC33775_I2C_CFG_OFFSET)
#define MC3377X_I2C_CFG_EN_ENABLED_ENUM_VAL     (MC33775_I2C_CFG_EN_ENABLED_ENUM_VAL)
#define MC3377X_I2C_CFG_EN_POS                  (MC33775_I2C_CFG_EN_POS)
#define MC3377X_I2C_CFG_CLKSEL_F_400K_ENUM_VAL  (MC33775_I2C_CFG_CLKSEL_F_400K_ENUM_VAL)
#define MC3377X_I2C_CFG_CLKSEL_POS              (MC33775_I2C_CFG_CLKSEL_POS)

/* Enumerate */
#define MC3377X_SYS_VERSION_OFFSET                     (MC33775_SYS_VERSION_OFFSET)
#define MC3377X_SYS_MODE_OFFSET                        (MC33775_SYS_MODE_OFFSET)
#define MC3377X_SYS_MODE_TARGETMODE_DEEPSLEEP_ENUM_VAL (MC33775_SYS_MODE_TARGETMODE_DEEPSLEEP_ENUM_VAL)
#define MC3377X_SYS_MODE_TARGETMODE_POS                (MC33775_SYS_MODE_TARGETMODE_POS)
#define MC3377X_SYS_COM_CFG_OFFSET                     (MC33775_SYS_COM_CFG_OFFSET)
#define MC3377X_SYS_COM_CFG_BUSFW_ENABLED_ENUM_VAL     (MC33775_SYS_COM_CFG_BUSFW_ENABLED_ENUM_VAL)
#define MC3377X_SYS_COM_CFG_BUSFW_POS                  (MC33775_SYS_COM_CFG_BUSFW_POS)
#define MC3377X_SYS_COM_TO_CFG_OFFSET                  (MC33775_SYS_COM_TO_CFG_OFFSET)
#define MC3377X_SYS_COM_TO_CFG_COMTODISABLE_POS        (MC33775_SYS_COM_TO_CFG_COMTODISABLE_POS)
#define MC3377X_SYS_COM_TO_CFG_COMTO_POS               (MC33775_SYS_COM_TO_CFG_COMTO_POS)
#define MC3377X_SYS_UID_LOW_OFFSET                     (MC33775_SYS_UID_LOW_OFFSET)

/* Measurement */
#define MC3377X_ALLM_APP_CTRL_OFFSET                    (MC33775_ALLM_APP_CTRL_OFFSET)
#define MC3377X_ALLM_APP_CTRL_PAUSEBAL_POS              (MC33775_ALLM_APP_CTRL_PAUSEBAL_POS)
#define MC3377X_PRMM_APP_CTRL_CAPVC_POS                 (MC33775_PRMM_APP_CTRL_CAPVC_POS)
#define MC3377X_PRMM_APP_CTRL_VCOLNUM_DISABLED_ENUM_VAL (MC33775_PRMM_APP_CTRL_VCOLNUM_DISABLED_ENUM_VAL)
#define MC3377X_PRMM_APP_CTRL_VCOLNUM_POS               (MC33775_PRMM_APP_CTRL_VCOLNUM_POS)
#define MC3377X_PRMM_APP_VC_CNT_OFFSET                  (MC33775_PRMM_APP_VC_CNT_OFFSET)
#define MC3377X_SECM_APP_AIN4_OFFSET                    (MC33775_SECM_APP_AIN4_OFFSET)
#define MC3377X_SECM_PER_NPNISENSE_OFFSET               (MC33775_SECM_PER_NPNISENSE_OFFSET)
#define MC3377X_ALLM_VCVB_CFG_OFFSET                    (MC33775_ALLM_VCVB_CFG_OFFSET)
#define MC3377X_PRMM_AIN_CFG_OFFSET                     (MC33775_PRMM_AIN_CFG_OFFSET)
#define MC3377X_SECM_AIN_CFG_OFFSET                     (MC33775_SECM_AIN_CFG_OFFSET)
#define MC3377X_ALLM_CFG_OFFSET                         (MC33775_ALLM_CFG_OFFSET)
#define MC3377X_ALLM_CFG_BALPAUSELEN_POS                (MC33775_ALLM_CFG_BALPAUSELEN_POS)
#define MC3377X_ALLM_CFG_MEASEN_POS                     (MC33775_ALLM_CFG_MEASEN_POS)

#endif

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__NXP_MC3377X_REG_DEF_H_ */
