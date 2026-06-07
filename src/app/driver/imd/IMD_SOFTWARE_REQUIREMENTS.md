# IMD — 绝缘监测装置驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 IMD（Insulation Monitoring Device，绝缘监测装置）驱动模块的软件需求。IMD 模块管理高压系统对地绝缘电阻的连续监测，通过状态机控制绝缘测量设备的启停和数据采集。

### 1.2 范围
- **涵盖**：IMD 状态机、绝缘测量初始化/启动/停止、绝缘电阻阈值检测、具体 IMD 设备驱动接口
- **不涵盖**：具体 IMD 设备通信协议（如 Bender IR155/ISO165C）

### 1.3 配置
| 参数 | 值 |
|------|-----|
| 调用周期 | 100ms |
| 绝缘电阻错误阈值 | 500 kΩ |
| 绝缘电阻警告阈值 | 750 kΩ |

## 2. 功能需求

### 2.1 状态机控制

#### REQ-001 — 初始化请求
**优先级**：高 | **函数**：`IMD_RequestInitialization()`

模块应接受初始化请求并启动 IMD 状态机初始化序列。

#### REQ-002 — 绝缘测量启动
**优先级**：高 | **函数**：`IMD_RequestInsulationMeasurement()`

模块应请求激活 IMD 设备开始绝缘电阻测量。

#### REQ-003 — 测量停止
**优先级**：高 | **函数**：`IMD_RequestMeasurementStop()`

模块应请求停止 IMD 设备并进入关机流程。

#### REQ-004 — 初始化状态查询
**优先级**：中 | **函数**：`IMD_GetInitializationState()`

#### REQ-005 — 周期性触发（100ms）
**优先级**：高 | **函数**：`IMD_Trigger()`

模块应每 100ms 被调用，驱动 IMD 状态机执行。

### 2.2 设备驱动接口（由具体 IMD 驱动实现）

#### REQ-006 — 初始化处理
**优先级**：高 | **函数**：`IMD_ProcessInitializationState()`

具体驱动应实现此函数：初始化所需软件模块和外设，但不启动绝缘测量。

#### REQ-007 — 使能处理
**优先级**：高 | **函数**：`IMD_ProcessEnableState()`

具体驱动应实现此函数：启用 IMD 设备启动绝缘测量。

#### REQ-008 — 运行处理
**优先级**：高 | **函数**：`IMD_ProcessRunningState()`

具体驱动应实现此函数：处理测量数据，更新数据库 `DATA_BLOCK_INSULATION_s`。

#### REQ-009 — 关机处理
**优先级**：中 | **函数**：`IMD_ProcessShutdownState()`

具体驱动应实现此函数：禁用 IMD 设备停止测量。

## 3. 状态机

**主状态**：`DUMMY` → `HAS_NEVER_RUN` → `UNINITIALIZED` → `INITIALIZATION` → `IMD_ENABLE` → `RUNNING` / `SHUTDOWN` / `ERROR`

**初始化子状态**：`ENTRY` → `INIT_0` → `INIT_1` → `INIT_EXIT`

**运行子状态**：`ENTRY` → `RUNNING_0` → `RUNNING_1` → `RUNNING_2`

## 4. 数据结构

`IMD_STATE_s`：包含状态机完整状态（当前/下一个/上一个状态和子状态）、定时器、计数器、请求类型、`IMD_INFORMATION_s`（初始化标志/设备启停）、数据库指针。

## 5. 接口需求

| 函数 | 说明 |
|------|------|
| `IMD_RequestInitialization()` | 请求初始化 |
| `IMD_RequestInsulationMeasurement()` | 请求启动测量 |
| `IMD_RequestMeasurementStop()` | 请求停止测量 |
| `IMD_GetInitializationState()` | 获取初始化状态 |
| `IMD_Trigger()` | 100ms 周期性触发 |
| `IMD_Process*State()` | 驱动回调接口（4 个） |

## 6. 具体 IMD 实现
- Bender IR155 (`bender_ir155.c/h`)
- Bender ISO165C (`bender_iso165c.c/h`)
- None (`none/none_none.c/h`)
