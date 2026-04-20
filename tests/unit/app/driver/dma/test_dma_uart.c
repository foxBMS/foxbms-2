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
 * @file    test_dma_uart.c
 * @author  foxBMS Team
 * @date    2020-11-24 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the dma module
 * @details TODO
 *
 */

/* cspell:ignore CHCTRL ELDOFFSET ELSOFFSET FRSOFFSET */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockHL_i2c.h"
#include "MockHL_spi.h"
#include "MockHL_sys_dma.h"
#include "Mockafe_dma.h"
#include "Mocki2c.h"
#include "Mockio.h"
#include "Mockspi.h"
#include "Mocktask.h"

#include "uart_cfg.h"

#include "dma.h"
#include "struct_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("dma.c")

TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/dma")
TEST_INCLUDE_PATH("../../src/app/driver/i2c")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/driver/uart")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/
TaskHandle_t ftsk_taskHandleI2c;

uint32_t spi_saveFmt0[] = {
    0U,
    0U,
    0U,
    0U,
    0U,
    0U,
};

SPI_BUSY_STATE_e spi_busyFlags[] = {
    SPI_IDLE,
    SPI_IDLE,
    SPI_IDLE,
    SPI_IDLE,
    SPI_IDLE,
};

const uint8_t spi_nrBusyFlags = sizeof(spi_busyFlags) / sizeof(SPI_BUSY_STATE_e);

DMA_CHANNEL_CONFIG_s dma_spiDmaChannels[DMA_NUMBER_SPI_INTERFACES] = {
    {DMA_CH0, DMA_CH1}, /* SPI1 */
    {DMA_CH2, DMA_CH3}, /* SPI2 */
    {DMA_CH4, DMA_CH5}, /* SPI3 */
    {DMA_CH6, DMA_CH7}, /* SPI4 */
    {DMA_CH8, DMA_CH9}, /* SPI5 */
};

DMA_REQUEST_CONFIG_s dma_spiDmaRequests[DMA_NUMBER_SPI_INTERFACES] = {
    {DMA_REQ_LINE_SPI1_TX, DMA_REQ_LINE_SPI1_RX}, /* SPI1 */
    {DMA_REQ_LINE_SPI2_TX, DMA_REQ_LINE_SPI2_RX}, /* SPI2 */
    {DMA_REQ_LINE_SPI3_TX, DMA_REQ_LINE_SPI3_RX}, /* SPI3 */
    {DMA_REQ_LINE_SPI4_TX, DMA_REQ_LINE_SPI4_RX}, /* SPI4 */
    {DMA_REQ_LINE_SPI5_TX, DMA_REQ_LINE_SPI5_RX}, /* SPI5 */
};

spiBASE_t *dma_spiInterfaces[DMA_NUMBER_SPI_INTERFACES] = {
    spiREG1, /* SPI1 */
    spiREG2, /* SPI2 */
    spiREG3, /* SPI3 */
    spiREG4, /* SPI4 */
    spiREG5, /* SPI5 */
};

uint8_t i2c_rxLastByteInterface1 = 0u;
uint8_t i2c_rxLastByteInterface2 = 0u;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

void testDMA_Initialize(void) {

    /* DMA control packets configuration for SPI  */
    g_dmaCTRL dma_controlPacketSpiTx = {
        .SADD      = 0u,                               /* source address             */
        .DADD      = 0u,                               /* destination  address       */
        .CHCTRL    = 0u,                               /* channel chain control      */
        .FRCNT     = 0u,                               /* frame count                */
        .ELCNT     = 1u,                               /* element count              */
        .ELDOFFSET = 0u,                               /* element destination offset */
        .ELSOFFSET = 0u,                               /* element destination offset */
        .FRDOFFSET = 0u,                               /* frame destination offset   */
        .FRSOFFSET = 0u,                               /* frame destination offset   */
        .PORTASGN  = (uint32_t)PORTA_READ_PORTB_WRITE, /* port assignment            */
        .RDSIZE    = (uint32_t)ACCESS_16_BIT,          /* read size                  */
        .WRSIZE    = (uint32_t)ACCESS_16_BIT,          /* write size                 */
        .TTYPE     = (uint32_t)FRAME_TRANSFER,         /* transfer type              */
        .ADDMODERD = (uint32_t)ADDR_INC1,              /* address mode read          */
        .ADDMODEWR = (uint32_t)ADDR_FIXED,             /* address mode write         */
        .AUTOINIT  = (uint32_t)AUTOINIT_OFF            /* autoinit                   */
    };

    g_dmaCTRL dma_controlPacketSpiRx = {
        .SADD      = 0u,                               /* source address             */
        .DADD      = 0u,                               /* destination  address       */
        .CHCTRL    = 0u,                               /* channel chain control      */
        .FRCNT     = 0u,                               /* frame count                */
        .ELCNT     = 1u,                               /* element count              */
        .ELDOFFSET = 0u,                               /* element destination offset */
        .ELSOFFSET = 0u,                               /* element destination offset */
        .FRDOFFSET = 0u,                               /* frame destination offset   */
        .FRSOFFSET = 0u,                               /* frame destination offset   */
        .PORTASGN  = (uint32_t)PORTB_READ_PORTA_WRITE, /* port assignment            */
        .RDSIZE    = (uint32_t)ACCESS_16_BIT,          /* read size                  */
        .WRSIZE    = (uint32_t)ACCESS_16_BIT,          /* write size                 */
        .TTYPE     = (uint32_t)FRAME_TRANSFER,         /* transfer type              */
        .ADDMODERD = (uint32_t)ADDR_FIXED,             /* address mode read          */
        .ADDMODEWR = (uint32_t)ADDR_INC1,              /* address mode write         */
        .AUTOINIT  = (uint32_t)AUTOINIT_OFF            /* autoinit                   */
    };

    /* DMA control packets configuration for I2C1  */
    g_dmaCTRL dma_controlPacketI2cTx = {
        .SADD      = 0u,                               /* source address             */
        .DADD      = 0u,                               /* destination  address       */
        .CHCTRL    = 0u,                               /* channel chain control      */
        .FRCNT     = 0u,                               /* frame count                */
        .ELCNT     = 1u,                               /* element count              */
        .ELDOFFSET = 0u,                               /* element destination offset */
        .ELSOFFSET = 0u,                               /* element destination offset */
        .FRDOFFSET = 0u,                               /* frame destination offset   */
        .FRSOFFSET = 0u,                               /* frame destination offset   */
        .PORTASGN  = (uint32_t)PORTA_READ_PORTB_WRITE, /* port assignment            */
        .RDSIZE    = (uint32_t)ACCESS_8_BIT,           /* read size                  */
        .WRSIZE    = (uint32_t)ACCESS_8_BIT,           /* write size                 */
        .TTYPE     = (uint32_t)FRAME_TRANSFER,         /* transfer type              */
        .ADDMODERD = (uint32_t)ADDR_INC1,              /* address mode read          */
        .ADDMODEWR = (uint32_t)ADDR_FIXED,             /* address mode write         */
        .AUTOINIT  = (uint32_t)AUTOINIT_OFF            /* autoinit                   */
    };

    g_dmaCTRL dma_controlPacketI2cRx = {
        .SADD      = 0u,                               /* source address             */
        .DADD      = 0u,                               /* destination  address       */
        .CHCTRL    = 0u,                               /* channel chain control      */
        .FRCNT     = 0u,                               /* frame count                */
        .ELCNT     = 1u,                               /* element count              */
        .ELDOFFSET = 0u,                               /* element destination offset */
        .ELSOFFSET = 0u,                               /* element destination offset */
        .FRDOFFSET = 0u,                               /* frame destination offset   */
        .FRSOFFSET = 0u,                               /* frame destination offset   */
        .PORTASGN  = (uint32_t)PORTB_READ_PORTA_WRITE, /* port assignment            */
        .RDSIZE    = (uint32_t)ACCESS_8_BIT,           /* read size                  */
        .WRSIZE    = (uint32_t)ACCESS_8_BIT,           /* write size                 */
        .TTYPE     = (uint32_t)FRAME_TRANSFER,         /* transfer type              */
        .ADDMODERD = (uint32_t)ADDR_FIXED,             /* address mode read          */
        .ADDMODEWR = (uint32_t)ADDR_INC1,              /* address mode write         */
        .AUTOINIT  = (uint32_t)AUTOINIT_OFF            /* autoinit                   */
    };

    /** DMA control packets configuration for SCI4 (UART) */
    g_dmaCTRL dma_controlPacketSci4Tx = {
        .SADD      = 0u,                               /* source address             */
        .DADD      = 0u,                               /* destination  address       */
        .CHCTRL    = 0u,                               /* channel chain control      */
        .FRCNT     = 0u,                               /* frame count                */
        .ELCNT     = 1u,                               /* element count              */
        .ELDOFFSET = 0u,                               /* element destination offset */
        .ELSOFFSET = 0u,                               /* element destination offset */
        .FRDOFFSET = 0u,                               /* frame destination offset   */
        .FRSOFFSET = 0u,                               /* frame destination offset   */
        .PORTASGN  = (uint32_t)PORTA_READ_PORTB_WRITE, /* port assignment            */
        .RDSIZE    = (uint32_t)ACCESS_8_BIT,           /* read size                  */
        .WRSIZE    = (uint32_t)ACCESS_8_BIT,           /* write size                 */
        .TTYPE     = (uint32_t)FRAME_TRANSFER,         /* transfer type              */
        .ADDMODERD = (uint32_t)ADDR_INC1,              /* address mode read          */
        .ADDMODEWR = (uint32_t)ADDR_FIXED,             /* address mode write         */
        .AUTOINIT  = (uint32_t)AUTOINIT_OFF            /* autoinit                   */
    };

    dmaEnable_Expect();

    /* Test SPI */

    for (uint8_t i = 0u; i < DMA_NUMBER_SPI_INTERFACES; i++) {
        dmaReqAssign_Expect(
            (dmaChannel_t)dma_spiDmaChannels[i].txChannel, (dmaRequest_t)dma_spiDmaRequests[i].txRequest);
        dmaReqAssign_Expect(
            (dmaChannel_t)dma_spiDmaChannels[i].rxChannel, (dmaRequest_t)dma_spiDmaRequests[i].rxRequest);

        dmaEnableInterrupt_Expect(
            (dmaChannel_t)(dmaChannel_t)dma_spiDmaChannels[i].txChannel, (dmaInterrupt_t)BTC, (dmaIntGroup_t)DMA_INTA);

        dma_controlPacketSpiTx.DADD = (uint32_t)(&(dma_spiInterfaces[i]->DAT1)) + DMA_BIG_ENDIAN_ADDRESS_16BIT;
        dma_controlPacketSpiRx.SADD = (uint32_t)(&(dma_spiInterfaces[i]->BUF)) + DMA_BIG_ENDIAN_ADDRESS_16BIT;

        dmaEnableInterrupt_Expect(
            (dmaChannel_t)(dmaChannel_t)dma_spiDmaChannels[i].rxChannel, (dmaInterrupt_t)BTC, (dmaIntGroup_t)DMA_INTA);
        dmaSetCtrlPacket_Expect((dmaChannel_t)dma_spiDmaChannels[i].txChannel, dma_controlPacketSpiTx);
        dmaSetCtrlPacket_Expect((dmaChannel_t)dma_spiDmaChannels[i].rxChannel, dma_controlPacketSpiRx);
        dmaSetChEnable_Expect((dmaChannel_t)dma_spiDmaChannels[i].txChannel, (dmaTriggerType_t)DMA_HW);
        dmaSetChEnable_Expect((dmaChannel_t)dma_spiDmaChannels[i].rxChannel, (dmaTriggerType_t)DMA_HW);
    }

    /* Test I2C1 */

    dmaReqAssign_Expect((dmaChannel_t)DMA_CHANNEL_I2C1_TX, (dmaRequest_t)DMA_REQ_LINE_I2C1_TX);
    dmaReqAssign_Expect((dmaChannel_t)DMA_CHANNEL_I2C1_RX, (dmaRequest_t)DMA_REQ_LINE_I2C1_RX);
    dmaEnableInterrupt_Expect(
        (dmaChannel_t)(dmaChannel_t)DMA_CHANNEL_I2C1_TX, (dmaInterrupt_t)BTC, (dmaIntGroup_t)DMA_INTA);
    dmaEnableInterrupt_Expect(
        (dmaChannel_t)(dmaChannel_t)DMA_CHANNEL_I2C1_RX, (dmaInterrupt_t)BTC, (dmaIntGroup_t)DMA_INTA);
    dmaEnableInterrupt_Expect(
        (dmaChannel_t)(dmaChannel_t)DMA_CHANNEL_I2C1_RX, (dmaInterrupt_t)LFS, (dmaIntGroup_t)DMA_INTA);

    dma_controlPacketI2cTx.DADD = (uint32_t)(&(i2cREG1->DXR)) + DMA_BIG_ENDIAN_ADDRESS_8BIT;
    dma_controlPacketI2cRx.SADD = (uint32_t)(&(i2cREG1->DRR)) + DMA_BIG_ENDIAN_ADDRESS_8BIT;

    dmaSetCtrlPacket_Expect((dmaChannel_t)DMA_CHANNEL_I2C1_TX, dma_controlPacketI2cTx);
    dmaSetCtrlPacket_Expect((dmaChannel_t)DMA_CHANNEL_I2C1_RX, dma_controlPacketI2cRx);
    dmaSetChEnable_Expect((dmaChannel_t)DMA_CHANNEL_I2C1_TX, (dmaTriggerType_t)DMA_HW);
    dmaSetChEnable_Expect((dmaChannel_t)DMA_CHANNEL_I2C1_RX, (dmaTriggerType_t)DMA_HW);

    /* Test for I2C2 */

    dmaReqAssign_Expect((dmaChannel_t)DMA_CHANNEL_I2C2_TX, (dmaRequest_t)DMA_REQ_LINE_I2C2_TX);
    dmaReqAssign_Expect((dmaChannel_t)DMA_CHANNEL_I2C2_RX, (dmaRequest_t)DMA_REQ_LINE_I2C2_RX);

    dmaEnableInterrupt_Expect(
        (dmaChannel_t)(dmaChannel_t)DMA_CHANNEL_I2C2_TX, (dmaInterrupt_t)BTC, (dmaIntGroup_t)DMA_INTA);
    dmaEnableInterrupt_Expect(
        (dmaChannel_t)(dmaChannel_t)DMA_CHANNEL_I2C2_RX, (dmaInterrupt_t)BTC, (dmaIntGroup_t)DMA_INTA);
    dmaEnableInterrupt_Expect(
        (dmaChannel_t)(dmaChannel_t)DMA_CHANNEL_I2C2_RX, (dmaInterrupt_t)LFS, (dmaIntGroup_t)DMA_INTA);

    dma_controlPacketI2cTx.DADD = (uint32_t)(&(i2cREG2->DXR)) + DMA_BIG_ENDIAN_ADDRESS_8BIT;
    dma_controlPacketI2cRx.SADD = (uint32_t)(&(i2cREG2->DRR)) + DMA_BIG_ENDIAN_ADDRESS_8BIT;

    dmaSetCtrlPacket_Expect((dmaChannel_t)DMA_CHANNEL_I2C2_TX, dma_controlPacketI2cTx);
    dmaSetCtrlPacket_Expect((dmaChannel_t)DMA_CHANNEL_I2C2_RX, dma_controlPacketI2cRx);
    dmaSetChEnable_Expect((dmaChannel_t)DMA_CHANNEL_I2C2_TX, (dmaTriggerType_t)DMA_HW);
    dmaSetChEnable_Expect((dmaChannel_t)DMA_CHANNEL_I2C2_RX, (dmaTriggerType_t)DMA_HW);

    /* Test for SCI4 */

    dmaReqAssign_Expect((dmaChannel_t)DMA_CHANNEL_SCI4_TX, (dmaRequest_t)DMA_REQ_LINE_SCI4_TX);
    dmaEnableInterrupt_Expect(
        (dmaChannel_t)(dmaChannel_t)DMA_CHANNEL_SCI4_TX, (dmaInterrupt_t)BTC, (dmaIntGroup_t)DMA_INTA);

    dma_controlPacketSci4Tx.DADD   = (uint32_t)(&(UART_REG->TD)) + DMA_BIG_ENDIAN_ADDRESS_8BIT;
    dma_controlPacketSci4Tx.RDSIZE = (uint32_t)ACCESS_8_BIT;
    dma_controlPacketSci4Tx.WRSIZE = (uint32_t)ACCESS_8_BIT;

    dmaSetCtrlPacket_Expect((dmaChannel_t)DMA_CHANNEL_SCI4_TX, dma_controlPacketSci4Tx);
    dmaSetChEnable_Expect((dmaChannel_t)DMA_CHANNEL_SCI4_TX, (dmaTriggerType_t)DMA_HW);

    DMA_Initialize();
}
