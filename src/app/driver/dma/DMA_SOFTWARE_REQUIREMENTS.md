# DMA — 直接存储器访问驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 DMA（Direct Memory Access，直接存储器访问）驱动模块的软件需求。DMA 模块负责配置和管理 MCU 的 DMA 控制器，为 SPI、I2C、UART 等外设提供无需 CPU 干预的数据传输能力。

### 1.2 范围
- **涵盖**：DMA 控制包配置（SPI/I2C/UART）、DMA 通道分配、DMA 中断处理
- **不涵盖**：DMA 硬件初始化（由 HAL 层负责）、各外设的 DMA 回调逻辑

### 1.3 定义与缩略语
| 术语 | 说明 |
|------|------|
| DMA | Direct Memory Access，直接存储器访问 |
| BTC | Block Transfer Complete，块传输完成中断 |
| LFS | Last Frame Start，最后一帧开始中断 |
| 控制包（Control Packet） | DMA 传输的配置参数集合 |

### 1.4 参考文献
- MCU DMA 模块技术参考手册

## 2. 总体描述

### 2.1 产品视角
DMA 模块位于 DRIVERS 层，通过 HAL 层 DMA 驱动配置控制包，为 SPI、I2C、UART 等外设模块提供 DMA 传输支持。

### 2.2 工作模式
DMA 以硬件触发模式（DMA_HW）工作，由外设请求触发数据传输。

## 3. 功能需求

### 3.1 DMA 初始化

#### REQ-001 — DMA 控制包配置（SPI）
**优先级**：高 | **函数**：`DMA_Initialize()`

模块应为每个 SPI 接口配置独立的 TX 和 RX DMA 控制包，包括：
- 源/目标地址、帧计数、元素计数
- 端口分配（PORTA_READ_PORTB_WRITE / PORTB_READ_PORTA_WRITE）
- 访问宽度（16 位）、传输类型（FRAME_TRANSFER）
- 地址模式（TX: ADDR_INC1→ADDR_FIXED, RX: ADDR_FIXED→ADDR_INC1）

#### REQ-002 — DMA 控制包配置（I2C）
**优先级**：高 | **函数**：`DMA_Initialize()`

模块应为 I2C1 和 I2C2 接口配置 TX/RX DMA 控制包，访问宽度为 8 位。

#### REQ-003 — DMA 控制包配置（UART）
**优先级**：中 | **函数**：`DMA_Initialize()`

当 `FOXBMS_UART_SUPPORT == 1` 时，模块应为 SCI4（UART）配置 TX DMA 控制包。

#### REQ-004 — DMA 通道分配与使能
**优先级**：高 | **函数**：`DMA_Initialize()`

模块应使用 `dmaReqAssign()` 为各外设分配 DMA 请求线，使用 `dmaSetChEnable()` 设置通道为硬件触发模式。

#### REQ-005 — DMA 中断使能
**优先级**：高 | **函数**：`DMA_Initialize()`

模块应为 SPI TX/RX、I2C TX/RX、UART TX 通道使能 BTC（块传输完成）中断，并为 I2C RX 额外使能 LFS 中断。

#### REQ-006 — NXP AFE SPI 从模式特殊处理
**优先级**：中 | **函数**：`DMA_Initialize()`

当使用 NXP AFE 驱动时，SPI4（从模式）不应使能 TX BTC 中断。

### 3.2 DMA 中断处理

#### REQ-007 — SPI TX BTC 中断处理
**优先级**：高 | **函数**：`dmaGroupANotification()`

当 SPI TX DMA 传输完成时，模块应等待 TX 缓冲区空闲后发送最后一个字节（含 CSHOLD 控制）。

#### REQ-008 — SPI RX BTC 中断处理
**优先级**：高 | **函数**：`dmaGroupANotification()`

当 SPI RX DMA 传输完成时，模块应：
- 禁用 DMA 请求
- 对于 NXP AFE 从模式 SPI：禁用 SPI、清除硬件 CS 引脚、调用 AFE_DmaCallback
- 对于主模式 SPI：设置 busy 标志为 IDLE、调用 AFE_DmaCallback（如需）

#### REQ-009 — I2C TX/RX BTC 中断处理
**优先级**：高 | **函数**：`dmaGroupANotification()`

当 I2C DMA 传输完成时，模块应禁用 I2C DMA 并发送 FreeRTOS 任务通知到 I2C 任务。

#### REQ-010 — I2C RX 超时处理
**优先级**：高 | **函数**：`dmaGroupANotification()`

I2C RX 接收时应等待最后一个字节，若超时则设置 STOP 条件并发送错误通知。

#### REQ-011 — I2C LFS 中断处理
**优先级**：中 | **函数**：`dmaGroupANotification()`

在接收最后一个字节之前，模块应发送 I2C STOP 条件以生成 NACK。

## 4. 非功能需求

- **REQ-012**：DMA 控制包使用栈分配（`g_dmaCTRL`），不占用静态内存
- **REQ-013**：中断处理函数 `dmaGroupANotification` 应在 ISR 上下文中执行，使用 `portYIELD_FROM_ISR` 进行上下文切换
- **REQ-014**：所有 DMA 通道配置应在 `dmaEnable()` 之后执行

## 5. 接口需求

### 5.1 公共 API
| 函数 | 说明 |
|------|------|
| `DMA_Initialize()` | 初始化所有 DMA 通道和中断 |
| `DMA_IsTransmitOngoing()` | 查询 SPI 传输是否进行中 |
| `DMA_SetTransmitOngoing()` | 设置 SPI 传输状态 |

### 5.2 依赖项
| 依赖模块 | 用途 |
|----------|------|
| `dma_cfg.h` | DMA 通道和请求线配置 |
| `spi.h` / `spi.c` | SPI 接口和配置 |
| `i2c.h` / `i2c.c` | I2C 接口和配置 |
| `afe_dma.h` | AFE DMA 回调函数 |

## 6. 追溯矩阵

| 需求编号 | 需求描述 | 函数 | 文件 |
|----------|----------|------|------|
| REQ-001 | SPI DMA 控制包配置 | `DMA_Initialize()` | dma.c:95-131 |
| REQ-002 | I2C DMA 控制包配置 | `DMA_Initialize()` | dma.c:134-170 |
| REQ-003 | UART DMA 控制包配置 | `DMA_Initialize()` | dma.c:174-192 |
| REQ-004 | 通道分配与使能 | `DMA_Initialize()` | dma.c:198-241 |
| REQ-005 | DMA 中断使能 | `DMA_Initialize()` | dma.c:221-253 |
| REQ-006 | NXP 从模式处理 | `DMA_Initialize()` | dma.c:208-218 |
| REQ-007 | SPI TX 中断处理 | `dmaGroupANotification()` | dma.c:331-355 |
| REQ-008 | SPI RX 中断处理 | `dmaGroupANotification()` | dma.c:358-401 |
| REQ-009 | I2C TX/RX 中断 | `dmaGroupANotification()` | dma.c:405-479 |
| REQ-010 | I2C RX 超时处理 | `dmaGroupANotification()` | dma.c:427-452 |
| REQ-011 | I2C LFS 中断 | `dmaGroupANotification()` | dma.c:493-502 |
