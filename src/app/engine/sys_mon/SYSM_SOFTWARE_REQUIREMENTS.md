# SYSM — 系统监控模块软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义 foxBMS 2 系统监控模块（SYSM）的软件需求。SYSM 模块负责监控各 FreeRTOS 任务的执行时序，检测时序违规（Timing Violation）并记录到非易失性存储器（FRAM）中。

### 1.2 范围

- **涵盖**：任务执行通知、时序违规检测、违规记录到 FRAM、违规状态查询与清除、FRAM 数据延迟写入
- **不涵盖**：FRAM 驱动本身的实现

### 1.3 参考文献

- [sys_mon.h](sys_mon.h) — 系统监控模块头文件
- [sys_mon.c](sys_mon.c) — 系统监控模块实现
- [sys_mon_cfg.h](../config/sys_mon_cfg.h) — 系统监控配置头文件

---

## 2. 功能需求

### 2.1 初始化与通知

#### REQ-APP_ENGINE_SYS_MON_001 — 系统监控初始化

**编号** | REQ-APP_ENGINE_SYS_MON_001
**优先级** | 高
**函数** | `SYSM_Initialize()`

**描述**：系统应在启动时初始化系统监控模块。从 FRAM 读取上次记录的时序违规数据到本地影子副本 `sysm_localFramCopy`。

---

#### REQ-APP_ENGINE_SYS_MON_002 — 任务执行通知

**编号** | REQ-APP_ENGINE_SYS_MON_002
**优先级** | 高
**函数** | `SYSM_Notify()`

**描述**：系统应提供任务执行通知接口，各受监控的任务在进入和退出时调用：

- **进入**（`SYSM_NOTIFY_ENTER`）：记录进入时间戳 `timestampEnter`
- **退出**（`SYSM_NOTIFY_EXIT`）：记录退出时间戳 `timestampExit`，计算本次执行时长 `duration = timestampExit - timestampEnter`

操作在临界区内进行以保证数据一致性。

---

### 2.2 时序违规检测

#### REQ-APP_ENGINE_SYS_MON_003 — 时序违规检查

**编号** | REQ-APP_ENGINE_SYS_MON_003
**优先级** | 高
**函数** | `SYSM_CheckNotifications()`

**描述**：系统应周期性检查所有已启用的受监控任务是否存在时序违规：

1. 使用上次检查的时间戳实现去抖（同一 tick 不重复检查）
2. 对每个已启用的任务，判断两项条件：
   - `time_since_last_call > cycleTime + maxJitter`（任务调用间隔超时）
   - `duration > cycleTime`（任务执行时长超限）
3. 若两项条件同时满足，则：
   - 报告诊断事件 `DIAG_ID_SYSTEM_MONITORING` -> `DIAG_EVENT_NOT_OK`
   - 若启用了记录（`SYSM_RECORDING_ENABLED`），将违规信息写入 FRAM
   - 调用配置的回调函数

---

#### REQ-APP_ENGINE_SYS_MON_004 — 时序违规记录

**编号** | REQ-APP_ENGINE_SYS_MON_004
**优先级** | 中
**函数** | `SYSM_RecordTimingViolation()`

**描述**：系统应在检测到时序违规后将其记录到本地 FRAM 影子副本中：
- 设置 `anyTimingIssueOccurred = true`
- 根据 taskId 更新对应任务的违规时长和进入时间戳
- 设置 `sysm_flagFramCopyHasChanges = true` 标记待写入

---

### 2.3 FRAM 数据管理

#### REQ-APP_ENGINE_SYS_MON_005 — FRAM 延迟写入

**编号** | REQ-APP_ENGINE_SYS_MON_005
**优先级** | 中
**函数** | `SYSM_UpdateFramData()`

**描述**：系统应将 FRAM 写入与主监控逻辑解耦。`SYSM_UpdateFramData()` 检查变更标志 `sysm_flagFramCopyHasChanges`，仅在有变更时将本地影子副本拷贝到 FRAM 全局结构体并调用 `FRAM_WriteData()`。写入操作在临界区内进行。

---

#### REQ-APP_ENGINE_SYS_MON_006 — FRAM 结构体拷贝

**编号** | REQ-APP_ENGINE_SYS_MON_006
**优先级** | 中
**函数** | `SYSM_CopyFramStruct()`

**描述**：系统应提供 FRAM 结构体的逐字段拷贝函数，显式复制每个成员以确保跨 C 版本兼容性，避免使用 `memcpy` 可能带来的对齐或版本兼容问题。

---

### 2.4 违规状态查询与清除

#### REQ-APP_ENGINE_SYS_MON_007 — 时序违规状态查询

**编号** | REQ-APP_ENGINE_SYS_MON_007
**优先级** | 中
**函数** | `SYSM_GetRecordedTimingViolations()`

**描述**：系统应提供查询已记录的时序违规状态的接口。从 FRAM 读取当前违规记录，若 `anyTimingIssueOccurred == false` 则所有违规标志为 false；否则逐一判断各任务的违规时长和进入时间戳是否非零。

---

#### REQ-APP_ENGINE_SYS_MON_008 — 时序违规清除

**编号** | REQ-APP_ENGINE_SYS_MON_008
**优先级** | 中
**函数** | `SYSM_ClearAllTimingViolations()`

**描述**：系统应提供清除所有时序违规的功能：
1. 对所有受监控任务发送 `DIAG_EVENT_OK` 清除诊断状态
2. 清零本地 FRAM 影子副本中的所有违规字段
3. 设置变更标志并立即提交到 FRAM

---

## 3. 非功能需求

| 需求编号 | 描述 |
|----------|------|
| REQ-APP_ENGINE_SYS_MON_NFR_001 | FRAM 写入应延迟批量执行，避免频繁写入影响性能 |
| REQ-APP_ENGINE_SYS_MON_NFR_002 | 通知和 FRAM 操作应在临界区内进行以保证数据一致性 |

---

## 4. 追溯矩阵

| 需求编号 | 源文件 | 函数 |
|----------|--------|------|
| REQ-APP_ENGINE_SYS_MON_001 | sys_mon.c | `SYSM_Initialize()` |
| REQ-APP_ENGINE_SYS_MON_002 | sys_mon.c | `SYSM_Notify()` |
| REQ-APP_ENGINE_SYS_MON_003 | sys_mon.c | `SYSM_CheckNotifications()` |
| REQ-APP_ENGINE_SYS_MON_004 | sys_mon.c | `SYSM_RecordTimingViolation()` |
| REQ-APP_ENGINE_SYS_MON_005 | sys_mon.c | `SYSM_UpdateFramData()` |
| REQ-APP_ENGINE_SYS_MON_006 | sys_mon.c | `SYSM_CopyFramStruct()` |
| REQ-APP_ENGINE_SYS_MON_007 | sys_mon.c | `SYSM_GetRecordedTimingViolations()` |
| REQ-APP_ENGINE_SYS_MON_008 | sys_mon.c | `SYSM_ClearAllTimingViolations()` |
