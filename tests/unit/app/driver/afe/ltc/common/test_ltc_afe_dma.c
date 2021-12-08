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
 * @file    test_ltc_afe_dma.c
 * @author  foxBMS Team
 * @date    2020-06-10 (date of creation)
 * @updated 2021-12-01 (date of last update)
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of the ltc_afe_dma.c module in ltc
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_sys_dma.h"
#include "Mockfassert.h"
#include "Mockio.h"
#include "Mockltc.h"
#include "Mockspi.h"

#include "ltc_cfg.h"
#include "spi_cfg.h"

#include "ltc_afe_dma.h"

/*========== Definitions and Implementations for Unit Test ==================*/
uint8_t ltc_RXPECbuffer[LTC_N_BYTES_FOR_DATA_TRANSMISSION] = {0};
uint8_t ltc_TXPECbuffer[LTC_N_BYTES_FOR_DATA_TRANSMISSION] = {0};

#define DMA_REQ_LINE_SPI1_TX (DMA_REQ1)
#define DMA_REQ_LINE_SPI1_RX (DMA_REQ0)
#define DMA_REQ_LINE_SPI2_TX (DMA_REQ3)
#define DMA_REQ_LINE_SPI2_RX (DMA_REQ2)
#define DMA_REQ_LINE_SPI3_TX (DMA_REQ15)
#define DMA_REQ_LINE_SPI3_RX (DMA_REQ14)

#define BIG_ENDIAN    (3U)
#define ELEMENT_COUNT (1U)
#define DMAREQEN_BIT  (0x10000U)
#define SPIEN_BIT     (0x1000000U)

#define DMA_REQ_LINE_TX (DMA_REQ_LINE_SPI1_TX)
#define DMA_REQ_LINE_RX (DMA_REQ_LINE_SPI1_RX)

LTC_STATE_s ltc_stateBase = {
    .timer                   = 0,
    .statereq                = LTC_STATE_NO_REQUEST,
    .state                   = LTC_STATEMACH_UNINITIALIZED,
    .substate                = 0,
    .laststate               = LTC_STATEMACH_UNINITIALIZED,
    .lastsubstate            = 0,
    .adcModereq              = LTC_ADCMODE_FAST_DCP0,
    .adcMode                 = LTC_ADCMODE_FAST_DCP0,
    .adcMeasChreq            = LTC_ADCMEAS_UNDEFINED,
    .adcMeasCh               = LTC_ADCMEAS_UNDEFINED,
    .numberOfMeasuredMux     = 32,
    .triggerentry            = 0,
    .ErrRetryCounter         = 0,
    .ErrRequestCounter       = 0,
    .VoltageSampleTime       = 0,
    .muxSampleTime           = 0,
    .commandDataTransferTime = 3,
    .commandTransferTime     = 3,
    .gpioClocksTransferTime  = 3,
    .muxmeas_seqptr          = NULL_PTR,
    .muxmeas_seqendptr       = NULL_PTR,
    .muxmeas_nr_end          = 0,
    .first_measurement_made  = false,
    .ltc_muxcycle_finished   = STD_NOT_OK,
    .check_spi_flag          = STD_NOT_OK,
    .balance_control_done    = STD_NOT_OK,
    .transmit_ongoing        = false,
    .dummyByte_ongoing       = STD_NOT_OK,
};

/* - configuring dma control packets   */
g_dmaCTRL afe_ltcDmaControlPacketTx = {
    .SADD      = 0u,                                /* source address             */
    .DADD      = 0u,                                /* destination  address       */
    .CHCTRL    = 0U,                                /* channel control            */
    .FRCNT     = LTC_N_BYTES_FOR_DATA_TRANSMISSION, /* frame count                */
    .ELCNT     = ELEMENT_COUNT,                     /* element count              */
    .ELDOFFSET = 0U,                                /* element destination offset */
    .ELSOFFSET = 0U,                                /* element destination offset */
    .FRDOFFSET = 0U,                                /* frame destination offset   */
    .FRSOFFSET = 0U,                                /* frame destination offset   */
    .PORTASGN  = PORTA_READ_PORTB_WRITE,            /* port assignment            */
    .RDSIZE    = ACCESS_8_BIT,                      /* read size                  */
    .WRSIZE    = ACCESS_8_BIT,                      /* write size                 */
    .TTYPE     = FRAME_TRANSFER,                    /* transfer type              */
    .ADDMODERD = ADDR_INC1,                         /* address mode read          */
    .ADDMODEWR = ADDR_FIXED,                        /* address mode write         */
    .AUTOINIT  = AUTOINIT_OFF,                      /* autoinit                   */
};

g_dmaCTRL afe_ltcDmaControlPacketRx = {
    .SADD      = 0u,                                /* source address             */
    .DADD      = 0u,                                /* destination  address       */
    .CHCTRL    = 0U,                                /* channel control            */
    .FRCNT     = LTC_N_BYTES_FOR_DATA_TRANSMISSION, /* frame count                */
    .ELCNT     = ELEMENT_COUNT,                     /* element count              */
    .ELDOFFSET = 0U,                                /* element destination offset */
    .ELSOFFSET = 0U,                                /* element destination offset */
    .FRDOFFSET = 0U,                                /* frame destination offset   */
    .FRSOFFSET = 0U,                                /* frame destination offset   */
    .PORTASGN  = PORTB_READ_PORTA_WRITE,            /* port assignment            */
    .RDSIZE    = ACCESS_8_BIT,                      /* read size                  */
    .WRSIZE    = ACCESS_8_BIT,                      /* write size                 */
    .TTYPE     = FRAME_TRANSFER,                    /* transfer type              */
    .ADDMODERD = ADDR_FIXED,                        /* address mode read          */
    .ADDMODEWR = ADDR_INC1,                         /* address mode write         */
    .AUTOINIT  = AUTOINIT_OFF,                      /* autoinit                   */
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testDummy(void) {
}
