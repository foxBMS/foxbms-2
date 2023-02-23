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
 * @file    mxm_basic_defines.h
 * @author  foxBMS Team
 * @date    2020-02-11 (date of creation)
 * @updated 2023-02-23 (date of last update)
 * @version v1.5.1
 * @ingroup DRIVERS
 * @prefix  MXM
 *
 * @brief   Basic defines for the complete Maxim driver
 *
 * @details def
 *
 */

#ifndef FOXBMS__MXM_BASIC_DEFINES_H_
#define FOXBMS__MXM_BASIC_DEFINES_H_

/*========== Includes =======================================================*/

#include "battery_system_cfg.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/**
 * @brief Maximum number of modules
 *
 * Maximum number of modules that can be connected to one UART daisy-chain.
 * This value is defined in the Maxim Battery Management Protocol and cannot be
 * changed.
 */
#define MXM_MAXIMUM_NR_OF_MODULES (32u)

/**
 * @brief Maximum number of cells per module
 *
 * Maximum number of cells that can be connected to one module. This value
 * is given by the hardware and cannot be changed.
 */
#define MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE (14u)

/**
 * @brief Maximum number of AUX ports per module
 *
 * Maximum number of AUX ports that can be on one module. This value is given
 * by the hardware and cannot be changed.
 */
#define MXM_MAXIMUM_NR_OF_AUX_PER_MODULE (6u)

/**
 * @brief Type of monitoring device.
 *
 * This enum describes the available types of monitoring devices.
 * These values also align with the register values on the corresponding
 * devices and should therefore not be altered.
 */
typedef enum {
    MXM_MODEL_ID_NONE     = 0u,      /*!< init value for registry */
    MXM_MODEL_ID_MAX17852 = 0x852u,  /*!< monitoring IC MAX17852 */
    MXM_MODEL_ID_MAX17853 = 0x853u,  /*!< monitoring IC MAX17853 */
    MXM_MODEL_ID_MAX17854 = 0x854u,  /*!< monitoring IC MAX17854 */
    MXM_MODEL_ID_invalid  = 0x1000u, /*!< invalid version ID (cannot be encoded in register or unknown value) */
} MXM_MODEL_ID_e;

/** values that the datacheck byte can assume */
typedef enum {
    MXM_DC_PEC_ERROR  = (1u << 7u), /*!< PEC Error */
    MXM_DC_ALRTFMEA   = (1u << 6u), /*!< FMEA alert */
    MXM_DC_ALRTSTATUS = (1u << 5u), /*!< alert from status register */
    MXM_DC_AUXOV      = (1u << 4u), /*!< OV or UT on AUX */
    MXM_DC_AUXUV      = (1u << 3u), /*!< UV or OT on AUX */
    MXM_DC_CELLOV     = (1u << 2u), /*!< OV on CELLs */
    MXM_DC_CELLUV     = (1u << 1u), /*!< UV on CELLs */
    MXM_DC_RESERVED   = 1u,         /*!< reserved */
    MXM_DC_EMPTY      = 0u,         /*!< empty data-check-byte */
} MXM_DC_BYTE_e;

/** valid values for the silicon version of connected monitoring ICs */
typedef enum {
    MXM_SILICON_VERSION_0       = 0x0u,  /*!< initialization value */
    MXM_SILICON_VERSION_1       = 0x1u,  /*!< silicon version 1 (known from data-sheet) */
    MXM_SILICON_VERSION_2       = 0x2u,  /*!< silicon version 2 */
    MXM_SILICON_VERSION_3       = 0x3u,  /*!< silicon version 3 */
    MXM_SILICON_VERSION_4       = 0x4u,  /*!< silicon version 4 (located on old EV kit 2018) */
    MXM_SILICON_VERSION_5       = 0x5u,  /*!< silicon version 5 (located on new EV kit 04.2019) */
    MXM_SILICON_VERSION_6       = 0x6u,  /*!< silicon version 6 */
    MXM_SILICON_VERSION_7       = 0x7u,  /*!< silicon version 7 (most recent as of 2021-11-21) */
    MXM_SILICON_VERSION_8       = 0x8u,  /*!< unknown silicon version */
    MXM_SILICON_VERSION_9       = 0x9u,  /*!< unknown silicon version */
    MXM_SILICON_VERSION_10      = 0xAu,  /*!< unknown silicon version */
    MXM_SILICON_VERSION_11      = 0xBu,  /*!< unknown silicon version */
    MXM_SILICON_VERSION_12      = 0xCu,  /*!< unknown silicon version */
    MXM_SILICON_VERSION_13      = 0xDu,  /*!< unknown silicon version */
    MXM_SILICON_VERSION_14      = 0xEu,  /*!< unknown silicon version */
    MXM_SILICON_VERSION_15      = 0xFu,  /*!< unknown silicon version */
    MXM_SILICON_VERSION_INVALID = 0x10u, /*!< due to the register size, no version number can take this value */
} MXM_SILICON_VERSION_e;

/** Conversion Type */
typedef enum {
    MXM_CONVERSION_UNIPOLAR,      /*!< unipolar measurement */
    MXM_CONVERSION_BIPOLAR,       /*!< bipolar measurement */
    MXM_CONVERSION_BLOCK_VOLTAGE, /*!< measurement of block voltage */
} MXM_CONVERSION_TYPE_e;

/** Type of measurement */
typedef enum {
    MXM_MEASURE_CELL_VOLTAGE,  /*!< cell voltage measurement */
    MXM_MEASURE_TEMP,          /*!< temperature measurement */
    MXM_MEASURE_BLOCK_VOLTAGE, /*!< block voltage measurement */
} MXM_MEASURE_TYPE_e;

/**
 * struct that summarizes all types of measurement values of the monitoring IC
 */
typedef struct {
    /** array of all cell voltages measured by the driver */
    uint16_t cellVoltages_mV[MXM_MAXIMUM_NR_OF_MODULES * MXM_MAXIMUM_NR_OF_CELLS_PER_MODULE];
    /** array of all voltages measured by the driver on the AUX inputs */
    uint16_t auxVoltages_mV[MXM_MAXIMUM_NR_OF_MODULES * MXM_MAXIMUM_NR_OF_AUX_PER_MODULE];
    /** array of all measured block voltages */
    uint16_t blockVoltages[MXM_MAXIMUM_NR_OF_MODULES];
} MXM_DATA_STORAGE_s;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__MXM_BASIC_DEFINES_H_ */
