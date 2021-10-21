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
 * @file    mxm_1785x.c
 * @author  foxBMS Team
 * @date    2019-01-15 (date of creation)
 * @updated 2021-07-14 (date of last update)
 * @ingroup DRIVERS
 * @prefix  MXM
 *
 * @brief   Driver for the MAX17841B ASCI and MAX1785x monitoring chip
 *
 * @details This file contains the main-state-machine that drives the
 *          monitoring ICs of the MAX1785x family by Maxim Integrated.
 *
 */

/*========== Includes =======================================================*/
#include "mxm_1785x.h"

#include "database.h"
#include "diag.h"
#include "mxm_1785x_tools.h"
#include "mxm_battery_management.h"
#include "mxm_registry.h"
#include "os.h"
#include "tsi.h"

/*========== Macros and Definitions =========================================*/

/** length of voltage-read array */
#define MXM_VOLTAGE_READ_ARRAY_LENGTH (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE + 3u)

/*========== Static Constant and Variable Definitions =======================*/
/**
 * @brief Mapping of voltage registers
 *
 * This array maps registers of the monitoring IC onto cell-numbers. The
 * register values are defined in the #MXM_REG_NAME_e enum.
 * For now the length of this array is #MXM_VOLTAGE_READ_ARRAY_LENGTH
 * as it is enabled for the measurement of all cells, two AUX-voltages and
 * one block voltage. This has to be adapted once this driver is enabled for
 * general operation.
 */
static const MXM_REG_NAME_e mxm_voltageCellAddresses[MXM_VOLTAGE_READ_ARRAY_LENGTH] = {
    MXM_REG_CELL1,
    MXM_REG_CELL2,
    MXM_REG_CELL3,
    MXM_REG_CELL4,
    MXM_REG_CELL5,
    MXM_REG_CELL6,
    MXM_REG_CELL7,
    MXM_REG_CELL8,
    MXM_REG_CELL9,
    MXM_REG_CELL10,
    MXM_REG_CELL11,
    MXM_REG_CELL12,
    MXM_REG_CELL13,
    MXM_REG_CELL14,
    MXM_REG_AUX2,
    MXM_REG_AUX3,
    MXM_REG_BLOCK,
};

/**
 * @brief Local cell voltage data block
 *
 * This local instance stores the measured cell voltages. In contrast to
 * #MXM_MONITORING_INSTANCE::localVoltages, the layout of this structure
 * changes with the defined battery-system as it is described by the
 * database_cfg.h. Mapping the values from
 * #MXM_MONITORING_INSTANCE::localVoltages to #mxm_cellVoltages and copying
 * these entries into the database is handled by #MXM_ParseVoltagesIntoDB().
 */
static DATA_BLOCK_CELL_VOLTAGE_s mxm_cellVoltages = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};

/**
 * @brief   Local cell temperature data block
 * @details This local instance stores the measured cell temperatures.
 */
static DATA_BLOCK_CELL_TEMPERATURE_s mxm_cellTemperatures = {.header.uniqueId = DATA_BLOCK_ID_CELL_TEMPERATURE_BASE};

/** @brief Local data structure for openwire results. */
static DATA_BLOCK_OPEN_WIRE_s mxm_openwire = {.header.uniqueId = DATA_BLOCK_ID_OPEN_WIRE_BASE};

/**
 * @brief   Balancing control
 * @details This variable stores information about which cells need balancing
 */
static DATA_BLOCK_BALANCING_CONTROL_s mxm_balancingControl = {.header.uniqueId = DATA_BLOCK_ID_BALANCING_CONTROL};

/**
 * @brief   Module index in the daisy chain
 * @details Used for balancing
 */
static uint8_t mxm_moduleBalancingIndex = {0};

/**
 * @brief   Even cells group needs to be balanced
 * @details Even and odd cells can't be balanced at the same time
 */
static uint8_t mxm_evenCellsNeedBalancing = {0};

/**
 * @brief   Odd cells group needs to be balanced
 * @details Even and odd cells can't be balanced at the same time
 */
static uint8_t mxm_oddCellsNeedBalancing = {0};

/**
 * @brief   Even cells group balancing done
 * @details Odd cells can be balanced now
 */
static uint8_t mxm_evenCellsBalancingProcessed = {0};

/**
 * @brief   Odd cells group balancing done
 * @details Even cells can be balanced now
 */
static uint8_t mxm_oddCellsBalancingProcessed = {0};

/**
 * @brief   Mask to control balacing
 * @details Bitfield masked with register BALSWCTRL, 16 bits => up to 14 cells
 */
uint16_t mxm_cellsToBalance = 0x00;

/** @brief Timers used to guarantee that balancing is performed periodically @{*/
static uint32_t mxm_previousTime = {0};
static uint32_t mxm_currentTime  = {0};
/**@}*/

/** @brief Delay in milliseconds before the balancing status is updated */
#define MXM_DELAY_BALANCING 10000u

/** @brief VAA reference voltage (3.3V) */
#define MXM_REF_VAA_mV (3300u)

/** @defgroup MXM_I2C_IMPLEMENTATION symbols and settings pertaining to the I2C implementation in MXM
 * @{
 */
/** @brief address of MUX0 */
#define MXM_I2C_MUX0_ADDRESS (0x4Cu)
/** @brief address of MUX1 */
#define MXM_I2C_MUX1_ADDRESS (0x4Du)
/**@}*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/** @brief Retrieves data from lower statemachine and writes it to the rx buffer. */
static void MXM_GetDataFrom5XStateMachine(MXM_MONITORING_INSTANCE_s *pInstance);

/**
 * @brief       Parse voltage values from a READALL receive buffer into an
 *              array.
 * @details     This function expects a preprocessed RX buffer with an answer
 *              to a READALL command. It will parse the data bytes from this
 *              message into an array structured like mxm_local_cellvoltages.
 *              The offset of the measured cell has to be defined. For example
 *              cell 1 has an offset of 0 and cell 4 has an offset of 3. The
 *              offset for the READALL command is always defined in reference
 *              to module 0.
 *
 *              If is_temperature_measurement is set to true, the function
 *              expects an auxiliary measurement (e.g. temperatures).
 * @param[in]   kpkVoltRxBuffer         array-pointer to the RX buffer
 * @param[in]   voltRxBufferLength      length of the RX buffer
 * @param[in]   measurementOffset       offset of the data in the cell voltage
 *                                      array (target)
 * @param[in]   conversionType          type of conversion that has been used
 *                                      for the measured data
 * @param[out]  pVoltagesTarget         array-pointer to the cell voltages
 *                                      array
 * @param[in]   meas_type               whether the measurement is temperature
 *                                      or cell
 * @param[in]   full_scale_reference_mV reference voltage of full scale
 */
static void MXM_ParseVoltageLineReadall(
    const uint8_t *const kpkVoltRxBuffer,
    uint16_t voltRxBufferLength,
    uint8_t measurementOffset,
    MXM_CONVERSION_TYPE_e conversionType,
    uint16_t *pVoltagesTarget,
    MXM_MEASURE_TYPE_e meas_type,
    uint32_t full_scale_reference_mV);

/**
 * @brief       Parse a RX buffer containing voltage values.
 * @details     This function parses a RX buffer containing the answer to a
 *              READALL command. It will check whether the message contains a
 *              READALL command and whether a supported register has been
 *              queried.
 *              Depending on the queried register, it will pass the proper
 *              parameters to #MXM_ParseVoltageLineReadall(). The contained
 *              data from this message will be parsed into a struct structured
 *              like #MXM_MONITORING_INSTANCE::localVoltages.
 * @param[in]   kpkVoltageRxBuffer      array-pointer to the RX buffer
 * @param[in]   voltageRxBufferLength   length of the RX buffer
 * @param[out]  datastorage             contains all measured voltages for local
 *                                      consumption in the module
 * @param[in]   conversionType          type of conversion that has been used for
 *                                      the measured data
 * @return      #STD_NOT_OK in the case that the RX buffer does not contain a
 *              valid message or the conversion-type is unknown,
 *              otherwise #STD_OK
 */
static STD_RETURN_TYPE_e MXM_ParseVoltageReadall(
    const uint8_t *const kpkVoltageRxBuffer,
    uint16_t voltageRxBufferLength,
    MXM_DATA_STORAGE_s *datastorage,
    MXM_CONVERSION_TYPE_e conversionType);

/**
 * @brief   Test the #MXM_ParseVoltageReadall()-function
 * @details Test the function #MXM_ParseVoltageReadall() by passing predefined
 *          RX buffer to it and checking the outcome. This function writes to
 *          the variable #MXM_MONITORING_INSTANCE::localVoltages and nulls it
 *          completely after execution. It is intended as a self-check that can
 *          be performed during startup of the driver.
 * @return  #STD_OK if the self-check has been performed successfully,
 *          otherwise #STD_NOT_OK
 */
static STD_RETURN_TYPE_e must_check_return MXM_ParseVoltageReadallTest(MXM_MONITORING_INSTANCE_s *pInstance);

/**
 * @brief           Encapsulation for reading voltages from a register
 * @details         This function encapsulates the request of state-changes and
 *                  following conversion for the reading of an arbitrary
 *                  measurement voltage of the daisy-chain. Its parameters are
 *                  a variable for tracking the state of the underlying
 *                  state-machines and the register address that has to be
 *                  queried. It returns whether the action has been successful
 *                  or not.
 *                  In order to obtain all cell voltages this function has to
 *                  be called for every relevant register address.
 * @param[in,out]   pState      pointer to the state-machine struct
 * @param[in]       regAddress  register address that shall be queried
 * @return          current state of the action:
 *                      - MXM_MON_STATE_PASS upon completion
 *                      - MXM_MON_STATE_PENDING as long as the action is
 *                        ongoing
 *                      - MXM_MON_STATE_FAIL if the function failed and could
 *                        not recover on its own
 */
static MXM_MONINTORING_STATE_e must_check_return
    MXM_MonGetVoltages(MXM_MONITORING_INSTANCE_s *pState, MXM_REG_NAME_e regAddress);

/**
 * @brief   Copies measured voltage data into the database.
 * @details This function copies the acquired voltage data from
 *          #MXM_MONITORING_INSTANCE::localVoltages into the database-struct mxm_cellVoltages and
 *          copies this struct into the database. This action is required due
 *          to different data layouts. This driver always stores its
 *          cell-voltages in an array with 14*32 = 448 cells in order to reduce
 *          the amount of different configurations and variants.
 *
 *          This function maps these values into the database-struct which
 *          scales with the number of connected cells and monitoring ICs.
 * @param[in]   kpkInstance pointer to the #MXM_MONITORING_INSTANCE_s struct
 * @return  #STD_OK if the action was successful or #STD_NOT_OK otherwise
 */
static STD_RETURN_TYPE_e MXM_ParseVoltagesIntoDB(const MXM_MONITORING_INSTANCE_s *const kpkInstance);

/**
 * @brief           Execute all preinit selfchecks.
 * @details         Executes the following self-checks:
 *                      - #MXM_CRC8SelfTest()
 *                      - #MXM_ConvertTest()
 *                      - #MXM_FirstSetBitTest()
 *                      - #MXM_ExtractValueFromRegisterTest()
 *                      - #MXM_ParseVoltageReadallTest()
 *                      - #MXM_5XUserAccessibleAddressSpaceCheckerSelfCheck()
 *
 *                  These self-checks do not need an initialized daisy-chain
 *                  and can therefore be executed at startup of the
 *                  state-machine.
 *
 *                  After execution of each test, the return value is stored in
 *                  the supplied state-struct. The function returns whether the
 *                  self-check has successfully passed.
 * @param[in,out]   pState  pointer to the state-struct, will write status into
 * @return          #STD_OK on success, #STD_NOT_OK on failure,
 *                  return value has to be used
 */
static STD_RETURN_TYPE_e must_check_return MXM_PreInitSelfCheck(MXM_MONITORING_INSTANCE_s *pState);

/**
 * @brief           State-Machine implementation for operation state
 * @details         This state-machine contains the "program" with which the
 *                  connected monitoring satellites are controlled. It is
 *                  entered by #MXM_StateMachine() once the daisy-chain has
 *                  been initialized and is in operation state.
 * @param[in,out]   pState  used as both input and output (stores
 *                          state-information, requests and intermediate values)
 */
static void MXM_StateMachineOperation(MXM_MONITORING_INSTANCE_s *pState);

/**
 * @brief           Fill the balancing datastructure
 * @details         This function fills the data-structure that describes
 *                  which balancing channels of the monitoring ICs should be
 *                  activated.
 * @return          #STD_NOT_OK in case of invalid access
 */
static STD_RETURN_TYPE_e MXM_ConstructBalancingBuffer(void);

/**
 * @brief           Handle the statemachine-transactions for a WRITEALL
 * @details         Before calling this function, update the command buffer of
 *                  the state-variable. Then call this function and pass on the
 *                  state-variable and the next state. The function will
 *                  handle the communication with the lower state-machine and
 *                  will transition into the next state, if the command has
 *                  been sent successfully.
 * @param[in,out]   pInstance   pointer to instance of the mxm
 *                              monitoring state-machine
 * @param[in]       nextState   state that should be entered upon successful
 *                              completion
 */
static void MXM_HandleStateWriteall(
    MXM_MONITORING_INSTANCE_s *pInstance,
    MXM_STATEMACHINE_OPERATION_STATES_e nextState);

/**
 * @brief           Handle the statemachine-transactions for a READALL
 * @details         Call this function and pass on the state-variable, the
 *                  register to be read and the next state. The function will
 *                  handle the communication with the lower state-machine and
 *                  will transition into the next state, if the command has
 *                  been sent successfully. Moreover it will return true when
 *                  transitioning. The return value has to be checked and used
 *                  to execute additional code if necessary.
 * @param[in,out]   pInstance   pointer to instance of the mxm
 *                              monitoring state-machine
 * @param[in]       registerName    register that should be read
 * @param[in]       nextState   state that should be entered upon successful
 *                              completion
 * @return          true when the state has been handled, false otherwise, use
 *                  this to execute additional code when the message has been
 *                  read.
 */
static bool must_check_return MXM_HandleStateReadall(
    MXM_MONITORING_INSTANCE_s *pInstance,
    MXM_REG_NAME_e registerName,
    MXM_STATEMACHINE_OPERATION_STATES_e nextState);

/**
 * @brief           Processes the retrieved information on openwire
 * @details         Parses through a retrieved RX buffer and writes into the
 *                  database.
 * @param[in,out]   kpkInstance     pointer to instance of the Maxim monitoring
 *                                  state-machine
 * @param[in,out]   pDataOpenWire   pointer to the local copy of the data-base
 *                                  entry
 */
static void MXM_ProcessOpenWire(
    const MXM_MONITORING_INSTANCE_s *const kpkInstance,
    DATA_BLOCK_OPEN_WIRE_s *pDataOpenWire);

/*========== Static Function Implementations ================================*/
static void MXM_GetDataFrom5XStateMachine(MXM_MONITORING_INSTANCE_s *pInstance) {
    MXM_5XGetRXBuffer(pInstance->pInstance5X, pInstance->rxBuffer, MXM_RX_BUFFER_LENGTH);
    pInstance->dcByte = MXM_5XGetLastDCByte(pInstance->pInstance5X);
}

static void MXM_HandleStateWriteall(
    MXM_MONITORING_INSTANCE_s *pInstance,
    MXM_STATEMACHINE_OPERATION_STATES_e nextState) {
    FAS_ASSERT(pInstance != NULL_PTR);

    if (pInstance->requestStatus5x == MXM_5X_STATE_UNSENT) {
        MXM_5XSetStateRequest(
            pInstance->pInstance5X,
            MXM_STATEMACH_5X_WRITEALL,
            pInstance->batteryCmdBuffer,
            &pInstance->requestStatus5x);
    } else if (pInstance->requestStatus5x == MXM_5X_STATE_UNPROCESSED) {
        /* wait for processing */
    } else if (pInstance->requestStatus5x == MXM_5X_STATE_PROCESSED) {
        /* continue to the substate specified by the request */
        pInstance->operationSubstate = nextState;
        pInstance->requestStatus5x   = MXM_5X_STATE_UNSENT;
    } else if (pInstance->requestStatus5x == MXM_5X_STATE_ERROR) {
        /* default-behavior: retry */
        pInstance->requestStatus5x = MXM_5X_STATE_UNSENT;
    } else {
        /* invalid value, trap */
        FAS_ASSERT(FAS_TRAP);
    }
}

static bool must_check_return MXM_HandleStateReadall(
    MXM_MONITORING_INSTANCE_s *pInstance,
    MXM_REG_NAME_e registerName,
    MXM_STATEMACHINE_OPERATION_STATES_e nextState) {
    FAS_ASSERT(pInstance != NULL_PTR);

    bool retval = false;
    if (pInstance->requestStatus5x == MXM_5X_STATE_UNSENT) {
        pInstance->batteryCmdBuffer.regAddress = registerName;
        MXM_5XSetStateRequest(
            pInstance->pInstance5X, MXM_STATEMACH_5X_READALL, pInstance->batteryCmdBuffer, &pInstance->requestStatus5x);
    } else if (pInstance->requestStatus5x == MXM_5X_STATE_PROCESSED) {
        MXM_GetDataFrom5XStateMachine(pInstance);
        pInstance->operationSubstate = nextState;
        pInstance->requestStatus5x   = MXM_5X_STATE_UNSENT;
        retval                       = true;
    } else if (pInstance->requestStatus5x == MXM_5X_STATE_UNPROCESSED) {
        /* do nothing */
    } else if (pInstance->requestStatus5x == MXM_5X_STATE_ERROR) {
        /* try to reset state */
        pInstance->requestStatus5x = MXM_5X_STATE_UNSENT;
    } else {
        /* invalid state */
        FAS_ASSERT(FAS_TRAP);
    }

    return retval;
}

static void MXM_ProcessOpenWire(
    const MXM_MONITORING_INSTANCE_s *const kpkInstance,
    DATA_BLOCK_OPEN_WIRE_s *pDataOpenWire) {
    FAS_ASSERT(kpkInstance != NULL_PTR);
    FAS_ASSERT(pDataOpenWire != NULL_PTR);

    uint8_t numberOfSatellites = MXM_5XGetNumberOfSatellites(kpkInstance->pInstance5X);
    uint8_t msg_len            = BATTERY_MANAGEMENT_TX_LENGTH_READALL + (2u * numberOfSatellites);
    /* step over every byte-tuple in the RX-buffer */
    for (uint8_t i_po = 2u; i_po < (msg_len - 2u); i_po = i_po + 2u) {
        /* calculate current module number (first in RX-buffer is last in line) */
        uint8_t calculatedModuleNumberInDaisyChain = numberOfSatellites - ((uint16_t)(i_po / 2u) - 1u) - 1u;
        uint8_t stringNumber                       = 0u;
        uint16_t moduleNumber                      = 0u;
        MXM_ConvertModuleToString(calculatedModuleNumberInDaisyChain, &stringNumber, &moduleNumber);
        uint16_t calculated_module_position = moduleNumber * MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE;

        /* step over every cell in the module and update the openwire struct accordingly */
        for (uint8_t c = 0u; c < MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE; c++) {
            if (c < 8u) {
                /* cell numbers under 8 can be found in the LSB */
                uint8_t mask = 1u << c;
                if ((uint8_t)(mask & kpkInstance->rxBuffer[i_po]) > 0u) {
                    pDataOpenWire->openwire[stringNumber][calculated_module_position + c] = 1;
                } else {
                    pDataOpenWire->openwire[stringNumber][calculated_module_position + c] = 0;
                }
            } else {
                /* cell numbers over or equal 8 can be found in the MSB */
                uint8_t mask = 1u << (c - 8u);
                if ((uint8_t)(mask & kpkInstance->rxBuffer[i_po + 1u]) > 0u) {
                    pDataOpenWire->openwire[stringNumber][calculated_module_position + c] = 1;
                } else {
                    pDataOpenWire->openwire[stringNumber][calculated_module_position + c] = 0;
                }
            }
        }
    }

    /* write database block */
    DATA_WRITE_DATA(pDataOpenWire);
}

static STD_RETURN_TYPE_e MXM_ConstructBalancingBuffer(void) {
    STD_RETURN_TYPE_e retval = STD_OK;

    /* Re-Initialize the cells to balance at each iteration */
    mxm_cellsToBalance = 0u;

    /*  /!\ WARNING
        In this function, EVEN cells refer to the even cells IN THE REAL MODULE, and NOT in the database index.
        --> EVEN cells in a module = ODD database index
        --> ODD cells in a module = EVEN database index
        e.g. :  Cell 2 in the module corresponds to index 1 in the database
                the cell index is even, but the database index is odd. */

    /* Iterate over all the cells of the module 'mxm_moduleBalancingIndex' in a daisy-chain */
    if (mxm_moduleBalancingIndex < MXM_MAXIMUM_NR_OF_MODULES) {
        for (uint8_t cell_index = 0; cell_index < BS_NR_OF_CELLS_PER_MODULE; cell_index++) {
            if (cell_index < MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE) {
                /* Determine the position of the cell 'cell_index' of module 'mxm_moduleBalancingIndex' in the DB */
                uint8_t stringNumber  = 0u;
                uint16_t moduleNumber = 0u;
                MXM_ConvertModuleToString(mxm_moduleBalancingIndex, &stringNumber, &moduleNumber);
                uint16_t db_index = (moduleNumber * BS_NR_OF_CELLS_PER_MODULE) + cell_index;
                if (mxm_balancingControl.balancingState[stringNumber][db_index] == 1u) {
                    /* Cell 'cell_index' of module 'mxm_moduleBalancingIndex' needs to be balanced.
                       Need to determine the balancing order --> even or odd cells?
                       If the balancing order has not been determined before, need to do it. */
                    if ((mxm_evenCellsNeedBalancing == 0u) && (mxm_oddCellsNeedBalancing == 0u)) {
                        if (((cell_index % 2u) != 0u) && (mxm_evenCellsBalancingProcessed == 0u)) {
                            mxm_evenCellsNeedBalancing = 1u;
                        } else if (((cell_index % 2u) == 0u) && (mxm_oddCellsBalancingProcessed == 0u)) {
                            mxm_oddCellsNeedBalancing = 1u;
                        }
                    }
                    /* If the even cells need to be balanced */
                    if ((mxm_evenCellsNeedBalancing == 1u) && ((cell_index % 2u) != 0u) &&
                        (mxm_evenCellsBalancingProcessed == 0u)) {
                        mxm_cellsToBalance |= (0x01u << cell_index);
                    }
                    /* If the odd cells need to be balanced */
                    else if (
                        (mxm_oddCellsNeedBalancing == 1u) && ((cell_index % 2u) == 0u) &&
                        (mxm_oddCellsBalancingProcessed == 0u)) {
                        mxm_cellsToBalance |= (0x01u << cell_index);
                    }
                }
            } else {
                retval = STD_NOT_OK;
            }
        }
    } else {
        retval = STD_NOT_OK;
    }
    return retval;
}

static void MXM_ParseVoltageLineReadall(
    const uint8_t *const kpkVoltRxBuffer,
    uint16_t voltRxBufferLength,
    uint8_t measurementOffset,
    MXM_CONVERSION_TYPE_e conversionType,
    uint16_t *pVoltagesTarget,
    MXM_MEASURE_TYPE_e meas_type,
    uint32_t full_scale_reference_mV) {
    uint8_t number_of_connected_devices =
        (voltRxBufferLength - 2u - 2u) /
        2u; /*!< buffer-length - length of start - length of end divided by two (LSB and MSB) */
    /* TODO impact of alive counter on rxBufferLength
     * otherwise offset at the end of message is currently 2 (DATACHECKBYTE and CRC) */
    for (uint8_t i = 2u; i < (voltRxBufferLength - 2u); i = i + 2u) {
        uint8_t calculated_module_number    = number_of_connected_devices - ((uint16_t)(i / 2u) - 1u) - 1u;
        uint16_t calculated_module_position = 0u;
        switch (meas_type) {
            case MXM_MEASURE_TEMP:
                calculated_module_position = calculated_module_number * MXM_MAXIMUM_NR_OF_AUX_PER_MODULE;
                break;
            case MXM_MEASURE_CELL_VOLTAGE:
                calculated_module_position = calculated_module_number * MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE;
                break;
            case MXM_MEASURE_BLOCK_VOLTAGE:
                calculated_module_position = calculated_module_number;
                break;
            default:
                FAS_ASSERT(FAS_TRAP);
                break;
        }
        uint16_t calculated_array_position = calculated_module_position + measurementOffset;

        /* check calculated array position */
        switch (meas_type) {
            case MXM_MEASURE_TEMP:
                FAS_ASSERT(calculated_array_position < (MXM_MAXIMUM_NR_OF_MODULES * MXM_MAXIMUM_NR_OF_AUX_PER_MODULE));
                break;
            case MXM_MEASURE_CELL_VOLTAGE:
                FAS_ASSERT(
                    calculated_array_position < (MXM_MAXIMUM_NR_OF_MODULES * MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE));
                break;
            case MXM_MEASURE_BLOCK_VOLTAGE:
                FAS_ASSERT(calculated_array_position < (MXM_MAXIMUM_NR_OF_MODULES));
                break;
            default:
                FAS_ASSERT(FAS_TRAP);
                break;
        }

        MXM_Convert(
            kpkVoltRxBuffer[i],
            kpkVoltRxBuffer[i + 1u],
            &pVoltagesTarget[calculated_array_position],
            conversionType,
            full_scale_reference_mV);
    }
}

static STD_RETURN_TYPE_e MXM_ParseVoltageReadall(
    const uint8_t *const kpkVoltageRxBuffer,
    uint16_t voltageRxBufferLength,
    MXM_DATA_STORAGE_s *datastorage,
    MXM_CONVERSION_TYPE_e conversionType) {
    STD_RETURN_TYPE_e retval = STD_OK;
    FAS_ASSERT(kpkVoltageRxBuffer != NULL_PTR);
    FAS_ASSERT(datastorage != NULL_PTR);

    uint8_t cell_offset = 0;
    if (kpkVoltageRxBuffer[0] != BATTERY_MANAGEMENT_READALL) {
        /* rxBuffer does not contain a READALL command */
        retval = STD_NOT_OK;
    } else if ((voltageRxBufferLength % 2u) != 0u) {
        /* without alive counter rx-buffer always should be of even length */
        /* TODO impact of alive-counter-byte */
        retval = STD_NOT_OK;
    } else if (
        (conversionType != MXM_CONVERSION_UNIPOLAR) && (conversionType != MXM_CONVERSION_BIPOLAR) &&
        (conversionType != MXM_CONVERSION_BLOCK_VOLTAGE)) {
        /* conversion type is not supported */
        retval = STD_NOT_OK;
    } else {
        switch ((MXM_REG_NAME_e)kpkVoltageRxBuffer[1]) {
            case MXM_REG_CELL1:
                cell_offset = 0u;
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    cell_offset,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    5000u);
                break;
            case MXM_REG_CELL2:
                cell_offset = 1u;
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    cell_offset,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    5000u);
                break;
            case MXM_REG_CELL3:
                cell_offset = 2u;
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    cell_offset,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    5000u);
                break;
            case MXM_REG_CELL4:
                cell_offset = 3u;
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    cell_offset,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    5000u);
                break;
            case MXM_REG_CELL5:
                cell_offset = 4u;
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    cell_offset,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    5000u);
                break;
            case MXM_REG_CELL6:
                cell_offset = 5u;
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    cell_offset,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    5000u);
                break;
            case MXM_REG_CELL7:
                cell_offset = 6u;
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    cell_offset,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    5000u);
                break;
            case MXM_REG_CELL8:
                cell_offset = 7u;
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    cell_offset,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    5000u);
                break;
            case MXM_REG_CELL9:
                cell_offset = 8u;
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    cell_offset,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    5000u);
                break;
            case MXM_REG_CELL10:
                cell_offset = 9u;
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    cell_offset,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    5000u);
                break;
            case MXM_REG_CELL11:
                cell_offset = 10u;
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    cell_offset,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    5000u);
                break;
            case MXM_REG_CELL12:
                cell_offset = 11u;
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    cell_offset,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    5000u);
                break;
            case MXM_REG_CELL13:
                cell_offset = 12u;
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    cell_offset,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    5000u);
                break;
            case MXM_REG_CELL14:
                cell_offset = 13u;
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    cell_offset,
                    conversionType,
                    datastorage->cellVoltages_mV,
                    MXM_MEASURE_CELL_VOLTAGE,
                    5000u);
                break;
            case MXM_REG_AUX0:
                cell_offset = 0u;
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    cell_offset,
                    conversionType,
                    datastorage->auxVoltages_mV,
                    MXM_MEASURE_TEMP,
                    MXM_REF_VAA_mV);
                break;
            case MXM_REG_AUX2:
                cell_offset = 2u;
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    cell_offset,
                    conversionType,
                    datastorage->auxVoltages_mV,
                    MXM_MEASURE_TEMP,
                    MXM_REF_VAA_mV);
                break;
            case MXM_REG_AUX3:
                cell_offset = 3u;
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    cell_offset,
                    conversionType,
                    datastorage->auxVoltages_mV,
                    MXM_MEASURE_TEMP,
                    MXM_REF_VAA_mV);
                break;
            case MXM_REG_BLOCK:
                cell_offset = 0u;
                MXM_ParseVoltageLineReadall(
                    kpkVoltageRxBuffer,
                    voltageRxBufferLength,
                    cell_offset,
                    conversionType,
                    datastorage->blockVoltages,
                    MXM_MEASURE_BLOCK_VOLTAGE,
                    65000u);
                /* TODO scaling and variable size  (65000)*/
                break;
            default:
                /* the read register is not a valid cell register */
                retval = STD_NOT_OK;
                break;
        }
    }
    return retval;
}

static STD_RETURN_TYPE_e must_check_return MXM_ParseVoltageReadallTest(MXM_MONITORING_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);
    STD_RETURN_TYPE_e retval = STD_OK;

    uint8_t test_buffer[100]  = {0};
    uint16_t testBufferLength = 100;

    /* not a readall buffer */
    test_buffer[0] = BATTERY_MANAGEMENT_HELLOALL;
    if (MXM_ParseVoltageReadall(test_buffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_NOT_OK) {
        retval = STD_NOT_OK;
    }

    /* not a cell voltage register */
    test_buffer[0] = BATTERY_MANAGEMENT_READALL;
    test_buffer[1] = MXM_REG_VERSION;
    if (MXM_ParseVoltageReadall(test_buffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_NOT_OK) {
        retval = STD_NOT_OK;
    }

    /* bogus conversion type */
    test_buffer[0] = BATTERY_MANAGEMENT_READALL;
    test_buffer[1] = MXM_REG_CELL1;
    if (MXM_ParseVoltageReadall(test_buffer, testBufferLength, &pInstance->localVoltages, (MXM_CONVERSION_TYPE_e)42) !=
        STD_NOT_OK) {
        retval = STD_NOT_OK;
    }

    /* not an even length of rxBuffer */
    test_buffer[0]   = BATTERY_MANAGEMENT_READALL;
    test_buffer[1]   = MXM_REG_CELL1;
    testBufferLength = 5;
    if (MXM_ParseVoltageReadall(test_buffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_NOT_OK) {
        retval = STD_NOT_OK;
    }

    /* test data for CELL1REG */
    test_buffer[0]   = BATTERY_MANAGEMENT_READALL;
    test_buffer[1]   = MXM_REG_CELL1;
    test_buffer[2]   = 0xFCu;
    test_buffer[3]   = 0xFFu;
    test_buffer[4]   = 0x00u;
    test_buffer[5]   = 0x00u;
    test_buffer[6]   = 0xFCu;
    test_buffer[7]   = 0xFFu;
    test_buffer[8]   = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    test_buffer[9]   = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 10u;
    if (MXM_ParseVoltageReadall(test_buffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        retval = STD_NOT_OK;
    } else {
        if ((pInstance->localVoltages.cellVoltages_mV[0] != 5000u) ||
            (pInstance->localVoltages.cellVoltages_mV[14] != 0u) ||
            (pInstance->localVoltages.cellVoltages_mV[28] != 5000u)) {
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_0 = 0; i_0 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_0++) {
        pInstance->localVoltages.cellVoltages_mV[i_0] = 0;
    }

    /* test data for CELL2REG */
    test_buffer[0]   = BATTERY_MANAGEMENT_READALL;
    test_buffer[1]   = MXM_REG_CELL2;
    test_buffer[2]   = 0xFCu;
    test_buffer[3]   = 0xFFu;
    test_buffer[4]   = 0x00u;
    test_buffer[5]   = 0x00u;
    test_buffer[6]   = 0xFCu;
    test_buffer[7]   = 0xFFu;
    test_buffer[8]   = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    test_buffer[9]   = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 10;
    if (MXM_ParseVoltageReadall(test_buffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        retval = STD_NOT_OK;
    } else {
        if ((pInstance->localVoltages.cellVoltages_mV[1] != 5000u) ||
            (pInstance->localVoltages.cellVoltages_mV[15] != 0u) ||
            (pInstance->localVoltages.cellVoltages_mV[29] != 5000u)) {
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_1 = 0; i_1 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_1++) {
        pInstance->localVoltages.cellVoltages_mV[i_1] = 0;
    }

    /* test data for CELL3REG */
    test_buffer[0]   = BATTERY_MANAGEMENT_READALL;
    test_buffer[1]   = MXM_REG_CELL3;
    test_buffer[2]   = 0xFCu;
    test_buffer[3]   = 0xFFu;
    test_buffer[4]   = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    test_buffer[5]   = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(test_buffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[2] != 5000u) {
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_2 = 0; i_2 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_2++) {
        pInstance->localVoltages.cellVoltages_mV[i_2] = 0;
    }

    /* test data for CELL4REG */
    test_buffer[0]   = BATTERY_MANAGEMENT_READALL;
    test_buffer[1]   = MXM_REG_CELL4;
    test_buffer[2]   = 0xFCu;
    test_buffer[3]   = 0xFFu;
    test_buffer[4]   = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    test_buffer[5]   = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(test_buffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[3] != 5000u) {
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_3 = 0; i_3 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_3++) {
        pInstance->localVoltages.cellVoltages_mV[i_3] = 0;
    }

    /* test data for CELL5REG */
    test_buffer[0]   = BATTERY_MANAGEMENT_READALL;
    test_buffer[1]   = MXM_REG_CELL5;
    test_buffer[2]   = 0xFCu;
    test_buffer[3]   = 0xFFu;
    test_buffer[4]   = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    test_buffer[5]   = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(test_buffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[4] != 5000u) {
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_4 = 0; i_4 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_4++) {
        pInstance->localVoltages.cellVoltages_mV[i_4] = 0;
    }

    /* test data for CELL6REG */
    test_buffer[0]   = BATTERY_MANAGEMENT_READALL;
    test_buffer[1]   = MXM_REG_CELL6;
    test_buffer[2]   = 0xFCu;
    test_buffer[3]   = 0xFFu;
    test_buffer[4]   = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    test_buffer[5]   = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(test_buffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[5] != 5000u) {
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_5 = 0; i_5 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_5++) {
        pInstance->localVoltages.cellVoltages_mV[i_5] = 0;
    }

    /* test data for CELL7REG */
    test_buffer[0]   = BATTERY_MANAGEMENT_READALL;
    test_buffer[1]   = MXM_REG_CELL7;
    test_buffer[2]   = 0xFCu;
    test_buffer[3]   = 0xFFu;
    test_buffer[4]   = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    test_buffer[5]   = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(test_buffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[6] != 5000u) {
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_6 = 0; i_6 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_6++) {
        pInstance->localVoltages.cellVoltages_mV[i_6] = 0;
    }

    /* test data for CELL8REG */
    test_buffer[0]   = BATTERY_MANAGEMENT_READALL;
    test_buffer[1]   = MXM_REG_CELL8;
    test_buffer[2]   = 0xFCu;
    test_buffer[3]   = 0xFFu;
    test_buffer[4]   = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    test_buffer[5]   = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(test_buffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[7] != 5000u) {
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_7 = 0; i_7 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_7++) {
        pInstance->localVoltages.cellVoltages_mV[i_7] = 0;
    }

    /* test data for CELL9REG */
    test_buffer[0]   = BATTERY_MANAGEMENT_READALL;
    test_buffer[1]   = MXM_REG_CELL9;
    test_buffer[2]   = 0xFCu;
    test_buffer[3]   = 0xFFu;
    test_buffer[4]   = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    test_buffer[5]   = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(test_buffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[8] != 5000u) {
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_8 = 0; i_8 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_8++) {
        pInstance->localVoltages.cellVoltages_mV[i_8] = 0;
    }

    /* test data for CELL10REG */
    test_buffer[0]   = BATTERY_MANAGEMENT_READALL;
    test_buffer[1]   = MXM_REG_CELL10;
    test_buffer[2]   = 0xFCu;
    test_buffer[3]   = 0xFFu;
    test_buffer[4]   = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    test_buffer[5]   = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(test_buffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[9] != 5000u) {
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_9 = 0; i_9 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_9++) {
        pInstance->localVoltages.cellVoltages_mV[i_9] = 0;
    }

    /* test data for CELL11REG */
    test_buffer[0]   = BATTERY_MANAGEMENT_READALL;
    test_buffer[1]   = MXM_REG_CELL11;
    test_buffer[2]   = 0xFCu;
    test_buffer[3]   = 0xFFu;
    test_buffer[4]   = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    test_buffer[5]   = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(test_buffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[10] != 5000u) {
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_10 = 0; i_10 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_10++) {
        pInstance->localVoltages.cellVoltages_mV[i_10] = 0;
    }

    /* test data for CELL12REG */
    test_buffer[0]   = BATTERY_MANAGEMENT_READALL;
    test_buffer[1]   = MXM_REG_CELL12;
    test_buffer[2]   = 0xFCu;
    test_buffer[3]   = 0xFFu;
    test_buffer[4]   = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    test_buffer[5]   = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(test_buffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[11] != 5000u) {
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_11 = 0; i_11 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_11++) {
        pInstance->localVoltages.cellVoltages_mV[i_11] = 0;
    }

    /* test data for CELL13REG */
    test_buffer[0]   = BATTERY_MANAGEMENT_READALL;
    test_buffer[1]   = MXM_REG_CELL13;
    test_buffer[2]   = 0xFCu;
    test_buffer[3]   = 0xFFu;
    test_buffer[4]   = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    test_buffer[5]   = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(test_buffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[12] != 5000u) {
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_12 = 0; i_12 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_12++) {
        pInstance->localVoltages.cellVoltages_mV[i_12] = 0;
    }

    /* test data for CELL14REG */
    test_buffer[0]   = BATTERY_MANAGEMENT_READALL;
    test_buffer[1]   = MXM_REG_CELL14;
    test_buffer[2]   = 0xFCu;
    test_buffer[3]   = 0xFFu;
    test_buffer[4]   = 0x42u; /* DATACHECKBYTE, irrelevant for function, filled with dummy bytes */
    test_buffer[5]   = 0x44u; /* CRCBYTE, irrelevant for function, filled with dummy bytes */
    testBufferLength = 6;
    if (MXM_ParseVoltageReadall(test_buffer, testBufferLength, &pInstance->localVoltages, MXM_CONVERSION_UNIPOLAR) !=
        STD_OK) {
        retval = STD_NOT_OK;
    } else {
        if (pInstance->localVoltages.cellVoltages_mV[13] != 5000u) {
            retval = STD_NOT_OK;
        }
    }

    /* null mxm_local_cellvoltages */
    for (uint16_t i_13 = 0; i_13 < (MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE * MXM_MAXIMUM_NR_OF_MODULES); i_13++) {
        pInstance->localVoltages.cellVoltages_mV[i_13] = 0;
    }

    return retval;
}

static MXM_MONINTORING_STATE_e must_check_return
    MXM_MonGetVoltages(MXM_MONITORING_INSTANCE_s *pState, MXM_REG_NAME_e regAddress) {
    FAS_ASSERT(pState != NULL_PTR);
    MXM_MONINTORING_STATE_e retval = MXM_MONITORING_STATE_PENDING;
    if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
        pState->batteryCmdBuffer.regAddress = regAddress;
        MXM_5XSetStateRequest(
            pState->pInstance5X, MXM_STATEMACH_5X_READALL, pState->batteryCmdBuffer, &pState->requestStatus5x);
    } else if (pState->requestStatus5x == MXM_5X_STATE_PROCESSED) {
        MXM_GetDataFrom5XStateMachine(pState);
        uint8_t temp_len = BATTERY_MANAGEMENT_TX_LENGTH_READALL +
                           (2u * MXM_5XGetNumberOfSatellites(pState->pInstance5X));
        if (regAddress == MXM_REG_BLOCK) {
            MXM_ParseVoltageReadall(pState->rxBuffer, temp_len, &pState->localVoltages, MXM_CONVERSION_BLOCK_VOLTAGE);
        } else {
            MXM_ParseVoltageReadall(pState->rxBuffer, temp_len, &pState->localVoltages, MXM_CONVERSION_UNIPOLAR);
        }
        pState->requestStatus5x = MXM_5X_STATE_UNSENT;
        retval                  = MXM_MONITORING_STATE_PASS;
    } else if (pState->requestStatus5x == MXM_5X_STATE_UNPROCESSED) {
        /* do nothing */
    } else if (pState->requestStatus5x == MXM_5X_STATE_ERROR) {
        /* try to reset state */
        pState->requestStatus5x = MXM_5X_STATE_UNSENT;
    } else {
        /* invalid state */
        FAS_ASSERT(FAS_TRAP);
    }
    return retval;
}

static STD_RETURN_TYPE_e MXM_ParseVoltagesIntoDB(const MXM_MONITORING_INSTANCE_s *const kpkInstance) {
    FAS_ASSERT(kpkInstance != NULL_PTR);
    STD_RETURN_TYPE_e retval = STD_OK;

    /* voltages */
    for (uint8_t i_mod = 0; i_mod < (BS_NR_OF_MODULES * BS_NR_OF_STRINGS); i_mod++) {
        if (i_mod < MXM_MAXIMUM_NR_OF_MODULES) {
            uint8_t stringNumber  = 0u;
            uint16_t moduleNumber = 0u;
            MXM_ConvertModuleToString(i_mod, &stringNumber, &moduleNumber);
            mxm_cellVoltages.moduleVoltage_mV[stringNumber][moduleNumber] =
                kpkInstance->localVoltages.blockVoltages[i_mod];
            /* every iteration that we hit a string first (module 0), we reset the packvoltage counter */
            if (moduleNumber == 0u) {
                mxm_cellVoltages.packVoltage_mV[stringNumber] = 0;
            }
            mxm_cellVoltages.packVoltage_mV[stringNumber] += kpkInstance->localVoltages.blockVoltages[i_mod];
            for (uint8_t i_bat = 0; i_bat < BS_NR_OF_CELLS_PER_MODULE; i_bat++) {
                if (i_bat < MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE) {
                    uint16_t cell_counter_db  = (moduleNumber * BS_NR_OF_CELLS_PER_MODULE) + i_bat;
                    uint16_t cell_counter_max = (i_mod * MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE) + i_bat;
                    mxm_cellVoltages.cellVoltage_mV[stringNumber][cell_counter_db] =
                        kpkInstance->localVoltages.cellVoltages_mV[cell_counter_max];
                }
            }
        } else {
            retval = STD_NOT_OK;
        }
    }

    /* temperatures */
    for (uint8_t i_mod = 0; i_mod < (BS_NR_OF_MODULES * BS_NR_OF_STRINGS); i_mod++) {
        if (i_mod < MXM_MAXIMUM_NR_OF_MODULES) {
            uint8_t stringNumber  = 0u;
            uint16_t moduleNumber = 0u;
            MXM_ConvertModuleToString(i_mod, &stringNumber, &moduleNumber);

            /* store aux measurement from AUX2 (MUX0) */
            if (kpkInstance->muxCounter < BS_NR_OF_TEMP_SENSORS_PER_MODULE) {
                const uint16_t temperatureIndexDb = (moduleNumber * BS_NR_OF_TEMP_SENSORS_PER_MODULE) +
                                                    kpkInstance->muxCounter;
                const uint16_t temperatureIndexMxm = (i_mod * MXM_MAXIMUM_NR_OF_AUX_PER_MODULE) + 2u;
                const uint16_t auxVoltage_mV       = kpkInstance->localVoltages.auxVoltages_mV[temperatureIndexMxm];
                /* const uint16_t temporaryVoltage    = (auxVoltage_mV / ((float)3300 - auxVoltage_mV)) * 1000; */
                const int16_t temperature_ddegC = TSI_GetTemperature(auxVoltage_mV);
                mxm_cellTemperatures.cellTemperature_ddegC[stringNumber][temperatureIndexDb] = temperature_ddegC;
            }
        } else {
            retval = STD_NOT_OK;
        }
    }

    DATA_WRITE_DATA(&mxm_cellVoltages, &mxm_cellTemperatures);

    return retval;
}

static STD_RETURN_TYPE_e must_check_return MXM_PreInitSelfCheck(MXM_MONITORING_INSTANCE_s *pState) {
    FAS_ASSERT(pState != NULL_PTR);
    STD_RETURN_TYPE_e retval                   = STD_OK;
    pState->selfCheck.crc                      = MXM_CRC8SelfTest();
    pState->selfCheck.conv                     = MXM_ConvertTest();
    pState->selfCheck.firstSetBit              = MXM_FirstSetBitTest();
    pState->selfCheck.extractValueFromRegister = MXM_ExtractValueFromRegisterTest();
    pState->selfCheck.parseVoltageReadall      = MXM_ParseVoltageReadallTest(pState);
    pState->selfCheck.addressSpaceChecker      = MXM_5XUserAccessibleAddressSpaceCheckerSelfCheck();

    if ((pState->selfCheck.crc == STD_OK) && (pState->selfCheck.addressSpaceChecker == STD_OK) &&
        (pState->selfCheck.conv == STD_OK) && (pState->selfCheck.firstSetBit == STD_OK) &&
        (pState->selfCheck.extractValueFromRegister == STD_OK) && (pState->selfCheck.parseVoltageReadall == STD_OK)) {
        retval = STD_OK;
    } else {
        retval = STD_NOT_OK;
    }

    return retval;
}

static void MXM_StateMachineOperation(MXM_MONITORING_INSTANCE_s *pState) {
    pState->operationRequested = false;
    /* TODO handle transition to measurement states properly with dedicated state-machine */
    /* TODO parse DATACHECKBYTE where available */

    MXM_MONINTORING_STATE_e temp_mon_state = MXM_MONITORING_STATE_FAIL;

    switch (pState->operationSubstate) {
        case MXM_OP_ENTRY_STATE:
            pState->operationSubstate = MXM_OP_SELECT_MUX_CHANNEL;
            break;
        case MXM_OP_SELECT_MUX_CHANNEL:
            /* configure I2CPNTR to channel number corresponding to mux counter*/
            pState->batteryCmdBuffer.regAddress = MXM_REG_I2CPNTR;
            pState->batteryCmdBuffer.lsb        = (0x01u << pState->muxCounter);
            pState->batteryCmdBuffer.msb        = 0x00u;
            MXM_HandleStateWriteall(pState, MXM_OP_WRITE_MUX0);
            break;
        case MXM_OP_WRITE_MUX0:
            /* send configuration to MUX0 for channel 0 (PNTR configured to 1u) */
            pState->batteryCmdBuffer.regAddress = MXM_REG_I2CSEND;
            pState->batteryCmdBuffer.lsb        = (MXM_I2C_MUX0_ADDRESS << 1u);
            pState->batteryCmdBuffer.msb        = 0x40u;
            MXM_HandleStateWriteall(pState, MXM_OP_WRITE_MUX1);
            break;
        case MXM_OP_WRITE_MUX1:
            /* send configuration to MUX1 for channel 0 (PNTR configured to 1u) */
            pState->batteryCmdBuffer.regAddress = MXM_REG_I2CSEND;
            pState->batteryCmdBuffer.lsb        = (MXM_I2C_MUX1_ADDRESS << 1u);
            pState->batteryCmdBuffer.msb        = 0x40u;
            MXM_HandleStateWriteall(pState, MXM_OP_SET_SCAN_STROBE);
            break;
        case MXM_OP_SET_SCAN_STROBE:
            pState->batteryCmdBuffer.regAddress = MXM_REG_SCANCTRL;
            /* set SCANSTROBE, enable 4x OVERSAMPL */
            pState->batteryCmdBuffer.lsb = 0x09u;
            /* enable AUTOBALSWDIS */
            pState->batteryCmdBuffer.msb = 0x10u;
            MXM_HandleStateWriteall(pState, MXM_OP_GET_SCAN_STROBE);
            break;
        case MXM_OP_GET_SCAN_STROBE:
            if (true == MXM_HandleStateReadall(pState, MXM_REG_SCANCTRL, MXM_OP_GET_VOLTAGES)) {
                /* no additional handling needed */
            }
            break;
        case MXM_OP_GET_VOLTAGES:
            temp_mon_state = MXM_MonGetVoltages(pState, mxm_voltageCellAddresses[pState->mxmVoltageCellCounter]);

            if (temp_mon_state == MXM_MONITORING_STATE_PASS) {
                pState->mxmVoltageCellCounter++;
                /* modified: read one additional aux entry */
                if (pState->mxmVoltageCellCounter >= MXM_VOLTAGE_READ_ARRAY_LENGTH) {
                    pState->mxmVoltageCellCounter = 0;
                    pState->operationSubstate     = MXM_OP_GET_ALRTSUM;
                }
            } else if (temp_mon_state == MXM_MONITORING_STATE_FAIL) {
                /* reinitialize */
                pState->state = MXM_STATEMACHINE_STATES_UNINITIALIZED;
            }
            break;
        case MXM_OP_GET_ALRTSUM:
            if (true == MXM_HandleStateReadall(pState, MXM_REG_ALRTSUM, MXM_OP_PARSE_VOLTAGES_INTO_DB)) {
                /* no additional handling needed */
            }
            break;
        case MXM_OP_PARSE_VOLTAGES_INTO_DB:
            if (MXM_ParseVoltagesIntoDB(pState) == STD_OK) {
                pState->operationSubstate    = MXM_OP_PINOPEN_ENTRY;
                pState->firstMeasurementDone = true;
            }
            if (pState->stopRequested == true) {
                pState->state = MXM_STATEMACHINE_STATES_IDLE;
            } else {
                /* do nothing */
            }
            break;
        case MXM_OP_PINOPEN_ENTRY:
            if (pState->openwireRequested == true) {
                pState->operationSubstate = MXM_OP_PINOPEN_SET_CURRENT_SOURCES;
                pState->openwireRequested = false;
            } else {
                pState->operationSubstate = MXM_OP_PINOPEN_EXIT;
            }
            break;
        case MXM_OP_PINOPEN_EXIT:
            pState->operationSubstate = MXM_OP_BAL_ENTRY;
            break;
        case MXM_OP_PINOPEN_SET_CURRENT_SOURCES:
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_CTSTCFG;
                pState->batteryCmdBuffer.lsb        = 0xFFu;
                pState->batteryCmdBuffer.msb        = 0xFFu; /* execute diagnostic on every cell */
            }
            MXM_HandleStateWriteall(pState, MXM_OP_PINOPEN_COMPSCAN);
            break;
        case MXM_OP_PINOPEN_COMPSCAN:
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_SCANCTRL;
                pState->batteryCmdBuffer.lsb        = 0x81u;
                pState->batteryCmdBuffer.msb        = 0x00u; /* request comp scan */
            }
            MXM_HandleStateWriteall(pState, MXM_OP_PINOPEN_GET_SCAN_STROBE);
            break;
        case MXM_OP_PINOPEN_GET_SCAN_STROBE:
            if (true == MXM_HandleStateReadall(pState, MXM_REG_SCANCTRL, MXM_OP_PINOPEN_GET_ALRT)) {
                /* no additional handling needed */
            }
            break;
        case MXM_OP_PINOPEN_GET_ALRT:
            if (true == MXM_HandleStateReadall(pState, MXM_REG_ALRTCOMPUV, MXM_OP_PINOPEN_PROCESS_OPENWIRE)) {
                /* no additional handling needed */
            }
            break;
        case MXM_OP_PINOPEN_PROCESS_OPENWIRE:
            MXM_ProcessOpenWire(pState, &mxm_openwire);
            pState->operationSubstate = MXM_OP_PINOPEN_RESTORE_CURRENT_SOURCE_CONF;
            break;
        case MXM_OP_PINOPEN_RESTORE_CURRENT_SOURCE_CONF:
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_CTSTCFG;
                pState->batteryCmdBuffer.lsb        = 0x00u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_OP_PINOPEN_EXIT);
            break;
        case MXM_OP_BAL_ENTRY:
            /* Get the current time */
            mxm_currentTime = OS_GetTickCount();

            /* NOTE: The overflow of the timer (coded on 32bits) is handled by the CPU
               Wait 'MXM_DELAY_BALANCING' milliseconds before processing the balancing */
            if ((mxm_currentTime - mxm_previousTime) < MXM_DELAY_BALANCING) {
                /* nothing to do, exit balancing state chain */
                pState->operationSubstate = MXM_OP_BAL_EXIT;
            } else {
                /* Balancing needs to be processed */
                mxm_previousTime = mxm_currentTime;
                /* First reset the balancing switches, and THEN
                   set the balancing switches according to the database */
                pState->operationSubstate = MXM_OP_BAL_CONTROL_RESET_ALL;

                /* Change the parity of cells to balance */
                if (mxm_evenCellsBalancingProcessed == 1u) {
                    mxm_evenCellsNeedBalancing = 0u;
                    mxm_oddCellsNeedBalancing  = 1u;
                }
                /* Same for odd cells */
                if (mxm_oddCellsBalancingProcessed == 1u) {
                    mxm_evenCellsNeedBalancing = 1u;
                    mxm_oddCellsNeedBalancing  = 0u;
                }
            }
            break;
        case MXM_OP_BAL_CONTROL_RESET_ALL:
            /* Send a WRITEALL command to open all balancing switches */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALSWCTRL;
                /* CBRESTART is not reset to 0, not needed. */
                pState->batteryCmdBuffer.lsb = 0x00U;
                pState->batteryCmdBuffer.msb = 0x00U;
            }
            MXM_HandleStateWriteall(pState, MXM_OP_BAL_CONTROL_SET_ALL);
            break;
        case MXM_OP_BAL_CONTROL_SET_ALL:
            /* Send a WRITEDEVICE command to each module in a daisy-chain
               to close appropriate balancing switches */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                mxm_evenCellsBalancingProcessed = 0u;
                mxm_oddCellsBalancingProcessed  = 0u;

                STD_RETURN_TYPE_e database_retval = DATA_READ_DATA(&mxm_balancingControl);
                if (STD_OK == database_retval) {
                    /* Construct the balancing buffer */
                    STD_RETURN_TYPE_e retval = MXM_ConstructBalancingBuffer();

                    if (retval == STD_OK) {
                        if (mxm_cellsToBalance > 0u) {
                            /* Some cells need to be balanced */
                            pState->batteryCmdBuffer.deviceAddress = mxm_moduleBalancingIndex;
                            pState->batteryCmdBuffer.regAddress    = MXM_REG_BALSWCTRL;
                            pState->batteryCmdBuffer.lsb           = (uint8_t)mxm_cellsToBalance;
                            pState->batteryCmdBuffer.msb           = (uint8_t)(mxm_cellsToBalance >> 8u);
                            MXM_5XSetStateRequest(
                                pState->pInstance5X,
                                MXM_STATEMACH_5X_WRITE_DEVICE,
                                pState->batteryCmdBuffer,
                                &pState->requestStatus5x);
                        } else {
                            /* It is not necessary to re-send 0 to the device, because it has been done previously
                           in the BALANCING_CONTROL_RESET_ALL sub-state */
                            pState->requestStatus5x = MXM_5X_STATE_PROCESSED;
                        }

                    } else {
                        /* this should not happen if the software works as expected */
                        /* TODO review this trap */
                        /* FAS_ASSERT(FAS_TRAP); */
                        pState->requestStatus5x = MXM_5X_STATE_PROCESSED;
                    }
                } else {
                    /* database read not successful, retry */
                    pState->requestStatus5x = MXM_5X_STATE_UNSENT;
                }
            } else if (pState->requestStatus5x == MXM_5X_STATE_UNPROCESSED) {
                /* wait for processing */
            } else if (pState->requestStatus5x == MXM_5X_STATE_PROCESSED) {
                if (mxm_moduleBalancingIndex < pState->highest5xDevice) {
                    /* Not all modules have been treated. Repeat this state with the next module */
                    mxm_moduleBalancingIndex++;
                    pState->operationSubstate = MXM_OP_BAL_CONTROL_SET_ALL;
                } else {
                    /* Finished the iteration of all modules in a daisy chain, continue */
                    mxm_moduleBalancingIndex = 0;

                    /* Update flags for the end of balancing */
                    if (mxm_evenCellsNeedBalancing == 1u) {
                        mxm_evenCellsBalancingProcessed = 1u;
                    } else if (mxm_oddCellsNeedBalancing == 1u) {
                        mxm_oddCellsBalancingProcessed = 1u;
                    }
                }

                /* Re-set the status to UNSENT to repeat the WRITE operation */
                pState->requestStatus5x   = MXM_5X_STATE_UNSENT;
                pState->operationSubstate = MXM_OP_BAL_EXIT;
            } else if (pState->requestStatus5x == MXM_5X_STATE_ERROR) {
                pState->requestStatus5x = MXM_5X_STATE_UNSENT;
            }
            break;
        case MXM_OP_BAL_START:
            /* Initiate balancing for all devices in a daisy chain */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALCTRL;
                /*  Manual ON MODE + Balancing halt in case of High temperature
                    + Alert when cell balancing is finished */
                pState->batteryCmdBuffer.lsb = 0x0EU;
                pState->batteryCmdBuffer.msb = 0x18U;
            }
            MXM_HandleStateWriteall(pState, MXM_OP_BAL_EXIT);
            break;
        case MXM_OP_BAL_EXIT:
            pState->operationSubstate = MXM_OP_CYCLE_END_ENTRY;
            break;
        case MXM_OP_CYCLE_END_ENTRY:
            /* actions that should be taken at the end of a measurement cycle */
            pState->operationSubstate = MXM_OP_INCREMENT_MUX_COUNTER;
            break;
        case MXM_OP_INCREMENT_MUX_COUNTER:
            if (pState->muxCounter < (8u - 1u)) {
                pState->muxCounter++;
            } else {
                pState->muxCounter = 0u;
            }
            pState->operationSubstate = MXM_OP_CYCLE_END_EXIT;
            break;
        case MXM_OP_CYCLE_END_EXIT:
            pState->operationSubstate = MXM_OP_ENTRY_STATE;
            break;
        /* "initialization" from here on */
        case MXM_INIT_DEVCFG1:
            /* switch to single UART with external loopback */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_DEVCFG1;
                pState->batteryCmdBuffer.lsb        = 0x02u;
                pState->batteryCmdBuffer.msb        = 0x01u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_SET_FMEA2);
            break;
        case MXM_INIT_SET_FMEA2:
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_FMEA2;
                pState->batteryCmdBuffer.lsb        = 0x00u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_STATUS1);
            break;
        case MXM_INIT_STATUS1:
            /* clear ALRTRST */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_STATUS1;
                pState->batteryCmdBuffer.lsb        = 0x00u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_GET_VERSION);
            break;
        case MXM_INIT_GET_VERSION:
            /* add version information to registry */
            if (true == MXM_HandleStateReadall(pState, MXM_REG_VERSION, MXM_INIT_GET_ID1)) {
                uint8_t temp_len = BATTERY_MANAGEMENT_TX_LENGTH_READALL +
                                   (2u * MXM_5XGetNumberOfSatellites(pState->pInstance5X));
                MXM_MonRegistryParseVersionIntoDevices(pState, temp_len);
            }
            break;
        case MXM_INIT_GET_ID1:
            /* add ID1 to registry */
            if (true == MXM_HandleStateReadall(pState, MXM_REG_ID1, MXM_INIT_GET_ID2)) {
                uint8_t temp_len = BATTERY_MANAGEMENT_TX_LENGTH_READALL +
                                   (2u * MXM_5XGetNumberOfSatellites(pState->pInstance5X));
                MXM_MonRegistryParseIdIntoDevices(pState, temp_len, MXM_REG_ID1);
            }
            break;
        case MXM_INIT_GET_ID2:
            /* add ID2 to registry */
            if (true == MXM_HandleStateReadall(pState, MXM_REG_ID2, MXM_INIT_MEASUREEN1)) {
                uint8_t temp_len = BATTERY_MANAGEMENT_TX_LENGTH_READALL +
                                   (2u * MXM_5XGetNumberOfSatellites(pState->pInstance5X));
                MXM_MonRegistryParseIdIntoDevices(pState, temp_len, MXM_REG_ID2);
            }
            break;
        case MXM_INIT_MEASUREEN1:
            /* enable all 14 cells */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_MEASUREEN1;
                pState->batteryCmdBuffer.lsb        = 0xFFu;
                pState->batteryCmdBuffer.msb        = 0x7Fu; /* TODO this also enables block measurement */
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_MEASUREEN2);
            break;
        case MXM_INIT_MEASUREEN2:
            /* enable AUX2 and AUX3 */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_MEASUREEN2;
                pState->batteryCmdBuffer.lsb        = 0x0Cu; /* AUX2 and AUX3 */
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_AUXGPIOCFG);
            break;
        case MXM_INIT_AUXGPIOCFG:
            /* switch GPIO2 and GPIO3 to AUX, enable I2C */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_AUXGPIOCFG;
                pState->batteryCmdBuffer.lsb        = 0x00u;
                /* pState->batteryCmdBuffer.msb = 0x3Eu; */ /* conf for MAX17853 */
                /* pState->batteryCmdBuffer.msb = 0x8Eu; */ /* conf for MAX17852, I2C enable */
                /* I2C enable, AUX2 and AUX3 */
                pState->batteryCmdBuffer.msb = 0x80u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_AUXTIMEREG);
            break;
        case MXM_INIT_AUXTIMEREG:
            /* configure settling time that NTC network takes for measurement to 500us */
            /* WARNING: reevaluate this value if thermistor supply is switched
              during sampling */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_AUXTIME;
                pState->batteryCmdBuffer.lsb        = 0x53u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_ACQCFG);
            break;
        case MXM_INIT_ACQCFG:
            /* set ACQGFC */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_ACQCFG;
                /* default values */
                pState->batteryCmdBuffer.lsb = 0x00u;
                /* we have to turn thrm switch manually on, as charging the
                network takes to long */
                pState->batteryCmdBuffer.msb = 0x06u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_UVTHSETREG);
            break;
        case MXM_INIT_UVTHSETREG:
            /* configure UVTHSETREG */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_UVTHSET;
                MXM_Unipolar14BitInto16Bit(
                    MXM_VoltageIntoUnipolar14Bit(1700u, 5000u),
                    &pState->batteryCmdBuffer.lsb,
                    &pState->batteryCmdBuffer.msb);
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_OVTHSETREG);
            break;
        case MXM_INIT_OVTHSETREG:
            /* configure OVTHSETREG */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_OVTHSET;
                MXM_Unipolar14BitInto16Bit(
                    MXM_VoltageIntoUnipolar14Bit(3300u, 5000u),
                    &pState->batteryCmdBuffer.lsb,
                    &pState->batteryCmdBuffer.msb);
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP1);
            break;
        case MXM_INIT_BALEXP1:
            /* set BALEXP1 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP1;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP2);
            break;
        case MXM_INIT_BALEXP2:
            /* set BALEXP2 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP2;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP3);
            break;
        case MXM_INIT_BALEXP3:
            /* set BALEXP3 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP3;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP4);
            break;
        case MXM_INIT_BALEXP4:
            /* set BALEXP4 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP4;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP5);
            break;
        case MXM_INIT_BALEXP5:
            /* set BALEXP5 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP5;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP6);
            break;
        case MXM_INIT_BALEXP6:
            /* set BALEXP6 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP6;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP7);
            break;
        case MXM_INIT_BALEXP7:
            /* set BALEXP7 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP7;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP8);
            break;
        case MXM_INIT_BALEXP8:
            /* set BALEXP8 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP8;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP9);
            break;
        case MXM_INIT_BALEXP9:
            /* set BALEXP9 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP9;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP10);
            break;
        case MXM_INIT_BALEXP10:
            /* set BALEXP10 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP10;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP11);
            break;
        case MXM_INIT_BALEXP11:
            /* set BALEXP11 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP11;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP12);
            break;
        case MXM_INIT_BALEXP12:
            /* set BALEXP12 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP12;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP13);
            break;
        case MXM_INIT_BALEXP13:
            /* set BALEXP13 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP13;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALEXP14);
            break;
        case MXM_INIT_BALEXP14:
            /* set BALEXP14 to have 1 minute timeout in manual balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALEXP14;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_BALSWDLY);
            break;
        case MXM_INIT_BALSWDLY:
            /* set BALSWDLY to 2 ms settling time after balancing */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_BALSWDLY;
                /* CELLDLY = 1920us (~2ms) --> 20*96us
            CELLDLY corresponds to the time to relax the cell before voltage measurement */
                pState->batteryCmdBuffer.lsb = 0x00u;
                pState->batteryCmdBuffer.msb = 0x14u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_ALRTOVEN);
            break;
        case MXM_INIT_ALRTOVEN:
            /* enable ALRTOVEN */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_ALRTOVEN;
                pState->batteryCmdBuffer.lsb        = 0xFFu;
                pState->batteryCmdBuffer.msb        = 0x3Fu;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_ALRTUVEN);
            break;
        case MXM_INIT_ALRTUVEN:
            /* enable ALRTUVEN */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_ALRTUVEN;
                pState->batteryCmdBuffer.lsb        = 0xFFu;
                pState->batteryCmdBuffer.msb        = 0x3Fu;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_COMPOPNTHREG);
            break;
        case MXM_INIT_COMPOPNTHREG:
            /* configure COMPOPNTHREG */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_COMPOPNTH;
                /* TODO 0.5V */
                MXM_Unipolar14BitInto16Bit(
                    MXM_VoltageIntoUnipolar14Bit(500u, 5000u),
                    &pState->batteryCmdBuffer.lsb,
                    &pState->batteryCmdBuffer.msb);
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_I2C_GET_STAT1);
            break;
        case MXM_INIT_I2C_GET_STAT1:
            if (true == MXM_HandleStateReadall(pState, MXM_REG_I2CSTAT, MXM_INIT_I2C_CFG)) {
                /* no additional handling needed */
            }
            break;
        case MXM_INIT_I2C_CFG:
            /* configure I2CCFG to
               * 400kHz
               * Alternate write Mode (just a pointer without data)
               * Combined Format
               * 7 Bit addressing
               * one byte pointer length
               * default
               */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_I2CCFG;
                pState->batteryCmdBuffer.lsb        = 0x00u;
                pState->batteryCmdBuffer.msb        = 0xE0u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_I2C_PNTR);
            break;
        case MXM_INIT_I2C_PNTR:
            /* configure I2CPNTR */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_I2CPNTR;
                pState->batteryCmdBuffer.lsb        = 0x01u;
                pState->batteryCmdBuffer.msb        = 0x00u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_I2C_SEND_MUX0);
            break;
        case MXM_INIT_I2C_SEND_MUX0:
            /* send configuration to MUX0 for channel 0 (PNTR configured to 1u) */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_I2CSEND;
                pState->batteryCmdBuffer.lsb        = (MXM_I2C_MUX0_ADDRESS << 1u);
                pState->batteryCmdBuffer.msb        = 0x40u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_I2C_SEND_MUX1);
            break;
        case MXM_INIT_I2C_SEND_MUX1:
            /* send configuration to MUX1 for channel 0 (PNTR configured to 1u) */
            if (pState->requestStatus5x == MXM_5X_STATE_UNSENT) {
                pState->batteryCmdBuffer.regAddress = MXM_REG_I2CSEND;
                pState->batteryCmdBuffer.lsb        = (MXM_I2C_MUX1_ADDRESS << 1u);
                pState->batteryCmdBuffer.msb        = 0x40u;
            }
            MXM_HandleStateWriteall(pState, MXM_INIT_GET_I2C_STAT2);
            break;
        case MXM_INIT_GET_I2C_STAT2:
            if (true == MXM_HandleStateReadall(pState, MXM_REG_I2CSTAT, MXM_OP_ENTRY_STATE)) {
                /* no additional handling needed */
            }
            break;
        default:
            /* invalid state */
            FAS_ASSERT(FAS_TRAP);
            break;
    }
}

/*========== Extern Function Implementations ================================*/

extern void MXM_StateMachine(MXM_MONITORING_INSTANCE_s *pInstance) {
    FAS_ASSERT(pInstance != NULL_PTR);

    if (pInstance->state == MXM_STATEMACHINE_STATES_UNINITIALIZED) {
        pInstance->requestStatus5x = MXM_5X_STATE_UNSENT;
        MXM_MonRegistryInit(pInstance);
        pInstance->state = MXM_STATEMACHINE_STATES_SELFCHECK_PRE_INIT;
    }

    switch (pInstance->state) {
        case MXM_STATEMACHINE_STATES_SELFCHECK_PRE_INIT:
            /* TODO proper selfchecks, that are only executed during IBIT, CBIT and PBIT */
            pInstance->resultSelfCheck = MXM_PreInitSelfCheck(pInstance);
            if (pInstance->resultSelfCheck == STD_OK) {
                pInstance->requestStatus5x = MXM_5X_STATE_UNSENT;
                pInstance->state           = MXM_STATEMACHINE_STATES_INIT;
            } else {
                /* software is broken by configuration/design */
                FAS_ASSERT(FAS_TRAP);
            }
            break;
        case MXM_STATEMACHINE_STATES_INIT:
            if (pInstance->requestStatus5x == MXM_5X_STATE_UNSENT) {
                MXM_5XSetStateRequest(
                    pInstance->pInstance5X,
                    MXM_STATEMACH_5X_INIT,
                    pInstance->batteryCmdBuffer,
                    &pInstance->requestStatus5x);
            } else if (pInstance->requestStatus5x == MXM_5X_STATE_UNPROCESSED) {
                /* just wait */
                /* TODO timeout? */
            } else if (pInstance->requestStatus5x == MXM_5X_STATE_ERROR) {
                /* TODO error handling */
            } else if (pInstance->allowStartup == false) {
                /* do not start yet, just wait */

            } else if ((pInstance->requestStatus5x == MXM_5X_STATE_PROCESSED) && (pInstance->allowStartup == true)) {
                /* update registry */
                /* TODO handle return value of this function */
                MXM_MonRegistryConnectDevices(pInstance, MXM_5XGetNumberOfSatellites(pInstance->pInstance5X));
                pInstance->requestStatus5x = MXM_5X_STATE_UNSENT;
                pInstance->state           = MXM_STATEMACHINE_STATES_SELFCHECK_POST_INIT;
            } else {
                FAS_ASSERT(FAS_TRAP);
            }

            break;
        case MXM_STATEMACHINE_STATES_SELFCHECK_POST_INIT:
            /* TODO more checks */
            pInstance->selfCheck.fmeaStatusASCI = STD_NOT_OK;
            if (pInstance->requestStatus5x == MXM_5X_STATE_UNSENT) {
                MXM_5XSetStateRequest(
                    pInstance->pInstance5X,
                    MXM_STATEMACH_5X_41B_FMEA_CHECK,
                    pInstance->batteryCmdBuffer,
                    &pInstance->requestStatus5x);
            } else if (pInstance->requestStatus5x == MXM_5X_STATE_UNPROCESSED) {
                /* just wait */
                /* TODO timeout? */
            } else if (pInstance->requestStatus5x == MXM_5X_STATE_ERROR) {
                pInstance->selfCheck.fmeaStatusASCI = STD_NOT_OK;
                /* TODO configuration or hardware failure */
                pInstance->resultSelfCheck = STD_NOT_OK;
                FAS_ASSERT(FAS_TRAP);
            } else if (pInstance->requestStatus5x == MXM_5X_STATE_PROCESSED) {
                pInstance->selfCheck.fmeaStatusASCI = STD_OK;
                if ((pInstance->resultSelfCheck == STD_OK) && (pInstance->selfCheck.fmeaStatusASCI == STD_OK) &&
                    (MXM_5XGetNumberOfSatellitesGood(pInstance->pInstance5X) == STD_OK)) {
                    pInstance->resultSelfCheck = STD_OK;
                    pInstance->requestStatus5x = MXM_5X_STATE_UNSENT;
                    pInstance->state           = MXM_STATEMACHINE_STATES_IDLE;
                }
            } else {
                FAS_ASSERT(FAS_TRAP);
            }
            break;
        case MXM_STATEMACHINE_STATES_IDLE:
            pInstance->stopRequested = false;
            if (pInstance->operationRequested == true) {
                pInstance->state = MXM_STATEMACHINE_STATES_OPERATION;
            } else {
                /* Do nothing */
            }
            break;
        case MXM_STATEMACHINE_STATES_OPERATION:
            MXM_StateMachineOperation(pInstance);
            break;
        default:
            FAS_ASSERT(FAS_TRAP);
            break;
    }
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern STD_RETURN_TYPE_e TEST_MXM_ParseVoltageReadallTest(MXM_MONITORING_INSTANCE_s *pInstance) {
    return MXM_ParseVoltageReadallTest(pInstance);
}
extern STD_RETURN_TYPE_e TEST_MXM_ParseVoltagesIntoDB(MXM_MONITORING_INSTANCE_s *pInstance) {
    return MXM_ParseVoltagesIntoDB(pInstance);
}
extern MXM_MONINTORING_STATE_e TEST_MXM_MonGetVoltages(
    MXM_MONITORING_INSTANCE_s *pInstance,
    MXM_REG_NAME_e regAddress) {
    return MXM_MonGetVoltages(pInstance, regAddress);
}
extern void TEST_MXM_HandleStateWriteall(
    MXM_MONITORING_INSTANCE_s *pInstance,
    MXM_STATEMACHINE_OPERATION_STATES_e nextState) {
    MXM_HandleStateWriteall(pInstance, nextState);
}
extern bool TEST_MXM_HandleStateReadall(
    MXM_MONITORING_INSTANCE_s *pInstance,
    MXM_REG_NAME_e registerName,
    MXM_STATEMACHINE_OPERATION_STATES_e nextState) {
    return MXM_HandleStateReadall(pInstance, registerName, nextState);
}
extern STD_RETURN_TYPE_e TEST_MXM_ParseVoltageReadall(
    uint8_t *volt_rx_buffer,
    uint16_t volt_rx_buffer_len,
    MXM_DATA_STORAGE_s *datastorage,
    MXM_CONVERSION_TYPE_e conversionType) {
    return MXM_ParseVoltageReadall(volt_rx_buffer, volt_rx_buffer_len, datastorage, conversionType);
}
extern void TEST_MXM_ProcessOpenWire(MXM_MONITORING_INSTANCE_s *pInstance, DATA_BLOCK_OPEN_WIRE_s *pDataOpenWire) {
    MXM_ProcessOpenWire(pInstance, pDataOpenWire);
}
#endif
