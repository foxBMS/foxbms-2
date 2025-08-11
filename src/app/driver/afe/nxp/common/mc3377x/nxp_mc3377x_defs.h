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
 * @file    nxp_mc3377x_defs.h
 * @author  foxBMS Team
 * @date    2020-09-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  N77X
 *
 * @brief   Definitions for the driver for the MC3377X analog front-end.
 * @details TODO
 */

#ifndef FOXBMS__NXP_MC3377X_DEFS_H_
#define FOXBMS__NXP_MC3377X_DEFS_H_

/*========== Includes =======================================================*/

#include "database.h"
#include "spi.h"

#if defined(FOXBMS_AFE_DRIVER_NXP_MC33775A)
#include "nxp_mc33775a_defs.h"
#endif

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/* TODO: Determine which defines are version-specific*/

/** error table for the LTC driver */
typedef struct {
    bool communicationOk[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING];        /*!<    */
    bool noCommunicationTimeout[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING]; /*!<    */
    bool crcIsValid[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING];             /*!<    */
    bool mux0IsOk[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING];               /*!<    */
    bool mux1IsOK[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING];               /*!<    */
    bool mux2IsOK[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING];               /*!<    */
    bool mux3IsOK[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING];               /*!<    */
} N77X_ERROR_TABLE_s;

/** error table for the LTC driver */
typedef struct {
    uint16_t current[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING];
} N77X_SUPPLY_CURRENT_s;

/** configuration of the mux channels */
typedef struct {
    uint8_t muxId;      /*!< multiplexer ID 0 - 3       */
    uint8_t muxChannel; /*!< multiplexer channel 0 - 7   */
} N77X_MUX_CH_CFG_s;

/** State requests for the N77X state machine */
typedef enum {
    N77X_STATE_INITIALIZATION_REQUEST, /*!<    */
    N77X_STATE_NO_REQUEST,             /*!<    */
} N77X_STATE_REQUEST_e;

/** This struct contains pointer to used data buffers */
typedef struct {
    DATA_BLOCK_CELL_VOLTAGE_s *cellVoltage;
    DATA_BLOCK_CELL_TEMPERATURE_s *cellTemperature;
    DATA_BLOCK_ALL_GPIO_VOLTAGES_s *allGpioVoltage;
    DATA_BLOCK_MIN_MAX_s *minMax;
    DATA_BLOCK_BALANCING_FEEDBACK_s *balancingFeedback;
    DATA_BLOCK_BALANCING_CONTROL_s *balancingControl;
    DATA_BLOCK_SLAVE_CONTROL_s *slaveControl;
    DATA_BLOCK_OPEN_WIRE_s *openWire; /* How to handle open wire check? What should be stored? */
    N77X_SUPPLY_CURRENT_s *supplyCurrent;
    N77X_ERROR_TABLE_s *errorTable;
} N77X_DATA_s;

/**
 * This structure contains all the variables relevant for the N77X state machine.
 * The user can get the current state of the N77X state machine with this variable
 */
typedef struct {
    bool firstMeasurementMade; /*!< flag that indicates if the first measurement cycle was completed */
    uint8_t currentString;     /*!< string currently being addressed */
    SPI_INTERFACE_CONFIG_s *pSpiTxSequenceStart; /*!< pointer to the start of SPI sequence to be used for Tx */
    SPI_INTERFACE_CONFIG_s *pSpiTxSequence;      /*!< pointer to the SPI sequence to be used for Tx */
    SPI_INTERFACE_CONFIG_s *pSpiRxSequenceStart; /*!< pointer to the start of SPI sequence to be used for Rx */
    SPI_INTERFACE_CONFIG_s *pSpiRxSequence;      /*!< pointer to the SPI sequence to be used for Rx */
    uint8_t currentMux[BS_NR_OF_STRINGS];        /*!< mux currently being addressed */
    N77X_MUX_CH_CFG_s *pMuxSequenceStart[BS_NR_OF_STRINGS]; /*!< pointer to the multiplexer sequence to be measured
                                                               (contains a list of elements [multiplexer id, multiplexer
                                                               channels]) (1,-1)...(3,-1),(0,1),...(0,7) */
    N77X_MUX_CH_CFG_s
        *pMuxSequence[BS_NR_OF_STRINGS]; /*!< pointer to the multiplexer sequence to be measured (contains
                                                          a list of elements [multiplexer id, multiplexer channels])
                                                          (1,-1)...(3,-1),(0,1),...(0,7) */
    N77X_DATA_s n77xData;
    uint64_t serialId[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING]; /*!< serial ID of the IC */
} N77X_STATE_s;

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__NXP_MC3377X_DEFS_H_ */
