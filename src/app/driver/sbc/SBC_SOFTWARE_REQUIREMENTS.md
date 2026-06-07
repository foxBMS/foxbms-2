# SBC — 系统基础芯片驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 SBC（System Basis Chip，系统基础芯片）驱动模块的软件需求。SBC 模块管理 NXP FS85xx 系列系统基础芯片，负责 MCU 电源监控、窗口看门狗管理和安全状态控制。

### 1.2 范围
- **涵盖**：SBC 状态机、FS85xx 初始化、看门狗触发、故障错误计数管理、电压监控
- **不涵盖**：FS85xx SPI 通信底层实现

### 1.3 定义
| 术语 | 说明 |
|------|------|
| SBC | System Basis Chip |
| FS85xx | NXP 安全系统基础芯片系列 |
| 窗口看门狗 | Window Watchdog，必须在特定时间窗口内触发 |

## 2. 功能需求

### 2.1 状态机管理

#### REQ-001 — 状态请求
**优先级**：高 | **函数**：`SBC_SetStateRequest()`

模块应接受 SBC 状态请求（初始化、错误），校验合法性并返回结果。

#### REQ-002 — 状态获取
**优先级**：高 | **函数**：`SBC_GetState()`

#### REQ-003 — 周期性触发（10ms）
**优先级**：高 | **函数**：`SBC_Trigger()`

模块应每 10ms 被调用，驱动 SBC 状态机序列。包含重入保护（`triggerEntry` 计数器）。

### 2.2 初始化流程

状态机初始化子状态序列：
- `SBC_ENTRY` → `SBC_INIT_RESET_FAULT_ERROR_COUNTER_PART1` → `PART2` → `SBC_INITIALIZE_SAFETY_PATH_CHECK` → `SBC_INITIALIZE_VOLTAGE_SUPERVISOR_PART3` → `PART4`

### 2.3 看门狗管理

#### REQ-004 — 窗口看门狗触发
**优先级**：高 | **宏定义**：`SBC_WINDOW_WATCHDOG_PERIOD_MS` = 100ms

模块应在初始化期间按要求触发看门狗，并在运行模式下维持周期性看门狗服务。

#### REQ-005 — 初始化看门狗计数
模块应在初始化期间通过 `requestWatchdogTrigger` 计数正确触发一定次数的看门狗以完成 SBC 初始化。

### 2.4 状态定义

**主状态**：`UNINITIALIZED` → `INITIALIZATION` → `RUNNING` / `ERROR`

**返回类型**：`SBC_OK`, `SBC_BUSY_OK`, `SBC_REQUEST_PENDING`, `SBC_ILLEGAL_REQUEST`, `SBC_ALREADY_INITIALIZED`

### 2.5 安全特性

- **REQ-006**：点火信号检测，用于控制 SBC 下电流程
- **REQ-007**：非法请求计数（`illegalRequestsCounter`）
- **REQ-008**：初始化重试机制（`retryCounter`）

## 3. 数据结构

`SBC_STATE_s`：包含定时器、看门狗触发计时、状态/子状态/历史状态、请求、计数器、FS85xx 实例指针、点火配置。公开实例 `sbc_stateMcuSupervisor`。

## 4. 接口需求

| 函数 | 说明 |
|------|------|
| `SBC_SetStateRequest(inst, req)` | 状态请求 |
| `SBC_GetState(inst)` | 获取状态 |
| `SBC_Trigger(inst)` | 10ms 周期性触发 |

## 5. 定时约束
- 任务周期：10ms（`SBC_STATEMACHINE_TASK_CYCLE_CONTEXT_MS = 10`）
- 短时：1 个周期
- 中时：5 个周期
- 长时：10 个周期
- 看门狗周期：100ms
