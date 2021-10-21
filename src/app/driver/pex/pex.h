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
 * @file    pex.h
 * @author  foxBMS Team
 * @date    2021-08-02 (date of creation)
 * @updated 2021-10-04 (date of last update)
 * @ingroup DRIVERS
 * @prefix  PEX
 *
 * @brief   Header for the driver for the NXP PCA9539 port expander module
 *
 */

#ifndef FOXBMS__PEX_H_
#define FOXBMS__PEX_H_

/*========== Includes =======================================================*/

#include "pex_cfg.h"

/*========== Macros and Definitions =========================================*/
/** Port expander pin high levels in registers
 * @{*/
#define PEX_PIN_LOW  (0u)
#define PEX_PIN_HIGH (1u)
/**@}*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   initialize local variable containing state of port expander.
 *
 */
extern void PEX_Initialize(void);

/**
 * @brief   sets pin to high.
 * @param   portExpander    port expander number
 * @param   pin             pin number
 */
extern void PEX_SetPin(uint8_t portExpander, uint8_t pin);

/**
 * @brief   sets pin to low.
 * @param   portExpander    port expander number
 * @param   pin             pin number
 *
 */
extern void PEX_ResetPin(uint8_t portExpander, uint8_t pin);

/**
 * @brief   get pin state of port expander pins.
 * @return  pin state: #PEX_PIN_LOW or #PEX_PIN_HIGH
 */
extern uint8_t PEX_GetPin(uint8_t portExpander, uint8_t pin);

/**
 * @brief   sets pin direction to input.
 * @param   portExpander    port expander number
 * @param   pin             pin number
 */
extern void PEX_SetPinDirectionInput(uint8_t portExpander, uint8_t pin);

/**
 * @brief   sets pin to input.
 * @param   portExpander    port expander number
 * @param   pin             pin number
 */
extern void PEX_SetPinDirectionOutput(uint8_t portExpander, uint8_t pin);

/**
 * @brief   sets pin polarity to inverted.
 * @param   portExpander    port expander number
 * @param   pin             pin number
 */
extern void PEX_SetPinPolarityInverted(uint8_t portExpander, uint8_t pin);

/**
 * @brief   sets pin polarity to retained.
 * @param   portExpander    port expander number
 * @param   pin             pin number
 */
extern void PEX_SetPinPolarityRetained(uint8_t portExpander, uint8_t pin);

/**
 * @brief   implements reading/writing to the port expander registers.
 * @details This function reads the desired port expander state (outputs,
 *          configuration) from the externally available variables and stores
 *          it in the local variable. It then applies this state to the devices
 *          via the I2C bus. It reads the pin input state via the I2C bus and
 *          writes it to the externally available port expander state.
 */
extern void PEX_Trigger(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__PEX_H_ */
