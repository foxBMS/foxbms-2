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
 * @file    mxm_1785x_tools.h
 * @author  foxBMS Team
 * @date    2020-07-15 (date of creation)
 * @updated 2021-12-06 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MXM
 *
 * @brief   This is a collection of helper functions for the MAX1785x ICs
 *
 * @details This collection of helper functions for the MAX1785x ICs helps to
 *          calculate the lsb and msb for register values and similar tasks.
 *
 */

#ifndef FOXBMS__MXM_1785X_TOOLS_H_
#define FOXBMS__MXM_1785X_TOOLS_H_

/*========== Includes =======================================================*/
#include "mxm_cfg.h"

#include "database.h"
#include "mxm_basic_defines.h"
#include "mxm_battery_management.h"
#include "mxm_register_map.h"

/*========== Macros and Definitions =========================================*/
/**
 * @brief States of the global Maxim state-machine
 *
 * State transitions are currently autonomous and will have to be integrated into
 * the Battery System state-machine.
 */
typedef enum {
    MXM_STATEMACHINE_STATES_UNINITIALIZED,       /*!< uninitialized state */
    MXM_STATEMACHINE_STATES_SELFCHECK_PRE_INIT,  /*!< self-check that has to be executed before initialization */
    MXM_STATEMACHINE_STATES_INIT,                /*!< initialization sequence */
    MXM_STATEMACHINE_STATES_SELFCHECK_POST_INIT, /*!< self-check that has to be executed after initialization */
    MXM_STATEMACHINE_STATES_IDLE,                /*!< idle state, the driver waits for operation here */
    MXM_STATEMACHINE_STATES_OPERATION,           /*!< operating state, the measurement cycle is executed here */
    MXM_STATEMACHINE_STATES_MAXSTATE,            /*!< highest state */
} MXM_STATEMACHINE_STATES_e;

/** states of the #MXM_StateMachineOperation() statemachine */
typedef enum {
    MXM_INIT_ENTRY,
    MXM_INIT_DEVCFG1,
    MXM_INIT_SET_STATUS2,
    MXM_INIT_STATUS1,
    MXM_INIT_GET_VERSION,
    MXM_INIT_GET_ID1,
    MXM_INIT_GET_ID2,
    MXM_INIT_MEASUREEN1,
    MXM_INIT_MEASUREEN2,
    MXM_INIT_AUXGPIOCFG,
    MXM_INIT_AUXTIMEREG,
    MXM_INIT_ACQCFG,
    MXM_INIT_UVTHSETREG,
    MXM_INIT_OVTHSETREG,
    MXM_INIT_BALEXP1,
    MXM_INIT_BALEXP2,
    MXM_INIT_BALEXP3,
    MXM_INIT_BALEXP4,
    MXM_INIT_BALEXP5,
    MXM_INIT_BALEXP6,
    MXM_INIT_BALEXP7,
    MXM_INIT_BALEXP8,
    MXM_INIT_BALEXP9,
    MXM_INIT_BALEXP10,
    MXM_INIT_BALEXP11,
    MXM_INIT_BALEXP12,
    MXM_INIT_BALEXP13,
    MXM_INIT_BALEXP14,
    MXM_INIT_BALSWDLY,
    MXM_INIT_ALRTOVEN,
    MXM_INIT_ALRTUVEN,
    MXM_INIT_COMPOPNTHREG,
    MXM_INIT_I2C_GET_STAT1,
    MXM_INIT_I2C_CFG,
    MXM_INIT_I2C_PNTR,
    MXM_INIT_I2C_SEND_MUX0,
    MXM_INIT_I2C_SEND_MUX1,
    MXM_INIT_GET_I2C_STAT2,
    MXM_OP_ENTRY_STATE,
    MXM_OP_DIAGNOSTIC_ENTRY,
    MXM_OP_DIAGNOSTIC_STATUS1,
    MXM_OP_DIAGNOSTIC_STATUS2,
    MXM_OP_DIAGNOSTIC_STATUS3,
    MXM_OP_DIAGNOSTIC_FMEA1,
    MXM_OP_DIAGNOSTIC_FMEA2,
    MXM_OP_DIAGNOSTIC_CLEAR_STATUS2,
    MXM_OP_DIAGNOSTIC_CLEAR_FMEA1,
    MXM_OP_DIAGNOSTIC_CLEAR_FMEA2,
    MXM_OP_DIAGNOSTIC_EXIT,
    MXM_OP_SELECT_MUX_CHANNEL,
    MXM_OP_WRITE_MUX0,
    MXM_OP_WRITE_MUX1,
    MXM_OP_SET_SCAN_STROBE,
    MXM_OP_GET_SCAN_STROBE,
    MXM_OP_GET_VOLTAGES,
    MXM_OP_GET_ALRTSUM,
    MXM_OP_PARSE_VOLTAGES_INTO_DB,
    MXM_OP_PINOPEN_ENTRY,
    MXM_OP_PINOPEN_EXIT,
    MXM_OP_PINOPEN_SET_CURRENT_SOURCES,
    MXM_OP_PINOPEN_COMPSCAN,
    MXM_OP_PINOPEN_GET_SCAN_STROBE,
    MXM_OP_PINOPEN_GET_ALRT,
    MXM_OP_PINOPEN_PROCESS_OPENWIRE,
    MXM_OP_PINOPEN_RESTORE_CURRENT_SOURCE_CONF,
    MXM_OP_BAL_ENTRY,
    MXM_OP_BAL_CONTROL_RESET_ALL,
    MXM_OP_BAL_CONTROL_SET_ALL,
    MXM_OP_BAL_START,
    MXM_OP_BAL_EXIT,
    MXM_OP_CYCLE_END_ENTRY,
    MXM_OP_INCREMENT_MUX_COUNTER,
    MXM_OP_CYCLE_END_EXIT,
} MXM_STATEMACHINE_OPERATION_STATES_e;

/** intermediate state-definition for #MXM_MonGetVoltages() */
typedef enum {
    MXM_MONITORING_STATE_PENDING, /*!< state completion is pending */
    MXM_MONITORING_STATE_PASS,    /*!< state passed successfully */
} MXM_MONITORING_STATE_e;

/**
 * struct describing the different return values of selfchecks that the driver can execute
 */
typedef struct {
    STD_RETURN_TYPE_e crc;         /*!< CRC self-check; stores the return value of #MXM_CRC8SelfTest(). */
    STD_RETURN_TYPE_e conv;        /*!< Conversion self-check; stores the return value of #MXM_ConvertTest(). */
    STD_RETURN_TYPE_e firstSetBit; /*!< First Set Bit self-check; stores the return value of #MXM_FirstSetBitTest(). */
    STD_RETURN_TYPE_e
        extractValueFromRegister; /*!< Extract value from Register self-check; stores the return value of #MXM_ExtractValueFromRegisterTest(). */
    STD_RETURN_TYPE_e
        parseVoltageReadall; /*!< Voltage parsing of READALL; stores the return value of #MXM_ParseVoltageReadallTest(). */
    STD_RETURN_TYPE_e
        addressSpaceChecker; /*!< Address space checker; stores the return value of #MXM_5XUserAccessibleAddressSpaceCheckerSelfCheck(). */
    STD_RETURN_TYPE_e
        fmeaStatusASCI; /*!< MAX17841B FMEA register; stores the value of the FMEA check conducted in the state #MXM_STATEMACH_41B_CHECK_FMEA. */
} MXM_SELFCHECK_s;

/* TODO implement filling of values */
/* TODO read and verify OTP register */

/** struct describing an entry into the monitoring registry */
typedef struct {
    bool connected;                      /*!< state variable, indicates whether monitoring IC is connected */
    uint8_t deviceAddress;               /*!< address that has been assigned during enumeration */
    MXM_MODEL_ID_e model;                /*!< model (e.g. 17853) */
    MXM_siliconVersion_e siliconVersion; /*!< silicon version of chip */
    uint32_t deviceID;                   /*!< 24-bit unique device ID */
    uint16_t registerStatus1;            /*!< content of the STATUS1 register */
    uint16_t registerStatus2;            /*!< content of the STATUS2 register */
    uint16_t registerStatus3;            /*!< content of the STATUS3 register */
    uint16_t registerFmea1;              /*!< content of the FMEA1 register */
    uint16_t registerFmea2;              /*!< content of the FMEA2 register */
} MXM_REGISTRY_ENTRY_s;

/**
 * @brief   Length of RX buffer
 * @details This define describes the length of the array
 *          #MXM_MONITORING_INSTANCE_s::rxBuffer. It is large enough to fit the largest
 *          message that can be written into a single buffer which consists of
 *          6 command bytes and #MXM_MAXIMUM_NR_OF_MODULES times 2 receive
 *          bytes for the maximum number of connected monitoring ICs.
 */
#define MXM_RX_BUFFER_LENGTH (100u)

/** struct that contains the state of the balancing subsystem */
typedef struct {
    uint8_t moduleBalancingIndex;     /*!< index of the module that is currently handled */
    bool evenCellsNeedBalancing;      /*!< indicates that even cells need balancing */
    bool oddCellsNeedBalancing;       /*!< indicates that odd cells need balancing */
    bool evenCellsBalancingProcessed; /*!< balancing of even cells has been processed */
    bool oddCellsBalancingProcessed;  /*!< balancing of odd cells has been processed */
    uint16_t cellsToBalance;          /*!< bitfield used for register BALSWCTRL, 16 bits for upt to 14 cells */
    uint32_t previousTimeStamp;       /*!< timestamp of previous balancing checkpoint */
    uint32_t currentTimeStamp;        /*!< timestamp of current balancing checkpoint */
    DATA_BLOCK_BALANCING_CONTROL_s *const pBalancingControl_table; /*!< balancing control table */
} MXM_BALANCING_STATE_s;

/** Status of the Maxim-main-state-machine. */
typedef struct {
    bool resetNecessary;             /*!< a reset of the whole driver is requested (due to an error) */
    uint8_t errorCounter;            /*!< counts the number of errors and issues a reset when the counter trips */
    uint32_t timestampLastError;     /*!< timestamp of last error counter increment; will be reset if old enough */
    MXM_STATEMACHINE_STATES_e state; /*!< state of the maxim state-machine */
    MXM_STATEMACHINE_OPERATION_STATES_e operationSubstate; /*!< substate during operation of monitoring */
    bool allowStartup;             /*!< indicates whether start of state-machine has been requested */
    bool operationRequested;       /*!< indicates whether the measurement cycle should run */
    bool firstMeasurementDone;     /*!< this bit is set after the first measurement cycle */
    bool stopRequested;            /*!< indicates that no new measurement cycles should be run */
    bool openwireRequested;        /*!< indicates that an openwire-check has been requested */
    bool undervoltageAlert;        /*!< whether undervoltage alert has occurred. TODO remove? replaced by DC? */
    uint8_t muxCounter;            /*!< counter for the currently selected mux channel */
    uint8_t diagnosticCounter;     /*!< upward counter for determining when a diagnostic cycle should be entered */
    MXM_DC_BYTE_e dcByte;          /*!< content of the data-check-byte */
    uint8_t mxmVoltageCellCounter; /*!< counter for getting all cellvoltages */
    uint8_t highest5xDevice;       /*!< address of highest monitoring device of the 5x family */
    MXM_5X_STATE_REQUEST_STATUS_e requestStatus5x;                /*!< status of request to 5x */
    MXM_5X_COMMAND_PAYLOAD_s batteryCmdBuffer;                    /*!< buffer for Battery Management Commands */
    STD_RETURN_TYPE_e resultSelfCheck;                            /*!< Status of driver-wide self-check */
    MXM_SELFCHECK_s selfCheck;                                    /*!< stores self check return values */
    MXM_BALANCING_STATE_s *const pBalancingState;                 /*!< pointer to the balancing structure */
    MXM_41B_INSTANCE_s *const pInstance41B;                       /*!< instance-pointer of the 41b-state-machine */
    MXM_5X_INSTANCE_s *const pInstance5X;                         /*!< instance-pointer of the 5x-state-machine */
    DATA_BLOCK_CELL_VOLTAGE_s *const pCellVoltages_table;         /*!< cell voltages table */
    DATA_BLOCK_CELL_TEMPERATURE_s *const pCellTemperatures_table; /*!< cell temperatures table */
    DATA_BLOCK_OPEN_WIRE_s *const pOpenwire_table;                /*!< open wire table */
    /**
     * @brief Local storage for cell-voltages
     *
     * This local variable stores the measured cell- and AUX-voltages. Independent
     * of the actual configuration, it stores the measurement values always at the
     * same place and never changes in size.
     * As an example the voltage of cell 5 (cells are numbered from 1 to 14)
     * in module 0 (modules are numbered from 0 to 31) will be in entry 4.
     * The voltage of cell 14 of module 1 will be in entry 27. This is independent
     * of the size of the setup which is defined in the defines
     * #BS_NR_OF_CELLS_PER_MODULE and #BS_NR_OF_MODULES.
     * Therefore, the size of the arrays inside this struct is defined as
     * #MXM_MAXIMUM_NR_OF_MODULES times #MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE.
     */
    MXM_DATA_STORAGE_s localVoltages;
    MXM_REGISTRY_ENTRY_s registry[MXM_MAXIMUM_NR_OF_MODULES]; /*!< registry for connected monitoring ICs */
    uint8_t rxBuffer[MXM_RX_BUFFER_LENGTH]; /*!< buffer that stores the incoming data from the lower state-machines.
        It contains the command sequence as it is received on the ASCI together with the Battery Management Protocol command. */
} MXM_MONITORING_INSTANCE_s;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
/**
 * @brief       Convert a measurement value to a voltage value.
 * @details     This function converts measurement values from the monitoring
 *              IC into a voltage value. It assumes that measurement values are
 *              spread over the LSB and MSB according to the data sheet. This
 *              means that the two lowest bits are 0 and the 14-bit measurement
 *              is spread over the 14 highest bits.
 *
 *              The conversion type describes the kind of measurement that has
 *              been executed.
 * @param[in]   lsb                     LSB of the value
 * @param[in]   msb                     MSB of the value
 * @param[out]  pTarget                 pointer in which the converted value
 *                                      shall be written
 * @param[in]   convType                type of conversion (bipolar or unipolar)
 * @param[in]   fullScaleReference_mV   reference voltage of full scale
 *                                      (for voltage measurements 5000)
 */
extern void MXM_Convert(
    uint8_t lsb,
    uint8_t msb,
    uint16_t *pTarget,
    MXM_CONVERSION_TYPE_e convType,
    uint32_t fullScaleReference_mV);

/**
 * @brief   Test the #MXM_Convert()-function.
 * @details This test-function is intended as a self-check. It tests whether
 *          the #MXM_Convert()-function computes the values as expected.
 * @return  #STD_OK on successful execution, otherwise #STD_NOT_OK
 */
extern STD_RETURN_TYPE_e must_check_return MXM_ConvertTest(void);

/**
 * @brief   Extract a value from a single register.
 * @details This function extracts a single value from a single register.
 *          The register has to be supplied as MSB and LSB. The bitmask has to
 *          be of type #MXM_REG_BM. The extracted value will be written into
 *          value.
 *
 *          The function can only extract values from single registers
 *          (MSB and LSB)and does not handle values that span multiple
 *          registers.
 * @param[in]   lsb     least significant byte of the register
 * @param[in]   msb     most significant byte of the register
 * @param[in]   bitmask bitmask for the value
 * @param[out]  pValue  pointer to a variable in which the value
 *                      should be written
 */
extern void MXM_ExtractValueFromRegister(uint8_t lsb, uint8_t msb, MXM_REG_BM bitmask, uint16_t *pValue);

/**
 * @brief   Test #MXM_ExtractValueFromRegister().
 * @details Self-check that tests whether #MXM_ExtractValueFromRegister() works
 *          as expected.
 * @return  #STD_OK if the self-check has passed successfully
 */
extern STD_RETURN_TYPE_e must_check_return MXM_ExtractValueFromRegisterTest(void);

/**
 * @brief   Test #MXM_FirstSetBit().
 * @details Tests the function #MXM_FirstSetBit() if it works as expected.
 * @return  #STD_OK if the self-check has passed successfully
 */
extern STD_RETURN_TYPE_e must_check_return MXM_FirstSetBitTest(void);

/**
 * @brief   convert a unipolar 14bit-value and shifts it into the 16bit-format
 * @details Takes a value in the unipolar 14bit format and transforms it into
 *          a 16bit register entry for the monitoring IC (separated into LSB
 *          and MSB).
 * @param[in]   inputValue  unipolar 14bit-value
 * @param[out]  lsb         LSB in 16bit-format
 * @param[out]  msb         MSB in 16bit-format
 */
extern void MXM_Unipolar14BitInto16Bit(uint16_t inputValue, uint8_t *lsb, uint8_t *msb);

/**
 * @brief   convert a voltage value into a unipolar 14bit value
 * @details Takes a voltage value in mV together with a reference for fullscale
 *          values into a unipolar 14bit value.
 * @param[in]   voltage_mV              voltage in mV
 * @param[in]   fullscaleReference_mV   value of the fullscale in mV;
 *                                      typically 5000
 * @return      unipolar 14bit value for usage with
 *              #MXM_Unipolar14BitInto16Bit()
 */
extern uint16_t MXM_VoltageIntoUnipolar14Bit(uint16_t voltage_mV, uint16_t fullscaleReference_mV);

/**
 * @brief       Get the string and module number from a global module number
 * @details     Maxim driver knows only one string internally and maps on demand
 *              this one string onto several strings (for instances where several
 *              modules are in the same daisy-chain but belong to different
 *              strings).
 *
 *              This function assumes that a string contains #BS_NR_OF_MODULES modules
 *              in #BS_NR_OF_STRINGS strings and that they are connected in
 *              a daisy-chain.
 * @param[in]   moduleNumber            number of the module in the daisy-chain (starts with 0)
 * @param[out]  pStringNumber           pointer where the string number will be written
 * @param[out]  pModuleNumberInString   module number in the string
 */
extern void MXM_ConvertModuleToString(
    const uint16_t moduleNumber,
    uint8_t *pStringNumber,
    uint16_t *pModuleNumberInString);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__MXM_1785X_TOOLS_H_ */
