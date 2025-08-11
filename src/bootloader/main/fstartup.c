/**
 * Copyright (C) 2009-2018 Texas Instruments Incorporated - www.ti.com
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * @file    fstartup.c
 * @date    11-Dec-2018
 * @updated 2025-08-05 (date of last update)
 * @version v1.10.0
 * @ingroup MAIN
 * @prefix  STU
 *
 * @brief   Startup Source File
 * @details This file is created by TI HALCoGen 04.07.01 and adapted to foxBMS
 *          specific needs.
 *          This file contains the VIM RAM setup and the startup routine.
 *          Changes are:
 *          - add 'section'-markers
 *          - add a prefix
 *          - add code justifications for SPA (Axivion)
 *          - make code unit testable ('UNITY_UNIT_TEST')
 *          - add function 'STU_GetResetSourceWithoutFlagReset'
 *
 */

/*========== Includes =======================================================*/
/* clang-format off */

/* For the unit test case, we need this include first for mocking purposes; see
 * file-level doxygen comment in 'tests/unit/bootloader/main/test_fstartup.c'
 * for more details */
#ifdef UNITY_UNIT_TEST
#include "mock_esm.h"
#endif

/* keep include order as suggested by TI HALCoGen */
#include "HL_sys_common.h"
#include "HL_system.h"
#include "HL_sys_vim.h"
#include "HL_sys_core.h"
#include "HL_esm.h"
#include "HL_reg_esm.h"
#include "HL_sys_mpu.h"
#include "HL_errata_SSWF021_45.h"
/* clang-format on */

#include "fstartup.h"

#include "fassert.h"
#include "main.h"

#include <stdint.h>
#include <stdlib.h>

/*========== Macros and Definitions =========================================*/
/** number of retries for the PLL to come up */
#define STU_PLL_RETRIES (5u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Handler for a failed PLL lock
 * @details If the PLL can not be locked the, this function shall be called to
 *          ensure that the application no further starts.
 * @return  This function never returns */
static void STU_HandlePllLockFail(void);

#ifndef UNITY_UNIT_TEST
/**
 * @brief   Get reset flag
 * @details Get reset source without reseting respective the flag in SYSESR
 *          register
 * @return  returns reset reason
 */
static resetSource_t STU_GetResetSourceWithoutFlagReset(void);
#else
extern resetSource_t STU_GetResetSourceWithoutFlagReset(void);
#endif

/*========== Static Function Implementations ================================*/
static void STU_HandlePllLockFail(void) {
    FAS_ASSERT(FAS_TRAP);
} /* LCOV_EXCL_LINE */

#ifndef UNITY_UNIT_TEST
static resetSource_t STU_GetResetSourceWithoutFlagReset(void) {
    register resetSource_t rst_source;

    if ((SYS_EXCEPTION & (uint32)POWERON_RESET) != 0u) {
        /* power-on reset condition */
        rst_source = POWERON_RESET;
    } else if ((SYS_EXCEPTION & (uint32)EXT_RESET) != 0u) {
        /*** Check for other causes of EXT_RESET that would take precedence **/
        if ((SYS_EXCEPTION & (uint32)OSC_FAILURE_RESET) != 0u) {
            /* Reset caused due to oscillator failure. Add user code here to handle oscillator failure */
            rst_source = OSC_FAILURE_RESET;
        } else if ((SYS_EXCEPTION & (uint32)WATCHDOG_RESET) != 0u) {
            /* Reset caused due watchdog violation */
            rst_source = WATCHDOG_RESET;
        } else if ((SYS_EXCEPTION & (uint32)WATCHDOG2_RESET) != 0u) {
            /* Reset caused due watchdog violation */
            rst_source = WATCHDOG2_RESET;
        } else if ((SYS_EXCEPTION & (uint32)SW_RESET) != 0u) {
            /* Reset caused due to software reset. */
            rst_source = SW_RESET;
        } else {
            /* Reset caused due to External reset. */
            rst_source = EXT_RESET;
        }
    } else if ((SYS_EXCEPTION & (uint32)DEBUG_RESET) != 0u) {
        /* Reset caused due Debug reset request */
        rst_source = DEBUG_RESET;
    } else if ((SYS_EXCEPTION & (uint32)CPU0_RESET) != 0u) {
        /* Reset caused due to CPU0 reset. CPU reset can be caused by CPU self-test completion, or by toggling the "CPU
         * RESET" bit of the CPU Reset Control Register. */
        rst_source = CPU0_RESET;
    } else {
        /* No_reset occurred. */
        rst_source = NO_RESET;
    }
    return rst_source;
}
#endif

/*========== Extern Function Implementations ================================*/
/** system entry point */
#pragma CODE_STATE(_c_int00, 32)
#pragma INTERRUPT(_c_int00, RESET)
/* SourceId : STARTUP_SourceId_001 */
/* DesignId : STARTUP_DesignId_001 */
/* Requirements : HL_CONQ_STARTUP_SR1 */
void _c_int00(void) {
    register resetSource_t rstSrc;

    /* Initialize Core Registers to avoid CCM Error */
    _coreInitRegisters_();

    /* Initialize Stack Pointers */
    _coreInitStackPointer_();

    /* Reset handler: the following instructions read from the system exception status register
     * to identify the cause of the CPU reset.
     */
    /* Changed in comparison to TI _c_int00 implementation. Readout flags but do NOT clear them! */
    rstSrc = STU_GetResetSourceWithoutFlagReset();

    switch (rstSrc) {
        case POWERON_RESET:
            /* Initialize L2RAM to avoid ECC errors right after power on */
            _memInit_();

            /* Add condition to check whether PLL can be started successfully */
            if (_errata_SSWF021_45_both_plls(STU_PLL_RETRIES) != 0u) {
                /* Put system in a safe state */
                STU_HandlePllLockFail();
            }

            /*SAFETYMCUSW 62 S MR:15.2, 15.5 <APPROVED> "Need to continue to handle POWERON Reset" */
            /* FALLTHRU */
        case DEBUG_RESET:
        case EXT_RESET:
        case SW_RESET:
            /* Initialize L2RAM to avoid ECC errors right after power on */
            if (rstSrc != POWERON_RESET) {
                _memInit_();
            }

            /* Enable CPU Event Export */
            /* This allows the CPU to signal any single-bit or double-bit errors detected
             * by its ECC logic for accesses to program flash or data RAM.
             */
            _coreEnableEventBusExport_();

            /* Check if there were ESM group3 errors during power-up.
             * These could occur during eFuse auto-load or during reads from flash OTP
             * during power-up. Device operation is not reliable and not recommended
             * in this case. */
            if ((esmREG->SR1[2]) != 0U) {
                esmGroup3Notification(esmREG, esmREG->SR1[2]);
            }

            /* Initialize System - Clock, Flash settings with Efuse self check */
            systemInit();

            /* Enable IRQ offset via Vic controller */
            _coreEnableIrqVicOffset_();

            /* Initialize VIM table */
            vimInit();

            /* Check if there were ESM group3 errors during power-up.
             * These could occur during eFuse auto-load or during reads from flash OTP
             * during power-up. Device operation is not reliable and not recommended
             * in this case. */
            esmInit();
            break;

        case OSC_FAILURE_RESET:
            break;

        case WATCHDOG_RESET:
        case WATCHDOG2_RESET:
            break;

        case CPU0_RESET:
            /* Enable CPU Event Export */
            /* This allows the CPU to signal any single-bit or double-bit errors detected
             * by its ECC logic for accesses to program flash or data RAM.
             */
            _coreEnableEventBusExport_();
            break;

        default:
            break;
    }

    _mpuInit_();

    _cacheEnable_();

    /* initialize global variable and constructors */
    __TI_auto_init();

#ifndef UNITY_UNIT_TEST
    /* call the application */
    /*SAFETYMCUSW 296 S MR:8.6 <APPROVED> "Startup code(library functions at block scope)" */
    /*SAFETYMCUSW 326 S MR:8.2 <APPROVED> "Startup code(Declaration for main in library)" */
    /*SAFETYMCUSW 60 D MR:8.8 <APPROVED> "Startup code(Declaration for main in library; Only doing an extern for the same)" */
    /* AXIVION Next Codeline Style MisraC2012-17.7, MisraC2012Directive-4.7 */
    main();
#endif

    /*SAFETYMCUSW 122 S MR:20.11 <APPROVED> "Startup code(exit and abort need to be present)" */
    /* AXIVION Next Codeline Style MisraC2012-21.8: exit is called as in generated code by TI */
#ifndef UNITY_UNIT_TEST
    exit(0); /* exit can not be tested in unit tests */
#endif
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern void TEST_STU_HandlePllLockFail(void) {
    STU_HandlePllLockFail();
} /* LCOV_EXCL_LINE */
#endif
