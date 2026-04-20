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
 * @file    test_emac.c
 * @author  foxBMS Team
 * @date    2025-08-14 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of some module
 * @details Detailed Test Description
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "MockFreeRTOS_IP.h"
#include "MockHL_mdio.h"
#include "MockNetworkInterface.h"
#include "Mockdp83869.h"
#include "Mockemac-low-level.h"
#include "Mockfoxmath.h"
#include "Mockftask.h"
#include "Mockos.h"

#include "emac.h"
#include "ftask.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("emac.c")

TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/emac")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/phy")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/driver/uart")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")
TEST_INCLUDE_PATH("../../src/os/freertos/freertos-plus/freertos-plus-tcp/source/include")
TEST_INCLUDE_PATH("../../src/os/freertos/freertos-plus/freertos-plus-tcp/source/portable/Compiler/CCS")

/*========== Definitions and Implementations for Unit Test ==================*/
#define TEST_MDIO_BASE_ADDRESS (0xFCF78900u)
#define TEST_EMAC_BASE_ADDRESS (0xFCF78000u)

#define TEST_PHY_ADDRESS (1u)

struct EmacTxBufferDescriptor {
    volatile struct EmacTxBufferDescriptor *next;
    volatile uint32_t pBuffer;               /* Pointer to the actual Buffer storing the data to be transmitted. */
    volatile uint32_t bufferOffsetAndLength; /*Buffer Offset and Buffer Length (16 bits each) */
    volatile uint32_t flagsAndPacketLength;  /* Status flags and Packet Length. (16 bits each)*/
} EMAC_TX_BUFFER_DESCRIPTOR_s;

EMAC_HDKIF_s emac_hdkifData[1];
OS_TASK_HANDLE ftsk_taskHandleEmac;

uint32_t test_Ram[2048u];
uint32_t test_Hdp                  = 0u;
uint32_t EMAC_CTRL_RAM_0_BASE      = 0u;
uint32_t *EMAC_TX0HDP              = &test_Hdp;
uint8_t emac_rxBuffers[10u][1536u] = {0};
EMAC_PACKET_BUFFER_s emac_txBuffer[3u];

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    EMAC_CTRL_RAM_0_BASE = (uint32_t)(&test_Ram[0u]);

    EMAC_HDKIF_s *hdkif             = &emac_hdkifData[0];
    hdkif->emacBaseAddress          = TEST_EMAC_BASE_ADDRESS;
    hdkif->mdioBaseAddress          = TEST_MDIO_BASE_ADDRESS;
    hdkif->emacControllerRamAddress = EMAC_CTRL_RAM_0_BASE;
    hdkif->phyAddress               = TEST_PHY_ADDRESS;
    hdkif->pPhyAutonegotiate        = &PHY_AutoNegotiate;
    hdkif->pPhyPartnerAbility       = &PHY_GetPartnerAbility;
}

void tearDown(void) {
}

uint32_t TEST_SwapBytesUint32_t(const uint32_t val, int NumCalls) {
    return val;
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing external function #testEMAC_SetupLink
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - AT1/1: Invalid hdkif pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Autonegotiation passes and partner supports duplex
 *            - RT2/3: Autonegotiation passes and partner doesn't support duplex
 *            - RT3/3: Autonegotiation fails
 */
void testEMAC_SetupLink(void) {
    /* ======= Assertion tests ============================================= */
    uint16_t partner_ability = 0;
    uint16_t partner_100fd   = 0x0100u;
    uint16_t partner_10bt    = 0x20;
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_EMAC_SetupLink(NULL_PTR));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/3: Test implementation */
    PHY_AutoNegotiate_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1E0u, STD_OK);
    PHY_GetPartnerAbility_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, &partner_ability, true);
    PHY_GetPartnerAbility_ReturnThruPtr_pPartnerAbility(&partner_100fd);
    EMAC_SetDuplexMode_Expect(TEST_EMAC_BASE_ADDRESS, 0x1u);
    OS_DelayTask_Expect(5u);
    /* ======= RT1/3: Call function under test */
    TEST_EMAC_SetupLink(&emac_hdkifData[0]);
    /* ======= RT2/3: Test implementation */
    PHY_AutoNegotiate_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1E0u, STD_OK);
    PHY_GetPartnerAbility_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, &partner_ability, true);
    PHY_GetPartnerAbility_ReturnThruPtr_pPartnerAbility(&partner_10bt);
    EMAC_SetDuplexMode_Expect(TEST_EMAC_BASE_ADDRESS, 0x0u);
    OS_DelayTask_Expect(5u);
    /* ======= RT2/3: Call function under test */
    TEST_EMAC_SetupLink(&emac_hdkifData[0]);
    /* ======= RT3/3: Test implementation */
    PHY_AutoNegotiate_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1E0u, STD_NOT_OK);
    OS_DelayTask_Expect(5u);
    /* ======= RT2/3: Call function under test */
    EMAC_RETURN_TYPE_e returnValue = TEST_EMAC_SetupLink(&emac_hdkifData[0]);
    TEST_ASSERT_EQUAL((uint32_t)EMAC_ERROR_CONNECT, returnValue);
}

/**
 * @brief   Testing external function #EMAC_InitializeHardware
 * @details The following cases need to be tested:
 *          - Argument validation:
 *          - Routine validation:
 *            - RT1/2: Function calls expected subroutine
 *            - RT2/2: Setup link fails
 *
 */
void testEMAC_InitializeHardware(void) {
    uint8_t eth_emacAddress[6u]         = {0x0u, 0x08u, 0xEEu, 0x03u, 0xA6u, 0x6Cu};
    uint8_t eth_emacAddress_swapped[6u] = {0x6Cu, 0xA6u, 0x03u, 0xEEu, 0x08u, 0x0u};
    uint16_t partner_ability            = 0;
    uint16_t partner_100fd              = 0x0100u;
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    EMAC_InitializeRegisters_Expect(0xFCF78800u, TEST_EMAC_BASE_ADDRESS);
    MDIOInit_Expect(TEST_MDIO_BASE_ADDRESS, 100000000, 1000000);
    EMAC_SetMacSourceAddress_Expect(TEST_EMAC_BASE_ADDRESS, eth_emacAddress_swapped);
    for (uint8_t channel = 0; channel < 8; channel++) {
        EMAC_SetMacAddress_Expect(TEST_EMAC_BASE_ADDRESS, channel, eth_emacAddress_swapped, 0x00180000u);
    }
    /* Call EMAC_SetupLink() */
    PHY_AutoNegotiate_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1E0u, STD_OK);
    PHY_GetPartnerAbility_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, &partner_ability, true);
    PHY_GetPartnerAbility_ReturnThruPtr_pPartnerAbility(&partner_100fd);
    EMAC_SetDuplexMode_Expect(TEST_EMAC_BASE_ADDRESS, 0x1u);
    OS_DelayTask_Expect(5u);

    EMAC_AcknowledgeControlCoreInterrupt_Expect(TEST_EMAC_BASE_ADDRESS, 0x1u);
    EMAC_AcknowledgeControlCoreInterrupt_Expect(TEST_EMAC_BASE_ADDRESS, 0x2u);

    EMAC_EnableMii_Expect(TEST_EMAC_BASE_ADDRESS);

    EMAC_EnableRxBroadCast_Expect(TEST_EMAC_BASE_ADDRESS, 0u);
    EMAC_EnableRxMultiCast_Expect(TEST_EMAC_BASE_ADDRESS, 0u);
    EMAC_SetRxUnicast_Expect(TEST_EMAC_BASE_ADDRESS, 0u);

    EMAC_SetDuplexMode_Expect(TEST_EMAC_BASE_ADDRESS, 0x1u);

    /* Enable Loopback based on GUI Input */
#if (EMAC_LOOPBACK_ENABLE)
    EMAC_EnableLoopback_Expect(TEST_EMAC_BASE_ADDRESS);
#else
    EMAC_DisableLoopback_Expect(TEST_EMAC_BASE_ADDRESS);
#endif

    EMAC_EnableTx_Expect(TEST_EMAC_BASE_ADDRESS);
    EMAC_EnableTxInterruptPulse_Expect(TEST_EMAC_BASE_ADDRESS, 0xFCF78800u, 0x0u, 0x0u);

    EMAC_SetNumberFreeBuffer_Expect(TEST_EMAC_BASE_ADDRESS, 0x0u, 10u);
    EMAC_EnableRx_Expect(TEST_EMAC_BASE_ADDRESS);
    EMAC_EnableRxInterruptPulse_Expect(TEST_EMAC_BASE_ADDRESS, 0xFCF78800u, 0x0u, 0x0u);
    EMAC_WriteRxHeaderDescriptorPointer_Expect(
        TEST_EMAC_BASE_ADDRESS, (uint32_t)emac_hdkifData->pRxChannel.pActiveHead, 0x0u);

    PHY_Initialize_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, STD_OK);
    /* ======= RT1/2: Call function under test */
    EMAC_InitializeHardware(eth_emacAddress);

    /* ======= RT2/2: Test implementation */
    EMAC_InitializeRegisters_Expect(0xFCF78800u, TEST_EMAC_BASE_ADDRESS);
    MDIOInit_Expect(TEST_MDIO_BASE_ADDRESS, 100000000, 1000000);
    EMAC_SetMacSourceAddress_Expect(TEST_EMAC_BASE_ADDRESS, eth_emacAddress_swapped);
    for (uint8_t channel = 0; channel < 8; channel++) {
        EMAC_SetMacAddress_Expect(TEST_EMAC_BASE_ADDRESS, channel, eth_emacAddress_swapped, 0x00180000u);
    }
    /* Call EMAC_SetupLink() */
    PHY_AutoNegotiate_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, 0x1E0u, STD_NOT_OK);
    OS_DelayTask_Expect(5u);

    EMAC_AcknowledgeControlCoreInterrupt_Expect(TEST_EMAC_BASE_ADDRESS, 0x1u);
    EMAC_AcknowledgeControlCoreInterrupt_Expect(TEST_EMAC_BASE_ADDRESS, 0x2u);

    EMAC_EnableMii_Expect(TEST_EMAC_BASE_ADDRESS);

    EMAC_EnableRxBroadCast_Expect(TEST_EMAC_BASE_ADDRESS, 0u);
    EMAC_EnableRxMultiCast_Expect(TEST_EMAC_BASE_ADDRESS, 0u);
    EMAC_SetRxUnicast_Expect(TEST_EMAC_BASE_ADDRESS, 0u);

    EMAC_SetDuplexMode_Expect(TEST_EMAC_BASE_ADDRESS, 0x1u);

    /* Enable Loopback based on GUI Input */
#if (EMAC_LOOPBACK_ENABLE)
    EMAC_EnableLoopback_Expect(TEST_EMAC_BASE_ADDRESS);
#else
    EMAC_DisableLoopback_Expect(TEST_EMAC_BASE_ADDRESS);
#endif

    EMAC_EnableTx_Expect(TEST_EMAC_BASE_ADDRESS);
    EMAC_EnableTxInterruptPulse_Expect(TEST_EMAC_BASE_ADDRESS, 0xFCF78800u, 0x0u, 0x0u);

    EMAC_SetNumberFreeBuffer_Expect(TEST_EMAC_BASE_ADDRESS, 0x0u, 10u);
    EMAC_EnableRx_Expect(TEST_EMAC_BASE_ADDRESS);
    EMAC_EnableRxInterruptPulse_Expect(TEST_EMAC_BASE_ADDRESS, 0xFCF78800u, 0x0u, 0x0u);
    EMAC_WriteRxHeaderDescriptorPointer_Expect(
        TEST_EMAC_BASE_ADDRESS, (uint32_t)emac_hdkifData->pRxChannel.pActiveHead, 0x0u);

    PHY_Initialize_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, STD_NOT_OK);
    /* ======= RT2/2: Call function under test */
    EMAC_RETURN_TYPE_e returnValue = EMAC_InitializeHardware(eth_emacAddress);
    TEST_ASSERT_EQUAL(EMAC_ERROR_PHY_INITIALIZATION, returnValue);
}

/**
 * @brief   Testing external function #EMAC_InitializeDma
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - None
 *          - Routine validation:
 *            - RT1/2: Function calls expected subroutine
 *            - RT2/2: Setup link fails
 *
 */
void testEMAC_InitializeDma(void) {
    EMAC_TX_CHANNEL_s *txChannelDma = &(emac_hdkifData[0].pTxChannel);
    volatile pEmacTxBufferDescriptor txBufferDescriptor;
    volatile EMAC_RX_BUFFER_DESCRIPTOR_s *pRxBufferDescriptor;
    txChannelDma->pFreeHead = (volatile pEmacTxBufferDescriptor)emac_hdkifData[0].emacControllerRamAddress;
    txBufferDescriptor      = (volatile pEmacTxBufferDescriptor)txChannelDma->pFreeHead;

    /* Add stub for SwapBytes. The normal leads only returns the half pointer for pBuffer */
    MATH_SwapBytesUint32_Stub(TEST_SwapBytesUint32_t);
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    /* Initialize tx buffer */
    volatile pEmacTxBufferDescriptor swappedBytes;
    for (uint16_t i = 0u; i < 170u; i++) {
        swappedBytes = (txBufferDescriptor + 1u + (i));
        MATH_SwapBytesUint32_ExpectAndReturn((uint32_t)(swappedBytes), (uint32_t)(swappedBytes));
        MATH_SwapBytesUint32_ExpectAndReturn((uint32_t)(swappedBytes), (uint32_t)(swappedBytes));
    }

    MATH_SwapBytesUint32_ExpectAndReturn((uint32_t)(txBufferDescriptor), (uint32_t)(txBufferDescriptor));

    /* Initialize rx buffer */
    pRxBufferDescriptor = (volatile EMAC_RX_BUFFER_DESCRIPTOR_s *)swappedBytes + 1u;

    MATH_SwapBytesUint32_ExpectAndReturn(1536u, 1536u);
    MATH_SwapBytesUint32_ExpectAndReturn(0x20000000u, 0x20000000u);
    for (uint8_t i = 0u; i < 10u; i++) {
        MATH_SwapBytesUint32_ExpectAndReturn(
            (uint32_t)(&emac_rxBuffers[i][0]) + 14u, (uint8_t)(&emac_rxBuffers[i][0]) + 14u);
        if (i != (9u)) {
            MATH_SwapBytesUint32_ExpectAndReturn(
                (uint32_t)(pRxBufferDescriptor + i + 1u), (uint32_t)(pRxBufferDescriptor + i + 1u));
        }
    }
    /* ======= RT1/1: Call function under test */
    EMAC_InitializeDma();
}

/**
 * @brief   Testing external function #testEMAC_EmacIsOwner
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: Invalid buffer pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutine
 */
void testEMAC_EmacIsOwner(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(EMAC_EmacIsOwner(NULL_PTR));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    MATH_SwapBytesUint32_ExpectAndReturn(
        (uint32_t)emac_hdkifData->pRxChannel.pActiveHead->flagsAndPacketLength,
        (uint32_t)emac_hdkifData->pRxChannel.pActiveHead->flagsAndPacketLength);

    /* ======= RT1/1: Call function under test */
    EMAC_EmacIsOwner(emac_hdkifData->pRxChannel.pActiveHead);
}

/**
 * @brief   Testing external function #EMAC_Transmit
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - AT1/1: Invalid buffer pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/2: First call of function
 *            - RT2/2: Second call of function
 *
 */
void testEMAC_Transmit(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(EMAC_Transmit(NULL_PTR));
    /* ======= Routine tests =============================================== */
    volatile pEmacTxBufferDescriptor pCurrentBufferDescriptor;
    uint8_t testPayload[8u]            = {'D', 'E', 'A', 'D', 'B', 'E', 'E', 'F'};
    EMAC_PACKET_BUFFER_s dmaDescriptor = {.length = 8, .totalLength = 8, .next = NULL, .pPayload = testPayload};
    pCurrentBufferDescriptor           = emac_hdkifData->pTxChannel.pFreeHead;

    /* ======= RT1/2: Test implementation */
    /* Indicate the start of the packet */
    MATH_SwapBytesUint32_ExpectAndReturn(0xA0000008u, 0xA0000008u);

    MATH_SwapBytesUint32_ExpectAndReturn((uint32_t)&testPayload[0u], (uint32_t)&testPayload[0u]);
    MATH_SwapBytesUint32_ExpectAndReturn(
        (uint32_t)(dmaDescriptor.length & 0xFFFFu), (uint32_t)(dmaDescriptor.length & 0xFFFFu));
    MATH_SwapBytesUint32_ExpectAndReturn(
        (uint32_t)pCurrentBufferDescriptor->next, (uint32_t)pCurrentBufferDescriptor->next);
    /* Indicate the end of the packet */
    MATH_SwapBytesUint32_ExpectAndReturn(0x40000000u, 0x40000000u);

    EMAC_WriteTxHeaderDescriptorPointer_Expect(TEST_EMAC_BASE_ADDRESS, (uint32_t)pCurrentBufferDescriptor, 0u);

    /* ======= RT1/2: Call function under test */
    EMAC_Transmit(&dmaDescriptor);

    /* ======= RT2/2: Test implementation */
    pCurrentBufferDescriptor = emac_hdkifData->pTxChannel.pFreeHead;
    /* Indicate the start of the packet */
    MATH_SwapBytesUint32_ExpectAndReturn(0xA0000008u, 0xA0000008u);

    MATH_SwapBytesUint32_ExpectAndReturn((uint32_t)&testPayload[0u], (uint32_t)&testPayload[0u]);
    MATH_SwapBytesUint32_ExpectAndReturn(
        (uint32_t)(dmaDescriptor.length & 0xFFFFu), (uint32_t)(dmaDescriptor.length & 0xFFFFu));
    MATH_SwapBytesUint32_ExpectAndReturn(
        (uint32_t)pCurrentBufferDescriptor->next, (uint32_t)pCurrentBufferDescriptor->next);
    /* Indicate the end of the packet */
    MATH_SwapBytesUint32_ExpectAndReturn(0x40000000u, 0x40000000u);

    emac_hdkifData->pTxChannel.pActiveTail->flagsAndPacketLength = 0x10000008u;
    MATH_SwapBytesUint32_ExpectAndReturn(0x10000008u, 0x10000008u);
    MATH_SwapBytesUint32_ExpectAndReturn(
        (uint32_t)emac_hdkifData->pTxChannel.pFreeHead, (uint32_t)emac_hdkifData->pTxChannel.pFreeHead);

    MATH_SwapBytesUint32_ExpectAndReturn(0x10000008u, 0x10000008u);

    EMAC_WriteTxHeaderDescriptorPointer_Expect(TEST_EMAC_BASE_ADDRESS, (uint32_t)pCurrentBufferDescriptor, 0u);

    /* ======= RT2/2: Call function under test */
    EMAC_Transmit(&dmaDescriptor);
}

/**
 * @brief   Testing external function #EMAC_InitializeTxBufferDescriptors
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - none
 *          - Routine validation:
 *            - RT1/1: unction calls expected subroutine
 *
 */
void test_EMAC_InitializeTxBufferDescriptors(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    /* ======= RT1/1: Call function under test */
    EMAC_InitializeTxBufferDescriptors();
    TEST_ASSERT_EQUAL(emac_txBuffer[0u].next, &emac_txBuffer[1u]);
    TEST_ASSERT_EQUAL(emac_txBuffer[1u].next, &emac_txBuffer[2u]);
    TEST_ASSERT_EQUAL(emac_txBuffer[2u].next, NULL_PTR);
}

/**
 * @brief   Testing external function #EMAC_InitializeTxBufferDescriptors
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - none
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutine
 *
 */
void test_EMAC_GetNextTxDescriptor(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    /* ======= RT1/1: Call function under test */
    EMAC_PACKET_BUFFER_s *pFirstBufferDescriptor  = EMAC_GetNextTxDescriptor();
    EMAC_PACKET_BUFFER_s *pSecondBufferDescriptor = EMAC_GetNextTxDescriptor();
    TEST_ASSERT_NOT_EQUAL(pFirstBufferDescriptor, pSecondBufferDescriptor);
}

/**
 * @brief   Testing external function #EMAC_StartOfPacket
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: Invalid buffer pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutine
 *
 */
void testEMAC_StartOfPacket(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(EMAC_EndOfReception(NULL_PTR));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    MATH_SwapBytesUint32_ExpectAndReturn(
        (uint32_t)emac_hdkifData->pRxChannel.pActiveHead->flagsAndPacketLength,
        (uint32_t)emac_hdkifData->pRxChannel.pActiveHead->flagsAndPacketLength);
    /* ======= RT1/1: Call function under test */
    bool startOfPacket = EMAC_StartOfPacket(emac_hdkifData->pRxChannel.pActiveHead);
    TEST_ASSERT_EQUAL(false, startOfPacket);
}

/**
 * @brief   Testing external function #EMAC_AcknowledgePacket
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - AT1/1: Invalid buffer pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutine
 *
 */
void testEMAC_AcknowledgePacket(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(EMAC_EndOfReception(NULL_PTR));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    EMAC_WriteRxCompletionPointer_Expect(
        TEST_EMAC_BASE_ADDRESS, (uint32_t)EMAC_CHANNELNUMBER, (uint32_t)emac_hdkifData->pRxChannel.pActiveHead);
    /* ======= RT1/1: Call function under test */
    EMAC_AcknowledgePacket(emac_hdkifData->pRxChannel.pActiveHead);
}

/**
 * @brief   Testing external function #EMAC_UpdateRxChannel
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - AT1/1: Invalid buffer pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutine
 *
 */
void testEMAC_UpdateRxChannel(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(EMAC_EndOfReception(NULL_PTR));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    MATH_SwapBytesUint32_ExpectAndReturn(
        (uint32_t)emac_hdkifData->pRxChannel.pActiveHead->next, (uint32_t)emac_hdkifData->pRxChannel.pActiveHead->next);
    MATH_SwapBytesUint32_ExpectAndReturn(
        (uint32_t)emac_hdkifData->pRxChannel.pActiveHead->next, (uint32_t)emac_hdkifData->pRxChannel.pActiveHead->next);
    /* ======= RT1/1: Call function under test */
    EMAC_UpdateRxChannel(emac_hdkifData->pRxChannel.pActiveHead);
}

/**
 * @brief   Testing external function #EMAC_UpdateLinkedList
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: Invalid buffer pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/2: Reception is ongoing
 *            - RT2/2: Reception has ended
 *
 */
void testEMAC_EndOfReception(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(EMAC_EndOfReception(NULL_PTR));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    MATH_SwapBytesUint32_ExpectAndReturn((uint32_t)0x20000000u, (uint32_t)0x20000000u);

    MATH_SwapBytesUint32_ExpectAndReturn(
        (uint32_t)emac_hdkifData->pRxChannel.pActiveHead->next, (uint32_t)emac_hdkifData->pRxChannel.pActiveHead->next);
    /* ======= RT1/2: Call function under test */
    EMAC_EndOfReception(emac_hdkifData->pRxChannel.pActiveHead);

    /* ======= RT2/2: Test implementation */
    MATH_SwapBytesUint32_ExpectAndReturn((uint32_t)0x10000000u, (uint32_t)0x10000000u);
    EMAC_WriteRxHeaderDescriptorPointer_Expect(
        TEST_EMAC_BASE_ADDRESS, (uint32_t)emac_hdkifData->pRxChannel.pFreeHead, (uint32_t)EMAC_CHANNELNUMBER);

    MATH_SwapBytesUint32_ExpectAndReturn(
        (uint32_t)emac_hdkifData->pRxChannel.pActiveHead->next, (uint32_t)emac_hdkifData->pRxChannel.pActiveHead->next);
    /* ======= RT2/2: Call function under test */
    emac_hdkifData->pRxChannel.pActiveHead->flagsAndPacketLength = 0x10000000u;
    EMAC_EndOfReception(emac_hdkifData->pRxChannel.pActiveHead);
}

/**
 * @brief   Testing external function #EMAC_UpdateLinkedList
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: Invalid buffer pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutine
 *
 */
void testEMAC_UpdateLinkedList(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(EMAC_UpdateLinkedList(NULL_PTR));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    MATH_SwapBytesUint32_ExpectAndReturn((uint32_t)0x20000000u, (uint32_t)0x20000000u);
    MATH_SwapBytesUint32_ExpectAndReturn((uint32_t)1536u, (uint32_t)1536u);
    MATH_SwapBytesUint32_ExpectAndReturn(
        (uint32_t)emac_hdkifData->pRxChannel.pActiveHead->next, (uint32_t)emac_hdkifData->pRxChannel.pActiveHead->next);
    /* ======= RT1/1: Call function under test */
    EMAC_UpdateLinkedList(emac_hdkifData->pRxChannel.pActiveHead);
    TEST_ASSERT_EQUAL(0x20000000u, emac_hdkifData->pRxChannel.pActiveHead->flagsAndPacketLength);
    TEST_ASSERT_EQUAL(0x00000600u, emac_hdkifData->pRxChannel.pActiveHead->bufferOffsetAndLength);
}

/**
 * @brief   Testing external function #EMAC_GetRxActiveHead
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - none
 *          - Routine validation:
 *            - RT1/1: Get buffer pointer
 *
 */
void testEMAC_GetRxActiveHead(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    /* ======= RT1/1: Call function under test */
    volatile EMAC_RX_BUFFER_DESCRIPTOR_s *testBufferDescriptor = EMAC_GetRxActiveHead();
    TEST_ASSERT_EQUAL(emac_hdkifData->pRxChannel.pActiveHead, testBufferDescriptor);
}

/**
 * @brief   Testing external function #EMAC_GetRxActiveTail
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - none
 *          - Routine validation:
 *            - RT1/1: Get buffer pointer
 *
 */
void testEMAC_GetRxActiveTail(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    /* ======= RT1/1: Call function under test */
    volatile EMAC_RX_BUFFER_DESCRIPTOR_s *testBufferDescriptor = EMAC_GetRxActiveTail();
    TEST_ASSERT_EQUAL(emac_hdkifData->pRxChannel.pActiveTail, testBufferDescriptor);
}

/**
 * @brief   Testing external function #EMAC_ReceivedDataLengthInBytes
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - none
 *          - Routine validation:
 *            - RT1/1: Call function as expected
 *
 */
void testEMAC_ReceiveSize(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    MATH_SwapBytesUint32_ExpectAndReturn(
        emac_hdkifData->pRxChannel.pActiveHead->flagsAndPacketLength,
        emac_hdkifData->pRxChannel.pActiveHead->flagsAndPacketLength);
    /* ======= RT1/1: Call function under test */
    uint16_t receivedSize = EMAC_ReceivedDataLengthInBytes();
    TEST_ASSERT_LESS_OR_EQUAL(8u, receivedSize);
}

/**
 * @brief   Testing external function #EMAC_TxInterruptServiceRoutine
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - none
 *          - Routine validation:
 *            - RT1/1: Call function as expected
 *
 */
void testEMAC_TxInterruptServiceRoutine(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    /* Call EMAC_TxInterruptHandler()*/
    MATH_SwapBytesUint32_ExpectAndReturn(0x10000008u, 0x10000008u);

    EMAC_AcknowledgeControlCoreInterrupt_Expect(TEST_EMAC_BASE_ADDRESS, 0x2u);
    /* ======= RT1/1: Call function under test */
    TEST_EMAC_TxInterruptServiceRoutine();
}

/**
 * @brief   Testing external function #EMAC_RxInterruptServiceRoutine
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - none
 *          - Routine validation:
 *            - RT1/1: Call function as expected
 *
 */
void testEMAC_RxInterruptServiceRoutine(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    OS_NotifyGiveFromIsr_Expect(ftsk_taskHandleEmac, &higherPriorityTaskWoken);
    /* ======= RT1/1: Call function under test */
    TEST_EMAC_RxInterruptServiceRoutine();
}

/**
 * @brief   Testing external function #EMAC_TxInterruptHandler
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - none
 *          - Routine validation:
 *            - RT1/1: Call function as expected
 *
 */
void testEMAC_TxInterruptHandler(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    /* Set Flags*/
    emac_hdkifData->pTxChannel.pNextBufferDescriptorToProcess->flagsAndPacketLength = 0x80000008u;
    /* ======= RT1/1: Test implementation */
    MATH_SwapBytesUint32_ExpectAndReturn(0x80000008u, 0x80000008u);

    MATH_SwapBytesUint32_ExpectAndReturn(0x80000008u, 0x80000008u);

    MATH_SwapBytesUint32_ExpectAndReturn(0x80000008u, 0x80000008u);

    MATH_SwapBytesUint32_ExpectAndReturn(
        (uint32_t)emac_hdkifData->pTxChannel.pNextBufferDescriptorToProcess->next,
        (uint32_t)emac_hdkifData->pTxChannel.pNextBufferDescriptorToProcess->next);

    emac_hdkifData->pTxChannel.pNextBufferDescriptorToProcess->next->flagsAndPacketLength = 0x40000008u;
    MATH_SwapBytesUint32_ExpectAndReturn(0x40000008u, 0x40000008u);

    MATH_SwapBytesUint32_ExpectAndReturn(0x80000000u, 0x80000000u);
    MATH_SwapBytesUint32_ExpectAndReturn(0x40000000u, 0x40000000u);

    EMAC_WriteTxCompletionPointer_Expect(
        TEST_EMAC_BASE_ADDRESS, 0u, (uint32_t)emac_hdkifData->pTxChannel.pNextBufferDescriptorToProcess->next);
    MATH_SwapBytesUint32_ExpectAndReturn(0x00000000u, 0x00000000u);

    /* ======= RT1/1: Call function under test */
    EMAC_TxInterruptHandler(&emac_hdkifData[0]);
}

/**
 * @brief   Testing external function #EMAC_GetPhyLinkStatus
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - none
 *          - Routine validation:
 *            - RT1/1: Call function as expected
 *
 */
void testEMAC_GetPhyLinkStatus(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    PHY_GetLinkStatus_ExpectAndReturn(TEST_MDIO_BASE_ADDRESS, TEST_PHY_ADDRESS, STD_OK);
    /* ======= RT1/1: Call function under test */
    STD_RETURN_TYPE_e linkStatus = EMAC_GetPhyLinkStatus();
    TEST_ASSERT_EQUAL(STD_OK, linkStatus);
}

/**
 * @brief   Testing external function #EMAC_SetNextPointerOfCurrentTail
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: Invalid buffer pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutine
 *
 */
void testEMAC_SetNextPointerOfCurrentTail(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(EMAC_SetNextPointerOfCurrentTail(NULL_PTR));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    MATH_SwapBytesUint32_ExpectAndReturn(
        (uint32_t)emac_hdkifData->pRxChannel.pFreeHead->next, (uint32_t)emac_hdkifData->pRxChannel.pFreeHead->next);
    /* ======= RT1/1: Call function under test */
    EMAC_SetNextPointerOfCurrentTail(emac_hdkifData->pRxChannel.pFreeHead->next);
}

/**
 * @brief   Testing external function #EMAC_GetEthernetBuffer
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: Invalid buffer pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutine
 *
 */
void testEMAC_GetEthernetBuffer(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(EMAC_GetEthernetBuffer(NULL_PTR));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    MATH_SwapBytesUint32_ExpectAndReturn(
        (uint32_t)emac_hdkifData->pRxChannel.pActiveHead->pBuffer,
        (uint32_t)emac_hdkifData->pRxChannel.pActiveHead->pBuffer);
    /* ======= RT1/1: Call function under test */
    uint8_t *buffer = EMAC_GetEthernetBuffer(emac_hdkifData->pRxChannel.pActiveHead);
    TEST_ASSERT_EQUAL(emac_hdkifData->pRxChannel.pActiveHead->pBuffer, buffer);
}

/**
 * @brief   Testing external function #EMAC_AcknowledgeRxInterrupt
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutine
 *
 */
void testEMAC_AcknowledgeRxInterrupt(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    EMAC_AcknowledgeControlCoreInterrupt_Expect(TEST_EMAC_BASE_ADDRESS, 0x01u);
    /* ======= RT1/1: Call function under test */
    EMAC_AcknowledgeRxInterrupt();
}
