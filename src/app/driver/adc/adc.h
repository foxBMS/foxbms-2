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
 * @file    adc.h
 * @author  foxBMS Team
 * @date    2019-01-07 (date of creation)
 * @updated 2021-03-24 (date of last update)
 * @ingroup DRIVERS
 * @prefix  ADC
 *
 * @brief   Headers for the driver for the ADC module.
 *
 */

#ifndef FOXBMS__ADC_H_
#define FOXBMS__ADC_H_

/*========== Includes =======================================================*/
#include "general.h"

#include "battery_system_cfg.h"

#include "io.h"
#include "spi.h"

/*========== Macros and Definitions =========================================*/

/** HET1 GIO register that the ADC is connected to. */
#define ADC_HET1_GIO (hetREG1)
/** Pin of HET1 that the ADC Reset is connected to. */
#define ADC_HET1_RESET_PIN (28U)

/** Voltage reference used by ADC0 */
#define ADC_VREF_1 (2.5f)
/** Voltage reference used by ADC1 */
#define ADC_VREF_2 (4.096f)
/** ADC digital gain, set in registers 0x11 to 0x14 */
#define ADC_GAIN (1.0f)

/**
 * Size of SPI messages used to send commands to the ADC
 * Example of commands: NULL, read or write register, lock, unlock...
 */
#define SINGLE_MESSAGE_LENGTH (2U)
/** Size of SPI messages used to retrieve conversion results of the ADC */
#define CONVERT_LENGTH (10U)
/** Number of channels measured by the ADC. ADS131A04 has four channels. */
#define ADC_NUMBER_OF_CHANNELS (4U)

/**
 * State for the ADC conversion
 */
typedef enum {
    ADC_INIT,
    ADC_ENDINIT,
    ADC_READY,
    ADC_UNLOCK,
    ADC_UNLOCKED,
    ADC_WRITE_ADC_ENA,
    ADC_READ_ADC_ENA,
    ADC_CHECK_ADC_ENA,
    ADC_WAKEUP,
    ADC_LOCK,
    ADC_CONVERT_1,
    ADC_CONVERT_2,
} ADC_STATE_e;

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/

/**
 * @brief   initializes the ADC devices. It is called during startup.
 */
extern void ADC_Initialize(void);

/**
 * @brief   determines which ADC is measured and stores result in database.
 * @details It alternates between measurement on ADC1 and ADC2. Measurement
 *          read by SPI.
 */
extern void ADC_Control(void);

/*========== Externalized Static Functions Prototypes (Unit Test) ===========*/

#endif /* FOXBMS__ADC_H_ */
