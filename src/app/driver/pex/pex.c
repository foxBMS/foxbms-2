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
 * @file    pex.c
 * @author  foxBMS Team
 * @date    2021-08-02 (date of creation)
 * @updated 2021-12-08 (date of last update)
 * @ingroup DRIVERS
 * @prefix  PEX
 *
 * @brief   Driver for the NXP PCA9539 port expander module
 *
 */

/*========== Includes =======================================================*/
#include "pex.h"

#include "diag.h"
#include "i2c.h"
#include "os.h"

/*========== Macros and Definitions =========================================*/
/** Number of ports per register */
#define PEX_NR_OF_PORTS_PER_REGISTER (8u)

/** Initial value: all 1 */
#define PEX_DEFAULT_VALUE_ALL_1 (0xFFu)
/** Initial value: all 0 */
#define PEX_DEFAULT_VALUE_ALL_0 (0x0u)

/** Address of Input Port 0 register (port 1 is one higher, 0x1u) */
#define PEX_INPUT_PORT0_REGISTER_ADDRESS (0x0u)
/** Address of Output Port 0 register (port 1 is one higher, 0x3u) */
#define PEX_OUTPUT_PORT0_REGISTER_ADDRESS (0x2u)
/** Address of Polarity Inversion Port 0 register (port 1 is one higher, 0x5u) */
#define PEX_POL_INV_PORT0_REGISTER_ADDRESS (0x4u)
/** Address of Configuration Port 0 register (port 1 is one higher, 0x7u) */
#define PEX_DIRECTION_PORT0_REGISTER_ADDRESS (0x6u)

/** Port expander pin polarity inversion in registers
 * @{*/
#define PEX_PIN_POLARITY_RETAINED (0u)
#define PEX_PIN_POLARITY_INVERTED (1u)
/**@}*/

/** Port expander pin direction configuration in registers
 * @{*/
#define PEX_PIN_DIRECTION_OUTPUT (0u)
#define PEX_PIN_DIRECTION_INPUT  (1u)
/**@}*/

/*========== Static Constant and Variable Definitions =======================*/
/** I2C write buffer for PEX */
static uint8_t pex_i2cDataWrite[2u] = {0};

/**
 * These variables are used to configure the port expanders (input, output,
 * configuration) from external modules.
 * @{
 */
static uint8_t pex_inputPort0[PEX_NR_OF_PORT_EXPANDERS]           = {0};
static uint8_t pex_inputPort1[PEX_NR_OF_PORT_EXPANDERS]           = {0};
static uint8_t pex_outputPort0[PEX_NR_OF_PORT_EXPANDERS]          = {0};
static uint8_t pex_outputPort1[PEX_NR_OF_PORT_EXPANDERS]          = {0};
static uint8_t pex_configPolarityPort0[PEX_NR_OF_PORT_EXPANDERS]  = {0};
static uint8_t pex_configPolarityPort1[PEX_NR_OF_PORT_EXPANDERS]  = {0};
static uint8_t pex_configDirectionPort0[PEX_NR_OF_PORT_EXPANDERS] = {0};
static uint8_t pex_configDirectionPort1[PEX_NR_OF_PORT_EXPANDERS] = {0};
/**@}*/

/** These local variables hold the state of the port expanders (input, output,
 * configuration)
 * @{
 */
static uint8_t pex_inputPort0Local[PEX_NR_OF_PORT_EXPANDERS]           = {0};
static uint8_t pex_inputPort1Local[PEX_NR_OF_PORT_EXPANDERS]           = {0};
static uint8_t pex_outputPort0Local[PEX_NR_OF_PORT_EXPANDERS]          = {0};
static uint8_t pex_outputPort1Local[PEX_NR_OF_PORT_EXPANDERS]          = {0};
static uint8_t pex_configPolarityPort0Local[PEX_NR_OF_PORT_EXPANDERS]  = {0};
static uint8_t pex_configPolarityPort1Local[PEX_NR_OF_PORT_EXPANDERS]  = {0};
static uint8_t pex_configDirectionPort0Local[PEX_NR_OF_PORT_EXPANDERS] = {0};
static uint8_t pex_configDirectionPort1Local[PEX_NR_OF_PORT_EXPANDERS] = {0};
/**@}*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/**
 * @brief   reads input state of port expander pins over I2C.
 * @return  #STD_OK if I2C transmission ok, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e PEX_ReadInputs(void);

/**
 * @brief   sets output state of port expander pins over I2C.
 * @return  #STD_OK if I2C transmission ok, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e PEX_WriteOutputs(void);

/**
 * @brief   sets polarity inversion state of port expander pins over I2C.
 * @return  #STD_OK if I2C transmission ok, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e PEX_WriteConfigPolarity(void);

/**
 * @brief   sets direction of port expander pins over I2C.
 * @return  #STD_OK if I2C transmission ok, #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e PEX_WriteConfigDirection(void);

/**
 * @brief   copies values from the externally available variables to the
 *          local ones.
 */
static void PEX_CopyToLocalVariable(void);

/**
 * @brief   copies values from the local variables to the
 *          externally available ones.
 */
static void PEX_GetFromLocalVariable(void);

/*========== Static Function Implementations ================================*/

static STD_RETURN_TYPE_e PEX_ReadInputs(void) {
    STD_RETURN_TYPE_e retVal = STD_OK;

    for (uint8_t i = 0u; i < PEX_NR_OF_PORT_EXPANDERS; i++) {
        /*
         * Input Port 0 as address, next read register will be Input Port 1
         * data sheet: Rev. 9 - 8 November 2017
         * "After the first byte is read, additional bytes may be read but
         * the data will now reflect the information in the other register in the pair.
         * For example, if you read Input port 1, then the next byte read would be Input port 0."
         */
        uint8_t pex_i2cDataRead[2u] = {0u, 0u};
        STD_RETURN_TYPE_e i2cReadReturn =
            I2C_Read(pex_addressList[i], PEX_INPUT_PORT0_REGISTER_ADDRESS, 2u, pex_i2cDataRead);
        if (i2cReadReturn == STD_NOT_OK) {
            retVal = STD_NOT_OK;
        } else {
            pex_inputPort0Local[i] = pex_i2cDataRead[0u];
            pex_inputPort1Local[i] = pex_i2cDataRead[1u];
        }
    }
    return retVal;
}

static STD_RETURN_TYPE_e PEX_WriteOutputs(void) {
    STD_RETURN_TYPE_e retVal = STD_OK;
    for (uint8_t i = 0u; i < PEX_NR_OF_PORT_EXPANDERS; i++) {
        /**
         * Outport Port 0 as address, next read register will be Output Port 1
         * data sheet: Rev. 9 - 8 November 2017
         * Figure 10: one register pair can be written in one transaction
         */
        pex_i2cDataWrite[0u] = pex_outputPort0Local[i];
        pex_i2cDataWrite[1u] = pex_outputPort1Local[i];
        STD_RETURN_TYPE_e i2cWriteReturn =
            I2C_Write(pex_addressList[i], PEX_OUTPUT_PORT0_REGISTER_ADDRESS, 2u, pex_i2cDataWrite);
        if (i2cWriteReturn == STD_NOT_OK) {
            retVal = STD_NOT_OK;
        }
    }
    return retVal;
}

static STD_RETURN_TYPE_e PEX_WriteConfigPolarity(void) {
    STD_RETURN_TYPE_e retVal = STD_OK;
    for (uint8_t i = 0u; i < PEX_NR_OF_PORT_EXPANDERS; i++) {
        /**
         * Inversion Polarity Port 0 as address, next read register will be Inversion Polarity Port 1
         * data sheet: Rev. 9 - 8 November 2017
         * Figure 10: one register pair can be written in one transaction
         */
        pex_i2cDataWrite[0u] = pex_configPolarityPort0Local[i];
        pex_i2cDataWrite[1u] = pex_configPolarityPort1Local[i];
        STD_RETURN_TYPE_e i2cWriteReturn =
            I2C_Write(pex_addressList[i], PEX_POL_INV_PORT0_REGISTER_ADDRESS, 2u, pex_i2cDataWrite);
        if (i2cWriteReturn == STD_NOT_OK) {
            retVal = STD_NOT_OK;
        }
    }
    return retVal;
}

static STD_RETURN_TYPE_e PEX_WriteConfigDirection(void) {
    STD_RETURN_TYPE_e retVal = STD_OK;
    for (uint8_t i = 0u; i < PEX_NR_OF_PORT_EXPANDERS; i++) {
        /**
         * Direction Port 0 as address, next read register will be Direction Port 1
         * data sheet: Rev. 9 - 8 November 2017
         * Figure 10: one register pair can be written in one transaction
         */
        pex_i2cDataWrite[0u] = pex_configDirectionPort0Local[i];
        pex_i2cDataWrite[1u] = pex_configDirectionPort1Local[i];
        STD_RETURN_TYPE_e i2cWriteReturn =
            I2C_Write(pex_addressList[i], PEX_DIRECTION_PORT0_REGISTER_ADDRESS, 2u, pex_i2cDataWrite);
        if (i2cWriteReturn == STD_NOT_OK) {
            retVal = STD_NOT_OK;
        }
    }
    return retVal;
}

static void PEX_CopyToLocalVariable(void) {
    OS_EnterTaskCritical();
    for (uint8_t i = 0u; i < PEX_NR_OF_PORT_EXPANDERS; i++) {
        pex_outputPort0Local[i]          = pex_outputPort0[i];
        pex_outputPort1Local[i]          = pex_outputPort1[i];
        pex_configPolarityPort0Local[i]  = pex_configPolarityPort0[i];
        pex_configPolarityPort1Local[i]  = pex_configPolarityPort1[i];
        pex_configDirectionPort0Local[i] = pex_configDirectionPort0[i];
        pex_configDirectionPort1Local[i] = pex_configDirectionPort1[i];
    }
    OS_ExitTaskCritical();
}

static void PEX_GetFromLocalVariable(void) {
    OS_EnterTaskCritical();
    for (uint8_t i = 0u; i < PEX_NR_OF_PORT_EXPANDERS; i++) {
        pex_inputPort0[i] = pex_inputPort0Local[i];
        pex_inputPort1[i] = pex_inputPort1Local[i];
    }
    OS_ExitTaskCritical();
}

/*========== Extern Function Implementations ================================*/

/**
 * In the data sheet, pins are grouped in two sets, numbered from 00 to 07 and from 10 to 17.
 * Each set is handled by a separate register, so pin 00 is numbered 0, pin 01 is numbered 01, ..,
 * pin 07 is numbered 07. But also pin 10 is numbered 0, pin 11 is numbered 01, .., pin 17 is numbered 07.
 * Using defines from 0 to 15, it is possible to get with pin number which register has to be addressed:
 * 0 to 7 means register for 00 to 07, 8 to 15 means register for 10 to 17.
 * Applying (pin%8) for 8 to 15 gives 0 to 7, so the numbering in register is obtained immediately.
 */
extern void PEX_Initialize(void) {
    for (uint8_t i = 0u; i < PEX_NR_OF_PORT_EXPANDERS; i++) {
        /** Default state of output registers data sheet: Rev. 9 - 8 November 2017, Table 7 and 8 */
        pex_outputPort0[i]      = PEX_DEFAULT_VALUE_ALL_1;
        pex_outputPort1[i]      = PEX_DEFAULT_VALUE_ALL_1;
        pex_outputPort0Local[i] = PEX_DEFAULT_VALUE_ALL_1;
        pex_outputPort1Local[i] = PEX_DEFAULT_VALUE_ALL_1;
        /**
         * Default state of polarity inversion registers data sheet: Rev. 9 - 8 November 2017, Table 9 and 10
         */
        pex_configPolarityPort0[i]      = PEX_DEFAULT_VALUE_ALL_0;
        pex_configPolarityPort1[i]      = PEX_DEFAULT_VALUE_ALL_0;
        pex_configPolarityPort0Local[i] = PEX_DEFAULT_VALUE_ALL_0;
        pex_configPolarityPort1Local[i] = PEX_DEFAULT_VALUE_ALL_0;
        /** Default state of direction registers data sheet: Rev. 9 - 8 November 2017, Table 11 and 12 */
        pex_configDirectionPort0[i]      = PEX_DEFAULT_VALUE_ALL_1;
        pex_configDirectionPort1[i]      = PEX_DEFAULT_VALUE_ALL_1;
        pex_configDirectionPort0Local[i] = PEX_DEFAULT_VALUE_ALL_1;
        pex_configDirectionPort1Local[i] = PEX_DEFAULT_VALUE_ALL_1;
    }
}

extern void PEX_Trigger(void) {
    PEX_CopyToLocalVariable();

    STD_RETURN_TYPE_e writeConfigDirectionReturn = PEX_WriteConfigDirection();
    STD_RETURN_TYPE_e writeConfigPolarityReturn  = PEX_WriteConfigPolarity();
    STD_RETURN_TYPE_e readInputsReturn           = PEX_ReadInputs();
    STD_RETURN_TYPE_e writeOutputsReturn         = PEX_WriteOutputs();

    /* notify diag if one of these functions failed, but continue normally */
    if ((writeConfigDirectionReturn == STD_NOT_OK) || (writeConfigPolarityReturn == STD_NOT_OK) ||
        (readInputsReturn == STD_NOT_OK) || (writeOutputsReturn == STD_NOT_OK)) {
        DIAG_Handler(DIAG_ID_I2C_PEX_ERROR, DIAG_EVENT_NOT_OK, DIAG_SYSTEM, 0u);
    } else {
        DIAG_Handler(DIAG_ID_I2C_PEX_ERROR, DIAG_EVENT_OK, DIAG_SYSTEM, 0u);
    }

    PEX_GetFromLocalVariable();
}

extern void PEX_SetPin(uint8_t portExpander, uint8_t pin) {
    FAS_ASSERT(portExpander < PEX_NR_OF_PORT_EXPANDERS);
    FAS_ASSERT(pin <= PEX_PIN17);

    if (pin <= PEX_PIN07) {
        OS_EnterTaskCritical();
        pex_outputPort0[portExpander] |= PEX_PIN_HIGH << pin;
        OS_ExitTaskCritical();
    } else { /* pin is in range PEX_PIN10 to PEX_PIN17 */
        OS_EnterTaskCritical();
        pex_outputPort1[portExpander] |= PEX_PIN_HIGH << (pin % PEX_NR_OF_PORTS_PER_REGISTER);
        OS_ExitTaskCritical();
    }
}

extern void PEX_ResetPin(uint8_t portExpander, uint8_t pin) {
    FAS_ASSERT(portExpander < PEX_NR_OF_PORT_EXPANDERS);
    FAS_ASSERT(pin <= PEX_PIN17);

    if (pin <= PEX_PIN07) {
        OS_EnterTaskCritical();
        pex_outputPort0[portExpander] &= ~(PEX_PIN_HIGH << pin);
        OS_ExitTaskCritical();
    } else { /* pin is in range PEX_PIN10 to PEX_PIN17 */
        OS_EnterTaskCritical();
        pex_outputPort1[portExpander] &= ~(PEX_PIN_HIGH << (pin % PEX_NR_OF_PORTS_PER_REGISTER));
        OS_ExitTaskCritical();
    }
}

extern uint8_t PEX_GetPin(uint8_t portExpander, uint8_t pin) {
    FAS_ASSERT(portExpander < PEX_NR_OF_PORT_EXPANDERS);
    FAS_ASSERT(pin <= PEX_PIN17);

    uint8_t pinState = UINT8_MAX;

    if (pin <= PEX_PIN07) {
        OS_EnterTaskCritical();
        pinState = (pex_inputPort0[portExpander] >> pin) & 0x1u;
        OS_ExitTaskCritical();
    } else { /* pin is in range PEX_PIN10 to PEX_PIN17 */
        OS_EnterTaskCritical();
        pinState = (pex_inputPort1[portExpander] >> (pin % PEX_NR_OF_PORTS_PER_REGISTER)) & 0x1u;
        OS_ExitTaskCritical();
    }

    if (pinState == 0u) {
        pinState = PEX_PIN_LOW;
    } else if (pinState == 1u) {
        pinState = PEX_PIN_HIGH;
    } else {
        /* invalid pin state */
        FAS_ASSERT(FAS_TRAP);
    }

    return pinState;
}

extern void PEX_SetPinDirectionInput(uint8_t portExpander, uint8_t pin) {
    FAS_ASSERT(portExpander < PEX_NR_OF_PORT_EXPANDERS);
    FAS_ASSERT(pin <= PEX_PIN17);

    if (pin <= PEX_PIN07) {
        OS_EnterTaskCritical();
        pex_configDirectionPort0[portExpander] |= PEX_PIN_DIRECTION_INPUT << pin;
        OS_ExitTaskCritical();
    } else { /* pin is in range PEX_PIN10 to PEX_PIN17 */
        OS_EnterTaskCritical();
        pex_configDirectionPort1[portExpander] |= PEX_PIN_DIRECTION_INPUT << (pin % PEX_NR_OF_PORTS_PER_REGISTER);
        OS_ExitTaskCritical();
    }
}

extern void PEX_SetPinDirectionOutput(uint8_t portExpander, uint8_t pin) {
    FAS_ASSERT(portExpander < PEX_NR_OF_PORT_EXPANDERS);
    FAS_ASSERT(pin <= PEX_PIN17);

    if (pin <= PEX_PIN07) {
        OS_EnterTaskCritical();
        pex_configDirectionPort0[portExpander] &= ~(((~PEX_PIN_DIRECTION_OUTPUT) & 0x1u) << pin);
        OS_ExitTaskCritical();
    } else { /* pin is in range PEX_PIN10 to PEX_PIN17 */
        OS_EnterTaskCritical();
        pex_configDirectionPort1[portExpander] &=
            ~(((~PEX_PIN_DIRECTION_OUTPUT) & 0x1u) << (pin % PEX_NR_OF_PORTS_PER_REGISTER));
        OS_ExitTaskCritical();
    }
}

extern void PEX_SetPinPolarityInverted(uint8_t portExpander, uint8_t pin) {
    FAS_ASSERT(portExpander < PEX_NR_OF_PORT_EXPANDERS);
    FAS_ASSERT(pin <= PEX_PIN17);

    if (pin <= PEX_PIN07) {
        OS_EnterTaskCritical();
        pex_configPolarityPort0[portExpander] |= PEX_PIN_POLARITY_INVERTED << pin;
        OS_ExitTaskCritical();
    } else { /* pin is in range PEX_PIN10 to PEX_PIN17 */
        OS_EnterTaskCritical();
        pex_configPolarityPort1[portExpander] |= PEX_PIN_POLARITY_INVERTED << (pin % PEX_NR_OF_PORTS_PER_REGISTER);
        OS_ExitTaskCritical();
    }
}

extern void PEX_SetPinPolarityRetained(uint8_t portExpander, uint8_t pin) {
    FAS_ASSERT(portExpander < PEX_NR_OF_PORT_EXPANDERS);
    FAS_ASSERT(pin <= PEX_PIN17);

    if (pin <= PEX_PIN07) {
        OS_EnterTaskCritical();
        pex_configPolarityPort0[portExpander] &= ~(((~PEX_PIN_POLARITY_RETAINED) & 0x1u) << pin);
        OS_ExitTaskCritical();
    } else { /* pin is in range PEX_PIN10 to PEX_PIN17 */
        OS_EnterTaskCritical();
        pex_configPolarityPort1[portExpander] &=
            ~(((~PEX_PIN_POLARITY_RETAINED) & 0x1u) << (pin % PEX_NR_OF_PORTS_PER_REGISTER));
        OS_ExitTaskCritical();
    }
}

/*========== Getter for static Variables (Unit Test) ========================*/
#ifdef UNITY_UNIT_TEST

#endif

/*========== Externalized Static Function Implementations (Unit Test) =======*/
