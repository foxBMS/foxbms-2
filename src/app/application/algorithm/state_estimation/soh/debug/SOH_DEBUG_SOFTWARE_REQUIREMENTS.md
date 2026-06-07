# SOH_DEBUG — 软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 SOH 调试模式（Debug）模块的软件需求规格。该模块提供 SOH（State of Health，健康状态）的调试用固定值实现。

> 注：SOH 的空实现需求文档见 [../none/SOH_NONE_REQUIREMENTS.md](../none/SOH_NONE_REQUIREMENTS.md)。

### 1.2 参考文献

- [soh_debug.c](soh_debug.c) — SOH 调试实现

## 2. 功能需求

#### REQ-001 — SOH 初始化调试值

**编号** | **优先级** | **函数**
REQ-001 | 中 | `SE_InitializeStateOfHealth()`

**描述**：系统应在调试模式下将 SOH 初始化为 0.0%（average=0.0, minimum=0.0, maximum=0.0）。验证 `pSohValues != NULL_PTR` 和 `stringNumber < BS_NR_OF_STRINGS`。

#### REQ-002 — SOH 计算固定调试值

**编号** | **优先级** | **函数**
REQ-002 | 中 | `SE_CalculateStateOfHealth()`

**描述**：系统应在调试模式下设置固定的 SOH 调试值：average=50.0%、minimum=49.9%、maximum=50.1%（仅对第一个电池串索引 0）。验证 `pSohValues != NULL_PTR`。

## 3. 追溯矩阵

| 需求编号 | 需求描述 | 函数 | 文件 |
|----------|---------|------|------|
| REQ-001 | SOH 初始化调试值 | `SE_InitializeStateOfHealth()` | [soh_debug.c](soh_debug.c) |
| REQ-002 | SOH 计算固定调试值 | `SE_CalculateStateOfHealth()` | [soh_debug.c](soh_debug.c) |
