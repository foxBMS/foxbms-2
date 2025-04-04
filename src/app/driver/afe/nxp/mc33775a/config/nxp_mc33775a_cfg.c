/**
 *
 * @copyright &copy; 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    nxp_mc33775a_cfg.c
 * @author  foxBMS Team
 * @date    2020-05-08 (date of creation)
 * @updated 2025-03-31 (date of last update)
 * @version v1.9.0
 * @ingroup DRIVERS_CONFIGURATION
 * @prefix  N775
 *
 * @brief   Configuration for the MC33775A analog front-end
 * @details TODO
 */

/*========== Includes =======================================================*/
#include "nxp_mc33775a_cfg.h"

#include "tsi.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/**
 * Default multiplexer measurement sequence
 * Must be adapted to the application
 */
N775_MUX_CH_CFG_s n775_muxSequence[N775_MUX_SEQUENCE_LENGTH] = {
    /*  multiplexer 0 measurement */
    {
        .muxId      = 0,
        .muxChannel = 0,
    },
    {
        .muxId      = 0,
        .muxChannel = 1,
    },
    {
        .muxId      = 0,
        .muxChannel = 2,
    },
    {
        .muxId      = 0,
        .muxChannel = 3,
    },
    {
        .muxId      = 0,
        .muxChannel = 4,
    },
    {
        .muxId      = 0,
        .muxChannel = 5,
    },
    {
        .muxId      = 0,
        .muxChannel = 6,
    },
    {
        .muxId      = 0,
        .muxChannel = 7,
    },
    /*     ,
      multiplexer 2 and 3 measurement
    {
        .muxId    = 0,
        .muxChannel    = 0xFF,    disable enabled mux
    },
    {
        .muxId    = 1,
        .muxChannel    = 0,
    },
    {
        .muxId    = 1,
        .muxChannel    = 1,
    },
    {
        .muxId    = 1,
        .muxChannel    = 2,
    },
    {
        .muxId    = 1,
        .muxChannel    = 3,
    },
    {
        .muxId    = 1,
        .muxChannel    = 4,
    },
    {
        .muxId    = 1,
        .muxChannel    = 5,
    },
    {
        .muxId    = 1,
        .muxChannel    = 6,
    },
    {
        .muxId    = 1,
        .muxChannel    = 7,
    },
    {
        .muxId    = 1,
        .muxChannel    = 0xFF,         disable enabled mux
    },

    {
        .muxId    = 2,
        .muxChannel    = 0,
    },
    {
        .muxId    = 2,
        .muxChannel    = 1,
    },
    {
        .muxId    = 2,
        .muxChannel    = 2,
    },
    {
        .muxId    = 2,
        .muxChannel    = 3,
    },
    {
        .muxId    = 2,
        .muxChannel    = 4,
    },
    {
        .muxId    = 2,
        .muxChannel    = 5,
    },
    {
        .muxId    = 2,
        .muxChannel    = 6,
    },
    {
        .muxId    = 2,
        .muxChannel    = 7,
    }*/
};

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/
int16_t N775_ConvertVoltagesToTemperatures(uint16_t adcVoltage_mV) {
    return TSI_GetTemperature(adcVoltage_mV); /* Convert degree Celsius to deci degree Celsius */
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
#endif
