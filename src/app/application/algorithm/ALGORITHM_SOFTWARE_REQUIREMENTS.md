# ALGORITHM — 软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 算法执行框架模块（ALGORITHM）的软件需求规格。该模块提供了一个通用的算法调度与执行框架，负责管理多个电池管理算法的初始化、周期性执行和执行时间监控。

### 1.2 范围

- **涵盖**：算法状态机管理、算法周期性调度、算法初始化流程、执行时间监控、算法重初始化机制
- **不涵盖**：具体算法的实现逻辑（如 SOC 估算、SOE 估算等），这些由各自模块独立定义

### 1.3 定义与缩略语

| 术语 | 说明 |
|------|------|
| ALGO | Algorithm 的缩写，本模块的前缀 |
| Tick | 算法调度的时间基准单位，默认为 100ms |
| Cycle Time | 算法的执行周期，必须是 Tick 的整数倍 |
| SRS | Software Requirements Specification，软件需求规格说明 |

### 1.4 参考文献

- [algorithm.c](algorithm.c) — 算法执行框架主实现
- [algorithm.h](algorithm.h) — 算法执行框架头文件
- [config/algorithm_cfg.c](config/algorithm_cfg.c) — 算法配置实现
- [config/algorithm_cfg.h](config/algorithm_cfg.h) — 算法配置头文件（数据结构定义）

## 2. 总体描述

### 2.1 产品视角

ALGORITHM 模块位于 foxBMS 2 的应用层（Application Layer），是一个算法调度框架。它不实现具体的电池管理算法，而是提供一个统一的执行环境，使得各种算法（如 SOC 估算、SOE 估算、移动平均等）可以按各自的周期被调度执行。

### 2.2 工作模式

模块以固定 Tick（`ALGO_TICK_ms = 100ms`）周期运行，在每次 Tick 中检查所有已注册算法的周期是否已到，对周期已到的算法执行其计算函数。算法具有明确的状态机生命周期：未初始化 → 就绪 → 运行中 → 就绪（循环）。

### 2.3 用户特征

本模块的使用者是其他算法实现模块，它们通过注册回调函数（初始化函数和计算函数）到 `algo_algorithms[]` 数组中来加入调度框架。

## 3. 功能需求

### 3.1 算法状态机管理

#### REQ-001 — 算法状态定义

**编号** | **优先级** | **函数/结构体**
REQ-001 | 高 | `ALGO_STATE_e`

**描述**：系统应定义算法的六种运行状态：未初始化（`ALGO_UNINITIALIZED`）、就绪（`ALGO_READY`）、运行中（`ALGO_RUNNING`）、阻塞（`ALGO_BLOCKED`）、初始化失败（`ALGO_FAILED_INIT`）和重初始化请求（`ALGO_REINIT_REQUESTED`）。

**处理流程**：
1. 算法注册后默认处于 `ALGO_UNINITIALIZED` 状态
2. 初始化成功后切换到 `ALGO_READY`
3. 执行前切换到 `ALGO_RUNNING`
4. 执行完成后切换回 `ALGO_READY`
5. 执行超时切换到 `ALGO_BLOCKED`
6. 初始化失败切换到 `ALGO_FAILED_INIT`
7. 收到重初始化请求时切换到 `ALGO_REINIT_REQUESTED`

#### REQ-002 — 算法任务配置结构

**编号** | **优先级** | **结构体**
REQ-002 | 高 | `ALGO_TASKS_s`

**描述**：系统应定义算法任务配置结构体，包含以下字段：当前状态（`state`）、执行周期（`cycleTime_ms`）、最大允许执行时长（`maxCalculationDuration_ms`）、启动时间戳（`startTime`）、初始化回调函数指针（`fpInitialization`）和计算回调函数指针（`fpAlgorithm`）。

**前置条件**：每个算法注册时必须填写所有字段，其中 `state` 初始值为 `ALGO_UNINITIALIZED`。

### 3.2 算法初始化

#### REQ-003 — 延迟初始化触发

**编号** | **优先级** | **函数**
REQ-003 | 高 | `ALGO_UnlockInitialization()`

**描述**：系统应提供 `ALGO_UnlockInitialization()` 函数，在任务临界区内设置初始化请求标志。该函数可由外部模块调用以触发算法初始化流程。

**前置条件**：系统已进入可调度状态。
**错误处理**：使用 `OS_EnterTaskCritical()` / `OS_ExitTaskCritical()` 保护临界区操作。

#### REQ-004 — 算法初始化执行

**编号** | **优先级** | **函数**
REQ-004 | 高 | `ALGO_Initialization()`（静态函数）

**描述**：系统应遍历所有已注册算法，对处于 `ALGO_UNINITIALIZED` 状态的算法执行初始化。若算法的初始化函数指针为空（`NULL_PTR`），直接将其状态设为 `ALGO_READY`；否则调用初始化函数并根据返回值设置状态。

**处理流程**：
1. 遍历 `algo_algorithms[]` 数组（索引 0 到 `algo_length - 1`）
2. 通过断言验证每个算法的 `cycleTime_ms` 是 `ALGO_TICK_ms` 的整数倍
3. 跳过已初始化的算法
4. 对未初始化算法：若 `fpInitialization == NULL_PTR`，直接设为 `ALGO_READY`
5. 否则调用 `fpInitialization()`，若返回 `STD_OK` 则设为 `ALGO_READY`，否则设为 `ALGO_FAILED_INIT`

**前置条件**：`algo_initializationRequested == true`
**后置条件**：所有算法的状态不再是 `ALGO_UNINITIALIZED`（除非初始化失败）
**错误处理**：断言失败时系统停止；返回值非预期时设为 `ALGO_FAILED_INIT`

### 3.3 算法周期性调度

#### REQ-005 — 主调度函数

**编号** | **优先级** | **函数**
REQ-005 | 高 | `ALGO_MainFunction()`

**描述**：系统应在每次 Tick 中检查初始化请求标志，若已请求则执行初始化流程。随后遍历所有算法，对满足执行条件的算法调用其计算函数。

**处理流程**：
1. 在临界区内读取初始化请求标志
2. 若标志为 `true`，调用 `ALGO_Initialization()` 并将标志重置为 `false`
3. 使用静态计数器 `counter_ticks` 累加 `ALGO_TICK_ms`
4. 遍历所有算法，判断是否需要执行：
   - `cycleTime_ms == 0` 时立即执行（ASAP 模式）
   - `cycleTime_ms != 0` 且 `counter_ticks % cycleTime_ms == 0` 时执行
5. 对满足条件的算法：若状态为 `ALGO_READY`，设为 `ALGO_RUNNING`，记录启动时间，调用 `fpAlgorithm()`，然后调用 `ALGO_MarkAsDone()`
6. 若状态为 `ALGO_REINIT_REQUESTED`，设为 `ALGO_UNINITIALIZED` 并触发初始化

**前置条件**：`algo_algorithms[]` 已完成配置
**后置条件**：所有到期算法已执行，计数器已累加

#### REQ-006 — 算法周期校验

**编号** | **优先级** | **函数**
REQ-006 | 高 | `ALGO_Initialization()`

**描述**：系统应在初始化阶段通过断言验证每个算法的 `cycleTime_ms` 是 `ALGO_TICK_ms` 的整数倍，确保调度时能精确对齐。

**处理流程**：`FAS_ASSERT((algo_algorithms[i].cycleTime_ms % ALGO_TICK_ms) == 0u)`

### 3.4 执行时间监控

#### REQ-007 — 执行时间监控

**编号** | **优先级** | **函数**
REQ-007 | 高 | `ALGO_MonitorExecutionTime()`

**描述**：系统应提供 `ALGO_MonitorExecutionTime()` 函数，检查所有处于 `ALGO_RUNNING` 状态的算法是否超出其最大允许执行时长。若超时，将该算法状态设为 `ALGO_BLOCKED`，阻止其继续执行。

**处理流程**：
1. 获取当前时间戳
2. 遍历所有算法
3. 对满足以下全部条件的算法判定为超时：`startTime != 0`、状态为 `ALGO_RUNNING`、`startTime + maxCalculationDuration_ms < 当前时间戳`
4. 超时算法状态设为 `ALGO_BLOCKED`

**后置条件**：超时算法被阻塞，不再参与后续调度
**错误处理**：超时后算法被阻塞，TODO：需添加诊断调用通知错误

### 3.5 算法状态切换

#### REQ-008 — 算法执行完成标记

**编号** | **优先级** | **函数**
REQ-008 | 中 | `ALGO_MarkAsDone(uint32_t algorithmIndex)`

**描述**：系统应在算法执行完成后将其状态从 `ALGO_RUNNING` 恢复为 `ALGO_READY`，使其可参与下一轮调度。若当前状态为 `ALGO_REINIT_REQUESTED`，不改变状态；若当前状态为 `ALGO_BLOCKED`，保持阻塞状态。

**处理流程**：
1. 断言 `algorithmIndex < algo_length`
2. 若状态为 `ALGO_REINIT_REQUESTED`，保持不变
3. 若状态非 `ALGO_BLOCKED`，设为 `ALGO_READY`
4. 否则保持不变

**前置条件**：`algorithmIndex` 有效

#### REQ-009 — 算法重初始化标记

**编号** | **优先级** | **函数**
REQ-009 | 中 | `ALGO_MarkAsReinit(uint32_t algorithmIndex)`

**描述**：系统应在任务临界区内将指定算法的状态设为 `ALGO_REINIT_REQUESTED`，使其在下一调度周期被重新初始化。

**处理流程**：
1. 断言 `algorithmIndex < algo_length`
2. 进入任务临界区
3. 将算法状态设为 `ALGO_REINIT_REQUESTED`
4. 退出任务临界区

**前置条件**：`algorithmIndex` 有效

## 4. 非功能需求

### 4.1 时序与性能

#### REQ-010 — 调度时间基准

**编号** | **优先级** | **宏定义**
REQ-010 | 高 | `ALGO_TICK_ms`

**描述**：系统应以 100ms 为调度 Tick 基准（`ALGO_TICK_ms = 100u`）。所有算法的执行周期必须为此基准的整数倍。

### 4.2 内存

#### REQ-011 — 静态内存分配

**编号** | **优先级**
REQ-011 | 中

**描述**：系统应使用编译期确定的静态内存分配。`algo_algorithms[]` 数组大小由注册的算法数量决定，`algo_length` 在编译期计算。初始化请求标志 `algo_initializationRequested` 为静态布尔变量。

### 4.3 鲁棒性

#### REQ-012 — 初始化失败处理

**编号** | **优先级** | **函数**
REQ-012 | 中 | `ALGO_Initialization()`

**描述**：若算法初始化函数返回非 `STD_OK` 值，系统应将算法状态设为 `ALGO_FAILED_INIT`，该算法不再参与后续调度执行。

#### REQ-013 — 执行超时保护

**编号** | **优先级** | **函数**
REQ-013 | 高 | `ALGO_MonitorExecutionTime()`

**描述**：系统应防止算法执行时间超过其声明的最大执行时长。超时算法被永久阻塞（设为 `ALGO_BLOCKED`），直到系统重启或收到重初始化请求。

#### REQ-014 — 临界区保护

**编号** | **优先级**
REQ-014 | 高

**描述**：系统应对初始化请求标志的读写操作使用任务临界区保护（`OS_EnterTaskCritical()` / `OS_ExitTaskCritical()`），防止与中断服务程序的竞态条件。

### 4.4 可配置性

#### REQ-015 — 算法注册机制

**编号** | **优先级** | **文件**
REQ-015 | 高 | `algorithm_cfg.c`

**描述**：系统应通过在 `algo_algorithms[]` 数组中添加条目来注册算法。每个条目需指定执行周期、最大执行时长、初始化函数指针和计算函数指针。数组长度 `algo_length` 自动计算。

### 4.5 可测试性

#### REQ-016 — 单元测试支持

**编号** | **优先级** | **函数**
REQ-016 | 低 | `TEST_ALGO_ResetInitializationRequest()`

**描述**：系统应在单元测试编译模式下（`UNITY_UNIT_TEST`）暴露 `TEST_ALGO_ResetInitializationRequest()` 函数，用于重置初始化请求标志，便于测试用例控制初始化流程。

## 5. 接口需求

### 5.1 公共 API

| 函数 | 用途 | 调用方 |
|------|------|--------|
| `ALGO_UnlockInitialization()` | 触发算法初始化 | 系统启动流程、外部触发 |
| `ALGO_MainFunction()` | 主调度函数 | OS 任务周期调用 |
| `ALGO_MonitorExecutionTime()` | 执行时间监控 | OS 任务周期调用 |
| `ALGO_MarkAsDone(uint32_t)` | 标记算法执行完成 | 算法实现内部 |
| `ALGO_MarkAsReinit(uint32_t)` | 请求算法重初始化 | 算法实现内部 |

### 5.2 依赖项

| 依赖模块 | 用途 |
|----------|------|
| `os.h` | 临界区保护（`OS_EnterTaskCritical`/`OS_ExitTaskCritical`）、时间戳获取（`OS_GetTickCount`） |
| `fassert.h` | 断言检查（`FAS_ASSERT`） |
| `fstd_types.h` | 标准返回类型（`STD_RETURN_TYPE_e`） |
| `moving_average.h` | 注册的第一个算法实例 |

### 5.3 调用方

- OS 任务调度器（周期性调用 `ALGO_MainFunction` 和 `ALGO_MonitorExecutionTime`）
- 系统初始化流程（调用 `ALGO_UnlockInitialization`）
- 各算法实现模块（通过回调函数注册）

## 6. 数据结构

### 6.1 算法状态枚举 `ALGO_STATE_e`

| 状态 | 值 | 说明 |
|------|-----|------|
| `ALGO_UNINITIALIZED` | 0 | 默认值，初始化尚未执行 |
| `ALGO_READY` | 1 | 算法就绪，可在下一 Tick 执行 |
| `ALGO_RUNNING` | 2 | 算法正在执行中 |
| `ALGO_BLOCKED` | 3 | 算法超出最大执行时长，已阻塞 |
| `ALGO_FAILED_INIT` | 4 | 初始化失败 |
| `ALGO_REINIT_REQUESTED` | 5 | 已请求重新初始化 |

### 6.2 算法任务结构体 `ALGO_TASKS_s`

| 字段 | 类型 | 说明 |
|------|------|------|
| `state` | `ALGO_STATE_e` | 当前执行状态 |
| `cycleTime_ms` | `uint32_t` | 算法执行周期（ms），0 表示 ASAP |
| `maxCalculationDuration_ms` | `uint32_t` | 最大允许执行时长（ms） |
| `startTime` | `uint32_t` | 算法启动执行时的时间戳 |
| `fpInitialization` | `ALGO_INITIALIZATION_FUNCTION_f *` | 初始化回调，可为 `NULL_PTR` |
| `fpAlgorithm` | `ALGO_COMPUTATION_FUNCTION_f *` | 计算回调函数 |

## 7. 状态机 / 控制流

### 7.1 算法生命周期状态图

```
UNINITIALIZED ──初始化成功──▶ READY ──周期到达──▶ RUNNING ──执行完成──▶ READY
      │                         ▲                      │
      │                         │                      │ 超时
      │                         │                      ▼
      ├──初始化失败──▶ FAILED_INIT                   BLOCKED
      │
      └── REINIT_REQUESTED ◀── 外部请求 ── (来自任何状态)
                │
                └── 下一 Tick 处理 ──▶ UNINITIALIZED (重新初始化)
```

### 7.2 主调度流程

```
ALGO_MainFunction() 被调用
    │
    ├── 检查初始化请求标志
    │   └── true → ALGO_Initialization() → 重置标志
    │
    ├── 遍历所有算法
    │   ├── 判断执行条件 (cycleTime == 0 或 cycleTime 周期已到)
    │   │   ├── 满足 → 状态为 ALGO_READY?
    │   │   │   ├── 是 → 设为 RUNNING，记录时间，执行 fpAlgorithm()
    │   │   │   │       └── ALGO_MarkAsDone() → 恢复为 READY
    │   │   │   └── 否 → 检查是否 REINIT_REQUESTED
    │   │   │           └── 是 → 设为 UNINITIALIZED，触发初始化
    │   │   └── 不满足 → 跳过
    │
    └── counter_ticks += ALGO_TICK_ms
```

## 8. 追溯矩阵

| 需求编号 | 需求描述 | 函数/结构体/宏 | 文件 |
|----------|---------|---------------|------|
| REQ-001 | 算法状态定义 | `ALGO_STATE_e` | [algorithm_cfg.h](config/algorithm_cfg.h) |
| REQ-002 | 算法任务配置结构 | `ALGO_TASKS_s` | [algorithm_cfg.h](config/algorithm_cfg.h) |
| REQ-003 | 延迟初始化触发 | `ALGO_UnlockInitialization()` | [algorithm.c](algorithm.c) |
| REQ-004 | 算法初始化执行 | `ALGO_Initialization()` | [algorithm.c](algorithm.c) |
| REQ-005 | 主调度函数 | `ALGO_MainFunction()` | [algorithm.c](algorithm.c) |
| REQ-006 | 算法周期校验 | `ALGO_Initialization()` | [algorithm.c](algorithm.c) |
| REQ-007 | 执行时间监控 | `ALGO_MonitorExecutionTime()` | [algorithm.c](algorithm.c) |
| REQ-008 | 算法执行完成标记 | `ALGO_MarkAsDone()` | [algorithm_cfg.c](config/algorithm_cfg.c) |
| REQ-009 | 算法重初始化标记 | `ALGO_MarkAsReinit()` | [algorithm_cfg.c](config/algorithm_cfg.c) |
| REQ-010 | 调度时间基准 | `ALGO_TICK_ms` | [algorithm_cfg.h](config/algorithm_cfg.h) |
| REQ-011 | 静态内存分配 | `algo_algorithms[]` | [algorithm_cfg.c](config/algorithm_cfg.c) |
| REQ-012 | 初始化失败处理 | `ALGO_Initialization()` | [algorithm.c](algorithm.c) |
| REQ-013 | 执行超时保护 | `ALGO_MonitorExecutionTime()` | [algorithm.c](algorithm.c) |
| REQ-014 | 临界区保护 | `ALGO_UnlockInitialization()`, `ALGO_MainFunction()`, `ALGO_MarkAsReinit()` | [algorithm.c](algorithm.c), [algorithm_cfg.c](config/algorithm_cfg.c) |
| REQ-015 | 算法注册机制 | `algo_algorithms[]` | [algorithm_cfg.c](config/algorithm_cfg.c) |
| REQ-016 | 单元测试支持 | `TEST_ALGO_ResetInitializationRequest()` | [algorithm.c](algorithm.c) |
