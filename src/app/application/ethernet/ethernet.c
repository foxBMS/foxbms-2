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
 * @file    ethernet.c
 * @author  foxBMS Team
 * @date    2025-05-30 (date of creation)
 * @updated 2026-04-20 (date of last update)
 * @version v1.11.0
 * @ingroup APPLICATION
 * @prefix  ETH
 *
 * @brief   Implementation of ethernet software
 * @details Here can the application part of the ethernet be placed.
 *          For example a TCP-sever.
 * @requirements REQ-001, REQ-002, REQ-003, REQ-004, REQ-005, REQ-006, REQ-007, REQ-008, REQ-009, REQ-010, REQ-011, REQ-012
 */

/*========== Includes =======================================================*/
#include "foxbms_config.h"

#include "ethernet.h"

#include "ethernet_cfg.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

#include "NetworkInterface.h"
#include "NetworkInterface_custom.h"
#include "ethernet_freertos.h"
#include "fassert.h"
#include "fstd_types.h"
#include "infinite-loop-helper.h"
#include "os.h"
#include "stdbool.h"
#include "utils.h"

#include <stdbool.h>

/*========== Macros and Definitions =========================================*/
#define ETH_SEED                       (53393467)  /**< REQ-001: 随机数种子值 */
#define ETH_DEFAULT_DEBUG_MESSAGE_SIZE (30u)       /**< REQ-003: 调试消息缓冲区大小 */

/** Server states
 * @req REQ-005
 */
typedef enum {
    ETH_SERVER_RUNNING,       /**< REQ-005: 正常运行状态 */
    ETH_SERVER_FIN_RECEIVED,  /**< REQ-005: 收到对端 FIN 状态 */
    ETH_SERVER_ERROR,         /**< REQ-005: 错误状态 */
} ETH_SERVER_STATE_e;

/*========== Static Constant and Variable Definitions =======================*/

/* REQ-002: 网络接口和端点描述符数组 */
NetworkInterface_t xInterfaces[1];
NetworkEndPoint_t xEndPoints[1];

/* REQ-010: Echo Socket 队列及静态存储 */
OS_QUEUE eth_echoSocketQueue                                                   = NULL_PTR;
StaticQueue_t eth_echoSocketQueueStruct                                        = {0};
uint8_t eth_echoSocketQueueStorage[ETH_ECHO_SERVER_BACKLOG * sizeof(Socket_t)] = {0};

/*========== Extern Constant and Variable Definitions =======================*/

/*========== Static Function Prototypes =====================================*/
/**
 *
 * @brief       Handles the echo server connection instance.
 * @details     If a connection to the echo server is established, the connection is handled by this task.
 *              If the connection gets closed the task deletes itself.
 * @req         REQ-003, REQ-004, REQ-005, REQ-006, REQ-007
 * @param[in]   pParameters task parameters
 */
static void ETH_EchoServerInstance(void *pParameters);
/**
 *
 * @brief       Listens for a TCP connection.
 * @details     Initializes a listening socket and waits for a connection.
 *              When a connection is established, a task for the #ETH_EchoServerInstance is created.
 * @req         REQ-008
 * @param[in]   pParameters task parameters
 */
static void ETH_ListenForConnection(void *pParameters);
#if (ipconfigUSE_TCP_WIN == 1)
/**
 *
 * @brief       Transfers the configuration for sliding windows.
 * @details     Fill in the buffer and window sizes that will be used by the socket.
 * @req         REQ-009
 * @param[in]   socket tSocket to configure
 */
static void ETH_ConfigureSlidingWindow(Socket_t socket);
#endif /* ipconfigUSE_TCP_WIN */

/*========== Static Function Implementations ================================*/
static void ETH_EchoServerInstance(void *pParameters) {
    FAS_ASSERT(pParameters == NULL_PTR);
    int32_t receivedBytes  = 0;
    int32_t sentBytes      = 0;
    int32_t totalSentBytes = 0;

    /* Run task continuously, but wait for a notification to create a new connection instance */
    while (FOREVER()) {

        /* REQ-003: 从队列获取已连接的 Socket */
        Socket_t connectedSocket = NULL_PTR;
        if (OS_ReceiveFromQueue(eth_echoSocketQueue, &connectedSocket, portMAX_DELAY) != OS_SUCCESS) {
            /* Failed to receive from queue, continue to next iteration */
            continue;
        }
        FAS_ASSERT(connectedSocket != NULL_PTR);

        /* Reset counters for each new connection */
        receivedBytes  = 0;
        sentBytes      = 0;
        totalSentBytes = 0;

        /* Send message to indicate Task has started */
        char buffer[ETH_DEFAULT_DEBUG_MESSAGE_SIZE] = "Started connection instance \n";
        sentBytes = FreeRTOS_send(connectedSocket, buffer, ETH_DEFAULT_DEBUG_MESSAGE_SIZE, 0);

        /* Attempt to create the buffer used to receive the string to be echoed
         * back.  This could be avoided using a zero copy interface that just returned
         * the same buffer. */
        uint8_t rxBuffer[ipconfigTCP_MSS] = {0};

        /* REQ-003: 配置 Socket 发送和接收超时 */
        static const TickType_t xReceiveTimeOut = ETH_ECHO_SERVER_RECEIVE_TIMEOUT;
        static const TickType_t xSendTimeOut    = ETH_ECHO_SERVER_SEND_TIMEOUT;
        BaseType_t optionSet =
            FreeRTOS_setsockopt(connectedSocket, 0, FREERTOS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof(xReceiveTimeOut));
        FAS_ASSERT(optionSet == pdFREERTOS_ERRNO_NONE);
        optionSet = FreeRTOS_setsockopt(connectedSocket, 0, FREERTOS_SO_SNDTIMEO, &xSendTimeOut, sizeof(xSendTimeOut));
        FAS_ASSERT(optionSet == pdFREERTOS_ERRNO_NONE);

        ETH_SERVER_STATE_e serverState = ETH_SERVER_RUNNING;
        while (serverState == ETH_SERVER_RUNNING) {
            /* Zero out the receive array so there is NULL at the end of the string
             * when it is printed out. */
            memset(&rxBuffer[0], 0x00, ipconfigTCP_MSS);

            /* Receive data on the socket. */
            receivedBytes = FreeRTOS_recv(connectedSocket, rxBuffer, ipconfigTCP_MSS, 0);

            /* REQ-004: 若接收到数据，原样回传（Echo） */
            if (receivedBytes > 0) {
                /* Reset sent bytes for each loop */
                sentBytes      = 0;
                totalSentBytes = 0;

                /* Call send() until all the data has been sent. */
                while ((sentBytes >= 0) && (totalSentBytes < receivedBytes)) {
                    sentBytes = FreeRTOS_send(
                        connectedSocket, &rxBuffer[0] + totalSentBytes, receivedBytes - totalSentBytes, 0);
                    if (sentBytes >= 0) {
                        totalSentBytes += sentBytes;
                        FreeRTOS_debug_printf(("Send data: %s\n", &rxBuffer[0]));
                    } else {
                        FreeRTOS_debug_printf(("FreeRTOS_send() error: %ld\n", (long)sentBytes));
                    }
                }

                if (sentBytes < 0) {
                    /* Error sending data. Socket closed? */
                    serverState = ETH_SERVER_ERROR;
                }
            } else if (receivedBytes == 0) {
                /* REQ-005: 对端关闭连接（FIN received），转入 FIN_RECEIVED 状态 */
                FreeRTOS_debug_printf(("FreeRTOS_recv() returned 0 (orderly shutdown)\n"));
                serverState = ETH_SERVER_FIN_RECEIVED;

            } else { /* receivedBytes < 0 */
                /* REQ-005: Socket 错误，转入 ERROR 状态 */
                FreeRTOS_debug_printf(("FreeRTOS_recv() error: %ld\n", (long)receivedBytes));
                serverState = ETH_SERVER_ERROR;
            }
        }

        BaseType_t success = pdFALSE;
        if (serverState == ETH_SERVER_FIN_RECEIVED) {
            /* REQ-006: 发起双向关闭（FREERTOS_SHUT_RDWR） */
            success = FreeRTOS_shutdown(connectedSocket, FREERTOS_SHUT_RDWR);
            if (success < 0) {
                FreeRTOS_debug_printf(("FreeRTOS_shutdown() error: %ld\n", success));
            }

            /* Wait for the shutdown to take effect, indicated by FreeRTOS_recv()
         * returning an error. */
            uint32_t timeOnShutdown = OS_GetTickCount();

            for (;;) {
                if (FreeRTOS_recv(connectedSocket, &rxBuffer[0], ipconfigTCP_MSS, 0) <= 0) {
                    break;
                }
                if ((OS_GetTickCount() - timeOnShutdown) >= ETH_ECHO_SERVER_SHUTDOWN_DELAY) {
                    FreeRTOS_debug_printf(("FreeRTOS_shutdown() timeout\n"));
                    break;
                }
            }
        }

        /* REQ-007: 释放 Socket 资源 */
        success = FreeRTOS_closesocket(connectedSocket);
        if (success != 0) {
            FreeRTOS_debug_printf(("FreeRTOS_closesocket() error: %x\n", success));
        }
    }
}

static void ETH_ListenForConnection(void *pParameters) {
    /* pParameters contains pxEndPoint */
    FAS_ASSERT(pParameters != NULL_PTR);
    struct freertos_sockaddr clientAddress = {0};

    BaseType_t success = pdFREERTOS_ERRNO_EINVAL;

    (void)pParameters; /* pxEndPoint is currently not used */

    /* REQ-008: 创建 IPv4 TCP Socket */
    Socket_t listeningSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);
    /* AXIVION Next Codeline IISB-LiteralSuffixesCheck:Content from FreeRTOS file */
    FAS_ASSERT(listeningSocket != FREERTOS_INVALID_SOCKET);

    /* Set a timeout so accept() will just wait for a connection. */
    static const TickType_t waitForConnectionTimeOut = portMAX_DELAY;
    BaseType_t optionSet                             = FreeRTOS_setsockopt(
        listeningSocket, 0, FREERTOS_SO_RCVTIMEO, &waitForConnectionTimeOut, sizeof(waitForConnectionTimeOut));
    FAS_ASSERT(optionSet == pdFREERTOS_ERRNO_NONE);

#if (ipconfigUSE_TCP_WIN == 1)
    {
        /* Sliding windows can increase throughput while minimising network
        traffic at the expense of consuming more RAM.*/
        /* Set the window and buffer sizes. */
        ETH_ConfigureSlidingWindow(listeningSocket);
    }
#endif /* ipconfigUSE_TCP_WIN */

    /* REQ-008: 绑定 Socket 到 Echo 端口并开始监听 */
    struct freertos_sockaddr bindAddress = {
        .sin_port   = FreeRTOS_htons(ETH_ECHO_SERVER_PORT_NUMBER),
        .sin_family = FREERTOS_AF_INET,
    };
    success = FreeRTOS_bind(listeningSocket, &bindAddress, sizeof(bindAddress));
    FAS_ASSERT(success == pdFREERTOS_ERRNO_NONE);
    success = FreeRTOS_listen(listeningSocket, ETH_ECHO_SERVER_BACKLOG);
    FAS_ASSERT(success == pdFREERTOS_ERRNO_NONE);

    socklen_t clientAddressSize = sizeof(clientAddress);
    while (FOREVER()) {
        /* REQ-008: 接受客户端连接 */
        Socket_t connectedSocket = FreeRTOS_accept(listeningSocket, &clientAddress, &clientAddressSize);
        /* AXIVION Next Codeline IISB-LiteralSuffixesCheck:Content from FreeRTOS file */
        FAS_ASSERT(connectedSocket != FREERTOS_INVALID_SOCKET);

        /* REQ-008: 将已连接 Socket 放入队列，供 Echo Server 处理 */
        OS_SendToBackOfQueue(eth_echoSocketQueue, &connectedSocket, 0u);
    }
}

#if (ipconfigUSE_TCP_WIN == 1)
static void ETH_ConfigureSlidingWindow(Socket_t socket) {
    FAS_ASSERT(socket != NULL_PTR);
    WinProperties_t windowProperties = {
        .lTxBufSize = ipconfigTCP_TX_BUFFER_LENGTH,
        .lTxWinSize = ETH_ECHO_SERVER_TX_WINDOW_SIZE,
        .lRxBufSize = ipconfigTCP_RX_BUFFER_LENGTH,
        .lRxWinSize = ETH_ECHO_SERVER_RX_WINDOW_SIZE,
    };

    BaseType_t optionSet =
        FreeRTOS_setsockopt(socket, 0, FREERTOS_SO_WIN_PROPERTIES, (void *)&windowProperties, sizeof(windowProperties));
    FAS_ASSERT(optionSet == pdFREERTOS_ERRNO_NONE);
}
#endif /* ipconfigUSE_TCP_WIN */

/*========== Extern Function Implementations ================================*/

extern void ETH_Initialize(void) {
    /* REQ-001: 设置伪随机数种子 */
    UTIL_SeedRandomNumber(ETH_SEED);

    /* REQ-001: 填充网络接口描述符（EMAC + PHY 驱动信息） */
    (void)NIC_FillInterfaceDescriptor(0, &(xInterfaces[0]));

    /* REQ-002: 配置网络参数（IP、掩码、网关、DNS、MAC） */
    FreeRTOS_FillEndPoint(
        &(xInterfaces[0]),
        &(xEndPoints[0]),
        eth_ipAddress,
        eth_netMask,
        eth_gatewayAddress,
        eth_dnsServerAddress,
        eth_emacAddress);

    /* REQ-001: 启动 FreeRTOS TCP/IP 协议栈 */
    BaseType_t success = FreeRTOS_IPInit_Multi();
    if (success == pdFALSE) {
        FreeRTOS_debug_printf(("FreeRTOS_IPInit_Multi() failed"));
    }
}

/* Externalized FreeRTOS Functions */
/* AXIVION Next Codeline CodingStyle-Naming.Function: Externalized function from FreeRTOS */
/* AXIVION Next Codeline CodingStyle-Naming.Parameter: Externalized function from FreeRTOS */
extern uint32_t ulApplicationGetNextSequenceNumber(
    uint32_t ulSourceAddress,
    uint16_t usSourcePort,
    uint32_t ulDestinationAddress,
    uint16_t usDestinationPort) {
    /* AXIVION Routine Generic-MissingParameterAssert: ulSourceAddress: parameter accepts whole range */
    /* AXIVION Routine Generic-MissingParameterAssert: usSourcePort: parameter accepts whole range */
    /* AXIVION Routine Generic-MissingParameterAssert: ulDestinationAddress: parameter accepts whole range */
    /* AXIVION Routine Generic-MissingParameterAssert: usDestinationPort: parameter accepts whole range */
    /* TODO: Implement function.
        ulApplicationGetNextSequenceNumber is an application defined hook
        (or callback) function that is called by the FreeRTOS-Plus-TCP stack to
        get a difficult to predict sequence number for the 4-value address
        tuple for a TCP connection.*/
    (void)ulSourceAddress;
    (void)usSourcePort;
    (void)ulDestinationAddress;
    (void)usDestinationPort;

    /* REQ-011: 返回伪随机数作为 TCP 序列号 */
    return UTIL_GetPseudoRandomNumber();
}

/* Called by FreeRTOS+TCP when the network connects or disconnects. Disconnect
 * events are only received if implemented in the MAC driver. */
/* AXIVION Next Codeline CodingStyle-Naming.Function: Externalized function from FreeRTOS */
/* AXIVION Next Codeline CodingStyle-Naming.Parameter: Externalized function from FreeRTOS */
extern void vApplicationIPNetworkEventHook_Multi(
    eIPCallbackEvent_t eNetworkEvent,
    struct xNetworkEndPoint *pxEndPoint) {
    /* AXIVION Enable CodingStyle-Naming.Function: */
    FAS_ASSERT((eNetworkEvent == eNetworkUp) || (eNetworkEvent == eNetworkDown));
    FAS_ASSERT(pxEndPoint != NULL_PTR);

    /* REQ-010: 网络已连接，创建 Socket 队列和任务 */
    if (eNetworkEvent == eNetworkUp) {
        /* REQ-010: 创建 Socket 队列（静态内存分配） */
        eth_echoSocketQueue = xQueueCreateStatic(
            ETH_ECHO_SERVER_BACKLOG, sizeof(Socket_t), eth_echoSocketQueueStorage, &eth_echoSocketQueueStruct);
        FAS_ASSERT(eth_echoSocketQueue != NULL_PTR);
        /* REQ-010: 创建监听任务和 Echo 服务器任务 */
        if (ETH_CreateListeningTask(&ETH_ListenForConnection, pxEndPoint) != ETH_OK) {
            FreeRTOS_debug_printf(("Failed to create Listening Task\n"));
        }
        if (ETH_CreateEchoServerTask(&ETH_EchoServerInstance) != ETH_OK) {
            FreeRTOS_debug_printf(("Failed to create Echo Server Task\n"));
        }
    }
}

/* AXIVION Next Codeline CodingStyle-Naming.Function: Externalized function from FreeRTOS */
/* AXIVION Next Codeline CodingStyle-Naming.Parameter: Externalized function from FreeRTOS */
extern BaseType_t xApplicationGetRandomNumber(uint32_t *pulNumber) {
    FAS_ASSERT(pulNumber != NULL_PTR);
    /* REQ-011: 通过指针参数返回伪随机数 */
    *(pulNumber) = UTIL_GetPseudoRandomNumber();
    return pdTRUE;
}

/* xApplicationDNSQueryHook is an application defined hook (or callback)
 * function that is called by the FreeRTOS-Plus-TCP stack to check whether the
 * LLMNR or NBNS name received is the same as the one device is looking for. */
/* AXIVION Next Codeline CodingStyle-Naming.Function: Externalized function from FreeRTOS */
/* AXIVION Next Codeline CodingStyle-Naming.Parameter: Externalized function from FreeRTOS */
extern BaseType_t xApplicationDNSQueryHook_Multi(struct xNetworkEndPoint *pxEndPoint, const char *pcName) {
    FAS_ASSERT(pxEndPoint != NULL_PTR);
    FAS_ASSERT(pcName != NULL_PTR);
    /* TODO: Implement for Domain Name System resolution. */
    (void)pxEndPoint;
    (void)pcName;
    return pdTRUE;
}

/*========== Externalized Static Function Implementations (Unit Test) =======*/
#ifdef UNITY_UNIT_TEST
void TEST_ETH_EchoServerInstance(void *pParameters) {
    ETH_EchoServerInstance(pParameters);
}
/* function pointer for the actual static function used in task creation */
void (*ETH_EchoServerInstance_Ptr)(void *pParameters) = ETH_EchoServerInstance;
void TEST_ETH_ListenForConnection(void *pParameters) {
    ETH_ListenForConnection(pParameters);
}
/* function pointer for the actual static function used in task creation */
void (*ETH_ListenForConnection_Ptr)(void *pParameters) = ETH_ListenForConnection;
void TEST_ETH_ConfigureSlidingWindow(Socket_t socket) {
    ETH_ConfigureSlidingWindow(socket);
}
#endif
