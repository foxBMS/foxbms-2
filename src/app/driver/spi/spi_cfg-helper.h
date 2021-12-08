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
 * @file    spi_cfg-helper.h
 * @author  foxBMS Team
 * @date    2021-12-08 (date of creation)
 * @updated 2021-12-08 (date of last update)
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  SPI
 *
 * @brief   Headers for the configuration for the SPI module
 *
 *
 */

#ifndef FOXBMS__SPI_CFG_HELPER_H_
#define FOXBMS__SPI_CFG_HELPER_H_

/*========== Includes =======================================================*/
#include "stdint.h"

/*========== Macros and Definitions =========================================*/
/** HW chip select bit position, TMS570LC4357 has maximum of six hardware chip
  * select pins per SPI node @{ */
#define SPI_HARDWARE_CHIP_SELECT_0_BIT_POSITION (0u)
#define SPI_HARDWARE_CHIP_SELECT_1_BIT_POSITION (1u)
#define SPI_HARDWARE_CHIP_SELECT_2_BIT_POSITION (2u)
#define SPI_HARDWARE_CHIP_SELECT_3_BIT_POSITION (3u)
#define SPI_HARDWARE_CHIP_SELECT_4_BIT_POSITION (4u)
#define SPI_HARDWARE_CHIP_SELECT_5_BIT_POSITION (5u)
/**@}*/

/** Bit mask to activate hardware chip select in the CSNR field of the SPIDAT1 register */
#define SPI_HARDWARE_CHIP_SELECT_ACTIVE (0u)

/** Bit mask to *not* activate hardware chip select in the CSNR field of the SPIDAT1 register */
#define SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE (1u)

/* clang-format off */
/* AXIVION Disable Style MisraC2012-2.2: Redundant expressions are kept to enhance code readability */
#define SPI_HARDWARE_CHIP_SELECT_DISABLE_ALL                                                       \
    ((uint8_t)(                                                                                \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_0_BIT_POSITION) | /* CS0 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_1_BIT_POSITION) | /* CS1 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_2_BIT_POSITION) | /* CS2 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_3_BIT_POSITION) | /* CS3 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_4_BIT_POSITION) | /* CS4 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_5_BIT_POSITION)   /* CS5 */ \
        ))
/* AXIVION Enable Style MisraC2012-2.2: */
/* clang-format on */

/* clang-format off */
/* AXIVION Disable Style MisraC2012-2.2: Redundant expressions are kept to enhance code readability */
#define SPI_HARDWARE_CHIP_SELECT_0_ACTIVE                                                      \
    ((uint8_t)(                                                                                \
        (SPI_HARDWARE_CHIP_SELECT_ACTIVE     << SPI_HARDWARE_CHIP_SELECT_0_BIT_POSITION) | /* CS0 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_1_BIT_POSITION) | /* CS1 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_2_BIT_POSITION) | /* CS2 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_3_BIT_POSITION) | /* CS3 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_4_BIT_POSITION) | /* CS4 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_5_BIT_POSITION)   /* CS5 */ \
    ))
/* AXIVION Enable Style MisraC2012-2.2: */
/* clang-format on */

/* clang-format off */
/* AXIVION Disable Style MisraC2012-2.2: Redundant expressions are kept to enhance code readability */
#define SPI_HARDWARE_CHIP_SELECT_1_ACTIVE                                                      \
    ((uint8_t)(                                                                                \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_0_BIT_POSITION) | /* CS0 */ \
        (SPI_HARDWARE_CHIP_SELECT_ACTIVE     << SPI_HARDWARE_CHIP_SELECT_1_BIT_POSITION) | /* CS1 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_2_BIT_POSITION) | /* CS2 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_3_BIT_POSITION) | /* CS3 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_4_BIT_POSITION) | /* CS4 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_5_BIT_POSITION)   /* CS5 */ \
    ))
/* AXIVION Enable Style MisraC2012-2.2: */
/* clang-format on */

/* clang-format off */
/* AXIVION Disable Style MisraC2012-2.2: Redundant expressions are kept to enhance code readability */
#define SPI_HARDWARE_CHIP_SELECT_2_ACTIVE                                                      \
    ((uint8_t)(                                                                                \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_0_BIT_POSITION) | /* CS0 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_1_BIT_POSITION) | /* CS1 */ \
        (SPI_HARDWARE_CHIP_SELECT_ACTIVE     << SPI_HARDWARE_CHIP_SELECT_2_BIT_POSITION) | /* CS2 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_3_BIT_POSITION) | /* CS3 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_4_BIT_POSITION) | /* CS4 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_5_BIT_POSITION)   /* CS5 */ \
    ))
/* AXIVION Enable Style MisraC2012-2.2: */
/* clang-format on */

/* clang-format off */
/* AXIVION Disable Style MisraC2012-2.2: Redundant expressions are kept to enhance code readability */
#define SPI_HARDWARE_CHIP_SELECT_3_ACTIVE                                                      \
    ((uint8_t)(                                                                                \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_0_BIT_POSITION) | /* CS0 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_1_BIT_POSITION) | /* CS1 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_2_BIT_POSITION) | /* CS2 */ \
        (SPI_HARDWARE_CHIP_SELECT_ACTIVE     << SPI_HARDWARE_CHIP_SELECT_3_BIT_POSITION) | /* CS3 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_4_BIT_POSITION) | /* CS4 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_5_BIT_POSITION)   /* CS5 */ \
    ))
/* AXIVION Enable Style MisraC2012-2.2: */
/* clang-format on */

/* clang-format off */
/* AXIVION Disable Style MisraC2012-2.2: Redundant expressions are kept to enhance code readability */
#define SPI_HARDWARE_CHIP_SELECT_4_ACTIVE                                                      \
    ((uint8_t)(                                                                                \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_0_BIT_POSITION) | /* CS0 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_1_BIT_POSITION) | /* CS1 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_2_BIT_POSITION) | /* CS2 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_3_BIT_POSITION) | /* CS3 */ \
        (SPI_HARDWARE_CHIP_SELECT_ACTIVE     << SPI_HARDWARE_CHIP_SELECT_4_BIT_POSITION) | /* CS4 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_5_BIT_POSITION)   /* CS5 */ \
    ))
/* AXIVION Enable Style MisraC2012-2.2: */
/* clang-format on */

/* clang-format off */
/* AXIVION Disable Style MisraC2012-2.2: Redundant expressions are kept to enhance code readability */
#define SPI_HARDWARE_CHIP_SELECT_5_ACTIVE                                                      \
    ((uint8_t)(                                                                                \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_0_BIT_POSITION) | /* CS0 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_1_BIT_POSITION) | /* CS1 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_2_BIT_POSITION) | /* CS2 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_3_BIT_POSITION) | /* CS3 */ \
        (SPI_HARDWARE_CHIP_SELECT_NOT_ACTIVE << SPI_HARDWARE_CHIP_SELECT_4_BIT_POSITION) | /* CS4 */ \
        (SPI_HARDWARE_CHIP_SELECT_ACTIVE     << SPI_HARDWARE_CHIP_SELECT_5_BIT_POSITION)   /* CS5 */ \
    ))
/* AXIVION Enable Style MisraC2012-2.2: */
/* clang-format on */

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__SPI_CFG_HELPER_H_ */
