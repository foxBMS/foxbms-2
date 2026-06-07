# CAN — CAN 总线通信驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 CAN（Controller Area Network）驱动模块的软件需求。CAN 模块负责管理 MCU 的 CAN 控制器（TMS570LC4357 的 DCAN），实现 BMS 与外部设备（充电器、电流传感器、IMD 等）之间的 CAN 通信。

### 1.2 范围
- **涵盖**：CAN 初始化（收发器、邮箱）、报文发送/接收、周期性报文调度、CAN 信号编解码、CBS（CAN Bus Scheduler）收发处理器
- **不涵盖**：CAN 总线物理层、CAN 配置文件内容

### 1.3 配置参数
| 参数 | 值 |
|------|-----|
| 消息邮箱总数 | 64（32 TX + 32 RX）|
| 调度周期（CAN_TICK） | 10ms |
| MCU 型号 | TMS570LC4357（不支持嵌套中断） |

## 2. 模块结构

CAN 模块包含以下子组件（44 个文件）：

```
can/
├── can.c/h                          # CAN 核心驱动（初始化、收发、调度）
├── cbs/
│   ├── can_helper.c/h                # CAN 信号编解码工具
│   ├── rx/                           # CAN 接收处理器 (CBS RX)
│   │   ├── can_cbs_rx.h              #   接收分发接口
│   │   ├── can_cbs_rx_afe_*.c        #   AFE 数据接收（电压/温度）
│   │   ├── can_cbs_rx_as_*.c         #   气溶胶传感器
│   │   ├── can_cbs_rx_cs_*.c         #   电流传感器 (Isabellenhütte IVT-S / LEM CAB500)
│   │   ├── can_cbs_rx_f_*.c          #   功能请求 (BMS状态请求/调试)
│   │   └── can_cbs_rx_imd_*.c        #   IMD 数据接收
│   ├── tx-async/                     # CAN 异步发送处理器 (CBS TX Async)
│   │   └── can_cbs_tx_f_*.c          #   崩溃转储/调试信息/致命错误/IMD请求
│   └── tx-cyclic/                    # CAN 周期发送处理器 (CBS TX Cyclic)
│       ├── can_cbs_tx_cyclic.h       #   周期发送接口
│       └── can_cbs_tx_f_*.c          #   BMS状态/电池数据/温度/电压/估算值
```

## 3. 功能需求

### 3.1 CAN 核心驱动

#### REQ-001 — CAN 初始化
**优先级**：高 | **函数**：`CAN_Initialize()`

模块应初始化 CAN 收发器引脚，使能 CAN 控制器。

#### REQ-002 — CAN 数据发送
**优先级**：高 | **函数**：`CAN_DataSend()`

模块应通过指定 CAN 节点的空闲消息邮箱发送 8 字节数据。支持标准帧和扩展帧 ID。邮箱全忙时返回 `STD_NOT_OK`。

#### REQ-003 — 消息队列发送
**优先级**：高 | **函数**：`CAN_SendMessagesFromQueue()`

模块应从 TX 队列读取待发送消息并通过 CAN 总线发送。

#### REQ-004 — CAN 主函数（周期性调度）
**优先级**：高 | **函数**：`CAN_MainFunction()`

模块应执行 CAN 时序检查并触发周期性报文的发送。每 `CAN_TICK_ms`（10ms）周期执行。

#### REQ-005 — RX 缓冲区处理
**优先级**：高 | **函数**：`CAN_ReadRxBuffer()`

模块应从接收缓冲区读取 CAN 帧。由于 TMS570LC43 不支持中断嵌套，CAN 帧在 ISR 中被接收到缓冲区，由本函数（非 ISR 上下文）执行接收回调以访问数据库。

#### REQ-006 — 周期性发送使能
**优先级**：中 | **函数**：`CAN_EnablePeriodic()`

模块应在完成首轮 AFE 测量后使能周期性 CAN 发送，防止发送未初始化数据。

### 3.2 CAN 信号编解码 (can_helper)

#### REQ-007 — TX 信号数据准备
**优先级**：高 | **函数**：`CAN_TxPrepareSignalData()`

模块应对信号数据应用缩放因子（factor）、偏移量（offset）和最小值/最大值限幅。

#### REQ-008 — RX 原始信号转换
**优先级**：高 | **函数**：`CAN_RxConvertRawSignalData()`

模块应将接收到的原始信号值通过 offset 和 factor 转换为物理值。

#### REQ-009 — 信号数据→CAN 报文组装
**优先级**：高 | **函数**：`CAN_TxSetMessageDataWithSignalData()`

模块应将信号数据按指定位起始位置和位长度组装到 64 位 CAN 报文中，支持大小端模式。

#### REQ-010 — CAN 报文→信号数据提取
**优先级**：高 | **函数**：`CAN_RxGetSignalDataFromMessageData()`

模块应从 64 位 CAN 报文中按位起始位置和位长度提取信号数据。

#### REQ-011 — 64位↔8字节转换
**优先级**：高 | **函数**：`CAN_TxSetCanDataWithMessageData()`, `CAN_RxGetMessageDataFromCanData()`

模块应支持 CAN 数据的 64 位表示和 8 字节数组表示之间的双向转换。

### 3.3 CBS 接收处理器

#### REQ-012 — AFE 数据接收
模块应接收并解析来自 AFE（模拟前端）的电池电压和温度 CAN 报文。

#### REQ-013 — 电流传感器数据接收
模块应支持 Isabellenhütte IVT-S 和 LEM CAB500 电流传感器的 CAN 报文接收。

#### REQ-014 — IMD 数据接收
模块应接收 Bender ISO165C IMD 的信息和响应 CAN 报文。

#### REQ-015 — 功能请求接收
模块应接收 BMS 状态请求和调试 CAN 报文。

### 3.4 CBS 发送处理器

#### REQ-016 — 周期性状态和测量数据发送
模块应周期性发送：BMS 状态、电池包/串的电压/温度、状态估算值、限值、最小最大值。

#### REQ-017 — 异步事件报文发送
模块应支持：崩溃转储、调试信息（构建配置/硬件识别/响应）、致命错误通知、IMD 请求。

## 4. 非功能需求

- **REQ-018**：CAN 中断不嵌套，RX 回调在非 ISR 上下文中执行
- **REQ-019**：周期性报文发送周期必须是 `CAN_TICK_ms` 的整数倍
- **REQ-020**：CAN 信号通过 `CAN_SIGNAL_TYPE_s` 结构定义（bitStart, bitLength, factor, offset, min, max）

## 5. 接口需求

### 5.1 核心 API
| 函数 | 说明 |
|------|------|
| `CAN_Initialize()` | 初始化 CAN 收发器 |
| `CAN_DataSend(node, id, idType, data)` | 发送 CAN 帧 |
| `CAN_SendMessagesFromQueue()` | 处理 TX 队列 |
| `CAN_MainFunction()` | CAN 调度主函数 |
| `CAN_ReadRxBuffer()` | 处理 RX 缓冲区 |
| `CAN_EnablePeriodic(cmd)` | 使能/禁用周期发送 |

### 5.2 信号处理 API
| 函数 | 说明 |
|------|------|
| `CAN_TxPrepareSignalData()` | 准备发送信号 |
| `CAN_RxConvertRawSignalData()` | 转换接收信号 |
| `CAN_TxSetMessageDataWithSignalData()` | 信号→报文 |
| `CAN_RxGetSignalDataFromMessageData()` | 报文→信号 |

### 5.3 依赖项
- `can_cfg.h` — CAN 消息配置和数据库映射
- `HL_can.h` — CAN 硬件寄存器
- `database.h` — 数据块读写
- CBS 各 RX/TX 模块

## 6. 追溯矩阵

| 需求 | 描述 | 函数 | 文件 |
|------|------|------|------|
| REQ-001 | 初始化 | `CAN_Initialize()` | can.c |
| REQ-002 | 数据发送 | `CAN_DataSend()` | can.c |
| REQ-003 | 队列发送 | `CAN_SendMessagesFromQueue()` | can.c |
| REQ-004 | 主调度 | `CAN_MainFunction()` | can.c |
| REQ-005 | RX 缓冲 | `CAN_ReadRxBuffer()` | can.c |
| REQ-006 | 周期使能 | `CAN_EnablePeriodic()` | can.c |
| REQ-007~011 | 信号编码 | can_helper.h 函数 | can_helper.c |
| REQ-012~015 | CBS RX | CBS RX 各文件 | cbs/rx/*.c |
| REQ-016~017 | CBS TX | CBS TX 各文件 | cbs/tx-*/*.c |
