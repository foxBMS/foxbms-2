/**
 *
 * @copyright &copy; 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    fassert.h
 * @author  foxBMS Team
 * @date    2020-03-20 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup ASSERT
 * @prefix  FAS
 *
 * @brief   Assert macro implementation
 *
 * @details The default implementation accommodates three behaviors,
 *          based on FAS_ASSERT_LEVEL symbol:
 *
 *          - When the FAS_ASSERT_LEVEL symbol is defined as
 *            FAS_ASSERT_LEVEL_INF_LOOP_AND_DISABLE_INTERRUPTS, the FAS_ASSERT
 *            macro is implemented as an infinite loop in which all interrupts
 *            are disabled. This will definitely trigger a watchdog reset.
 *
 *          - When FAS_ASSERT_LEVEL symbol is defined as
 *            FAS_ASSERT_LEVEL_INF_LOOP_FOR_DEBUG, the validation performed by
 *            the FAS_ASSERT macro is enabled, and a failed validation triggers
 *            an infinite loop. This configuration is recommended for
 *            development environments, as it prevents further execution.
 *            Depending on the configuration this might lead to a watchdog
 *            reset. This mode is intended for investigation of problems by a
 *            developer.
 *
 *          - When FAS_ASSERT_LEVEL symbol is defined as
 *            FAS_ASSERT_LEVEL_NO_OPERATION, the FAS_ASSERT macro is defined as
 *            empty and does nothing. It might be necessary to activate this
 *            mode in resource-constrained situations. Generally it is not
 *            recommended to use this option as it will not notice the
 *            undefined system-states that the assert should catch.
 *
 *          Generally assertions should be used for asserting states and values
 *          that should occur when the program is executed correctly. Do *not*
 *          assert anything that *might* be false. As an example it is
 *          perfectly good practice to assert that the input to a function is
 *          not a null pointer if the function is not designed to accept null
 *          pointers, but it is not good to check if a communication error has
 *          occurred somewhere. Assertions should never trip in a bug-free
 *          program.
 *
 *          Usage is FAS_ASSERT(condition that should be true).
 */

#ifndef FOXBMS__FASSERT_H_
#define FOXBMS__FASSERT_H_

/*========== Includes =======================================================*/
#include <stdbool.h>
#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/* AXIVION Disable Style MisraC2012Directive-1.1: 'pragma' required to tell the
 * TI ARM CGT compiler, that this is an interrupt function
 * (see SPNU151V-January1998-RevisedFebruary2020: 5.11.29 The SWI_ALIAS Pragma)
 */
/* AXIVION Disable Style MisraC2012-1.2: Function is implemented in assembler
 * and this is the way to tell it the TI compiler (see
 * src\os\freertos\portable\ccs\arm_cortex-r5\portasm.asm::swiPortDisableInterrupts)
 */
/* AXIVION Disable Style MisraC2012-8.6: Function definition is in assembler
 * (see
 * src\os\freertos\portable\ccs\arm_cortex-r5\portasm.asm::swiPortDisableInterrupts)
 */
/**
 * @brief   Disable interrupts
 * @details This alias is mapped to an ASM function and disables all interrupts
 *          by writing to the SPSR (Saved Program Status Register) register
 *          through the control field mask byte PSR[7:0] (privileged
 *          software execution)
 */
#pragma SWI_ALIAS(FAS_DisableInterrupts, 5)
extern void FAS_DisableInterrupts(void);
/* AXIVION Enable Style MisraC2012-8.6: */
/* AXIVION Enable Style MisraC2012-1.2: */
/* AXIVION Enable Style MisraC2012Directive-1.1: */

/**
 * @brief       Define that evaluates to essential boolean false thus tripping
 *              an assert.
 * @details     Call FAS_ASSERT() with this define in order to stop the code
 *              and trip an assertion.
 */
#define FAS_TRAP (0u == 1u)

/**
 * @brief       Struct for storing assert information
 * @details     This struct is intended for storing, information on the place
 *              in the code where an assert has been violated.
 */
typedef struct {
    uint32_t *pc;  /*!< value of the program counter register */
    uint32_t line; /*!< line number where an assert has triggered */
} FAS_ASSERT_LOCATION_s;

/**
 * @brief   Copy the assert location into the assert struct.
 * @details Takes the location of the last assertion and stores it into the
 *          static fas_assertLocation.
 *          This definition has to be at this position in order to be used by
 *          the macros below.
 *          If you get issues in a unit test with this being not defined, try
 *          to add this header to the unit tests includes explicitly.
 * @param[in]   pc      address of the program counter where the assertion
 *                      occurred
 * @param[in]   line    line where the assertion occurred
 */
extern void FAS_StoreAssertLocation(uint32_t *pc, uint32_t line);

/**
 * @def     FAS_ASSERT_LEVEL_INF_LOOP_AND_DISABLE_INTERRUPTS
 * @brief   This assert level traps the complete program by going into an
 *          infinite loop and disabling all interrupts.
 *
 * @def     FAS_ASSERT_LEVEL_INF_LOOP_FOR_DEBUG
 * @brief   This assert level traps the current task by going into an
 *          infinite loop.
 *
 * @def     FAS_ASSERT_LEVEL_NO_OPERATION
 * @brief   This assert level does nothing (except for the standard recording
 *          of the assert location which does every level).
 */
#define FAS_ASSERT_LEVEL_INF_LOOP_AND_DISABLE_INTERRUPTS (0u)
#define FAS_ASSERT_LEVEL_INF_LOOP_FOR_DEBUG              (1u)
#define FAS_ASSERT_LEVEL_NO_OPERATION                    (2u)

/**
 * @def     FAS_ASSERT_LEVEL
 * @brief   Set the assert level to a standard value if not set by the build.
 */
#ifndef FAS_ASSERT_LEVEL
#define FAS_ASSERT_LEVEL FAS_ASSERT_LEVEL_INF_LOOP_AND_DISABLE_INTERRUPTS
#endif

/*============= define how the assert shall behave =============*/
#if FAS_ASSERT_LEVEL == FAS_ASSERT_LEVEL_INF_LOOP_AND_DISABLE_INTERRUPTS
/** Assert macro will trigger a watchdog reset */
static inline void FAS_InfiniteLoop(void) {
    /* disable IRQ interrupts */
    FAS_DisableInterrupts();
    /* AXIVION Next Codeline Style MisraC2012-2.2: an infinite loop is intended to stop further code execution */
    while (true) { /* Stay here until watchdog reset happens */
    }
}
#elif FAS_ASSERT_LEVEL == FAS_ASSERT_LEVEL_INF_LOOP_FOR_DEBUG
/** Assert macro will stay in infinite loop */
static inline void FAS_InfiniteLoop(void) {
    while (true) {
        /* Stay here to ease debugging */
    }
}
#elif FAS_ASSERT_LEVEL == FAS_ASSERT_LEVEL_NO_OPERATION
static inline void FAS_InfiniteLoop(void) {
}
#else
#error "Invalid value for FAS_ASSERT_LEVEL"
#endif

/*============= define the recording macro =============*/
#ifdef UNITY_UNIT_TEST
/**
 * @def     __curpc(x)
 * @brief   replaces in unit test the (platform-specific) function for the retrieval of the program counter
 */
static inline uint32_t __curpc(void) {
    return 0u;
}
#endif

/**
 * @brief   Record the assert location
 * @details Retrieves the program counter (with __curpc()) and line-number at
 *          the current location and passes it to #FAS_StoreAssertLocation().
 *
 *          It is important that this is a macro in order to insert it directly
 *          at he assert location in the code
 */
/* AXIVION Next Construct Style MisraC2012-11.5: The program counter needs to be casted to platform register width */
#define FAS_ASSERT_RECORD()                    \
    do {                                       \
        uint32_t *pc = (uint32_t *)__curpc();  \
        FAS_StoreAssertLocation(pc, __LINE__); \
    } while (false)

/*============= define the assertion-macro =============*/
/**
 * @def     FAS_ASSERT(x)
 * @brief   Assertion macro that asserts that x is true
 * @details This macro asserts the taken argument x. If the assertion fails
 *          it calls #FAS_ASSERT_RECORD() and then #FAS_InfiniteLoop().
 *
 *          In unit tests this is replace by an exception that is thrown in
 *          order to be able to test for a failed assertion.
 *
 *          Use this macro if you want to assert. If the assertion fails
 *          the macro will take action based on the configuration of this module.
 *          See #FAS_ASSERT_LEVEL for reference.
 *
 *          If the macro passes, it is just ignored. If you want to definitely
 *          fail, you can use the value #FAS_TRAP as an argument which is
 *          designed to fail in all cases.
 */
#ifdef UNITY_UNIT_TEST
#include "CException.h"
#define FAS_ASSERT(x) \
    if (!(x))         \
    Throw(0)
#else
#define FAS_ASSERT(x)            \
    do {                         \
        if (!(x)) {              \
            FAS_ASSERT_RECORD(); \
            FAS_InfiniteLoop();  \
        }                        \
    } while (false)
#endif

#if defined(__STDC_VERSION__) /* We have some newer compiler (C94 at least) */
#if __STDC_VERSION__ == 199409L
#warning "Ignoring static asserts in C94 mode (FAS_STATIC_ASSERT)."
#define FAS_STATIC_ASSERT(cond, msg)
#elif __STDC_VERSION__ == 199901L
#if defined(__TI_COMPILER_VERSION__)
/* The TI compiler seems to have hacks to enable static assert see include/sys/cdefs.h */
#define FAS_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else /* for other compilers just ignore the static assert */
#warning "Ignoring static asserts in C99 mode (FAS_STATIC_ASSERT)."
#define FAS_STATIC_ASSERT(...)
#endif
#elif __STDC_VERSION__ == 201112L
#define FAS_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#elif __STDC_VERSION__ == 201710L
#define FAS_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#endif
#else
/* if __STDC_VERSION__ is not defined, we have some very old compiler and we
   need to ignore static asserting */
#warning "Ignoring static asserts in C89/C90 mode (FAS_STATIC_ASSERT)."
#define FAS_STATIC_ASSERT(cond, msg)
#endif

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__FASSERT_H_ */
