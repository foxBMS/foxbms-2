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

/** @file sbc_fs8x_common.h
 * @brief Driver common structures, enums, macros and configuration values.
 *
 * This header file contains settings for following driver functionalities:
 * - Watchdog type selection.
 * - Communication type selection (SPI/I2C).
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

#ifndef SBC_FS8X_COMMON_H_
#define SBC_FS8X_COMMON_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "spi.h"
#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** @defgroup WdTypes Watchdog types
 *
 * Note that periodic call of function @ref FS8x_WD_Refresh() is up to the application.
 * @{ */
#define FS8x_WD_DISABLE 0       /**< Watchdog refresh disabled. */
#define FS8x_WD_SIMPLE 1        /**< Simple Watchdog refresh. */
#define FS8x_WD_CHALLENGER 2    /**< Challenger watchdog refresh. */
/** @} */

/** @brief Define Watchdog type (see @ref WdTypes). */
#define FS8x_WD_TYPE FS8x_WD_SIMPLE

/** @defgroup CommTypes Allowed communication types
 *
 * If SPI or I2C communication is selected, the other implementation will not be compiled.
 * Actual communication type can be selected in @ref fs8x_drv_data_t::communicationMode.
 * @{ */
#define FS8x_COMM_SPI 0     /**< SPI communication implementation will be compiled. */
#define FS8x_COMM_I2C 1     /**< I2C communication implementation will be compiled. */
#define FS8x_COMM_BOTH 2    /**< SPI and I2C communication implementation will be compiled. */
/** @} */

/** @brief Define communication type (see @ref CommTypes). */
#define FS8x_COMM_TYPE FS8x_COMM_SPI

/** @defgroup EnumsDefs Enums definition
 * @{ */
/** @brief Status return codes. */
typedef enum
{
    fs8xStatusOk    = 0U,   /**< No error. */
    fs8xStatusError = 1U    /**< Error. */
} fs8x_status_t;

/** @brief Actually selected communication type (SPI or I2C).
 *
 * Note that the value of #FS8x_COMM_TYPE define should match selected value. */
typedef enum
{
    fs8xSPI = 0,  /**< SPI */
    fs8xI2C = 1   /**< I2C */
} fs8x_commType_t;

/** @brief Command type. */
typedef enum {
    fs8xRegRead,    /**< Register Read */
    fs8xRegWrite    /**< Register Write */
} fs8x_command_type_t;
/** @} */

/** @defgroup StructDefs Struct definitions
 * @{ */
/** @brief Structure representing transmit data frame. */
typedef struct
{
    /** @brief Main/Fail Safe register selection. */
    bool isFailSafe;

    /** @brief Register address. */
    uint8_t registerAddress;

    /** @brief Command type (R/W). */
    fs8x_command_type_t commandType;

    /** @brief Data to be written to the register.
     *
     * If commandType is "read", this value will be ignored. */
    uint16_t writeData;

} fs8x_tx_frame_t;

/** @brief Structure representing received data frame. */
typedef struct
{
    /** @brief A device status is returned into this byte after a successful transfer.
     *
     * Valid just for the SPI communication. Status byte is not returned if
     * I2C communication is selected. */
    uint8_t deviceStatus;

    /** @brief Content of a read register. */
    uint16_t readData;

} fs8x_rx_frame_t;

/** @brief This data structure is used by the FS8x driver (this is the first
* parameter of most the FS8x functions). It contains a configuration of the
* FS8x device. */
typedef struct
{
    /** @brief Actual watchdog seed value. See @ref FS8x_WD_ChangeSeed() for details. */
    uint16_t watchdogSeed;

    /** @brief Actual communication mode (SPI or I2C). See @ref fs8x_commType_t for details. */
    fs8x_commType_t communicationMode;

    /** @brief SBC I2C address.
     *
     * This is not the full I2C address but just a value of the OTP register
     * OTP_CFG_I2C - I2CDEVADDR[3:0]. */
    uint8_t i2cAddressOtp;

} fs8x_drv_data_t;
/** @} */

/** @name Bitwise operations used by this driver. */
/** @{ */
/** @brief This macro updates value of bits specified by the mask. It is assumed that
 *  value is already shifted. */
#define FS8x_BO_SETVAL(data, val, mask)   (((data) & ~(mask)) | (val))

/** @brief This macro updates value of bits specified by the mask. Additionally range
 *  check on the value is performed. It is assumed that value is not shifted. */
#define FS8x_BO_SETVAL_EXT(data, value, mask, shift) \
    (((data) & ~(mask << shift)) | (((value) & (mask)) << (shift)))

/** @brief This macro returns value specified by the mask. */
#define FS8x_BO_GETVAL(data, mask, shift) ((data) & (mask) << (shift))

/** @brief Macro for getting value from register.
 *
 * @param value Value read from register.
 * @param mask Bit selection.
 * @param shift Bit shift.
 * @returns Masked and r-shifted value. */
#define FS8x_BO_GET_REG_VALUE(value, mask, shift) (((value) & (mask)) >> (shift))
/** @} */

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/

#endif /* SBC_FS8X_COMMON_H_ */
