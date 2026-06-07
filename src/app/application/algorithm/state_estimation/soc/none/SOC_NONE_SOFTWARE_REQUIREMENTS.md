# SOC_NONE — 软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 SOC 空实现（None）模块的软件需求规格。该模块是 SOC 估算的空实现，用于不需要 SOC 估算功能的配置场景。

### 1.2 范围

- **涵盖**：SOC 接口的空实现、参数有效性断言
- **不涵盖**：任何实际的 SOC 估算逻辑

### 1.3 参考文献

- [soc_none.c](soc_none.c) — SOC 空实现

## 2. 总体描述

### 2.1 产品视角

SOC_NONE 模块是 SOC 算法的空实现，实现了 `state_estimation.h` 中定义的所有 SOC API 接口。所有函数仅进行参数有效性断言，不执行任何计算。`SE_GetStateOfChargeFromVoltage()` 始终返回 0.0%。

## 3. 功能需求

#### REQ-001 — SOC 初始化空实现

**编号** | **优先级** | **函数**
REQ-001 | 中 | `SE_InitializeStateOfCharge()`

**描述**：系统应提供 SOC 初始化的空实现，验证 `pSocValues != NULL_PTR`、`ccPresent` 为布尔值、`stringNumber < BS_NR_OF_STRINGS`，不执行任何实际初始化操作。

#### REQ-002 — SOC 计算空实现

**编号** | **优先级** | **函数**
REQ-002 | 中 | `SE_CalculateStateOfCharge()`

**描述**：系统应提供 SOC 周期计算的空实现，仅验证 `pSocValues != NULL_PTR`。

#### REQ-003 — 电压→SOC 零返回值

**编号** | **优先级** | **函数**
REQ-003 | 中 | `SE_GetStateOfChargeFromVoltage()`

**描述**：系统应始终返回 SOC 值 0.0%，不执行任何查表或计算。

## 4. 非功能需求

#### REQ-004 — 接口兼容性

**编号** | **优先级**
REQ-004 | 中

**描述**：系统应保持与 `state_estimation.h` 中定义的 SOC API 接口完全兼容，可在编译期替换。

## 5. 追溯矩阵

| 需求编号 | 需求描述 | 函数 | 文件 |
|----------|---------|------|------|
| REQ-001 | SOC 初始化空实现 | `SE_InitializeStateOfCharge()` | [soc_none.c](soc_none.c) |
| REQ-002 | SOC 计算空实现 | `SE_CalculateStateOfCharge()` | [soc_none.c](soc_none.c) |
| REQ-003 | 电压→SOC 零返回值 | `SE_GetStateOfChargeFromVoltage()` | [soc_none.c](soc_none.c) |
| REQ-004 | 接口兼容性 | 全部 3 个接口函数 | [soc_none.c](soc_none.c) |
