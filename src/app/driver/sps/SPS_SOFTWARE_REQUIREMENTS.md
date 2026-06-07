# SPS — 智能功率开关驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 SPS（Smart Power Switch，智能功率开关）驱动模块的软件需求。SPS 模块通过 SPI 接口控制外部智能功率开关芯片，实现接触器驱动和通用 I/O 输出控制，并提供电流反馈检测功能。

### 1.2 范围
- **涵盖**：SPS 初始化、通道状态请求（接触器/通用IO）、反馈检测（电流/PEX）
- **不涵盖**：SPI 底层通信、SPS 芯片硬件设计

## 2. 功能需求

### 2.1 通道控制

#### REQ-001 — SPS 初始化
**优先级**：高 | **函数**：`SPS_Initialize()`

模块应初始化所有 SPS 通道的 I/O 引脚（方向、初始状态）。

#### REQ-002 — 周期性控制（10ms）
**优先级**：高 | **函数**：`SPS_Ctrl()`

模块应每 10ms 执行 SPS 状态机，含重入保护。

#### REQ-003 — 接触器通道状态请求
**优先级**：高 | **函数**：`SPS_RequestContactorState()`

模块应将接触器通道的状态请求（开/关）发送到 SPS 芯片。请求前校验通道归属为 `SPS_AFF_CONTACTOR`。

#### REQ-004 — 通用 IO 通道状态请求
**优先级**：中 | **函数**：`SPS_RequestGeneralIoState()`

模块应将通用 IO 通道的状态请求发送到 SPS 芯片。请求前校验通道归属为 `SPS_AFF_GENERAL_IO`。

### 2.2 反馈检测

#### REQ-005 — 电流反馈检测
**优先级**：高 | **函数**：`SPS_GetChannelCurrentFeedback()`

模块应通过比较通道电流与阈值来判断接触器是否已闭合。

#### REQ-006 — PEX 反馈检测
**优先级**：中 | **函数**：`SPS_GetChannelPexFeedback()`

模块应通过 PEX（端口扩展器）读取通道的辅助反馈信号。

### 2.3 辅助功能

#### REQ-007 — 通道归属查询
**优先级**：低 | **函数**：`SPS_GetChannelAffiliation()`

#### REQ-008 — 全量通用 IO 关断
**优先级**：中 | **函数**：`SPS_SwitchOffAllGeneralIoChannels()`

模块应关闭所有配置为通用 IO 的 SPS 通道。

## 3. 接口需求

| 函数 | 说明 |
|------|------|
| `SPS_Initialize()` | 初始化 |
| `SPS_Ctrl()` | 10ms 周期性控制 |
| `SPS_RequestContactorState(ch, fn)` | 接触器状态请求 |
| `SPS_RequestGeneralIoState(ch, fn)` | GPIO 状态请求 |
| `SPS_GetChannelCurrentFeedback(ch)` | 电流反馈 |
| `SPS_GetChannelPexFeedback(ch, no)` | PEX 反馈 |
| `SPS_GetChannelAffiliation(ch)` | 通道归属 |
| `SPS_SwitchOffAllGeneralIoChannels()` | 全量关断 |

## 4. 依赖项
`contactor_cfg.h`, `sps_cfg.h`, `sps_types.h`, PEX 模块
