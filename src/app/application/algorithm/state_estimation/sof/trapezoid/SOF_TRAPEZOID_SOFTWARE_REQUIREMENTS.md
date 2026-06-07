# SOF_TRAPEZOID — 软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 SOF（State of Function，功能状态）梯形降额法模块的软件需求规格。该模块根据电池单体的电压和温度，使用梯形降额曲线计算电池系统允许的最大充放电电流限制。

### 1.2 范围

- **涵盖**：SOF 降额曲线预计算、基于电压的电流限制计算、基于温度的电流限制计算（区分充放电方向和温度高低）、电压/温度约束的最小值合并、电池包级电流汇总、BMS 错误状态下的电流清零
- **不涵盖**：降额参数的具体配置值（由 sof_trapezoid_cfg 模块定义）

### 1.3 定义与缩略语

| 术语 | 说明 |
|------|------|
| SOF | State of Function，功能状态 |
| Derating | 降额，根据温度/电压条件降低允许的最大电流 |
| Limp Home | 跛行回家模式，故障时以最低电流维持行驶 |
| MOL/RSL/MSL | Maximum Operating Limit / Recommended Safety Limit / Maximum Safety Limit |
| Continuous/Peak | 持续电流 / 峰值电流 |
| ddegC | deci-degree Celsius，十分之一摄氏度 |

### 1.4 参考文献

- [sof_trapezoid.c](sof_trapezoid.c) — SOF 梯形降额算法实现
- [sof_trapezoid.h](sof_trapezoid.h) — SOF 数据结构与接口声明
- [sof_trapezoid_cfg.c](sof_trapezoid_cfg.c) — SOF 配置参数实例
- [sof_trapezoid_cfg.h](sof_trapezoid_cfg.h) — SOF 配置宏定义

## 2. 总体描述

### 2.1 产品视角

SOF_TRAPEZOID 模块是 foxBMS 2 中负责计算电池系统功能状态的核心模块。它不通过状态估计包装器调用，而是独立地被算法调度框架调用。模块使用梯形降额曲线方法：在正常范围内允许最大电流，在降额区间线性减少电流，在极限区间将电流限制为零或 limp home 值。

### 2.2 工作模式

模块分为两个阶段：
1. **初始化阶段**（`SOF_Init()`）：预计算降额曲线的斜率和偏移量
2. **运行阶段**（`SOF_Calculation()`）：每次调用时读取 min/max 电压和温度，逐串计算电流限制，取最保守值汇总为电池包级限制

降额曲线分为 8 个维度：低温放电、高温放电、低温充电、高温充电、上限电压（充电限制）、下限电压（放电限制）。

## 3. 功能需求

### 3.1 降额曲线预计算

#### REQ-001 — 降额曲线参数计算

**编号** | **优先级** | **函数**
REQ-001 | 高 | `SOF_CalculateCurves()`

**描述**：系统应在初始化阶段根据配置参数预计算所有降额曲线的斜率和偏移量。使用两点式线性方程（y = slope × x + offset），其中两点为 (limit, maxCurrent) 和 (cutoff, 0 或 limpHomeCurrent)。

**处理流程**：
1. 低温放电降额：slope = (maxDischargeCurrent - limpHomeCurrent) / (cutoffLow - limitLow)
2. 高温放电降额：slope = (0 - maxDischargeCurrent) / (limitHigh - cutoffHigh)
3. 低温充电降额：slope = (maxChargeCurrent - 0) / (cutoffLow - limitLow)
4. 高温充电降额：slope = (0 - maxChargeCurrent) / (limitHigh - cutoffHigh)
5. 上限电压（放电限制）：slope = (maxDischargeCurrent - 0) / (cutoffLower - limitLower)
6. 下限电压（充电限制）：slope = (maxChargeCurrent - 0) / (cutoffUpper - limitUpper)

### 3.2 电压约束电流限制

#### REQ-002 — 基于电压的电流限制计算

**编号** | **优先级** | **函数**
REQ-002 | 高 | `SOF_CalculateVoltageBasedCurrentLimit()`

**描述**：系统应根据最小单体电压计算允许的放电电流，根据最大单体电压计算允许的充电电流。

**处理流程**：
- **放电方向（最小电压）**：
  - 若 `minVoltage ≤ limitLower` → 放电电流 = 0
  - 若 `minVoltage ≤ cutoffLower` → 放电电流 = slope × (minVoltage - limitLower) （梯形降额）
  - 否则 → 放电电流 = maxDischargeCurrent
- **充电方向（最大电压）**：
  - 若 `maxVoltage ≥ limitUpper` → 充电电流 = 0
  - 若 `maxVoltage ≥ cutoffUpper` → 充电电流 = slope × (maxVoltage - limitUpper)
  - 否则 → 充电电流 = maxChargeCurrent

### 3.3 温度约束电流限制

#### REQ-003 — 低温放电降额

**编号** | **优先级** | **函数**
REQ-003 | 高 | `SOF_CalculateTemperatureBasedCurrentLimit()`

**描述**：系统应根据最小单体温度对放电电流进行低温降额：温度 ≤ limitLowDischarge 时限制为 limpHomeCurrent，在 (limit, cutoff] 区间线性降额，高于 cutoff 时允许最大电流。

#### REQ-004 — 低温充电降额

**编号** | **优先级** | **函数**
REQ-004 | 高 | `SOF_CalculateTemperatureBasedCurrentLimit()`

**描述**：系统应根据最小单体温度对充电电流进行低温降额：温度 ≤ limitLowCharge 时充电电流 = 0，在 (limit, cutoff] 区间线性增加，高于 cutoff 时允许最大电流。

#### REQ-005 — 高温放电降额

**编号** | **优先级** | **函数**
REQ-005 | 高 | `SOF_CalculateTemperatureBasedCurrentLimit()`

**描述**：系统应根据最大单体温度对放电电流进行高温降额：温度 ≥ limitHighDischarge 时放电电流 = 0，在 [cutoff, limit) 区间线性降额。取低温降额结果和高温降额结果中的较小值。

#### REQ-006 — 高温充电降额

**编号** | **优先级** | **函数**
REQ-006 | 高 | `SOF_CalculateTemperatureBasedCurrentLimit()`

**描述**：系统应根据最大单体温度对充电电流进行高温降额：温度 ≥ limitHighCharge 时充电电流 = 0，在 [cutoff, limit) 区间线性降额。取低温降额结果和高温降额结果中的较小值。

### 3.4 综合电流限制

#### REQ-007 — 电压/温度约束合并

**编号** | **优先级** | **函数**
REQ-007 | 高 | `SOF_MinimumOfTwoSofValues()`

**描述**：系统应将电压约束电流和温度约束电流取最小值（逐个电流方向/类型），得到最保守（安全）的电流限制值。使用 `MATH_MinimumOfTwoFloats()` 对连续充电、峰值充电、连续放电、峰值放电四个维度分别取最小。

#### REQ-008 — 逐串计算与汇总

**编号** | **优先级** | **函数**
REQ-008 | 高 | `SOF_Calculation()`

**描述**：系统应遍历所有已闭合的电池串（`BMS_IsStringClosed(s) == true`），逐串计算电压和温度约束的电流限制，取每个串的最小值，然后乘以闭合串数得到电池包级电流限制。

**处理流程**：
1. 读取 min/max 数据库块
2. 重置电池包电流为 0
3. 遍历所有电池串，对已闭合的串计算电压约束和温度约束电流
4. 每串取 `SOF_MinimumOfTwoSofValues()` 作为该串允许电流
5. 跟踪所有串中连续充放电电流的最小值
6. 电池包级 = 闭合串数 × 单串最小电流
7. 限制不超过 `BS_MAXIMUM_STRING_CURRENT_mA`

#### REQ-009 — 错误状态电流清零

**编号** | **优先级** | **函数**
REQ-009 | 高 | `SOF_Calculation()`

**描述**：当 BMS 正在转入 ERROR 状态（`BMS_IsTransitionToErrorStateActive() == true`）时，系统应将所有电池包级允许电流清零（连续充电、连续放电、峰值充电、峰值放电均设为 0.0），确保安全。

#### REQ-010 — 未闭合串处理

**编号** | **优先级** | **函数**
REQ-010 | 中 | `SOF_Calculation()`

**描述**：对于未闭合的电池串，系统应将该串的所有允许电流值设为 0.0，该串不计入闭合串数。

### 3.5 SOF 初始化

#### REQ-011 — SOF 模块初始化

**编号** | **优先级** | **函数**
REQ-011 | 高 | `SOF_Init()`

**描述**：系统应提供 `SOF_Init()` 函数，调用 `SOF_CalculateCurves()` 预计算降额曲线参数。该函数在系统启动时调用一次。

## 4. 非功能需求

#### REQ-012 — 编译期除零避免

**编号** | **优先级**
REQ-012 | 中

**描述**：系统应在配置层面确保降额区间宽度（cutoff - limit）不为零，避免斜率为无穷大。通过正确的配置宏定义保证。

## 5. 接口需求

### 5.1 公共 API

| 函数 | 用途 | 调用方 |
|------|------|--------|
| `SOF_Init()` | SOF 初始化（预计算曲线） | 系统启动流程 |
| `SOF_Calculation()` | SOF 周期计算 | 算法调度框架 |

### 5.2 依赖项

| 依赖模块 | 用途 |
|----------|------|
| `database.h` | 读取 min/max 数据、写入 SOF 数据 |
| `bms.h` | 电池串闭合状态、BMS 错误状态 |
| `foxmath.h` | `MATH_MinimumOfTwoFloats()` |
| `sof_trapezoid_cfg.h` | 降额配置参数 |

## 6. 数据结构

### 6.1 SOF_CURRENT_LIMITS_s

| 字段 | 类型 | 说明 |
|------|------|------|
| `continuousChargeCurrent_mA` | `float_t` | 最大连续充电电流 (mA) |
| `peakChargeCurrent_mA` | `float_t` | 最大峰值充电电流 (mA) |
| `continuousDischargeCurrent_mA` | `float_t` | 最大连续放电电流 (mA) |
| `peakDischargeCurrent_mA` | `float_t` | 最大峰值放电电流 (mA) |

### 6.2 SOF_CURVE_s

包含 8 组 slope/offset 对，分别对应低温放电、高温放电、低温充电、高温充电、上限电压、下限电压的降额曲线参数。

## 7. 追溯矩阵

| 需求编号 | 需求描述 | 函数 | 文件 |
|----------|---------|------|------|
| REQ-001 | 降额曲线参数计算 | `SOF_CalculateCurves()` | [sof_trapezoid.c](sof_trapezoid.c) |
| REQ-002 | 电压约束电流限制 | `SOF_CalculateVoltageBasedCurrentLimit()` | [sof_trapezoid.c](sof_trapezoid.c) |
| REQ-003 | 低温放电降额 | `SOF_CalculateTemperatureBasedCurrentLimit()` | [sof_trapezoid.c](sof_trapezoid.c) |
| REQ-004 | 低温充电降额 | `SOF_CalculateTemperatureBasedCurrentLimit()` | [sof_trapezoid.c](sof_trapezoid.c) |
| REQ-005 | 高温放电降额 | `SOF_CalculateTemperatureBasedCurrentLimit()` | [sof_trapezoid.c](sof_trapezoid.c) |
| REQ-006 | 高温充电降额 | `SOF_CalculateTemperatureBasedCurrentLimit()` | [sof_trapezoid.c](sof_trapezoid.c) |
| REQ-007 | 电压/温度约束合并 | `SOF_MinimumOfTwoSofValues()` | [sof_trapezoid.c](sof_trapezoid.c) |
| REQ-008 | 逐串计算与汇总 | `SOF_Calculation()` | [sof_trapezoid.c](sof_trapezoid.c) |
| REQ-009 | 错误状态电流清零 | `SOF_Calculation()` | [sof_trapezoid.c](sof_trapezoid.c) |
| REQ-010 | 未闭合串处理 | `SOF_Calculation()` | [sof_trapezoid.c](sof_trapezoid.c) |
| REQ-011 | SOF 模块初始化 | `SOF_Init()` | [sof_trapezoid.c](sof_trapezoid.c) |
| REQ-012 | 编译期除零避免 | 配置宏设计 | [sof_trapezoid_cfg.h](sof_trapezoid_cfg.h) |
