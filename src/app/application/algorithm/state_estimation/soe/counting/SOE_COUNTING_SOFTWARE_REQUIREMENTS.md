# SOE_COUNTING — 软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 SOE（State of Energy，能量状态）库仑计数法估算模块的软件需求规格。该模块通过能量计数（EC）和功率积分两种方式估算电池的 SOE（0% ~ 100%）。

### 1.2 范围

- **涵盖**：SOE 初始化（FRAM 恢复）、基于能量计数器的 SOE 估算、基于功率积分（I×V×Δt）的 SOE 估算、电压-SOE 查表重校准、SOE 限幅、百分比↔Wh 双向转换
- **不涵盖**：电压/电流原始数据采集

### 1.3 定义与缩略语

| 术语 | 说明 |
|------|------|
| SOE | State of Energy，能量状态（0% ~ 100%） |
| EC | Energy Counting，能量计数 |
| FRAM | Ferroelectric RAM，非易失性存储器 |
| LUT | Look-Up Table，查找表 |

### 1.4 参考文献

- [soe_counting.c](soe_counting.c) — SOE 能量计数算法实现
- [soe_counting_cfg.h](soe_counting_cfg.h) — SOE 配置头文件

## 2. 总体描述

### 2.1 产品视角

SOE_COUNTING 模块是状态估计框架中 SOE 算法的具体实现，实现 `state_estimation.h` 中定义的 SOE API 接口。支持两种模式：EC 传感器模式（使用传感器提供的能量计数值）和功率积分模式（手动积分 I×V×Δt）。

## 3. 功能需求

### 3.1 SOE 初始化

#### REQ-001 — FRAM 持久化 SOE 恢复

**编号** | **优先级** | **函数**
REQ-001 | 高 | `SE_InitializeStateOfEnergy()`

**描述**：系统应从 FRAM 读取持久化的 SOE 值（average、minimum、maximum）、充放电能量吞吐量，恢复到数据库结构体，执行 [0.0, 100.0] 限幅检查，并计算对应的 Wh 值。

#### REQ-002 — EC 传感器模式初始化

**编号** | **优先级** | **函数**
REQ-002 | 高 | `SE_InitializeStateOfEnergy()`

**描述**：当 EC 传感器可用（`ec_present == true`）时，系统应读取能量计数器当前值，计算缩放偏移量（ecOffset），结合 FRAM 中的 SOE 值设置 `ecScalingAverage/Minimum/Maximum`。

### 3.2 SOE 周期计算

#### REQ-003 — 静态重校准

**编号** | **优先级** | **函数**
REQ-003 | 高 | `SOE_RecalibrateViaLookupTable()`

**描述**：当 BMS 处于静止状态（`BMS_AT_REST`）时，系统应通过电压-SOE 查表对 SOE 进行重校准，读取 min/max/average 电压并转换为 SOE 百分比。

#### REQ-004 — 功率积分 SOE 更新

**编号** | **优先级** | **函数**
REQ-004 | 高 | `SE_CalculateStateOfEnergy()`

**描述**：在非 EC 模式下，系统应检测电流时间戳变化，计算能量增量（deltaSOE_Wh = (I×V/1000/1000) × Δt / 3600），更新 SOE 的 Wh 值和百分比值，并累积充放电能量吞吐量。

#### REQ-005 — 能量计数 SOE 更新

**编号** | **优先级** | **函数**
REQ-005 | 高 | `SE_CalculateStateOfEnergy()`

**描述**：在 EC 传感器模式下，系统应检测 EC 时间戳变化，计算 SOE 增量（deltaSoe_perc = (EC_Wh / stringEnergy_Wh) × 100），通过缩放偏移转换为实际 SOE 百分比。

### 3.3 SOE 数值管理

#### REQ-006 — SOE 百分比限幅

**编号** | **优先级** | **函数**
REQ-006 | 高 | `SOE_CheckDatabaseSoePercentageLimits()`

**描述**：系统应确保 SOE 百分比值（average、minimum、maximum）保持在 [0.0, 100.0] 范围内。

#### REQ-007 — 百分比↔Wh 双向转换

**编号** | **优先级** | **函数**
REQ-007 | 中 | `SOE_GetStringSoePercentageFromEnergy()`, `SOE_GetStringEnergyFromSoePercentage()`

**描述**：系统应提供 SOE 百分比与能量（Wh）之间的双向转换：`soe_perc = (energy_Wh / SOE_STRING_ENERGY_Wh) × 100` 和 `energy_Wh = SOE_STRING_ENERGY_Wh × (soe_perc / 100)`。

#### REQ-008 — 电压-SOE 查表插值

**编号** | **优先级** | **函数**
REQ-008 | 中 | `SOE_GetFromVoltage()`

**描述**：系统应使用 `bc_stateOfEnergyLookupTable` 和 `MATH_LinearInterpolation()` 进行电压→SOE 线性插值，电压超出范围时返回边界值（0.0% 或 100.0%）。

### 3.4 配置参数

#### REQ-009 — 电池串能量配置

**编号** | **优先级** | **宏定义**
REQ-009 | 高 | `SOE_CELL_ENERGY_Wh`, `SOE_STRING_ENERGY_Wh`

**描述**：系统应在编译期计算电池串总能量：`SOE_STRING_ENERGY_Wh = BC_ENERGY_Wh × (BS_NR_OF_CELL_BLOCKS_PER_STRING × BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK)`。

## 4. 非功能需求

#### REQ-010 — 未初始化保护

**编号** | **优先级**
REQ-010 | 中

**描述**：若 `soe_state.soeInitialized == false`，`SE_CalculateStateOfEnergy()` 应立即退出。

## 5. 追溯矩阵

| 需求编号 | 需求描述 | 函数/宏 | 文件 |
|----------|---------|--------|------|
| REQ-001 | FRAM 持久化恢复 | `SE_InitializeStateOfEnergy()` | [soe_counting.c](soe_counting.c) |
| REQ-002 | EC 传感器初始化 | `SE_InitializeStateOfEnergy()` | [soe_counting.c](soe_counting.c) |
| REQ-003 | 静态重校准 | `SOE_RecalibrateViaLookupTable()` | [soe_counting.c](soe_counting.c) |
| REQ-004 | 功率积分 SOE 更新 | `SE_CalculateStateOfEnergy()` | [soe_counting.c](soe_counting.c) |
| REQ-005 | 能量计数 SOE 更新 | `SE_CalculateStateOfEnergy()` | [soe_counting.c](soe_counting.c) |
| REQ-006 | SOE 百分比限幅 | `SOE_CheckDatabaseSoePercentageLimits()` | [soe_counting.c](soe_counting.c) |
| REQ-007 | 百分比↔Wh 转换 | `SOE_GetStringSoePercentageFromEnergy()`, `SOE_GetStringEnergyFromSoePercentage()` | [soe_counting.c](soe_counting.c) |
| REQ-008 | 电压-SOE 查表插值 | `SOE_GetFromVoltage()` | [soe_counting.c](soe_counting.c) |
| REQ-009 | 电池串能量配置 | `SOE_CELL_ENERGY_Wh`, `SOE_STRING_ENERGY_Wh` | [soe_counting_cfg.h](soe_counting_cfg.h) |
| REQ-010 | 未初始化保护 | `SE_CalculateStateOfEnergy()` | [soe_counting.c](soe_counting.c) |
