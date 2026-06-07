# SPI — 串行外设接口驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 SPI（Serial Peripheral Interface）驱动模块的软件需求。SPI 模块管理 MCU 的多个 SPI 外设，提供阻塞式和 DMA 式数据传输，支持硬件/软件片选和总线锁定机制。

### 1.2 范围
- **涵盖**：SPI 初始化、阻塞传输、DMA 传输、片选管理、总线锁定/解锁
- **不涵盖**：SPI 引脚和时钟配置（HAL/cfg 层负责）

### 1.3 定义与缩略语
| 术语 | 说明 |
|------|------|
| SPI | Serial Peripheral Interface，串行外设接口 |
| CS | Chip Select，片选信号 |
| DMA | 直接存储器访问 |
| CSHOLD | 片选保持位 |

## 2. 总体描述

SPI 模块位于 DRIVERS 层，管理 MCU 的 5 个 SPI 接口。支持主/从模式，提供阻塞式和 DMA 式传输，为 AFE、FRAM 等外设提供底层 SPI 通信能力。

## 3. 功能需求

### 3.1 SPI 初始化与控制

#### REQ-001 — SPI 初始化
**优先级**：高 | **函数**：`SPI_Initialize()`

模块应调用 HAL 层初始化所有 SPI 接口，并配置各外设的片选信号。

### 3.2 SPI 数据传输

#### REQ-002 — 阻塞式 SPI 发送
**优先级**：高 | **函数**：`SPI_TransmitData()`

模块应以阻塞模式通过 SPI 发送数据，自动控制片选信号。

#### REQ-003 — 阻塞式 SPI 收发
**优先级**：高 | **函数**：`SPI_TransmitReceiveData()`

模块应以阻塞模式同时发送和接收数据，自动控制片选信号。

#### REQ-004 — FRAM 专用 SPI 收发
**优先级**：中 | **函数**：`SPI_FramTransmitReceiveData()`

模块应为 FRAM 提供不驱动片选的 SPI 收发包装函数（片选由 FRAM 驱动自行管理）。

#### REQ-005 — DMA 式 SPI 收发
**优先级**：高 | **函数**：`SPI_TransmitReceiveDataDma()`

模块应通过 DMA 进行 SPI 数据传输，适用于大量数据的高效传输。

#### REQ-006 — SPI 从模式 DMA 接收
**优先级**：中 | **函数**：`SPI_SlaveSetReceiveDataDma()`

模块应支持 SPI 从模式的 DMA 数据接收配置。

#### REQ-007 — 唤醒字节
**优先级**：中 | **函数**：`SPI_TransmitDummyByte()`

模块应发送一个空字节来唤醒或初始化目标 SPI 从设备。

### 3.3 总线管理

#### REQ-008 — SPI 总线锁定
**优先级**：高 | **函数**：`SPI_Lock()`

模块应提供 SPI 总线互斥锁定机制，防止多个任务同时访问同一 SPI 接口。

#### REQ-009 — SPI 总线解锁
**优先级**：高 | **函数**：`SPI_Unlock()`

模块应在传输完成后解锁 SPI 总线。

#### REQ-010 — 接口可用性检查
**优先级**：中 | **函数**：`SPI_CheckInterfaceAvailable()`

模块应查询指定 SPI 接口是否可用（非忙状态）。

#### REQ-011 — 引脚功能设置
**优先级**：中 | **函数**：`SPI_SetFunctional()`

模块应支持切换 SPI 引脚为硬件 SPI 控制或 GIO 控制。

#### REQ-012 — DMA 最后字节发送
**优先级**：高 | **函数**：`SPI_DmaSendLastByte()`

SPI DMA 传输时，最后一个字节应单独发送（CSHOLD=0 以释放片选）。

#### REQ-013 — SPI 接口索引查询
**优先级**：低 | **函数**：`SPI_GetSpiIndex()`

## 4. 非功能需求

- **REQ-014**：SPI 总线使用 busy flags 数组实现互斥（`spi_busyFlags[]`）
- **REQ-015**：支持硬件片选和软件片选两种模式

## 5. 接口需求

### 5.1 公共 API（11 个公开函数）
见 3.1-3.3 各需求关联函数。

### 5.2 依赖项
`spi_cfg.h`, `HL_spi.h`, `dma.h`, `io.h`, `os.h`

## 6. 追溯矩阵

| 需求 | 描述 | 函数 | 文件 |
|------|------|------|------|
| REQ-001 | 初始化 | `SPI_Initialize()` | spi.c |
| REQ-002~007 | 数据传输 | `SPI_Transmit*` | spi.c |
| REQ-008~013 | 总线管理 | `SPI_Lock/Unlock` | spi.c |
