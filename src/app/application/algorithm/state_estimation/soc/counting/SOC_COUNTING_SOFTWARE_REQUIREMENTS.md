# SOC_COUNTING — 软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 SOC（State of Charge，荷电状态）库仑计数法估算模块的软件需求规格。该模块通过电流积分（库仑计数/安时积分）和电压查表重校准两种方法估算电池的 SOC。

### 1.2 范围

- **涵盖**：SOC 初始化（FRAM 持久化恢复）、基于电流传感器 CC 值的库仑计数 SOC 估算、基于电流积分的 SOC 估算、基于电压-SOC 查表的重校准、SOC 限幅、非易失性存储更新
- **不涵盖**：SOC 查表法实现（由 lookup-table 子模块实现）、电压/电流原始数据采集

### 1.3 定义与缩略语

| 术语 | 说明 |
|------|------|
| SOC | State of Charge，荷电状态（0% ~ 100%） |
| CC | Coulomb Counting，库仑计数 |
| FRAM | Ferroelectric RAM，非易失性铁电存储器 |
| LUT | Look-Up Table，查找表 |
| MOL/RSL/MSL | Maximum Operating Limit / Recommended Safety Limit / Maximum Safety Limit |

### 1.4 参考文献

- [soc_counting.c](soc_counting.c) — SOC 库仑计数算法实现
- [soc_counting_cfg.h](soc_counting_cfg.h) — SOC 配置头文件

## 2. 总体描述

### 2.1 产品视角

SOC_COUNTING 模块是状态估计框架中 SOC 算法的具体实现之一。它实现 `state_estimation.h` 中定义的 SOC API 接口（`SE_InitializeStateOfCharge`、`SE_CalculateStateOfCharge`、`SE_GetStateOfChargeFromVoltage`），通过库仑计数和电流积分两种方式估算 SOC。模块维护内部状态结构体 `SOC_STATE_s` 和 FRAM 持久化数据。

### 2.2 工作模式

模块支持两种电流数据源：
- **CC 传感器模式**（`sensorCcUsed = true`）：使用电流传感器提供的库仑计数值进行 SOC 估算
- **电流积分模式**（`sensorCcUsed = false`）：通过对电流测量值进行时间积分来估算 SOC

当 BMS 处于静止状态（`BMS_AT_REST`）时，通过电压-SOC 查表进行重校准。

### 2.3 用户特征

本模块由状态估计包装器（`state_estimation.c`）通过标准 API 调用。

## 3. 功能需求

### 3.1 SOC 初始化

#### REQ-001 — FRAM 持久化 SOC 恢复

**编号** | **优先级** | **函数**
REQ-001 | 高 | `SE_InitializeStateOfCharge()`

**描述**：系统应在 SOC 初始化时从 FRAM 读取持久化的 SOC 值（average、minimum、maximum）、充放电吞吐量，恢复到数据库结构体中。应对恢复后的 SOC 百分比进行 [0.0, 100.0] 限幅检查。

**前置条件**：`pSocValues != NULL_PTR`, `stringNumber < BS_NR_OF_STRINGS`
**后置条件**：数据库 SOC 值已从 FRAM 恢复，`soc_state.socInitialized = true`

#### REQ-002 — CC 传感器模式初始化

**编号** | **优先级** | **函数**
REQ-002 | 高 | `SE_InitializeStateOfCharge()`

**描述**：当电流传感器支持 CC 功能（`ccPresent == true`）时，系统应启用 CC 传感器模式，根据当前库仑计数值计算缩放偏移量，用于后续 SOC 百分比转换。

**处理流程**：
1. 设置 `sensorCcUsed[stringNumber] = true`
2. 保存当前 CC 值到 `previousCurrentCountingValue_As`
3. 计算缩放偏移：`scalingOffset_perc = (abs(CC) / stringCapacity_As) * 100`
4. 结合 FRAM 中持久化的 SOC 值计算 `ccScalingAverage/Minimum/Maximum`

#### REQ-003 — 电流积分模式初始化

**编号** | **优先级** | **函数**
REQ-003 | 中 | `SE_InitializeStateOfCharge()`

**描述**：当电流传感器不支持 CC 功能（`ccPresent == false`）时，系统应使用电流积分模式，记录当前时间戳作为后续增量计算的基准。

### 3.2 SOC 周期计算

#### REQ-004 — 静态重校准

**编号** | **优先级** | **函数**
REQ-004 | 高 | `SE_CalculateStateOfCharge()`, `SOC_RecalibrateViaLookupTable()`

**描述**：当 BMS 处于静止状态（`BMS_AT_REST`）时，系统应通过电压-SOC 查表对 SOC 进行重校准。读取单体电压最小值、最大值、平均值，分别通过 `SE_GetStateOfChargeFromVoltage()` 转换为 SOC 百分比，更新数据库和 FRAM。

#### REQ-005 — 电流积分 SOC 更新

**编号** | **优先级** | **函数**
REQ-005 | 高 | `SE_CalculateStateOfCharge()`

**描述**：在电流积分模式下，系统应检测电流数据时间戳变化，计算时间步长和 SOC 增量（deltaSOC_perc = (current_mA * timeStep_s / capacity_mAs) * 100），更新 average/minimum/maximum SOC 值，并累计充放电吞吐量。

**处理流程**：
1. 检测 `previousTimestamp != currentTimestamp`
2. 计算 `timeStep_s` 和 `deltaSOC_perc`
3. 根据电流方向（`BS_CURRENT_DIRECTION_FLOAT`）调整 delta 符号
4. 更新三种 SOC 值（average/minimum/maximum）
5. 根据充放电方向累积 `chargeThroughput_As` 或 `dischargeThroughput_As`
6. 限幅检查并更新 NVM

#### REQ-006 — 库仑计数 SOC 更新

**编号** | **优先级** | **函数**
REQ-006 | 高 | `SE_CalculateStateOfCharge()`

**描述**：在 CC 传感器模式下，系统应检测 CC 数据时间戳变化，计算 SOC 增量（deltaSoc_perc = (CC_As / capacity_As) * 100），通过缩放偏移转换为实际 SOC 百分比，并累计充放电吞吐量。

### 3.3 SOC 数值管理

#### REQ-007 — SOC 百分比限幅

**编号** | **优先级** | **函数**
REQ-007 | 高 | `SOC_CheckDatabaseSocPercentageLimits()`

**描述**：系统应确保所有 SOC 百分比值（average、minimum、maximum）保持在 [0.0, 100.0] 范围内。超出上限时截断为 100.0%，低于下限时截断为 0.0%。

#### REQ-008 — 非易失性存储更新

**编号** | **优先级** | **函数**
REQ-008 | 中 | `SOC_UpdateNvmValues()`

**描述**：系统应在 SOC 值变化后同步更新 FRAM 中的持久化副本，包括 SOC 百分比值和充放电吞吐量，确保系统重启后能恢复正确的 SOC。

#### REQ-009 — SOC 值设置

**编号** | **优先级** | **函数**
REQ-009 | 中 | `SOC_SetValue()`

**描述**：系统应提供统一的 SOC 值设置函数，同时更新数据库和 FRAM，若启用 CC 传感器还需重新计算缩放偏移量（考虑当前 CC 值和电流方向），并进行限幅检查和 NVM 更新。

### 3.4 电压→SOC 转换

#### REQ-010 — 电压-SOC 查表插值

**编号** | **优先级** | **函数**
REQ-010 | 高 | `SE_GetStateOfChargeFromVoltage()`

**描述**：系统应根据单体电压值在预定义的电压-SOC 查找表（`bc_stateOfChargeLookupTable`）中进行查找和线性插值，返回对应的 SOC 百分比。查找表按电压降序排列，电压超出表范围时返回边界值（0.0% 或 100.0%）。

**处理流程**：
1. 遍历查找表（从索引 1 开始，避免外推）
2. 找到电压所在的区间（between_low / between_high）
3. 若在范围内：`MATH_LinearInterpolation()` 线性插值
4. 若低于最低电压：返回 `SOC_MINIMUM_SOC_perc` (0.0%)
5. 若高于最高电压：返回 100.0%

### 3.5 辅助计算

#### REQ-011 — 安时→百分比转换

**编号** | **优先级** | **函数**
REQ-011 | 中 | `SOC_GetStringSocPercentageFromCharge(uint32_t charge_As)`

**描述**：系统应将安时（As）单位的电荷量转换为 SOC 百分比：`soc_perc = (charge_mAs / SOC_STRING_CAPACITY_mAs) * 100`。

### 3.6 配置参数

#### REQ-012 — 电池串容量配置

**编号** | **优先级** | **宏定义**
REQ-012 | 高 | `SOC_STRING_CAPACITY_mAh`, `SOC_STRING_CAPACITY_mAs`, `SOC_STRING_CAPACITY_As`

**描述**：系统应在编译期根据电池配置参数计算电池串容量：`SOC_STRING_CAPACITY_mAh = BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK * BC_CAPACITY_mAh`，并提供 mAs 和 As 两种单位的换算。

## 4. 非功能需求

### 4.1 鲁棒性

#### REQ-013 — 未初始化保护

**编号** | **优先级**
REQ-013 | 中

**描述**：若 SOC 尚未完成初始化（`soc_state.socInitialized == false`），`SE_CalculateStateOfCharge()` 应立即退出，不执行任何计算。

#### REQ-014 — 零时间步长保护

**编号** | **优先级**
REQ-014 | 中

**描述**：在电流积分模式下，若计算出的时间步长 `timeStep_s <= 0.0f`，应跳过本次更新，避免除零或无效计算。

## 5. 接口需求

### 5.1 公共 API（实现 state_estimation.h 接口）

| 函数 | 用途 |
|------|------|
| `SE_InitializeStateOfCharge()` | SOC 初始化 |
| `SE_CalculateStateOfCharge()` | SOC 周期计算 |
| `SE_GetStateOfChargeFromVoltage()` | 电压→SOC 查表转换 |

### 5.2 依赖项

| 依赖模块 | 用途 |
|----------|------|
| `database.h` | 电流/CC 数据读取 |
| `fram.h` | FRAM 非易失性存储读写 |
| `bms.h` | 电池系统状态查询、电流方向判断 |
| `foxmath.h` | 线性插值 |
| `state_estimation.h` | API 接口声明 |
| `battery_cell_cfg.h` / `battery_system_cfg.h` | 电池配置参数 |

## 6. 数据结构

### 6.1 SOC_STATE_s

| 字段 | 类型 | 说明 |
|------|------|------|
| `socInitialized` | `bool` | SOC 初始化完成标志 |
| `sensorCcUsed[]` | `bool[BS_NR_OF_STRINGS]` | CC 传感器使用标志 |
| `ccScalingAverage[]` | `float_t[]` | 平均 SOC 的 CC 缩放偏移 |
| `ccScalingMinimum[]` | `float_t[]` | 最小 SOC 的 CC 缩放偏移 |
| `ccScalingMaximum[]` | `float_t[]` | 最大 SOC 的 CC 缩放偏移 |
| `chargeThroughput_As[]` | `float_t[]` | 充电吞吐量 (As) |
| `dischargeThroughput_As[]` | `float_t[]` | 放电吞吐量 (As) |
| `previousCurrentCountingValue_As[]` | `float_t[]` | 上次 CC 值 |
| `previousTimestamp[]` | `uint32_t[]` | 上次时间戳 |

## 7. 控制流

```
SE_CalculateStateOfCharge()
    │
    ├── socInitialized == false? → 退出
    │
    ├── BMS_AT_REST? → SOC_RecalibrateViaLookupTable() (REQ-004)
    │   └── 读 min/max 电压 → SE_GetStateOfChargeFromVoltage() → SOC_SetValue()
    │
    └── 非静止状态
        ├── sensorCcUsed == false? → 电流积分模式 (REQ-005)
        │   ├── 检测 timestamp 变化
        │   ├── 计算 deltaSOC_perc = (I * Δt / capacity) * 100
        │   └── 更新 SOC 值和吞吐量
        │
        └── sensorCcUsed == true? → CC 模式 (REQ-006)
            ├── 检测 timestamp 变化
            ├── 计算 deltaSoc_perc = (CC / capacity) * 100
            └── SOC = ccScaling - deltaSoc_perc
```

## 8. 追溯矩阵

| 需求编号 | 需求描述 | 函数/宏 | 文件 |
|----------|---------|--------|------|
| REQ-001 | FRAM 持久化 SOC 恢复 | `SE_InitializeStateOfCharge()` | [soc_counting.c](soc_counting.c) |
| REQ-002 | CC 传感器模式初始化 | `SE_InitializeStateOfCharge()` | [soc_counting.c](soc_counting.c) |
| REQ-003 | 电流积分模式初始化 | `SE_InitializeStateOfCharge()` | [soc_counting.c](soc_counting.c) |
| REQ-004 | 静态重校准 | `SOC_RecalibrateViaLookupTable()` | [soc_counting.c](soc_counting.c) |
| REQ-005 | 电流积分 SOC 更新 | `SE_CalculateStateOfCharge()` | [soc_counting.c](soc_counting.c) |
| REQ-006 | 库仑计数 SOC 更新 | `SE_CalculateStateOfCharge()` | [soc_counting.c](soc_counting.c) |
| REQ-007 | SOC 百分比限幅 | `SOC_CheckDatabaseSocPercentageLimits()` | [soc_counting.c](soc_counting.c) |
| REQ-008 | 非易失性存储更新 | `SOC_UpdateNvmValues()` | [soc_counting.c](soc_counting.c) |
| REQ-009 | SOC 值设置 | `SOC_SetValue()` | [soc_counting.c](soc_counting.c) |
| REQ-010 | 电压-SOC 查表插值 | `SE_GetStateOfChargeFromVoltage()` | [soc_counting.c](soc_counting.c) |
| REQ-011 | 安时→百分比转换 | `SOC_GetStringSocPercentageFromCharge()` | [soc_counting.c](soc_counting.c) |
| REQ-012 | 电池串容量配置 | `SOC_STRING_CAPACITY_*` | [soc_counting_cfg.h](soc_counting_cfg.h) |
| REQ-013 | 未初始化保护 | `SE_CalculateStateOfCharge()` | [soc_counting.c](soc_counting.c) |
| REQ-014 | 零时间步长保护 | `SE_CalculateStateOfCharge()` | [soc_counting.c](soc_counting.c) |
