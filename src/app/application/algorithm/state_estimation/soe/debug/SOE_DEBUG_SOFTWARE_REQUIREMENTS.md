# SOE_DEBUG — 软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 SOE 调试模式（Debug）模块的软件需求规格。该模块是 SOE 估算的桩实现，用于调试和开发阶段。

### 1.2 参考文献

- [soe_debug.c](soe_debug.c) — SOE 调试桩实现
- [soe_debug_cfg.h](soe_debug_cfg.h) — SOE 调试配置

## 2. 功能需求

#### REQ-001 — SOE 初始化桩

**编号** | **优先级** | **函数**
REQ-001 | 中 | `SE_InitializeStateOfEnergy()`

**描述**：系统应验证参数 `pSoeValues != NULL_PTR` 和 `stringNumber < BS_NR_OF_STRINGS`，不执行实际初始化。

#### REQ-002 — SOE 计算桩

**编号** | **优先级** | **函数**
REQ-002 | 中 | `SE_CalculateStateOfEnergy()`

**描述**：系统应验证 `pSoeValues != NULL_PTR`，不执行实际计算。

#### REQ-003 — 调试能量配置

**编号** | **优先级** | **宏定义**
REQ-003 | 低 | `SOE_CELL_ENERGY_Wh`

**描述**：系统应定义调试用的单体能量常量 `SOE_CELL_ENERGY_Wh = 20000u`（Wh）。

## 3. 追溯矩阵

| 需求编号 | 需求描述 | 函数/宏 | 文件 |
|----------|---------|--------|------|
| REQ-001 | SOE 初始化桩 | `SE_InitializeStateOfEnergy()` | [soe_debug.c](soe_debug.c) |
| REQ-002 | SOE 计算桩 | `SE_CalculateStateOfEnergy()` | [soe_debug.c](soe_debug.c) |
| REQ-003 | 调试能量配置 | `SOE_CELL_ENERGY_Wh` | [soe_debug_cfg.h](soe_debug_cfg.h) |
