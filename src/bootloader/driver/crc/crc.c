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
 * @file    crc.c
 * @author  foxBMS Team
 * @date    2022-02-22 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup DRIVERS
 * @prefix  CRC
 *
 * @brief   Implementation of the software- and hardware-based Cyclic
 *          Redundancy Check (CRC) calculation functions
 * @details Contains software-based and hardware-based CRC calculation functions.
 */

/*========== Includes =======================================================*/
#include "general.h"

#include "crc.h"

#include "HL_crc.h"
#include "HL_rti.h"
#include "HL_sys_core.h"
#include "HL_sys_dma.h"
#include "HL_system.h"

#include "crc64_calc.h"
#include "fassert.h"
#include "fstd_types.h"
#include "rti.h"

#include <stdint.h>

/*========== Macros and Definitions =========================================*/
/** The number of 64-bits data will be used for CRC calculation every time */
#define CRC_NUM_OF_64_BITS_DATA_EVERY_CRC_CALCULATION (1024u)

/** Mask used to clear bits in config register to set CRC module to Data Capture Mode */
#define CRC_DATA_CAPTURE_MODE_CLEAR_MASK (0xFFFFFFFCu)

/** Mask used to set bits in config register to set CRC module to Semi-CPU Mode */
#define CRC_SEMI_CPU_MODE_SET_MASK (0x2u)

/** Maximum time to wait for on-board CRC calculation */
#define CRC_ON_BOARD_TIME_OUT_us (10000000u)

/** Number of bits in a uint32_t type memory container for variable */
#define CRC_NUM_OF_BITS_IN_UINT32 (32u)

/** Number of bits in crcREG1->PSA_SIGREGH1 */
#define CRC_NUM_OF_BITS_PSA_SIGREGH1 (32u)

/** Ratio between the HCLK and the pre-scaler clock that clock the timeout counter of CRC  */
#define CRC_RATIO_HCLK_CRC_TIMEOUT_PRE_SCALER_CLOCK (64u)

/** Time RTI COMPARE 3 */
#define CRC_TIME_RTI_COMPARE_3_us ((uint32_t)(200u / HCLK_FREQ))

/** HCLK_FREQ */
#define CRC_HCLK_FREQ_MHz ((uint32_t)HCLK_FREQ)

/** Bit to enable DMA request 3 (which will be used to set SETDMA3 in RTISETINTENA)) */
#define CRC_BIT_NUMBER_ENABLE_DMA_REQUEST_3 (11u)

/** Number of DMA request line */
#define CRC_DMA_REQUEST_LINE (19u)

/*========== Static Constant and Variable Definitions =======================*/
/** variable where the calculated CRC 64-bit signature will be saved */
static uint64_t crc_crcSignature64Bits = 0u;

/** variable where the number of the CRC sector calculation will be saved */
static uint32_t crc_crcSectorCompletionNumber = 0u;

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 * @brief   Swap the higher 32 bits with the lower 32 bits of the provided
 *          value
 * @param   crcValue64Bits 64 bits input value
 */
static uint64_t CRC_SwapCrc64Bits(uint64_t crcValue64Bits);

/*========== Static Function Implementations ================================*/
static uint64_t CRC_SwapCrc64Bits(uint64_t crcValue64Bits) {
    /* AXIVION Routine Generic-MissingParameterAssert: crcValue64Bits: parameter accepts whole range */
    uint32_t tmp = 0u;
    /** Union to swap the higher 32 bits with the lower 32 bits of the 64-bit
     *  CRC signature */
    union {
        uint64_t crc_64;
        uint32_t crc_32[2];
    } CRC_UN;

    CRC_UN.crc_64 = crcValue64Bits;

    tmp              = CRC_UN.crc_32[0];
    CRC_UN.crc_32[0] = CRC_UN.crc_32[1];
    CRC_UN.crc_32[1] = tmp;

    crcValue64Bits = CRC_UN.crc_64;
    return crcValue64Bits;
}

/*========== Extern Function Implementations ================================*/
/* GCOVR_EXCL_START */
uint64_t CRC_SemiAutoCrcCalculation(uint32_t programAddress, uint32_t dataSizeIn64Bits, uint64_t root) {
    /* AXIVION Routine Generic-MissingParameterAssert: root: parameter accepts whole range */
    FAS_ASSERT(programAddress != 0u);
    FAS_ASSERT(dataSizeIn64Bits != 0u);

    bool gotoNext      = true;
    uint64_t retValCrc = 0u;

    /* Reset static variables */
    crc_crcSignature64Bits        = 0u;
    crc_crcSectorCompletionNumber = 0u;

    /* Initialize RTI Module */
    rtiInit();

    /* Start counter of rtiREG1 */
    rtiStartCounter(rtiREG1, rtiCOUNTER_BLOCK1);

    /* Enable DMA request 3 (bit 11) */
    rtiEnableNotification(rtiREG1, 1u << CRC_BIT_NUMBER_ENABLE_DMA_REQUEST_3);

    /* Initialize CRC module */
    crcInit();

    /* Enable CPU IRQ line */
    _enable_IRQ_interrupt_();

    /* Enable Compression complete interrupt for Channel 0 */
    crcEnableNotification(crcREG1, CRC_CH1_CC);

    /* Configure DMA for contents to be verified */
    g_dmaCTRL g_dmaCTRLPKT;
    /* initial source address */
    g_dmaCTRLPKT.SADD = programAddress;
    /* initial destination address */
    g_dmaCTRLPKT.DADD = (uint32_t)(&(crcREG1->PSA_SIGREGL1));
    /* channel control */
    g_dmaCTRLPKT.CHCTRL = 1u;
    /* read size */
    g_dmaCTRLPKT.RDSIZE = ACCESS_64_BIT;
    /* write size */
    g_dmaCTRLPKT.WRSIZE = ACCESS_64_BIT;
    /* frame count */
    g_dmaCTRLPKT.FRCNT = dataSizeIn64Bits / CRC_NUM_OF_64_BITS_DATA_EVERY_CRC_CALCULATION;
    /* element count */
    g_dmaCTRLPKT.ELCNT = CRC_NUM_OF_64_BITS_DATA_EVERY_CRC_CALCULATION;
    /* element source offset: 0u << g_dmaCTRLPKT.RDSIZE */
    g_dmaCTRLPKT.ELSOFFSET = 0u;
    /* frame source offset: 0u << (g_dmaCTRLPKT.RDSIZE * g_dmaCTRLPKT.ELCNT)*/
    g_dmaCTRLPKT.FRSOFFSET = 0u;
    /* element destination offset: 0u << g_dmaCTRLPKT.WRSIZE */
    g_dmaCTRLPKT.ELDOFFSET = 0u;
    /* frame destination offset: 0u << (g_dmaCTRLPKT.WRSIZE * g_dmaCTRLPKT.ELCNT)*/
    g_dmaCTRLPKT.FRDOFFSET = 0u;
    /* Port A Read (SRAM) - Port B Write (CRC) */
    g_dmaCTRLPKT.PORTASGN = 1u;
    /* transfer type */
    g_dmaCTRLPKT.TTYPE = FRAME_TRANSFER;
    /* address mode read */
    g_dmaCTRLPKT.ADDMODERD = ADDR_INC1;
    /* address mode write */
    g_dmaCTRLPKT.ADDMODEWR = ADDR_FIXED;
    /* autoinit off */
    g_dmaCTRLPKT.AUTOINIT = AUTOINIT_OFF;

    /* CRC Configuration. */
    /* The finish compression interrupt will be triggered once the scount has
     * been zero. */
    crcConfig_t sCrcParams;
    sCrcParams.crc_channel = CRC_CH1;
    sCrcParams.mode        = CRC_SEMI_CPU;
    sCrcParams.pcount      = g_dmaCTRLPKT.ELCNT;
    sCrcParams.scount      = g_dmaCTRLPKT.FRCNT;
    sCrcParams.wdg_preload = 0u;
    /* The timeout counter is clocked by a pre-scaler clock which is permanently
    running at division 64 of HCLK clock. The following equation is from
    data sheet SPNA235 August 2016 p. 10 formula 3.6.1 (3) */
    sCrcParams.block_preload = (g_dmaCTRLPKT.FRCNT * CRC_TIME_RTI_COMPARE_3_us * CRC_HCLK_FREQ_MHz) /
                               CRC_RATIO_HCLK_CRC_TIMEOUT_PRE_SCALER_CLOCK;

    /* Reset the CRC channel configuration */
    crcChannelReset(crcREG1, CRC_CH1);
    crcSetConfig(crcREG1, &sCrcParams);

    /* Set to data capture mode to plant seed for next sector */
    crcREG1->CTRL2 &= CRC_DATA_CAPTURE_MODE_CLEAR_MASK;
    /* AXIVION Disable Style MisraC2012Directive-4.1: this cast is supposed to
     * only extract one part (32 high bits) of 64-bit CRC signature stored in
     * the root variable. */
    crcREG1->PSA_SIGREGH1 = (uint32_t)(root);
    /* AXIVION Disable Style MisraC2012Directive-4.1: this cast is supposed to
     * only extract one part (32 low bits) of 64-bit CRC signature stored in
     * the root variable. */
    crcREG1->PSA_SIGREGL1 = (uint32_t)(root >> CRC_NUM_OF_BITS_PSA_SIGREGH1);
    /* AXIVION Enable Style MisraC2012Directive-4.1 */
    crcREG1->CTRL2 |= CRC_SEMI_CPU_MODE_SET_MASK;

    /* Enable DMA */
    dmaEnable();

    /* Enable the DMA channel 1 interrupts */
    dmaEnableInterrupt(DMA_CH1, FTC, DMA_INTA);
    dmaEnableInterrupt(DMA_CH1, LFS, DMA_INTA);
    dmaEnableInterrupt(DMA_CH1, BTC, DMA_INTA);

    /* Assign the DMA request line 19 what will be triggered by RTI timer for
     * DMA chanel 1 */
    dmaReqAssign(DMA_CH1, (dmaRequest_t)CRC_DMA_REQUEST_LINE);

    /* Assign DMA Control Packet to DMA Channel */
    dmaSetCtrlPacket(DMA_CH1, g_dmaCTRLPKT);

    /* Set the DMA Channel to trigger on hardware request (because RTI -> DMA
     * request belong to the 48 hardware DMA requests) */
    dmaSetChEnable(DMA_CH1, DMA_HW);

    /* Reset the rti counter */
    if (RTI_ResetFreeRunningCount() == 0u) {
        gotoNext = false;
    }

    if (gotoNext) {
        rtiStartCounter(rtiREG1, rtiCOUNTER_BLOCK0);
        uint32_t startCounter = RTI_GetFreeRunningCount();

        /* Wait until the calculation has been finished */
        while (crc_crcSectorCompletionNumber != sCrcParams.scount) {
            if (RTI_IsTimeElapsed(startCounter, CRC_ON_BOARD_TIME_OUT_us) == true) {
                break;
            }
        };

        /* Reset the configurations (disable DMA, RTI, CRC)*/
        dmaDisable();
        rtiStopCounter(rtiREG1, rtiCOUNTER_BLOCK1);
        rtiDisableNotification(rtiREG1, 1u << CRC_BIT_NUMBER_ENABLE_DMA_REQUEST_3);
        crcDisableNotification(crcREG1, CRC_CH1_CC | CRC_CH1_FAIL | CRC_CH1_OR | CRC_CH1_UR | CRC_CH1_TO);

        /* Check the number of completed sector */
        if (crc_crcSectorCompletionNumber == sCrcParams.scount) {
            retValCrc = crc_crcSignature64Bits;
        }
    }

    return retValCrc;
    /* GCOVR_EXCL_STOP */
}

/* GCOVR_EXCL_START */
#ifndef UNITY_UNIT_TEST
/* in the unit test case we mock 'HL_crc', so we have an implementation */
void crcNotification(crcBASE_t *crc, uint32 flags) {
    FAS_ASSERT(crc != NULL_PTR);
    /* AXIVION Routine Generic-MissingParameterAssert: flags: parameter accepts whole range */
    if (CRC_CH1_CC == flags) {
        /* Reading the compressed CRC from the CRC module */
        crc_crcSignature64Bits = crcGetSectorSig(crcREG1, CRC_CH1);

        /* Set to data capture mode to plant seed for next sector */
        crcREG1->CTRL2 &= CRC_DATA_CAPTURE_MODE_CLEAR_MASK;
        uint32_t t1 = (uint32_t)(crc_crcSignature64Bits >> CRC_NUM_OF_BITS_IN_UINT32);
        /* AXIVION Next Codeline Style MisraC2012Directive-4.1: it is intended
        to only take the lower 32 bits of crc signature */
        uint32_t t2           = (uint32_t)crc_crcSignature64Bits;
        crcREG1->PSA_SIGREGH1 = t2;
        crcREG1->PSA_SIGREGL1 = t1;
        crcREG1->CTRL2 |= CRC_SEMI_CPU_MODE_SET_MASK;

        /* increase crc_crcSectorCompletionNumber by 1 every time a section has
         * been successfully calculated */
        crc_crcSectorCompletionNumber++;
    }
}
#endif
/* GCOVR_EXCL_STOP */

extern uint64_t CRC_CalculateCrc64(const uint64_t *pkData, uint32_t dataSizeIn64Bits, uint64_t root) {
    /* AXIVION Routine Generic-MissingParameterAssert: root: parameter accepts whole range */
    FAS_ASSERT(dataSizeIn64Bits != 0u);
    FAS_ASSERT(pkData != NULL_PTR);

    /* Calculate in every turn a new 8-bytes of CRC signature for 8-bytes of
     * data */
    uint64_t data_64_bits   = 0u;
    uint64_t crcValue64Bits = root;

    const uint64_t *pkDataForLoop = pkData;

    for (uint32_t idx64Bits = 0u; idx64Bits < dataSizeIn64Bits; idx64Bits++) {
        data_64_bits = *pkDataForLoop;

        data_64_bits   = CRC_SwapCrc64Bits(data_64_bits);
        crcValue64Bits = CRC_SwapCrc64Bits(crcValue64Bits);

        crcValue64Bits = calc_crc64_user(crcValue64Bits, data_64_bits);

        crcValue64Bits = CRC_SwapCrc64Bits(crcValue64Bits);

        /* Update the pointer for next 8 bytes of data */
        pkDataForLoop++;
    }

    return crcValue64Bits;
}

/*================== Static functions ======================================*/

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
extern uint64_t TEST_CRC_SwapCrc64Bits(uint64_t crcValue64Bits) {
    return CRC_SwapCrc64Bits(crcValue64Bits);
}
#endif
