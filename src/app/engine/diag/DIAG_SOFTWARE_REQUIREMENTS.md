# DIAG — 诊断引擎模块软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义 foxBMS 2 诊断引擎模块（DIAG）的软件需求。DIAG 模块负责 BMS 全系统的错误检测、错误记录、错误上报和错误回调处理。

### 1.2 范围

- **涵盖**：诊断模块初始化、诊断事件处理（DIAG_Handler）、阈值计数器管理、错误/警告标志位操作、致命错误（Fatal Error）CAN 上报与重发、事件记录与去重、诊断回调函数接口、诊断条目状态查询
- **不涵盖**：各诊断回调函数（cbs）的具体业务逻辑

### 1.3 定义与缩略语

| 缩略语 | 说明 |
|--------|------|
| DIAG | 诊断引擎模块（Diagnosis Engine） |
| Fatal Error | 致命错误，触发后需通过 CAN 上报并可能导致接触器断开 |
| Threshold | 错误阈值，事件发生次数超过阈值后才认为错误有效 |
| CBS | 回调函数（Callback Function），诊断事件触发后执行的业务逻辑 |

### 1.4 参考文献

- [diag.h](diag.h) — 诊断模块头文件
- [diag.c](diag.c) — 诊断模块实现
- [diag_cfg.h](../config/diag_cfg.h) — 诊断配置头文件
- [diag_cbs.h](cbs/diag_cbs.h) — 诊断回调函数声明

---

## 2. 总体描述

### 2.1 产品视角

DIAG 模块是 BMS 的统一错误处理框架。各业务模块在检测到异常时调用 `DIAG_Handler()` 报告事件，DIAG 模块负责：事件去重、阈值判定、错误/警告标志位管理、事件记录、致命错误 CAN 上报，以及通过回调函数触发相应的错误处理逻辑。

### 2.2 工作模式

诊断模块使用 state 模式：
- **UNINITIALIZED**：模块未初始化
- **INITIALIZED**：模块就绪，可处理诊断事件

事件类型：
- `DIAG_EVENT_OK`：无异常，递减阈值计数器
- `DIAG_EVENT_NOT_OK`：检测到异常，递增阈值计数器
- `DIAG_EVENT_RESET`：重置计数器（用于重新初始化场景）

---

## 3. 功能需求

### 3.1 诊断模块初始化

#### REQ-APP_ENGINE_DIAG_001 — 诊断模块初始化

**编号** | REQ-APP_ENGINE_DIAG_001
**优先级** | 高
**函数** | `DIAG_Initialize()`

**描述**：系统应初始化诊断模块，包括：
1. 建立 `id2ch[]` 查找表（diagnosis ID → 配置通道索引）
2. 根据 `enable_evaluate` 配置初始化 `err_enableflag[]` 启用掩码
3. 构建致命错误链表（`pFatalErrorLinkTable`），统计致命错误条目数
4. 验证致命错误配置了有意义的延时（不能为 `DIAG_DELAY_DISCARD`）
5. 创建并启动致命错误周期性重发定时器

**前置条件**：`diag_dev_pointer` 非空，配置数组有效

**后置条件**：模块状态设为 `DIAG_STATE_INITIALIZED`

**错误处理**：
- Diagnosis ID 超出 `DIAG_ID_MAX`：设置 retval = `STD_NOT_OK`
- 致命错误配置延时为 DISCARD：触发 `FAS_ASSERT(FAS_TRAP)`

---

### 3.2 诊断事件处理

#### REQ-APP_ENGINE_DIAG_002 — 诊断事件处理器

**编号** | REQ-APP_ENGINE_DIAG_002
**优先级** | 高
**函数** | `DIAG_Handler()`

**描述**：系统应提供统一的诊断事件处理入口，根据事件类型执行不同逻辑：

**DIAG_EVENT_OK（无异常）**：
1. 若阈值计数器 > 1 → 递减计数器
2. 若计数器 == 1 → 清零计数器，清除错误/警告标志位，记录事件消失
3. 若事件为致命错误 → 发送致命错误清除 CAN 消息
4. 若启用评估 → 调用回调函数（传入 `DIAG_EVENT_RESET`）

**DIAG_EVENT_NOT_OK（检测到异常）**：
1. 若计数器 < 阈值 → 递增计数器，返回 OK
2. 若计数器 == 阈值 → 设置错误标志位，清除警告标志位，记录事件
3. 若为致命错误 → 发送致命错误 CAN 消息
4. 若启用评估 → 调用回调函数
5. 若计数器 > 阈值 → 返回 ERR_OCCURRED（已超过阈值，不重复记录）
6. 若错误未启用（enable mask 为 0）→ 设置警告标志位，返回 WARNING_OCCURRED

**DIAG_EVENT_RESET（重置）**：
1. 清除错误/警告标志位，清零计数器，记录事件
2. 若启用评估 → 调用回调函数

**输入验证**：
- `diagId >= DIAG_ID_MAX` → 返回 `DIAG_HANDLER_RETURN_WRONG_ID`
- 影响级别无效 → 返回 `DIAG_HANDLER_INVALID_ERR_IMPACT`
- 字符串级事件但 data >= BS_NR_OF_STRINGS → 返回 `DIAG_HANDLER_INVALID_DATA`
- 模块未初始化 → 返回 `DIAG_HANDLER_RETURN_NOT_READY`

---

#### REQ-APP_ENGINE_DIAG_003 — 诊断事件便捷检查

**编号** | REQ-APP_ENGINE_DIAG_003
**优先级** | 中
**函数** | `DIAG_CheckEvent()`

**描述**：系统应提供便捷的诊断事件检查接口，自动将 `STD_OK` 条件映射为 `DIAG_EVENT_OK`，非 `STD_OK` 映射为 `DIAG_EVENT_NOT_OK`，简化调用方代码。

---

### 3.3 事件记录与去重

#### REQ-APP_ENGINE_DIAG_004 — 诊断事件写入/去重

**编号** | REQ-APP_ENGINE_DIAG_004
**优先级** | 高
**函数** | `DIAG_EntryWrite()`

**描述**：系统应提供事件记录功能并防止重复记录：
1. 若诊断模块已锁定（`diag_locked > 0`）→ 不记录
2. 若相同事件 ID 的相同事件类型已在之前记录 → 不重复记录（需等到事件状态翻转）
3. 若上次是 `DIAG_EVENT_OK` 而本次是 `DIAG_EVENT_RESET` → 不记录（仅在上次为错误时才记录 RESET）
4. 若同一事件 ID 的记录次数超过 `DIAG_MAX_ENTRIES_OF_ERROR` → 丢弃后续记录
5. 记录成功后递增 `reportedErrorCount` 和 `totalErrorCount`

---

### 3.4 致命错误管理

#### REQ-APP_ENGINE_DIAG_005 — 致命错误设置

**编号** | REQ-APP_ENGINE_DIAG_005
**优先级** | 高
**函数** | `DIAG_SetFatalErrorById()`

**描述**：系统应在检测到致命错误时通过 CAN 发送错误 ID，将其标记到 `diag_activeFatalErrors[]` 数组中。若之前无活动致命错误，启动重发定时器。递增活动致命错误计数。

---

#### REQ-APP_ENGINE_DIAG_006 — 致命错误清除

**编号** | REQ-APP_ENGINE_DIAG_006
**优先级** | 高
**函数** | `DIAG_ClearFatalErrorById()`

**描述**：系统应在致命错误消失时通过 CAN 发送清除消息，取消标记，递减活动计数。若无活动致命错误，停止重发定时器。

---

#### REQ-APP_ENGINE_DIAG_007 — 致命错误周期性重发

**编号** | REQ-APP_ENGINE_DIAG_007
**优先级** | 中
**函数** | `DIAG_ResendFatalErrors()`

**描述**：系统应周期性重发所有活动致命错误（若 `diag_activeFatalErrorCount > 0`），遍历所有标记的错误 ID 并通过 CAN 发送。通过 FreeRTOS 软件定时器驱动，重发周期为 100ms。

---

### 3.5 状态查询

#### REQ-APP_ENGINE_DIAG_008 — 诊断条目状态查询

**编号** | REQ-APP_ENGINE_DIAG_008
**优先级** | 中
**函数** | `DIAG_GetDiagnosisEntryState()`

**描述**：系统应提供查询指定诊断条目是否触发的接口。遍历所有电池串的阈值计数器，若任一串的计数器超过配置阈值则返回 `STD_NOT_OK`。

---

#### REQ-APP_ENGINE_DIAG_009 — 致命错误全局检查

**编号** | REQ-APP_ENGINE_DIAG_009
**优先级** | 高
**函数** | `DIAG_IsAnyFatalErrorSet()`

**描述**：系统应提供全局致命错误检查接口，遍历所有致命错误链接表中的条目，若任一处于触发状态则返回 true。

---

#### REQ-APP_ENGINE_DIAG_010 — 诊断延时查询

**编号** | REQ-APP_ENGINE_DIAG_010
**优先级** | 中
**函数** | `DIAG_GetDelay()`

**描述**：系统应提供查询指定诊断条目的配置延时值（ms）的接口。延时值用于在检测到故障后延迟过渡到错误状态的时间。

---

## 4. 非功能需求

### 4.1 时序与性能

| 需求编号 | 描述 |
|----------|------|
| REQ-APP_ENGINE_DIAG_NFR_001 | 致命错误 CAN 消息应周期性重发（默认周期 100ms），确保上级控制器能够接收到 |
| REQ-APP_ENGINE_DIAG_NFR_002 | 诊断事件处理应高效，通过位操作（位掩码）管理错误/警告标志 |

### 4.2 内存

| 需求编号 | 描述 |
|----------|------|
| REQ-APP_ENGINE_DIAG_NFR_003 | `DIAG_ID_MAX` 应小于 255（UINT8_MAX），确保 ID 可用 uint8_t 表示 |
| REQ-APP_ENGINE_DIAG_NFR_004 | 错误/警告标志使用位图数组 `uint32_t[(DIAG_ID_MAX + 31) / 32]`，节省内存 |

### 4.3 鲁棒性

| 需求编号 | 描述 |
|----------|------|
| REQ-APP_ENGINE_DIAG_NFR_005 | 诊断模块在未初始化时应拒绝处理事件，返回 `DIAG_HANDLER_RETURN_NOT_READY` |
| REQ-APP_ENGINE_DIAG_NFR_006 | 事件写入时通过 `diag_locked` 标志实现互斥保护 |
| REQ-APP_ENGINE_DIAG_NFR_007 | 致命错误必须配置有效延时（不能为 `DIAG_DELAY_DISCARD`），初始化时断言检查 |
| REQ-APP_ENGINE_DIAG_NFR_008 | 每种错误类型最多记录 `DIAG_MAX_ENTRIES_OF_ERROR` 次，防止缓冲区溢出 |

---

## 5. 接口需求

### 5.1 公共 API

| 函数 | 说明 |
|------|------|
| `DIAG_Initialize()` | 初始化诊断模块 |
| `DIAG_Handler()` | 诊断事件处理入口 |
| `DIAG_CheckEvent()` | 便捷事件检查（STD_OK → DIAG_EVENT_OK） |
| `DIAG_GetDiagnosisEntryState()` | 查询诊断条目状态 |
| `DIAG_IsAnyFatalErrorSet()` | 检查是否存在活动致命错误 |
| `DIAG_GetDelay()` | 查询诊断条目配置延时 |
| `DIAG_PrintErrors()` | 打印错误缓冲区（桩） |

### 5.2 依赖项

| 被依赖模块 | 调用目的 |
|------------|----------|
| `CANTX` | 发送致命错误 CAN 消息 |
| `TIMER` | 创建/操作致命错误重发定时器 |
| `database` | 通过 `diag_kDatabaseShim` 访问数据库 |

### 5.3 调用方

所有需要错误检测的模块（BMS、SYS、AFE、CAN、MEAS、BAL 等）均通过 `DIAG_Handler()` 或 `DIAG_CheckEvent()` 报告诊断事件。

---

## 6. 追溯矩阵

| 需求编号 | 源文件 | 函数/宏 |
|----------|--------|---------|
| REQ-APP_ENGINE_DIAG_001 | diag.c | `DIAG_Initialize()` |
| REQ-APP_ENGINE_DIAG_002 | diag.c | `DIAG_Handler()` |
| REQ-APP_ENGINE_DIAG_003 | diag.c | `DIAG_CheckEvent()` |
| REQ-APP_ENGINE_DIAG_004 | diag.c | `DIAG_EntryWrite()` |
| REQ-APP_ENGINE_DIAG_005 | diag.c | `DIAG_SetFatalErrorById()` |
| REQ-APP_ENGINE_DIAG_006 | diag.c | `DIAG_ClearFatalErrorById()` |
| REQ-APP_ENGINE_DIAG_007 | diag.c | `DIAG_ResendFatalErrors()` |
| REQ-APP_ENGINE_DIAG_008 | diag.c | `DIAG_GetDiagnosisEntryState()` |
| REQ-APP_ENGINE_DIAG_009 | diag.c | `DIAG_IsAnyFatalErrorSet()` |
| REQ-APP_ENGINE_DIAG_010 | diag.c | `DIAG_GetDelay()` |
