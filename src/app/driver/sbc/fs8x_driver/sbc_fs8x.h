/*
 * Copyright (c) 2016 - 2018, NXP Semiconductors, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @file sbc_fs8x.h
 * @brief FS8x driver interface.
 *
 * @author nxf44615
 * @version 1.1
 * @date 9-Oct-2018
 * @copyright Copyright (c) 2016 - 2018, NXP Semiconductors, Inc.
 *
 * @updated 2020-03-18 (date of last update)
 * Adapted driver to pass interface configuration as parameter into the
 * functions to enable the usage of multiple ICs in the system.
 */

#ifndef SBC_FS8X_H_
#define SBC_FS8X_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include <stdbool.h>
#include "sbc_fs8x_map.h"
#include "sbc_fs8x_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/** @brief Watchdog seed default value. */
#define FS8x_WD_SEED_DEFAULT 0x5AB2U

/** @addtogroup EnumsDefs
 * @{ */
/** @brief Voltage outputs. Can be used with function @ref FS8x_SetRegulatorState(). */
typedef enum
{
    fs8xBuck1 = 4,  /**< BUCK1 */
    fs8xBuck2 = 3,  /**< BUCK2 */
    fs8xBuck3 = 2,  /**< BUCK3 */
    fs8xLdo1 = 1,   /**< LDO1 */
    fs8xLdo2 = 0,   /**< LDO2 */
    fs8xVBoost = 5, /**< VBOOST */
    fs8xVPre = 6    /**< VPRE */
} fs8x_reg_output_t;

/** AMUX channel selection. Can be used with function @ref FS8x_SwitchAMUXchannel(). */
typedef enum
{
    fs8xAmuxGnd = FS8X_M_AMUX_GND,                                  /**< GND */
    fs8xAmuxVddio = FS8X_M_AMUX_VDDIO,                              /**< VDDIO */
    fs8xAmuxTempSensor = FS8X_M_AMUX_TEMPERATURE_SENSOR,            /**< Temperature sensor */
    fs8xAmuxBandgapMain = FS8X_M_AMUX_BAND_GAP_MAIN,                /**< Main Bandgap */
    fs8xAmuxBandgapFailSafe = FS8X_M_AMUX_BAND_GAP_FAIL_SAFE,       /**< Fail-safe bandgap */
    fs8xAmuxVbuck1Volt = FS8X_M_AMUX_VBUCK1_VOLTAGE,                /**< BUCK1 */
    fs8xAmuxVbuck2Volt = FS8X_M_AMUX_VBUCK2_VOLTAGE,                /**< BUCK2 */
    fs8xAmuxVbuck3VoltDiv2_5 = FS8X_M_AMUX_VBUCK3_VOLTAGE_DIVIDED,  /**< BUCK3 divided by 2.5 */
    fs8xAmuxVpreVoltDiv2_5 = FS8X_M_AMUX_VPRE_VOLTAGE_DIVIDED,      /**< VPRE divided by 2.5 */
    fs8xAmuxVboostVoltDiv2_5 = FS8X_M_AMUX_VBOOST_VOLTAGE_DIVIDED,  /**< VBOOST divided by 2.5 */
    fs8xAmuxVldo1VoltDiv2_5 = FS8X_M_AMUX_VLDO1_VOLTAGE_DIVIDED,    /**< VLDO1 divided by 2.5 */
    fs8xAmuxVldo2VoltDiv2_5 = FS8X_M_AMUX_VLDO2_VOLTAGE_DIVIDED,    /**< VLDO2 divided by 2.5 */
    fs8xAmuxVbosVoltDiv2_5 = FS8X_M_AMUX_VBOS_VOLTAGE_DIVIDED,      /**< VBOS divided by 2.5 */
    fs8xAmuxVsup1VoltDiv7_5 = FS8X_M_AMUX_VSUP1_VOLTAGE_DIVIDED,    /**< VSUP1 divided by 7.5 */
    fs8xAmuxWake1VoltDiv7_5 = FS8X_M_AMUX_WAKE1_VOLTAGE_DIVIDED,    /**< WAKE1 divided by 7.5 */
    fs8xAmuxWake2VoltDiv7_5 = FS8X_M_AMUX_WAKE2_VOLTAGE_DIVIDED,    /**< WAKE2 divided by 7.5 */
    fs8xAmuxVana = FS8X_M_AMUX_VANA,                                /**< Vana */
    fs8xAmuxVdig = FS8X_M_AMUX_VDIG,                                /**< Vdig */
    fs8xAmuxVdigFs = FS8X_M_AMUX_VDIG_FS,                           /**< Vdig_fs */
    fs8xAmuxPsync = FS8X_M_AMUX_PSYNC_VOLTAGE,                      /**< PSYNC */
    fs8xAmuxVsup1VoltDiv14 = FS8X_M_RATIO_RATIO_14 | FS8X_M_AMUX_VSUP1_VOLTAGE_DIVIDED, /**< VSUP1 divided by 14 */
    fs8xAmuxWake1VoltDiv14 = FS8X_M_RATIO_RATIO_14 | FS8X_M_AMUX_WAKE1_VOLTAGE_DIVIDED, /**< WAKE1 divided by 14 */
    fs8xAmuxWake2VoltDiv14 = FS8X_M_RATIO_RATIO_14 | FS8X_M_AMUX_WAKE2_VOLTAGE_DIVIDED  /**< WAKE2 divided by 14 */
} fs8x_amux_selection_t;
/** @} */

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/
/** @defgroup API Driver API
 * @{ */
/** @brief Changes seed of LFSR used for watchdog.
 *
 * The watchdog seed can be changed just during the INIT_FS phase (for challenger WD)
 * or during the OPEN watchdog window (for simple WD). Timing is up to the application!
 * @param [in,out] spiInterface  SPI communication interface config
 * @param [in] drvData           Driver run-time data.
 * @param [in] wdSeed            Watchdog LFSR seed.
 * @return @ref fs8x_status_t "Status return code." */
fs8x_status_t FS8x_WD_ChangeSeed(SPI_INTERFACE_CONFIG_s* spiInterface, fs8x_drv_data_t* drvData, uint16_t wdSeed);

/** @brief Performs the watchdog refresh.
 * @param [in,out] spiInterface  SPI communication interface config
 * @param [in] drvData           Driver run-time data.
 * @return @ref fs8x_status_t "Status return code." */
fs8x_status_t FS8x_WD_Refresh(SPI_INTERFACE_CONFIG_s* spiInterface, fs8x_drv_data_t* drvData);

/** @brief FS0B release routine.
 *
 * This function also checks all preconditions before
 * it sends the register write command (LBIST_OK = ABIST1_OK = ABIST2_OK = 1,
 * Fault Error Counter = 0). If any precondition is not met, the function returns
 * @ref fs8xStatusError.
 * @param [in,out] spiInterface  SPI communication interface config
 * @param [in] drvData           Driver run-time data.
 * @return @ref fs8x_status_t "Status return code." */
fs8x_status_t FS8x_FS0B_Release(SPI_INTERFACE_CONFIG_s* spiInterface, fs8x_drv_data_t* drvData);

/** @brief Switches a desired channel to the AMUX pin.
 * @param [in,out] spiInterface  SPI communication interface config
 * @param [in] drvData           Driver run-time data.
 * @param [in] channelSelection  Selected channel to be delivered to AMUX pin.
 * @return @ref fs8x_status_t "Status return code." */
fs8x_status_t FS8x_SwitchAMUXchannel(SPI_INTERFACE_CONFIG_s* spiInterface, fs8x_drv_data_t* drvData,
        fs8x_amux_selection_t channelSelection);

/** @brief Sets state (enable/disable) of the selected voltage regulator.
 * @param [in,out] spiInterface  SPI communication interface config
 * @param [in] drvData           Driver run-time data.
 * @param [in] vreg              Voltage regulator enum (LDO1-2, BUCK1-3, BOOST, VPRE).
 * @param [in] enable            State (enable = true / disable = false).
 * @return @ref fs8x_status_t "Status return code." */
fs8x_status_t FS8x_SetRegulatorState(SPI_INTERFACE_CONFIG_s* spiInterface, fs8x_drv_data_t* drvData,
        fs8x_reg_output_t vreg, bool enable);

/** @brief Reads actual Fault Error Counter value.
 * @param [in,out] spiInterface         SPI communication interface config
 * @param [in]  drvData                 Driver run-time data.
 * @param [out] faultErrorCounterValue  Fault Error counter value storage.
 * @return @ref fs8x_status_t "Status return code." */
fs8x_status_t FS8x_GetFaultErrorCounterValue(SPI_INTERFACE_CONFIG_s* spiInterface, fs8x_drv_data_t* drvData,
        uint8_t* faultErrorCounterValue);
/** @} */
#endif /* SBC_FS8X_H_ */
