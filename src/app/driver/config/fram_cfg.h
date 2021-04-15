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
 * @file    fram_cfg.h
 * @author  foxBMS Team
 * @date    2020-03-05 (date of creation)
 * @updated 2020-03-05 (date of last update)
 * @ingroup DRIVERS
 * @prefix  FRAM
 *
 * @brief   Headers for the configuration for the FRAM module
 *
 *
 */

#ifndef FOXBMS__FRAM_CFG_H_
#define FOXBMS__FRAM_CFG_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "battery_system_cfg.h"

/*========== Macros and Definitions =========================================*/
/**
 * @brief   IDs for projects that use a memory layout
 * @details This enum can be use to differentiate between projects. While an older
 *          version of the same project should be upgradeable, the entry of another
 *          project should be just discarded if a conflict occurs.
 *
 *          In order for the ID to stay the same it is important that the defined
 *          macros are not changed and that each and every macro has a different
 *          value.
 */
typedef uint16_t FRAM_PROJECT_ID;

/** this is the standard main development branch */
#define FRAM_PROJECT_ID_FOXBMS_BASELINE ((FRAM_PROJECT_ID)0u)

/** configuration struct of database channel (data block) */
typedef struct {
    void *blockptr;
    uint16_t datalength;
    uint32_t address;
} FRAM_BASE_HEADER_s;

/** fram block identification numbers */
typedef enum FRAM_BLOCK_ID {
    FRAM_BLOCK_ID_VERSION,
    FRAM_BLOCK_ID_SOC,
    FRAM_BLOCK_ID_SBC_INIT_STATE,
    FRAM_BLOCK_ID_DEEP_DISCHARGE_FLAG,
    FRAM_BLOCK_ID_SOE,
    FRAM_BLOCK_MAX, /**< DO NOT CHANGE, MUST BE THE LAST ENTRY */
} FRAM_BLOCK_ID_e;

/**
 * @brief   Stores the version of the memory layout of the FRAM
 * @details This struct stores with which memory-layout version the FRAM has
 *          been written. This allows the BMS to recognize an incompatible
 *          memory layout.
 */
typedef struct FRAM_VERSION {
    FRAM_PROJECT_ID project; /*!< an identifier for the project, it is not
                                    intended to migrate between different projects */
    uint16_t version;        /*!< version counter, i.e. for updating to a new version in the same project */
} FRAM_VERSION_s;

/** struct for the FRAM entry of the SBC driver */
typedef struct FRAM_SBC_INIT {
    uint8_t phase;
    STD_RETURN_TYPE_e finState;
} FRAM_SBC_INIT_s;

/**
 * state of charge (SOC). Since SOC is voltage dependent, three different
 * values are used, min, max and average. SOC defined as a float number between
 * 0.0f and 100.0f (0% and 100%)
 */
typedef struct FRAM_SOC {
    float minimumSoc_perc[BS_NR_OF_STRINGS]; /*!< minimum SOC */
    float maximumSoc_perc[BS_NR_OF_STRINGS]; /*!< maximum SOC */
    float averageSoc_perc[BS_NR_OF_STRINGS]; /*!< average SOC */
} FRAM_SOC_s;

/**
 * state of energy (SOE). Since SOE is voltage dependent, three different
 * values are used, min, max and average. SOE defined as a float number between
 * 0.0f and 100.0f (0% and 100%)
 */
typedef struct FRAM_SOE {
    float minimumSoe_perc[BS_NR_OF_STRINGS]; /*!< minimum SOE */
    float maximumSoe_perc[BS_NR_OF_STRINGS]; /*!< maximum SOE */
    float averageSoe_perc[BS_NR_OF_STRINGS]; /*!< average SOE */
} FRAM_SOE_s;
/**
 * flag to indicate if a deep-discharge in a string has been detected
 */
typedef struct FRAM_DEEP_DISCHARGE_FLAG {
    bool deepDischargeFlag[BS_NR_OF_STRINGS]; /*!< false (0): no error, true (1): deep-discharge detected */
} FRAM_DEEP_DISCHARGE_FLAG_s;

/*========== Extern Constant and Variable Declarations ======================*/

extern FRAM_BASE_HEADER_s fram_base_header[FRAM_BLOCK_MAX];

/**
 * Variables to be stored in FRAM
 */
/**@{*/
extern FRAM_VERSION_s fram_version;
extern FRAM_SOC_s fram_soc;
extern FRAM_SOE_s fram_soe;
extern FRAM_SBC_INIT_s fram_sbcInit;
extern FRAM_DEEP_DISCHARGE_FLAG_s fram_deepDischargeFlags;
/**@}*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__FRAM_CFG_H_ */
