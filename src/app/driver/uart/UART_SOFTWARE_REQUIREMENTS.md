# UART — 串行通信驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 UART（RS232）驱动模块的软件需求。UART 模块负责通过 SCI 接口和 DMA 实现 RS232 串行通信，支持格式化输出和软件流控制。

### 1.2 范围
- **涵盖**：UART 初始化、数据收发、格式化输出、XON/XOFF 软件流控制
- **不涵盖**：SCI 硬件初始化（HAL 层负责）

## 2. 功能需求

#### REQ-001 — UART 初始化
**优先级**：高 | **函数**：`UART_Initialize()`

#### REQ-002 — UART 数据读取
**优先级**：高 | **函数**：`UART_Read()`

模块应从 UART RX 队列中读取指定数量的字节。注意：读取的数据不以 '\0' 结尾。

#### REQ-003 — 软件流控制
**优先级**：中 | **函数**：`UART_HandleFlowControl()`

模块应通过 XON（`0x11`）/ XOFF（`0x13`）字符实现软件流控制，由 UART 任务在收到通知时调用。

#### REQ-004 — 格式化输出
**优先级**：高 | **函数**：`UART_Printf()`

模块应提供类似 printf 的格式化字符串输出功能，发送前自动获取 `uart_txSemaphore` 信号量。

#### REQ-005 — 发送互斥
模块应通过 `uart_txSemaphore` 二元信号量保护 UART 发送访问，防止多任务并发发送冲突。

## 3. 接口需求
| 函数 | 说明 |
|------|------|
| `UART_Initialize()` | 初始化 UART 硬件 |
| `UART_Read(buf, n)` | 读取数据 |
| `UART_HandleFlowControl()` | 流控制处理 |
| `UART_Printf(fmt, ...)` | 格式化输出 |
