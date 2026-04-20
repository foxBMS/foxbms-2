/**
 *
 * @copyright &copy; 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    app_build_cfg.h
 * @author  foxBMS Team
 * @date    2024-10-02 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup MAIN
 * @prefix  VER
 *
 * @brief   Header file for the application build information that is generated
 *          by the toolchain
 * @details Defines the structure and type of information that is stored in the
 *          application build configuration variable.
 *          The application build information is extracted from
 *          'conf/bms/bms.json'.
 *          These enums must be aligned with the sym/dbc files.
 */

#ifndef FOXBMS__APP_BUILD_CFG_H_
#define FOXBMS__APP_BUILD_CFG_H_

/*========== Includes =======================================================*/

/*========== Macros and Definitions =========================================*/

/** Enums for the build information */
/** @{
 * AFE types enum
 */
typedef enum {
    /* Higher two hexadecimal digits are the different manufacturers
    * and the lower two digits are the specific AFE type */

    /* DEBUG: 0x0000 - 0x0099 */
    AFE_DEBUG_DEFAULT = 0x0000,
    AFE_DEBUG_CAN     = 0x0001,
    /* LTC: 0x0100 - 0x0199 */
    AFE_LTC_LTC6804_1 = 0x0100,
    AFE_LTC_LTC6806   = 0x0101,
    AFE_LTC_LTC6811_1 = 0x0102,
    AFE_LTC_LTC6812_1 = 0x0103,
    AFE_LTC_LTC6813_1 = 0x0104,
    /* NXP: 0x0200 - 0x0299 */
    AFE_NXP_MC33775A = 0x0200,
    /* MAXIM: 0x0300 - 0x0399 */
    AFE_MAXIM_MAX17852 = 0x0300,
    /* ADI: 0x0400 - 0x0499 */
    AFE_ADI_ADES1830 = 0x0400,
    /* TI: 0x0500 - 0x0599 */
    AFE_TI_DUMMY = 0x0501,
    /* Current Limit -> 0xFFFF (4 digits) */
} VER_AFE_e;
/** @} */

/** @{
 * soc algorithm enum
 */
typedef enum {
    SOC_ALGORITHM_NONE,
    SOC_ALGORITHM_DEBUG,
    SOC_ALGORITHM_COUNTING,
    SOC_ALGORITHM_LOOKUP_TABLE,
    SOC_ALGORITHM_INVALID,
} VER_SOC_ALGORITHM_e;
/** @} */

/** @{
 * soe algorithm enum
 */
typedef enum {
    SOE_ALGORITHM_NONE,
    SOE_ALGORITHM_DEBUG,
    SOE_ALGORITHM_COUNTING,
    SOE_ALGORITHM_INVALID,
} VER_SOE_ALGORITHM_e;
/** @} */

/** @{
 * sof algorithm enum
 */
typedef enum {
    SOF_ALGORITHM_TRAPEZOID,
    SOF_ALGORITHM_INVALID,
} VER_SOF_ALGORITHM_e;
/** @} */

/** @{
 * soh algorithm enum
 */
typedef enum {
    SOH_ALGORITHM_NONE,
    SOH_ALGORITHM_DEBUG,
    SOH_ALGORITHM_INVALID,
} VER_SOH_ALGORITHM_e;
/** @} */

/** @{
 *  balancing strategy enum
 */
typedef enum {
    BALANCING_STRATEGY_NONE,
    BALANCING_STRATEGY_VOLTAGE,
    BALANCING_STRATEGY_HISTORY,
} VER_BALANCING_STRATEGY_e;
/** @} */

/** @{
 *  IMD enum
 */
typedef enum {
    IMD_NONE,
    IMD_BENDER_ISO165C,
    IMD_BENDER_IR155,
} VER_IMD_e;
/** @} */

/** @{
 *  rtos enum
 */
typedef enum {
    RTOS_FREERTOS,
    RTOS_SAFERTOS,
} VER_RTOS_e;
/** @} */

/** @{
 *  temperature sensor method enum
 */
typedef enum {
    TEMPERATURE_SENSOR_METHOD_POLYNOMIAL,
    TEMPERATURE_SENSOR_METHOD_LOOKUP_TABLE,
} VER_TEMPERATURE_SENSOR_METHOD_e;
/** @} */

/** @{
 *  temperature sensors enum
 */
typedef enum {
    TEMPERATURE_SENSOR_FAK00,
    TEMPERATURE_SENSOR_EPC00,
    TEMPERATURE_SENSOR_EPC01,
    TEMPERATURE_SENSOR_EPC02,
    TEMPERATURE_SENSOR_MUR00,
    TEMPERATURE_SENSOR_SEM00,
    TEMPERATURE_SENSOR_VIS00,
    TEMPERATURE_SENSOR_VIS01,
    TEMPERATURE_SENSOR_VIS02,
    TEMPERATURE_SENSOR_TDK00,
    TEMPERATURE_SENSOR_TDK01,
} VER_TEMPERATURE_SENSOR_e;
/** @} */

/** struct type that describes the build configuration of the system */
typedef struct {
    const VER_SOC_ALGORITHM_e socAlgorithm;                        /*!< state estimator for soc*/
    const VER_SOE_ALGORITHM_e soeAlgorithm;                        /*!< state estimator for soe*/
    const VER_SOF_ALGORITHM_e sofAlgorithm;                        /*!< state estimator for sof*/
    const VER_SOH_ALGORITHM_e sohAlgorithm;                        /*!< state estimator for soh*/
    const VER_IMD_e imd;                                           /*!< name of the imd*/
    const VER_BALANCING_STRATEGY_e balancingStrategy;              /*!< balancing strategy*/
    const VER_RTOS_e rtos;                                         /*!< used rtos*/
    const VER_AFE_e afe;                                           /*!< name of the afe*/
    const VER_TEMPERATURE_SENSOR_e temperatureSensor;              /*!< name of the temperature sensor*/
    const VER_TEMPERATURE_SENSOR_METHOD_e temperatureSensorMethod; /*!< method of the temperature sensor*/
} VER_BUILD_CONFIGURATION_s;

/*========== Extern Constant and Variable Declarations ======================*/

/** build configuration of the system */
extern const VER_BUILD_CONFIGURATION_s ver_foxbmsBuildConfiguration;

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/
#ifdef UNITY_UNIT_TEST
#endif

#endif /* FOXBMS__APP_BUILD_CFG_H_ */
