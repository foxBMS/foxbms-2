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
 * @file    nxpfs85xx.c
 * @author  foxBMS Team
 * @date    2020-03-18 (date of creation)
 * @updated 2021-07-14 (date of last update)
 * @ingroup DRIVERS
 * @prefix  SBC
 *
 * @brief   Driver for the SBC module
 *
 * @details It must always be used when creating new c source files.
 *
 */

/*========== Includes =======================================================*/
#include "nxpfs85xx.h"

#include "HL_gio.h"
#include "HL_system.h"

#include "diag.h"
#include "fram.h"
#include "fsystem.h"
#include "io.h"
#include "masterinfo.h"
#include "mcu.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/
typedef enum SBC_INIT_PHASE {
    SBC_UNINITIALIZED,
    SBC_FIN_TEST,
    SBC_RSTB_ASSERTION_TEST,
    SBC_INITIALIZED,
} SBC_INIT_PHASE_e;

/*========== Extern Constant and Variable Definitions =======================*/
FS85xx_STATE_s fs85xx_mcuSupervisor = {
    .pSpiInterface                  = &spi_kSbcMcuInterface,
    .configValues.watchdogSeed      = FS8x_WD_SEED_DEFAULT,
    .configValues.communicationMode = fs8xSPI,
    .configValues.i2cAddressOtp     = 0, /* Not used as SPI is selected */
    .fin.finUsed                    = true,
    .fin.finState                   = STD_NOT_OK,
    .fin.pGIOport = &(systemREG1->SYSPC4), /* FIN connected to ECLK1 (ball A12): PRIVILEGE MODE REQUIRED! */
    .fin.pin      = 0,
    .mainRegister = {0},
    .fsRegister   = {0},
    .nvram.entry  = FRAM_BLOCK_ID_SBC_INIT_STATE,
    .nvram.data   = &fram_sbcInit,
    .mode         = SBC_NORMAL_MODE, /* default value */
};

/*========== Static Function Prototypes =====================================*/
/**
 * @brief       Checks register value against expected value
 * @details     If actual register value equals expected value #STD_OK is
 *              returned if this is not the case this indicates an error and
 *              #STD_NOT_OK is returned.
 * @param[in]   registerValue           actual register value of interest
 * @param[in]   expectedRegisterValue   expected register value
 * @return      #STD_OK if register value equals expected, else #STD_NOT_OK
 */
static STD_RETURN_TYPE_e SBC_CheckRegisterValues(uint32_t registerValue, uint32_t expectedRegisterValue);

/**
 * @brief           Updates register values
 * @details         Updates register value of passed SBC instance with new
 *                  values
 * @param[in,out]   pInstance       SBC instance that is updated
 * @param[in]       isFailSafe      true if fail-safe register, false if main register
 * @param[in]       registerAddress address of register that is updated
 * @param[in]       registerValue   register value
 */
static void SBC_UpdateRegister(
    FS85xx_STATE_s *pInstance,
    bool isFailSafe,
    uint32_t registerAddress,
    uint32_t registerValue);

/**
 * @brief           Reads SBC register value
 * @details         Reads SBC register value from registerAddress and updates
 *                  register in SBC state variable if reading was successful
 * @param[in,out]   pInstance       SBC instance that is updated
 * @param[in]       isFailSafe      true if fail-safe register, false if main
 *                                  register
 * @param[in]       registerAddress address of register that is read from
 * @return          #STD_OK if reading was successful, otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e SBC_ReadBackRegister(FS85xx_STATE_s *pInstance, bool isFailSafe, uint8_t registerAddress);

/**
 * @brief       Write to fail-safe register
 * @details     Writes to fail-safe register (can be done during FS_INIT phase
 *              only)
 * @param[in]   pInstance       SBC instance that is updated
 * @param[in]   registerAddress address of register
 * @param[in]   registerValue   value that is written into register
 * @return      #STD_OK if writting was successful, other #STD_NOT_OK
 */
static STD_RETURN_TYPE_e SBC_WriteRegisterFsInit(
    FS85xx_STATE_s *pInstance,
    uint8_t registerAddress,
    uint16_t registerValue);

/**
 * @brief           Write to fail-safe register
 * @details         Writes to fail-safe register (can be done during FS_INIT
 *                  phase only), reads back if write process was successful and
 *                  afterwards updates register value of passed SBC instance
 * @param[in,out]   pInstance       SBC instance that is updated
 * @param[in]       registerAddress address of register
 * @param[in]       registerValue   value that is written into register
 * @return          #STD_OK if writting was successful, other #STD_NOT_OK
 */
static STD_RETURN_TYPE_e SBC_WriteBackRegisterFsInit(
    FS85xx_STATE_s *pInstance,
    uint8_t registerAddress,
    uint16_t registerValue);

/**
 * @brief           Clears flags in register
 * @details         Writes to register, reads back if clear process was
 *                  successful and afterwards updates register value of passed
 *                  SBC instance
 * @param[in,out]   pInstance  SBC instance that is updated
 * @param[in]       registerAddress address of register
 * @param[in]       isFailSafe      true if fail-safe register, false if main register
 * @param[in]       registerValue   value that is written into register
 * @return          #STD_OK if writting was successful, other #STD_NOT_OK
 */
static STD_RETURN_TYPE_e SBC_ClearRegisterFlags(
    FS85xx_STATE_s *pInstance,
    uint8_t registerAddress,
    bool isFailSafe,
    uint16_t registerValue);

static STD_RETURN_TYPE_e SBC_ReadBackAllRegisters(FS85xx_STATE_s *pInstance);

/**
 * @brief           Perform RSTB safety path check
 * @details         This functions performs a safety path check to make sure
 *                  external components connected to RSTB are available to
 *                  bring the safety critical outputs to known levels during
 *                  operation.
 * @param[in,out]   pInstance  SBC instance for which the RSTB path is checked
 * @return          #STD_OK if path check was successful, other #STD_NOT_OK
 */
static STD_RETURN_TYPE_e SBC_PerformPathCheckRSTB(FS85xx_STATE_s *pInstance);

/**
 * @brief           Perform FS0B safety path check
 * @details         This functions performs a safety path check to make sure
 *                  external components connected to FS0B are available to
 *                  bring the safety critical outputs to known levels during
 *                  operation.
 * @param[in,out]   pInstance   SBC instance for which the FS0B path is checked
 */
static STD_RETURN_TYPE_e SBC_PerformPathCheckFS0B(FS85xx_STATE_s *pInstance);

/*========== Static Function Implementations ================================*/
static STD_RETURN_TYPE_e SBC_CheckRegisterValues(uint32_t registerValue, uint32_t expectedRegisterValue) {
    STD_RETURN_TYPE_e retval = STD_OK;
    if (registerValue != expectedRegisterValue) {
        retval = STD_NOT_OK;
    }
    return retval;
}

static void SBC_UpdateRegister(
    FS85xx_STATE_s *pInstance,
    bool isFailSafe,
    uint32_t registerAddress,
    uint32_t registerValue) {
    FAS_ASSERT(pInstance != NULL_PTR);

    /* Check if fail-safe or main register needs to be updated */
    if (isFailSafe == true) {
        /* Update fail-safe register */
        switch (registerAddress) {
            case FS8X_FS_GRL_FLAGS_ADDR:
                pInstance->fsRegister.grl_flags = registerValue;
                break;

            case FS8X_FS_I_OVUV_SAFE_REACTION1_ADDR:
                pInstance->fsRegister.iOvervoltageUndervolateSafeReaction1 = registerValue;
                break;

            case FS8X_FS_I_OVUV_SAFE_REACTION2_ADDR:
                pInstance->fsRegister.iOvervoltageUndervolateSafeReaction2 = registerValue;
                break;

            case FS8X_FS_I_WD_CFG_ADDR:
                pInstance->fsRegister.iWatchdogConfiguration = registerValue;
                break;

            case FS8X_FS_I_SAFE_INPUTS_ADDR:
                pInstance->fsRegister.i_safe_inputs = registerValue;
                break;

            case FS8X_FS_I_FSSM_ADDR:
                pInstance->fsRegister.iFailSafeSateMachine = registerValue;
                break;

            case FS8X_FS_I_SVS_ADDR:
                pInstance->fsRegister.i_svs = registerValue;
                break;

            case FS8X_FS_WD_WINDOW_ADDR:
                pInstance->fsRegister.watchdogWindow = registerValue;
                break;

            case FS8X_FS_WD_SEED_ADDR:
                pInstance->fsRegister.watchdogSeed = registerValue;
                break;

            case FS8X_FS_WD_ANSWER_ADDR:
                pInstance->fsRegister.watchdogAnswer = registerValue;
                break;

            case FS8X_FS_OVUVREG_STATUS_ADDR:
                pInstance->fsRegister.overvoltageUndervoltageRegisterStatus = registerValue;
                break;

            case FS8X_FS_RELEASE_FS0B_ADDR:
                pInstance->fsRegister.releaseFs0bPin = registerValue;
                break;

            case FS8X_FS_SAFE_IOS_ADDR:
                pInstance->fsRegister.safeIos = registerValue;
                break;

            case FS8X_FS_DIAG_SAFETY_ADDR:
                pInstance->fsRegister.diag_safety = registerValue;
                break;

            case FS8X_FS_INTB_MASK_ADDR:
                pInstance->fsRegister.intb_mask = registerValue;
                break;

            case FS8X_FS_STATES_ADDR:
                pInstance->fsRegister.states = registerValue;
                break;

            default:
                FAS_ASSERT(FAS_TRAP); /* This case should never be reached */
                break;
        }
    } else {
        /* Update main register */
        switch (registerAddress) {
            case FS8X_M_FLAG_ADDR:
                pInstance->mainRegister.flag = registerValue;
                break;

            case FS8X_M_MODE_ADDR:
                pInstance->mainRegister.mode = registerValue;
                break;

            case FS8X_M_REG_CTRL1_ADDR:
                pInstance->mainRegister.registerControl1 = registerValue;
                break;

            case FS8X_M_REG_CTRL2_ADDR:
                pInstance->mainRegister.registerControl2 = registerValue;
                break;

            case FS8X_M_AMUX_ADDR:
                pInstance->mainRegister.analogMultiplexer = registerValue;
                break;

            case FS8X_M_CLOCK_ADDR:
                pInstance->mainRegister.clock = registerValue;
                break;

            case FS8X_M_INT_MASK1_ADDR:
                pInstance->mainRegister.int_mask1 = registerValue;
                break;

            case FS8X_M_INT_MASK2_ADDR:
                pInstance->mainRegister.int_mask2 = registerValue;
                break;

            case FS8X_M_FLAG1_ADDR:
                pInstance->mainRegister.flag1 = registerValue;
                break;

            case FS8X_M_FLAG2_ADDR:
                pInstance->mainRegister.flag2 = registerValue;
                break;

            case FS8X_M_VMON_REGX_ADDR:
                pInstance->mainRegister.vmon_regx = registerValue;
                break;

            case FS8X_M_LVB1_SVS_ADDR:
                pInstance->mainRegister.lvb1_svs = registerValue;
                break;

            case FS8X_M_MEMORY0_ADDR:
                pInstance->mainRegister.memory0 = registerValue;
                break;

            case FS8X_M_MEMORY1_ADDR:
                pInstance->mainRegister.memory1 = registerValue;
                break;

            case FS8X_M_DEVICEID_ADDR:
                pInstance->mainRegister.deviceId = registerValue;
                break;

            default:
                FAS_ASSERT(FAS_TRAP); /* This case should never be reached */
                break;
        }
    }
}

static STD_RETURN_TYPE_e SBC_ReadBackRegister(FS85xx_STATE_s *pInstance, bool isFailSafe, uint8_t registerAddress) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    fs8x_rx_frame_t rxTemp   = {0};

    FAS_ASSERT(pInstance != NULL_PTR);

    if (fs8xStatusOk !=
        FS8x_ReadRegister(pInstance->pSpiInterface, &(pInstance->configValues), isFailSafe, registerAddress, &rxTemp)) {
        retval = STD_NOT_OK;
    } else {
        SBC_UpdateRegister(pInstance, isFailSafe, registerAddress, rxTemp.readData);
        retval = STD_OK;
    }
    return retval;
}

static STD_RETURN_TYPE_e SBC_WriteRegisterFsInit(
    FS85xx_STATE_s *pInstance,
    uint8_t registerAddress,
    uint16_t registerValue) {
    STD_RETURN_TYPE_e retval = STD_OK;

    FAS_ASSERT(pInstance != NULL_PTR);

    /* Write to register */
    if (fs8xStatusOk !=
        FS8x_WriteRegisterInit(pInstance->pSpiInterface, &(pInstance->configValues), registerAddress, registerValue)) {
        retval = STD_NOT_OK;
    } else {
        retval = STD_OK;
    }
    return retval;
}

static STD_RETURN_TYPE_e SBC_WriteBackRegisterFsInit(
    FS85xx_STATE_s *pInstance,
    uint8_t registerAddress,
    uint16_t registerValue) {
    STD_RETURN_TYPE_e retval = STD_OK;
    fs8x_rx_frame_t rxTemp   = {0};

    FAS_ASSERT(pInstance != NULL_PTR);

    /* Write to register */
    if (fs8xStatusOk !=
        FS8x_WriteRegisterInit(pInstance->pSpiInterface, &(pInstance->configValues), registerAddress, registerValue)) {
        retval |= STD_NOT_OK;
    } else {
        /* Read back register value and check if write process was successful */
        if (fs8xStatusOk !=
            FS8x_ReadRegister(pInstance->pSpiInterface, &(pInstance->configValues), true, registerAddress, &rxTemp)) {
            retval |= STD_NOT_OK;
        } else {
            if (rxTemp.readData == registerValue) {
                /* Written register value equals read value */
                SBC_UpdateRegister(pInstance, true, registerAddress, rxTemp.readData);
            } else {
                retval |= STD_NOT_OK;
            }
        }
    }
    return retval;
}

static STD_RETURN_TYPE_e SBC_ClearRegisterFlags(
    FS85xx_STATE_s *pInstance,
    uint8_t registerAddress,
    bool isFailSafe,
    uint16_t registerValue) {
    STD_RETURN_TYPE_e retval = STD_OK;
    fs8x_rx_frame_t rxTemp   = {0};

    FAS_ASSERT(pInstance != NULL_PTR);

    /* Write to register */
    if (fs8xStatusOk !=
        FS8x_WriteRegister(
            pInstance->pSpiInterface, &(pInstance->configValues), isFailSafe, registerAddress, registerValue)) {
        retval |= STD_NOT_OK;
    } else {
        /* Read back register value and check if write process was successful */
        if (fs8xStatusOk !=
            FS8x_ReadRegister(
                pInstance->pSpiInterface, &(pInstance->configValues), isFailSafe, registerAddress, &rxTemp)) {
            retval |= STD_NOT_OK;
        } else {
            /* Check if all flags have been cleared: '1' is written to registers to clear respective flags */
            if ((rxTemp.readData & registerValue) == 0u) {
                /* Written register value equals read value */
                SBC_UpdateRegister(pInstance, isFailSafe, registerAddress, rxTemp.readData);
            } else {
                retval |= STD_NOT_OK;
            }
        }
    }
    return retval;
}

/*========== Extern Function Implementations ================================*/
extern STD_RETURN_TYPE_e FS85X_InitFS(FS85xx_STATE_s *pInstance) {
    STD_RETURN_TYPE_e retval  = STD_OK;
    uint16_t registerMask     = 0u;
    uint16_t expRegisterValue = 0u;
    uint16_t registerValue    = 0u;

    FAS_ASSERT(pInstance != NULL_PTR);
    /* Default init value: #STD_NOT_OK, will be set to #STD_OK later in init if
       no short-circuit between FIN and RSTB detected. Do not apply external
       clock on this pin until finState == #STD_OK */
    FAS_ASSERT(pInstance->fin.finState == STD_NOT_OK);

    /**  First: Verify following conditions:
    1. Verify LBIST (logical-built-in-self-test) and ABIST1 (analog-built-in-self-test1) are pass
    2. Verify Debug mode is not activated
    3. Verify there is no OTP CRC error
    4. Verify PGOOD was released: PGOOD is connected to power-on-reset of the MCU

    Second: Configure FS_I and FS_I_NOT registers

    - Write the desired data in the FS_I_Register_A (DATA)
    - Write the opposite in the FS_I_NOT_Register_A (DATA_NOT)
    - Only the utility bits must be inverted in the DATA_NOT content. The RESERVED bits are not considered and can be
      written at '0'. If the comparison result is correct, then the REG_CORRUPT is set to '0'. If the comparison result
      is wrong, then the REG_CORRUPT bit is set to '1'. The REG_CORRUPT monitoring is active as soon as the INIT_FS is
      closed by the first good watchdog refresh. INIT_FS must be closed by the first good watchdog refresh before 256ms
      timeout.

    1. Configure VCOREMON_OV_UV impact on RSTB and FS0B
    2. Configure VDDIO_OV_UV impact on RSTB and FS0B
    3. Configure VMONx_OV_UV impact on RSTB and FS0B
    4. Configure ABIST2 assignment
    5. Configure the WD window period, the WD window duty cycle, the WD counters limits,
    and its impact on RSTB and FS0B. Ensure that the configuration does not violate the
    FTTI requirement at system level.
    6. Configure the Fault Error Counter limit and its impact on RSTB and FS0B at
    intermediate value
    7. Configure the RSTB pulse duration
    8. Configure MCU FCCU error monitoring and its impact on RSTB and FS0B
    9. Configure Ext. IC error monitoring and its impact on RSTB and FS0B
    10.Configure FS0B short to high impact on RSTB

    Third: Execute

    1. Close INIT_FS by sending the first good WD refresh
    2. Execute ABIST2 and verify it is pass
    3. Clear all the flags by writing in FS_DIAG_SAFETY, FS_OVUVREG_STATUS
    4. Clear the fault error counter to 0 with consecutive good WD refresh
    5. Perform RSTB path check (repeat steps 1 to 4 after RSTB is released)
    6. Release FS0B pin
    7. Perform FS0B safety path check
    8. Refresh the WD according to its configuration
    9. Check FS_GRL_FLAGS register after each WD refresh

    The FS85 is now ready. If everything is OK for the MCU, it can release its own
    safety path and the ECU starts.
 */

    /* --------------------- First: Verify --------------------------------- */

    /** 1.: Verify LBIST and ABIST1 */
    if (STD_OK != SBC_ReadBackRegister(pInstance, true, FS8X_FS_DIAG_SAFETY_ADDR)) {
        retval = STD_NOT_OK;
    }
    registerMask     = (FS8X_FS_LBIST_OK_MASK | FS8X_FS_ABIST1_OK_MASK);
    expRegisterValue = (FS8X_FS_LBIST_OK_PASS | FS8X_FS_ABIST1_OK_PASS);
    if (STD_OK != SBC_CheckRegisterValues((pInstance->fsRegister.diag_safety & registerMask), expRegisterValue)) {
        retval = STD_NOT_OK;
    }

    /** Read FS STATES register */
    if (STD_OK != SBC_ReadBackRegister(pInstance, true, FS8X_FS_STATES_ADDR)) {
        retval = STD_NOT_OK;
    }

    /** 2.: Check if debug mode is active */
    registerMask     = FS8X_FS_DBG_MODE_MASK;
    expRegisterValue = FS8X_FS_DBG_MODE_NO_DEBUG;
    if (STD_OK != SBC_CheckRegisterValues((pInstance->fsRegister.states & registerMask), expRegisterValue)) {
        /* Debug mode active: this can only be the case if debugger connected */
        MINFO_SetDebugProbeConnectionState(MINFO_DEBUG_PROBE_CONNECTED);
        pInstance->mode = SBC_DEBUG_MODE;
    } else {
        /* Debug mode not active: no debugger connected */
        MINFO_SetDebugProbeConnectionState(MINFO_DEBUG_PROBE_NOT_CONNECTED);
        pInstance->mode = SBC_NORMAL_MODE;
    }

    /** 3.: Verify that no OPT CRC error */
    registerMask     = FS8X_FS_OTP_CORRUPT_MASK;
    expRegisterValue = FS8X_FS_OTP_CORRUPT_NO_ERROR;
    if (STD_OK != SBC_CheckRegisterValues((pInstance->fsRegister.states & registerMask), expRegisterValue)) {
        retval = STD_NOT_OK;
    }

    /** -------- Second: Configure fail-safe init registers ------------------*/
    /** Check if SBC is in FS_INIT state, if not switch SBC in FS_INIT state.
     * Specific configurations can only be done in FS_INIT state */
    registerMask     = FS8X_FS_FSM_STATE_MASK;
    expRegisterValue = FS8X_FS_FSM_STATE_INIT_FS;
    if (STD_OK != SBC_CheckRegisterValues((pInstance->fsRegister.states & registerMask), expRegisterValue)) {
        /* SBC not in FS_INIT state -> request FS_INIT state */
        /* Write FS_STATE request */
        registerValue = FS8X_FS_GO_TO_INITFS_GO_BACK_TO_INIT_FS;
        if (STD_OK != SBC_WriteRegisterFsInit(pInstance, FS8X_FS_SAFE_IOS_ADDR, registerValue)) {
            retval = STD_NOT_OK;
        } else {
            /* Check if SBC now in FS_STATE */
            if (STD_OK != SBC_ReadBackRegister(pInstance, true, FS8X_FS_STATES_ADDR)) {
                retval = STD_NOT_OK;
            } else {
                /* Read register now check if SBC in FS_INIT state */
                if (STD_OK !=
                    SBC_CheckRegisterValues((pInstance->fsRegister.states & registerMask), expRegisterValue)) {
                    /* SBC not in FS_INIT state -> error */
                    retval = STD_NOT_OK;
                }
            }
        }
    }

    /** 1.: Configure VCOREMON_OV_UV impact on RSTB and FS0B */
    /** 2.: Configure VDDIO_OV_UV impact on RSTB and FS0B */
    /** 4.: Configure ABIST2 assignment */
    registerValue = 0;

    /** Select VCOREMON_OV options:
     *  - FS8X_FS_I_VCOREMON_OV_FS_IMPACT_NO_EFFECT
     *  - FS8X_FS_I_VCOREMON_OV_FS_IMPACT_FS0B
     *  - FS8X_FS_I_VCOREMON_OV_FS_IMPACT_FS0B_RSTB */
    registerValue |= FS8X_FS_I_VCOREMON_OV_FS_IMPACT_NO_EFFECT;

    /** Select VCOREMON_UV options:
     *  - FS8X_FS_I_VCOREMON_UV_FS_IMPACT_NO_EFFECT
     *  - FS8X_FS_I_VCOREMON_UV_FS_IMPACT_FS0B
     *  - FS8X_FS_I_VCOREMON_UV_FS_IMPACT_FS0B_RSTB */
    registerValue |= FS8X_FS_I_VCOREMON_UV_FS_IMPACT_NO_EFFECT;

    /** Select VDDIO_OV options:
     *  - FS8X_FS_I_VDDIO_OV_FS_IMPACT_NO_EFFECT
     *  - FS8X_FS_I_VDDIO_OV_FS_IMPACT_FS0B
     *  - FS8X_FS_I_VDDIO_OV_FS_IMPACT_FS0B_RSTB */
    registerValue |= FS8X_FS_I_VDDIO_OV_FS_IMPACT_NO_EFFECT;

    /** Select VDDIO_UV options:
     *  - FS8X_FS_I_VDDIO_UV_FS_IMPACT_NO_EFFECT
     *  - FS8X_FS_I_VDDIO_UV_FS_IMPACT_FS0B
     *  - FS8X_FS_I_VDDIO_UV_FS_IMPACT_FS0B_RSTB */
    registerValue |= FS8X_FS_I_VDDIO_UV_FS_IMPACT_NO_EFFECT;

    /** Select ABIST2 options:
     *  + VCOREMON
     *    - FS8X_FS_I_VCOREMON_ABIST2_NO_ABIST
     *    - FS8X_FS_I_VCOREMON_ABIST2_VCOREMON_BIST
     *  + VDDIO
     *    - FS8X_FS_I_VDDIO_ABIST2_NO_ABIST
     *    - FS8X_FS_I_VDDIO_ABIST2_VDDIO_BIST
     *  + VMONx (VMON1 - VMON4)
     *    - FS8X_FS_I_VMONx_ABIST2_NO_ABIST
     *    - FS8X_FS_I_VMONx_ABIST2_VMONx_BIST */
    registerValue |= FS8X_FS_I_VCOREMON_ABIST2_VCOREMON_BIST;
    registerValue |= FS8X_FS_I_VDDIO_ABIST2_VDDIO_BIST;
    registerValue |= FS8X_FS_I_VMON1_ABIST2_VMON1_BIST;
    registerValue |= FS8X_FS_I_VMON2_ABIST2_VMON2_BIST;
    registerValue |= FS8X_FS_I_VMON3_ABIST2_VMON3_BIST;
    registerValue |= FS8X_FS_I_VMON4_ABIST2_VMON4_BIST;

    /* Write register */
    if (STD_OK != SBC_WriteBackRegisterFsInit(pInstance, FS8X_FS_I_OVUV_SAFE_REACTION1_ADDR, registerValue)) {
        retval = STD_NOT_OK;
    }

    /** 3.: Configure VMONx_OV_UV impact on RSTB and FS0B */
    registerValue = 0;
    /** Select VMONx_OV options:
     *  - FS8X_FS_I_VMONx_OV_FS_IMPACT_NO_EFFECT
     *  - FS8X_FS_I_VMONx_OV_FS_IMPACT_FS0B
     *  - FS8X_FS_I_VMONx_OV_FS_IMPACT_FS0B_RSTB */
    registerValue |= FS8X_FS_I_VMON1_OV_FS_IMPACT_NO_EFFECT;
    registerValue |= FS8X_FS_I_VMON2_OV_FS_IMPACT_NO_EFFECT;
    registerValue |= FS8X_FS_I_VMON3_OV_FS_IMPACT_NO_EFFECT;
    registerValue |= FS8X_FS_I_VMON4_OV_FS_IMPACT_NO_EFFECT;

    /** Select VMONx_UV options:
     *  - FS8X_FS_I_VMONx_UV_FS_IMPACT_NO_EFFECT
     *  - FS8X_FS_I_VMONx_UV_FS_IMPACT_FS0B
     *  - FS8X_FS_I_VMONx_UV_FS_IMPACT_FS0B_RSTB */
    registerValue |= FS8X_FS_I_VMON1_UV_FS_IMPACT_NO_EFFECT;
    registerValue |= FS8X_FS_I_VMON2_UV_FS_IMPACT_NO_EFFECT;
    registerValue |= FS8X_FS_I_VMON3_UV_FS_IMPACT_NO_EFFECT;
    registerValue |= FS8X_FS_I_VMON4_UV_FS_IMPACT_NO_EFFECT;

    /* Write register */
    if (STD_OK != SBC_WriteBackRegisterFsInit(pInstance, FS8X_FS_I_OVUV_SAFE_REACTION2_ADDR, registerValue)) {
        retval = STD_NOT_OK;
    }

    /** 5.: Configure the WD window period, the WD window duty cycle, the WD counters limits, and its impact on RSTB and
       FS0B. Ensure that the configuration does not violate the FTTI requirement at system level. */
    registerValue = 0;

    /** WD window period options:
     *  - FS8X_FS_WD_WINDOW_DISABLE
     *  - FS8X_FS_WD_WINDOW_xxxxMS */
    registerValue |= FS8X_FS_WD_WINDOW_128MS;

    /** WD window duty cycle options:
     *  - FS8X_FS_WDW_DC_31_25
     *  - FS8X_FS_WDW_DC_37_5
     *  - FS8X_FS_WDW_DC_50
     *  - FS8X_FS_WDW_DC_62_5
     *  - FS8X_FS_WDW_DC_68_75 */
    registerValue |= FS8X_FS_WDW_DC_50;

    /** WD fault recovery strategy
     *  - FS8X_FS_WDW_RECOVERY_DISABLE
     *  - FS8X_FS_WDW_RECOVERY_xxxxMS */
    registerValue |= FS8X_FS_WDW_RECOVERY_128MS;

    /* Write register */
    if (STD_OK != SBC_WriteBackRegisterFsInit(pInstance, FS8X_FS_WD_WINDOW_ADDR, registerValue)) {
        retval = STD_NOT_OK;
    }

    /** 6.: Configure the Fault Error Counter limit and its impact on RSTB and FS0B at intermediate value */
    /** Configure the RSTB pulse duration */
    /** Configure FS0B short to high impact on RSTB */
    registerValue = 0;

    /**  Fault Error Counter limit options:
     *  - FS8X_FS_I_FLT_ERR_CNT_LIMIT_2
     *  - FS8X_FS_I_FLT_ERR_CNT_LIMIT_4
     *  - FS8X_FS_I_FLT_ERR_CNT_LIMIT_6
     *  - FS8X_FS_I_FLT_ERR_CNT_LIMIT_8 */
    registerValue |= FS8X_FS_I_FLT_ERR_CNT_LIMIT_8;

    /** Fault Error Counter impact options:
     *  - FS8X_FS_I_FLT_ERR_IMPACT_NO_EFFECT
     *  - FS8X_FS_I_FLT_ERR_IMPACT_FS0B
     *  - FS8X_FS_I_FLT_ERR_IMPACT_FS0B_RSTB */
    registerValue |= FS8X_FS_I_FLT_ERR_IMPACT_FS0B_RSTB;

    /** 7.: RSTB pulse duration options:
     *  - FS8X_FS_I_RSTB_DUR_1MS
     *  - FS8X_FS_I_RSTB_DUR_10MS */
    registerValue |= FS8X_FS_I_RSTB_DUR_10MS;

    /** 10.: FS0B short to high impact on RSTB options:
     *  - FS8X_FS_I_FS0B_SC_HIGH_CFG_NO_ASSERTION
     *  - FS8X_FS_I_FS0B_SC_HIGH_CFG_RESET_ASSERTED */
    registerValue |= FS8X_FS_I_FS0B_SC_HIGH_CFG_NO_ASSERTION;

    /** After POR fault-error counter is set to 1 on default, it is reset after two consecutive good WD refreshes. This
       part of the register is read-only so a write access has no influence. Set this bit for a successful comparison
       between written and read register value */
    registerValue |= FS8X_FS_I_FLT_ERR_CNT_1;

    /* Write register */
    if (STD_OK != SBC_WriteRegisterFsInit(pInstance, FS8X_FS_I_FSSM_ADDR, registerValue)) {
        retval = STD_NOT_OK;
    }

    /** 8.: Configure MCU FCCU error monitoring and its impact on RSTB and FS0B */
    /** 9.: Configure Ext. IC error monitoring and its impact on RSTB and FS0B */
    registerValue = 0;

    /** MCU FCCU error monitoring options:
     *  + Input option:
     *    - FS8X_FS_I_FCCU_CFG_NO_MONITORING
     *    - FS8X_FS_I_FCCU_CFG_FCCU1_INPUT
     *    - FS8X_FS_I_FCCU_CFG_FCCU1_FCCU2_INPUT
     *    - FS8X_FS_I_FCCU_CFG_FCCU1_FCCU2_PAIR (bi-stable protocol)
     *  + Polarity option (independent):
     *    - FS8X_FS_I_FCCUx_FLT_POL_FCCUx_L
     *    - FS8X_FS_I_FCCUx_FLT_POL_FCCUx_H
     *  + Polarity option (bi-stable)
     *    - FS8X_FS_I_FCCU12_FLT_POL_FCCU1_L_FCCU2_H
     *    - FS8X_FS_I_FCCU12_FLT_POL_FCCU1_H_FCCU2_L
     *  + Impact option (independent)
     *    - FS8X_FS_I_FCCUx_FS_REACT_FS0B
     *    - FS8X_FS_I_FCCUx_FS_REACT_FS0B_RSTB
     *  + Impact option (bi-stable)
     *    - FS8X_FS_I_FCCU12_FS_IMPACT_FS0B
     *    - FS8X_FS_I_FCCU12_FS_IMPACT_FS0B_RSTB
     */
    registerValue |= FS8X_FS_I_FCCU_CFG_NO_MONITORING;
    registerValue |= FS8X_FS_I_FCCU1_FLT_POL_FCCU1_L;
    registerValue |= FS8X_FS_I_FCCU1_FS_REACT_FS0B;

    /** Ext. IC error monitoring options:
     *  + Polarity options:
     *    - FS8X_FS_I_ERRMON_FLT_POLARITY_NEGATIVE_EDGE
     *    - FS8X_FS_I_ERRMON_FLT_POLARITY_POSITIVE_EDGE
     *  + Error acknowledgment time options:
     *    - FS8X_FS_I_ERRMON_ACK_TIME_1MS
     *    - FS8X_FS_I_ERRMON_ACK_TIME_8MS
     *    - FS8X_FS_I_ERRMON_ACK_TIME_16MS
     *    - FS8X_FS_I_ERRMON_ACK_TIME_32MS
     *  + Error monitoring impact options:
     *    - FS8X_FS_I_ERRMON_FS_IMPACT_FS0B
     *    - FS8X_FS_I_ERRMON_FS_IMPACT_FS0B_RSTB */
    registerValue |= FS8X_FS_I_ERRMON_FLT_POLARITY_POSITIVE_EDGE;
    registerValue |= FS8X_FS_I_ERRMON_ACK_TIME_32MS;
    registerValue |= FS8X_FS_I_ERRMON_FS_IMPACT_FS0B;

    /* Write register */
    if (STD_OK != SBC_WriteRegisterFsInit(pInstance, FS8X_FS_I_SAFE_INPUTS_ADDR, registerValue)) {
        retval = STD_NOT_OK;
    }

    /* -------------------- Third: Execute --------------------------------- */

    /** 1.: Close INIT_FS by sending the first good WD refresh */
    if (STD_OK != SBC_TriggerWatchdog(pInstance)) {
        retval = STD_NOT_OK;
    }
    /** 2.: Execute ABIST2 and verify it is pass */
    /** ABIST2 is executed automatically after closing of INIT_FS, duration: 1.2ms max */
    MCU_delay_us(1200u);

    if (STD_OK != SBC_ReadBackRegister(pInstance, true, FS8X_FS_DIAG_SAFETY_ADDR)) {
        retval = STD_NOT_OK;
    }
    registerMask     = FS8X_FS_ABIST2_OK_MASK;
    expRegisterValue = FS8X_FS_ABIST2_OK_MASK;
    if (STD_OK != SBC_CheckRegisterValues((pInstance->fsRegister.diag_safety & registerMask), expRegisterValue)) {
        retval = STD_NOT_OK;
    }

    /** 3.: Clear all the flags by writing in FS_DIAG_SAFETY */
    registerValue = 0;
    /** Flags are cleared by writting '1' to register */
    registerValue |= (FS8X_FS_FCCU12_ERROR_DETECTED | FS8X_FS_FCCU1_ERROR_DETECTED | FS8X_FS_FCCU2_ERROR_DETECTED);
    registerValue |= FS8X_FS_ERRMON_ERROR_DETECTED;
    registerValue |= (FS8X_FS_BAD_WD_DATA_BAD_WD_REFRESH | FS8X_FS_BAD_WD_TIMING_BAD_WD_REFRESH);
    registerValue |=
        (FS8X_FS_SPI_FS_CLK_WRONG_NUMBER_OF_CLOCK_CYCLES | FS8X_FS_SPI_FS_REQ_SPI_VIOLATION |
         FS8X_FS_SPI_FS_CRC_ERROR_DETECTED);
    registerValue |= (FS8X_FS_I2C_FS_CRC_ERROR_DETECTED | FS8X_FS_I2C_FS_REQ_I2C_VIOLATION);
    if (STD_OK != SBC_ClearRegisterFlags(pInstance, FS8X_FS_DIAG_SAFETY_ADDR, true, registerValue)) {
        retval = STD_NOT_OK;
    }

    /** Clear all the flags by writing in FS_OVUVREG_STATUS */
    registerValue = 0;
    /** Flags are cleared by writting '1' to register */
    registerValue |= (FS8X_FS_VCOREMON_OV_OVERVOLTAGE_REPORTED | FS8X_FS_VCOREMON_UV_UNDERVOLTAGE_REPORTED);
    registerValue |= (FS8X_FS_VDDIO_OV_OVERVOLTAGE_REPORTED | FS8X_FS_VDDIO_UV_UNDERVOLTAGE_REPORTED);
    registerValue |=
        (FS8X_FS_VMON4_OV_OVERVOLTAGE_REPORTED | FS8X_FS_VMON4_UV_UNDERVOLTAGE_REPORTED |
         FS8X_FS_VMON3_OV_OVERVOLTAGE_REPORTED | FS8X_FS_VMON3_UV_UNDERVOLTAGE_REPORTED |
         FS8X_FS_VMON2_OV_OVERVOLTAGE_REPORTED | FS8X_FS_VMON2_UV_UNDERVOLTAGE_REPORTED |
         FS8X_FS_VMON1_OV_OVERVOLTAGE_REPORTED | FS8X_FS_VMON1_UV_UNDERVOLTAGE_REPORTED);
    registerValue |= FS8X_FS_FS_DIG_REF_OV_OVERVOLTAGE_REPORTED;
    registerValue |= FS8X_FS_FS_OSC_DRIFT_OSCILLATOR_DRIFT;
    if (STD_OK != SBC_ClearRegisterFlags(pInstance, FS8X_FS_OVUVREG_STATUS_ADDR, true, registerValue)) {
        retval = STD_NOT_OK;
    }

    /** Clear flags FLAG1 register */
    registerValue = FS8X_M_VBOSUVH_EVENT_OCCURRED;
    registerValue |= FS8X_M_VBOOSTUVH_EVENT_OCCURRED;
    if (STD_OK != SBC_ClearRegisterFlags(pInstance, FS8X_M_FLAG1_ADDR, false, registerValue)) {
        retval = STD_NOT_OK;
    }

    /** Clear flags FLAG2 register */
    registerValue = FS8X_M_VSUPUV7_EVENT_OCCURRED;
    registerValue |=
        (FS8X_M_VPREUVL_EVENT_OCCURRED | FS8X_M_VPREUVH_EVENT_OCCURRED | FS8X_M_VSUPUVL_EVENT_OCCURRED |
         FS8X_M_VSUPUVH_EVENT_OCCURRED | FS8X_M_WK2FLG_EVENT_OCCURRED);
    if (STD_OK != SBC_ClearRegisterFlags(pInstance, FS8X_M_FLAG2_ADDR, false, registerValue)) {
        retval = STD_NOT_OK;
    }

    /** Read out all registers for debug purpose */
    if (STD_OK != SBC_ReadBackAllRegisters(pInstance)) {
        retval = STD_OK;
    }

    return retval;
}

extern STD_RETURN_TYPE_e FS85X_Init_ReqWDGRefreshes(FS85xx_STATE_s *pInstance, uint8_t *requiredWatchdogRefreshes) {
    STD_RETURN_TYPE_e retval = STD_OK;

    FAS_ASSERT(pInstance != NULL_PTR);

    /** Clear the fault error counter to 0 with consecutive good WD refreshes.
     * The watchdog refresh counter is used to decrement the fault error counter. Each time the watchdog is properly
     * refreshed, the watchdog refresh counter is incremented by '1'. Each time the watchdog refresh counter reaches
     * its maximum value ('6' by default) and if next WD refresh is also good, the fault error counter is decremented
     * by '1'. Whatever the position the watchdog refresh counter is in, each time there is a wrong refresh watchdog,
     * the watchdog refresh counter is reset to '0'. */

    /** Read out FS_I_WD_CFG register to get watchdog refresh counter limit and value */
    if (STD_OK != SBC_ReadBackRegister(pInstance, true, FS8X_FS_I_WD_CFG_ADDR)) {
        retval = STD_NOT_OK;
    }

    /** Get refresh counter value */
    uint8_t watchdogRefreshLimit   = 0;
    uint8_t watchdogRefreshCounter = FS8x_BO_GET_REG_VALUE(
        pInstance->fsRegister.iWatchdogConfiguration, FS8X_FS_I_WD_RFR_CNT_MASK, FS8X_FS_I_WD_RFR_CNT_SHIFT);

    /** Get refresh counter limit register value */
    if (FS8X_FS_I_WD_RFR_LIMIT_6 ==
        FS8x_BO_GET_REG_VALUE(
            pInstance->fsRegister.iWatchdogConfiguration, FS8X_FS_I_WD_RFR_LIMIT_MASK, FS8X_FS_I_WD_RFR_LIMIT_SHIFT)) {
        watchdogRefreshLimit = 6u;
    } else if (
        FS8X_FS_I_WD_RFR_LIMIT_4 ==
        FS8x_BO_GET_REG_VALUE(
            pInstance->fsRegister.iWatchdogConfiguration, FS8X_FS_I_WD_RFR_LIMIT_MASK, FS8X_FS_I_WD_RFR_LIMIT_SHIFT)) {
        watchdogRefreshLimit = 4u;
    } else if (
        FS8X_FS_I_WD_RFR_LIMIT_2 ==
        FS8x_BO_GET_REG_VALUE(
            pInstance->fsRegister.iWatchdogConfiguration, FS8X_FS_I_WD_RFR_LIMIT_MASK, FS8X_FS_I_WD_RFR_LIMIT_SHIFT)) {
        watchdogRefreshLimit = 2u;
    } else {
        watchdogRefreshLimit = 1u;
    }

    /** Get fault error counter */
    uint8_t faultCounter = 0u;
    if (fs8xStatusOk !=
        FS8x_GetFaultErrorCounterValue(pInstance->pSpiInterface, &(pInstance->configValues), &faultCounter)) {
        retval = STD_NOT_OK;
    }

    /** Get number of required watchdog refreshes to clear fault error counter to 0 */
    *requiredWatchdogRefreshes = faultCounter + (watchdogRefreshLimit - watchdogRefreshCounter);

    return retval;
}

extern STD_RETURN_TYPE_e FS85X_CheckFaultErrorCounter(FS85xx_STATE_s *pInstance) {
    STD_RETURN_TYPE_e retval = STD_OK;

    /* Check fault error counter */
    uint8_t faultCounter = 0u;
    retval |= FS8x_GetFaultErrorCounterValue(pInstance->pSpiInterface, &(pInstance->configValues), &faultCounter);
    if (faultCounter != 0u) {
        retval = STD_NOT_OK;
    }
    return retval;
}

extern STD_RETURN_TYPE_e FS85X_SafetyPathChecks(FS85xx_STATE_s *pInstance) {
    STD_RETURN_TYPE_e retval = STD_OK;

    FAS_ASSERT(pInstance != NULL_PTR);

    /* Perform RSTB path check (repeat steps 1 to 4 after RSTB is released) */
    if (STD_OK != SBC_PerformPathCheckRSTB(pInstance)) {
        retval = STD_NOT_OK;
    }

    /* FS0B pin can not be released in debug mode */
    if (pInstance->mode != SBC_DEBUG_MODE) {
        /* Release FS0B pin */
        if (fs8xStatusOk != FS8x_FS0B_Release(pInstance->pSpiInterface, &(pInstance->configValues))) {
            retval = STD_NOT_OK;
        }
    }

    /* Perform FS0B safety path check */
    if (STD_OK != SBC_PerformPathCheckFS0B(pInstance)) {
        retval = STD_NOT_OK;
    }

    /* Init finished successfully if retval still okay */
    if (retval == STD_OK) {
        pInstance->nvram.data->phase = (uint8_t)SBC_INITIALIZED;
        FRAM_Write(pInstance->nvram.entry);
    }
    return retval;
}

extern UNIT_TEST_WEAK_IMPL fs8x_status_t MCU_SPI_TransferData(
    SPI_INTERFACE_CONFIG_s *pSpiInterface,
    uint8_t *txFrame,
    uint16_t frameLengthBytes,
    uint8_t *rxFrame) {
    uint16_t sbc_txBuffer[FS8x_COMM_FRAME_SIZE];
    uint16_t sbc_rxBuffer[FS8x_COMM_FRAME_SIZE];
    fs8x_status_t spiCommunicationState = fs8xStatusError;

    FAS_ASSERT(txFrame != NULL_PTR);
    FAS_ASSERT(rxFrame != NULL_PTR);

    /* Copy TX data in TX array, reset RX array */
    for (uint8_t i = 0u; i < FS8x_COMM_FRAME_SIZE; i++) {
        sbc_txBuffer[FS8x_COMM_FRAME_SIZE - 1u - i] = (uint16_t)txFrame[i]; /* txFrame[0] will be sent last */
        sbc_rxBuffer[i]                             = 0;
    }

    STD_RETURN_TYPE_e spiRetval =
        SPI_TransmitReceiveData(pSpiInterface, sbc_txBuffer, sbc_rxBuffer, FS8x_COMM_FRAME_SIZE);

    if (spiRetval == STD_OK) {
        /* No error flag set - copy RX data in RX array */
        for (uint8_t i = 0u; i < FS8x_COMM_FRAME_SIZE; i++) {
            rxFrame[i] = (uint8_t)sbc_rxBuffer[FS8x_COMM_FRAME_SIZE - 1u - i];
        }
        spiCommunicationState = fs8xStatusOk;
    }

    return spiCommunicationState;
}

extern STD_RETURN_TYPE_e SBC_TriggerWatchdog(FS85xx_STATE_s *pInstance) {
    STD_RETURN_TYPE_e retval = STD_NOT_OK;
    if (fs8xStatusOk == FS8x_WD_Refresh(pInstance->pSpiInterface, &(pInstance->configValues))) {
        /* Check GRL_FLAGS register if watchdog refresh was valid */
        if (STD_OK == SBC_ReadBackRegister(pInstance, true, FS8X_FS_GRL_FLAGS_ADDR)) {
            uint16_t registerMask     = FS8X_FS_FS_WD_G_MASK;
            uint16_t expRegisterValue = FS8X_FS_FS_WD_G_GOOD_WD_REFRESH;
            if (STD_OK == SBC_CheckRegisterValues((pInstance->fsRegister.grl_flags & registerMask), expRegisterValue)) {
                /* GOOD watchdog refresh */
                retval = STD_OK;
            } else {
                /* BAD watchdog refresh: get reason */
                if (STD_OK == SBC_ReadBackRegister(pInstance, true, FS8X_FS_DIAG_SAFETY_ADDR)) {
                    registerMask     = FS8X_FS_BAD_WD_TIMING_MASK;
                    expRegisterValue = FS8X_FS_BAD_WD_TIMING_BAD_WD_REFRESH;
                    if (STD_OK ==
                        SBC_CheckRegisterValues((pInstance->fsRegister.diag_safety & registerMask), expRegisterValue)) {
                        /* TODO: Notify someone: bad watchdog refresh was caused by wrong window or in timeout */
                    }
                    registerMask     = FS8X_FS_BAD_WD_DATA_MASK;
                    expRegisterValue = FS8X_FS_BAD_WD_DATA_BAD_WD_REFRESH;
                    if (STD_OK ==
                        SBC_CheckRegisterValues((pInstance->fsRegister.diag_safety & registerMask), expRegisterValue)) {
                        /* TODO: Notify someone: bad watchdog refresh was caused by error in data */
                    }
                }
            }
        }
    }
    return retval;
}

static STD_RETURN_TYPE_e SBC_ReadBackAllRegisters(FS85xx_STATE_s *pInstance) {
    STD_RETURN_TYPE_e retval = STD_OK;

    /* Read all fail-safe registers */
    SBC_ReadBackRegister(pInstance, true, FS8X_FS_GRL_FLAGS_ADDR);
    SBC_ReadBackRegister(pInstance, true, FS8X_FS_I_OVUV_SAFE_REACTION1_ADDR);
    SBC_ReadBackRegister(pInstance, true, FS8X_FS_I_OVUV_SAFE_REACTION2_ADDR);
    SBC_ReadBackRegister(pInstance, true, FS8X_FS_I_WD_CFG_ADDR);
    SBC_ReadBackRegister(pInstance, true, FS8X_FS_I_SAFE_INPUTS_ADDR);
    SBC_ReadBackRegister(pInstance, true, FS8X_FS_I_FSSM_ADDR);
    SBC_ReadBackRegister(pInstance, true, FS8X_FS_I_SVS_ADDR);
    SBC_ReadBackRegister(pInstance, true, FS8X_FS_WD_WINDOW_ADDR);
    SBC_ReadBackRegister(pInstance, true, FS8X_FS_WD_SEED_ADDR);
    SBC_ReadBackRegister(pInstance, true, FS8X_FS_WD_ANSWER_ADDR);
    SBC_ReadBackRegister(pInstance, true, FS8X_FS_OVUVREG_STATUS_ADDR);
    SBC_ReadBackRegister(pInstance, true, FS8X_FS_RELEASE_FS0B_ADDR);
    SBC_ReadBackRegister(pInstance, true, FS8X_FS_SAFE_IOS_ADDR);
    SBC_ReadBackRegister(pInstance, true, FS8X_FS_DIAG_SAFETY_ADDR);
    SBC_ReadBackRegister(pInstance, true, FS8X_FS_INTB_MASK_ADDR);
    SBC_ReadBackRegister(pInstance, true, FS8X_FS_STATES_ADDR);

    /* Read main registers */
    SBC_ReadBackRegister(pInstance, FALSE, FS8X_M_FLAG_ADDR);
    SBC_ReadBackRegister(pInstance, FALSE, FS8X_M_MODE_ADDR);
    SBC_ReadBackRegister(pInstance, FALSE, FS8X_M_REG_CTRL1_ADDR);
    SBC_ReadBackRegister(pInstance, FALSE, FS8X_M_REG_CTRL2_ADDR);
    SBC_ReadBackRegister(pInstance, FALSE, FS8X_M_AMUX_ADDR);
    SBC_ReadBackRegister(pInstance, FALSE, FS8X_M_CLOCK_ADDR);
    SBC_ReadBackRegister(pInstance, FALSE, FS8X_M_INT_MASK1_ADDR);
    SBC_ReadBackRegister(pInstance, FALSE, FS8X_M_INT_MASK2_ADDR);
    SBC_ReadBackRegister(pInstance, FALSE, FS8X_M_FLAG1_ADDR);
    SBC_ReadBackRegister(pInstance, FALSE, FS8X_M_FLAG2_ADDR);
    SBC_ReadBackRegister(pInstance, FALSE, FS8X_M_VMON_REGX_ADDR);
    SBC_ReadBackRegister(pInstance, FALSE, FS8X_M_LVB1_SVS_ADDR);
    SBC_ReadBackRegister(pInstance, FALSE, FS8X_M_MEMORY0_ADDR);
    SBC_ReadBackRegister(pInstance, FALSE, FS8X_M_MEMORY1_ADDR);
    SBC_ReadBackRegister(pInstance, FALSE, FS8X_M_DEVICEID_ADDR);
    return retval;
}

static STD_RETURN_TYPE_e SBC_PerformPathCheckRSTB(FS85xx_STATE_s *pInstance) {
    STD_RETURN_TYPE_e retval = STD_OK;
    resetSource_t rstReason  = MINFO_GetResetSource();
    bool test_assertionRSTB  = false; /* Do not test RSTB assertion on default */

    /** Assertion of RSTB pin will trigger reset, check if reset reason was power-cycle.
       If so, check if short circuit between FIN and RSTB pin exists */
    if (rstReason == POWERON_RESET) {
        /** First check if FIN is used */
        if (pInstance->fin.finUsed == true) {
            /** Write to NVRAM to determine after reset and if short-circuit
             * between RSTB and FIN present what exactly caused the reset. */
            pInstance->nvram.data->phase = (uint8_t)SBC_FIN_TEST;
            FRAM_Write(pInstance->nvram.entry);

            /** MCU SBC is connected to ECLK1 -> privilege mode is required to access register */
            FSYS_RaisePrivilege();

            /** Last reset was caused by power-cycle */
            /** Set level of FIN pin low and check if this generates reset */
            IO_PinReset(pInstance->fin.pGIOport, pInstance->fin.pin);

            /** Pulses longer than 2000ns trigger reset -> wait 10us to check if
               reset is triggered by short between RSTB and FIN */
            MCU_delay_us(10u);

            /** If we reach this line of code, no reset has taken place.
               Everything okay. Set level of FIN pin back to high */
            IO_PinSet(pInstance->fin.pGIOport, pInstance->fin.pin);

            /** No further register access required -> leave privilege mode */
            FSYS_SwitchToUserMode();

            /** FIN state okay, no short circuit. Update also in nvram struct */
            DIAG_Handler(DIAG_ID_SBC_FIN_STATE, DIAG_EVENT_OK, DIAG_SYSTEM, 0);
            pInstance->fin.finState         = STD_OK;
            pInstance->nvram.data->finState = STD_OK;

            /** Continue with RSTB assertion test */
            test_assertionRSTB = true;
        } else {
            /** Power-cycle but no FIN pin used -> continue with RSTB check */
            test_assertionRSTB = true;
        }
    } else if (rstReason == EXT_RESET) {
        /** Last reset reason was external reset via nRST pin (EXT_RESET)
           Readout FRAM to determine in which state the SBC was prior to reset */
        FRAM_Read(pInstance->nvram.entry);
        if ((SBC_INIT_PHASE_e)pInstance->nvram.data->phase == SBC_FIN_TEST) {
            /** Short-circuit between FIN and RSTB: Do not apply CLK on FIN */
            /** Update nvram FIN state */
            pInstance->nvram.data->finState = STD_NOT_OK;
            /** FIN state not okay, but still in SBC init phase after power-cycle
             * continue now with RSTB assertion */
            test_assertionRSTB = true;
        } else if ((SBC_INIT_PHASE_e)pInstance->nvram.data->phase == SBC_RSTB_ASSERTION_TEST) {
            /** Reset was triggered by SPI RSTB assertion test -> continue with SBC init phase */
            test_assertionRSTB = false;

            /** Diagnosis of the RSTB pin/event is available by reading the FS_SAFE_IOs register:
             * RSTB_EVENT bit reports an activation of RSTB pin. */
            SBC_ReadBackRegister(pInstance, true, FS8X_FS_SAFE_IOS_ADDR);

            /** Check RSTB_EVENT if RSTB has been activated */
            if (STD_OK ==
                SBC_CheckRegisterValues(
                    (pInstance->fsRegister.safeIos & FS8X_FS_RSTB_EVENT_MASK), FS8X_FS_RSTB_EVENT_RESET_OCCURRED)) {
                /** RSTB pin should be sensed high and no RSTB short to high */
                uint16_t mask = (FS8X_FS_RSTB_DIAG_MASK | FS8X_FS_RSTB_SNS_MASK | FS8X_FS_RSTB_DRV_MASK);
                uint16_t expRegisterValue =
                    (FS8X_FS_RSTB_DIAG_NO_FAILURE | FS8X_FS_RSTB_SNS_PAD_SENSED_HIGH |
                     FS8X_FS_RSTB_DRV_COMMAND_SENSED_HIGH);
                if (STD_OK != SBC_CheckRegisterValues((pInstance->fsRegister.safeIos & mask), expRegisterValue)) {
                    retval = STD_NOT_OK;
                }
                /** Reset RSTB_EVENT flag */
                SBC_WriteBackRegisterFsInit(pInstance, FS8X_FS_SAFE_IOS_ADDR, FS8X_FS_RSTB_EVENT_RESET_OCCURRED);
                /** Update diag flag */
                DIAG_Handler(DIAG_ID_SBC_RSTB_STATE, DIAG_EVENT_OK, DIAG_SYSTEM, 0);
            } else {
                /** RSTB has not been activated but this should have been the case */
                retval = STD_NOT_OK;
                DIAG_Handler(DIAG_ID_SBC_RSTB_STATE, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0);
            }
        } else {
            /** Reset was not caused by SBC initialization or power-cycle.
             * Continue with SBC init phase as RSTB assertion is only tested
             * after power-cycle occurred */
            test_assertionRSTB = false;
        }
        /** Copy FIN state info from nvram variable into local state variable. This restores lost data from rest or
           updates local FIN state if short-circuit between FIN and RSTB has been detected */
        pInstance->fin.finState = pInstance->nvram.data->finState;
        if (pInstance->fin.finState == STD_OK) {
            DIAG_Handler(DIAG_ID_SBC_FIN_STATE, DIAG_EVENT_OK, DIAG_SYSTEM, 0);
        } else {
            DIAG_Handler(DIAG_ID_SBC_FIN_STATE, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0);
        }
    } else {
        /** Reset was not caused by power-cycle or SBC. SBC has already been initialized successfully after detected
         * power-cycle. Everything okay. Read FIN state from NVRAM and continue with normal operation */
        FRAM_Read(pInstance->nvram.entry);
        test_assertionRSTB      = false;
        pInstance->fin.finState = pInstance->nvram.data->finState;
        if (pInstance->fin.finState == STD_OK) {
            DIAG_Handler(DIAG_ID_SBC_FIN_STATE, DIAG_EVENT_OK, DIAG_SYSTEM, 0);
        } else {
            DIAG_Handler(DIAG_ID_SBC_FIN_STATE, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0);
        }
    }

    /** Verify the hardware connection between the MCU reset pin and the FS85 reset pin */
    if (test_assertionRSTB == true) {
        /** Write to NVRAM to determine after reset and if RSTB was asserted correctly */
        pInstance->nvram.data->phase = (uint8_t)SBC_RSTB_ASSERTION_TEST;
        FRAM_Write(pInstance->nvram.entry);

        SBC_ReadBackRegister(pInstance, true, FS8X_FS_STATES_ADDR);

        uint16_t registerValue = FS8X_FS_RSTB_REQ_RSTB_ASSERTION;
        SBC_WriteRegisterFsInit(pInstance, FS8X_FS_SAFE_IOS_ADDR, registerValue);

        while (1) {
            ;
        }

        /* Controller should never reach this point as reset via RSTB is requested */
    }
    return retval;
}

static STD_RETURN_TYPE_e SBC_PerformPathCheckFS0B(FS85xx_STATE_s *pInstance) {
    STD_RETURN_TYPE_e retval = STD_OK;
    /* TBD */
    return retval;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
