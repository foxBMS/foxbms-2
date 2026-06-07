# SOC_LOOKUP_TABLE — 软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 SOC 查表法（Lookup Table）估算模块的软件需求规格。该模块通过单体电压→SOC 查找表直接映射来估算电池的 SOC，不依赖于电流传感器数据。

> 注：本模块的配置需求文档见 [SOC_LOOKUP_TABLE_CFG_REQUIREMENTS.md](SOC_LOOKUP_TABLE_CFG_REQUIREMENTS.md)。

### 1.2 范围

- **涵盖**：基于电压-SOC 查表的 SOC 估算、数据时间戳变化检测、SOC 限幅、FRAM 持久化
- **不涵盖**：配置宏定义（由 soc_lookup-table_cfg.h 和已有 CFG 需求文档覆盖）

### 1.3 参考文献

- [soc_lookup-table.c](soc_lookup-table.c) — SOC 查表法实现
- [soc_lookup-table_cfg.h](soc_lookup-table_cfg.h) — SOC 查表法配置
- [SOC_LOOKUP_TABLE_CFG_REQUIREMENTS.md](SOC_LOOKUP_TABLE_CFG_REQUIREMENTS.md) — 已有配置需求文档

## 2. 总体描述

### 2.1 产品视角

SOC_LOOKUP_TABLE 模块是 SOC 算法的一种可替换实现，通过检测单体电压最小/最大值数据库块的时间戳变化，使用 `SE_GetStateOfChargeFromVoltage()` 将电压值转换为 SOC 百分比。该模块不使用电流传感器数据。

### 2.2 工作模式

模块通过比较 `soc_tableMinMax.header.timestamp` 与 `soc_state.previousTimestamp` 来检测新的电压数据。当检测到新数据时，分别将最小、最大、平均单体电压转换为 SOC 百分比。

## 3. 功能需求

### 3.1 SOC 初始化

#### REQ-001 — SOC 查表法初始化

**编号** | **优先级** | **函数**
REQ-001 | 高 | `SE_InitializeStateOfCharge()`

**描述**：系统应验证参数有效性（`pSocValues != NULL_PTR`、`stringNumber < BS_NR_OF_STRINGS`）后设置 `soc_state.socInitialized = true`，标记 SOC 已初始化。

### 3.2 SOC 周期计算

#### REQ-002 — 时间戳变化检测

**编号** | **优先级** | **函数**
REQ-002 | 高 | `SE_CalculateStateOfCharge()`

**描述**：系统应在每次调用时读取最小/最大值数据库块，通过比较 `soc_tableMinMax.header.timestamp` 与 `soc_state.previousTimestamp` 检测是否有新的电压数据。

#### REQ-003 — 电压→SOC 查表映射

**编号** | **优先级** | **函数**
REQ-003 | 高 | `SE_CalculateStateOfCharge()`

**描述**：当检测到新电压数据时，系统应对每个电池串分别调用 `SE_GetStateOfChargeFromVoltage()`，将最小单体电压、最大单体电压和平均单体电压转换为 SOC 百分比，作为 min/max/average SOC 值。

#### REQ-004 — SOC 限幅与持久化

**编号** | **优先级** | **函数**
REQ-004 | 中 | `SOC_CheckDatabaseSocPercentageLimits()`, `SOC_UpdateNvmValues()`

**描述**：系统应在每次更新 SOC 值后执行 [0.0, 100.0] 限幅检查，并将结果同步到 FRAM 非易失性存储。

### 3.3 电压→SOC 转换

#### REQ-005 — 电压-SOC 查表插值

**编号** | **优先级** | **函数**
REQ-005 | 高 | `SE_GetStateOfChargeFromVoltage()`

**描述**：系统应根据单体电压在 `bc_stateOfChargeLookupTable` 中查找并进行线性插值，返回 SOC 百分比。查找表按电压降序排列。电压超出表范围时不进行外推，返回边界值（0.0% 或 100.0%）。

## 4. 非功能需求

#### REQ-006 — 接口兼容性

**编号** | **优先级**
REQ-006 | 中

**描述**：系统应实现 `state_estimation.h` 中定义的完整 SOC API 接口（初始化、计算、电压查表），确保与其他 SOC 实现可在编译期互换。

#### REQ-007 — 单元测试支持

**编号** | **优先级**
REQ-007 | 低

**描述**：系统应在 `UNITY_UNIT_TEST` 模式下暴露 `TEST_SE_GetSocStateInitialized()`、`TEST_SOC_CheckDatabaseSocPercentageLimits()`、`TEST_SOC_UpdateNvmValues()` 测试函数。

## 5. 追溯矩阵

| 需求编号 | 需求描述 | 函数 | 文件 |
|----------|---------|------|------|
| REQ-001 | SOC 查表法初始化 | `SE_InitializeStateOfCharge()` | [soc_lookup-table.c](soc_lookup-table.c) |
| REQ-002 | 时间戳变化检测 | `SE_CalculateStateOfCharge()` | [soc_lookup-table.c](soc_lookup-table.c) |
| REQ-003 | 电压→SOC 查表映射 | `SE_CalculateStateOfCharge()` | [soc_lookup-table.c](soc_lookup-table.c) |
| REQ-004 | SOC 限幅与持久化 | `SOC_CheckDatabaseSocPercentageLimits()`, `SOC_UpdateNvmValues()` | [soc_lookup-table.c](soc_lookup-table.c) |
| REQ-005 | 电压-SOC 查表插值 | `SE_GetStateOfChargeFromVoltage()` | [soc_lookup-table.c](soc_lookup-table.c) |
| REQ-006 | 接口兼容性 | 全部接口函数 | [soc_lookup-table.c](soc_lookup-table.c) |
| REQ-007 | 单元测试支持 | `TEST_*` 函数 | [soc_lookup-table.c](soc_lookup-table.c) |
