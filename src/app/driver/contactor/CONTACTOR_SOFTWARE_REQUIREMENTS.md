# CONTACTOR — 接触器驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 Contactor（接触器）驱动模块的软件需求。Contactor 模块管理电池组主回路中的所有接触器（主正、主负、预充电、充电接触器等），控制其开闭并检测反馈状态。

### 1.2 范围
- **涵盖**：接触器开闭控制、预充电管理、反馈状态检测与诊断、全量急停
- **不涵盖**：接触器硬件驱动电路

## 2. 功能需求

#### REQ-001 — 接触器状态获取
**优先级**：高 | **函数**：`CONT_GetContactorState()`

模块应返回指定电池串中指定类型接触器的电气状态（闭合/断开）。

#### REQ-002 — 接触器闭合/断开
**优先级**：高 | **函数**：`CONT_CloseContactor()`, `CONT_OpenContactor()`

模块应向指定接触器发出闭合或断开的状态请求。

#### REQ-003 — 预充电管理
**优先级**：高 | **函数**：`CONT_ClosePrecharge()`, `CONT_OpenPrecharge()`

模块应支持预充电接触器的独立控制。

#### REQ-004 — 全量预充电断开
**优先级**：中 | **函数**：`CONT_OpenAllPrechargeContactors()`

模块应遍历所有接触器，断开所有已闭合的预充电接触器。

#### REQ-005 — 全量急停
**优先级**：高 | **函数**：`CONT_OpenAllContactors()`

模块应断开所有接触器（紧急停机）。

#### REQ-006 — 反馈校验与诊断
**优先级**：高 | **函数**：`CONT_CheckFeedback()`

模块应检查所有接触器的反馈信号是否与实际设置值一致，不匹配时触发 DIAG 诊断条目。

#### REQ-007 — 接触器模块初始化
**优先级**：高 | **函数**：`CONT_Initialize()`

## 3. 接口需求
| 函数 | 说明 |
|------|------|
| `CONT_GetContactorState(str, type)` | 获取接触器状态 |
| `CONT_Close/OpenContactor(str, type)` | 开闭接触器 |
| `CONT_Close/OpenPrecharge(str)` | 预充电控制 |
| `CONT_OpenAllPrechargeContactors()` | 断开所有预充电 |
| `CONT_OpenAllContactors()` | 急停全部断开 |
| `CONT_CheckFeedback()` | 反馈诊断 |
| `CONT_Initialize()` | 初始化 |
