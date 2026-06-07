# EMAC — 以太网 MAC 驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 EMAC（Ethernet MAC）驱动模块的软件需求。EMAC 模块负责管理 TMS570LC43 的以太网 MAC 控制器，处理以太网数据包的收发、缓冲描述符链管理和 PHY 交互。

### 1.2 范围
- **涵盖**：EMAC 硬件/DMA 初始化、数据包收发、缓冲描述符管理、PHY 链路状态检测、中断处理
- **不涵盖**：TCP/IP 协议栈（FreeRTOS+TCP）、PHY 底层寄存器操作

## 2. 功能需求

### 2.1 初始化

#### REQ-001 — EMAC 硬件初始化
**优先级**：高 | **函数**：`EMAC_InitializeHardware()`

模块应初始化 EMAC 控制器、MDIO 接口和 PHY 芯片，配置 MAC 地址。

#### REQ-002 — EMAC DMA 初始化
**优先级**：中 | **函数**：`EMAC_InitializeDma()`

当启用 TCP 支持时，模块应将 EMAC 接收缓冲区与 TCP 协议栈的网络缓冲区描述符连接。

#### REQ-003 — TX 缓冲描述符初始化
**优先级**：高 | **函数**：`EMAC_InitializeTxBufferDescriptors()`

### 2.2 数据包收发

#### REQ-004 — 数据包发送
**优先级**：高 | **函数**：`EMAC_Transmit()`

模块应通过 DMA 描述符链发送以太网数据包，支持分片传输（多个 buffer 组成一个包）。

#### REQ-005 — TX 中断处理
**优先级**：高 | **函数**：`EMAC_TxInterruptHandler()`, `emacTxNotification()`

模块应在 TX 中断中处理已发送的描述符，并通知上层。

#### REQ-006 — RX 通知处理
**优先级**：高 | **函数**：`emacRxNotification()`

模块应将收到的数据包传递给上层协议栈。

### 2.3 缓冲描述符管理

#### REQ-007 — 缓冲描述符状态查询
**优先级**：高 | **函数**：`EMAC_EmacIsOwner()`, `EMAC_StartOfPacket()`

#### REQ-008 — 链表更新
**优先级**：高 | **函数**：`EMAC_UpdateLinkedList()`, `EMAC_SetNextPointerOfCurrentTail()`

模块应维护接收缓冲描述符的链表结构。

#### REQ-009 — 数据包确认
**优先级**：高 | **函数**：`EMAC_AcknowledgePacket()`, `EMAC_EndOfReception()`

处理完数据包后应确认并归还缓冲描述符。

### 2.4 PHY 交互

#### REQ-010 — PHY 链路状态
**优先级**：高 | **函数**：`EMAC_GetPhyLinkStatus()`

## 3. 数据结构
- `EMAC_RX_BUFFER_DESCRIPTOR_s` — RX 缓冲描述符（next 指针、pBuffer、偏移/长度、标志/包长）
- `EMAC_RX_CHANNEL_s` / `EMAC_TX_CHANNEL_s` — 收发通道管理
- `EMAC_HDKIF_s` — 以太网接口私有数据（MAC 地址、基地址、PHY 参数）
- `EMAC_PACKET_BUFFER_s` — 数据包缓冲（支持分片链表）

## 4. 接口需求（18 个公开函数）
关键 API：`EMAC_InitializeHardware`, `EMAC_Transmit`, `EMAC_TxInterruptHandler`, `EMAC_GetPhyLinkStatus`, `emacRx/TxNotification`

## 5. 来源说明
基于 TI HALCoGen 生成的 EMAC 驱动修改，以适配 DP83869 PHY 和 foxBMS 编码规范。
