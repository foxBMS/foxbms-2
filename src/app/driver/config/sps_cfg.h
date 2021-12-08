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
 * @file    sps_cfg.h
 * @author  foxBMS Team
 * @date    2020-10-14 (date of creation)
 * @updated 2021-11-08 (date of last update)
 * @ingroup DRIVERS_CONF
 * @prefix  SPS
 *
 * @brief   Header for the configuration for the driver for the smart power switches
 *
 */

#ifndef FOXBMS__SPS_CFG_H_
#define FOXBMS__SPS_CFG_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "battery_system_cfg.h"

#include "sps_types.h"

/*========== Macros and Definitions =========================================*/

/** GIO defines for pin to drive reset line of SPS @{ */
#define SPS_RESET_GIO_PORT     (hetREG2->DOUT)
#define SPS_RESET_GIO_PORT_DIR (hetREG2->DIR)
#define SPS_RESET_PIN          (16u)
/**@}*/

/** GIO defines for pin to drive feedback enable MOSFET of SPS @{ */
#define SPS_FEEDBACK_GIO_PORT     (hetREG2->DOUT)
#define SPS_FEEDBACK_GIO_PORT_DIR (hetREG2->DIR)
#define SPS_FEEDBACK_PIN          (9u)
/**@}*/

/** Each SPS IC has four outputs */
#define SPS_NR_CONTACTOR_PER_IC (4u)

/** Number of SPS IC that are populated on the hardware (in daisy-chain) */
#define SPS_NR_OF_IC (2u)

/** One channel for each contactor is required in this application */
#define SPS_NR_OF_REQUIRED_CONTACTOR_CHANNELS (BS_NR_OF_CONTACTORS)

/** Calculate the number of available SPS channels */
#define SPS_NR_OF_AVAILABLE_SPS_CHANNELS (SPS_NR_CONTACTOR_PER_IC * SPS_NR_OF_IC)

/* check if the number of contactors fits in the available channels */
#if (SPS_NR_OF_REQUIRED_CONTACTOR_CHANNELS > SPS_NR_OF_AVAILABLE_SPS_CHANNELS)
#error "Not enough contactor channels available for the required number of contactors."
#endif

/** Defines the buffer size to communicate with the SPS IC.
 *
 * One 16 bit word per SPS IC so buffer size is equivalent to the number of
 * SPS ICs.
 *
 *  - 1 SPS IC -> Buffer size = 1
 *  - 2 SPS IC -> Buffer size = 2
 *  - 3 SPS IC -> Buffer size = 3
 */
#define SPS_SPI_BUFFERSIZE SPS_NR_OF_IC

/** Addresses of used control registers @{ */
#define SPS_GLOBAL_CONTROL_REGISTER_ADDRESS (0x00u)
#define SPS_OUTPUT_CONTROL_REGISTER_ADDRESS (0x02u)
#define SPS_C_CONTROL_REGISTER_ADDRESS      (0x16u)
/**@}*/
/** Addresses of used diagnostic registers @{ */
#define SPS_OD_IOUT1_DIAG_REGISTER_ADDRESS    (0x08u)
#define SPS_OD_IOUT2_DIAG_REGISTER_ADDRESS    (0x09u)
#define SPS_OD_IOUT3_DIAG_REGISTER_ADDRESS    (0x0Au)
#define SPS_OD_IOUT4_DIAG_REGISTER_ADDRESS    (0x0Bu)
#define SPS_EN_IRQ_PIN_DIAG_REGISTER_ADDRESS  (0x11u)
#define SPS_EN_WARN_PIN_DIAG_REGISTER_ADDRESS (0x13u)
/**@}*/

/**
 * These bits and positions are defined in
 * figure 7 page 10 in data sheet Rev. 2 - 11 September 2019
 */
/** This bit is set to 0 for a read register access, to 1 for a write register access */
#define SPS_RW_BIT_POSITION (15u)
/** Define for read register access */
#define SPS_RW_READ (0u)
/** Define for write register access */
#define SPS_RW_WRITE (1u)
/** In Tx SPI buffer, register address starts at bit 8 */
#define SPS_ADDRESS_BIT_START (8u)
/** This bit is set to 0 to read a diagnostic register, to 1 to read a control register */
#define SPS_DIAG_CTRL_BIT_POSITION (7u)

/** value for the SPS normal mode */
#define SPS_NORMAL_MODE (0x01u)
/** value for the SPS strong drive */
#define SPS_STRONG_DRIVE (0x00u)
/** value for the SPS medium drive */
#define SPS_MEDIUM_DRIVE (0x01u)
/** bitshift for the mode bit in the SPS
 *
 * used for bitshifting #SPS_NORMAL_MODE to the right position
 */
#define SPS_MODE_BIT_START (6u)
/** bitshift for the drive strength bit in the SPS */
#define SPS_DRIVE_STRENGTH_BIT_START (5u)

/** LSB of current measurement of SPS channel in mA */
#define SPS_I_MEASUREMENT_LSB_mA (0.98f)

/** current threshold that recognizes a contactor as closed */
#define SPS_CHANNEL_ON_DEFAULT_THRESHOLD_mA (20.0f)

/** bitmask for reading the on-demand output current from a SPI transaction */
#define SPS_BITMASK_DIAGNOSTIC_ONDEMAND_OUTPUT_CURRENT (0x1FFFu)

/** spi block identification numbers */
typedef enum SPS_WRITE_TYPE {
    SPS_replaceCurrentValue,
    SPS_orWithCurrentValue,
    SPS_andWithCurrentValue,
} SPS_WRITE_TYPE_e;

/** sps read types */
typedef enum SPS_READ_TYPE {
    SPS_READ_DIAGNOSTIC_REGISTER, /*!< diagnostic register read */
    SPS_READ_CONTROL_REGISTER,    /*!< control register read */
} SPS_READ_TYPE_e;

/** functional state of a SPS channel */
typedef enum SPS_CHANNEL_FUNCTION {
    SPS_CHANNEL_OFF, /*!< sps channel is switched off */
    SPS_CHANNEL_ON,  /*!< sps channel is switched on */
} SPS_CHANNEL_FUNCTION_e;

/** SPS channel state */
typedef struct SPS_CHANNEL_STATE {
    SPS_CHANNEL_FUNCTION_e channelRequested; /*!< requested state of the channel */
    SPS_CHANNEL_FUNCTION_e channel;          /*!< state of the channel */
    float current_mA;                        /*!< current flow in this channel in mA */
    const SPS_CHANNEL_AFFILIATION_e
        affiliation;                    /*!< affiliation of the channel (if it is contactor or something else) */
    const float thresholdFeedbackOn_mA; /*!< current threshold in mA above which the channel is considered "on" */
} SPS_CHANNEL_STATE_s;

/** SPS channel mapping to feedback pin */
typedef struct SPS_CHANNEL_FEEDBACK_MAPPING {
    const uint8_t pexDevice;  /*!< port expander number */
    const uint8_t pexChannel; /*!< pin number on the addresses port expander */
} SPS_CHANNEL_FEEDBACK_MAPPING_s;

/*================== Main precharge configuration ====================*/

/*========== Extern Constant and Variable Declarations ======================*/

/** State for the CONT handling */
typedef enum SPS_STATE {
    SPS_START,
    SPS_RESET_LOW,
    SPS_RESET_HIGH,
    SPS_CONFIGURE_CONTROL_REGISTER,
    SPS_TRIGGER_CURRENT_MEASUREMENT,
    SPS_READ_MEASURED_CURRENT1,
    SPS_READ_MEASURED_CURRENT2,
    SPS_READ_MEASURED_CURRENT3,
    SPS_READ_MEASURED_CURRENT4,
    SPS_READ_EN_IRQ_PIN,
} SPS_STATE_e;

/** Actions to do for the Smart Power Switch IC */
typedef enum SPS_ACTION {
    SPS_ACTION_CONFIGURE_CONTROL_REGISTER,
    SPS_ACTION_TRIGGER_CURRENT_MEASUREMENT,
    SPS_ACTION_READ_CURRENT_MEASUREMENT1,
    SPS_ACTION_READ_CURRENT_MEASUREMENT2,
    SPS_ACTION_READ_CURRENT_MEASUREMENT3,
    SPS_ACTION_READ_CURRENT_MEASUREMENT4,
    SPS_ACTION_READ_EN_IRQ_PIN,
} SPS_ACTION_e;

extern SPS_CHANNEL_STATE_s sps_channelStatus[SPS_NR_OF_AVAILABLE_SPS_CHANNELS];

extern const SPS_CHANNEL_FEEDBACK_MAPPING_s sps_kChannelFeedbackMapping[SPS_NR_OF_AVAILABLE_SPS_CHANNELS];

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__SPS_CFG_H_ */
