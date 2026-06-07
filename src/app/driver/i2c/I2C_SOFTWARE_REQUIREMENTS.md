# I2C — I²C 总线驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 I2C（Inter-Integrated Circuit）驱动模块的软件需求。I2C 模块负责管理 MCU 的 I2C 外设，提供阻塞式和 DMA 式的读写接口，用于与 I2C 从设备（如温湿度传感器）通信。

### 1.2 范围
- **涵盖**：I2C 初始化、阻塞式读写、DMA 式读写、组合写读操作、总线状态检测、超时处理
- **不涵盖**：I2C 引脚配置（HAL 层负责）、I2C 从设备协议

### 1.3 定义与缩略语
| 术语 | 说明 |
|------|------|
| I2C / I²C | Inter-Integrated Circuit，双线串行总线 |
| NACK | Not Acknowledge，从设备无应答 |
| STOP Condition | I2C 停止条件，标记传输结束 |
| DMA | 直接存储器访问 |

### 1.4 参考文献
- Technical Reference Manual SPNU563A, March 2018, p.1769 eq.65

## 2. 总体描述

I2C 模块位于 DRIVERS 层，通过 HAL 层 I2C 驱动操作 I2C1/I2C2 外设。支持阻塞式和 DMA 式传输，DMA 模式通过 FreeRTOS 任务通知实现异步等待。

## 3. 功能需求

### 3.1 I2C 总线操作

#### REQ-001 — I2C 硬件初始化
**优先级**：高 | **函数**：`I2C_Initialize()`

#### REQ-002 — 阻塞式 I2C 读
**优先级**：高 | **函数**：`I2C_Read()`

模块应通过轮询模式从 I2C 从设备读取数据，支持 NACK 和超时检测，超时时间为 `I2C_TIMEOUT_us`（1000μs）。

#### REQ-003 — 阻塞式 I2C 写
**优先级**：高 | **函数**：`I2C_Write()`

模块应通过轮询模式向 I2C 从设备写入数据，等待 TX 缓冲区空闲后逐字节发送。

#### REQ-004 — 阻塞式 I2C 写读
**优先级**：高 | **函数**：`I2C_WriteRead()`

模块应先写入从设备地址（如有），再切换为接收模式读取数据。

### 3.2 I2C DMA 操作

#### REQ-005 — DMA 式 I2C 读
**优先级**：高 | **函数**：`I2C_ReadDma()`

模块应通过 DMA 从 I2C 从设备读取数据（nrBytes > 1）。配置 DMA 目标地址后使能 I2C RX DMA，等待 FreeRTOS 通知完成。

#### REQ-006 — DMA 式 I2C 写
**优先级**：高 | **函数**：`I2C_WriteDma()`

模块应通过 DMA 向 I2C 从设备写入数据，等待 FreeRTOS 通知完成。

#### REQ-007 — DMA 式 I2C 写读
**优先级**：高 | **函数**：`I2C_WriteReadDma()`

模块应通过 DMA 先写后读，TX 完成后切换方向并配置 RX DMA。

#### REQ-008 — DMA 最后一字节读取
**优先级**：高 | **函数**：`I2C_ReadLastRxByte()`

由于 I2C DMA 接收最后字节由 LFS 中断处理，模块应提供读取最后接收字节的接口。

### 3.3 总线状态与超时

#### REQ-009 — 总线忙检测
**优先级**：高

所有传输操作前应检测 I2C 总线是否忙（`I2C_BUSBUSY`），忙时返回 `STD_NOT_OK`。

#### REQ-010 — NACK 检测
**优先级**：高

传输过程中检测到 NACK 时应立即设置 STOP 条件并返回 `STD_NOT_OK`。

#### REQ-011 — STOP 等待超时
**优先级**：高 | **函数**：`I2C_WaitStop()`

每次传输后应等待 STOP 条件被检测到，超时后强制发送 STOP。

### 3.4 配置参数

#### REQ-012 — I2C 超时与通知配置
**优先级**：中 | **宏定义**

模块应定义：`I2C_TIMEOUT_us`=1000, `I2C_NOTIFICATION_TIMEOUT_ms`=2, `I2C_TX_TIME_MARGIN_us`=5。

## 4. 非功能需求

- **REQ-013**：DMA 配置期间进入任务临界区和特权模式，配置完成后恢复
- **REQ-014**：所有公开函数对输入指针进行非空断言检查
- **REQ-015**：单元测试暴露所有静态函数供测试

## 5. 接口需求

### 5.1 公共 API（8 个公开函数）
| 函数 | 说明 |
|------|------|
| `I2C_Initialize()` | 初始化 I2C 硬件 |
| `I2C_Read/Write/WriteRead()` | 阻塞式读写 |
| `I2C_ReadDma/WriteDma/WriteReadDma()` | DMA 式读写 |
| `I2C_ReadLastRxByte()` | 读取最后接收字节 |
| `I2C_WaitReceive()` | 等待 RX 缓冲区就绪 |

### 5.2 依赖项
`HL_i2c.h`, `dma.h`, `mcu.h`, `os.h`, `fsystem.h`, `database.h`, `diag.h`

## 6. 追溯矩阵

| 需求 | 描述 | 函数 | 文件 |
|------|------|------|------|
| REQ-001 | 初始化 | `I2C_Initialize()` | i2c.c:222-224 |
| REQ-002 | 阻塞读 | `I2C_Read()` | i2c.c:226-281 |
| REQ-003 | 阻塞写 | `I2C_Write()` | i2c.c:283-333 |
| REQ-004 | 阻塞写读 | `I2C_WriteRead()` | i2c.c:335-413 |
| REQ-005 | DMA 读 | `I2C_ReadDma()` | i2c.c:415-508 |
| REQ-006 | DMA 写 | `I2C_WriteDma()` | i2c.c:510-597 |
| REQ-007 | DMA 写读 | `I2C_WriteReadDma()` | i2c.c:599-764 |
| REQ-008 | 最后字节读取 | `I2C_ReadLastRxByte()` | i2c.c:766-770 |
