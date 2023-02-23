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
 * @file    nxp_mc33775a_defs.h
 * @author  foxBMS Team
 * @date    2020-09-01 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  N775
 *
 * @brief   Definitions for the driver for the MC33775A analog front-end.
 *
 */

#ifndef FOXBMS__NXP_MC33775A_DEFS_H_
#define FOXBMS__NXP_MC33775A_DEFS_H_

/*========== Includes =======================================================*/

#include "database.h"
#include "spi.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/* Use of mux for temperature measurement */
#define N775_USE_MUX_FOR_TEMP (true)
/* Read mux state after setting it */
#define N775_CHECK_MUX_STATE (true)
/* Check supply current (part of MC33775A DIAG) */
#define N775_CHECK_SUPPLY_CURRENT (false)
/* Broadcast address for the N775 slaves */
#define N775_BROADCAST_ADDRESS (63u)
/* Time to wait between activity on bus and wake up of slave */
#define N775_WAKEUP_TIME_MS (3u)
/* Timeout in 10ms to go into sleep, max 255 (corresponding to 2550ms) */
#define N775_TIMEOUT_TO_SLEEP_10MS (200u)
/* Timeout enable, 0u in register = enabled */
#define N775_TIMEOUT_ENABLED (0u)
/* Timeout enable, 0x5A in register = disabled */
#define N775_TIMEOUT_DISABLED (0x5Au)
/* Timeout enable switch*/
#define N775_TIMEOUT_SWITCH (N775_TIMEOUT_ENABLED)
/* Time to wait after measurements started */
#define N775_TIME_AFTER_MEASUREMENT_START_MS (5u)
/* Default chain address */
#define N775_DEFAULT_CHAIN_ADDRESS (1u)
/* Measurement capture time */
/* + 1 to take balancing pause time before measurement into account */
#define N775_MEASUREMENT_CAPTURE_TIME_MS (5u + 1u)
/* Time to wait after measurement capture for measurements to be ready */
#define N775_MEASUREMENT_READY_TIME_MS (1u)
/* Value stored in N775A register when not valid */
#define N775_INVALID_REGISTER_VALUE (0x8000u)
/* Length of the mux measurement sequence */
#define N775_MUX_SEQUENCE_LENGTH (8u)
/* Time in 10us for pause of balancing before measurement start */
/* Wait time afer capture must be increased if this value is too high */
#define N775_BALPAUSELEN_10US (100u)
/**
 * Upper bits of ADG728 mux address byte
 *  Set to 0: bit2, bit1 (address), bit0 (R/W)
 */
#define N775_ADG728_ADDRESS_UPPERBITS (0x98u)
/* I2C R/W bit, read */
#define N775_I2C_READ (1u)
/* I2C R/W bit, write */
#define N775_I2C_WRITE (0u)
/* Dummy byte for I2C, replaced by read data */
#define N775_I2C_DUMMY_BYTE (0x0u)
/* GPIO position (0 to 7) used for multiplexed temperature measurement */
#define N775_MUXED_TEMP_GPIO_POSITION (0u)
/* Global balancing timer (0x0 - 0xFFFF) */
#define N775_GLOBAL_BALANCING_TIMER (0xFFFFu)
/* Pre-balancing timer (0x0 - 0xFFFF) */
#define N775_PRE_BALANCING_TIMER (0x0u)
/* All channel balancing timer (0x0 - 0xFFFF) */
#define N775_ALL_CHANNEL_BALANCING_TIMER (0xFFFFu)
/* Global balancing timer value to reach to reset it to max */
#define N775_GLOBAL_BALANCING_TIMER_VALUE_SET_TO_MAX_AGAIN (0x1000u)
/* Maximum number of tries to avoid endless loop when waiting for a flag to be ready */
#define N775_FLAG_READY_TRIES (5u)

/* Timeout in ms when waiting for an I2C transaction over NXP slave */
#define N775_I2C_FINISHED_TIMEOUT_ms (50u)

/** error table for the LTC driver */
typedef struct {
    bool communicationOk[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING];        /*!<    */
    bool noCommunicationTimeout[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING]; /*!<    */
    bool crcIsValid[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING];             /*!<    */
    bool mux0IsOk[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING];               /*!<    */
    bool mux1IsOK[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING];               /*!<    */
    bool mux2IsOK[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING];               /*!<    */
    bool mux3IsOK[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING];               /*!<    */
} N775_ERRORTABLE_s;

/** error table for the LTC driver */
typedef struct {
    uint16_t current[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING];
} N775_SUPPLY_CURRENT_s;

/** configuration of the mux channels */
typedef struct {
    uint8_t muxId;      /*!< multiplexer ID 0 - 3       */
    uint8_t muxChannel; /*!< multiplexer channel 0 - 7   */
} N775_MUX_CH_CFG_s;

/** States of the N775 state machine */
typedef enum {
    N775_STATEMACH_UNINITIALIZED,  /*!<    */
    N775_STATEMACH_INITIALIZATION, /*!<    */
    N775_STATEMACH_WAKEUP,         /*!<    */
    N775_STATEMACH_ENUMERATE,      /*!<    */
    N775_STATEMACH_INITIALIZED,    /*!<    */
    N775_STATEMACH_IDLE,           /*!<    */
    N775_STATEMACH_STARTMEAS,      /*!<    */
    N775_STATEMACH_READVOLTAGE,    /*!<    */
    N775_STATEMACH_BALANCECONTROL,
} N775_STATEMACH_e;

/** General substates */
typedef enum {
    N775_ENTRY,             /*!<        */
    N775_SECOND_WAKEUP,     /*!<        */
    N775_CHECK_ENUMERATION, /*!<        */
    N775_ERROR_ENTRY,       /*!<        */
    N775_ERROR_PROCESSED,   /*!<    */
} N775_STATEMACH_SUB_e;

/** Substates for the uninitialized state */
typedef enum {
    N775_ENTRY_UNINITIALIZED = 0, /*!< Initialize-sequence */
} N775_STATEMACH_UNINITIALIZED_SUB_e;

/** Substates for the initialization state */
typedef enum {
    /* Init-Sequence */
    N775_ENTRY_INITIALIZATION         = 0, /*!<    */
    N775_START_INIT_INITIALIZATION    = 1, /*!<    */
    N775_RE_ENTRY_INITIALIZATION      = 2, /*!<    */
    N775_READ_INITIALIZATION_REGISTER = 3, /*!<    */
    N775_CHECK_INITIALIZATION         = 4, /*!<    */
    N775_EXIT_INITIALIZATION          = 5, /*!<    */
} N775_STATEMACH_INITIALIZATION_SUB_e;

/** State requests for the N775 statemachine */
typedef enum {
    N775_STATE_INITIALIZATION_REQUEST, /*!<    */
    N775_STATE_NO_REQUEST,             /*!<    */
} N775_STATE_REQUEST_e;

/**
 * Possible return values when state requests are made to the N775 statemachine
 */
typedef enum {
    N775_OK                  = 0,  /*!< N775 --> ok                             */
    N775_BUSY_OK             = 1,  /*!< N775 under load --> ok                  */
    N775_REQUEST_PENDING     = 2,  /*!< requested to be executed               */
    N775_ILLEGAL_REQUEST     = 3,  /*!< Request can not be executed            */
    N775_SPI_ERROR           = 4,  /*!< Error state: Source: SPI               */
    N775_PEC_ERROR           = 5,  /*!< Error state: Source: PEC               */
    N775_MUX_ERROR           = 6,  /*!< Error state: Source: MUX               */
    N775_INIT_ERROR          = 7,  /*!< Error state: Source: Initialization    */
    N775_OK_FROM_ERROR       = 8,  /*!< Return from error --> ok               */
    N775_GENERAL_ERROR       = 20, /*!< General error state                    */
    N775_ALREADY_INITIALIZED = 30, /*!< Initialization of N775 already finished */
    N775_ILLEGAL_TASK_TYPE   = 99, /*!< Illegal                                */
} N775_RETURN_TYPE_e;

/** This struct contains pointer to used data buffers */
typedef struct {
    DATA_BLOCK_CELL_VOLTAGE_s *cellVoltage;
    DATA_BLOCK_CELL_TEMPERATURE_s *cellTemperature;
    DATA_BLOCK_ALL_GPIO_VOLTAGES_s *allGpioVoltage;
    DATA_BLOCK_MIN_MAX_s *minMax;
    DATA_BLOCK_BALANCING_FEEDBACK_s *balancingFeedback;
    DATA_BLOCK_USER_MUX_s *userMux;
    DATA_BLOCK_BALANCING_CONTROL_s *balancingControl;
    DATA_BLOCK_SLAVE_CONTROL_s *slaveControl;
    DATA_BLOCK_OPEN_WIRE_s *openWire; /* Wie genau open wire check behandeln? Was genau abspeichern? */
    N775_SUPPLY_CURRENT_s *supplyCurrent;
    N775_ERRORTABLE_s *errorTable;
    uint64_t uid[BS_NR_OF_STRINGS][BS_NR_OF_MODULES_PER_STRING];
} N775_DATAPTR_s;

/** This struct contains error counter and pointer to used error buffers */
typedef struct {
    uint32_t *errPECCnt; /* array length: Number of used N775s */
    uint32_t *errSPICnt; /* array length: Number of used N775s */
    uint8_t *n775Status; /* array length: Number of used N775s */
    uint8_t errPECRetryCnt;
    uint8_t errSPIRetryCnt;
    uint8_t errOccurred;
    uint32_t nrOfConsecutiveErrors;
} N775_ERROR_s;

/**
 * This struct contains the measurement configuration for the N775
 * Measurement is deactivated with value = N775_STATE_NO_REQUEST
 */
typedef struct {
    N775_STATE_REQUEST_e measVoltage;             /* activated = N775_STATE_VOLTAGEMEASUREMENT_REQUEST */
    N775_STATE_REQUEST_e measVoltage2Cells;       /* activated = N775_STATE_VOLTAGEMEASUREMENT_2CELLS_REQUEST */
    N775_STATE_REQUEST_e measVoltageSumOfCells;   /* activated = N775_STATE_VOLTAGEMEASUREMENT_SC_REQUEST */
    N775_STATE_REQUEST_e measMux;                 /* activated = N775_STATE_MUXMEASUREMENT_REQUEST */
    N775_STATE_REQUEST_e balancing;               /* activated = N775_STATE_BALANCECONTROL_REQUEST */
    N775_STATE_REQUEST_e balancing_feedback;      /* activated = N775_STATE_BALANCEFEEDBACK_REQUEST */
    N775_STATE_REQUEST_e measAllGpio;             /* activated = N775_STATE_ALL_GPIO_MEASUREMENT_REQUEST */
    N775_STATE_REQUEST_e userIO;                  /* activated = N775_STATE_USER_IO_REQUEST */
    N775_STATE_REQUEST_e readEEPROM;              /* activated = N775_STATE_EEPROM_READ_UID_REQUEST */
    N775_STATE_REQUEST_e measTemperature;         /* activated = N775_STATE_TEMP_SENS_READ_REQUEST */
    N775_STATE_REQUEST_e openWireCheck;           /* activated = N775_STATE_OPENWIRE_CHECK_REQUEST */
    N775_STATE_REQUEST_e deviceParameterCheck;    /* activated = N775_STATEMACH_DEVICE_PARAMETER_REQUEST */
    N775_STATE_REQUEST_e accuracyADCverification; /* activated = N775_STATEMACH_ADC_ACCURACY_REQUEST */
    N775_STATE_REQUEST_e digitalFilterCheck;      /* activated = N775_STATEMACH_DIGITAL_FILTER_REQUEST */
    uint8_t taskCycleCnt;                         /* holds the current state machine index */
    uint8_t numberActiveOfStates;                 /* number of active states */
    uint8_t activeStates[12]; /* array holds the different substates that are executed one after another */
                              /* maximum number of states : 12 */
} N775_CONFIG_s;

/**
 *
 */
typedef enum {
    N775_NOT_REUSED                        = 0,
    N775_REUSE_READ_VOLTAGE_FOR_ADOW_PUP   = 1,
    N775_REUSE_READ_VOLTAGE_FOR_ADOW_PDOWN = 2,
} N775_REUSE_MODE_e;

/** TI port expander IO direction (input or output) */
typedef enum {
    N775_PORT_EXPANDER_TI_OUTPUT = 0x0,
    N775_PORT_EXPANDER_TI_INPUT  = 0xFF,
} N775_PORT_EXPANDER_TI_DIRECTION_e;

/**
 * This structure contains all the variables relevant for the N775 state machine.
 * The user can get the current state of the N775 state machine with this variable
 */
typedef struct {
    bool firstMeasurementMade; /*!< flag that indicates if the first measurement cycle was completed */
    uint8_t currentString;     /*!< string currently being addressed */
    SPI_INTERFACE_CONFIG_s *pSpiTxSequenceStart; /*!< pointer to the start of SPI sequence to be used for Tx */
    SPI_INTERFACE_CONFIG_s *pSpiTxSequence;      /*!< pointer to the SPI sequence to be used for Tx */
    SPI_INTERFACE_CONFIG_s *pSpiRxSequenceStart; /*!< pointer to the start of SPI sequence to be used for Rx */
    SPI_INTERFACE_CONFIG_s *pSpiRxSequence;      /*!< pointer to the SPI sequence to be used for Rx */
    uint8_t currentMux[BS_NR_OF_STRINGS];        /*!< mux currently being addressed */
    N775_MUX_CH_CFG_s *pMuxSequenceStart
        [BS_NR_OF_STRINGS]; /*!< pointer to the multiplexer sequence to be measured (contains a list of elements [multiplexer id, multiplexer channels]) (1,-1)...(3,-1),(0,1),...(0,7) */
    N775_MUX_CH_CFG_s *pMuxSequence
        [BS_NR_OF_STRINGS]; /*!< pointer to the multiplexer sequence to be measured (contains a list of elements [multiplexer id, multiplexer channels]) (1,-1)...(3,-1),(0,1),...(0,7) */
    N775_DATAPTR_s n775Data;
} N775_STATE_s;

/** This structure reflects the messages used by the NXP MC33775A */
typedef struct {
    uint16_t head;
    uint16_t dataHead;
    uint16_t data[4];
    uint16_t crc;
    uint16_t dataLength;
} N775_MESSAGE_s;

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__NXP_MC33775A_DEFS_H_ */
