/**
 *
 * @copyright &copy; 2010 - 2022, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
 * @file    can_cfg_tx_boot-message.c
 * @author  foxBMS Team
 * @date    2019-12-04 (date of creation)
 * @updated 2022-10-27 (date of last update)
 * @version v1.4.1
 * @ingroup DRIVERS
 * @prefix  CANTX
 *
 * @brief   Driver for the CAN module
 *
 * @details Implementation of the CAN Interrupts, initialization, buffers,
 *          receive and transmit interfaces.
 */

/*========== Includes =======================================================*/
#include "can_cfg_tx_boot-message.h"

#include "version_cfg.h"

#include "HL_het.h"
#include "HL_reg_system.h"

#include "can.h"
#include "can_cfg_tx-message-definitions.h"
#include "can_helper.h"
#include "database.h"
#include "foxmath.h"
#include "ftask.h"

/*========== Macros and Definitions =========================================*/
/** maximum distance from release that can be encoded in the boot message */
#define CANTX_BOOT_MESSAGE_MAXIMUM_RELEASE_DISTANCE (31u)
#if CANTX_BOOT_MESSAGE_MAXIMUM_RELEASE_DISTANCE > UINT8_MAX
#error "This code assumes that the define is smaller or equal to UINT8_MAX")
#endif

/** bit position of boot message byte 3 version control flag */
#define CANTX_BOOT_MESSAGE_BYTE_3_BIT_VERSION_CONTROL (0u)

/** bit position of boot message byte 3 dirty flag */
#define CANTX_BOOT_MESSAGE_BYTE_3_BIT_DIRTY_FLAG (1u)

/** bit position of boot message byte 3 release distance overflow flag */
#define CANTX_BOOT_MESSAGE_BYTE_3_BIT_DISTANCE_OVERFLOW_FLAG (2u)

/** bit position of boot message byte 3 release distance counter */
#define CANTX_BOOT_MESSAGE_BYTE_3_BIT_DISTANCE_COUNTER (3u)

/*========== Static Constant and Variable Definitions =======================*/

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/

/*========== Static Function Implementations ================================*/

/*========== Extern Function Implementations ================================*/

extern STD_RETURN_TYPE_e CANTX_TransmitBootMessage(void) {
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    /* Set major number */
    data[CAN_BYTE_0_POSITION] = ver_foxbmsVersionInformation.major;
    /* Set minor number */
    data[CAN_BYTE_1_POSITION] = ver_foxbmsVersionInformation.minor;
    /* Set patch number */
    data[CAN_BYTE_2_POSITION] = ver_foxbmsVersionInformation.patch;

    /* intermediate variable for message byte 3 */
    uint8_t versionControlByte = 0u;

    /* Set version control flags */
    if (ver_foxbmsVersionInformation.underVersionControl == true) {
        versionControlByte |= (0x01u << CANTX_BOOT_MESSAGE_BYTE_3_BIT_VERSION_CONTROL);
    }
    if (ver_foxbmsVersionInformation.isDirty == true) {
        versionControlByte |= (0x01u << CANTX_BOOT_MESSAGE_BYTE_3_BIT_DIRTY_FLAG);
    }
    /* Set overflow flag (if release distance is larger than 31) */
    if (ver_foxbmsVersionInformation.distanceFromLastRelease > CANTX_BOOT_MESSAGE_MAXIMUM_RELEASE_DISTANCE) {
        /* we need to set the overflow flag */
        versionControlByte |= (0x01u << CANTX_BOOT_MESSAGE_BYTE_3_BIT_DISTANCE_OVERFLOW_FLAG);
    }

    /* Set release distance (capped to maximum value) */
    const uint8_t distanceCapped = (uint8_t)MATH_MinimumOfTwoUint16_t(
        ver_foxbmsVersionInformation.distanceFromLastRelease, (uint16_t)CANTX_BOOT_MESSAGE_MAXIMUM_RELEASE_DISTANCE);
    versionControlByte |= (distanceCapped << CANTX_BOOT_MESSAGE_BYTE_3_BIT_DISTANCE_COUNTER);

    /* assign assembled byte to data byte */
    data[CAN_BYTE_3_POSITION] = versionControlByte;

    /* Read out device register with unique ID */
    const uint32_t deviceRegister = systemREG1->DEVID;

    /* Set unique ID */
    data[CAN_BYTE_4_POSITION] = (uint8_t)((deviceRegister >> 24u) & 0xFFu);
    data[CAN_BYTE_5_POSITION] = (uint8_t)((deviceRegister >> 16u) & 0xFFu);
    data[CAN_BYTE_6_POSITION] = (uint8_t)((deviceRegister >> 8u) & 0xFFu);
    data[CAN_BYTE_7_POSITION] = (uint8_t)(deviceRegister & 0xFFu);

    STD_RETURN_TYPE_e retval = CAN_DataSend(CAN_NODE_1, CANTX_BOOT_ID, &data[0]);

    return retval;
}

extern STD_RETURN_TYPE_e CANTX_TransmitDieId(void) {
    uint8_t data[] = {GEN_REPEAT_U(0u, GEN_STRIP(CAN_MAX_DLC))};

    /* Read out device register with die ID low and high */
    const uint32_t dieIdLow  = systemREG1->DIEIDL;
    const uint32_t dieIdHigh = systemREG1->DIEIDH;

    /* set die ID */
    /* AXIVION Disable Style Generic-NoMagicNumbers: The magic numbers are used to divide down the registers into the CAN message */
    data[CAN_BYTE_0_POSITION] = (uint8_t)((dieIdHigh >> 24u) & 0xFFu);
    data[CAN_BYTE_1_POSITION] = (uint8_t)((dieIdHigh >> 16u) & 0xFFu);
    data[CAN_BYTE_2_POSITION] = (uint8_t)((dieIdHigh >> 8u) & 0xFFu);
    data[CAN_BYTE_3_POSITION] = (uint8_t)(dieIdHigh & 0xFFu);
    data[CAN_BYTE_4_POSITION] = (uint8_t)((dieIdLow >> 24u) & 0xFFu);
    data[CAN_BYTE_5_POSITION] = (uint8_t)((dieIdLow >> 16u) & 0xFFu);
    data[CAN_BYTE_6_POSITION] = (uint8_t)((dieIdLow >> 8u) & 0xFFu);
    data[CAN_BYTE_7_POSITION] = (uint8_t)(dieIdLow & 0xFFu);
    /* AXIVION Enable Style Generic-NoMagicNumbers: */

    STD_RETURN_TYPE_e retval = CAN_DataSend(CAN_NODE_1, CANTX_DIE_ID_ID, &data[0]);

    return retval;
}

/*========== Getter for static Variables (Unit Test) ========================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
