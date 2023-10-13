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
 * @file    adi_ades183x_defs.h
 * @author  foxBMS Team
 * @date    2015-09-01 (date of creation)
 * @updated 2023-10-12 (date of last update)
 * @version v1.6.0
 * @ingroup DRIVERS
 * @prefix  ADI
 *
 * @brief   Headers for the driver for the ades183x analog front-end.
 *
 */

#ifndef FOXBMS__ADI_ADES183X_DEFS_H_
#define FOXBMS__ADI_ADES183X_DEFS_H_

/*========== Includes =======================================================*/
#if defined(FOXBMS_AFE_DRIVER_ADI_ADES1830)
#include "adi_ades1830_defs.h"
#endif

#include "battery_system_cfg.h"

#include "database.h"
#include "spi.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/** used to determine if a PEC fault should be injected in the register write function */
typedef enum {
    ADI_PEC_NO_FAULT_INJECTION,
    ADI_COMMAND_PEC_FAULT_INJECTION,
    ADI_DATA_PEC_FAULT_INJECTION,
} ADI_PEC_FAULT_INJECTION_e;

/** used to determine which voltage register has to be read */
typedef enum {
    ADI_CELL_VOLTAGE_REGISTER,
    ADI_AVERAGE_CELL_VOLTAGE_REGISTER,
    ADI_FILTERED_CELL_VOLTAGE_REGISTER,
    ADI_REDUNDANT_CELL_VOLTAGE_REGISTER,
    ADI_VOLTAGE_REGISTER_TYPE_E_MAX,
} ADI_VOLTAGE_REGISTER_TYPE_e;

/** used to determine where read voltages have to be stored */
typedef enum {
    ADI_CELL_VOLTAGE,
    ADI_AVERAGE_CELL_VOLTAGE,
    ADI_FILTERED_CELL_VOLTAGE,
    ADI_REDUNDANT_CELL_VOLTAGE,
    ADI_CELL_VOLTAGE_OPEN_WIRE_EVEN,
    ADI_CELL_VOLTAGE_OPEN_WIRE_ODD,
    ADI_CELL_VOLTAGE_AVERAGE_OPEN_WIRE,
    ADI_CELL_VOLTAGE_REDUNDANT_OPEN_WIRE,
    ADI_VOLTAGE_STORE_LOCATION_E_MAX,
} ADI_VOLTAGE_STORE_LOCATION_e;

/** used to determine which voltage register has to be read */
typedef enum {
    ADI_AUXILIARY_REGISTER,
    ADI_REDUNDANT_AUXILIARY_REGISTER,
    ADI_AUXILIARY_REGISTER_TYPE_E_MAX,
} ADI_AUXILIARY_REGISTER_TYPE_e;

/** used to determine where read voltages have to be stored */
typedef enum {
    ADI_AUXILIARY_VOLTAGE,
    ADI_REDUNDANT_AUXILIARY_VOLTAGE,
    ADI_AUXILIARY_VOLTAGE_OPEN_WIRE,
    ADI_AUXILIARY_STORE_LOCATION_E_MAX,
} ADI_AUXILIARY_STORE_LOCATION_e;

/* configuration sanitation */
#if BS_NR_OF_CELL_BLOCKS_PER_MODULE > ADI_MAX_SUPPORTED_CELLS
#error "Number of cell blocks per module cannot be higher than maximum number of cells per module"
#endif
#if BS_NR_OF_GPIOS_PER_MODULE != ADI_TOTAL_GPIO_NUMBER
#error "Number of GPIOs must be 10"
#endif
#if BS_NR_OF_TEMP_SENSORS_PER_MODULE > BS_NR_OF_GPIOS_PER_MODULE
#error "Number of temperature sensors cannot be higher than number of GPIOs"
#endif

/**
 * Default configuration
 * @{
 */
#define ADI_DEFAULT_CTH_COMPARISON_THRESHOLD (ADI_COMPARISON_THRESHOLD_9_1_mV)
#define ADI_DEFAULT_REFON_SETUP              (1u)
#define ADI_DEFAULT_FLAG_D_SETUP             (ADI_FLAG_D_DEFAULT)
#define ADI_DEFAULT_OWA_SETUP                (0u)
#define ADI_DEFAULT_OWRNG_SETUP              (0u)
#define ADI_DEFAULT_SOAKON_SETUP             (0u)
#define ADI_DEFAULT_GPO_1_8_SETUP            (0xFFu)
#define ADI_DEFAULT_GPO_9_10_SETUP           (0x03u)
#define ADI_DEFAULT_IIR_SETUP                (ADI_IIR_FILTER_PARAMETER_32)
#define ADI_DEFAULT_COMM_BK_SETUP            (0u)
#define ADI_DEFAULT_MUTE_ST_SETUP            (0u)
#define ADI_DEFAULT_SNAP_ST_SETUP            (0u)
#define ADI_DEFAULT_VUV_0_7_SETUP            (0u)
#define ADI_DEFAULT_VUV_8_11_SETUP           (0x8u)
#define ADI_DEFAULT_VOV_0_3_SETUP            (0xFu)
#define ADI_DEFAULT_VOV_4_11_SETUP           (0x7Fu)
#define ADI_DEFAULT_DCT0_0_5_SETUP           (0u)
#define ADI_DEFAULT_DTRNG_SETUP              (0u)
#define ADI_DEFAULT_DTMEN_SETUP              (0u)
#define ADI_DEFAULT_DCC_1_8_SETUP            (0u)
#define ADI_DEFAULT_DCC_9_16_SETUP           (0u)
/**@}*/

/* First wait this time after issuing ADAX command for all channels */
#define ADI_WAIT_TIME_1_FOR_ADAX_FULL_CYCLE (10u)
/* Retrieve voltages, then wait this time, all channels will have been measured */
#define ADI_WAIT_TIME_2_FOR_ADAX_FULL_CYCLE (8u)

/* When OW check activated for cell N, cell N voltage must be reduced compared to the
   value without check activated. This value for the reduction arises from the voltage
   divider formed by the 470 Ohm resistor activated  for the check between cell N and
   cell N-1 and the input RC filter resistances. */
#define ADI_NO_OW_CELL_VOLTAGE_REDUCTION (9.0f / 10.0f)
/* Margin for the check of the voltage reduction */
#define ADI_OW_CELL_VOLTAGE_REDUCTION_MARGIN_mV (1000.0f)
/* Margin for the check of the voltage reduction during OW latent fault check */
#define ADI_OW_LATENT_CHECK_MARGIN (0.95f)

/* Mask to select even cells, AFE starts counting from 1 */
#define ADI_ODD_CELLS_MASK (0x5555u)
/* Mask to select odd cells, AFE starts counting from 1 */
#define ADI_EVEN_CELLS_MASK (0xAAAAu)

/* TODO: check reduction */
/* When OW check activated for GPIO, GPIO voltage must be reduced compared to the
   value without check activated. This value for the reduction arises from the voltage
   divider formed by the pull-up resistor activated for the check and the input RC
   filter resistances. */
#define ADI_NO_OW_GPIO_VOLTAGE_REDUCTION (10.0f / 12.0f)
/* Margin for the check of the voltage reduction */
#define ADI_OW_GPIO_VOLTAGE_REDUCTION_MARGIN_mV (1000.0f)

/* Number of DCC bits in one byte of configuration register group B*/
#define ADI_CFGRB_NUMBER_OF_DCC_BITS_PER_BYTE (8u)

/* IC maximum wake-up time */
#define ADI_T_WAKE_us (500u)
/* IC maximum ready time */
#define ADI_T_READY_us (10u)
/* T_wake is max 500 us, wait time set for ADI_N_ADI ICs */
#define ADI_DAISY_CHAIN_WAKE_UP_TIME_us (ADI_N_ADI * ADI_T_WAKE_us)
/* T_ready is max 10 us, wait time set for ADI_N_ADI ICs */
#define ADI_DAISY_CHAIN_READY_TIME_us (ADI_N_ADI * ADI_T_READY_us)
/* Conversion coefficient for us to ms */
#define ADI_COEFFICIENT_US_TO_MS (1000u)

/** Timeout in milliseconds for interrupt-based SPI transmission. */
#define ADI_TRANSMISSION_TIMEOUT (10u)

/** isoSPI wakeup time */
#define ADI_SPI_WAKEUP_WAIT_TIME_US (100u)

/** Number of used ADI ICs */
/* Start auxiliary channel number */
#define ADI_START_AUX_CHANNEL (1u)

/** Number of used ADI ICs */
#define ADI_N_ADI (BS_NR_OF_MODULES_PER_STRING)

/* Maximum value for cell voltages */
#define ADI_MAX_CELL_VOLTAGE_mV (4500)
/* Minimum value for cell voltages */
#define ADI_MIN_CELL_VOLTAGE_mV (2000)

/* Comparison threshold between auxiliary and redundant auxiliary measurements */
#define ADI_AUXILIARY_COMPARISON_THRESHOLD_mV (9)
/* Minimum value for GPIO voltages (given in SM) */
#define ADI_MIN_GPIO_VOLTAGE_mV (100)

/* Maximum value for digital supply voltage VD */
#define ADI_MAX_VD_mV (3528.0f)
/* Minimum value for digital supply voltage VD */
#define ADI_MIN_VD_mV (2754.0f)
/* Maximum value for analog supply voltage VA */
#define ADI_MAX_VA_mV (5486.0f)
/* Minimum value for analog supply voltage VA */
#define ADI_MIN_VA_mV (4512.0f)
/* Maximum value for secondary reference voltage VREF2 */
#define ADI_MAX_VREF2_mV (3015.0f)
/* Minimum value for secondary reference voltage VREF2 */
#define ADI_MIN_VREF2_mV (2985.0f)
/* Maximum value for die temperature ITMP */
#define ADI_MAX_ITMP_degC (125.0f)
/* Minimum value for die temperature ITMP */
#define ADI_MIN_ITMP_degC (-40.0f)

/* Threshold for diagnostic when oscillator is set to run faster  */
#define ADI_OSCILLATOR_FASTER_THRESHOLD (70u)
/* Threshold for diagnostic when oscillator is set to run slower  */
#define ADI_OSCILLATOR_SLOWER_THRESHOLD (52u)

/**
 * Values used for PEC latent fault check
 * @{
 */
#define ADI_PEC_LATENT_FAULT_CHECK_TESTDATA0_BYTE0 (1u)
#define ADI_PEC_LATENT_FAULT_CHECK_TESTDATA0_BYTE1 (2u)
#define ADI_PEC_LATENT_FAULT_CHECK_TESTDATA0_BYTE2 (3u)
#define ADI_PEC_LATENT_FAULT_CHECK_TESTDATA0_BYTE3 (4u)
#define ADI_PEC_LATENT_FAULT_CHECK_TESTDATA0_BYTE4 (5u)
#define ADI_PEC_LATENT_FAULT_CHECK_TESTDATA0_BYTE5 (6u)
#define ADI_PEC_LATENT_FAULT_CHECK_TESTDATA1_BYTE0 (7u)
#define ADI_PEC_LATENT_FAULT_CHECK_TESTDATA1_BYTE1 (8u)
#define ADI_PEC_LATENT_FAULT_CHECK_TESTDATA1_BYTE2 (9u)
#define ADI_PEC_LATENT_FAULT_CHECK_TESTDATA1_BYTE3 (10u)
#define ADI_PEC_LATENT_FAULT_CHECK_TESTDATA1_BYTE4 (11u)
#define ADI_PEC_LATENT_FAULT_CHECK_TESTDATA1_BYTE5 (12u)
#define ADI_PEC_LATENT_FAULT_CHECK_TESTDATA2_BYTE0 (13u)
#define ADI_PEC_LATENT_FAULT_CHECK_TESTDATA2_BYTE1 (14u)
#define ADI_PEC_LATENT_FAULT_CHECK_TESTDATA2_BYTE2 (15u)
#define ADI_PEC_LATENT_FAULT_CHECK_TESTDATA2_BYTE3 (16u)
#define ADI_PEC_LATENT_FAULT_CHECK_TESTDATA2_BYTE4 (17u)
#define ADI_PEC_LATENT_FAULT_CHECK_TESTDATA2_BYTE5 (18u)
/**@}*/

/**
 * Values to write in FLAG_D bits in configuration register A
 * to activate diagnostics
 * @{
 */
#define ADI_FLAG_D_DEFAULT          (0x00u) /* 0b00000000u */
#define ADI_FLAG_D_UV_CHECK         (0x04u) /* 0b00000100u */
#define ADI_FLAG_D_OV_CHECK         (0x0Cu) /* 0b00001100u */
#define ADI_FLAG_D_OSC_FASTER       (0x01u) /* 0b00000001u */
#define ADI_FLAG_D_OSC_SLOWER       (0x02u) /* 0b00000010u */
#define ADI_FLAG_D_FORCE_THSD       (0x10u) /* 0b00010000u */
#define ADI_FLAG_D_FORCE_TMODCHK    (0x80u) /* 0b10000000u */
#define ADI_FLAG_D_INDUCE_ECC_ERROR (0x60u) /* 0b01100000u */
/**@}*/

/**
 * Maximum queue timeout time in milliseconds
 */
#define ADI_QUEUE_TIMEOUT_MS ((TickType_t)0u)

/**
 * voltage conversion coefficients
 * @{
 */
#define ADI_VOLTAGE_CONVERSION_FACTOR (150e-6f)
#define ADI_VOLTAGE_CONVERSION_UNIT   (1000.0f)
#define ADI_VOLTAGE_CONVERSION_OFFSET (1500.0f)
/**@}*/

/**
 * die temperature conversion coefficients
 * @{
 */
#define ADI_DIE_TEMPERATURE_VOLTAGE_OFFSET (1.5f)
#define ADI_DIE_TEMPERATURE_KELVIN_OFFSET  (273.0f)
#define ADI_DIE_TEMPERATURE_SCALING_FACTOR (7.5f)
/**@}*/

/* Define to shift a full byte*/
#define ADI_BYTE_SHIFT (8u)
/* Define to select complete PEC10 without command counter */
#define ADI_PEC10_FULL_EXCLUDE_COMMAND_COUNTER (0x3Fu)
/* Define to select PEC10 bit8 and bit9 without command counter */
#define ADI_PEC10_MSB_EXCLUDE_COMMAND_COUNTER (0x3u)
/* Mask to select one byte */
#define ADI_ONE_BYTE_MASK (0xFFu)
/* Maximum size of a register */
#define ADI_MAX_REGISTER_SIZE_IN_BYTES (6u)
/* Size of command plus associated PEC in bytes */
#define ADI_COMMAND_AND_PEC_SIZE_IN_BYTES (4u)
/* Size of command in bytes */
#define ADI_COMMAND_SIZE_IN_BYTES (2u)
/* Size of PEC in bytes */
#define ADI_PEC_SIZE_IN_BYTES (2u)
/* Maximum position of a bit in a bytes */
#define ADI_MAX_BIT_POSITION_IN_BYTE (7u)
/* Size of a raw voltage measurement in bytes*/
#define ADI_RAW_VOLTAGE_SIZE_IN_BYTES (2u)
/* Maximum number of voltages per register */
#define ADI_MAX_NUMBER_OF_VOLTAGES_IN_REGISTER (3u)
/* Maximum number of GPIO voltages per register */
#define ADI_MAX_NUMBER_OF_GPIO_VOLTAGES_IN_REGISTER (3u)
/* Number of GPIO voltages in AUX register D */
#define ADI_NUMBER_OF_GPIO_VOLTAGES_IN_REGISTER_D (1u)
/* Number of bytes used to compute PEC */
#define ADI_SIZE_OF_DATA_FOR_PEC_COMPUTATION (6u)
/* Number of bytes used to compute PEC */
#define ADI_SIZE_OF_DATA_FOR_PEC_COMPUTATION_WITH_COUNTER (7u)

/**
 * Number of Bytes to be transmitted in daisy-chain
 * For first 4 Bytes:
 *  - 2 Bytes: command
 *  - 2 Bytes: CRC
 * Following Bytes: Data
 *  - 6 Bytes data per IC
 *  - 2 Bytes CRC per IC
 */
#define ADI_N_BYTES_FOR_DATA_TRANSMISSION \
    (ADI_COMMAND_AND_PEC_SIZE_IN_BYTES + ((ADI_MAX_REGISTER_SIZE_IN_BYTES + ADI_PEC_SIZE_IN_BYTES) * ADI_N_ADI))

/**
 * Offset to select voltages of specific voltage register
 * in a table containing all voltages in a linear fashion
 * @{
 */
#define ADI_VOLTAGE_00_02_OFFSET (0u)
#define ADI_VOLTAGE_03_05_OFFSET (3u)
#define ADI_VOLTAGE_06_08_OFFSET (6u)
#define ADI_VOLTAGE_09_11_OFFSET (9u)
#define ADI_VOLTAGE_12_14_OFFSET (12u)
#define ADI_VOLTAGE_15_18_OFFSET (15u)
/**@}*/

/**
 * position of command bytes and PEC bytes in frame
 * @{
 */
#define ADI_COMMAND_FIRST_BYTE_POSITION      (0u)
#define ADI_COMMAND_SECOND_BYTE_POSITION     (1u)
#define ADI_COMMAND_PEC_FIRST_BYTE_POSITION  (2u)
#define ADI_COMMAND_PEC_SECOND_BYTE_POSITION (3u)
/**@}*/

/**
 * position of PEC bytes in data frame
 * @{
 */
#define ADI_DATA_PEC_FIRST_BYTE_POSITION  (0u)
#define ADI_DATA_PEC_SECOND_BYTE_POSITION (1u)
/**@}*/

/**
 * COMM definitions
 * @{
 */
#define ADI_ICOM_START            (0x60u)
#define ADI_ICOM_STOP             (0x10u)
#define ADI_ICOM_BLANK            (0x00u)
#define ADI_ICOM_NO_TRANSMIT      (0x70u)
#define ADI_FCOM_MASTER_ACK       (0x00u)
#define ADI_FCOM_MASTER_NACK      (0x08u)
#define ADI_FCOM_MASTER_NACK_STOP (0x09u)
/**@}*/

/* RequirementId: D7.1 V0R4 SIF-4.30.03.01 */
/** error table for the driver */
typedef struct {
    bool spiIsOk[BS_NR_OF_STRINGS];
    /* RequirementId: D7.1 V0R4 SIF-4.10.02.02 */
    /* RequirementId: D7.1 V0R4 SIF-4.20.03.01 */
    bool crcIsOk[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool commandCounterIsOk[BS_NR_OF_STRINGS][ADI_N_ADI];
    /* RequirementId: D7.1 V0R4 SIF-4.20.02.01 */
    bool configurationAIsOk[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool configurationBIsOk[BS_NR_OF_STRINGS][ADI_N_ADI];
    /* RequirementId: D7.1 V0R4 SIF-4.40.01.03 */
    bool voltageRegisterContentIsNotStuck[BS_NR_OF_STRINGS][ADI_N_ADI];
    /* RequirementId: D7.1 V0R4 SIF-4.40.02.03 */
    bool auxiliaryRegisterContentIsNotStuck[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool adcComparisonIsOk[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool compFlagIsCleared[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool spiFltFlagIsCleared[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool filteredVoltageRangeIsOk[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool evenWireIsOk[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool oddWireIsOk[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool evenWireHasNoLatentFault[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool oddWireHasNoLatentFault[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool adcCompFlagsHaveNoLatentFault[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool auxComparisonIsOk[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool auxVoltageRangeIsOk[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool analogSupplyOvervoltageFlagIsCleared[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool analogSupplyUndervoltageFlagIsCleared[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool digitalSupplyOvervoltageFlagIsCleared[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool digitalSupplyUndervoltageFlagIsCleared[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool vdeFlagIsCleared[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool oscchkFlagIsCleared[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool analogSupplyOvervoltageFlagHasNoLatentFault[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool analogSupplyUndervoltageFlagHasNoLatentFault[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool digitalSupplyOvervoltageFlagHasNoLatentFault[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool digitalSupplyUndervoltageFlagHasNoLatentFault[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool vdeFlagHasNoLatentFault[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool vdelFlagHasNoLatentFault[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool analogSupplyRangeIsOk[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool digitalSupplyRangeIsOk[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool vresRangeIsOk[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool dieTemperatureIsOk[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool oscchkFlagHasNoLatentFault[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool ocCounterFasterFlagHasNoLatentFault[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool ocCounterSlowerFlagHasNoLatentFault[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool ocCounterNormalFlagHasNoLatentFault[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool thsdFlagHasNoLatentFault[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool tmodchkFlagHasNoLatentFault[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool tmodchkFlagIsCleared[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool cmedFlagIsCleared[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool smedFlagIsCleared[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool sleepFlagIsCleared[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool cmedFlagHasNoLatentFault[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool smedFlagHasNoLatentFault[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool initResetAuxValuesAreOk[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool initClearedAuxValuesAreOk[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool pecCheckHasNoLatentFault[BS_NR_OF_STRINGS][ADI_N_ADI];
    bool compareLogicFaultFlagHasNoLatentFault[BS_NR_OF_STRINGS][ADI_N_ADI];
    /* RequirementId: D7.1 V0R4 SIF-4.40.01.03 */
    bool noClockFreezeDetectorIsOk[BS_NR_OF_STRINGS][ADI_N_ADI];
} ADI_ERROR_TABLE_s;

/** Define used to access bytes in answer frames */
#define ADI_FIRST_DATA_BYTE_POSITION_IN_TRANSMISSION_FRAME (4u)

/** Defines to access voltage register set */
/**@{*/
#define ADI_RESULT_REGISTER_SET_A (0u)
#define ADI_RESULT_REGISTER_SET_B (1u)
#define ADI_RESULT_REGISTER_SET_C (2u)
#define ADI_RESULT_REGISTER_SET_D (3u)
#define ADI_RESULT_REGISTER_SET_E (4u)
#define ADI_RESULT_REGISTER_SET_F (5u)
/**@}*/

/** Generic defines to access the bytes of a register set */
/**@{*/
#define ADI_REGISTER_OFFSET0 (0u)
#define ADI_REGISTER_OFFSET1 (1u)
#define ADI_REGISTER_OFFSET2 (2u)
#define ADI_REGISTER_OFFSET3 (3u)
#define ADI_REGISTER_OFFSET4 (4u)
#define ADI_REGISTER_OFFSET5 (5u)
/**@}*/

/* 16 bit-values in registers are often stored as two bytes, so the higher byte
   must be shifted by 8 bits to reconstruct the value */
#define ADI_DATA_BUFFER_OFFSET_IN_BITS (8u)

/** Defines for to access auxiliary register set */
/**@{*/
#define ADI_AUXILIARY_RESULT_REGISTER_SET_A (0u)
#define ADI_AUXILIARY_RESULT_REGISTER_SET_B (1u)
#define ADI_AUXILIARY_RESULT_REGISTER_SET_C (2u)
#define ADI_AUXILIARY_RESULT_REGISTER_SET_D (3u)
#define ADI_AUXILIARY_RESULT_REGISTER_SET_E (4u)
/**@}*/

/** Mask to select command counter value in received data */
#define ADI_COMMAND_COUNTER_MASK (0xFCu)
/** Position of command counter in answer byte */
#define ADI_COMMAND_COUNTER_POSITION (2u)

/** Command counter loops back to start value when incremented further after having reached this value */
#define ADI_COMMAND_COUNTER_MAX_VALUE (63u)
/** Command counter loops back to this value when incremented further after having reached max value */
#define ADI_COMMAND_COUNTER_RESTART_VALUE (1u)
/** Command counter loops set to this value after power cycling, transition to sleep or with RSTCC command */
#define ADI_COMMAND_COUNTER_RESET_VALUE (0u)

/** In command definitions, first byte value put at first position */
#define ADI_COMMAND_BYTE0_POSITION (0u)
/** In command definitions, second byte value put at second position */
#define ADI_COMMAND_BYTE1_POSITION (1u)
/** In command definitions, INC value put at third position */
#define ADI_COMMAND_INC_POSITION (2u)
/** In definition of command, position of the length of the data */
#define ADI_COMMAND_DATA_LENGTH_POSITION (3u)

/* C-ADC vs S-ADC comparison voltage threshold */
#define ADI_COMPARISON_THRESHOLD_5_1_mV   (0x00u) /* 0b000u */
#define ADI_COMPARISON_THRESHOLD_8_1_mV   (0x01u) /* 0b001u */
#define ADI_COMPARISON_THRESHOLD_9_1_mV   (0x02u) /* 0b010u */
#define ADI_COMPARISON_THRESHOLD_10_05_mV (0x03u) /* 0b011u */
#define ADI_COMPARISON_THRESHOLD_15_mV    (0x04u) /* 0b100u */
#define ADI_COMPARISON_THRESHOLD_19_95_mV (0x05u) /* 0b101u */
#define ADI_COMPARISON_THRESHOLD_25_05_mV (0x06u) /* 0b110u */
#define ADI_COMPARISON_THRESHOLD_45_05_mV (0x07u) /* 0b111u */

/* IIR filter setting */
#define ADI_IIR_FILTER_DISABLED      (0x00u) /* 0b000u */
#define ADI_IIR_FILTER_PARAMETER_2   (0x01u) /* 0b001u */
#define ADI_IIR_FILTER_PARAMETER_4   (0x02u) /* 0b010u */
#define ADI_IIR_FILTER_PARAMETER_8   (0x03u) /* 0b011u */
#define ADI_IIR_FILTER_PARAMETER_16  (0x04u) /* 0b100u */
#define ADI_IIR_FILTER_PARAMETER_32  (0x05u) /* 0b101u */
#define ADI_IIR_FILTER_PARAMETER_128 (0x06u) /* 0b110u */
#define ADI_IIR_FILTER_PARAMETER_256 (0x07u) /* 0b111u */

/** Time to wait for the IIR filter to settle after it was reset and preloaded with samples */
#define ADI_IIR_SETTLING_TIME_ms (5u)

/** Time to wait after restarting measurements */
#define ADI_MEASUREMENT_RESTART_WAIT_TIME_ms (1u)

/** Time to wait after stopping measurements */
#define ADI_MEASUREMENT_STOP_WAIT_TIME_ms (1u)

/** Time to balance after having stopped measurement and before restarting them */
#define ADI_BALANCING_TIME_ms (20u)

/* Time in ms needed for a redundant ADC measurement */
#define ADI_REDUNDANT_ADC_MEASUREMENT_TIME_ms (8u)

/* Time in ms needed for a redundant ADC measurement in OW latent fault check */
#define ADI_OW_LATENT_FAULT_MEASUREMENT_TIME_ms (16u)

/* Commands are made out of 11 bits according to data sheet */
#define ADI_COMMAND_CODE_LENGTH (11u)

/* Each command is defined through 4 bytes: 2 command bytes, 1 command counter byte, 1 length byte */
#define ADI_COMMAND_DEFINITION_LENGTH (4u)

/* Value used as initialization when creating a mask to set configuration bits in commands */
#define ADI_COMMAND_MASK_SEED (0xFFFFu)

/* Value used as initialization when creating a mask to set bits in data written to the AFEs */
#define ADI_DATA_MASK_SEED (0xFFFFu)

/* Reset value of registers */
#define ADI_REGISTER_POR_VALUE (0x7FFFu)

/* Value of registers after a clear command was issued */
#define ADI_REGISTER_CLEARED_VALUE (0x8000u)

/* Time to wait for reference to be on, max 4.5ms */
#define ADI_TREFUP_ms (5u)

/* Time to wait after soft reset */
#define ADI_TSOFTRESET_ms (50u)

/**
 * ades183x command definitions
 * @{
 */
/** Start cell voltage conversion */
#define ADI_ADCV_BYTE0 (0x02u) /* Table 52 - */
#define ADI_ADCV_BYTE1 (0x60u)
#define ADI_ADCV_INC   (1u)
#define ADI_ADCV_LEN   (0u)
/**/
#define ADI_ADCV_RD_POS   (8u)
#define ADI_ADCV_RD_LEN   (1u)
#define ADI_ADCV_CONT_POS (7u)
#define ADI_ADCV_CONT_LEN (1u)
#define ADI_ADCV_DCP_POS  (4u)
#define ADI_ADCV_DCP_LEN  (1u)
#define ADI_ADCV_RSTF_POS (2u)
#define ADI_ADCV_RSTF_LEN (1u)
#define ADI_ADCV_OW01_POS (0u)
#define ADI_ADCV_OW01_LEN (2u)
/**/
/** Start redundant cell voltage conversion */
#define ADI_ADSV_BYTE0 (0x01u)
#define ADI_ADSV_BYTE1 (0x68u)
#define ADI_ADSV_INC   (1u)
#define ADI_ADSV_LEN   (0u)
/**/
#define ADI_ADSV_CONT_POS (7u)
#define ADI_ADSV_CONT_LEN (1u)
#define ADI_ADSV_DCP_POS  (4u)
#define ADI_ADSV_DCP_LEN  (1u)
#define ADI_ADSV_OW01_POS (0u)
#define ADI_ADSV_OW01_LEN (2u)
/** Start GPIO voltage conversion */
#define ADI_ADAX_BYTE0 (0x04u)
#define ADI_ADAX_BYTE1 (0x10u)
#define ADI_ADAX_INC   (1u)
#define ADI_ADAX_LEN   (0u)
/**/
#define ADI_ADAX_OW_POS   (8u)
#define ADI_ADAX_OW_LEN   (1u)
#define ADI_ADAX_PUP_POS  (7u)
#define ADI_ADAX_PUP_LEN  (1u)
#define ADI_ADAX_CH4_POS  (6u)
#define ADI_ADAX_CH4_LEN  (1u)
#define ADI_ADAX_CH03_POS (0u)
#define ADI_ADAX_CH03_LEN (4u)
/** Start redundant GPIO voltage conversion */
#define ADI_ADAX2_BYTE0 (0x04u)
#define ADI_ADAX2_BYTE1 (0x00u)
#define ADI_ADAX2_INC   (1u)
#define ADI_ADAX2_LEN   (0u)
/**/
#define ADI_ADAX2_CH03_POS (0u)
#define ADI_ADAX2_CH03_LEN (4u)
/** Write configuration registers */
#define ADI_WRCFGA_BYTE0 (0x00u)
#define ADI_WRCFGA_BYTE1 (0x01u)
#define ADI_WRCFGA_INC   (1u)
#define ADI_WRCFGA_LEN   (6u)
/**/
#define ADI_WRCFGB_BYTE0 (0x00u)
#define ADI_WRCFGB_BYTE1 (0x24u)
#define ADI_WRCFGB_INC   (1u)
#define ADI_WRCFGB_LEN   (6u)
/** Read configuration registers */
#define ADI_RDCFGA_BYTE0 (0x00u)
#define ADI_RDCFGA_BYTE1 (0x02u)
#define ADI_RDCFGA_INC   (0u)
#define ADI_RDCFGA_LEN   (6u)
/**/
#define ADI_RDCFGB_BYTE0 (0x00u)
#define ADI_RDCFGB_BYTE1 (0x26u)
#define ADI_RDCFGB_INC   (0u)
#define ADI_RDCFGB_LEN   (6u)
/** Read cell voltage registers */
#define ADI_RDCVA_BYTE0 (0x00u)
#define ADI_RDCVA_BYTE1 (0x04u)
#define ADI_RDCVA_INC   (0u)
#define ADI_RDCVA_LEN   (6u)
/**/
#define ADI_RDCVB_BYTE0 (0x00u)
#define ADI_RDCVB_BYTE1 (0x06u)
#define ADI_RDCVB_INC   (0u)
#define ADI_RDCVB_LEN   (6u)
/**/
#define ADI_RDCVC_BYTE0 (0x00u)
#define ADI_RDCVC_BYTE1 (0x08u)
#define ADI_RDCVC_INC   (0u)
#define ADI_RDCVC_LEN   (6u)
/**/
#define ADI_RDCVD_BYTE0 (0x00u)
#define ADI_RDCVD_BYTE1 (0x0Au)
#define ADI_RDCVD_INC   (0u)
#define ADI_RDCVD_LEN   (6u)
/**/
#define ADI_RDCVE_BYTE0 (0x00u)
#define ADI_RDCVE_BYTE1 (0x09u)
#define ADI_RDCVE_INC   (0u)
#define ADI_RDCVE_LEN   (6u)
/**/
#define ADI_RDCVF_BYTE0 (0x00u)
#define ADI_RDCVF_BYTE1 (0x0Bu)
#define ADI_RDCVF_INC   (0u)
#define ADI_RDCVF_LEN   (6u)
/** Read averaged voltage registers */
#define ADI_RDACA_BYTE0 (0x00u)
#define ADI_RDACA_BYTE1 (0x44u)
#define ADI_RDACA_INC   (0u)
#define ADI_RDACA_LEN   (6u)
/**/
#define ADI_RDACB_BYTE0 (0x00u)
#define ADI_RDACB_BYTE1 (0x46u)
#define ADI_RDACB_INC   (0u)
#define ADI_RDACB_LEN   (6u)
/**/
#define ADI_RDACC_BYTE0 (0x00u)
#define ADI_RDACC_BYTE1 (0x48u)
#define ADI_RDACC_INC   (0u)
#define ADI_RDACC_LEN   (6u)
/**/
#define ADI_RDACD_BYTE0 (0x00u)
#define ADI_RDACD_BYTE1 (0x4Au)
#define ADI_RDACD_INC   (0u)
#define ADI_RDACD_LEN   (6u)
/**/
#define ADI_RDACE_BYTE0 (0x00u)
#define ADI_RDACE_BYTE1 (0x49u)
#define ADI_RDACE_INC   (0u)
#define ADI_RDACE_LEN   (6u)
/**/
#define ADI_RDACF_BYTE0 (0x00u)
#define ADI_RDACF_BYTE1 (0x4Bu)
#define ADI_RDACF_INC   (0u)
#define ADI_RDACF_LEN   (6u)
/** Read filtered voltage registers */
#define ADI_RDFCA_BYTE0 (0x00u)
#define ADI_RDFCA_BYTE1 (0x12u)
#define ADI_RDFCA_INC   (0u)
#define ADI_RDFCA_LEN   (6u)
/**/
#define ADI_RDFCB_BYTE0 (0x00u)
#define ADI_RDFCB_BYTE1 (0x13u)
#define ADI_RDFCB_INC   (0u)
#define ADI_RDFCB_LEN   (6u)
/**/
#define ADI_RDFCC_BYTE0 (0x00u)
#define ADI_RDFCC_BYTE1 (0x14u)
#define ADI_RDFCC_INC   (0u)
#define ADI_RDFCC_LEN   (6u)
/**/
#define ADI_RDFCD_BYTE0 (0x00u)
#define ADI_RDFCD_BYTE1 (0x15u)
#define ADI_RDFCD_INC   (0u)
#define ADI_RDFCD_LEN   (6u)
/**/
#define ADI_RDFCE_BYTE0 (0x00u)
#define ADI_RDFCE_BYTE1 (0x16u)
#define ADI_RDFCE_INC   (0u)
#define ADI_RDFCE_LEN   (6u)
/**/
#define ADI_RDFCF_BYTE0 (0x00u)
#define ADI_RDFCF_BYTE1 (0x17u)
#define ADI_RDFCF_INC   (0u)
#define ADI_RDFCF_LEN   (6u)
/** Read redundant voltage registers */
#define ADI_RDSVA_BYTE0 (0x00u)
#define ADI_RDSVA_BYTE1 (0x03u)
#define ADI_RDSVA_INC   (0u)
#define ADI_RDSVA_LEN   (6u)
/**/
#define ADI_RDSVB_BYTE0 (0x00u)
#define ADI_RDSVB_BYTE1 (0x05u)
#define ADI_RDSVB_INC   (0u)
#define ADI_RDSVB_LEN   (6u)
/**/
#define ADI_RDSVC_BYTE0 (0x00u)
#define ADI_RDSVC_BYTE1 (0x07u)
#define ADI_RDSVC_INC   (0u)
#define ADI_RDSVC_LEN   (6u)
/**/
#define ADI_RDSVD_BYTE0 (0x00u)
#define ADI_RDSVD_BYTE1 (0x0Du)
#define ADI_RDSVD_INC   (0u)
#define ADI_RDSVD_LEN   (6u)
/**/
#define ADI_RDSVE_BYTE0 (0x00u)
#define ADI_RDSVE_BYTE1 (0x0Eu)
#define ADI_RDSVE_INC   (0u)
#define ADI_RDSVE_LEN   (6u)
/**/
#define ADI_RDSVF_BYTE0 (0x00u)
#define ADI_RDSVF_BYTE1 (0x0Fu)
#define ADI_RDSVF_INC   (0u)
#define ADI_RDSVF_LEN   (6u)
/** Snapshot  commands */
#define ADI_SNAPSHOT_BYTE0   (0x00u)
#define ADI_SNAPSHOT_BYTE1   (0x2Du)
#define ADI_SNAPSHOT_INC     (1u)
#define ADI_SNAPSHOT_LEN     (0u)
#define ADI_UNSNAPSHOT_BYTE0 (0x00u)
#define ADI_UNSNAPSHOT_BYTE1 (0x2Fu)
#define ADI_UNSNAPSHOT_INC   (1u)
#define ADI_UNSNAPSHOT_LEN   (0u)
/** Read auxiliary registers */
#define ADI_RDAUXA_BYTE0 (0x00u)
#define ADI_RDAUXA_BYTE1 (0x19u)
#define ADI_RDAUXA_INC   (0u)
#define ADI_RDAUXA_LEN   (6u)
/**/
#define ADI_RDAUXB_BYTE0 (0x00u)
#define ADI_RDAUXB_BYTE1 (0x1Au)
#define ADI_RDAUXB_INC   (0u)
#define ADI_RDAUXB_LEN   (6u)
/**/
#define ADI_RDAUXC_BYTE0 (0x00u)
#define ADI_RDAUXC_BYTE1 (0x1Bu)
#define ADI_RDAUXC_INC   (0u)
#define ADI_RDAUXC_LEN   (6u)
/**/
#define ADI_RDAUXD_BYTE0 (0x00u)
#define ADI_RDAUXD_BYTE1 (0x1Fu)
#define ADI_RDAUXD_INC   (0u)
#define ADI_RDAUXD_LEN   (6u)
/**/
#define ADI_RDAUXE_BYTE0 (0x00u)
#define ADI_RDAUXE_BYTE1 (0x36u)
#define ADI_RDAUXE_INC   (0u)
#define ADI_RDAUXE_LEN   (6u)
/** Read redundant auxiliary registers */
#define ADI_RDRAXA_BYTE0 (0x00u)
#define ADI_RDRAXA_BYTE1 (0x1Cu)
#define ADI_RDRAXA_INC   (0u)
#define ADI_RDRAXA_LEN   (6u)
/**/
#define ADI_RDRAXB_BYTE0 (0x00u)
#define ADI_RDRAXB_BYTE1 (0x1Du)
#define ADI_RDRAXB_INC   (0u)
#define ADI_RDRAXB_LEN   (6u)
/**/
#define ADI_RDRAXC_BYTE0 (0x00u)
#define ADI_RDRAXC_BYTE1 (0x1Eu)
#define ADI_RDRAXC_INC   (0u)
#define ADI_RDRAXC_LEN   (6u)
/**/
#define ADI_RDRAXD_BYTE0 (0x00u)
#define ADI_RDRAXD_BYTE1 (0x25u)
#define ADI_RDRAXD_INC   (0u)
#define ADI_RDRAXD_LEN   (6u)
/** Clear AUX registers */
#define ADI_CLRAUX_BYTE0 (0x07u)
#define ADI_CLRAUX_BYTE1 (0x12u)
#define ADI_CLRAUX_INC   (1u)
#define ADI_CLRAUX_LEN   (0u)
/** Write PWM registers */
#define ADI_WRPWMA_BYTE0 (0x00u)
#define ADI_WRPWMA_BYTE1 (0x20u)
#define ADI_WRPWMA_INC   (1u)
#define ADI_WRPWMA_LEN   (6u)
#define ADI_WRPWMB_BYTE0 (0x00u)
#define ADI_WRPWMB_BYTE1 (0x21u)
#define ADI_WRPWMB_INC   (1u)
#define ADI_WRPWMB_LEN   (6u)
/** Read PWM registers */
#define ADI_RDPWMA_BYTE0 (0x00u)
#define ADI_RDPWMA_BYTE1 (0x22u)
#define ADI_RDPWMA_INC   (0u)
#define ADI_RDPWMA_LEN   (6u)
#define ADI_RDPWMB_BYTE0 (0x00u)
#define ADI_RDPWMB_BYTE1 (0x23u)
#define ADI_RDPWMB_INC   (0u)
#define ADI_RDPWMB_LEN   (6u)
/** Mute discharge */
#define ADI_MUTE_BYTE0 (0x00u)
#define ADI_MUTE_BYTE1 (0x28u)
#define ADI_MUTE_INC   (1u)
#define ADI_MUTE_LEN   (0u)
/** Unmute discharge */
#define ADI_UNMUTE_BYTE0 (0x00u)
#define ADI_UNMUTE_BYTE1 (0x29u)
#define ADI_UNMUTE_INC   (1u)
#define ADI_UNMUTE_LEN   (0u)
/** Clear command counter */
#define ADI_RSTCC_BYTE0 (0x00u)
#define ADI_RSTCC_BYTE1 (0x2Eu)
#define ADI_RSTCC_INC   (0u)
#define ADI_RSTCC_LEN   (0u)
/** Clear cell registers */
#define ADI_CLRCELL_BYTE0 (0x07u)
#define ADI_CLRCELL_BYTE1 (0x11u)
#define ADI_CLRCELL_INC   (1u)
#define ADI_CLRCELL_LEN   (0u)
/** Clear flags in Status Register Group C */
#define ADI_CLRFLAG_BYTE0 (0x07u)
#define ADI_CLRFLAG_BYTE1 (0x17u)
#define ADI_CLRFLAG_INC   (1u)
#define ADI_CLRFLAG_LEN   (6u)
/**/
#define ADI_CLRFLAG_DATA_LENGTH (6u)
/** Read status register */
#define ADI_RDSTATA_BYTE0 (0x00u)
#define ADI_RDSTATA_BYTE1 (0x30u)
#define ADI_RDSTATA_INC   (0u)
#define ADI_RDSTATA_LEN   (6u)
/**/
#define ADI_RDSTATB_BYTE0 (0x00u)
#define ADI_RDSTATB_BYTE1 (0x31u)
#define ADI_RDSTATB_INC   (0u)
#define ADI_RDSTATB_LEN   (6u)
/**/
#define ADI_RDSTATC_BYTE0 (0x00u)
#define ADI_RDSTATC_BYTE1 (0x32u)
#define ADI_RDSTATC_INC   (0u)
#define ADI_RDSTATC_LEN   (6u)
/**/
#define ADI_RDSTATC_ERR_POS (6u)
#define ADI_RDSTATC_ERR_LEN (1u)
/**/
#define ADI_RDSTATD_BYTE0 (0x00u)
#define ADI_RDSTATD_BYTE1 (0x33u)
#define ADI_RDSTATD_INC   (0u)
#define ADI_RDSTATD_LEN   (6u)
/**/
#define ADI_RDSTATE_BYTE0 (0x00u)
#define ADI_RDSTATE_BYTE1 (0x34u)
#define ADI_RDSTATE_INC   (0u)
#define ADI_RDSTATE_LEN   (6u)
/** Read serial ID */
#define ADI_RDSID_BYTE0 (0x00u)
#define ADI_RDSID_BYTE1 (0x2Cu)
#define ADI_RDSID_INC   (0u)
#define ADI_RDSID_LEN   (6u)
/** Soft Reset */
#define ADI_SRST_BYTE0 (0x00u)
#define ADI_SRST_BYTE1 (0x27u)
#define ADI_SRST_INC   (0u)
#define ADI_SRST_LEN   (0u)
/**@}*/

/* Configuration Register A */
/* CFGRA0 */
#define ADI_CFGRA0_CTH_0_2_POS  (0u)
#define ADI_CFGRA0_CTH_0_2_MASK (0x7u)
#define ADI_CFGRA0_REFON_POS    (7u)
#define ADI_CFGRA0_REFON_MASK   (0x80u)
/* CFGRA1 */
#define ADI_CFGRA1_FLAG_D_0_7_POS  (0u)
#define ADI_CFGRA1_FLAG_D_0_7_MASK (0xFFu)
/* CFGRA2 */
#define ADI_CFGRA2_OWA_0_2_POS  (3u)
#define ADI_CFGRA2_OWA_0_2_MASK (0x38u)
#define ADI_CFGRA2_OWRNG_POS    (6u)
#define ADI_CFGRA2_OWRNG_MASK   (0x40u)
#define ADI_CFGRA2_SOAKON_POS   (7u)
#define ADI_CFGRA2_SOAKON_MASK  (0x80u)
/* CFGRA3 */
#define ADI_CFGRA3_GPO_1_8_POS  (0u)
#define ADI_CFGRA3_GPO_1_8_MASK (0xFFu)
/* CFGRA4 */
#define ADI_CFGRA4_GPO_9_10_POS  (0u)
#define ADI_CFGRA4_GPO_9_10_MASK (0x3u)
/* CFGRA5 */
#define ADI_CFGRA5_FC_0_2_POS   (0u)
#define ADI_CFGRA5_FC_0_2_MASK  (0x7u)
#define ADI_CFGRA5_COMM_BK_POS  (3u)
#define ADI_CFGRA5_COMM_BK_MASK (0x8u)
#define ADI_CFGRA5_MUTE_ST_POS  (4u)
#define ADI_CFGRA5_MUTE_ST_MASK (0x10u)
#define ADI_CFGRA5_SNAP_ST_POS  (5u)
#define ADI_CFGRA5_SNAP_ST_MASK (0x20u)

/* Configuration Register B */
/* CFGRB0 */
#define ADI_CFGRB0_VUV_0_7_POS  (0u)
#define ADI_CFGRB0_VUV_0_7_MASK (0xFFu)
/* CFGRB1 */
#define ADI_CFGRB1_VUV_8_11_POS  (0u)
#define ADI_CFGRB1_VUV_8_11_MASK (0xFu)
#define ADI_CFGRB1_VOV_0_3_POS   (4u)
#define ADI_CFGRB1_VOV_0_3_MASK  (0xF0u)
/* CFGRB2 */
#define ADI_CFGRB2_VOV_4_11_POS  (0u)
#define ADI_CFGRB2_VOV_4_11_MASK (0xFFu)
/* CFGRB3 */
#define ADI_CFGRB3_DCT0_0_5_POS  (0u)
#define ADI_CFGRB3_DCT0_0_5_MASK (0x3Fu)
#define ADI_CFGRB3_DTRNG_POS     (6u)
#define ADI_CFGRB3_DTRNG_MASK    (0x40u)
#define ADI_CFGRB3_DTMEN_POS     (7u)
#define ADI_CFGRB3_DTMEN_MASK    (0x80u)
/* CFGRB4 */
#define ADI_CFGRB4_DCC_1_8_POS  (0u)
#define ADI_CFGRB4_DCC_1_8_MASK (0xFFu)
/* CFGRB5 */
#define ADI_CFGRB5_DCC_9_16_POS  (0u)
#define ADI_CFGRB5_DCC_9_16_MASK (0xFFu)

/* Status Register A */
/* STAR0 */
#define ADI_STAR0_VREF2_0_7_POS  (0u)
#define ADI_STAR0_VREF2_0_7_MASK (0xFFu)
/* STAR1 */
#define ADI_STAR1_VREF2_8_15_POS  (0u)
#define ADI_STAR1_VREF2_8_15_MASK (0xFFu)
/* STAR2 */
#define ADI_STAR2_ITMP_0_7_POS  (0u)
#define ADI_STAR2_ITMP_0_7_MASK (0xFFu)
/* STAR3 */
#define ADI_STAR3_ITMP_8_15_POS  (0u)
#define ADI_STAR3_ITMP_8_15_MASK (0xFFu)

/* Status Register B */
/* STBR0 */
#define ADI_STBR0_VD_0_7_POS  (0u)
#define ADI_STBR0_VD_0_7_MASK (0xFFu)
/* STBR1 */
#define ADI_STBR1_VD_8_15_POS  (0u)
#define ADI_STBR1_VD_8_15_MASK (0xFFu)
/* STBR2 */
#define ADI_STBR2_VA_0_7_POS  (0u)
#define ADI_STBR2_VA_0_7_MASK (0xFFu)
/* STBR3 */
#define ADI_STBR3_VA_8_15_POS  (0u)
#define ADI_STBR3_VA_8_15_MASK (0xFFu)
/* STBR4 */
#define ADI_STBR4_VRES_0_7_POS  (0u)
#define ADI_STBR4_VRES_0_7_MASK (0xFFu)
/* STBR5 */
#define ADI_STBR5_VRES_8_15_POS  (0u)
#define ADI_STBR5_VRES_8_15_MASK (0xFFu)

/* Status Register C */
/* STCR0 */
#define ADI_STCR0_CS1FLT_POS  (0u)
#define ADI_STCR0_CS1FLT_MASK (0x01u)
#define ADI_STCR0_CS2FLT_POS  (1u)
#define ADI_STCR0_CS2FLT_MASK (0x02u)
#define ADI_STCR0_CS3FLT_POS  (2u)
#define ADI_STCR0_CS3FLT_MASK (0x04u)
#define ADI_STCR0_CS4FLT_POS  (3u)
#define ADI_STCR0_CS4FLT_MASK (0x08u)
#define ADI_STCR0_CS5FLT_POS  (4u)
#define ADI_STCR0_CS5FLT_MASK (0x10u)
#define ADI_STCR0_CS6FLT_POS  (5u)
#define ADI_STCR0_CS6FLT_MASK (0x20u)
#define ADI_STCR0_CS7FLT_POS  (6u)
#define ADI_STCR0_CS7FLT_MASK (0x40u)
#define ADI_STCR0_CS8FLT_POS  (7u)
#define ADI_STCR0_CS8FLT_MASK (0x80u)
/* STCR1 */
#define ADI_STCR1_CS9FLT_POS   (0u)
#define ADI_STCR1_CS9FLT_MASK  (0x01u)
#define ADI_STCR1_CS10FLT_POS  (1u)
#define ADI_STCR1_CS10FLT_MASK (0x02u)
#define ADI_STCR1_CS11FLT_POS  (2u)
#define ADI_STCR1_CS11FLT_MASK (0x04u)
#define ADI_STCR1_CS12FLT_POS  (3u)
#define ADI_STCR1_CS12FLT_MASK (0x08u)
#define ADI_STCR1_CS13FLT_POS  (4u)
#define ADI_STCR1_CS13FLT_MASK (0x10u)
#define ADI_STCR1_CS14FLT_POS  (5u)
#define ADI_STCR1_CS14FLT_MASK (0x20u)
#define ADI_STCR1_CS15FLT_POS  (6u)
#define ADI_STCR1_CS15FLT_MASK (0x40u)
#define ADI_STCR1_CS16FLT_POS  (7u)
#define ADI_STCR1_CS16FLT_MASK (0x80u)
/* STCR2 */
#define ADI_STCR2_CT_6_10_POS  (0u)
#define ADI_STCR2_CT_6_10_MASK (0x1Fu)
/* STCR3 */
#define ADI_STCR3_CTS_0_1_POS  (0u)
#define ADI_STCR3_CTS_0_1_MASK (0x03u)
#define ADI_STCR3_CT_0_5_POS   (2u)
#define ADI_STCR3_CT_0_5_MASK  (0xFCu)
/* STCR4 */
#define ADI_STCR4_SMED_POS   (0u)
#define ADI_STCR4_SMED_MASK  (0x01u)
#define ADI_STCR4_SED_POS    (1u)
#define ADI_STCR4_SED_MASK   (0x02u)
#define ADI_STCR4_CMED_POS   (2u)
#define ADI_STCR4_CMED_MASK  (0x04u)
#define ADI_STCR4_CED_POS    (3u)
#define ADI_STCR4_CED_MASK   (0x08u)
#define ADI_STCR4_VD_UV_POS  (4u)
#define ADI_STCR4_VD_UV_MASK (0x10u)
#define ADI_STCR4_VD_OV_POS  (5u)
#define ADI_STCR4_VD_OV_MASK (0x20u)
#define ADI_STCR4_VA_UV_POS  (6u)
#define ADI_STCR4_VA_UV_MASK (0x40u)
#define ADI_STCR4_VA_OV_POS  (7u)
#define ADI_STCR4_VA_OV_MASK (0x80u)
/* STCR5 */
#define ADI_STCR5_OSCCHK_POS   (0u)
#define ADI_STCR5_OSCCHK_MASK  (0x01u)
#define ADI_STCR5_TMODCHK_POS  (1u)
#define ADI_STCR5_TMODCHK_MASK (0x02u)
#define ADI_STCR5_THSD_POS     (2u)
#define ADI_STCR5_THSD_MASK    (0x04u)
#define ADI_STCR5_SLEEP_POS    (3u)
#define ADI_STCR5_SLEEP_MASK   (0x08u)
#define ADI_STCR5_SPIFLT_POS   (4u)
#define ADI_STCR5_SPIFLT_MASK  (0x10u)
#define ADI_STCR5_COMP_POS     (5u)
#define ADI_STCR5_COMP_MASK    (0x20u)
#define ADI_STCR5_VDE_POS      (6u)
#define ADI_STCR5_VDE_MASK     (0x40u)
#define ADI_STCR5_VDEL_POS     (7u)
#define ADI_STCR5_VDEL_MASK    (0x80u)

/* Status Register D */
/* STDR0 */
#define ADI_STDR0_C1UV_POS  (0u)
#define ADI_STDR0_C1UV_MASK (0x01u)
#define ADI_STDR0_C1OV_POS  (1u)
#define ADI_STDR0_C1OV_MASK (0x02u)
#define ADI_STDR0_C2UV_POS  (2u)
#define ADI_STDR0_C2UV_MASK (0x04u)
#define ADI_STDR0_C2OV_POS  (3u)
#define ADI_STDR0_C2OV_MASK (0x08u)
#define ADI_STDR0_C3UV_POS  (4u)
#define ADI_STDR0_C3UV_MASK (0x10u)
#define ADI_STDR0_C3OV_POS  (5u)
#define ADI_STDR0_C3OV_MASK (0x20u)
#define ADI_STDR0_C4UV_POS  (6u)
#define ADI_STDR0_C4UV_MASK (0x40u)
#define ADI_STDR0_C4OV_POS  (7u)
#define ADI_STDR0_C4OV_MASK (0x80u)
/* STDR1 */
#define ADI_STDR1_C5UV_POS   (0u)
#define ADI_STDR1_C5UV_MASK  (0x01u)
#define ADI_STDR1_C50OV_POS  (1u)
#define ADI_STDR1_C50OV_MASK (0x02u)
#define ADI_STDR1_C6UV_POS   (2u)
#define ADI_STDR1_C6UV_MASK  (0x04u)
#define ADI_STDR1_C6OV_POS   (3u)
#define ADI_STDR1_C6OV_MASK  (0x08u)
#define ADI_STDR1_C7UV_POS   (4u)
#define ADI_STDR1_C7UV_MASK  (0x10u)
#define ADI_STDR1_C7OV_POS   (5u)
#define ADI_STDR1_C7OV_MASK  (0x20u)
#define ADI_STDR1_C8UV_POS   (6u)
#define ADI_STDR1_C8UV_MASK  (0x40u)
#define ADI_STDR1_C8OV_POS   (7u)
#define ADI_STDR1_C8OV_MASK  (0x80u)
/* STDR2 */
#define ADI_STDR2_C9UV_POS   (0u)
#define ADI_STDR2_C9UV_MASK  (0x01u)
#define ADI_STDR2_C9OV_POS   (1u)
#define ADI_STDR2_C9OV_MASK  (0x02u)
#define ADI_STDR2_C10UV_POS  (2u)
#define ADI_STDR2_C10UV_MASK (0x04u)
#define ADI_STDR2_C10OV_POS  (3u)
#define ADI_STDR2_C10OV_MASK (0x08u)
#define ADI_STDR2_C11UV_POS  (4u)
#define ADI_STDR2_C11UV_MASK (0x10u)
#define ADI_STDR2_C11OV_POS  (5u)
#define ADI_STDR2_C11OV_MASK (0x20u)
#define ADI_STDR2_C12UV_POS  (6u)
#define ADI_STDR2_C12UV_MASK (0x40u)
#define ADI_STDR2_C12OV_POS  (7u)
#define ADI_STDR2_C12OV_MASK (0x80u)
/* STDR3 */
#define ADI_STDR3_C13UV_POS  (0u)
#define ADI_STDR3_C13UV_MASK (0x01u)
#define ADI_STDR3_C13OV_POS  (1u)
#define ADI_STDR3_C13OV_MASK (0x02u)
#define ADI_STDR3_C14UV_POS  (2u)
#define ADI_STDR3_C14UV_MASK (0x04u)
#define ADI_STDR3_C14OV_POS  (3u)
#define ADI_STDR3_C14OV_MASK (0x08u)
#define ADI_STDR3_C15UV_POS  (4u)
#define ADI_STDR3_C15UV_MASK (0x10u)
#define ADI_STDR3_C15OV_POS  (5u)
#define ADI_STDR3_C15OV_MASK (0x20u)
#define ADI_STDR3_C16UV_POS  (6u)
#define ADI_STDR3_C16UV_MASK (0x40u)
#define ADI_STDR3_C16OV_POS  (7u)
#define ADI_STDR3_C16OV_MASK (0x80u)
/* STDR5 */
#define ADI_STDR5_OC_CNTR_0_7_POS  (0u)
#define ADI_STDR5_OC_CNTR_0_7_MASK (0xFFu)

/* Status Register E */
/* STER4 */
#define ADI_STER4_GPI_1_8_POS  (0u)
#define ADI_STER4_GPI_1_8_MASK (0xFFu)
/* STER5 */
#define ADI_STER5_GPI_9_10_POS  (0u)
#define ADI_STER5_GPI_9_10_MASK (0x03u)
#define ADI_STER5_REV_0_3_POS   (4u)
#define ADI_STER5_REV_0_3_MASK  (0xF0u)

/*========== Extern Constant and Variable Declarations ======================*/

/** Used to choose which configuration register must be addressed */
typedef enum {
    ADI_CFG_REGISTER_SET_A,
    ADI_CFG_REGISTER_SET_B,
    ADI_CFG_REGISTER_SET_E_MAX,
} ADI_CFG_REGISTER_SET_e;

/* If needed, add definition of database entries here */

/** This struct contains pointer to used data buffers */
typedef struct {
    uint16_t *txBuffer;
    uint16_t *rxBuffer;
    DATA_BLOCK_CELL_VOLTAGE_s *cellVoltage;
    DATA_BLOCK_CELL_VOLTAGE_s *cellVoltageAverage;
    DATA_BLOCK_CELL_VOLTAGE_s *cellVoltageFiltered;
    DATA_BLOCK_CELL_VOLTAGE_s *cellVoltageRedundant;
    DATA_BLOCK_CELL_TEMPERATURE_s *cellTemperature;
    DATA_BLOCK_BALANCING_CONTROL_s *balancingControl;
    DATA_BLOCK_ALL_GPIO_VOLTAGES_s *allGpioVoltages;
    DATA_BLOCK_ALL_GPIO_VOLTAGES_s *allGpioVoltagesRedundant;
    DATA_BLOCK_CELL_VOLTAGE_s *cellVoltageOpenWireEven;
    DATA_BLOCK_CELL_VOLTAGE_s *cellVoltageOpenWireOdd;
    DATA_BLOCK_CELL_VOLTAGE_s *cellVoltageAverageOpenWire;
    DATA_BLOCK_CELL_VOLTAGE_s *cellVoltageRedundantOpenWire;
    DATA_BLOCK_ALL_GPIO_VOLTAGES_s *allGpioVoltageOpenWire;
    DATA_BLOCK_OPEN_WIRE_s *openWire;
    uint8_t commandCounter[BS_NR_OF_STRINGS][ADI_N_ADI];
    ADI_ERROR_TABLE_s *errorTable;
} ADI_DATA_s;

/**
 * This structure contains all the variables relevant for the ades183x state machine.
 * The user can get the current state of the ades183x state machine with this variable
 */
typedef struct {
    bool measurementStarted;     /*!< flag indicates that the driver has received the request to start measurements */
    bool firstMeasurementMade;   /*!< flag indicates if first measurement cycle was completed for all strings */
    bool firstDiagnosticMade;    /*!< flag indicates if all diagnostics made at least one time for all strings */
    uint8_t spiNumberInterfaces; /*!< number of SPI channels that have to be measured */
    uint8_t currentString;       /*!< string currently being addressed */
    uint8_t redundantAuxiliaryChannel
        [BS_NR_OF_STRINGS]; /*!< auxiliary channel for which a redundant measurement must be made */
    uint64_t serialId[BS_NR_OF_STRINGS][ADI_N_ADI]; /*!< serial ID of the IC */
    uint8_t revision[BS_NR_OF_STRINGS][ADI_N_ADI];  /*!< revision of the IC */
    ADI_DATA_s data;                                /*!< contains pointers to the local data buffer */
} ADI_STATE_s;

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__ADI_ADES183X_DEFS_H_ */
