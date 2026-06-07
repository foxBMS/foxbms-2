# SOC_DEBUG — 软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 SOC 调试模式（Debug）模块的软件需求规格。该模块是 SOC 估算的桩实现（Stub），用于调试和开发阶段，不执行实际的 SOC 计算。

### 1.2 范围

- **涵盖**：SOC 接口的调试桩实现、参数有效性断言
- **不涵盖**：实际的 SOC 估算逻辑

### 1.3 参考文献

- [soc_debug.c](soc_debug.c) — SOC 调试桩实现

## 2. 总体描述

### 2.1 产品视角

SOC_DEBUG 模块是 SOC 算法的一种可替换实现，实现了 `state_estimation.h` 中定义的所有 SOC API 接口。所有函数仅进行参数有效性断言，不执行实际计算。该模块用于调试场景或不需要 SOC 估算的简化配置。

### 2.2 工作模式

所有对外接口函数仅检查参数有效性后立即返回，其中 `SE_GetStateOfChargeFromVoltage()` 始终返回固定值 50%。

## 3. 功能需求

#### REQ-001 — SOC 初始化桩

**编号** | **优先级** | **函数**
REQ-001 | 中 | `SE_InitializeStateOfCharge()`

**描述**：系统应提供 SOC 初始化的空实现，仅验证 `pSocValues != NULL_PTR` 和 `stringNumber < BS_NR_OF_STRINGS`，不执行任何实际初始化操作。

#### REQ-002 — SOC 计算桩

**编号** | **优先级** | **函数**
REQ-002 | 中 | `SE_CalculateStateOfCharge()`

**描述**：系统应提供 SOC 周期计算的空实现，仅验证 `pSocValues != NULL_PTR`，不更新任何 SOC 值。

#### REQ-003 — 电压→SOC 固定返回值

**编号** | **优先级** | **函数**
REQ-003 | 中 | `SE_GetStateOfChargeFromVoltage()`

**描述**：系统应在调试模式下始终返回固定的 SOC 值 50.0%（0.50f），不执行查表或插值运算。

## 4. 非功能需求

#### REQ-004 — 接口兼容性

**编号** | **优先级**
REQ-004 | 中

**描述**：系统应保持与 `state_estimation.h` 中定义的 SOC API 接口完全兼容，确保可以在编译期与其他 SOC 实现互换。

## 5. 追溯矩阵

| 需求编号 | 需求描述 | 函数 | 文件 |
|----------|---------|------|------|
| REQ-001 | SOC 初始化桩 | `SE_InitializeStateOfCharge()` | [soc_debug.c](soc_debug.c) |
| REQ-002 | SOC 计算桩 | `SE_CalculateStateOfCharge()` | [soc_debug.c](soc_debug.c) |
| REQ-003 | 电压→SOC 固定返回值 | `SE_GetStateOfChargeFromVoltage()` | [soc_debug.c](soc_debug.c) |
| REQ-004 | 接口兼容性 | 全部 3 个接口函数 | [soc_debug.c](soc_debug.c) |
