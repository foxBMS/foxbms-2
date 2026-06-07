# INTERLOCK — 互锁驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 Interlock（互锁）驱动模块的软件需求。Interlock 模块管理高压互锁回路（HVIL）的检测，确保高压系统在安全条件下运行。

### 1.2 范围
- **涵盖**：互锁状态机、互锁回路电状态检测、初始化流程
- **不涵盖**：硬件互锁回路设计

## 2. 功能需求

#### REQ-001 — 状态机状态请求
**优先级**：高 | **函数**：`ILCK_SetStateRequest()`

模块应接受状态请求（如初始化）并校验请求合法性，非法请求被拒绝并增加 `ErrRequestCounter`。

#### REQ-002 — 状态机当前状态获取
**优先级**：高 | **函数**：`ILCK_GetState()`

#### REQ-003 — 周期性触发（1ms）
**优先级**：高 | **函数**：`ILCK_Trigger()`

模块应每 1ms 被调用一次，执行互锁状态机的事件序列。

## 3. 状态机

### 主状态
- `ILCK_STATEMACHINE_UNINITIALIZED` — 未初始化
- `ILCK_STATEMACHINE_INITIALIZED` — 已初始化
- `ILCK_STATEMACHINE_UNDEFINED` — 未定义

### 状态请求
- `ILCK_STATE_INITIALIZATION_REQUEST` — 初始化请求
- `ILCK_STATE_NO_REQUEST` — 无请求

### 返回类型
- `ILCK_OK` / `ILCK_REQUEST_PENDING` / `ILCK_ALREADY_INITIALIZED` / `ILCK_ILLEGAL_REQUEST`

## 4. 数据结构
`ILCK_STATE_s`：包含计时器、状态请求、主状态/子状态、上一次状态、错误计数、重入保护和通用计数器的完整状态结构。

## 5. 接口需求
| 函数 | 说明 |
|------|------|
| `ILCK_SetStateRequest(req)` | 状态请求 |
| `ILCK_GetState()` | 获取当前状态 |
| `ILCK_Trigger()` | 1ms 周期性触发 |
