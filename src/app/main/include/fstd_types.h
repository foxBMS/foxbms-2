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
 * @file    fstd_types.h
 * @author  foxBMS Team
 * @date    2015-12-20 (date of creation)
 * @updated 2021-11-09 (date of last update)
 * @ingroup GENERAL_CONF
 * @prefix  STD
 *
 * @brief   Definition of foxBMS standard types
 */

#ifndef FOXBMS__FSTD_TYPES_H_
#define FOXBMS__FSTD_TYPES_H_

/*========== Includes =======================================================*/

/*========== Macros and Definitions =========================================*/

/** @def    NULL
 * @brief   NULL definition
 * @details This define describes a null. Use it to compare against null.
 */
#ifndef NULL
#define NULL ((void *)0u)
#endif

/**
 * @def     NULL_PTR
 * @brief   Null pointer
 * @details This define describes a null pointer. Use it to compare for null
 *          pointer access.
 */
#ifndef NULL_PTR
#define NULL_PTR ((void *)(0u))
#endif

/*========== Extern Constant and Variable Declarations ======================*/
/** enum for standard return type */
typedef enum STD_RETURN_TYPE {
    STD_OK,     /**< ok     */
    STD_NOT_OK, /**< not ok */
} STD_RETURN_TYPE_e;

/** enum for pin state */
typedef enum STD_PIN_STATE {
    STD_PIN_LOW,       /*!< electrical pin state: low */
    STD_PIN_HIGH,      /*!< electrical pin state: high */
    STD_PIN_UNDEFINED, /*!< electrical pin state: undefined */
} STD_PIN_STATE_e;

/*========== Extern Function Prototypes =====================================*/

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__FSTD_TYPES_H_ */
