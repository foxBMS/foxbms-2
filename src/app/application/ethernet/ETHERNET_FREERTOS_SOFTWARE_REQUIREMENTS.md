# ETHERNET_FREERTOS — 软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 以太网模块 FreeRTOS 抽象层的软件需求规格。该层封装了 FreeRTOS 任务创建/删除操作，为上层的以太网核心模块（`ethernet.c`）提供操作系统任务管理接口。

### 1.2 范围

- **涵盖**：监听任务创建、Echo 服务器任务创建、任务删除、单例任务保护
- **不涵盖**：FreeRTOS 内核调度实现、任务间通信（队列/信号量）、具体任务函数逻辑

### 1.3 定义与缩略语

| 术语 | 说明 |
|------|------|
| FreeRTOS | 嵌入式实时操作系统 |
| Task/任务 | FreeRTOS 调度执行的独立线程 |
| 静态任务创建 | 使用预分配内存创建任务（`xTaskCreateStatic`） |
| 单例模式 | 确保每种任务在系统中只有一个实例运行 |

### 1.4 参考文献

- FreeRTOS 任务管理 API 文档
- foxBMS 2 系统架构文档
- [ethernet.h](ethernet.h) — 上层以太网核心模块
- [ethernet_cfg.h](../config/ethernet_cfg.h) — 任务栈大小配置

---

## 2. 总体描述

### 2.1 产品视角

该模块位于 foxBMS 2 软件架构的 **APPLICATION** 层（`@ingroup APPLICATION`）。模块前缀为 `ETH`。

该模块的作用是作为 FreeRTOS 操作系统的适配层（Adapter），将操作系统相关的任务创建/删除操作与上层的业务逻辑（Echo 服务器、连接监听）解耦。这样，当 TCP 支持作为可选功能构建时，可以通过此抽象层灵活管理。

### 2.2 工作模式

该模块提供三个公共接口：

1. **`ETH_CreateListeningTask()`**：创建 TCP 连接监听任务（单例）
2. **`ETH_CreateEchoServerTask()`**：创建 Echo 服务器处理任务（单例）
3. **`ETH_DeleteCurrentTask()`**：删除当前正在运行的任务

所有任务创建均使用 **静态内存分配**（`xTaskCreateStatic`），这意味着任务栈和 TCB（Task Control Block）内存在编译期确定，避免了运行时动态内存分配带来的碎片和失败风险。

### 2.3 用户特征

本模块仅由 `ethernet.c` 中的 `vApplicationIPNetworkEventHook_Multi()` 函数调用（在 `eNetworkUp` 事件触发时）。

---

## 3. 功能需求

### 3.1 任务管理

#### REQ-001 — 监听任务创建

| 属性 | 值 |
|------|-----|
| **编号** | REQ-001 |
| **优先级** | 高 |
| **函数** | `ETH_CreateListeningTask()` |

**描述**：系统应提供监听任务的创建功能，使用 FreeRTOS 的静态任务创建 API（`xTaskCreateStatic`）创建一个用于 TCP 连接监听的 FreeRTOS 任务。

**任务属性**：

| 属性 | 值 |
|------|-----|
| 任务名 | `"Listening-Task"` |
| 任务函数 | 由调用方传入（通常为 `ETH_ListenForConnection`） |
| 栈大小 | `ETH_LISTENING_TASK_STACK_SIZE_B / GEN_BYTES_PER_WORD` 字 |
| 优先级 | `PRIORITY_BELOW_NORMAL` |
| 任务参数 | 网络端点指针（`NetworkEndPoint_t *`） |
| 栈内存 | `eth_stackListenforConnection[]`（静态分配） |
| TCB 内存 | `eth_taskListenforConnection`（静态分配） |

**处理流程**：
1. 参数校验：`taskFunction` 和 `pEndPoint` 不得为空
2. 进入临界区，检查 `taskAlreadyCreated` 标志
3. 若任务已创建，退出临界区并返回 `ETH_TASK_ALREADY_CREATED`
4. 若任务未创建，设置 `taskAlreadyCreated = true`，退出临界区
5. 调用 `xTaskCreateStatic()` 创建任务
6. 任务创建失败时触发断言

**返回值**：
- `ETH_OK`：任务创建成功
- `ETH_TASK_ALREADY_CREATED`：任务已存在，不重复创建

---

#### REQ-002 — Echo 服务器任务创建

| 属性 | 值 |
|------|-----|
| **编号** | REQ-002 |
| **优先级** | 高 |
| **函数** | `ETH_CreateEchoServerTask()` |

**描述**：系统应提供 Echo 服务器任务的创建功能，使用 FreeRTOS 的静态任务创建 API（`xTaskCreateStatic`）创建一个用于处理 Echo 服务器连接的 FreeRTOS 任务。

**任务属性**：

| 属性 | 值 |
|------|-----|
| 任务名 | `"Server-Task"` |
| 任务函数 | 由调用方传入（通常为 `ETH_EchoServerInstance`） |
| 栈大小 | `ETH_ECHO_SERVER_TASK_STACK_SIZE_B / GEN_BYTES_PER_WORD` 字 |
| 优先级 | `PRIORITY_BELOW_NORMAL` |
| 任务参数 | `NULL_PTR` |
| 栈内存 | `eth_stackEchoServer[]`（静态分配） |
| TCB 内存 | `eth_taskEchoServer`（静态分配） |

**处理流程**：
1. 参数校验：`taskFunction` 不得为空
2. 进入临界区，检查 `taskAlreadyCreated` 标志
3. 若任务已创建，退出临界区并返回 `ETH_TASK_ALREADY_CREATED`
4. 若任务未创建，设置 `taskAlreadyCreated = true`，退出临界区
5. 调用 `xTaskCreateStatic()` 创建任务
6. 任务创建失败时触发断言

**返回值**：
- `ETH_OK`：任务创建成功
- `ETH_TASK_ALREADY_CREATED`：任务已存在，不重复创建

---

#### REQ-003 — 单例任务保护

| 属性 | 值 |
|------|-----|
| **编号** | REQ-003 |
| **优先级** | 高 |
| **函数** | `ETH_CreateListeningTask()`, `ETH_CreateEchoServerTask()` |

**描述**：系统应确保每种任务（监听任务和 Echo 服务器任务）在系统中只创建一个实例。通过以下机制实现：

1. 每个任务创建函数内部使用**独立的** `static bool taskAlreadyCreated` 标志
2. 在临界区内检查标志，防止竞态条件
3. 若任务已创建，返回 `ETH_TASK_ALREADY_CREATED` 而不创建新任务

**保护机制**：
- 使用 `OS_EnterTaskCritical()` / `OS_ExitTaskCritical()` 保护标志的检查和设置操作
- 标志在函数内部为静态变量，函数间互不影响

---

#### REQ-004 — 当前任务删除

| 属性 | 值 |
|------|-----|
| **编号** | REQ-004 |
| **优先级** | 中 |
| **函数** | `ETH_DeleteCurrentTask()` |

**描述**：系统应提供删除当前正在运行任务的功能。调用 FreeRTOS 的 `vTaskDelete(NULL)` 删除调用此函数的任务自身。

**前置条件**：调用任务必须是通过 FreeRTOS API 创建的有效任务

**后置条件**：调用任务被终止并从调度器中移除

---

### 3.2 返回类型定义

#### REQ-005 — 返回状态枚举

| 属性 | 值 |
|------|-----|
| **编号** | REQ-005 |
| **优先级** | 中 |
| **定义位置** | `ethernet_freertos.h:66` |

**描述**：系统应定义 `ETH_RETURN_TYPE_e` 枚举类型，用于表示任务创建操作的返回状态。

**枚举值**：

| 值 | 含义 |
|----|------|
| `ETH_OK` | 操作成功 |
| `ETH_TASK_ALREADY_CREATED` | 任务已存在，不重复创建 |

---

## 4. 非功能需求

### 4.1 内存

| 编号 | 需求描述 | 关联代码 |
|------|---------|---------|
| NFR-001 | 监听任务栈应使用静态内存分配，大小由 `ETH_LISTENING_TASK_STACK_SIZE_B` 配置（默认 2048 字节） | `eth_stackListenforConnection[]` |
| NFR-002 | Echo 服务器任务栈应使用静态内存分配，大小由 `ETH_ECHO_SERVER_TASK_STACK_SIZE_B` 配置（默认 4096 字节） | `eth_stackEchoServer[]` |
| NFR-003 | 任务 TCB 应使用静态内存分配 | `eth_taskListenforConnection`, `eth_taskEchoServer` |
| NFR-004 | 任务栈大小转换宏 `ETH_BYTES_TO_WORDS` 应将字节数转换为字数（除以 `GEN_BYTES_PER_WORD`） | `ETH_BYTES_TO_WORDS()` |

### 4.2 鲁棒性

| 编号 | 需求描述 | 关联代码 |
|------|---------|---------|
| NFR-005 | 所有外部传入的函数指针参数必须通过 `FAS_ASSERT` 非空校验 | `ETH_CreateListeningTask()`, `ETH_CreateEchoServerTask()` |
| NFR-006 | 任务创建失败（返回 `NULL`）时必须触发断言 | `FAS_ASSERT(eth_taskHandleListening != NULL)` |
| NFR-007 | 任务创建函数的单例检查必须在临界区内执行，防止竞态条件 | `OS_EnterTaskCritical()` / `OS_ExitTaskCritical()` |

### 4.3 可配置性

| 编号 | 需求描述 | 关联配置宏 |
|------|---------|-----------|
| NFR-008 | 监听任务栈大小应可配置 | `ETH_LISTENING_TASK_STACK_SIZE_B` |
| NFR-009 | Echo 服务器任务栈大小应可配置 | `ETH_ECHO_SERVER_TASK_STACK_SIZE_B` |

---

## 5. 接口需求

### 5.1 公共 API

| 函数 | 说明 | 来源文件 |
|------|------|---------|
| `ETH_CreateListeningTask()` | 创建监听任务（单例） | `ethernet_freertos.h` |
| `ETH_CreateEchoServerTask()` | 创建 Echo 服务器任务（单例） | `ethernet_freertos.h` |
| `ETH_DeleteCurrentTask()` | 删除当前任务 | `ethernet_freertos.h` |

### 5.2 依赖项

| 模块 | 用途 |
|------|------|
| `FreeRTOS_Routing.h` | FreeRTOS 任务路由 |
| `FreeRTOS_Sockets.h` | FreeRTOS Socket 定义 |
| `os.h` | OS 任务临界区操作（`OS_EnterTaskCritical` 等） |
| `ethernet_cfg.h` | 任务栈大小配置 |
| `fassert.h` | 断言检查 |
| `general.h` | 通用定义（`GEN_BYTES_PER_WORD`） |

### 5.3 调用方

- `ethernet.c` — `vApplicationIPNetworkEventHook_Multi()` 在网络就绪时调用任务创建函数

---

## 6. 数据结构

### 6.1 返回类型枚举

```c
typedef enum {
    ETH_OK,                    /**< 操作成功 */
    ETH_TASK_ALREADY_CREATED,  /**< 任务已创建，不重复操作 */
} ETH_RETURN_TYPE_e;
```

### 6.2 全局变量

| 变量 | 类型 | 用途 |
|------|------|------|
| `eth_taskHandleListening` | `OS_TASK_HANDLE` | 监听任务句柄 |
| `eth_taskHandleEchoServer` | `OS_TASK_HANDLE` | Echo 服务器任务句柄 |

### 6.3 模块内部宏

| 宏 | 定义 | 用途 |
|----|------|------|
| `ETH_BYTES_TO_WORDS(V)` | `(V) / GEN_BYTES_PER_WORD` | 字节数转字数（用于栈大小转换） |

---

## 7. 控制流

### 7.1 任务创建流程

```
ETH_CreateListeningTask(taskFunction, pEndPoint)
  ├── FAS_ASSERT(taskFunction != NULL_PTR)
  ├── FAS_ASSERT(pEndPoint != NULL_PTR)
  ├── OS_EnterTaskCritical()
  ├── [taskAlreadyCreated == true?] ──Yes──> OS_ExitTaskCritical() → return ETH_TASK_ALREADY_CREATED
  ├── taskAlreadyCreated = true
  ├── OS_ExitTaskCritical()
  ├── xTaskCreateStatic(taskFunction, "Listening-Task", stackSize, pEndPoint, ...)
  ├── FAS_ASSERT(handle != NULL)
  └── return ETH_OK

ETH_CreateEchoServerTask(taskFunction)
  ├── FAS_ASSERT(taskFunction != NULL_PTR)
  ├── OS_EnterTaskCritical()
  ├── [taskAlreadyCreated == true?] ──Yes──> OS_ExitTaskCritical() → return ETH_TASK_ALREADY_CREATED
  ├── taskAlreadyCreated = true
  ├── OS_ExitTaskCritical()
  ├── xTaskCreateStatic(taskFunction, "Server-Task", stackSize, NULL, ...)
  ├── FAS_ASSERT(handle != NULL)
  └── return ETH_OK
```

### 7.2 调用时序

```
vApplicationIPNetworkEventHook_Multi(eNetworkUp, ...)
  ├── xQueueCreateStatic() — 创建 Socket 队列
  ├── ETH_CreateListeningTask(&ETH_ListenForConnection, pxEndPoint)
  │     └── xTaskCreateStatic() → eth_taskHandleListening
  └── ETH_CreateEchoServerTask(&ETH_EchoServerInstance)
        └── xTaskCreateStatic() → eth_taskHandleEchoServer
```

---

## 8. 追溯矩阵

| 需求编号 | 需求标题 | 源文件 | 函数/宏 |
|---------|---------|--------|---------|
| REQ-001 | 监听任务创建 | `ethernet_freertos.c:83` | `ETH_CreateListeningTask()` |
| REQ-002 | Echo 服务器任务创建 | `ethernet_freertos.c:114` | `ETH_CreateEchoServerTask()` |
| REQ-003 | 单例任务保护 | `ethernet_freertos.c:87,117` | 各函数内 `taskAlreadyCreated` 标志 |
| REQ-004 | 当前任务删除 | `ethernet_freertos.c:143` | `ETH_DeleteCurrentTask()` |
| REQ-005 | 返回状态枚举 | `ethernet_freertos.h:66` | `ETH_RETURN_TYPE_e` |
| NFR-001 | 监听任务栈静态内存 | `ethernet_freertos.c:88` | `eth_stackListenforConnection[]` |
| NFR-002 | Echo 任务栈静态内存 | `ethernet_freertos.c:118` | `eth_stackEchoServer[]` |
| NFR-003 | TCB 静态内存 | `ethernet_freertos.c:89,119` | `eth_taskListenforConnection`, `eth_taskEchoServer` |
| NFR-004 | 栈大小转换宏 | `ethernet_freertos.c:68` | `ETH_BYTES_TO_WORDS()` |
| NFR-005 | 指针非空校验 | `ethernet_freertos.c:84,115` | `FAS_ASSERT` 调用 |
| NFR-006 | 任务创建失败断言 | `ethernet_freertos.c:109,138` | `FAS_ASSERT(handle != NULL)` |
| NFR-007 | 临界区保护 | `ethernet_freertos.c:91,121` | `OS_EnterTaskCritical()` / `OS_ExitTaskCritical()` |
| NFR-008 | 监听栈大小可配置 | `ethernet_cfg.h:103` | `ETH_LISTENING_TASK_STACK_SIZE_B` |
| NFR-009 | Echo 栈大小可配置 | `ethernet_cfg.h:102` | `ETH_ECHO_SERVER_TASK_STACK_SIZE_B` |
