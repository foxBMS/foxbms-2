# ETHERNET — 软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 以太网（Ethernet）模块的软件需求规格。该模块基于 FreeRTOS+TCP 协议栈实现 TCP Echo 服务器功能，用于 BMS 系统的网络通信。

### 1.2 范围

- **涵盖**：以太网初始化、TCP Echo 服务器、TCP 连接监听、Socket 生命周期管理、网络事件处理、伪随机数生成
- **不涵盖**：EMAC/PHY 驱动层实现、FreeRTOS+TCP 协议栈内部实现、TLS/SSL 加密通信、HTTP/HTTPS 服务、UDP 通信

### 1.3 定义与缩略语

| 术语 | 说明 |
|------|------|
| EMAC | Ethernet Media Access Controller（以太网介质访问控制器） |
| PHY | Physical Layer Transceiver（物理层收发器） |
| TCP | Transmission Control Protocol（传输控制协议） |
| Echo Server | 回显服务器，将接收到的数据原样发回客户端 |
| FIN | TCP 连接结束标志 |
| MSS | Maximum Segment Size（最大分段大小） |
| DNS | Domain Name System（域名系统） |
| NIC | Network Interface Controller（网络接口控制器） |

### 1.4 参考文献

- FreeRTOS+TCP 协议栈文档
- foxBMS 2 系统架构文档
- [ethernet_cfg.h](../config/ethernet_cfg.h) — 以太网配置定义
- [ethernet_freertos.h](ethernet_freertos.h) — FreeRTOS 抽象层接口

---

## 2. 总体描述

### 2.1 产品视角

以太网模块位于 foxBMS 2 软件架构的 **APPLICATION** 层（`@ingroup APPLICATION`）。模块前缀为 `ETH`。

该模块依赖于：
- FreeRTOS+TCP 协议栈（`FreeRTOS_IP.h`、`FreeRTOS_Sockets.h`）
- 网络接口驱动层（`NetworkInterface.h`）
- FreeRTOS 任务抽象层（`ethernet_freertos.h`）
- BMS 配置模块（`ethernet_cfg.h`）
- 操作系统抽象层（`os.h`）

### 2.2 工作模式

模块采用 **事件驱动 + 任务并行** 的工作模式：

1. **初始化阶段**：随机数种子初始化 → 网络接口描述符填充 → 网络参数配置 → TCP/IP 协议栈启动
2. **运行阶段**：网络连接事件触发 → 创建监听任务 + Echo 服务器任务 → 等待客户端连接 → 数据回显
3. **连接生命周期**：RUNNING（数据收发） → FIN_RECEIVED（收到关闭请求） / ERROR（发生错误） → Socket 关闭

### 2.3 用户特征

本模块由 BMS 系统内部调用，无直接面向最终用户的操作界面。系统集成者通过配置文件 `ethernet_cfg.h/.c` 设定网络参数。

---

## 3. 功能需求

### 3.1 模块初始化

#### REQ-001 — 以太网模块初始化

| 属性 | 值 |
|------|-----|
| **编号** | REQ-001 |
| **优先级** | 高 |
| **函数** | `ETH_Initialize()` |

**描述**：系统应提供以太网模块的初始化功能，依次完成随机数种子设置、网络接口描述符填充、网络参数配置和 FreeRTOS TCP/IP 协议栈启动。

**处理流程**：
1. 调用 `UTIL_SeedRandomNumber()` 设置伪随机数生成器种子（`ETH_SEED = 53393467`）
2. 调用 `NIC_FillInterfaceDescriptor()` 填充网络接口描述符（EMAC + PHY 驱动信息）
3. 调用 `FreeRTOS_FillEndPoint()` 配置网络参数（IP 地址、子网掩码、网关、DNS、MAC 地址）
4. 调用 `FreeRTOS_IPInit_Multi()` 启动 TCP/IP 协议栈
5. 若协议栈初始化失败，输出调试信息

**前置条件**：网络接口硬件已就绪

**后置条件**：TCP/IP 协议栈已启动，等待网络连接事件触发

**错误处理**：`FreeRTOS_IPInit_Multi()` 返回 `pdFALSE` 时输出调试信息

---

#### REQ-002 — 网络参数配置

| 属性 | 值 |
|------|-----|
| **编号** | REQ-002 |
| **优先级** | 高 |
| **函数** | `ETH_Initialize()` / `FreeRTOS_FillEndPoint()` |

**描述**：系统应在以太网初始化时配置以下网络参数，参数值来源于 `ethernet_cfg.c` 中定义的外部常量：

| 参数 | 配置常量 | 默认值 |
|------|---------|--------|
| IP 地址 | `eth_ipAddress` | `169.254.107.24` |
| 子网掩码 | `eth_netMask` | `255.255.0.0` |
| 网关地址 | `eth_gatewayAddress` | `169.254.107.1` |
| DNS 服务器地址 | `eth_dnsServerAddress` | `0.0.0.0` |
| MAC 地址 | `eth_emacAddress` | `00:08:EE:03:A6:6C` |

---

### 3.2 TCP Echo 服务器

#### REQ-003 — Echo 服务器实例处理

| 属性 | 值 |
|------|-----|
| **编号** | REQ-003 |
| **优先级** | 高 |
| **函数** | `ETH_EchoServerInstance()` |

**描述**：系统应实现 TCP Echo 服务器实例，从 Socket 队列获取已连接的客户端 Socket，接收客户端发送的数据并原样回传（Echo），管理连接的完整生命周期。

**处理流程**：
1. 从 `eth_echoSocketQueue` 队列阻塞等待获取已连接的 Socket
2. 向客户端发送 "Started connection instance" 确认消息
3. 创建接收缓冲区（大小为 `ipconfigTCP_MSS`）
4. 配置 Socket 的接收超时（`ETH_ECHO_SERVER_RECEIVE_TIMEOUT`）和发送超时（`ETH_ECHO_SERVER_SEND_TIMEOUT`）
5. 进入循环，执行数据收发处理
6. 连接结束后关闭 Socket

**错误处理**：
- 队列接收失败时继续下一轮迭代
- Socket 为空时触发断言（`FAS_ASSERT`）

---

#### REQ-004 — Echo 数据收发

| 属性 | 值 |
|------|-----|
| **编号** | REQ-004 |
| **优先级** | 高 |
| **函数** | `ETH_EchoServerInstance()` |

**描述**：系统应在 Echo 服务器运行状态下：
1. 使用 `FreeRTOS_recv()` 接收客户端数据
2. 若接收到的字节数 > 0，循环调用 `FreeRTOS_send()` 将所有接收的数据原样发回
3. 若 `FreeRTOS_send()` 出错（返回 < 0），转入错误状态

**数据处理规则**：
- 每轮接收前清零接收缓冲区（`memset`）
- 发送循环持续到所有已接收数据发送完毕或发送出错
- 调试打印已发送的数据内容和错误信息

---

#### REQ-005 — Echo 服务器状态机

| 属性 | 值 |
|------|-----|
| **编号** | REQ-005 |
| **优先级** | 中 |
| **函数** | `ETH_EchoServerInstance()` |

**描述**：系统应使用 `ETH_SERVER_STATE_e` 状态机管理 Echo 服务器连接生命周期。

**状态定义**：

| 状态 | 含义 | 转换条件 |
|------|------|---------|
| `ETH_SERVER_RUNNING` | 正常运行，处理数据收发 | 初始状态 |
| `ETH_SERVER_FIN_RECEIVED` | 收到对端 FIN（`FreeRTOS_recv()` 返回 0） | 对端有序关闭 |
| `ETH_SERVER_ERROR` | 发生错误（接收 < 0 或发送 < 0） | 接收/发送错误 |

---

#### REQ-006 — 连接有序关闭

| 属性 | 值 |
|------|-----|
| **编号** | REQ-006 |
| **优先级** | 中 |
| **函数** | `ETH_EchoServerInstance()` |

**描述**：当 Echo 服务器收到对端 FIN（状态为 `ETH_SERVER_FIN_RECEIVED`）时，系统应：
1. 调用 `FreeRTOS_shutdown()` 发起双向关闭（`FREERTOS_SHUT_RDWR`）
2. 轮询 `FreeRTOS_recv()` 等待关闭生效（最长等待 `ETH_ECHO_SERVER_SHUTDOWN_DELAY`）
3. 超时后输出调试信息并退出等待

---

#### REQ-007 — Socket 资源释放

| 属性 | 值 |
|------|-----|
| **编号** | REQ-007 |
| **优先级** | 高 |
| **函数** | `ETH_EchoServerInstance()` |

**描述**：系统应在 Echo 服务器连接处理结束后（无论正常关闭或错误），调用 `FreeRTOS_closesocket()` 释放 Socket 资源。若关闭失败，输出调试信息。

---

### 3.3 TCP 连接监听

#### REQ-008 — TCP 连接监听

| 属性 | 值 |
|------|-----|
| **编号** | REQ-008 |
| **优先级** | 高 |
| **函数** | `ETH_ListenForConnection()` |

**描述**：系统应提供 TCP 连接监听功能，在指定端口上持续监听并接受客户端 TCP 连接。

**处理流程**：
1. 创建 IPv4 TCP Socket（`FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP)`）
2. 设置 Socket 接收超时为 `portMAX_DELAY`
3. 若启用 TCP 滑动窗口（`ipconfigUSE_TCP_WIN == 1`），调用 `ETH_ConfigureSlidingWindow()` 配置窗口大小
4. 绑定 Socket 到 `ETH_ECHO_SERVER_PORT_NUMBER` 端口
5. 开始监听，最大等待队列长度为 `ETH_ECHO_SERVER_BACKLOG`
6. 进入无限循环，调用 `FreeRTOS_accept()` 接受连接
7. 将已连接的 Socket 通过 `OS_SendToBackOfQueue()` 发送到 `eth_echoSocketQueue` 队列

**错误处理**：
- Socket 创建失败时触发断言
- `setsockopt`、`bind`、`listen`、`accept` 失败时均触发断言

---

#### REQ-009 — TCP 滑动窗口配置

| 属性 | 值 |
|------|-----|
| **编号** | REQ-009 |
| **优先级** | 低 |
| **函数** | `ETH_ConfigureSlidingWindow()` |
| **条件** | `ipconfigUSE_TCP_WIN == 1` |

**描述**：当 FreeRTOS+TCP 协议栈启用 TCP 滑动窗口功能时，系统应配置 Socket 的发送/接收窗口大小和缓冲区大小，以优化网络吞吐量。

**配置参数**：

| 参数 | 配置值来源 |
|------|-----------|
| 发送缓冲区大小 | `ipconfigTCP_TX_BUFFER_LENGTH` |
| 发送窗口大小 | `ETH_ECHO_SERVER_TX_WINDOW_SIZE` |
| 接收缓冲区大小 | `ipconfigTCP_RX_BUFFER_LENGTH` |
| 接收窗口大小 | `ETH_ECHO_SERVER_RX_WINDOW_SIZE` |

---

### 3.4 网络事件处理

#### REQ-010 — 网络连接事件处理

| 属性 | 值 |
|------|-----|
| **编号** | REQ-010 |
| **优先级** | 高 |
| **函数** | `vApplicationIPNetworkEventHook_Multi()` |

**描述**：系统应在 FreeRTOS+TCP 协议栈触发网络事件时，根据事件类型执行相应操作。

**处理流程**：
- **事件类型仅包括 `eNetworkUp` 和 `eNetworkDown`**（断言的合法值）
- **`eNetworkUp`（网络已连接）**：
  1. 创建 Socket 队列（静态内存分配，容量 = `ETH_ECHO_SERVER_BACKLOG`）
  2. 调用 `ETH_CreateListeningTask()` 创建监听任务
  3. 调用 `ETH_CreateEchoServerTask()` 创建 Echo 服务器任务
  4. 创建失败时输出调试信息
- **`eNetworkDown`（网络已断开）**：当前无额外处理，仅验证参数

---

### 3.5 协议栈回调函数

#### REQ-011 — 伪随机数生成

| 属性 | 值 |
|------|-----|
| **编号** | REQ-011 |
| **优先级** | 中 |
| **函数** | `ulApplicationGetNextSequenceNumber()`, `xApplicationGetRandomNumber()` |

**描述**：
1. `ulApplicationGetNextSequenceNumber()` 应返回伪随机数作为 TCP 连接的 4 元组序列号，调用 `UTIL_GetPseudoRandomNumber()` 获取
2. `xApplicationGetRandomNumber()` 应通过指针参数返回伪随机数，用于 FreeRTOS+TCP 协议栈内部随机数需求

---

#### REQ-012 — DNS 查询钩子

| 属性 | 值 |
|------|-----|
| **编号** | REQ-012 |
| **优先级** | 低 |
| **函数** | `xApplicationDNSQueryHook_Multi()` |

**描述**：系统应提供 DNS 查询钩子函数，供 FreeRTOS+TCP 协议栈检查 LLMNR 或 NBNS 名称是否匹配设备所需名称。当前实现为占位函数（TODO），始终返回 `pdTRUE`。

---

## 4. 非功能需求

### 4.1 时序与性能

| 编号 | 需求描述 | 关联代码 |
|------|---------|---------|
| NFR-001 | Echo 服务器接收超时应可配置（默认 `portMAX_DELAY`） | `ETH_ECHO_SERVER_RECEIVE_TIMEOUT` |
| NFR-002 | Echo 服务器发送超时应可配置（默认 500ms） | `ETH_ECHO_SERVER_SEND_TIMEOUT` |
| NFR-003 | Socket 关闭等待超时应可配置（默认 5000ms） | `ETH_ECHO_SERVER_SHUTDOWN_DELAY` |
| NFR-004 | 监听任务应无限期等待客户端连接（`portMAX_DELAY`） | `ETH_ListenForConnection()` |

### 4.2 内存

| 编号 | 需求描述 | 关联代码 |
|------|---------|---------|
| NFR-005 | Socket 队列应使用静态内存分配 | `eth_echoSocketQueueStorage[]`, `eth_echoSocketQueueStruct` |
| NFR-006 | Echo 服务器接收缓冲区大小应为 `ipconfigTCP_MSS` | `rxBuffer[ipconfigTCP_MSS]` |
| NFR-007 | 网络接口和端点描述符应使用栈静态数组（各 1 个） | `xInterfaces[1]`, `xEndPoints[1]` |

### 4.3 鲁棒性

| 编号 | 需求描述 | 关联代码 |
|------|---------|---------|
| NFR-008 | 所有外部传入的指针参数必须通过 `FAS_ASSERT` 非空校验 | 各函数的 `FAS_ASSERT` 调用 |
| NFR-009 | Socket 创建失败时必须触发断言 | `listeningSocket != FREERTOS_INVALID_SOCKET` |
| NFR-010 | 网络事件类型必须为合法值（`eNetworkUp` 或 `eNetworkDown`） | `vApplicationIPNetworkEventHook_Multi()` |
| NFR-011 | Echo 服务器实例应运行在无限循环中（`FOREVER()`），持续处理连接请求 | `ETH_EchoServerInstance()` |
| NFR-012 | 监听任务应运行在无限循环中（`FOREVER()`），持续接受连接 | `ETH_ListenForConnection()` |

### 4.4 可配置性

| 编号 | 需求描述 | 关联配置文件 |
|------|---------|-------------|
| NFR-013 | 网络参数（IP、掩码、网关、DNS、MAC）应通过 `ethernet_cfg.c` 配置 | `ethernet_cfg.c` |
| NFR-014 | Echo 服务器端口号应可配置 | `ETH_ECHO_SERVER_PORT_NUMBER` |
| NFR-015 | 最大同时连接数（Backlog）应可配置 | `ETH_ECHO_SERVER_BACKLOG` |
| NFR-016 | TCP 滑动窗口功能应可通过宏 `ipconfigUSE_TCP_WIN` 条件编译 | `#if (ipconfigUSE_TCP_WIN == 1)` |

---

## 5. 接口需求

### 5.1 公共 API

| 函数 | 说明 | 来源文件 |
|------|------|---------|
| `ETH_Initialize()` | 以太网模块初始化 | `ethernet.h` |
| `ulApplicationGetNextSequenceNumber()` | 获取 TCP 序列号（FreeRTOS 回调） | `ethernet.c` |
| `vApplicationIPNetworkEventHook_Multi()` | 网络事件回调（FreeRTOS 回调） | `ethernet.c` |
| `xApplicationGetRandomNumber()` | 获取随机数（FreeRTOS 回调） | `ethernet.c` |
| `xApplicationDNSQueryHook_Multi()` | DNS 查询钩子（FreeRTOS 回调） | `ethernet.c` |

### 5.2 依赖项

| 模块 | 用途 |
|------|------|
| `FreeRTOS_IP.h` / `FreeRTOS_Sockets.h` | TCP/IP 协议栈 API |
| `NetworkInterface.h` | 网络接口描述符填充 |
| `ethernet_freertos.h` | FreeRTOS 任务创建/删除 |
| `ethernet_cfg.h` | 网络参数和服务器配置 |
| `os.h` | OS 队列和临界区操作 |
| `fassert.h` | 断言检查 |
| `utils.h` | 随机数工具函数 |

### 5.3 调用方

- BMS 系统初始化流程（调用 `ETH_Initialize()`）
- FreeRTOS+TCP 协议栈（回调 `vApplicationIPNetworkEventHook_Multi` 等）

---

## 6. 数据结构

### 6.1 服务器状态枚举

```c
typedef enum {
    ETH_SERVER_RUNNING,       /**< 正常运行 */
    ETH_SERVER_FIN_RECEIVED,  /**< 收到对端 FIN */
    ETH_SERVER_ERROR,         /**< 错误状态 */
} ETH_SERVER_STATE_e;
```

### 6.2 全局变量

| 变量 | 类型 | 用途 |
|------|------|------|
| `xInterfaces[1]` | `NetworkInterface_t` | 网络接口描述符数组 |
| `xEndPoints[1]` | `NetworkEndPoint_t` | 网络端点描述符数组 |
| `eth_echoSocketQueue` | `OS_QUEUE` | Echo Socket 队列句柄 |
| `eth_echoSocketQueueStruct` | `StaticQueue_t` | Socket 队列静态结构体 |
| `eth_echoSocketQueueStorage[]` | `uint8_t[]` | Socket 队列存储区 |

### 6.3 配置宏（模块内部）

| 宏 | 值 | 用途 |
|----|-----|------|
| `ETH_SEED` | `53393467` | 随机数种子 |
| `ETH_DEFAULT_DEBUG_MESSAGE_SIZE` | `30u` | 调试消息缓冲区大小 |

---

## 7. 控制流

### 7.1 初始化流程

```
ETH_Initialize()
  ├── UTIL_SeedRandomNumber(ETH_SEED)
  ├── NIC_FillInterfaceDescriptor(0, &xInterfaces[0])
  ├── FreeRTOS_FillEndPoint(&xInterfaces[0], &xEndPoints[0], ...)
  │    参数: eth_ipAddress, eth_netMask, eth_gatewayAddress, eth_dnsServerAddress, eth_emacAddress
  └── FreeRTOS_IPInit_Multi()
        └── [网络就绪后回调] vApplicationIPNetworkEventHook_Multi(eNetworkUp, ...)
              ├── xQueueCreateStatic() — 创建 Socket 队列
              ├── ETH_CreateListeningTask(&ETH_ListenForConnection, pxEndPoint)
              └── ETH_CreateEchoServerTask(&ETH_EchoServerInstance)
```

### 7.2 运行时数据流

```
客户端 ──TCP──> FreeRTOS_accept() ──Socket──> OS_SendToBackOfQueue()
                                                       │
                                              eth_echoSocketQueue
                                                       │
                                           OS_ReceiveFromQueue()
                                                       │
                                          ETH_EchoServerInstance()
                                            ├── FreeRTOS_recv()
                                            ├── FreeRTOS_send()  (Echo 回传)
                                            ├── FreeRTOS_shutdown() (FIN)
                                            └── FreeRTOS_closesocket()
```

---

## 8. 追溯矩阵

| 需求编号 | 需求标题 | 源文件 | 函数/宏 |
|---------|---------|--------|---------|
| REQ-001 | 以太网模块初始化 | `ethernet.c:314` | `ETH_Initialize()` |
| REQ-002 | 网络参数配置 | `ethernet.c:323` | `FreeRTOS_FillEndPoint()` |
| REQ-003 | Echo 服务器实例处理 | `ethernet.c:128` | `ETH_EchoServerInstance()` |
| REQ-004 | Echo 数据收发 | `ethernet.c:168` | `ETH_EchoServerInstance()` |
| REQ-005 | Echo 服务器状态机 | `ethernet.c:82` | `ETH_SERVER_STATE_e` |
| REQ-006 | 连接有序关闭 | `ethernet.c:211` | `ETH_EchoServerInstance()` |
| REQ-007 | Socket 资源释放 | `ethernet.c:234` | `ETH_EchoServerInstance()` |
| REQ-008 | TCP 连接监听 | `ethernet.c:241` | `ETH_ListenForConnection()` |
| REQ-009 | TCP 滑动窗口配置 | `ethernet.c:297` | `ETH_ConfigureSlidingWindow()` |
| REQ-010 | 网络连接事件处理 | `ethernet.c:370` | `vApplicationIPNetworkEventHook_Multi()` |
| REQ-011 | 伪随机数生成 | `ethernet.c:344,395` | `ulApplicationGetNextSequenceNumber()`, `xApplicationGetRandomNumber()` |
| REQ-012 | DNS 查询钩子 | `ethernet.c:407` | `xApplicationDNSQueryHook_Multi()` |
| NFR-001 | 接收超时可配置 | `ethernet_cfg.h:94` | `ETH_ECHO_SERVER_RECEIVE_TIMEOUT` |
| NFR-002 | 发送超时可配置 | `ethernet_cfg.h:92` | `ETH_ECHO_SERVER_SEND_TIMEOUT` |
| NFR-003 | 关闭超时可配置 | `ethernet_cfg.h:96` | `ETH_ECHO_SERVER_SHUTDOWN_DELAY` |
| NFR-004 | 监听无限等待 | `ethernet.c:260` | `ETH_ListenForConnection()` |
| NFR-005 | Socket 队列静态内存 | `ethernet.c:95` | `eth_echoSocketQueueStorage[]` |
| NFR-006 | 接收缓冲区大小 | `ethernet.c:157` | `rxBuffer[ipconfigTCP_MSS]` |
| NFR-007 | 接口/端点静态内存 | `ethernet.c:91` | `xInterfaces[1]`, `xEndPoints[1]` |
| NFR-008 | 指针非空校验 | `ethernet.c` 多处 | `FAS_ASSERT` 调用 |
| NFR-009 | Socket 创建断言 | `ethernet.c:257` | `listeningSocket != FREERTOS_INVALID_SOCKET` |
| NFR-010 | 网络事件校验 | `ethernet.c:374` | `vApplicationIPNetworkEventHook_Multi()` |
| NFR-011 | Echo 服务器无限循环 | `ethernet.c:135` | `FOREVER()` |
| NFR-012 | 监听任务无限循环 | `ethernet.c:286` | `FOREVER()` |
| NFR-013 | 网络参数可配置 | `ethernet_cfg.c` | `eth_ipAddress` 等 |
| NFR-014 | 端口号可配置 | `ethernet_cfg.h:90` | `ETH_ECHO_SERVER_PORT_NUMBER` |
| NFR-015 | Backlog 可配置 | `ethernet_cfg.h:85` | `ETH_ECHO_SERVER_BACKLOG` |
| NFR-016 | 滑动窗口条件编译 | `ethernet.c:117,265,296` | `ipconfigUSE_TCP_WIN` |
