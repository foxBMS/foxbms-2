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
 * @file    test_ethernet.c
 * @author  foxBMS Team
 * @date    2025-07-24 (date of creation)
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
/* Normally, we would need to include the mock of 'FreeRTOS_IP.h' as it
 * declares 'FreeRTOS_IPInit_Multi', but this is not possible due to how
 * functions are declared across the FreeRTOS stack.
 * Including 'MockFreeRTOS_IP.h' leads to multiple definition errors.
 * For a detailed understanding of the problem, just try to run the unit tests
 * when adding this invalid include.
 * The work around for this is to provide a dummy implementation of
 * 'FreeRTOS_IPInit_Multi' and NOT including the mock header. */
#include "unity.h"
#include "MockFreeRTOS_Routing.h"
#include "MockFreeRTOS_Sockets.h"
#include "MockNetworkInterface.h"
#include "MockNetworkInterface_custom.h"
#include "Mockethernet_cfg.h"
#include "Mockethernet_freertos.h"
#include "Mockinfinite-loop-helper.h"
#include "Mockmpu_prototypes.h"
#include "Mockos.h"
#include "Mockutils.h"

#include "ethernet.h"
#include "fstd_types.h"
#include "test_assert_helper.h"

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("ethernet.c")
TEST_INCLUDE_PATH("../../src/app/application/ethernet")
TEST_INCLUDE_PATH("../../src/app/application/config")
TEST_INCLUDE_PATH("../../src/app/driver/phy")
TEST_INCLUDE_PATH("../../src/app/driver/emac")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/foxmath")
TEST_INCLUDE_PATH("../../src/app/driver/uart")
TEST_INCLUDE_PATH("../../src/os/freertos/freertos-plus/freertos-plus-tcp/source/include")
TEST_INCLUDE_PATH("../../src/os/freertos/freertos-plus/freertos-plus-tcp/source/portable/Compiler/CCS")
TEST_INCLUDE_PATH("../../src/os/freertos/freertos-plus/freertos-plus-tcp/source/portable/NetworkInterface/tms570lc435")

/*========== Definitions and Implementations for Unit Test ==================*/
const uint8_t eth_emacAddress[6u]      = {0x0u, 0x08u, 0xEEu, 0x03u, 0xA6u, 0x6Cu};
const uint8_t eth_ipAddress[4u]        = {169u, 254u, 107u, 24u};
const uint8_t eth_netMask[4u]          = {255u, 255u, 0u, 0u};
const uint8_t eth_gatewayAddress[4u]   = {169u, 254u, 107u, 1u};
const uint8_t eth_dnsServerAddress[4u] = {0u, 0u, 0u, 0u};

NetworkInterface_t testInterfaces[1];
NetworkEndPoint_t testEndPoints[1];

OS_QUEUE eth_testSocketQueue                              = (OS_QUEUE)0x12345678;
StaticQueue_t eth_testSocketQueueStruct                   = {0};
uint8_t eth_testSocketQueueStorage[1u * sizeof(Socket_t)] = {0};

extern OS_QUEUE eth_echoSocketQueue;

/* dummy implementation due to the mockable includes, see comment before
 * includes */
BaseType_t FreeRTOS_IPInit_Multi(void) {
    return (BaseType_t)0u;
}

Socket_t test_ListeningSocket  = (Socket_t)0x22222222;
Socket_t test_ConnectionSocket = (Socket_t)0x22242232;

/*========== Setup and Teardown =============================================*/
void setUp(void) {
    eth_echoSocketQueue = eth_testSocketQueue;
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/
/**
 * @brief   Testing static function #ETH_EchoServerInstance with received bytes
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: Invalid pParameter pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/2: Call function and received bytes
 *            - RT2/4: Call function and received bytes - send error
 *            - RT3/4: Call function and received bytes - non graceful shutdown
 *            - RT4/4: Call function and received bytes - shutdown time out
 *
 *
 */
void testETH_EchoServerInstance_ReceivedBytes(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    uint8_t testBuffer[536]     = {0};
    Socket_t testReceivedSocket = NULL_PTR;
    /* ======= RT1/2: Test implementation */
    FOREVER_ExpectAndReturn(1);
    OS_ReceiveFromQueue_ExpectAndReturn(eth_testSocketQueue, &testReceivedSocket, portMAX_DELAY, OS_SUCCESS);
    OS_ReceiveFromQueue_ReturnThruPtr_pvBuffer(&test_ConnectionSocket);

    char startingMessage[30] = "Started connection instance \n";
    FreeRTOS_send_ExpectAndReturn(test_ConnectionSocket, startingMessage, 30, 0, 30);

    /* Set timeouts for send and receive */
    static const TickType_t xReceiveTimeOut = portMAX_DELAY;
    static const TickType_t xSendTimeOut    = pdMS_TO_TICKS(500u);
    FreeRTOS_setsockopt_ExpectAndReturn(test_ConnectionSocket, 0, 0, &xReceiveTimeOut, sizeof(xReceiveTimeOut), 0);
    FreeRTOS_setsockopt_ExpectAndReturn(test_ConnectionSocket, 0, 1, &xSendTimeOut, sizeof(xSendTimeOut), 0);

    /* Received 32 Bytes */
    FreeRTOS_recv_ExpectAndReturn(test_ConnectionSocket, testBuffer, 536, 0, 32);

    /* Answer in 8 byte steps */
    FreeRTOS_send_ExpectAndReturn(test_ConnectionSocket, testBuffer, 32, 0, 8);
    FreeRTOS_send_ExpectAndReturn(test_ConnectionSocket, testBuffer + 8, 24, 0, 8);
    FreeRTOS_send_ExpectAndReturn(test_ConnectionSocket, testBuffer + 16, 16, 0, 8);
    FreeRTOS_send_ExpectAndReturn(test_ConnectionSocket, testBuffer + 24, 8, 0, 8);

    /* Received nothing more */
    FreeRTOS_recv_ExpectAndReturn(test_ConnectionSocket, testBuffer, 536, 0, 0);

    /* Shut down procedure */
    FreeRTOS_shutdown_ExpectAndReturn(test_ConnectionSocket, 2, 0);
    OS_GetTickCount_ExpectAndReturn(0);

    /* Received nothing -> shutdown complete */
    FreeRTOS_recv_ExpectAndReturn(test_ConnectionSocket, testBuffer, 536, 0, -1);

    FreeRTOS_closesocket_ExpectAndReturn(test_ConnectionSocket, 0);
    FOREVER_ExpectAndReturn(0);

    /* ======= RT1/2: Call function under test */
    TEST_ETH_EchoServerInstance(test_ConnectionSocket);

    /* ======= RT2/4: Test implementation */
    FOREVER_ExpectAndReturn(1);
    OS_ReceiveFromQueue_ExpectAndReturn(eth_testSocketQueue, &testReceivedSocket, portMAX_DELAY, OS_SUCCESS);
    OS_ReceiveFromQueue_ReturnThruPtr_pvBuffer(&test_ConnectionSocket);

    FreeRTOS_send_ExpectAndReturn(test_ConnectionSocket, startingMessage, 30, 0, 30);

    /* Set timeouts for send and receive */
    FreeRTOS_setsockopt_ExpectAndReturn(test_ConnectionSocket, 0, 0, &xReceiveTimeOut, sizeof(xReceiveTimeOut), 0);
    FreeRTOS_setsockopt_ExpectAndReturn(test_ConnectionSocket, 0, 1, &xSendTimeOut, sizeof(xSendTimeOut), 0);

    /* Received 32 Bytes */
    FreeRTOS_recv_ExpectAndReturn(test_ConnectionSocket, testBuffer, 536, 0, 32);

    /* Answer in 8 byte steps - connection lost */
    FreeRTOS_send_ExpectAndReturn(test_ConnectionSocket, testBuffer, 32, 0, -pdFREERTOS_ERRNO_ENOTCONN);

    FreeRTOS_closesocket_ExpectAndReturn(test_ConnectionSocket, 0);
    FOREVER_ExpectAndReturn(0);

    /* ======= RT2/4: Call function under test */
    TEST_ETH_EchoServerInstance(test_ConnectionSocket);

    /* ======= RT3/4: Test implementation */
    FOREVER_ExpectAndReturn(1);
    OS_ReceiveFromQueue_ExpectAndReturn(eth_testSocketQueue, &testReceivedSocket, portMAX_DELAY, OS_SUCCESS);
    OS_ReceiveFromQueue_ReturnThruPtr_pvBuffer(&test_ConnectionSocket);
    FreeRTOS_send_ExpectAndReturn(test_ConnectionSocket, startingMessage, 30, 0, 30);

    /* Set timeouts for send and receive */
    FreeRTOS_setsockopt_ExpectAndReturn(test_ConnectionSocket, 0, 0, &xReceiveTimeOut, sizeof(xReceiveTimeOut), 0);
    FreeRTOS_setsockopt_ExpectAndReturn(test_ConnectionSocket, 0, 1, &xSendTimeOut, sizeof(xSendTimeOut), 0);

    /* Received 32 Bytes */
    FreeRTOS_recv_ExpectAndReturn(test_ConnectionSocket, testBuffer, 536, 0, 32);

    /* Answer in 8 byte steps */
    FreeRTOS_send_ExpectAndReturn(test_ConnectionSocket, testBuffer, 32, 0, 8);
    FreeRTOS_send_ExpectAndReturn(test_ConnectionSocket, testBuffer + 8, 24, 0, 8);
    FreeRTOS_send_ExpectAndReturn(test_ConnectionSocket, testBuffer + 16, 16, 0, 8);
    FreeRTOS_send_ExpectAndReturn(test_ConnectionSocket, testBuffer + 24, 8, 0, 8);

    /* Received nothing more */
    FreeRTOS_recv_ExpectAndReturn(test_ConnectionSocket, testBuffer, 536, 0, -1);

    FreeRTOS_closesocket_ExpectAndReturn(test_ConnectionSocket, 0);
    FOREVER_ExpectAndReturn(0);

    /* ======= RT3/4: Call function under test */
    TEST_ETH_EchoServerInstance(test_ConnectionSocket);

    /* ======= RT4/4: Test implementation */
    FOREVER_ExpectAndReturn(1);
    OS_ReceiveFromQueue_ExpectAndReturn(eth_testSocketQueue, &testReceivedSocket, portMAX_DELAY, OS_SUCCESS);
    OS_ReceiveFromQueue_ReturnThruPtr_pvBuffer(&test_ConnectionSocket);
    FreeRTOS_send_ExpectAndReturn(test_ConnectionSocket, startingMessage, 30, 0, 30);

    /* Set timeouts for send and receive */
    FreeRTOS_setsockopt_ExpectAndReturn(test_ConnectionSocket, 0, 0, &xReceiveTimeOut, sizeof(xReceiveTimeOut), 0);
    FreeRTOS_setsockopt_ExpectAndReturn(test_ConnectionSocket, 0, 1, &xSendTimeOut, sizeof(xSendTimeOut), 0);

    /* Received 32 Bytes */
    FreeRTOS_recv_ExpectAndReturn(test_ConnectionSocket, testBuffer, 536, 0, 32);

    /* Answer in 8 byte steps */
    FreeRTOS_send_ExpectAndReturn(test_ConnectionSocket, testBuffer, 32, 0, 8);
    FreeRTOS_send_ExpectAndReturn(test_ConnectionSocket, testBuffer + 8, 24, 0, 8);
    FreeRTOS_send_ExpectAndReturn(test_ConnectionSocket, testBuffer + 16, 16, 0, 8);
    FreeRTOS_send_ExpectAndReturn(test_ConnectionSocket, testBuffer + 24, 8, 0, 8);

    /* Received nothing more */
    FreeRTOS_recv_ExpectAndReturn(test_ConnectionSocket, testBuffer, 536, 0, 0);

    /* Shut down procedure */
    FreeRTOS_shutdown_ExpectAndReturn(test_ConnectionSocket, 2, 0);
    OS_GetTickCount_ExpectAndReturn(0);

    /* Received nothing -> shutdown complete */
    FreeRTOS_recv_ExpectAndReturn(test_ConnectionSocket, testBuffer, 536, 0, 3);

    OS_GetTickCount_ExpectAndReturn(2000);
    FreeRTOS_recv_ExpectAndReturn(test_ConnectionSocket, testBuffer, 536, 0, 3);
    OS_GetTickCount_ExpectAndReturn(7000);

    FreeRTOS_closesocket_ExpectAndReturn(test_ConnectionSocket, 0);
    FOREVER_ExpectAndReturn(0);

    /* ======= RT4/4: Call function under test */
    TEST_ETH_EchoServerInstance(test_ConnectionSocket);
}
/**
 * @brief   Testing static function #ETH_EchoServerInstance and nothing received
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None. Done in previous tests
 *          - Routine validation:
 *            - RT1/2: Call function and nothing received
 *            - RT2/2: Queue time out while waiting for socket
 *
 */
void testETH_EchoServerInstance_NotReceivedBytes(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_ETH_EchoServerInstance((void *)0x12345678));
    /* ======= Routine tests =============================================== */
    uint8_t testBuffer[536]     = {0};
    Socket_t testReceivedSocket = NULL_PTR;
    /* ======= RT1/2: Test implementation */
    FOREVER_ExpectAndReturn(1);
    OS_ReceiveFromQueue_ExpectAndReturn(eth_testSocketQueue, &testReceivedSocket, portMAX_DELAY, OS_SUCCESS);
    OS_ReceiveFromQueue_ReturnThruPtr_pvBuffer(&test_ConnectionSocket);
    char buffer[30] = "Started connection instance \n";
    FreeRTOS_send_ExpectAndReturn(test_ConnectionSocket, buffer, 30, 0, 30);

    /* Set timeouts for send and receive */
    static const TickType_t xReceiveTimeOut = portMAX_DELAY;
    static const TickType_t xSendTimeOut    = pdMS_TO_TICKS(500u);
    FreeRTOS_setsockopt_ExpectAndReturn(test_ConnectionSocket, 0, 0, &xReceiveTimeOut, sizeof(xReceiveTimeOut), 0);
    FreeRTOS_setsockopt_ExpectAndReturn(test_ConnectionSocket, 0, 1, &xSendTimeOut, sizeof(xSendTimeOut), 0);

    /* Nothing received */
    FreeRTOS_recv_ExpectAndReturn(test_ConnectionSocket, testBuffer, 536, 0, 0);

    /* Shut down procedure */
    FreeRTOS_shutdown_ExpectAndReturn(test_ConnectionSocket, 2, 0);
    OS_GetTickCount_ExpectAndReturn(0);
    /* Received nothing -> shutdown complete */
    FreeRTOS_recv_ExpectAndReturn(test_ConnectionSocket, testBuffer, 536, 0, -1);

    FreeRTOS_closesocket_ExpectAndReturn(test_ConnectionSocket, 0);
    FOREVER_ExpectAndReturn(0);

    /* ======= RT1/1: Call function under test */
    TEST_ETH_EchoServerInstance(test_ConnectionSocket);

    /* ======= RT2/2: Test implementation */
    FOREVER_ExpectAndReturn(1);
    OS_ReceiveFromQueue_ExpectAndReturn(eth_testSocketQueue, &testReceivedSocket, portMAX_DELAY, OS_FAIL);
    FOREVER_ExpectAndReturn(0);
    /* ======= RT2/2: Call function under test */
    TEST_ETH_EchoServerInstance(test_ConnectionSocket);
}

/**
 * @brief   Testing static function #ETH_ListenForConnection with socket created successfully
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/1: Invalid pParameter pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Call function as expected
 *
 */
void testETH_ListenForConnection_CreatesSocketSuccessfully(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_ETH_ListenForConnection(NULL_PTR));
    /* ======= Routine tests =============================================== */
    void *pParameters = (void *)0x12345678;

    /* ======= RT1/1: Test implementation */
    FreeRTOS_socket_ExpectAndReturn(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP, test_ListeningSocket);
    struct freertos_sockaddr clientAddress           = {0};
    static const TickType_t waitForConnectionTimeOut = portMAX_DELAY;
    FreeRTOS_setsockopt_ExpectAndReturn(
        test_ListeningSocket, 0, 0, &waitForConnectionTimeOut, sizeof(waitForConnectionTimeOut), 0);

#if (ipconfigUSE_TCP_WIN == 1)
    {
        WinProperties_t winProps;
        winProps.lTxBufSize = ipconfigTCP_TX_BUFFER_LENGTH;
        winProps.lTxWinSize = 2u;
        winProps.lRxBufSize = ipconfigTCP_RX_BUFFER_LENGTH;
        winProps.lRxWinSize = 2u;
        FreeRTOS_setsockopt_ExpectAndReturn(test_ListeningSocket, 0, 13, (void *)&winProps, sizeof(winProps), 0);
    }
#endif /* ipconfigUSE_TCP_WIN */

    /* Bind */

    struct freertos_sockaddr bindAddress = {0};
    bindAddress.sin_port                 = FreeRTOS_htons(ETH_ECHO_SERVER_PORT_NUMBER);
    bindAddress.sin_family               = FREERTOS_AF_INET;
    FreeRTOS_bind_ExpectAndReturn(test_ListeningSocket, &bindAddress, sizeof(bindAddress), pdFREERTOS_ERRNO_NONE);
    FreeRTOS_listen_ExpectAndReturn(test_ListeningSocket, 1u, pdFREERTOS_ERRNO_NONE);
    FOREVER_ExpectAndReturn(1);
    /* Accept */
    socklen_t size = sizeof(clientAddress);
    FreeRTOS_accept_ExpectAndReturn(test_ListeningSocket, &clientAddress, &size, test_ConnectionSocket);

    OS_SendToBackOfQueue_ExpectAndReturn(eth_testSocketQueue, &test_ConnectionSocket, 0u, OS_SUCCESS);
    FOREVER_ExpectAndReturn(0);
    /* ======= RT1/1: Call function under test */
    TEST_ETH_ListenForConnection(pParameters);
}

/**
 * @brief   Testing static function #ETH_ListenForConnection with socket creation fails
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - None. Done in previous tests
 *          - Routine validation:
 *            - RT1/1: Call function with socket creation fails
 *
 */
void test_ETH_ListenForConnection_SocketCreationFails(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    void *pParameters = (void *)0x12345678;
    /* ======= RT1/1: Test implementation */
    FreeRTOS_socket_ExpectAndReturn(
        FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP, FREERTOS_INVALID_SOCKET);
    /* ======= RT1/1: Call function under test */
    TEST_ASSERT_FAIL_ASSERT(TEST_ETH_ListenForConnection(pParameters));
}

/**
 * @brief   Testing static function #testETH_ConfigureSlidingWindow
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - AT1/1: Invalid pParameter pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Call function as expected
 *
 */
void testETH_ConfigureSlidingWindow(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(TEST_ETH_ConfigureSlidingWindow(NULL_PTR));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    WinProperties_t winProps;
    winProps.lTxBufSize = ipconfigTCP_TX_BUFFER_LENGTH;
    winProps.lTxWinSize = 2u;
    winProps.lRxBufSize = ipconfigTCP_RX_BUFFER_LENGTH;
    winProps.lRxWinSize = 2u;
    FreeRTOS_setsockopt_ExpectAndReturn(test_ListeningSocket, 0, 13, (void *)&winProps, sizeof(winProps), 0);
    /* ======= RT1/1: Call function under test */
    TEST_ETH_ConfigureSlidingWindow(test_ListeningSocket);
}

/**
 * @brief   Testing external function #ETH_Initialize
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - none
 *          - Routine validation:
 *            - RT1/1: Call function as expected
 *
 */
void testETH_Initialize(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    UTIL_SeedRandomNumber_Expect(53393467);

    NIC_FillInterfaceDescriptor_ExpectAndReturn(0, &(testInterfaces[0]), &(testInterfaces[0]));

    FreeRTOS_FillEndPoint_Expect(
        &(testInterfaces[0]),
        &(testEndPoints[0]),
        eth_ipAddress,
        eth_netMask,
        eth_gatewayAddress,
        eth_dnsServerAddress,
        eth_emacAddress);
    /* ======= RT1/1: Call function under test */
    ETH_Initialize();
}

/**
 * @brief   Testing externalized FreeRTOS function
 *          #ulApplicationGetNextSequenceNumber
 * @details The following cases need to be tested:
 *          - Argument validation:
 *             - none
 *          - Routine validation:
 *            - RT1/1: Call function as expected
 *
 */
void testApplicationGetNextSequenceNumber(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    UTIL_GetPseudoRandomNumber_ExpectAndReturn(42u);
    /* ======= RT1/1: Call function under test */
    uint32_t sequenceNumber = ulApplicationGetNextSequenceNumber(0xDEFFDEFF, 0x1F40, 0xA9FE6B01, 0x1F40);
    TEST_ASSERT_NOT_EQUAL(53393467, sequenceNumber);
}

/**
 * @brief   Testing externalized FreeRTOS function
 *          #vApplicationIPNetworkEventHook_Multi
 * @details The following cases need to be tested:
 *          - Argument validation:
 *            - AT1/2: eIPCallbackEvent_t out of range; assert
 *            - AT2/2: Invalid hdkif pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/2: Call function with network up
 *            - RT2/2: Call function with network up
 *
 */
void testApplicationIPNetworkEventHook_Multi(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(vApplicationIPNetworkEventHook_Multi(2u, &(testEndPoints[0])));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(vApplicationIPNetworkEventHook_Multi(eNetworkUp, NULL_PTR));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/2: Test implementation */
    MPU_xQueueGenericCreateStatic_ExpectAndReturn(
        1u,
        sizeof(Socket_t),
        eth_testSocketQueueStorage,
        &eth_testSocketQueueStruct,
        queueQUEUE_TYPE_BASE,
        eth_testSocketQueue);
    ETH_CreateListeningTask_ExpectAndReturn(ETH_ListenForConnection_Ptr, &(testEndPoints[0]), 0);
    ETH_CreateEchoServerTask_ExpectAndReturn(ETH_EchoServerInstance_Ptr, 0);
    /* ======= RT1/2: Call function under test */
    vApplicationIPNetworkEventHook_Multi(eNetworkUp, &(testEndPoints[0]));

    /* ======= RT2/2: Test implementation */
    /* ======= RT2/2: Call function under test */
    vApplicationIPNetworkEventHook_Multi(eNetworkDown, &(testEndPoints[0]));
}

/**
 * @brief   Testing externalized FreeRTOS function
 *          #xApplicationGetRandomNumber
 * @details The following cases need to be tested:
 *          - Argument validation:
 *              - AT1/1: Invalid pulNumber pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Call function as expected
 */
void testApplicationGetRandomNumber(void) {
    /* ======= Assertion tests ============================================= */
    /* ======= AT1/1 ======= */
    TEST_ASSERT_FAIL_ASSERT(xApplicationGetRandomNumber(NULL_PTR));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    UTIL_GetPseudoRandomNumber_ExpectAndReturn(42u);
    /* ======= RT1/1: Call function under test */
    uint32_t randomNumber = 0u;
    xApplicationGetRandomNumber(&randomNumber);
    TEST_ASSERT_NOT_EQUAL(53393467, randomNumber);
}

/**
 * @brief   Testing externalized FreeRTOS function
 *          #xApplicationDNSQueryHook_Multi
 * @details The following cases need to be tested:
 *          - Argument validation:
 *              - AT1/2: Invalid pxEndPoint pointer &rarr; assert
 *              - AT2/2: Invalid pcName pointer &rarr; assert
 *          - Routine validation:
 *            - RT1/1: Call function as expected
 */
void testApplicationDNSQueryHook_Multi(void) {
    /* ======= Assertion tests ============================================= */
    char testName[] = "TEST";
    /* ======= AT1/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(xApplicationDNSQueryHook_Multi(NULL_PTR, testName));
    /* ======= AT2/2 ======= */
    TEST_ASSERT_FAIL_ASSERT(xApplicationDNSQueryHook_Multi(&(testEndPoints[0]), NULL_PTR));
    /* ======= Routine tests =============================================== */
    /* ======= RT1/1: Test implementation */
    /* ======= RT1/1: Call function under test */
    xApplicationDNSQueryHook_Multi(&(testEndPoints[0]), testName);
}
