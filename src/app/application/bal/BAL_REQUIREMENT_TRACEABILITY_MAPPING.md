# BAL — 需求可追溯性映射

**文件**: `bal.c`
**版本**: v1.11.0
**创建日期**: 2026-06-05
**配套文档**: [BAL_SOFTWARE_REQUIREMENTS.md](BAL_SOFTWARE_REQUIREMENTS.md)

---

## 文档说明

本文档建立了 BAL 模块软件需求与源代码之间的双向追溯关系。通过本文档可以：

1. **按需求查找代码**：从需求编号出发，快速定位到实现该需求的函数和代码行
2. **按代码查找需求**：从函数名出发，了解其所实现的需求
3. **验证覆盖率**：检查所有函数是否都有对应的需求描述，以及所有需求是否都有对应的代码实现

本文档与 [BAL_SOFTWARE_REQUIREMENTS.md](BAL_SOFTWARE_REQUIREMENTS.md) 配套使用。需求文档描述"做什么"，本映射文档记录"在哪里实现"。

---

## 需求到代码映射表

### 1. 状态历史记录管理

#### FR-3.1.1 — 保存上一状态与子状态

| 项目 | 内容 |
|------|------|
| **需求编号** | FR-3.1.1 |
| **需求描述** | 系统应保存状态机的上一状态和上一子状态，以便追踪状态变迁历史 |
| **代码位置** | [`bal.c:72-83`](bal.c#L72-L83) |
| **实现方式** | `BAL_SaveLastStates()` 函数：比较当前状态/子状态与历史值，按需更新 `lastState` 和 `lastSubstate` |
| **涉及数据结构** | `BAL_STATE_s::lastState`, `BAL_STATE_s::lastSubstate`, `BAL_STATE_s::state`, `BAL_STATE_s::substate` |

---

### 2. 重入保护

#### FR-3.2.1 — 重入检查

| 项目 | 内容 |
|------|------|
| **需求编号** | FR-3.2.1 |
| **需求描述** | 系统应提供重入保护机制，确保状态机触发函数每次仅被一个执行上下文调用 |
| **代码位置** | [`bal.c:85-98`](bal.c#L85-L98) |
| **实现方式** | `BAL_CheckReEntrance()` 函数：在临界区内检查并递增 `triggerEntry` 计数器，已运行时返回 `0xFF` |
| **涉及数据结构** | `BAL_STATE_s::triggerEntry` |
| **涉及宏/常量** | `0xFF`（重入标志返回值） |

---

### 3. 状态请求转移

#### FR-3.3.1 — 状态请求转移

| 项目 | 内容 |
|------|------|
| **需求编号** | FR-3.3.1 |
| **需求描述** | 系统应从当前状态结构体中原子读取并重置状态请求 |
| **代码位置** | [`bal.c:100-110`](bal.c#L100-L110) |
| **实现方式** | `BAL_TransferStateRequest()` 函数：在临界区内读取 `stateRequest` 字段并重置为 `BAL_STATE_NO_REQUEST` |
| **涉及数据结构** | `BAL_STATE_s::stateRequest` |

---

### 4. 状态请求合法性校验

#### FR-3.4.1 — 直接通过类请求校验

| 项目 | 内容 |
|------|------|
| **需求编号** | FR-3.4.1 |
| **需求描述** | 系统应识别 ERROR_REQUEST / GLOBAL_ENABLE_REQUEST / GLOBAL_DISABLE_REQUEST / NO_BALANCING_REQUEST / ALLOW_BALANCING_REQUEST 为始终合法的请求 |
| **代码位置** | [`bal.c:112-127`](bal.c#L112-L127) |
| **实现方式** | `BAL_CheckStateRequest()` 函数：通过一系列 `if` 判断对上述 5 种请求直接返回 `BAL_OK` |
| **涉及数据结构** | `BAL_STATE_s::balancingGlobalAllowed`（被 GLOBAL_ENABLE/DISABLE 修改） |

#### FR-3.4.2 — 初始化请求校验

| 项目 | 内容 |
|------|------|
| **需求编号** | FR-3.4.2 |
| **需求描述** | 系统应仅在无待处理请求且当前为未初始化状态时接受初始化请求 |
| **代码位置** | [`bal.c:129-143`](bal.c#L129-L143) |
| **实现方式** | `BAL_CheckStateRequest()` 函数：先检查 `stateRequest == NO_REQUEST`，再检查 `state == UNINITIALIZED`，返回相应结果 |
| **涉及数据结构** | `BAL_STATE_s::stateRequest`, `BAL_STATE_s::state` |

---

### 5. 初始化功能

#### FR-3.5.1 — 均衡模块初始化

| 项目 | 内容 |
|------|------|
| **需求编号** | FR-3.5.1 |
| **需求描述** | 系统应提供均衡模块的通用初始化函数，禁用均衡功能 |
| **代码位置** | [`bal.c:146-152`](bal.c#L146-L152) |
| **实现方式** | `BAL_Init()` 函数：从数据库读取控制数据，设置 `enableBalancing = false`，写回数据库 |
| **涉及数据结构** | `DATA_BLOCK_BALANCING_CONTROL_s::enableBalancing` |
| **涉及函数** | `DATA_READ_DATA()`, `DATA_WRITE_DATA()` |

---

### 6. 状态机状态处理

#### FR-3.6.1 — 未初始化状态处理

| 项目 | 内容 |
|------|------|
| **需求编号** | FR-3.6.1 |
| **需求描述** | 系统应在未初始化状态下处理状态请求：INIT_REQUEST → 进入初始化，非法请求 → 计数 |
| **代码位置** | [`bal.c:154-165`](bal.c#L154-L165) |
| **实现方式** | `BAL_ProcessStateUninitialized()` 函数：根据 `stateRequest` 类型分别处理 |
| **涉及数据结构** | `BAL_STATE_s::timer`, `BAL_STATE_s::state`, `BAL_STATE_s::substate`, `BAL_STATE_s::errorRequestCounter` |
| **涉及宏** | `BAL_FSM_SHORTTIME_100ms` |

#### FR-3.6.2 — 初始化状态处理

| 项目 | 内容 |
|------|------|
| **需求编号** | FR-3.6.2 |
| **需求描述** | 系统应在初始化状态下转换到已初始化状态 |
| **代码位置** | [`bal.c:167-172`](bal.c#L167-L172) |
| **实现方式** | `BAL_ProcessStateInitialization()` 函数：设置定时器、状态和子状态 |
| **涉及数据结构** | `BAL_STATE_s::timer`, `BAL_STATE_s::state`, `BAL_STATE_s::substate` |
| **涉及宏** | `BAL_FSM_SHORTTIME_100ms` |

#### FR-3.6.3 — 已初始化状态处理

| 项目 | 内容 |
|------|------|
| **需求编号** | FR-3.6.3 |
| **需求描述** | 系统应在已初始化状态下确认初始化完成并进入均衡检查状态 |
| **代码位置** | [`bal.c:174-180`](bal.c#L174-L180) |
| **实现方式** | `BAL_ProcessStateInitialized()` 函数：设置 `initializationFinished = STD_OK`，转换到 `CHECK_BALANCING` 状态 |
| **涉及数据结构** | `BAL_STATE_s::initializationFinished`, `BAL_STATE_s::timer`, `BAL_STATE_s::state`, `BAL_STATE_s::substate` |
| **涉及宏** | `BAL_FSM_SHORTTIME_100ms` |

---

### 7. 非功能需求映射

| 需求编号 | 需求描述 | 代码位置 | 实现方式 |
|----------|----------|----------|----------|
| NFR-4.1.1 | 100ms 触发周期 | [`bal_cfg.h:65`](../config/bal_cfg.h#L65) | `BAL_FSM_SHORTTIME_100ms = 1u`（1 × 100ms） |
| NFR-4.1.2 | 5s 长延时 | [`bal_cfg.h:69`](../config/bal_cfg.h#L69) | `BAL_FSM_LONGTIME_100ms = 50u`（50 × 100ms） |
| NFR-4.1.3 | 1s 均衡周期 | [`bal_cfg.h:72`](../config/bal_cfg.h#L72) | `BAL_FSM_BALANCING_TIME_100ms = 10u`（10 × 100ms） |
| NFR-4.3.1 | 空指针保护 | [`bal.c:73,86,101,113,147,155,168,175`](bal.c) | 每个 extern 函数入口处 `FAS_ASSERT(ptr != NULL_PTR)` |
| NFR-4.3.2 | 重入保护 | [`bal.c:85-98`](bal.c#L85-L98) | `BAL_CheckReEntrance()` 的 `triggerEntry` 计数器 |
| NFR-4.3.3 | 临界区保护 | [`bal.c:89-95`](bal.c#L89-L95)、[`bal.c:104-108`](bal.c#L104-L108) | `OS_EnterTaskCritical()` / `OS_ExitTaskCritical()` |
| NFR-4.3.4 | 阈值边界限定 | [`bal_cfg.c:75-86`](../config/bal_cfg.c#L75-L86) | `BAL_SetBalancingThreshold()` 中对 `bal_threshold_mV` 的边界检查 |
| NFR-4.4.1 | 配置集中管理 | [`bal_cfg.h:65-90`](../config/bal_cfg.h#L65-L90) | 所有配置宏集中定义 |
| NFR-4.5.1 | 单元测试支持 | [`bal.h:234-237,240-242`](bal.h#L234-L237) | `#ifdef UNITY_UNIT_TEST` 暴露测试接口 |

---

## 代码到需求反向映射表

### 关键函数与需求关联

#### `BAL_SaveLastStates()` — [`bal.c:72-83`](bal.c#L72-L83)

- **实现需求**: FR-3.1.1
- **功能**: 保存状态机上一状态和子状态的历史记录
- **断言**: `FAS_ASSERT(pBalancingState != NULL_PTR)`

---

#### `BAL_CheckReEntrance()` — [`bal.c:85-98`](bal.c#L85-L98)

- **实现需求**: FR-3.2.1, NFR-4.3.2, NFR-4.3.3
- **功能**: 状态机触发函数重入检查，防止并发调用
- **断言**: `FAS_ASSERT(currentState != NULL_PTR)`
- **临界区**: 是

---

#### `BAL_TransferStateRequest()` — [`bal.c:100-110`](bal.c#L100-L110)

- **实现需求**: FR-3.3.1, NFR-4.3.3
- **功能**: 原子读取并重置状态请求
- **断言**: `FAS_ASSERT(currentState != NULL_PTR)`
- **临界区**: 是

---

#### `BAL_CheckStateRequest()` — [`bal.c:112-144`](bal.c#L112-L144)

- **实现需求**: FR-3.4.1, FR-3.4.2
- **功能**: 校验状态请求的合法性，返回校验结果
- **断言**: `FAS_ASSERT(pCurrentState != NULL_PTR)`
- **关键分支**:
  - `BAL_STATE_ERROR_REQUEST` → 直接通过 (L114)
  - `BAL_STATE_GLOBAL_ENABLE/DISABLE_REQUEST` → 设置标志后通过 (L117-123)
  - `BAL_STATE_NO_BALANCING/ALLOW_BALANCING_REQUEST` → 直接通过 (L125-126)
  - `BAL_STATE_INIT_REQUEST` → 仅在 UNINITIALIZED 状态通过 (L131-136)

---

#### `BAL_Init()` — [`bal.c:146-152`](bal.c#L146-L152)

- **实现需求**: FR-3.5.1
- **功能**: 均衡模块通用初始化，禁用均衡
- **断言**: `FAS_ASSERT(pControl != NULL_PTR)`
- **依赖**: `DATA_READ_DATA()`, `DATA_WRITE_DATA()`

---

#### `BAL_ProcessStateUninitialized()` — [`bal.c:154-165`](bal.c#L154-L165)

- **实现需求**: FR-3.6.1
- **功能**: 处理未初始化状态的请求
- **断言**: `FAS_ASSERT(pCurrentState != NULL_PTR)`
- **关键分支**:
  - `INIT_REQUEST` → 转换到 `BAL_FSM_INITIALIZATION` (L156-159)
  - `NO_REQUEST` → 无操作 (L160-161)
  - 其他 → `errorRequestCounter++` (L163)

---

#### `BAL_ProcessStateInitialization()` — [`bal.c:167-172`](bal.c#L167-L172)

- **实现需求**: FR-3.6.2
- **功能**: 从初始化状态转换到已初始化状态
- **断言**: `FAS_ASSERT(currentState != NULL_PTR)`

---

#### `BAL_ProcessStateInitialized()` — [`bal.c:174-180`](bal.c#L174-L180)

- **实现需求**: FR-3.6.3
- **功能**: 完成初始化确认，进入均衡检查状态
- **断言**: `FAS_ASSERT(currentState != NULL_PTR)`

---

### 配置文件函数与需求关联

| 函数 | 文件位置 | 实现需求 | 功能 |
|------|----------|----------|------|
| `BAL_SetBalancingThreshold()` | [`bal_cfg.c:75-86`](../config/bal_cfg.c#L75-L86) | NFR-4.3.4 | 设置均衡阈值（含边界检查） |
| `BAL_GetBalancingThreshold_mV()` | [`bal_cfg.c:88-90`](../config/bal_cfg.c#L88-L90) | — | 获取当前均衡阈值 |

---

## 需求覆盖率统计

| 类别 | 总数 | 已覆盖 | 覆盖率 |
|------|------|--------|--------|
| 功能需求 (FR) | 8 | 8 | 100% |
| 非功能需求 (NFR) | 9 | 9 | 100% |
| 公开函数 (extern) | 8 | 8 | 100% |
| 配置宏 | 9 | 9 | 100% |

---

## 使用本映射文档

### 按需求查找代码

1. 在需求文档中找到需求编号（如 FR-3.4.2）
2. 在本文件"需求到代码映射表"中查找对应编号
3. 查看"代码位置"列，点击链接可直接跳转到源代码
4. 查看"实现方式"列了解具体实现逻辑

### 按代码查找需求

1. 找到要分析的函数名（如 `BAL_CheckStateRequest`）
2. 在本文件"代码到需求反向映射表"中查找对应函数
3. 查看"实现需求"列了解该函数满足哪些需求
4. 在需求文档中阅读需求的完整描述

### 验证覆盖率

1. 检查"需求覆盖率统计"表格
2. 若存在未覆盖的函数，将其加入"代码到需求反向映射表"
3. 若存在未映射代码的需求，在对应函数中确认实现或标记为未实现

---

**文档生成日期**: 2026-06-05
**生成工具**: Claude Code (write-software-requirements skill)
**适用范围**: foxBMS 2 v1.11.0
