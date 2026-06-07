# BAL — 电池均衡模块软件需求规格说明

**文件**: `bal.c`
**版本**: v1.11.0
**创建日期**: 2020-02-24
**最后更新**: 2026-04-20
**所属层级**: APPLICATION
**模块前缀**: BAL
**许可证**: BSD-3-Clause

---

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 电池管理系统（Battery Management System）中电池均衡模块（Balancing Module, BAL）的软件需求规格。BAL 模块负责管理电池单体之间的电压均衡过程，通过有限状态机（Finite State Machine, FSM）控制均衡的启动、执行、停止以及故障处理。

### 1.2 范围

#### 涵盖内容

- BAL 状态机的生命周期管理（未初始化 → 初始化 → 运行）
- 状态机状态请求的合法性校验与转移
- 重入保护机制
- 均衡全局使能/禁用的控制
- 状态历史记录保存
- 均衡阈值配置与访问
- 均衡策略（电压均衡、历史均衡、无均衡）的框架接口

#### 不涵盖内容

- 具体均衡策略的内部算法实现（属于 `bal_strategy_voltage.c`、`bal_strategy_history.c`、`bal_strategy_none.c` 的职责）
- 硬件 AFE（Analog Front End）驱动的具体操作
- 电池模型和 SOC/SOH 估算算法

### 1.3 定义与缩略语

| 术语 | 英文全称 | 说明 |
|------|----------|------|
| BAL | Balancing | 电池均衡模块 |
| FSM | Finite State Machine | 有限状态机 |
| AFE | Analog Front End | 模拟前端芯片 |
| SOC | State of Charge | 荷电状态 |
| SOH | State of Health | 健康状态 |
| RTM | Requirements Traceability Mapping | 需求可追溯性映射 |

### 1.4 参考文献

| 编号 | 文档名称 | 说明 |
|------|----------|------|
| [1] | `bal.h` | BAL 模块头文件，定义数据结构与接口 |
| [2] | `bal_cfg.h` / `bal_cfg.c` | BAL 模块配置参数 |
| [3] | `bal_strategy_voltage.c` | 基于电压的均衡策略实现 |
| [4] | `bal_strategy_history.c` | 基于历史数据的均衡策略实现 |
| [5] | `database.h` | 数据库访问接口 |

---

## 2. 总体描述

### 2.1 产品视角

BAL 模块位于 foxBMS 2 的 APPLICATION 层，是电池均衡功能的核心控制模块。它向上接收 BMS 主控模块（`bms.c`）的状态请求，向下通过均衡策略模块驱动 AFE 芯片执行实际的电池单体均衡操作。

```
┌──────────────────────────────┐
│          BMS 主控             │
│         (bms.c)              │
└─────────────┬────────────────┘
              │ 状态请求
┌─────────────▼────────────────┐
│        BAL 均衡模块           │
│    ┌────────────────────┐    │
│    │  BAL 状态机 (FSM)  │    │
│    └────────┬───────────┘    │
│             │                │
│    ┌────────▼───────────┐    │
│    │   均衡策略接口      │    │
│    │  (voltage/history/ │    │
│    │   none)            │    │
│    └────────┬───────────┘    │
└─────────────┼────────────────┘
              │
┌─────────────▼────────────────┐
│       AFE 硬件驱动            │
└──────────────────────────────┘
```

### 2.2 工作模式

BAL 模块基于有限状态机（FSM）运行，具有以下主要工作模式：

| 模式 | 状态 | 说明 |
|------|------|------|
| 未初始化 | `BAL_FSM_UNINITIALIZED` | 上电初始状态，等待初始化请求 |
| 初始化中 | `BAL_FSM_INITIALIZATION` | 执行初始化序列 |
| 已初始化 | `BAL_FSM_INITIALIZED` | 初始化完成，准备进入运行状态 |
| 检查均衡 | `BAL_FSM_CHECK_BALANCING` | 检查是否需要执行均衡 |
| 执行均衡 | `BAL_FSM_BALANCE` | 正在执行电池均衡 |
| 禁止均衡 | `BAL_FSM_NO_BALANCING` | 条件不满足，禁止均衡 |
| 允许均衡 | `BAL_FSM_ALLOW_BALANCING` | 条件满足，允许均衡 |
| 全局禁用 | `BAL_FSM_GLOBAL_DISABLE` | 全局均衡禁用 |
| 全局使能 | `BAL_FSM_GLOBAL_ENABLE` | 全局均衡使能 |
| 错误 | `BAL_FSM_ERROR` | 错误状态 |

状态机由定时器触发，触发周期为 **100ms**（由 `BAL_Trigger()` 函数在 `bms.c` 中以 100ms 周期调用）。

### 2.3 用户特征

本模块的使用者是 BMS 主控模块和应用层其他需要均衡状态信息的模块。调用方通过 `BAL_SetStateRequest()` 向状态机发起状态请求，通过 `BAL_GetInitializationState()` 查询初始化状态。

---

## 3. 功能需求

### 3.1 状态历史记录管理

#### FR-3.1.1 — 保存上一状态与子状态

- **编号**: FR-3.1.1
- **优先级**: 高
- **对应函数**: `BAL_SaveLastStates()`
- **描述**: 系统应保存状态机的上一状态（lastState）和上一子状态（lastSubstate），以便追踪状态变迁历史。

**输入**

| 参数 | 类型 | 描述 |
|------|------|------|
| `pBalancingState` | `BAL_STATE_s *` | 指向均衡状态结构体的指针 |

**前置条件**

- `pBalancingState` 不能为 NULL（通过 `FAS_ASSERT` 强制校验）

**处理流程**

1. 断言检查 `pBalancingState` 非空
2. 如果当前状态（`state`）与上一状态（`lastState`）不同，则同时更新 `lastState` 和 `lastSubstate`
3. 否则，如果当前子状态（`substate`）与上一子状态（`lastSubstate`）不同，则仅更新 `lastSubstate`
4. 否则，不做任何更新（状态和子状态均未变化）

**后置条件**

- `lastState` 反映最近一次发生状态变化时的状态值
- `lastSubstate` 反映最近一次发生状态或子状态变化时的子状态值

**错误处理**

- 若 `pBalancingState == NULL`，触发断言失败

---

### 3.2 重入保护

#### FR-3.2.1 — 重入检查

- **编号**: FR-3.2.1
- **优先级**: 高
- **对应函数**: `BAL_CheckReEntrance()`
- **描述**: 系统应提供重入保护机制，确保状态机触发函数每次仅被一个执行上下文调用。当检测到重入时返回错误码。

**输入**

| 参数 | 类型 | 描述 |
|------|------|------|
| `currentState` | `BAL_STATE_s *` | 指向当前状态结构体的指针 |

**前置条件**

- `currentState` 不能为 NULL
- 函数调用应在临界区保护下进行

**处理流程**

1. 断言检查 `currentState` 非空
2. 进入任务临界区（`OS_EnterTaskCritical()`）
3. 检查 `triggerEntry` 计数器是否为 0
4. 若为 0，递增 `triggerEntry`（标记函数正在运行）
5. 若不为 0，设置返回值 `retval = 0xFF`（表示重入检测）
6. 退出任务临界区（`OS_ExitTaskCritical()`）
7. 返回 `retval`

**后置条件**

- 若返回值 = `0`：`triggerEntry` 已递增，函数正常进入
- 若返回值 = `0xFF`：检测到重入，调用方应放弃本次执行

**错误处理**

- 若 `currentState == NULL`，触发断言失败

---

### 3.3 状态请求转移

#### FR-3.3.1 — 状态请求转移

- **编号**: FR-3.3.1
- **优先级**: 高
- **对应函数**: `BAL_TransferStateRequest()`
- **描述**: 系统应从当前状态结构体中读出待处理的状态请求，并将其重置为 `BAL_STATE_NO_REQUEST`（无请求），实现请求的原子转移。

**输入**

| 参数 | 类型 | 描述 |
|------|------|------|
| `currentState` | `BAL_STATE_s *` | 指向当前状态结构体的指针 |

**前置条件**

- `currentState` 不能为 NULL

**处理流程**

1. 断言检查 `currentState` 非空
2. 进入任务临界区
3. 读取 `currentState->stateRequest` 到局部变量 `retval`
4. 将 `currentState->stateRequest` 重置为 `BAL_STATE_NO_REQUEST`
5. 退出任务临界区
6. 返回原始的 `stateRequest` 值

**后置条件**

- `currentState->stateRequest == BAL_STATE_NO_REQUEST`
- 返回值包含转移前的原始状态请求

**错误处理**

- 若 `currentState == NULL`，触发断言失败

---

### 3.4 状态请求合法性校验

#### FR-3.4.1 — 直接通过类请求校验

- **编号**: FR-3.4.1
- **优先级**: 高
- **对应函数**: `BAL_CheckStateRequest()`
- **描述**: 系统应识别以下状态请求为"始终合法"的请求，无需检查当前状态即可直接返回 `BAL_OK`：
  - `BAL_STATE_ERROR_REQUEST`（进入错误状态的请求）
  - `BAL_STATE_GLOBAL_ENABLE_REQUEST`（全局使能均衡请求）
  - `BAL_STATE_GLOBAL_DISABLE_REQUEST`（全局禁用均衡请求）
  - `BAL_STATE_NO_BALANCING_REQUEST`（禁止均衡请求）
  - `BAL_STATE_ALLOW_BALANCING_REQUEST`（允许均衡请求）

**处理流程**

1. 若 `stateRequest == BAL_STATE_ERROR_REQUEST`，返回 `BAL_OK`
2. 若 `stateRequest == BAL_STATE_GLOBAL_ENABLE_REQUEST`，设置 `balancingGlobalAllowed = true`，返回 `BAL_OK`
3. 若 `stateRequest == BAL_STATE_GLOBAL_DISABLE_REQUEST`，设置 `balancingGlobalAllowed = false`，返回 `BAL_OK`
4. 若 `stateRequest == BAL_STATE_NO_BALANCING_REQUEST` 或 `BAL_STATE_ALLOW_BALANCING_REQUEST`，返回 `BAL_OK`

**后置条件**

- 全局使能/禁用请求会立即修改 `balancingGlobalAllowed` 标志
- 其他请求仅验证合法性，不修改状态

---

#### FR-3.4.2 — 初始化请求校验

- **编号**: FR-3.4.2
- **优先级**: 高
- **对应函数**: `BAL_CheckStateRequest()`
- **描述**: 系统应仅在以下条件**同时**满足时，接受 `BAL_STATE_INIT_REQUEST`（初始化请求）：
  - 当前没有待处理的状态请求（`stateRequest == BAL_STATE_NO_REQUEST`）
  - 当前状态为 `BAL_FSM_UNINITIALIZED`（未初始化状态）

**处理流程**

1. 若 `currentState->stateRequest != BAL_STATE_NO_REQUEST`，返回 `BAL_REQUEST_PENDING`
2. 若 `stateRequest == BAL_STATE_INIT_REQUEST`：
   - 检查 `currentState->state` 是否为 `BAL_FSM_UNINITIALIZED`
   - 若是，返回 `BAL_OK`
   - 否则，返回 `BAL_ALREADY_INITIALIZED`
3. 若 `stateRequest` 为其他值，返回 `BAL_ILLEGAL_REQUEST`

**错误处理**

| 返回值 | 含义 |
|--------|------|
| `BAL_OK` | 请求合法，可以执行 |
| `BAL_REQUEST_PENDING` | 存在待处理的请求，当前请求被拒绝 |
| `BAL_ALREADY_INITIALIZED` | 状态机已初始化，不允许重复初始化 |
| `BAL_ILLEGAL_REQUEST` | 在当前状态下该请求非法 |

---

### 3.5 初始化功能

#### FR-3.5.1 — 均衡模块初始化

- **编号**: FR-3.5.1
- **优先级**: 高
- **对应函数**: `BAL_Init()`
- **描述**: 系统应提供均衡模块的通用初始化函数，从数据库中读取均衡控制数据块，并禁用均衡功能。

**输入**

| 参数 | 类型 | 描述 |
|------|------|------|
| `pControl` | `DATA_BLOCK_BALANCING_CONTROL_s *` | 指向均衡控制数据块的指针 |

**前置条件**

- `pControl` 不能为 NULL

**处理流程**

1. 断言检查 `pControl` 非空
2. 从数据库读取均衡控制数据（`DATA_READ_DATA(pControl)`）
3. 设置 `enableBalancing = false`（禁用均衡）
4. 将数据写回数据库（`DATA_WRITE_DATA(pControl)`）
5. 返回 `STD_OK`

**后置条件**

- 均衡功能已被禁用（`enableBalancing == false`）
- 数据已同步回数据库

**错误处理**

- 若 `pControl == NULL`，触发断言失败

---

### 3.6 状态机状态处理

#### FR-3.6.1 — 未初始化状态处理

- **编号**: FR-3.6.1
- **优先级**: 高
- **对应函数**: `BAL_ProcessStateUninitialized()`
- **描述**: 系统应在未初始化状态（`BAL_FSM_UNINITIALIZED`）下处理传入的状态请求：
  - 接收到 `BAL_STATE_INIT_REQUEST` 时，转换到初始化状态
  - 接收到 `BAL_STATE_NO_REQUEST` 时，不做任何操作
  - 接收到其他请求时，递增非法请求计数器

**输入**

| 参数 | 类型 | 描述 |
|------|------|------|
| `pCurrentState` | `BAL_STATE_s *` | 指向当前状态结构体的指针 |
| `stateRequest` | `BAL_STATE_REQUEST_e` | 传入的状态请求 |

**处理流程**

1. 断言检查 `pCurrentState` 非空
2. 若 `stateRequest == BAL_STATE_INIT_REQUEST`：
   - 设置定时器为 `BAL_FSM_SHORTTIME_100ms`（100ms）
   - 设置状态为 `BAL_FSM_INITIALIZATION`
   - 设置子状态为 `BAL_ENTRY`
3. 若 `stateRequest == BAL_STATE_NO_REQUEST`：不做操作
4. 否则：`errorRequestCounter++`（递增非法请求计数器）

**后置条件**

- 若请求为初始化：状态机进入 `BAL_FSM_INITIALIZATION` 状态
- 若请求非法：错误计数器递增

**错误处理**

- 非法请求不阻塞系统运行，仅记录到 `errorRequestCounter`
- 若 `pCurrentState == NULL`，触发断言失败

---

#### FR-3.6.2 — 初始化状态处理

- **编号**: FR-3.6.2
- **优先级**: 高
- **对应函数**: `BAL_ProcessStateInitialization()`
- **描述**: 系统应在初始化状态（`BAL_FSM_INITIALIZATION`）下直接转换到已初始化状态（`BAL_FSM_INITIALIZED`）。

**输入**

| 参数 | 类型 | 描述 |
|------|------|------|
| `currentState` | `BAL_STATE_s *` | 指向当前状态结构体的指针 |

**处理流程**

1. 断言检查 `currentState` 非空
2. 设置定时器为 `BAL_FSM_SHORTTIME_100ms`（100ms）
3. 设置状态为 `BAL_FSM_INITIALIZED`
4. 设置子状态为 `BAL_ENTRY`

**后置条件**

- 状态机进入 `BAL_FSM_INITIALIZED` 状态
- 下一触发周期（100ms 后）将进入下一状态

**错误处理**

- 若 `currentState == NULL`，触发断言失败

---

#### FR-3.6.3 — 已初始化状态处理

- **编号**: FR-3.6.3
- **优先级**: 高
- **对应函数**: `BAL_ProcessStateInitialized()`
- **描述**: 系统应在已初始化状态（`BAL_FSM_INITIALIZED`）下完成初始化确认并转换到均衡检查状态（`BAL_FSM_CHECK_BALANCING`）。

**输入**

| 参数 | 类型 | 描述 |
|------|------|------|
| `currentState` | `BAL_STATE_s *` | 指向当前状态结构体的指针 |

**处理流程**

1. 断言检查 `currentState` 非空
2. 设置 `initializationFinished = STD_OK`（标记初始化完成）
3. 设置定时器为 `BAL_FSM_SHORTTIME_100ms`（100ms）
4. 设置状态为 `BAL_FSM_CHECK_BALANCING`
5. 设置子状态为 `BAL_ENTRY`

**后置条件**

- `initializationFinished == STD_OK`
- 状态机进入 `BAL_FSM_CHECK_BALANCING` 状态
- 外部模块可通过 `BAL_GetInitializationState()` 查询到初始化已完成

**错误处理**

- 若 `currentState == NULL`，触发断言失败

---

### 3.7 BAL 状态机状态转移图

```
    ┌──────────────┐
    │ UNINITIALIZED│ ◄────────────────────── 上电初始
    └──────┬───────┘
           │ INIT_REQUEST
    ┌──────▼───────┐
    │INITIALIZATION│ (100ms)
    └──────┬───────┘
           │
    ┌──────▼───────┐
    │ INITIALIZED  │ (100ms)
    └──────┬───────┘
           │
    ┌──────▼──────────┐
    │ CHECK_BALANCING │ ◄──── 运行主循环
    └──────┬──────────┘
           │
    ┌──────▼───────┐    ┌──────────────┐
    │   BALANCE    │    │ NO_BALANCING │
    └──────────────┘    └──────────────┘
                              │
                        ┌─────▼──────┐
                        │ALLOW_BAL.. │
                        └────────────┘

    ┌──────────────┐    ┌──────────────┐
    │GLOBAL_DISABLE│    │ GLOBAL_ENABLE│
    └──────────────┘    └──────────────┘

    ┌──────────────┐
    │   ERROR      │ (任意状态下接收到 ERROR_REQUEST)
    └──────────────┘
```

---

## 4. 非功能需求

### 4.1 时序与性能

#### NFR-4.1.1 — 状态机触发周期

- **编号**: NFR-4.1.1
- **描述**: BAL 状态机应以 **100ms** 为周期被 `BAL_Trigger()` 函数定时触发。
- **依据**: `BAL_FSM_SHORTTIME_100ms = 1u`（1 × 100ms），`bal.h` 中文档说明该函数每 100ms 调用一次

#### NFR-4.1.2 — 长延时周期

- **编号**: NFR-4.1.2
- **描述**: 系统应支持 5 秒（`BAL_FSM_LONGTIME_100ms = 50u`，即 50 × 100ms）的长延时，用于均衡等待等场景。
- **依据**: `bal_cfg.h` 中 `BAL_FSM_LONGTIME_100ms (50u)`

#### NFR-4.1.3 — 均衡执行时间

- **编号**: NFR-4.1.3
- **描述**: 均衡执行周期应为 1 秒（`BAL_FSM_BALANCING_TIME_100ms = 10u`，即 10 × 100ms）。
- **依据**: `bal_cfg.h` 中 `BAL_FSM_BALANCING_TIME_100ms (10u)`

### 4.2 内存

#### NFR-4.2.1 — 状态结构体大小

- **编号**: NFR-4.2.1
- **描述**: `BAL_STATE_s` 结构体应保持紧凑，仅包含状态机运行所必需的字段，避免不必要的内存开销。
- **依据**: 结构体包含约 13 个字段，用于嵌入式系统

### 4.3 鲁棒性

#### NFR-4.3.1 — 空指针保护

- **编号**: NFR-4.3.1
- **描述**: 所有接受指针参数的公开函数（extern）必须在函数入口处使用 `FAS_ASSERT` 进行非空校验。
- **依据**: `bal.c` 中所有 8 个 extern 函数均以 `FAS_ASSERT(ptr != NULL_PTR)` 开头

#### NFR-4.3.2 — 重入保护

- **编号**: NFR-4.3.2
- **描述**: 状态机触发函数应具备重入检测能力，防止同一函数被多个执行上下文并发调用。
- **依据**: `BAL_CheckReEntrance()` 函数的 `triggerEntry` 计数器机制

#### NFR-4.3.3 — 临界区保护

- **编号**: NFR-4.3.3
- **描述**: 所有涉及状态变量读-修改-写（read-modify-write）的操作必须在任务临界区（`OS_EnterTaskCritical()` / `OS_ExitTaskCritical()`）内执行，确保操作的原子性。
- **依据**: `BAL_CheckReEntrance()`、`BAL_TransferStateRequest()`、`BAL_SetBalancingThreshold()` 均使用临界区保护

#### NFR-4.3.4 — 输入边界限定

- **编号**: NFR-4.3.4
- **描述**: 均衡阈值设置函数应对输入参数进行边界检查，确保阈值在 `[BAL_MINIMUM_THRESHOLD_mV, BAL_MAXIMUM_THRESHOLD_mV]` 范围内。
- **依据**: `bal_cfg.c` 中 `BAL_SetBalancingThreshold()` 的边界限定逻辑（0mV ~ 5000mV）

### 4.4 可配置性

#### NFR-4.4.1 — 配置参数集中管理

- **编号**: NFR-4.4.1
- **描述**: 所有 BAL 模块可调参数应集中定义在 `bal_cfg.h` 中，包括但不限于：

| 宏定义 | 默认值 | 单位 | 说明 |
|--------|--------|------|------|
| `BAL_FSM_SHORTTIME_100ms` | `1u` | ×100ms | 短延时（100ms） |
| `BAL_FSM_LONGTIME_100ms` | `50u` | ×100ms | 长延时（5s） |
| `BAL_FSM_BALANCING_TIME_100ms` | `10u` | ×100ms | 均衡时间（1s） |
| `BAL_DEFAULT_THRESHOLD_mV` | `200` | mV | 默认均衡阈值 |
| `BAL_MAXIMUM_THRESHOLD_mV` | `5000` | mV | 最大均衡阈值 |
| `BAL_MINIMUM_THRESHOLD_mV` | `0` | mV | 最小均衡阈值 |
| `BAL_HYSTERESIS_mV` | `200` | mV | 均衡结束迟滞 |
| `BAL_LOWER_VOLTAGE_LIMIT_mV` | `2000` | mV | 均衡电压下限 |
| `BAL_UPPER_TEMPERATURE_LIMIT_ddegC` | `700` | 0.1°C | 均衡温度上限（70°C） |

### 4.5 可测试性

#### NFR-4.5.1 — 单元测试支持

- **编号**: NFR-4.5.1
- **描述**: 模块应通过条件编译（`#ifdef UNITY_UNIT_TEST`）暴露内部函数和数据，支持单元测试框架直接访问静态函数和静态变量。
- **依据**: `bal.h` 中 `UNITY_UNIT_TEST` 区块暴露了 `TEST_BAL_GetBalancingControl()`、`TEST_BAL_GetBalancingState()`、`BAL_GetState()` 等测试接口

---

## 5. 接口需求

### 5.1 公共 API

| 函数 | 返回类型 | 描述 |
|------|----------|------|
| `BAL_SaveLastStates()` | `void` | 保存上一状态和子状态 |
| `BAL_CheckReEntrance()` | `uint8_t` | 重入检查 |
| `BAL_TransferStateRequest()` | `BAL_STATE_REQUEST_e` | 转移状态请求 |
| `BAL_CheckStateRequest()` | `BAL_RETURN_TYPE_e` | 校验状态请求合法性 |
| `BAL_Init()` | `STD_RETURN_TYPE_e` | 均衡模块初始化 |
| `BAL_ProcessStateUninitialized()` | `void` | 处理未初始化状态 |
| `BAL_ProcessStateInitialization()` | `void` | 处理初始化状态 |
| `BAL_ProcessStateInitialized()` | `void` | 处理已初始化状态 |
| `BAL_SetStateRequest()` | `BAL_RETURN_TYPE_e` | 设置状态机请求（在策略文件中实现） |
| `BAL_GetInitializationState()` | `STD_RETURN_TYPE_e` | 获取初始化状态（在策略文件中实现） |
| `BAL_Trigger()` | `void` | 状态机触发函数（在策略文件中实现） |
| `BAL_SetBalancingThreshold()` | `void` | 设置均衡阈值 |
| `BAL_GetBalancingThreshold_mV()` | `int32_t` | 获取均衡阈值 |

### 5.2 依赖项

| 依赖模块 | 头文件 | 使用功能 |
|----------|--------|----------|
| BAL 配置 | `bal_cfg.h` | 时间常量、阈值宏 |
| 数据库 | `database.h` | `DATA_READ_DATA()`、`DATA_WRITE_DATA()`、`DATA_BLOCK_BALANCING_CONTROL_s` |
| 操作系统 | `os.h` | `OS_EnterTaskCritical()`、`OS_ExitTaskCritical()` |
| 标准库 | `<stdbool.h>`、`<stdint.h>` | 布尔类型、整数类型 |

### 5.3 调用方

| 调用模块 | 调用接口 | 场景 |
|----------|----------|------|
| `bms.c` | `BAL_Trigger()` | 以 100ms 周期触发状态机 |
| `bms.c` | `BAL_SetStateRequest()` | 发起均衡相关状态请求 |
| `bms.c` | `BAL_GetInitializationState()` | 查询均衡初始化状态 |
| 均衡策略模块 | `BAL_SaveLastStates()` | 保存状态历史 |
| 均衡策略模块 | `BAL_CheckReEntrance()` | 重入检查 |
| 均衡策略模块 | `BAL_TransferStateRequest()` | 获取待处理请求 |
| 均衡策略模块 | `BAL_CheckStateRequest()` | 验证请求合法性 |
| 均衡策略模块 | `BAL_ProcessState*()` | 执行各状态的处理逻辑 |

---

## 6. 数据结构

### 6.1 BAL_STATE_s — 均衡状态结构体

| 字段 | 类型 | 描述 |
|------|------|------|
| `timer` | `uint16_t` | 状态机定时器，单位为 100ms 计数 |
| `stateRequest` | `BAL_STATE_REQUEST_e` | 当前待处理的状态请求 |
| `state` | `BAL_FSM_e` | 状态机当前状态 |
| `substate` | `BAL_FSM_SUB_e` | 状态机当前子状态 |
| `lastState` | `BAL_FSM_e` | 上一状态（历史记录） |
| `lastSubstate` | `uint8_t` | 上一子状态（历史记录） |
| `triggerEntry` | `uint8_t` | 重入保护计数器（0=未运行，非0=正在运行） |
| `errorRequestCounter` | `uint32_t` | 非法请求累计计数 |
| `initializationFinished` | `STD_RETURN_TYPE_e` | 初始化完成标志 |
| `active` | `bool` | 均衡是否激活 |
| `balancingThreshold` | `int32_t` | 有效均衡阈值（mV） |
| `balancingAllowed` | `bool` | 是否允许均衡（局部） |
| `balancingGlobalAllowed` | `bool` | 是否全局允许均衡 |

### 6.2 枚举类型

#### BAL_FSM_e — 状态机状态枚举

| 枚举值 | 说明 |
|--------|------|
| `BAL_FSM_UNINITIALIZED` | 未初始化 |
| `BAL_FSM_INITIALIZATION` | 初始化中 |
| `BAL_FSM_INITIALIZED` | 已初始化 |
| `BAL_FSM_CHECK_BALANCING` | 检查均衡条件 |
| `BAL_FSM_BALANCE` | 执行均衡 |
| `BAL_FSM_NO_BALANCING` | 禁止均衡 |
| `BAL_FSM_ALLOW_BALANCING` | 允许均衡 |
| `BAL_FSM_GLOBAL_DISABLE` | 全局禁用 |
| `BAL_FSM_GLOBAL_ENABLE` | 全局使能 |
| `BAL_FSM_UNDEFINED` | 未定义（保留） |
| `BAL_FSM_RESERVED1` | 保留 |
| `BAL_FSM_ERROR` | 错误状态 |

#### BAL_FSM_SUB_e — 状态机子状态枚举

| 枚举值 | 说明 |
|--------|------|
| `BAL_ENTRY` | 入口子状态 |
| `BAL_CHECK_IMBALANCES` | 检查不均衡 |
| `BAL_COMPUTE_IMBALANCES` | 计算不均衡量 |
| `BAL_ACTIVATE_BALANCING` | 激活均衡电阻 |
| `BAL_CHECK_LOWEST_VOLTAGE` | 检查最低电压 |
| `BAL_CHECK_CURRENT` | 检查电流 |

#### BAL_STATE_REQUEST_e — 状态请求枚举

| 枚举值 | 说明 |
|--------|------|
| `BAL_STATE_INIT_REQUEST` | 初始化请求 |
| `BAL_STATE_ERROR_REQUEST` | 进入错误状态请求 |
| `BAL_STATE_NO_BALANCING_REQUEST` | 禁止均衡请求 |
| `BAL_STATE_ALLOW_BALANCING_REQUEST` | 允许均衡请求 |
| `BAL_STATE_GLOBAL_DISABLE_REQUEST` | 全局禁用请求 |
| `BAL_STATE_GLOBAL_ENABLE_REQUEST` | 全局使能请求 |
| `BAL_STATE_NO_REQUEST` | 无请求（默认） |

#### BAL_RETURN_TYPE_e — 状态请求返回类型

| 枚举值 | 说明 |
|--------|------|
| `BAL_OK` | 操作成功 |
| `BAL_BUSY_OK` | 忙，但操作可继续 |
| `BAL_REQUEST_PENDING` | 存在待处理请求 |
| `BAL_ILLEGAL_REQUEST` | 非法请求 |
| `BAL_INIT_ERROR` | 初始化错误 |
| `BAL_OK_FROM_ERROR` | 从错误中恢复 |
| `BAL_ERROR` | 一般性错误 |
| `BAL_ALREADY_INITIALIZED` | 已初始化（拒绝重复初始化） |
| `BAL_ILLEGAL_TASK_TYPE` | 非法任务类型 |

---

## 7. 状态机 / 控制流

### 7.1 初始化流程

```
BAL_Init()
  └─ DATA_READ_DATA(pControl)
  └─ enableBalancing = false
  └─ DATA_WRITE_DATA(pControl)
  └─ return STD_OK
```

### 7.2 状态机运行流程

```
BAL_Trigger() (每 100ms 调用)
  ├─ BAL_CheckReEntrance()          // 重入检查
  ├─ BAL_TransferStateRequest()     // 获取待处理请求
  ├─ BAL_CheckStateRequest()        // 验证请求合法性
  └─ switch(state):
      ├─ BAL_FSM_UNINITIALIZED  → BAL_ProcessStateUninitialized()
      ├─ BAL_FSM_INITIALIZATION → BAL_ProcessStateInitialization()
      ├─ BAL_FSM_INITIALIZED    → BAL_ProcessStateInitialized()
      ├─ BAL_FSM_CHECK_BALANCING → (策略模块处理)
      ├─ BAL_FSM_BALANCE        → (策略模块处理)
      ├─ BAL_FSM_NO_BALANCING   → (策略模块处理)
      ├─ BAL_FSM_ALLOW_BALANCING → (策略模块处理)
      ├─ BAL_FSM_GLOBAL_DISABLE → (策略模块处理)
      ├─ BAL_FSM_GLOBAL_ENABLE  → (策略模块处理)
      └─ BAL_FSM_ERROR          → (策略模块处理)
  └─ BAL_SaveLastStates()           // 保存状态历史
```

### 7.3 状态请求合法性矩阵

| 当前状态 \ 请求类型 | INIT | ERROR | NO_BAL | ALLOW_BAL | GLOBAL_DIS | GLOBAL_EN | NO_REQ |
|---------------------|------|-------|--------|-----------|------------|-----------|--------|
| UNINITIALIZED | ✅(1) | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |
| 其他状态 | ❌(2) | ✅ | ✅ | ✅ | ✅ | ✅ | ✅ |

> (1) 仅当 `stateRequest == NO_REQUEST` 时接受 INIT_REQUEST
> (2) 返回 `BAL_ALREADY_INITIALIZED`

---

## 8. 追溯矩阵

### 8.1 需求 → 代码位置映射

下表提供每个需求到源代码的快速追溯。代码中也通过 `@req` 注释标记了对应的需求编号。

#### 功能需求（FR）

| 需求编号 | 需求简述 | 实现文件 | 函数 / 定义 |
|----------|----------|----------|-------------|
| FR-3.1.1 | 保存上一状态与子状态 | `bal.c` | `BAL_SaveLastStates()` |
| FR-3.2.1 | 重入检查 | `bal.c` | `BAL_CheckReEntrance()` |
| FR-3.3.1 | 状态请求转移 | `bal.c` | `BAL_TransferStateRequest()` |
| FR-3.4.1 | 直接通过类请求校验 | `bal.c` | `BAL_CheckStateRequest()`（前半部分） |
| FR-3.4.2 | 初始化请求校验 | `bal.c` | `BAL_CheckStateRequest()`（后半部分） |
| FR-3.5.1 | 均衡模块初始化 | `bal.c` | `BAL_Init()` |
| FR-3.6.1 | 未初始化状态处理 | `bal.c` | `BAL_ProcessStateUninitialized()` |
| FR-3.6.2 | 初始化状态处理 | `bal.c` | `BAL_ProcessStateInitialization()` |
| FR-3.6.3 | 已初始化状态处理 | `bal.c` | `BAL_ProcessStateInitialized()` |

#### 非功能需求（NFR）

| 需求编号 | 需求简述 | 实现文件 | 函数 / 定义 |
|----------|----------|----------|-------------|
| NFR-4.1.1 | 状态机触发周期 100ms | `bal_cfg.h` | `BAL_FSM_SHORTTIME_100ms (1u)` |
| NFR-4.1.2 | 长延时周期 5s | `bal_cfg.h` | `BAL_FSM_LONGTIME_100ms (50u)` |
| NFR-4.1.3 | 均衡执行时间 1s | `bal_cfg.h` | `BAL_FSM_BALANCING_TIME_100ms (10u)` |
| NFR-4.2.1 | 状态结构体紧凑 | `bal.h` | `BAL_STATE_s` 结构体（13 字段） |
| NFR-4.3.1 | 空指针保护 | `bal.c` | 所有 extern 函数的 `FAS_ASSERT(ptr != NULL_PTR)` |
| NFR-4.3.2 | 重入保护 | `bal.c` | `BAL_CheckReEntrance()` 的 `triggerEntry` 计数 |
| NFR-4.3.3 | 临界区保护 | `bal.c` / `bal_cfg.c` | `OS_EnterTaskCritical()` / `OS_ExitTaskCritical()` |
| NFR-4.3.4 | 输入边界限定 | `bal_cfg.c` | `BAL_SetBalancingThreshold()` 的边界检查 |
| NFR-4.4.1 | 配置参数集中管理 | `bal_cfg.h` | 所有 `#define BAL_*` 宏定义 |
| NFR-4.5.1 | 单元测试支持 | `bal.h` / `bal.c` | `#ifdef UNITY_UNIT_TEST` 区块 |

#### 数据结构

| 章节 | 内容 | 实现文件 | 定义 |
|------|------|----------|------|
| 6.1 | BAL_STATE_s 状态结构体 | `bal.h` | `typedef struct { ... } BAL_STATE_s` |
| 6.2 | BAL_FSM_e 状态枚举 | `bal.h` | `typedef enum { ... } BAL_FSM_e` |
| 6.2 | BAL_FSM_SUB_e 子状态枚举 | `bal.h` | `typedef enum { ... } BAL_FSM_SUB_e` |
| 6.2 | BAL_STATE_REQUEST_e 请求枚举 | `bal.h` | `typedef enum { ... } BAL_STATE_REQUEST_e` |
| 6.2 | BAL_RETURN_TYPE_e 返回类型枚举 | `bal.h` | `typedef enum { ... } BAL_RETURN_TYPE_e` |

### 8.2 代码 → 需求反向索引

代码中通过 `@req` 注释可快速定位对应需求。主要入口点：

- `bal.h` — 枚举、结构体、API 声明（所有功能需求和数据结构）
- `bal.c` — 函数实现（FR-3.x.x）和非功能需求落地（NFR-4.3.1/4.3.2/4.3.3）
- `bal_cfg.h` — 配置宏定义（NFR-4.1.x, NFR-4.4.1）
- `bal_cfg.c` — 阈值管理实现（NFR-4.3.4, NFR-4.3.3）

详见 [BAL_REQUIREMENT_TRACEABILITY_MAPPING.md](BAL_REQUIREMENT_TRACEABILITY_MAPPING.md)

---

**文档生成日期**: 2026-06-05
**文档更新日期**: 2026-06-06（添加代码位置追溯表）
**生成工具**: Claude Code (write-software-requirements skill)
**适用范围**: foxBMS 2 v1.11.0
