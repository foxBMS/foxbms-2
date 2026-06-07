# SYS — 系统管理模块软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义 foxBMS 2 系统管理模块（SYS）的软件需求。SYS 模块是 BMS 系统级状态机，负责系统上电后的整体初始化序列、运行模式切换以及错误状态管理。

### 1.2 范围

- **涵盖**：系统级状态机（FSM）的状态定义与状态转移逻辑、初始化序列、预运行序列、状态请求管理、多重调用检测、启动引导消息发送、软件复位
- **不涵盖**：各子模块（BMS/BAL/SBC 等）自身的状态机实现细节

### 1.3 定义与缩略语

| 缩略语 | 说明 |
|--------|------|
| FSM | 有限状态机（Finite State Machine） |
| SYS | 系统管理模块（System Management） |
| SBC | 系统基础芯片（System Basis Chip） |
| BIST | 内置自检（Built-In Self-Test） |
| IMD | 绝缘监测设备（Insulation Monitoring Device） |
| RTC | 实时时钟（Real-Time Clock） |
| FRAM | 铁电随机存取存储器（Ferroelectric RAM） |

### 1.4 参考文献

- [sys.h](sys.h) — SYS 模块头文件
- [sys.c](sys.c) — SYS 模块实现
- [reset.c](reset.c) — 软件复位实现
- [reset.h](reset.h) — 软件复位头文件
- [sys_cfg.h](../config/sys_cfg.h) — SYS 配置头文件
- [sys_cfg.c](../config/sys_cfg.c) — SYS 配置实现

---

## 2. 总体描述

### 2.1 产品视角

SYS 模块位于 ENGINE 层，是整个 BMS 系统启动和运行的核心调度模块。它在 FreeRTOS 环境中以 10ms 周期运行（`SYS_Trigger()`），驱动系统从"未初始化"状态经过完整的初始化序列进入"运行"状态。

### 2.2 工作模式

系统状态机定义以下顶层状态：

```
UNINITIALIZED → INITIALIZATION → PRE_RUNNING → RUNNING
                     ↓                ↓
                   ERROR  ←───────────┘
```

- **UNINITIALIZED**：等待初始化请求
- **INITIALIZATION**：执行底层硬件初始化（SBC、CAN、RTC、BIST、引导消息）
- **PRE_RUNNING**：执行高层模块初始化（互锁、均衡、测量、电流传感器、IMD、BMS）
- **RUNNING**：正常运行模式
- **ERROR**：错误处理（不可恢复）

### 2.3 用户特征

SYS 模块由 BMS 主任务（Cyclic 10ms）调用，不直接面向终端用户。状态请求由 BMS 任务或其他引擎模块发起。

---

## 3. 功能需求

### 3.1 状态机核心管理

#### REQ-APP_ENGINE_SYS_001 — 系统状态请求设置

**编号** | REQ-APP_ENGINE_SYS_001
**优先级** | 高
**函数** | `SYS_SetStateRequest()`

**描述**：系统应提供设置状态机请求的接口，在接受请求前验证其合法性。错误请求可在任意状态被接受；初始化请求仅允许在 UNINITIALIZED 状态且无挂起请求时被接受；若已有请求挂起则返回 SYS_REQUEST_PENDING。

**前置条件**：系统已初始化，`sys_state` 全局变量有效

**后置条件**：若请求合法，`sys_state.stateRequest` 被设置为对应请求值

**错误处理**：
- `stateRequest == SYS_STATE_NO_REQUEST`：不作任何处理
- 非 UNINITIALIZED 状态下收到 `SYS_STATE_INITIALIZATION_REQUEST`：返回 `SYS_ALREADY_INITIALIZED`
- 已有请求挂起时：返回 `SYS_REQUEST_PENDING`
- 其他非法请求：返回 `SYS_ILLEGAL_REQUEST`

---

#### REQ-APP_ENGINE_SYS_002 — 状态机触发函数

**编号** | REQ-APP_ENGINE_SYS_002
**优先级** | 高
**函数** | `SYS_Trigger()`

**描述**：系统应提供 10ms 周期触发函数来驱动状态机运行。每次被调用时先检测是否发生多重调用（重入保护），再检查定时器是否到期（timer > 0），仅当定时器到期时才执行状态机处理逻辑。

**前置条件**：`pSystemState` 非空

**处理流程**：
1. 断言检查 `pSystemState != NULL_PTR`
2. 多重调用检测：`SYS_CheckMultipleCalls()` → 若返回 `SYS_MULTIPLE_CALLS_YES`，返回 `STD_NOT_OK`
3. 定时器检查：若 `timer > 0`，递减 timer；若递减后仍 > 0，递减 `triggerEntry` 并返回 `STD_OK`
4. 执行 `SYS_RunStateMachine()` 处理当前状态
5. 递减 `triggerEntry` 计数器

**后置条件**：状态机被推进一个处理周期

---

#### REQ-APP_ENGINE_SYS_003 — 当前状态查询

**编号** | REQ-APP_ENGINE_SYS_003
**优先级** | 中
**函数** | `SYS_GetSystemState()`, `SYS_GetSystemSubstate()`

**描述**：系统应提供获取当前系统状态和子状态的只读接口。

---

### 3.2 状态机核心逻辑

#### REQ-APP_ENGINE_SYS_004 — 状态机主调度（RunStateMachine）

**编号** | REQ-APP_ENGINE_SYS_004
**优先级** | 高
**函数** | `SYS_RunStateMachine()`

**描述**：系统应根据当前状态分发到对应的状态处理函数，并在处理完成后根据返回的下一状态执行状态转移。

**处理流程**：
1. 从 `sys_state` 传输状态请求（调用 `SYS_TransferStateRequest()`）
2. 根据 `currentState` 分发处理：
   - `SYS_FSM_STATE_UNINITIALIZED`：检查是否有 `SYS_STATE_INITIALIZATION_REQUEST`，若有则转入 INITIALIZATION 状态
   - `SYS_FSM_STATE_INITIALIZATION`：调用 `SYS_ProcessInitializationState()`
   - `SYS_FSM_STATE_PRE_RUNNING`：调用 `SYS_ProcessPreRunningState()`
   - `SYS_FSM_STATE_RUNNING`：调用 `SYS_ProcessRunningState()`
   - `SYS_FSM_STATE_ERROR`：调用 `SYS_ProcessErrorState()`
3. 对非法状态通过 `FAS_ASSERT(FAS_TRAP)` 触发断言

**错误处理**：无效状态触发 `FAS_ASSERT(FAS_TRAP)`

---

#### REQ-APP_ENGINE_SYS_005 — 状态转移与子状态转移

**编号** | REQ-APP_ENGINE_SYS_005
**优先级** | 高
**函数** | `SYS_SetState()`, `SYS_SetSubstate()`

**描述**：系统应提供状态转移和子状态转移的内部函数。

`SYS_SetState()` 的处理逻辑：
- 若下一状态和子状态均与当前相同：重置 nextState/nextSubstate 为 DUMMY，提前退出
- 若下一状态为 ERROR：直接进入错误状态（跳过 ENTRY 子状态）
- 若下一状态不同且非 ERROR：更新 previous/current 状态，currentSubstate 设为 ENTRY
- 若仅子状态不同：委托 `SYS_SetSubstate()` 处理

`SYS_SetSubstate()` 的处理逻辑：
- 设置 timer、更新 previousSubstate/currentSubstate、重置 nextSubstate

---

#### REQ-APP_ENGINE_SYS_006 — 多重调用检测

**编号** | REQ-APP_ENGINE_SYS_006
**优先级** | 高
**函数** | `SYS_CheckMultipleCalls()`

**描述**：系统应检测 `SYS_Trigger()` 是否被重入调用。通过 `triggerEntry` 计数器实现：若为 0 则递增并返回 NO；若非 0 则返回 YES。检测在临界区内进行以保证原子性。

---

#### REQ-APP_ENGINE_SYS_007 — 状态请求传输

**编号** | REQ-APP_ENGINE_SYS_007
**优先级** | 中
**函数** | `SYS_TransferStateRequest()`

**描述**：系统应在临界区内将 `sys_state.stateRequest` 读出并重置为 `SYS_STATE_NO_REQUEST`，以保证请求传输的原子性。

---

### 3.3 初始化序列

#### REQ-APP_ENGINE_SYS_008 — 深度放电检查

**编号** | REQ-APP_ENGINE_SYS_008
**优先级** | 高
**函数** | `SYS_ProcessInitializationState()`（`SYS_FSM_CHECK_DEEP_DISCHARGE` 子状态）

**描述**：系统应在初始化阶段读取 FRAM 中存储的深度放电标志，对每个电池串检查是否发生过深度放电，若检测到深度放电则向诊断模块报告事件。

---

#### REQ-APP_ENGINE_SYS_009 — SBC 初始化

**编号** | REQ-APP_ENGINE_SYS_009
**优先级** | 高
**函数** | `SYS_ProcessInitializationState()`（`SYS_FSM_SUBSTATE_START_INITIALIZATION_SBC` / `SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_SBC` 子状态）

**描述**：系统应启动 SBC（系统基础芯片）初始化，并等待其状态机进入 RUNNING 状态。若超时（`SYS_STATE_MACHINE_SBC_INIT_TIMEOUT_MS`），应转入 ERROR 状态，错误子状态为 `SYS_FSM_SUBSTATE_SBC_INITIALIZATION_ERROR`。

---

#### REQ-APP_ENGINE_SYS_010 — CAN 收发器初始化

**编号** | REQ-APP_ENGINE_SYS_010
**优先级** | 高
**函数** | `SYS_ProcessInitializationState()`（`SYS_FSM_SUBSTATE_INITIALIZATION_CAN` 子状态）

**描述**：系统应在初始化序列中调用 `CAN_Initialize()` 完成 CAN 收发器初始化。

---

#### REQ-APP_ENGINE_SYS_011 — RTC 初始化等待

**编号** | REQ-APP_ENGINE_SYS_011
**优先级** | 中
**函数** | `SYS_ProcessInitializationState()`（`SYS_FSM_SUBSTATE_INITIALIZATION_RTC` 子状态）

**描述**：系统应等待 RTC 模块初始化完成（`RTC_IsRtcModuleInitialized()` 返回 true）后，才进入下一初始化步骤。

---

#### REQ-APP_ENGINE_SYS_012 — 启动自检（BIST）

**编号** | REQ-APP_ENGINE_SYS_012
**优先级** | 高
**函数** | `SYS_ProcessInitializationState()`（`SYS_FSM_SUBSTATE_START_UP_BIST` 子状态）、`SYS_GeneralMacroBist()`

**描述**：系统应在初始化阶段执行启动自检：
- 通用宏自检：验证 `general.h` 中 `GEN_REPEAT_U` 宏的正确性，通过填充 Magic Number (42) 并逐元素断言验证
- 数据库自检：调用 `DATA_ExecuteDataBist()` 验证数据库完整性

所有自检函数若失败则通过断言进入死循环（不转入 ERROR 状态）。

---

#### REQ-APP_ENGINE_SYS_013 — 引导消息发送

**编号** | REQ-APP_ENGINE_SYS_013
**优先级** | 中
**函数** | `SYS_ProcessInitializationState()`（`SYS_FSM_SUBSTATE_SEND_BOOT_MESSAGE` 子状态）、`SYS_SendBootMessage()`

**描述**：系统应在初始化完成时通过 CAN 总线发送引导消息序列，包含：引导魔数（起始/结束）、BMS 版本信息、Git 提交哈希、MCU 唯一芯片 ID、MCU 批号、MCU 晶圆信息、启动时间戳。任一步骤失败通过断言触发陷阱。

---

### 3.4 预运行序列

#### REQ-APP_ENGINE_SYS_014 — 互锁初始化

**编号** | REQ-APP_ENGINE_SYS_014
**优先级** | 高
**函数** | `SYS_ProcessPreRunningState()`（`SYS_FSM_SUBSTATE_INITIALIZE_INTERLOCK` 子状态）

**描述**：系统应在预运行阶段初始化互锁模块，发送 `ILCK_STATE_INITIALIZATION_REQUEST` 请求。

---

#### REQ-APP_ENGINE_SYS_015 — 均衡模块初始化

**编号** | REQ-APP_ENGINE_SYS_015
**优先级** | 高
**函数** | `SYS_ProcessPreRunningState()`（`SYS_FSM_SUBSTATE_START_INITIALIZATION_BAL` / `SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL` / `SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BAL_GLOBAL_ENABLE` 子状态）

**描述**：系统应初始化均衡模块并等待完成：
1. 发送 `BAL_STATE_INIT_REQUEST` 初始化请求
2. 等待 `BAL_GetInitializationState()` 返回 `STD_OK`
3. 根据编译配置 `BS_BALANCING_DEFAULT_INACTIVE` 设置全局均衡使能/禁用状态
4. 若超时（参看各自超时阈值），转入 ERROR 状态

---

#### REQ-APP_ENGINE_SYS_016 — 首次测量周期启动与等待

**编号** | REQ-APP_ENGINE_SYS_016
**优先级** | 高
**函数** | `SYS_ProcessPreRunningState()`（`SYS_FSM_SUBSTATE_START_FIRST_MEASUREMENT_CYCLE` / `SYS_FSM_SUBSTATE_WAIT_FIRST_MEASUREMENT_CYCLE` 子状态）

**描述**：系统应启动首次测量周期（`MEAS_StartMeasurement()`），等待其完成后解锁算法模块初始化（`ALGO_UnlockInitialization()`）。若超时（由 AFE 驱动类型决定超时阈值 `SYS_STATE_MACHINE_INITIALIZATION_FIRST_MEASUREMENT_TIMEOUT_MS`），转入 ERROR 状态。

---

#### REQ-APP_ENGINE_SYS_017 — 电流传感器存在性检测

**编号** | REQ-APP_ENGINE_SYS_017
**优先级** | 高
**函数** | `SYS_ProcessPreRunningState()`（`SYS_FSM_SUBSTATE_START_CURRENT_SENSOR_PRESENCE_CHECK` / `SYS_FSM_SUBSTATE_WAIT_CURRENT_SENSOR_PRESENCE_CHECK` 子状态）

**描述**：系统应在预运行阶段检测各电池串的电流传感器是否在线：
1. 使能 CAN 周期性传输（`CAN_EnablePeriodic(true)`）
2. 等待各串电流传感器及 CC/EC 子传感器的状态报告
3. 对每个存在的传感器，根据其 CC/EC 能力分别初始化 SOC（库仑计数）和 SOE（能量计数）
4. 初始化 SOH 状态估算
5. 所有传感器就绪后初始化 SOF 模块
6. 若超时，转入 ERROR 状态

---

#### REQ-APP_ENGINE_SYS_018 — IMD 初始化

**编号** | REQ-APP_ENGINE_SYS_018
**优先级** | 中
**函数** | `SYS_ProcessPreRunningState()`（`SYS_FSM_SUBSTATE_INITIALIZATION_IMD` 子状态）

**描述**：系统应初始化绝缘监测设备（IMD）。最多重试 `SYS_STATE_MACHINE_INITIALIZATION_REQUEST_RETRY_COUNTER`（3 次），若均失败则转入 ERROR 状态。

---

#### REQ-APP_ENGINE_SYS_019 — BMS 状态机初始化

**编号** | REQ-APP_ENGINE_SYS_019
**优先级** | 高
**函数** | `SYS_ProcessPreRunningState()`（`SYS_FSM_SUBSTATE_START_INITIALIZATION_BMS` / `SYS_FSM_SUBSTATE_WAIT_INITIALIZATION_BMS` 子状态）

**描述**：系统应初始化 BMS 状态机（`BMS_STATE_INITIALIZATION_REQUEST`），等待 `BMS_GetInitializationState()` 返回 `STD_OK`。若超时，转入 ERROR 状态。BMS 初始化是预运行序列的最后一步，成功完成后转入 RUNNING 状态。

---

### 3.5 运行与错误状态

#### REQ-APP_ENGINE_SYS_020 — 运行状态

**编号** | REQ-APP_ENGINE_SYS_020
**优先级** | 中
**函数** | `SYS_ProcessRunningState()`

**描述**：系统在 RUNNING 状态下应保持当前状态。定时器设为 `SYS_FSM_LONG_TIME`（100ms 即 10 × 10ms 周期）。

---

#### REQ-APP_ENGINE_SYS_021 — 错误状态

**编号** | REQ-APP_ENGINE_SYS_021
**优先级** | 高
**函数** | `SYS_ProcessErrorState()`

**描述**：系统在 ERROR 状态下应保持当前状态，不进行自动恢复。定时器设为 `SYS_FSM_LONG_TIME`。

---

### 3.6 软件复位

#### REQ-APP_ENGINE_SYS_022 — 软件复位触发

**编号** | REQ-APP_ENGINE_SYS_022
**优先级** | 中
**函数** | `SYS_TriggerSoftwareReset()`

**描述**：系统应提供软件复位接口，当前为桩实现（stub）。

---

### 3.7 BIST 自检

#### REQ-APP_ENGINE_SYS_023 — 通用宏自检

**编号** | REQ-APP_ENGINE_SYS_023
**优先级** | 中
**函数** | `SYS_GeneralMacroBist()`

**描述**：系统应在启动自检阶段验证 `general.h` 中 `GEN_REPEAT_U` 和 `GEN_STRIP` 宏的正确性，使用 Magic Number 42 填充测试数组，逐元素断言检查。

---

## 4. 非功能需求

### 4.1 时序与性能

| 需求编号 | 描述 |
|----------|------|
| REQ-APP_ENGINE_SYS_NFR_001 | `SYS_Trigger()` 应在 10ms 周期任务上下文中调用（`SYS_TASK_CYCLE_CONTEXT_MS = 10`） |
| REQ-APP_ENGINE_SYS_NFR_002 | 状态间短等待为 1 个周期（10ms），中等等待为 5 个周期（50ms），长等待为 10 个周期（100ms） |
| REQ-APP_ENGINE_SYS_NFR_003 | 各初始化步骤的超时阈值应可通过宏配置（SBC: 1000ms, BAL: 500ms, IMD: 500ms, 一般: 200ms） |
| REQ-APP_ENGINE_SYS_NFR_004 | 首次测量超时 `SYS_STATE_MACHINE_INITIALIZATION_FIRST_MEASUREMENT_TIMEOUT_MS` 应根据 AFE 驱动类型选择（DTNXP: 1000ms, 其他: 200ms） |

### 4.2 鲁棒性

| 需求编号 | 描述 |
|----------|------|
| REQ-APP_ENGINE_SYS_NFR_005 | 所有公开函数入口处应通过 `FAS_ASSERT` 对指针参数进行空指针检查 |
| REQ-APP_ENGINE_SYS_NFR_006 | `SYS_Trigger()` 应防止重入调用（函数运行标志 `triggerEntry`） |
| REQ-APP_ENGINE_SYS_NFR_007 | 状态请求的读写操作应在临界区（`OS_EnterTaskCritical()`/`OS_ExitTaskCritical()`）内进行 |
| REQ-APP_ENGINE_SYS_NFR_008 | 非法状态请求应被拒绝并递增 `illegalRequestsCounter` |
| REQ-APP_ENGINE_SYS_NFR_009 | 所有初始化子步骤的超时检测应保证系统不会永久卡在某一子状态 |

### 4.3 内存

| 需求编号 | 描述 |
|----------|------|
| REQ-APP_ENGINE_SYS_NFR_010 | 系统状态结构体 `SYS_STATE_s` 包含 timer, stateRequest, 状态/子状态追踪, 计数器共约 32 字节 |

---

## 5. 接口需求

### 5.1 公共 API

| 函数 | 返回类型 | 说明 |
|------|----------|------|
| `SYS_SetStateRequest()` | `SYS_RETURN_TYPE_e` | 设置系统状态请求 |
| `SYS_Trigger()` | `STD_RETURN_TYPE_e` | 10ms 周期触发函数 |
| `SYS_GetSystemState()` | `SYS_FSM_STATES_e` | 获取当前系统状态 |
| `SYS_GetSystemSubstate()` | `SYS_FSM_SUBSTATES_e` | 获取当前系统子状态 |
| `SYS_SendBootMessage()` | `void` | 发送 CAN 引导消息 |
| `SYS_TriggerSoftwareReset()` | `void` | 触发软件复位（桩） |

### 5.2 依赖项

| 被依赖模块 | 调用目的 |
|------------|----------|
| `FRAM` | 读取深度放电标志 |
| `DIAG` | 报告诊断事件 |
| `SBC` | 系统基础芯片状态管理 |
| `CAN` / `CANTX` | CAN 收发器初始化与数据发送 |
| `RTC` | 实时时钟初始化 |
| `ILCK` | 互锁模块初始化 |
| `BAL` | 电池均衡模块初始化 |
| `MEAS` | 测量周期控制 |
| `ALGO` / `SE` / `SOF` | 算法/状态估算初始化 |
| `BMS` | BMS 状态机初始化 |
| `IMD` | 绝缘监测初始化 |
| `DATA` | 数据库自检 |
| `OS` | 临界区保护 |

### 5.3 调用方

| 调用方 | 调用函数 |
|--------|----------|
| BMS 主任务 (Cyclic 10ms) | `SYS_Trigger()` |

---

## 6. 数据结构

### 6.1 系统状态结构体 `SYS_STATE_s`

| 成员 | 类型 | 说明 |
|------|------|------|
| `timer` | `uint16_t` | 状态机定时器（单位：SYS_Trigger 调用次数） |
| `stateRequest` | `SYS_STATE_REQUEST_e` | 当前待处理的状态请求 |
| `nextState` | `SYS_FSM_STATES_e` | 下一状态 |
| `nextSubstate` | `SYS_FSM_SUBSTATES_e` | 下一子状态 |
| `currentState` | `SYS_FSM_STATES_e` | 当前状态 |
| `currentSubstate` | `SYS_FSM_SUBSTATES_e` | 当前子状态 |
| `previousState` | `SYS_FSM_STATES_e` | 前一状态 |
| `previousSubstate` | `SYS_FSM_SUBSTATES_e` | 前一子状态 |
| `illegalRequestsCounter` | `uint32_t` | 非法请求计数器 |
| `initializationTimeout` | `uint16_t` | 初始化超时计数器 |
| `triggerEntry` | `uint8_t` | 触发函数重入保护标志 |

### 6.2 关键枚举类型

**SYS_FSM_STATES_e**：`DUMMY`, `HAS_NEVER_RUN`, `UNINITIALIZED`, `INITIALIZATION`, `PRE_RUNNING`, `RUNNING`, `ERROR`

**SYS_STATE_REQUEST_e**：`INITIALIZATION_REQUEST`, `ERROR_REQUEST`, `NO_REQUEST`

**SYS_RETURN_TYPE_e**：`SYS_OK`, `SYS_BUSY_OK`, `SYS_REQUEST_PENDING`, `SYS_ILLEGAL_REQUEST`, `SYS_ALREADY_INITIALIZED`

---

## 7. 状态机 / 控制流

### 7.1 顶层状态转移图

```text
                          ┌────────────────────────────────────┐
                          │                                    │
                          ▼                                    │
UNINITIALIZED ──► INITIALIZATION ──► PRE_RUNNING ──► RUNNING   │
                      │                    │                    │
                      └────────────────────┴────────────────────┘
                                    ERROR
```

### 7.2 初始化序列流程

```text
ENTRY → CHECK_DEEP_DISCHARGE → START_INIT_SBC → WAIT_INIT_SBC
→ INIT_CAN → INIT_RTC → START_UP_BIST → SEND_BOOT_MESSAGE → PRE_RUNNING
```

### 7.3 预运行序列流程

```text
ENTRY → INIT_INTERLOCK → START_INIT_BAL → WAIT_INIT_BAL
→ WAIT_INIT_BAL_GLOBAL_ENABLE → START_FIRST_MEAS → WAIT_FIRST_MEAS
→ START_CURRENT_SENSOR_PRESENCE_CHECK → WAIT_CURRENT_SENSOR_PRESENCE_CHECK
→ INIT_MISC → INIT_IMD → START_INIT_BMS → WAIT_INIT_BMS → RUNNING
```

---

## 8. 追溯矩阵

| 需求编号 | 源文件 | 函数/宏/变量 |
|----------|--------|-------------|
| REQ-APP_ENGINE_SYS_001 | sys.c, sys.h | `SYS_SetStateRequest()`, `SYS_CheckStateRequest()` |
| REQ-APP_ENGINE_SYS_002 | sys.c, sys.h | `SYS_Trigger()` |
| REQ-APP_ENGINE_SYS_003 | sys.c, sys.h | `SYS_GetSystemState()`, `SYS_GetSystemSubstate()` |
| REQ-APP_ENGINE_SYS_004 | sys.c | `SYS_RunStateMachine()` |
| REQ-APP_ENGINE_SYS_005 | sys.c | `SYS_SetState()`, `SYS_SetSubstate()` |
| REQ-APP_ENGINE_SYS_006 | sys.c | `SYS_CheckMultipleCalls()` |
| REQ-APP_ENGINE_SYS_007 | sys.c | `SYS_TransferStateRequest()` |
| REQ-APP_ENGINE_SYS_008 | sys.c | `SYS_ProcessInitializationState()` (CHECK_DEEP_DISCHARGE) |
| REQ-APP_ENGINE_SYS_009 | sys.c | `SYS_ProcessInitializationState()` (SBC 子状态) |
| REQ-APP_ENGINE_SYS_010 | sys.c | `SYS_ProcessInitializationState()` (CAN 子状态) |
| REQ-APP_ENGINE_SYS_011 | sys.c | `SYS_ProcessInitializationState()` (RTC 子状态) |
| REQ-APP_ENGINE_SYS_012 | sys.c | `SYS_ProcessInitializationState()` (BIST 子状态), `SYS_GeneralMacroBist()` |
| REQ-APP_ENGINE_SYS_013 | sys.c, sys_cfg.c | `SYS_ProcessInitializationState()` (SEND_BOOT_MESSAGE), `SYS_SendBootMessage()` |
| REQ-APP_ENGINE_SYS_014 | sys.c | `SYS_ProcessPreRunningState()` (INIT_INTERLOCK 子状态) |
| REQ-APP_ENGINE_SYS_015 | sys.c | `SYS_ProcessPreRunningState()` (BAL 子状态) |
| REQ-APP_ENGINE_SYS_016 | sys.c | `SYS_ProcessPreRunningState()` (MEAS 子状态) |
| REQ-APP_ENGINE_SYS_017 | sys.c | `SYS_ProcessPreRunningState()` (CURRENT_SENSOR_PRESENCE 子状态) |
| REQ-APP_ENGINE_SYS_018 | sys.c | `SYS_ProcessPreRunningState()` (IMD 子状态) |
| REQ-APP_ENGINE_SYS_019 | sys.c | `SYS_ProcessPreRunningState()` (BMS 子状态) |
| REQ-APP_ENGINE_SYS_020 | sys.c | `SYS_ProcessRunningState()` |
| REQ-APP_ENGINE_SYS_021 | sys.c | `SYS_ProcessErrorState()` |
| REQ-APP_ENGINE_SYS_022 | reset.c, reset.h | `SYS_TriggerSoftwareReset()` |
| REQ-APP_ENGINE_SYS_023 | sys.c | `SYS_GeneralMacroBist()` |
