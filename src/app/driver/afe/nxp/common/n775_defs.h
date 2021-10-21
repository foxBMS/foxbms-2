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
 * @file    n775_defs.h
 * @author  foxBMS Team
 * @date    2015-09-01 (date of creation)
 * @updated 2015-09-01 (date of last update)
 * @ingroup DRIVERS
 * @prefix  N775
 *
 * @brief   Definitions for the driver for the MC33775A monitoring chip.
 *
 */

#ifndef FOXBMS__N775_DEFS_H_
#define FOXBMS__N775_DEFS_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "database.h"
#include "spi.h"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/** Command types supported by the MC33775A */
typedef enum {
    N775_CMD_WAKEUP_NOP = 0x00,
    N775_CMD_READ       = 0x01,
    N775_CMD_WRITE      = 0x02,
    N775_CMD_RESPONSE   = 0x03,
} N775_COMMAND_e;

/** MADD values supported by the MC33775A */
typedef enum {
    N775_MADD_MASTER0 = 0x00,
    N775_MADD_MASTER1 = 0x01,
} N775_MADD_e;

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
    N775_STATE_INIT_REQUEST, /*!<    */
    N775_STATE_NO_REQUEST,   /*!<    */
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
    N775_ERROR               = 20, /*!< General error state                    */
    N775_ALREADY_INITIALIZED = 30, /*!< Initialization of N775 already finished */
    N775_ILLEGAL_TASK_TYPE   = 99, /*!< Illegal                                */
} N775_RETURN_TYPE_e;

/** configuration of the mux channels */
typedef struct {
    uint8_t muxID; /*!< multiplexer ID 0 - 3       */
    uint8_t muxCh; /*!< multiplexer channel 0 - 7   */
} N775_MUX_CH_CFG_s;

/** struct that holds the multiplexer sequence */
typedef struct {
    uint8_t nr_of_steps;       /*!< number of steps in the multiplexer sequence   */
    N775_MUX_CH_CFG_s *seqptr; /*!< pointer to the multiplexer sequence   */
} N775_MUX_SEQUENCE_s;

/** This struct contains pointer to used data buffers */
typedef struct {
    DATA_BLOCK_CELL_VOLTAGE_s *cellvoltage;
    DATA_BLOCK_CELL_TEMPERATURE_s *celltemperature;
    DATA_BLOCK_MIN_MAX_s *minmax;
    DATA_BLOCK_BALANCING_FEEDBACK_s *balancing_feedback;
    DATA_BLOCK_USER_MUX_s *user_mux;
    DATA_BLOCK_BALANCING_CONTROL_s *balancing_control;
    DATA_BLOCK_SLAVE_CONTROL_s *user_io_control;
    DATA_BLOCK_OPEN_WIRE_s *openWire_check; /* Wie genau open wire check behandeln? Was genau abspeichern? */
    int *openWire_buffer;                   /* BS_NR_OF_BAT_CELLS */
    uint8_t *spi_TX_withPEC;                /* 12 byte */
    uint8_t *spi_RX_withPEC;                /* 12 byte */
    uint16_t *GPIOVoltages;                 /* N7752_NUMBER_OF_GPIOS * NR_OF_N775s */
    uint16_t *valid_GPIOPECs;               /* NR_OF_N775s */
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
    N775_STATE_REQUEST_e measVoltageSumofCells;   /* activated = N775_STATE_VOLTAGEMEASUREMENT_SC_REQUEST */
    N775_STATE_REQUEST_e measMux;                 /* activated = N775_STATE_MUXMEASUREMENT_REQUEST */
    N775_STATE_REQUEST_e balancing;               /* activated = N775_STATE_BALANCECONTROL_REQUEST */
    N775_STATE_REQUEST_e balancing_feedback;      /* activated = N775_STATE_BALANCEFEEDBACK_REQUEST */
    N775_STATE_REQUEST_e measAllGPIO;             /* activated = N775_STATE_ALLGPIOMEASUREMENT_REQUEST */
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
    N775_NOT_REUSED                    = 0,
    N775_REUSE_READVOLT_FOR_ADOW_PUP   = 1,
    N775_REUSE_READVOLT_FOR_ADOW_PDOWN = 2,
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
typedef struct N775_STATE {
    uint16_t timer; /*!< time in ms before the state machine processes the next state, e.g. in counts of 1ms */
    N775_STATE_REQUEST_e stateReq;          /*!< current state request made to the state machine */
    N775_STATEMACH_e state;                 /*!< state of Driver State Machine */
    uint8_t subState;                       /*!< current substate of the state machine */
    N775_STATEMACH_e lastState;             /*!< previous state of the state machine */
    uint8_t lastSubState;                   /*!< previous substate of the state machine */
    uint32_t errRequestCounter;             /*!< counts the number of illegal requests to the N775 state machine */
    uint8_t triggerEntry;                   /*!< counter for re-entrance protection (function running flag) */
    STD_RETURN_TYPE_e firstMeasurementMade; /*!< flag that indicates if the first measurement cycle was completed */
    STD_RETURN_TYPE_e checkSpiFlag;         /*!< indicates if interrupt flag or timer must be considered */
    STD_RETURN_TYPE_e balanceControlDone;   /*!< indicates if balance control was done */
    bool txTransmitOngoing;                 /*!< indicates if a transmission is ongoing with the daisy-chain */
    bool rxTransmitOngoing;                 /*!< indicates if a transmission is ongoing with the daisy-chain */
    uint16_t totalMessages;                 /*!< total number of messages to be received from the daisy-chain */
    uint16_t remainingMessages; /*!< counter of number of messages still to be received from the daisy-chain */
} N775_STATE_s;

/** This structure reflects the messages used by the NXP MC33775A */
typedef struct N775_MESSAGE {
    uint16_t head;
    uint16_t dataHead;
    uint16_t data[4];
    uint16_t crc;
    uint16_t dataLength;
} N775_MESSAGE_s;

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__N775_DEFS_H_ */
