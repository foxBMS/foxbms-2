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
 * @file    test_dma.c
 * @author  foxBMS Team
 * @date    2020-04-01 (date of creation)
 * @updated 2025-08-07 (date of last update)
 * @version v1.10.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Tests for the dma module
 * @details TODO
 *
 */

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

void testDummy(void) {
}
