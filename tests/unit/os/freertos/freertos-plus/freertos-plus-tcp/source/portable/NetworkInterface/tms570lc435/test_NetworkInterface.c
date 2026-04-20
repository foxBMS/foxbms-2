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
 * @file    test_NetworkInterface.c
 * @author  foxBMS Team
 * @date    2020-08-10 (date of creation)
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
#include "MockFreeRTOS_Routing.h"
#include "MockNetworkBufferManagement.h"
#include "Mockdatabase.h"
#include "Mockemac-low-level.h"
#include "Mockemac.h"
#include "Mockfoxmath.h"
#include "Mockinfinite-loop-helper.h"
#include "Mockos.h"

#include "NetworkInterface.h"
#include "NetworkInterface_custom.h"
#include "fstd_types.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("NetworkInterface.c")
TEST_INCLUDE_PATH("../../src/os/freertos/freertos-plus/freertos-plus-tcp/source/include")
TEST_INCLUDE_PATH("../../src/os/freertos/freertos-plus/freertos-plus-tcp/source/portable/Compiler/CCS")
TEST_INCLUDE_PATH("../../src/os/freertos/freertos-plus/freertos-plus-tcp/source/portable/NetworkInterface/tms570lc435")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/emac")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/uart")

/*========== Definitions and Implementations for Unit Test ==================*/
BaseType_t xSendEventStructToIPTask(const IPStackEvent_t *pxEvent, TickType_t uxTimeout) {
    return 1u;
}

#define BUFFER_SIZE         (ipTOTAL_ETHERNET_FRAME_SIZE + ipBUFFER_PADDING)
#define BUFFER_SIZE_ROUNDED ((BUFFER_SIZE + 7u) & ~0x0007u)

NetworkInterface_t xInterfaces[1];
NetworkEndPoint_t endpoint;
NetworkInterface_t *nic_pInterface = &xInterfaces[0];
uint8_t emac_rxBuffers[10u][1536u] = {0};
/* static EMAC_PACKET_BUFFER_s emacTxBuffer = {0}; */

static NetworkBufferDescriptor_t xNetworkBuffers[10];
volatile EMAC_RX_BUFFER_DESCRIPTOR_s testBufferDescriptor =
    {.pBuffer = 0, .next = NULL_PTR, .bufferOffsetAndLength = 0, .flagsAndPacketLength = 0};

static DATA_BLOCK_PHY_s testTablePhy = {.header.uniqueId = DATA_BLOCK_ID_PHY};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    testBufferDescriptor.pBuffer = (uint32_t)&(emac_rxBuffers[0u][0u]);
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing external function #NIC_FormatBuffer
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - AT1/1: Invalid network buffer descriptor &rarr; assert
 *             - AT1/2: Invalid EMAC buffer &rarr; assert
 *          - Routine validation:
 *            - RT1/2: Function formats buffer correctly
 *            - RT2/2: Too short message needs padding

 */
void testNIC_FormatBuffer(void) {
    NetworkBufferDescriptor_t testBuffer;
    uint8_t testData[100];
    testBuffer.pucEthernetBuffer = testData;
    testBuffer.xDataLength       = 100;

    EMAC_PACKET_BUFFER_s emacBuffer;
    uint8_t emacPayload[1500];
    emacBuffer.pPayload = emacPayload;

    /* Fill test data */
    for (uint32_t i = 0; i < 100; i++) {
        testData[i] = (uint8_t)i;
    }

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_NIC_FormatBuffer(NULL_PTR, &emacBuffer));
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_NIC_FormatBuffer(&testBuffer, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Call function under test */
    TEST_NIC_FormatBuffer(&testBuffer, &emacBuffer);

    /* ======= RT1/2: Verify results */
    TEST_ASSERT_EQUAL(100, emacBuffer.totalLength);
    TEST_ASSERT_EQUAL(100, emacBuffer.length);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(testData, emacBuffer.pPayload, 100);

    /* ======= RT2/2: Test implementation */
    testBuffer.xDataLength  = 48;
    uint8_t paddingData[12] = {0};
    /* ======= RT2/2: Call function under test */
    TEST_NIC_FormatBuffer(&testBuffer, &emacBuffer);
    /* ======= RT1/2: Verify results */
    TEST_ASSERT_EQUAL(60, emacBuffer.totalLength);
    TEST_ASSERT_EQUAL(60, emacBuffer.length);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(testData, emacBuffer.pPayload, 48);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(paddingData, emacBuffer.pPayload + 48, 12);
}

/**
 * @brief   Testing external function #xNetworkInterfaceInitialise
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - AT1/1: Invalid interface pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/1: EMAC initialization fails
 *            - RT2/2: EMAC initialization succeeds

 */
void testNetworkInterfaceInitialise(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_xNetworkInterfaceInitialise(NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    EMAC_InitializeHardware_ExpectAndReturn(xInterfaces[0].pxEndPoint->xMACAddress.ucBytes, 1);
    /* ======= RT1/1: Call function under test */
    BaseType_t result = TEST_xNetworkInterfaceInitialise(&xInterfaces[0]);
    TEST_ASSERT_EQUAL(pdFAIL, result);

    /* ======= RT2/2: Test implementation */
    EMAC_InitializeHardware_ExpectAndReturn(xInterfaces[0].pxEndPoint->xMACAddress.ucBytes, 0);
    /* ======= RT2/2: Call function under test */
    result = TEST_xNetworkInterfaceInitialise(&xInterfaces[0]);
    TEST_ASSERT_EQUAL(pdPASS, result);
}

/**
 * @brief   Testing static function #xNetworkInterfaceOutput
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - AT1/1: Invalid descriptor pointer &rarr; assert
 *             - AT1/2: Invalid network buffer pointer &rarr; assert
 *             - AT1/3: Invalid release after send value &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Transmission succeeds, release buffer
 *            - RT2/1: Transmission succeeds, don't release buffer
 *            - RT3/1: Transmission fails, release buffer

 */
void testNetworkInterfaceOutput(void) {
    NetworkBufferDescriptor_t testBuffer;
    uint8_t testData[100];
    testBuffer.pucEthernetBuffer = testData;
    testBuffer.xDataLength       = 100;

    /* Fill test data */
    for (uint32_t i = 0; i < 100; i++) {
        testData[i] = (uint8_t)i;
    }

    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_xNetworkInterfaceOutput(NULL_PTR, &testBuffer, pdTRUE));
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_xNetworkInterfaceOutput(&xInterfaces[0], NULL_PTR, pdTRUE));
    /* ======= AT1/3 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_xNetworkInterfaceOutput(&xInterfaces[0], &testBuffer, 5));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
}

/**
 * @brief   Testing external function #uxNetworkInterfaceAllocateRAMToBuffers
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - AT1/1: Invalid buffer pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutine
 *
 */
void testNetworkInterfaceAllocateRAMToBuffers(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(uxNetworkInterfaceAllocateRAMToBuffers(NULL_PTR));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    EMAC_InitializeDma_Expect();
    EMAC_InitializeTxBufferDescriptors_Expect();

    /* ======= RT1/1: Call function under test */
    uxNetworkInterfaceAllocateRAMToBuffers(xNetworkBuffers);
}

/**
 * @brief   Testing external function #xGetPhyLinkStatus
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - None
 *          - Routine validation:
 *            - RT1/1: Function calls expected subroutine
 *
 */
void testGetPhyLinkStatus(void) {
    testTablePhy.linkStatus = true;
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    DATA_Read1DataBlock_ExpectAndReturn(&testTablePhy, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&testTablePhy);
    EMAC_GetPhyLinkStatus_ExpectAndReturn(STD_OK);
    /* ======= RT1/1: Call function under test */
    BaseType_t linkStatus = xGetPhyLinkStatus(xInterfaces);
    TEST_ASSERT_EQUAL(1u, linkStatus);
}

void testNIC_TransferToTcp(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    volatile EMAC_RX_BUFFER_DESCRIPTOR_s *pNextBufferDescriptorToProcess = &testBufferDescriptor;
    /* ======= RT1/1: Test implementation */
    EMAC_ReceivedDataLengthInBytes_ExpectAndReturn(72);
    pxGetNetworkBufferWithDescriptor_ExpectAndReturn(72, 0, &xNetworkBuffers[0]);

    EMAC_GetRxActiveHead_ExpectAndReturn(pNextBufferDescriptorToProcess);
    EMAC_GetEthernetBuffer_ExpectAndReturn(pNextBufferDescriptorToProcess, &(emac_rxBuffers[0][0]));

    FreeRTOS_MatchingEndpoint_ExpectAndReturn(&xInterfaces[0], &(emac_rxBuffers[0][0]), &endpoint);

    /* TODO: Test all paths*/

    /* ======= RT1/1: Call function under test */
    TEST_NIC_TransferToTcp();
}

void testNIC_ProcessRxPacket(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    volatile EMAC_RX_BUFFER_DESCRIPTOR_s *pNextBufferDescriptorToProcess = &testBufferDescriptor;
    /* ======= RT1/1: Test implementation */
    EMAC_GetRxActiveHead_ExpectAndReturn(pNextBufferDescriptorToProcess);
    EMAC_GetRxActiveTail_ExpectAndReturn(pNextBufferDescriptorToProcess);
    EMAC_EmacIsOwner_ExpectAndReturn(pNextBufferDescriptorToProcess, false);

    /*NIC_TransferToTcp();*/
    EMAC_ReceivedDataLengthInBytes_ExpectAndReturn(72);
    pxGetNetworkBufferWithDescriptor_ExpectAndReturn(72, 0, &xNetworkBuffers[0]);
    EMAC_GetRxActiveHead_ExpectAndReturn(pNextBufferDescriptorToProcess);
    EMAC_GetEthernetBuffer_ExpectAndReturn(pNextBufferDescriptorToProcess, &(emac_rxBuffers[0][0]));
    FreeRTOS_MatchingEndpoint_ExpectAndReturn(&xInterfaces[0], &(emac_rxBuffers[0][0]), &endpoint);

    EMAC_UpdateLinkedList_Expect(pNextBufferDescriptorToProcess);

    /* Acknowledge that this packet is processed */
    EMAC_AcknowledgePacket_Expect(pNextBufferDescriptorToProcess);
    EMAC_SetNextPointerOfCurrentTail_Expect(pNextBufferDescriptorToProcess);
    EMAC_EndOfReception_Expect(pNextBufferDescriptorToProcess);
    EMAC_AcknowledgeRxInterrupt_Expect();

    /* ======= RT1/1: Call function under test */
    TEST_NIC_ProcessRxPacket();
}

/**
 * @brief   Testing external function #NIC_FillInterfaceDescriptor
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - AT1/1: Invalid EMAC index &rarr; assert
 *             - AT1/2: Invalid interface pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Function initializes interface correctly

 */
void testNIC_FillInterfaceDescriptor(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(NIC_FillInterfaceDescriptor(EMAC_MAX_INSTANCE, &xInterfaces[0]));
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(NIC_FillInterfaceDescriptor(0, NULL_PTR));

    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    FreeRTOS_AddNetworkInterface_ExpectAndReturn(&xInterfaces[0], &xInterfaces[0]);
    /* ======= RT1/1: Call function under test */
    NetworkInterface_t *result = NIC_FillInterfaceDescriptor(0, &xInterfaces[0]);
    TEST_ASSERT_EQUAL(&xInterfaces[0], result);
    TEST_ASSERT_EQUAL_STRING("TMS570", xInterfaces[0].pcName);
    TEST_ASSERT_EQUAL_PTR(pNetworkInterfaceInitialise, xInterfaces[0].pfInitialise);
    TEST_ASSERT_EQUAL_PTR(pNetworkInterfaceOutput, xInterfaces[0].pfOutput);
    TEST_ASSERT_EQUAL_PTR(xGetPhyLinkStatus, xInterfaces[0].pfGetPhyLinkStatus);
}

/**
 * @brief   Testing external function #NIC_Receive
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - none
 *          - Routine validation:
 *            - RT1/2: No Message received
 *            - RT2/2: Message received
 *
 */
void testNIC_Receive(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    volatile EMAC_RX_BUFFER_DESCRIPTOR_s *pNextBufferDescriptorToProcess = &testBufferDescriptor;
    /* ======= RT1/2: Test implementation */
    FOREVER_ExpectAndReturn(1u);
    OS_NotifyTake_ExpectAndReturn(false, (TickType_t)1000u, 0u);
    DATA_Read1DataBlock_ExpectAndReturn(&testTablePhy, STD_OK);
    DATA_Read1DataBlock_ReturnThruPtr_pDataToReceiver0(&testTablePhy);
    EMAC_GetPhyLinkStatus_ExpectAndReturn(STD_OK);
    FOREVER_ExpectAndReturn(0u);
    /* ======= RT1/2: Call function under test */
    NIC_Receive();

    /* ======= RT2/2: Test implementation */
    FOREVER_ExpectAndReturn(1u);
    OS_NotifyTake_ExpectAndReturn(false, (TickType_t)1000u, 1u);

    EMAC_GetRxActiveHead_ExpectAndReturn(pNextBufferDescriptorToProcess);
    EMAC_StartOfPacket_ExpectAndReturn(pNextBufferDescriptorToProcess, true);

    /* NIC_ProcessPacket*/
    EMAC_GetRxActiveHead_ExpectAndReturn(pNextBufferDescriptorToProcess);
    EMAC_GetRxActiveTail_ExpectAndReturn(pNextBufferDescriptorToProcess);
    EMAC_EmacIsOwner_ExpectAndReturn(pNextBufferDescriptorToProcess, false);

    /*NIC_TransferToTcp();*/
    EMAC_ReceivedDataLengthInBytes_ExpectAndReturn(72);
    pxGetNetworkBufferWithDescriptor_ExpectAndReturn(72, 0, &xNetworkBuffers[0]);
    EMAC_GetRxActiveHead_ExpectAndReturn(pNextBufferDescriptorToProcess);
    EMAC_GetEthernetBuffer_ExpectAndReturn(pNextBufferDescriptorToProcess, &(emac_rxBuffers[0][0]));
    FreeRTOS_MatchingEndpoint_ExpectAndReturn(&xInterfaces[0], &(emac_rxBuffers[0][0]), &endpoint);

    EMAC_UpdateLinkedList_Expect(pNextBufferDescriptorToProcess);

    /* Acknowledge that this packet is processed */
    EMAC_AcknowledgePacket_Expect(pNextBufferDescriptorToProcess);
    EMAC_SetNextPointerOfCurrentTail_Expect(pNextBufferDescriptorToProcess);
    EMAC_EndOfReception_Expect(pNextBufferDescriptorToProcess);
    EMAC_AcknowledgeRxInterrupt_Expect();

    EMAC_StartOfPacket_ExpectAndReturn(pNextBufferDescriptorToProcess, false);
    FOREVER_ExpectAndReturn(0u);

    /* ======= RT2/2: Call function under test */
    NIC_Receive();
}
