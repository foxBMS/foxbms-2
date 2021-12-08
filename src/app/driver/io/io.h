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
 * @file    io.h
 * @author  foxBMS Team
 * @date    2020-03-19 (date of creation)
 * @updated 2021-07-14 (date of last update)
 * @ingroup DRIVERS
 * @prefix  IO
 *
 * @brief   Header for the driver for the IO module
 *
 * @details TODO
 */

#ifndef FOXBMS__IO_H_
#define FOXBMS__IO_H_

/*========== Includes =======================================================*/
#include "general.h"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief   Set pin to output by writing in pin direction register.
 * @details The TMS570LC43 has only few dedicated GPIO register. Most of the
 *          time the IOs are controlled by a dedicated register in the
 *          corresponding peripheral.
 * @param   pRegisterAddress    address of pin direction register
 * @param   pin                 bit position corresponding to pin in register
 */
extern void IO_SetPinDirectionToOutput(volatile uint32_t *pRegisterAddress, uint32_t pin);

/**
 * @brief   Set pin to input by writing in pin direction register.
 * @details The TMS570LC43 has only few dedicated GPIO register. Most of the
 *          time the IOs are controlled by a dedicated register in the
 *          corresponding peripheral.
 * @param   pRegisterAddress    address of pin direction register
 * @param   pin                 bit position corresponding to pin in register
 */
extern void IO_SetPinDirectionToInput(volatile uint32_t *pRegisterAddress, uint32_t pin);

/**
 * @brief   Set pin by writing in pin output register.
 * @details The TMS570LC43 has only few dedicated GPIO register. Most of the
 *          time the IOs are controlled by a dedicated register in the
 *          corresponding peripheral.
 * @param   pRegisterAddress    address of pin output register
 * @param   pin                 bit position corresponding to pin in register
 */
extern void IO_PinSet(volatile uint32_t *pRegisterAddress, uint32_t pin);

/**
 * @brief   Reset pin by writing in pin output register.
 * @details The TMS570LC43 has only few dedicated GPIO register. Most of the
 *          time the IOs are controlled by a dedicated register in the
 *          corresponding peripheral.
 * @param   pRegisterAddress    address of pin output register
 * @param   pin                 bit position corresponding to pin in register
 */
extern void IO_PinReset(volatile uint32_t *pRegisterAddress, uint32_t pin);

/**
 * @brief   Get pin state.
 * @details The TMS570LC43 has only few dedicated GPIO register. Most of the
 *          time the IOs are controlled by a dedicated register in the
 *          corresponding peripheral.
 * @param   pRegisterAddress    address of pin output register
 * @param   pin                 bit position corresponding to pin in register
 * @return  #STD_PIN_STATE_e state of the pin
 */
extern STD_PIN_STATE_e IO_PinGet(const volatile uint32_t *pRegisterAddress, uint32_t pin);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__IO_H_ */
