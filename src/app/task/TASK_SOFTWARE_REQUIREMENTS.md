# TASK 子系统 — 软件需求规格说明

## 1. 引言

### 1.1 目的

本文档描述 foxBMS 2 任务子系统（Task Subsystem）的软件需求规格。Task 子系统是 foxBMS 2 的实时操作系统（RTOS）抽象层和任务管理基础设施，负责管理系统所有任务的创建、调度、同步以及跨模块通信。

### 1.2 范围

**涵盖范围：**
- OS 抽象层（`os` 模块）—— 封装 FreeRTOS API，提供操作系统无关接口
- 任务管理（`ftask` 模块）—— 系统任务的创建与生命周期管理
- 任务配置（`config` 模块）—— 各任务的优先级、周期、栈大小及用户代码配置
- FreeRTOS 配置验证（`os_freertos_config-validation.h`）—— 编译期配置正确性校验
- 软件定时器封装（`timer` 模块）—— FreeRTOS 软件定时器的安全封装

**不涵盖范围：**
- BMS 应用层算法逻辑（由 `ftask_cfg.c` 中调用的各模块负责）
- FreeRTOS 内核本身的实现
- 硬件定时器驱动

### 1.3 定义与缩略语

| 术语 | 英文原文 | 说明 |
|------|---------|------|
| RTOS | Real-Time Operating System | 实时操作系统 |
| FreeRTOS | — | 本系统使用的开源 RTOS 内核 |
| 任务（Task） | — | RTOS 中独立的执行线程 |
| 队列（Queue） | — | 任务间通信的 FIFO 消息通道 |
| 信号量（Semaphore） | — | 任务间同步机制 |
| 临界区（Critical Section） | — | 不被中断打断的代码段 |
| 栈溢出（Stack Overflow） | — | 任务栈空间不足导致的越界写入 |
| 相位（Phase） | — | 任务从调度器启动到首次执行之间的延迟时间 |
| ISR | Interrupt Service Routine | 中断服务例程 |
| FPU | Floating Point Unit | 浮点运算单元 |
| 崩溃转储（Crash Dump） | — | 通过 CAN 总线发送的致命错误信息 |

### 1.4 参考文献

- FreeRTOS API Reference Manual
- foxBMS 2 系统架构文档
- MISRA C:2012 编码规范

---

## 2. 总体描述

### 2.1 产品视角

Task 子系统位于 foxBMS 2 软件架构的应用层（APP 层），是整个系统的任务调度基础设施。它封装了 FreeRTOS 内核 API，向上层 BMS 应用模块提供：

- 统一的操作系统接口（`OS_*` 前缀函数）
- 预定义的系统任务框架（Engine、Cyclic 1ms/10ms/100ms、Algorithm 100ms、I2C、AFE、UART、EMAC）
- 标准化的任务间通信机制（队列、通知、信号量）
- 软件定时器管理

```
┌─────────────────────────────────────────────────┐
│          BMS 应用层 (algorithm, bal, bms, ...)    │
├─────────────────────────────────────────────────┤
│          Task 子系统 (本模块)                      │
│  ┌──────────┐ ┌────────┐ ┌───────┐ ┌──────────┐ │
│  │ os (抽象) │ │ ftask  │ │config │ │  timer   │ │
│  └─────┬────┘ └───┬────┘ └───┬───┘ └────┬─────┘ │
├────────┼──────────┼──────────┼───────────┼───────┤
│  FreeRTOS 内核 (第三方)                         │
└─────────────────────────────────────────────────┘
```

### 2.2 工作模式

系统启动后按以下顺序进入各状态：

1. **OS_OFF** → 系统初始上电
2. **OS_INITIALIZE_SCHEDULER** → 调度器初始化（含缓存控制）
3. **OS_CREATE_QUEUES** → 创建所有任务间通信队列
4. **OS_CREATE_TASKS** → 创建所有 FreeRTOS 任务
5. **OS_INIT_PRE_OS** → 任务创建完毕，等待调度器启动
6. **OS_SCHEDULER_RUNNING** → 调度器启动，Engine 任务开始执行
7. **OS_ENGINE_RUNNING** → Engine 任务完成数据库初始化
8. **OS_PRE_CYCLIC_INITIALIZATION_HAS_FINISHED** → 预循环初始化完成
9. **OS_SYSTEM_RUNNING** → 系统全功能运行（Algorithm 100ms 任务启动后）

### 2.3 用户特征

本子系统面向嵌入式 BMS 固件开发者。用户需要了解 FreeRTOS 任务模型、BMS 系统模块划分以及配置宏系统。

---

## 3. 功能需求

### 3.1 OS 抽象层 — 系统初始化与调度

#### REQ-001 — 操作系统初始化

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-001` | 必须 | `OS_InitializeOperatingSystem()`, `OS_InitializeScheduler()`, `OS_StartScheduler()`

**描述**：系统应提供统一的操作系统初始化流程，依次完成调度器初始化、队列创建、任务创建，并最终启动调度器。调度器启动后不应返回。

**处理流程**：
1. 将系统启动状态置为 `OS_INITIALIZE_SCHEDULER`
2. 调用 `OS_InitializeScheduler()` 根据配置启用/禁用缓存
3. 将系统启动状态置为 `OS_CREATE_QUEUES`
4. 调用 `FTSK_CreateQueues()` 创建所有队列
5. 将系统启动状态置为 `OS_CREATE_TASKS`
6. 调用 `FTSK_CreateTasks()` 创建所有任务
7. 将系统启动状态置为 `OS_INIT_PRE_OS`
8. 调用 `OS_StartScheduler()` 启动调度器

**错误处理**：调度器启动后若意外返回，系统应触发 `FAS_ASSERT(FAS_TRAP)` 进入陷阱状态。

---

### 3.2 OS 抽象层 — 启动状态管理

#### REQ-002 — 系统启动状态跟踪

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-002` | 必须 | `OS_BOOT_STATE_e`, `os_boot`, `os_schedulerStartTime`

**描述**：系统应维护一个全局启动状态枚举（`os_boot`），记录从关机到全功能运行的完整启动过程。各任务在启动时应检查此状态以确定自身是否满足运行条件。

**状态定义**：
- `OS_OFF` → `OS_INITIALIZE_SCHEDULER` → `OS_CREATE_QUEUES` → `OS_CREATE_TASKS` → `OS_INIT_PRE_OS` → `OS_SCHEDULER_RUNNING` → `OS_ENGINE_RUNNING` → `OS_PRE_CYCLIC_INITIALIZATION_HAS_FINISHED` → `OS_SYSTEM_RUNNING`
- 错误状态：`OS_INIT_OS_FATALERROR_SCHEDULER`, `OS_INIT_OS_FATALERROR`

**前置条件**：`os_boot` 变量声明为 `volatile`，确保跨任务可见性。

---

### 3.3 OS 抽象层 — 系统计时器

#### REQ-003 — 系统运行时间计数器

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-003` | 必须 | `OS_IncrementTimer()`, `OS_TIMER_s`, `os_timer`

**描述**：系统应维护一个多级运行时间计数器（`OS_TIMER_s`），通过逐级进位记录毫秒到天级别的时间。计数器应在 1ms 周期性任务中递增。

**计数器结构**：
- `timer_1ms` (uint8_t, 0~9) → 每毫秒递增，到 10 进位
- `timer_10ms` (uint8_t, 0~9) → 每 10ms 递增，到 10 进位
- `timer_100ms` (uint8_t, 0~9) → 每 100ms 递增，到 10 进位
- `timer_sec` (uint8_t, 0~59) → 每秒递增，到 60 进位
- `timer_min` (uint8_t, 0~59) → 每分递增，到 60 进位
- `timer_h` (uint8_t, 0~23) → 每小时递增，到 24 进位
- `timer_d` (uint16_t) → 每天递增

**前置条件**：此函数必须在 1ms 周期性任务中调用，配合 `RTC_IncrementSystemTime()` 使用。

---

### 3.4 OS 抽象层 — 时间流逝检查

#### REQ-004 — 时间流逝判定

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-004` | 必须 | `OS_CheckTimeHasPassed()`, `OS_CheckTimeHasPassedWithTimestamp()`

**描述**：系统应提供两种时间流逝判定函数，用于检查自某个时间戳以来是否已过指定毫秒数。两种函数分别支持"以当前时间为参考"和"以指定时间为参考"的判定模式。

**处理逻辑**（`OS_CheckTimeHasPassedWithTimestamp`）：
1. 若 `timeToPass_ms == 0`，立即返回 `true`
2. 计算 `timeDifference_ms = currentTimeStamp_ms - oldTimeStamp_ms`
3. 若 `timeDifference_ms == 0` 且 `timeToPass_ms > 0`，返回 `false`
4. 若 `timeDifference_ms >= timeToPass_ms`，返回 `true`
5. 否则返回 `false`

**输入**：

| 参数 | 类型 | 说明 |
|------|------|------|
| `oldTimeStamp_ms` | `uint32_t` | 起始时间戳 (ms) |
| `currentTimeStamp_ms` | `uint32_t` | 当前时间戳 (ms)，仅 `WithTimestamp` 版本 |
| `timeToPass_ms` | `uint32_t` | 需要经过的时间 (ms) |

**注意**：支持 uint32 时间戳回绕（溢出后的正确比较）。

---

#### REQ-005 — 时间流逝检查自检

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-005` | 必须 | `OS_CheckTimeHasPassedSelfTest()`

**描述**：系统应提供时间流逝判定函数的自检功能，通过预定义测试用例验证判定逻辑的正确性。

**测试用例**：
1. `(0u, 0u, 0u) → true` — 零时间总是已过
2. `(0u, 1u, 1u) → true` — 正好经过 1ms
3. `(1u, 2u, 2u) → false` — 经过 1ms 不满足 2ms 要求
4. `(0u, 1u, 0u) → true` — 要求 0ms 总是已过
5. `(1u, 0u, 1u) → true` — 时间戳回绕场景

---

### 3.5 OS 抽象层 — 任务同步原语

#### REQ-006 — 系统节拍与任务延迟

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-006` | 必须 | `OS_GetTickCount()`, `OS_DelayTask()`, `OS_DelayTaskUntil()`

**描述**：系统应提供系统节拍计数值获取、绝对延迟和相对延迟三种时间控制接口。

- `OS_GetTickCount()`：返回当前 FreeRTOS 系统节拍数
- `OS_DelayTask(milliseconds)`：将调用任务阻塞指定毫秒数（参数必须 > 0）
- `OS_DelayTaskUntil(pPreviousWakeTime, milliseconds)`：将调用任务阻塞到指定绝对时间点（最小延迟 1 个 tick）

---

#### REQ-007 — 临界区管理

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-007` | 必须 | `OS_EnterTaskCritical()`, `OS_ExitTaskCritical()`

**描述**：系统应提供进入和退出临界区的接口，直接封装 FreeRTOS 的 `taskENTER_CRITICAL()` 和 `taskEXIT_CRITICAL()` 宏。

---

#### REQ-008 — 任务通知

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-008` | 必须 | `OS_WaitForNotification()`, `OS_NotifyFromIsr()`, `OS_NotifyGive()`, `OS_NotifyGiveFromIsr()`, `OS_NotifyTake()`

**描述**：系统应封装 FreeRTOS 任务通知 API，提供以下功能：

- `OS_WaitForNotification()` — 阻塞等待任务通知（任务上下文调用）
- `OS_NotifyFromIsr()` — 从 ISR 发送通知（覆写模式），并在 ISR 末尾触发调度
- `OS_NotifyGive()` — 发送计数型通知
- `OS_NotifyGiveFromIsr()` — 从 ISR 发送计数型通知
- `OS_NotifyTake()` — 获取计数型通知（可选退出时清零）

**前置条件**：等待函数不可在 ISR 中调用；通知发送函数不可在任务上下文中调用（FromIsr 版本例外）。

---

#### REQ-009 — 索引化任务通知

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-009` | 必须 | `OS_WaitForNotificationIndexed()`, `OS_NotifyIndexedFromIsr()`, `OS_ClearNotificationIndexed()`

**描述**：系统应支持多索引任务通知，允许同一个任务在多个索引上独立等待/发送/清除通知。

---

#### REQ-010 — 队列管理

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-010` | 必须 | `OS_ReceiveFromQueue()`, `OS_SendToBackOfQueue()`, `OS_SendToBackOfQueueFromIsr()`, `OS_GetNumberOfStoredMessagesInQueue()`

**描述**：系统应封装 FreeRTOS 队列 API，提供统一的队列接收、发送（尾部）、ISR 发送和队列消息计数功能。所有函数返回 `OS_SUCCESS`/`OS_FAIL`。

---

#### REQ-011 — 任务挂起与恢复

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-011` | 必须 | `OS_SuspendTask()`, `OS_ResumeTask()`

**描述**：系统应提供任务挂起和恢复接口。传入 `NULL` 句柄将操作调用任务自身。恢复时需确保句柄非空。

---

#### REQ-012 — 信号量管理

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-012` | 必须 | `OS_SemaphoreGive()`, `OS_SemaphoreGiveFromIsr()`, `OS_SemaphoreTake()`

**描述**：系统应封装 FreeRTOS 信号量 API，提供信号量释放（give）、ISR 释放和获取（take）功能。获取信号量时支持超时等待。

---

#### REQ-013 — FPU 上下文管理

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-013` | 必须 | `OS_MarkTaskAsRequiringFpuContext()`

**描述**：使用浮点运算的任务必须在启动时调用此函数，指示操作系统在任务切换时保存/恢复 FPU 寄存器上下文。

**前置条件**：必须在任务函数内部调用。

---

### 3.6 OS 抽象层 — FreeRTOS 配置验证

#### REQ-014 — FreeRTOS 配置编译期验证

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-014` | 必须 | `os_freertos_config-validation.h`

**描述**：系统应在编译期验证 FreeRTOS 配置的正确性，包括：

1. **必选 FreeRTOS 功能**：若未定义 `INCLUDE_vTaskDelayUntil` 或 `INCLUDE_xTaskGetSchedulerState`，触发编译错误
2. **任务栈大小下限检查**：Engine、Cyclic 1ms/10ms/100ms、Algorithm 100ms 各任务的栈大小必须大于 `configMINIMAL_STACK_SIZE * GEN_BYTES_PER_WORD`
3. **类型大小假设**：编译期断言 `sizeof(StackType_t) == sizeof(uint32_t)`

---

### 3.7 FreeRTOS 钩子函数

#### REQ-015 — 空闲任务钩子

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-015` | 必须 | `vApplicationIdleHook()`, `FTSK_RunUserCodeIdle()`

**描述**：系统应实现 FreeRTOS 空闲任务钩子，在系统空闲时调用 `FTSK_RunUserCodeIdle()` 执行用户定义的空闲代码。

---

#### REQ-016 — 空闲任务和定时器任务内存配置

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-016` | 条件（静态分配） | `vApplicationGetIdleTaskMemory()`, `vApplicationGetTimerTaskMemory()`

**描述**：当启用静态内存分配时，系统应提供空闲任务和定时器任务的内存缓冲区（TCB 和栈空间）。

**前置条件**：`configSUPPORT_STATIC_ALLOCATION == 1`；定时器任务内存仅当同时满足 `configUSE_TIMERS > 0` 时提供。

---

#### REQ-017 — 栈溢出检测与崩溃转储

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-017` | 条件（栈检测开启） | `vApplicationStackOverflowHook()`, `CANTX_CrashDump()`

**描述**：当 FreeRTOS 检测到栈溢出时，系统应尝试通过 CAN 总线向外发送崩溃转储消息（`CANTX_FATAL_ERRORS_ACTIONS_STACK_OVERFLOW`），随后触发硬件陷阱。

**前置条件**：`configCHECK_FOR_STACK_OVERFLOW > 0`

---

### 3.8 任务管理 — 队列

#### REQ-020 — 系统队列创建

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-020` | 必须 | `FTSK_CreateQueues()`, `ftask_allQueuesCreated`

**描述**：系统应在调度器启动前创建所有任务间通信队列。队列使用 FreeRTOS 静态分配，创建完成后应以临界区保护的方式将 `ftask_allQueuesCreated` 标志置为 `true`。

**创建的队列及规格**：

| 队列名称 | 长度 | 元素大小 | 用途 |
|---------|------|---------|------|
| Database Queue | 1 | `sizeof(DATA_QUEUE_MESSAGE_s)` | 数据库消息传递 |
| IMD CAN Data Queue | 5 | `sizeof(CAN_BUFFER_ELEMENT_s)` | 绝缘监测设备 CAN 消息 |
| CAN Receive Queue | 50 | `sizeof(CAN_BUFFER_ELEMENT_s)` | CAN Rx 消息缓冲 |
| CAN Tx Unsent Messages Queue | 5 | `sizeof(CAN_BUFFER_ELEMENT_s)` | 未发送 CAN 消息跟踪 |
| AFE Request Queue | 1 | `sizeof(AFE_REQUEST_e)` | AFE 请求 |
| RTC set time Queue | 1 | `sizeof(RTC_TIME_DATA_s)` | RTC 时间设置 |
| AFE to I2C Queue | 1 | `sizeof(AFE_I2C_QUEUE_s)` | I2C 发送 |
| AFE from I2C Queue | 1 | `sizeof(AFE_I2C_QUEUE_s)` | I2C 接收 |
| CAN→AFE Cell Temperatures Queue | 5 | — | CAN 调试：电芯温度 (条件编译) |
| CAN→AFE Cell Voltages Queue | 5 | — | CAN 调试：电芯电压 (条件编译) |
| UART Receive Queue | 1024 | `sizeof(uint8_t)` | UART 接收缓冲 (条件编译) |

**后置条件**：每个队列创建后应调用 `FAS_ASSERT` 验证非空，并注册到 FreeRTOS 队列注册表。

---

### 3.9 任务管理 — 任务创建

#### REQ-021 — 系统任务创建

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-021` | 必须 | `FTSK_CreateTasks()`

**描述**：系统应在调度器启动前使用 FreeRTOS 静态分配 API 创建所有系统任务。每个任务创建后应验证其句柄非空。

**创建的任务**：

| 任务名称 | 入口函数 | 栈配置宏 |
|---------|---------|---------|
| TaskEngine | `FTSK_CreateTaskEngine` | `FTSK_TASK_ENGINE_STACK_SIZE_IN_BYTES` |
| TaskCyclic1ms | `FTSK_CreateTaskCyclic1ms` | `FTSK_TASK_CYCLIC_1MS_STACK_SIZE_IN_BYTES` |
| TaskCyclic10ms | `FTSK_CreateTaskCyclic10ms` | `FTSK_TASK_CYCLIC_10MS_STACK_SIZE_IN_BYTES` |
| TaskCyclic100ms | `FTSK_CreateTaskCyclic100ms` | `FTSK_TASK_CYCLIC_100MS_STACK_SIZE_IN_BYTES` |
| TaskCyclicAlgorithm100ms | `FTSK_CreateTaskCyclicAlgorithm100ms` | `FTSK_TASK_CYCLIC_ALGORITHM_100MS_STACK_SIZE_IN_BYTES` |
| TaskI2c | `FTSK_CreateTaskI2c` | `FTSK_TASK_I2C_STACK_SIZE_IN_BYTES` |
| TaskAfe | `FTSK_CreateTaskAfe` (条件) | `FTSK_TASK_AFE_STACK_SIZE_IN_BYTES` |
| TaskUart | `FTSK_CreateTaskUart` (条件) | `FTSK_TASK_UART_STACK_SIZE_IN_BYTES` |
| TaskEmac | `FTSK_CreateTaskEmac` (条件) | `FTSK_TASK_EMAC_STACK_SIZE_IN_BYTES` |

**后置条件**：栈大小从字节转换为 FreeRTOS 所需的字数（`FTSK_BYTES_TO_WORDS` 宏）。

---

### 3.10 任务管理 — 各任务行为

#### REQ-022 — Engine 任务

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-022` | 必须 | `FTSK_CreateTaskEngine()`, `FTSK_InitializeUserCodeEngine()`, `FTSK_RunUserCodeEngine()`

**描述**：Engine 任务应为全网最高优先级任务（`OS_PRIORITY_REAL_TIME`），负责：
1. 标记 FPU 上下文
2. 设置启动状态为 `OS_SCHEDULER_RUNNING`
3. 执行用户代码初始化（数据库、FRAM、系统监控）
4. 设置启动状态为 `OS_ENGINE_RUNNING`
5. 进入无限循环，每个周期：
   - 通知系统监控进入（`SYSM_NOTIFY_ENTER`）
   - 调用 `DATA_Task()` 处理数据库消息
   - 调用 `SYSM_CheckNotifications()` 检查任务通知
   - 通知系统监控退出（`SYSM_NOTIFY_EXIT`）

**前置条件**：`pvParameters` 必须为 `NULL_PTR`。

---

#### REQ-023 — Cyclic 1ms 任务

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-023` | 必须 | `FTSK_CreateTaskCyclic1ms()`, `FTSK_RunUserCodeCyclic1ms()`

**描述**：Cyclic 1ms 任务应以 1ms 周期间隔执行（优先级 `OS_PRIORITY_VERY_HIGH`），负责：
1. 等待 Engine 任务就绪（`os_boot == OS_ENGINE_RUNNING`）
2. 执行预循环初始化
3. 设置启动状态为 `OS_PRE_CYCLIC_INITIALIZATION_HAS_FINISHED`
4. 进入无限循环，每个周期：
   - 通知系统监控
   - 调用 `OS_IncrementTimer()` 更新系统时间
   - 调用 `DIAG_UpdateFlags()` 更新诊断标志
   - （条件编译）调用 `MEAS_Control()` 执行 FSM 型 AFE 测量控制
   - 调用 `CAN_ReadRxBuffer()` 读取 CAN Rx 缓冲区
   - 使用 `OS_DelayTaskUntil` 延迟到下一周期（1ms）

**后置条件**：此任务的完成标志着预循环初始化阶段的结束，10ms/100ms 等任务在此之后才能开始运行。

---

#### REQ-024 — Cyclic 10ms 任务

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-024` | 必须 | `FTSK_CreateTaskCyclic10ms()`, `FTSK_RunUserCodeCyclic10ms()`

**描述**：Cyclic 10ms 任务应以 10ms 周期间隔执行（优先级 `OS_PRIORITY_HIGH`，相位 2ms）。每个周期调用：`SYSM_UpdateFramData()`, `SYS_Trigger()`, `ILCK_Trigger()`, `ADC_Control()`, `SPS_Ctrl()`, `CAN_MainFunction()`, `SOF_Calculation()`, `ALGO_MonitorExecutionTime()`, `SBC_Trigger()`, `BMS_Trigger()`。每 5 个周期（50ms）额外执行 `MRC_ValidateAfeMeasurement()` 和 `MRC_ValidatePackMeasurement()`。

**前置条件**：任务启动前必须等待 `os_boot == OS_PRE_CYCLIC_INITIALIZATION_HAS_FINISHED`。

---

#### REQ-025 — Cyclic 100ms 任务

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-025` | 必须 | `FTSK_CreateTaskCyclic100ms()`, `FTSK_RunUserCodeCyclic100ms()`

**描述**：Cyclic 100ms 任务应以 100ms 周期间隔执行（优先级 `OS_PRIORITY_ABOVE_NORMAL`，相位 56ms）。每个周期调用：`BAL_Trigger()`, `IMD_Trigger()`, `LED_Trigger()`, `MINFO_CheckSupplyVoltageClamp30c()`。每 10 个周期（1s）额外执行 `SE_RunStateEstimations()`。

---

#### REQ-026 — Cyclic Algorithm 100ms 任务

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-026` | 必须 | `FTSK_CreateTaskCyclicAlgorithm100ms()`, `FTSK_RunUserCodeCyclicAlgorithm100ms()`

**描述**：Cyclic Algorithm 100ms 任务应以 100ms 周期间隔执行（优先级 `OS_PRIORITY_NORMAL`，相位 64ms）。每个周期调用 `ALGO_MainFunction()`。此任务启动后将 `os_boot` 设置为 `OS_SYSTEM_RUNNING`，标志系统全功能就绪。

---

#### REQ-027 — 连续运行型任务（I2C/AFE/UART/EMAC）

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-027` | 必须 | `FTSK_CreateTaskI2c()`, `FTSK_CreateTaskAfe()`, `FTSK_CreateTaskUart()`, `FTSK_CreateTaskEmac()`

**描述**：I2C、AFE、UART、EMAC 任务应以非固定周期的连续运行模式执行：

- **I2C 任务**（优先级 `OS_PRIORITY_HIGH`）：每周期调用 `PEX_Trigger()`, `HTSEN_Trigger()`, `RTC_Trigger()`，随后延迟 2ms
- **AFE 任务**（条件编译，优先级 `OS_PRIORITY_ABOVE_HIGH`）：每周期调用 `MEAS_Control()`
- **UART 任务**（条件编译，优先级 `OS_PRIORITY_NORMAL`）：处理 UART 流控制，等待通知
- **EMAC 任务**（条件编译，优先级 `OS_PRIORITY_ABOVE_NORMAL`）：接收以太网数据包

**前置条件**：所有连续运行型任务在启动前必须等待 `os_boot == OS_PRE_CYCLIC_INITIALIZATION_HAS_FINISHED`。

---

### 3.11 任务配置

#### REQ-028 — 任务参数化配置

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-028` | 必须 | `ftask_cfg.h`, `ftask_cfg.c`, `OS_TASK_DEFINITION_s`

**描述**：系统应通过编译期宏定义和运行时结构体的方式集中配置每个系统任务的参数。每个任务定义包括：优先级（`OS_PRIORITY_e`）、相位（ms）、周期时间（ms）、栈大小（字节）、任务参数指针。

**默认配置表**：

| 任务 | 优先级 | 相位(ms) | 周期(ms) | 栈大小(byte) |
|------|--------|---------|---------|-------------|
| Engine | REALTIME | 0 | 1 | 1024 |
| Cyclic 1ms | VERY_HIGH | 0 | 1 | 1024 |
| Cyclic 10ms | HIGH | 2 | 10 | 5120 |
| Cyclic 100ms | ABOVE_NORMAL | 56 | 100 | 1024 |
| Cyclic Algorithm 100ms | NORMAL | 64 | 100 | 1024 |
| I2C | HIGH | 0 | 0 (连续) | 2048 |
| AFE | ABOVE_HIGH | 0 | 0 (连续) | 4096 |
| UART | NORMAL | 0 | 0 (连续) | 1024 |
| EMAC | ABOVE_NORMAL | 10 | 0 (连续) | 2048 |

**非功能约束**：Engine 任务配置不可修改，否则将破坏系统启动序列。

---

### 3.12 软件定时器封装

#### REQ-029 — 定时器创建

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-029` | 必须 | `TIMER_Create()`

**描述**：系统应提供 FreeRTOS 软件定时器创建封装（使用静态分配 `xTimerCreateStatic`），参数包括定时器名称、周期（ms）、自动重载标志、定时器 ID、回调函数、内存缓冲区。创建前应断言所有必需参数非空。

---

#### REQ-030 — 定时器生命周期管理

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-030` | 必须 | `TIMER_Start()`, `TIMER_Stop()`, `TIMER_Reset()`, `TIMER_Delete()`

**描述**：系统应提供定时器启动、停止、重置和删除的封装接口。所有操作在定时器句柄非空时执行，返回 `STD_OK`/`STD_NOT_OK`。若定时器命令队列满，可阻塞等待指定 tick 数。

---

### 3.13 内存管理钩子

#### REQ-031 — malloc 失败钩子

**编号** | **优先级** | **代码映射**
---|---|---
`REQ-031` | 必须 | `vApplicationMallocFailedHook()`

**描述**：系统应实现 FreeRTOS malloc 失败钩子，当内存分配失败时触发 `FAS_ASSERT(false)` 进入陷阱。此钩子对 TCP 协议栈的 malloc 调用尤为重要。

---

## 4. 非功能需求

### 4.1 时序与性能

| 编号 | 需求 |
|------|------|
| `NFR-001` | 1ms 任务的最大 jitter 不超过 1ms |
| `NFR-002` | 10ms 任务的最大 jitter 不超过 2ms |
| `NFR-003` | 100ms 任务（常规和算法）的最大 jitter 不超过 5ms |
| `NFR-004` | 调度器启动后 Engine 任务作为第一个用户任务立即执行 |
| `NFR-005` | Engine 任务必须具有全网最高优先级以确保数据库处理不被延迟 |

### 4.2 内存

| 编号 | 需求 |
|------|------|
| `NFR-006` | 所有队列和任务 TCB/栈必须使用静态内存分配（`xQueueCreateStatic`/`xTaskCreateStatic`），禁止动态内存分配 |
| `NFR-007` | 各任务栈大小必须大于 FreeRTOS 最小栈大小要求，此约束在编译期验证 |
| `NFR-008` | `StackType_t` 的大小假定为 4 字节（`uint32_t`），此假设在编译期验证 |

### 4.3 鲁棒性

| 编号 | 需求 |
|------|------|
| `NFR-009` | 所有函数参数在解引用前必须通过 `FAS_ASSERT` 进行非空检查 |
| `NFR-010` | 所有队列创建后必须断言非空 |
| `NFR-011` | 所有任务创建后必须断言句柄非空 |
| `NFR-012` | 调度器启动后若意外返回，必须触发 `FAS_ASSERT(FAS_TRAP)` |
| `NFR-013` | 栈溢出检测触发后，必须尝试通过 CAN 发送崩溃转储消息，然后进入陷阱 |
| `NFR-014` | malloc 失败时，必须触发断言陷阱 |
| `NFR-015` | Engine 任务初始化过程中任何步骤失败均应触发陷阱 |

### 4.4 可配置性

| 编号 | 需求 |
|------|------|
| `NFR-016` | 支持通过 `foxbms_config.h` 中的编译宏条件编译可选特性（`FOXBMS_AFE_DRIVER_TYPE_NO_FSM`, `FOXBMS_UART_SUPPORT`, `FOXBMS_TCP_SUPPORT`, `FOXBMS_AFE_DRIVER_DEBUG_CAN`） |
| `NFR-017` | 支持通过 `FOXBMS_RTOS_FREERTOS` 宏选择 RTOS 实现 |
| `NFR-018` | 支持通过 `OS_ENABLE_CACHE` 宏控制指令/数据缓存的启用 |
| `NFR-019` | FreeRTOS 软件定时器支持通过 `configUSE_TIMERS` 和 `configSUPPORT_STATIC_ALLOCATION` 条件编译 |
| `NFR-020` | 栈溢出检测通过 `configCHECK_FOR_STACK_OVERFLOW` 控制 |

### 4.5 可测试性

| 编号 | 需求 |
|------|------|
| `NFR-021` | 在 `UNITY_UNIT_TEST` 宏定义下，应暴露内部静态变量（如 `os_timer`）的访问函数供单元测试使用 |
| `NFR-022` | 在 `UNITY_UNIT_TEST` 宏定义下，FreeRTOS 钩子函数应使用不同的符号名避免与 mock 冲突 |
| `NFR-023` | `OS_CheckTimeHasPassedSelfTest()` 应为无副作用函数，可在系统运行时任意时刻调用以验证时间判定逻辑 |

### 4.6 编码规范

| 编号 | 需求 |
|------|------|
| `NFR-024` | 所有代码应遵循 MISRA C:2012 规范，使用 `AXIVION` 注解标注已知偏差 |
| `NFR-025` | 公共函数使用 `OS_`, `FTSK_`, `TIMER_` 前缀命名；FreeRTOS 原生钩子保留 FreeRTOS 命名约定 |

---

## 5. 接口需求

### 5.1 OS 抽象层公共 API（`os.h`）

| 函数 | 签名 | 用途 |
|------|------|------|
| `OS_InitializeScheduler` | `void → void` | 初始化调度器（缓存控制） |
| `OS_StartScheduler` | `void → void` | 启动调度器 |
| `OS_InitializeOperatingSystem` | `void → void` | 完整操作系统初始化 |
| `OS_IncrementTimer` | `void → void` | 递增系统时间计数器 |
| `OS_GetTickCount` | `void → uint32_t` | 获取系统节拍 |
| `OS_DelayTask` | `uint32_t → void` | 相对延迟 |
| `OS_DelayTaskUntil` | `uint32_t*, uint32_t → void` | 绝对延迟 |
| `OS_EnterTaskCritical` | `void → void` | 进入临界区 |
| `OS_ExitTaskCritical` | `void → void` | 退出临界区 |
| `OS_MarkTaskAsRequiringFpuContext` | `void → void` | 标记 FPU 上下文 |
| `OS_WaitForNotification` | `uint32_t*, uint32_t → OS_STD_RETURN_e` | 等待通知 |
| `OS_NotifyFromIsr` | `TaskHandle_t, uint32_t → OS_STD_RETURN_e` | 从 ISR 发送通知 |
| `OS_WaitForNotificationIndexed` | `uint32_t, uint32_t*, uint32_t → OS_STD_RETURN_e` | 索引化等待通知 |
| `OS_NotifyIndexedFromIsr` | `TaskHandle_t, uint32_t, uint32_t → OS_STD_RETURN_e` | 从 ISR 发送索引化通知 |
| `OS_ClearNotificationIndexed` | `uint32_t → OS_STD_RETURN_e` | 清除索引化通知 |
| `OS_ReceiveFromQueue` | `OS_QUEUE, void*, uint32_t → OS_STD_RETURN_e` | 从队列接收 |
| `OS_SendToBackOfQueue` | `OS_QUEUE, const void*, uint32_t → OS_STD_RETURN_e` | 发送到队列尾部 |
| `OS_SendToBackOfQueueFromIsr` | `OS_QUEUE, const void*, long* → OS_STD_RETURN_e` | 从 ISR 发送到队列 |
| `OS_SuspendTask` | `TaskHandle_t → void` | 挂起任务 |
| `OS_ResumeTask` | `TaskHandle_t → void` | 恢复任务 |
| `OS_GetNumberOfStoredMessagesInQueue` | `OS_QUEUE → uint32_t` | 获取队列消息数 |
| `OS_CheckTimeHasPassed` | `uint32_t, uint32_t → bool` | 检查时间流逝（当前时间） |
| `OS_CheckTimeHasPassedWithTimestamp` | `uint32_t, uint32_t, uint32_t → bool` | 检查时间流逝（指定时间戳） |
| `OS_CheckTimeHasPassedSelfTest` | `void → STD_RETURN_TYPE_e` | 时间检查自检 |
| `OS_GetOsTimer` | `void → OS_TIMER_s` | 获取系统计时器值 |
| `OS_NotifyGive` | `TaskHandle_t → uint32_t` | 发送计数型通知 |
| `OS_NotifyGiveFromIsr` | `TaskHandle_t, BaseType_t* → void` | ISR 发送计数型通知 |
| `OS_NotifyTake` | `BaseType_t, TickType_t → uint32_t` | 获取计数型通知 |
| `OS_SemaphoreGive` | `OS_SEMAPHORE_HANDLE → void` | 释放信号量 |
| `OS_SemaphoreGiveFromIsr` | `OS_SEMAPHORE_HANDLE, BaseType_t* → void` | ISR 释放信号量 |
| `OS_SemaphoreTake` | `OS_SEMAPHORE_HANDLE, TickType_t → OS_STD_RETURN_e` | 获取信号量 |

### 5.2 任务管理公共 API（`ftask.h`）

| 函数 | 用途 |
|------|------|
| `FTSK_CreateQueues` | 创建所有系统队列 |
| `FTSK_CreateTasks` | 创建所有系统任务 |
| `FTSK_CreateTaskEngine` | Engine 任务入口 |
| `FTSK_CreateTaskCyclic1ms` | Cyclic 1ms 任务入口 |
| `FTSK_CreateTaskCyclic10ms` | Cyclic 10ms 任务入口 |
| `FTSK_CreateTaskCyclic100ms` | Cyclic 100ms 任务入口 |
| `FTSK_CreateTaskCyclicAlgorithm100ms` | Cyclic Algorithm 100ms 任务入口 |
| `FTSK_CreateTaskI2c` | I2C 任务入口 |
| `FTSK_CreateTaskAfe` | AFE 任务入口（条件） |
| `FTSK_CreateTaskUart` | UART 任务入口（条件） |
| `FTSK_CreateTaskEmac` | EMAC 任务入口（条件） |

### 5.3 用户代码函数（`ftask_cfg.h` / `ftask_cfg.c`）

| 函数 | 调用者 |
|------|--------|
| `FTSK_InitializeUserCodeEngine` | Engine 任务 |
| `FTSK_RunUserCodeEngine` | Engine 任务（每周期） |
| `FTSK_InitializeUserCodePreCyclicTasks` | Cyclic 1ms 任务（一次性） |
| `FTSK_RunUserCodeCyclic1ms` | Cyclic 1ms 任务（每周期） |
| `FTSK_RunUserCodeCyclic10ms` | Cyclic 10ms 任务（每周期） |
| `FTSK_RunUserCodeCyclic100ms` | Cyclic 100ms 任务（每周期） |
| `FTSK_RunUserCodeCyclicAlgorithm100ms` | Cyclic Algorithm 100ms 任务（每周期） |
| `FTSK_RunUserCodeI2c` | I2C 任务（每周期） |
| `FTSK_RunUserCodeAfe` | AFE 任务（条件，每周期） |
| `FTSK_RunUserCodeUart` | UART 任务（条件，每周期） |
| `FTSK_RunUserCodeEmac` | EMAC 任务（条件，每周期） |
| `FTSK_RunUserCodeIdle` | 空闲任务钩子 |

### 5.4 定时器公共 API（`timer.h`）

| 函数 | 用途 |
|------|------|
| `TIMER_Create` | 创建定时器 |
| `TIMER_Delete` | 删除定时器 |
| `TIMER_Start` | 启动定时器 |
| `TIMER_Stop` | 停止定时器 |
| `TIMER_Reset` | 重置定时器 |

### 5.5 依赖项

**外部依赖（公共模块）：**
- `fstd_types.h` — 标准类型定义
- `fassert.h` — 断言系统
- `database.h` — 数据库队列消息类型
- `can_cfg.h` — CAN 缓冲区类型
- `rtc.h` — RTC 时间数据结构
- `foxbms_config.h` — 全局配置宏

**外部依赖（FreeRTOS）：**
- `FreeRTOS.h`, `task.h`, `queue.h`, `semphr.h`, `timers.h`

**内部调用方（task 子系统被以下模块依赖）：**
- `bms` — 通过 CAN 队列、数据库队列、任务通知
- `can` — 通过 CAN Rx/Tx 队列
- `diag` — 通过 `DIAG_UpdateFlags` 在 1ms 任务中调用
- `sys_mon` — 通过 `SYSM_Notify` 接口
- 所有 BMS 应用模块 — 通过数据库队列、任务调度

---

## 6. 数据结构

### 6.1 `OS_TIMER_s` — 系统运行时间计数器

```c
typedef struct {
    uint8_t  timer_1ms;   // 毫秒 (0~9)
    uint8_t  timer_10ms;  // 10毫秒 (0~9)
    uint8_t  timer_100ms; // 100毫秒 (0~9)
    uint8_t  timer_sec;   // 秒 (0~59)
    uint8_t  timer_min;   // 分 (0~59)
    uint8_t  timer_h;     // 时 (0~23)
    uint16_t timer_d;     // 天
} OS_TIMER_s;
```

### 6.2 `OS_TASK_DEFINITION_s` — 任务定义

```c
typedef struct {
    OS_PRIORITY_e priority;     // 任务优先级
    uint32_t phase;            // 首次启动相位偏移 (ms)
    uint32_t cycleTime;        // 周期时间 (ms)
    uint32_t stackSize_B;      // 栈大小 (字节)
    void *pvParameters;        // 任务参数
} OS_TASK_DEFINITION_s;
```

### 6.3 `OS_BOOT_STATE_e` — 启动状态枚举

11 种有效状态（从 `OS_OFF` 到 `OS_BOOT_STATE_MAX`），覆盖系统从关机到全功能运行的完整生命周期。

### 6.4 `OS_PRIORITY_e` — 任务优先级枚举

10 级优先级（从 `OS_PRIORITY_IDLE` 到 `OS_PRIORITY_REAL_TIME`），与 FreeRTOSConfig.h 保持同步。

### 6.5 队列句柄（全局变量）

所有队列句柄声明在 `ftask.h`/`ftask_freertos.c` 中，类型为 `OS_QUEUE`（即 `QueueHandle_t`），初始值为 `NULL_PTR`。

---

## 7. 状态机 / 控制流

### 7.1 系统启动序列

```
OS_OFF
  └─ OS_InitializeOperatingSystem()
       ├─ OS_INITIALIZE_SCHEDULER
       │    └─ OS_InitializeScheduler() [缓存控制]
       ├─ OS_CREATE_QUEUES
       │    └─ FTSK_CreateQueues() [创建10+个队列]
       ├─ OS_CREATE_TASKS
       │    └─ FTSK_CreateTasks() [创建5-9个任务]
       └─ OS_INIT_PRE_OS
            └─ OS_StartScheduler() → 永不返回
                 ├─ TaskEngine 开始执行
                 │    ├─ OS_SCHEDULER_RUNNING
                 │    ├─ FTSK_InitializeUserCodeEngine()
                 │    └─ OS_ENGINE_RUNNING
                 │         └─ 进入无限循环 [DATA_Task + SYSM_CheckNotifications]
                 ├─ TaskCyclic1ms 开始执行 (等待 OS_ENGINE_RUNNING)
                 │    ├─ FTSK_InitializeUserCodePreCyclicTasks()
                 │    ├─ OS_PRE_CYCLIC_INITIALIZATION_HAS_FINISHED
                 │    └─ 进入无限循环 [OS_IncrementTimer + DIAG + MEAS + CAN Rx]
                 ├─ TaskCyclic10ms (等待 PRE_CYCLIC_INIT, 相位 2ms)
                 ├─ TaskCyclic100ms (等待 PRE_CYCLIC_INIT, 相位 56ms)
                 ├─ TaskCyclicAlgorithm100ms (等待 PRE_CYCLIC_INIT, 相位 64ms)
                 │    └─ OS_SYSTEM_RUNNING
                 ├─ TaskI2c, TaskAfe, TaskUart, TaskEmac 并行执行
                 └─ Idle Hook → FTSK_RunUserCodeIdle()
```

### 7.2 任务间等待关系

```
Engine ──(os_boot == OS_ENGINE_RUNNING)──→ Cyclic 1ms
Cyclic 1ms ──(os_boot == OS_PRE_CYCLIC_INITIALIZATION_HAS_FINISHED)──→ 所有其他任务
Algorithm 100ms ──(设置 os_boot = OS_SYSTEM_RUNNING)──→ 系统全功能就绪
```

---

## 8. 追溯矩阵

| 需求编号 | 需求标题 | 源文件 | 关键函数/宏 |
|---------|---------|--------|------------|
| `REQ-001` | 操作系统初始化 | `os.c`, `os_freertos.c` | `OS_InitializeOperatingSystem`, `OS_InitializeScheduler`, `OS_StartScheduler` |
| `REQ-002` | 系统启动状态跟踪 | `os.h`, `os.c`, `ftask.c` | `OS_BOOT_STATE_e`, `os_boot`, `os_schedulerStartTime` |
| `REQ-003` | 系统运行时间计数器 | `os.c`, `os.h` | `OS_IncrementTimer`, `OS_TIMER_s`, `os_timer` |
| `REQ-004` | 时间流逝判定 | `os.c` | `OS_CheckTimeHasPassed`, `OS_CheckTimeHasPassedWithTimestamp` |
| `REQ-005` | 时间流逝检查自检 | `os.c` | `OS_CheckTimeHasPassedSelfTest` |
| `REQ-006` | 系统节拍与任务延迟 | `os.h`, `os_freertos.c` | `OS_GetTickCount`, `OS_DelayTask`, `OS_DelayTaskUntil` |
| `REQ-007` | 临界区管理 | `os.h`, `os_freertos.c` | `OS_EnterTaskCritical`, `OS_ExitTaskCritical` |
| `REQ-008` | 任务通知 | `os.h`, `os_freertos.c` | `OS_WaitForNotification`, `OS_NotifyFromIsr`, `OS_NotifyGive`, `OS_NotifyGiveFromIsr`, `OS_NotifyTake` |
| `REQ-009` | 索引化任务通知 | `os.h`, `os_freertos.c` | `OS_WaitForNotificationIndexed`, `OS_NotifyIndexedFromIsr`, `OS_ClearNotificationIndexed` |
| `REQ-010` | 队列管理 | `os.h`, `os_freertos.c` | `OS_ReceiveFromQueue`, `OS_SendToBackOfQueue`, `OS_SendToBackOfQueueFromIsr`, `OS_GetNumberOfStoredMessagesInQueue` |
| `REQ-011` | 任务挂起与恢复 | `os.h`, `os_freertos.c` | `OS_SuspendTask`, `OS_ResumeTask` |
| `REQ-012` | 信号量管理 | `os.h`, `os_freertos.c` | `OS_SemaphoreGive`, `OS_SemaphoreGiveFromIsr`, `OS_SemaphoreTake` |
| `REQ-013` | FPU 上下文管理 | `os.h`, `os_freertos.c` | `OS_MarkTaskAsRequiringFpuContext` |
| `REQ-014` | FreeRTOS 配置编译期验证 | `os_freertos_config-validation.h` | `FAS_STATIC_ASSERT`, `#error` |
| `REQ-015` | 空闲任务钩子 | `os_freertos.c` | `vApplicationIdleHook`, `FTSK_RunUserCodeIdle` |
| `REQ-016` | 空闲/定时器任务内存 | `os_freertos.c` | `vApplicationGetIdleTaskMemory`, `vApplicationGetTimerTaskMemory` |
| `REQ-017` | 栈溢出检测与崩溃转储 | `os_freertos.c` | `vApplicationStackOverflowHook`, `CANTX_CrashDump` |
| `REQ-018` | 缓存控制 | `os_freertos.c` | `OS_InitializeScheduler`, `OS_ENABLE_CACHE` |
| `REQ-019` | 堆分配失败钩子 | `ftask_freertos.c` | `vApplicationMallocFailedHook` |
| `REQ-020` | 系统队列创建 | `ftask.h`, `ftask_freertos.c` | `FTSK_CreateQueues`, 各队列句柄和存储区 |
| `REQ-021` | 系统任务创建 | `ftask.h`, `ftask_freertos.c` | `FTSK_CreateTasks` |
| `REQ-022` | Engine 任务 | `ftask.c`, `ftask_cfg.c` | `FTSK_CreateTaskEngine`, `FTSK_InitializeUserCodeEngine`, `FTSK_RunUserCodeEngine` |
| `REQ-023` | Cyclic 1ms 任务 | `ftask.c`, `ftask_cfg.c` | `FTSK_CreateTaskCyclic1ms`, `FTSK_RunUserCodeCyclic1ms` |
| `REQ-024` | Cyclic 10ms 任务 | `ftask.c`, `ftask_cfg.c` | `FTSK_CreateTaskCyclic10ms`, `FTSK_RunUserCodeCyclic10ms` |
| `REQ-025` | Cyclic 100ms 任务 | `ftask.c`, `ftask_cfg.c` | `FTSK_CreateTaskCyclic100ms`, `FTSK_RunUserCodeCyclic100ms` |
| `REQ-026` | Cyclic Algorithm 100ms | `ftask.c`, `ftask_cfg.c` | `FTSK_CreateTaskCyclicAlgorithm100ms`, `FTSK_RunUserCodeCyclicAlgorithm100ms` |
| `REQ-027` | 连续运行型任务 | `ftask.c`, `ftask_cfg.c` | `FTSK_CreateTaskI2c/Afe/Uart/Emac`, `FTSK_RunUserCodeI2c/Afe/Uart/Emac` |
| `REQ-028` | 任务参数化配置 | `ftask_cfg.h`, `ftask_cfg.c` | `OS_TASK_DEFINITION_s`, 各 `FTSK_TASK_*` 宏 |
| `REQ-029` | 定时器创建 | `timer.c` | `TIMER_Create` |
| `REQ-030` | 定时器生命周期管理 | `timer.c` | `TIMER_Start`, `TIMER_Stop`, `TIMER_Reset`, `TIMER_Delete` |
| `REQ-031` | malloc 失败钩子 | `ftask_freertos.c` | `vApplicationMallocFailedHook` |

---

*本文档由 foxBMS 2 任务子系统源代码自动分析生成，版本 v1.11.0，生成日期 2026-06-06。*
