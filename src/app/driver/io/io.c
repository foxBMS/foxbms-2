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
 * @file    io.c
 * @author  foxBMS Team
 * @date    2020-06-05 (date of creation)
 * @updated 2021-07-14 (date of last update)
 * @ingroup DRIVERS
 * @prefix  IO
 *
 * @brief   Driver for the IO module
 *
 *
 */

/*========== Includes =======================================================*/
#include "io.h"

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern void IO_SetPinDirectionToOutput(volatile uint32_t *pRegisterAddress, uint32_t pin) {
    FAS_ASSERT(pRegisterAddress != NULL_PTR);
    FAS_ASSERT(pin <= LARGEST_PIN_NUMBER);

    *pRegisterAddress |= (uint32_t)((uint32_t)1u << pin);
}

extern void IO_SetPinDirectionToInput(volatile uint32_t *pRegisterAddress, uint32_t pin) {
    FAS_ASSERT(pRegisterAddress != NULL_PTR);
    FAS_ASSERT(pin <= LARGEST_PIN_NUMBER);

    *pRegisterAddress &= ~(uint32_t)((uint32_t)1u << pin);
}

extern void IO_PinSet(volatile uint32_t *pRegisterAddress, uint32_t pin) {
    FAS_ASSERT(pRegisterAddress != NULL_PTR);
    FAS_ASSERT(pin <= LARGEST_PIN_NUMBER);

    *pRegisterAddress |= (uint32_t)((uint32_t)1u << pin);
}

extern void IO_PinReset(volatile uint32_t *pRegisterAddress, uint32_t pin) {
    FAS_ASSERT(pRegisterAddress != NULL_PTR);
    FAS_ASSERT(pin <= LARGEST_PIN_NUMBER);

    *pRegisterAddress &= ~(uint32_t)((uint32_t)1u << pin);
}

extern STD_PIN_STATE_e IO_PinGet(const volatile uint32_t *pRegisterAddress, uint32_t pin) {
    FAS_ASSERT(pRegisterAddress != NULL_PTR);
    FAS_ASSERT(pin <= LARGEST_PIN_NUMBER);

    STD_PIN_STATE_e retval = STD_PIN_UNDEFINED;
    uint8_t pinState       = (uint8_t)((*pRegisterAddress & ((uint32_t)1u << (pin))) >> pin);

    /* pinState 0 equals a low level */
    if (0u == pinState) {
        retval = STD_PIN_LOW;
    } else {
        retval = STD_PIN_HIGH;
    }
    return retval;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
