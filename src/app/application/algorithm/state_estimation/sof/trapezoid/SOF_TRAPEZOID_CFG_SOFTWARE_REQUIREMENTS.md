# SOF_TRAPEZOID_CFG — 软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 SOF 梯形降额法配置模块的软件需求规格。该模块定义了 SOF 计算所需的所有降额参数、阈值和配置结构体。

### 1.2 参考文献

- [sof_trapezoid_cfg.c](sof_trapezoid_cfg.c) — SOF 配置实例
- [sof_trapezoid_cfg.h](sof_trapezoid_cfg.h) — SOF 配置宏与结构体定义

## 2. 功能需求

### 2.1 电流限制配置

#### REQ-001 — 最大充放电电流配置

**编号** | **优先级** | **宏定义**
REQ-001 | 高 | `SOF_STRING_CURRENT_CONTINUOUS_CHARGE_mA`, `SOF_STRING_CURRENT_CONTINUOUS_DISCHARGE_mA`

**描述**：系统应根据电池单体数据手册配置每串的最大连续充电电流和放电电流：`BC_CURRENT_MAX_CHARGE_MOL_mA × 并联数`、`BC_CURRENT_MAX_DISCHARGE_MOL_mA × 并联数`。

#### REQ-002 — Limp Home 电流配置

**编号** | **优先级** | **宏定义**
REQ-002 | 中 | `SOF_STRING_CURRENT_LIMP_HOME_mA`

**描述**：系统应定义跛行回家模式下的最低放电电流，默认 20000 mA，由系统工程师根据整车需求设定。

### 2.2 温度降额配置

#### REQ-003 — 低温降额阈值

**编号** | **优先级** | **宏定义**
REQ-003 | 高 | `SOF_TEMPERATURE_LOW_CUTOFF_DISCHARGE_ddegC`, `SOF_TEMPERATURE_LOW_LIMIT_DISCHARGE_ddegC`, `SOF_TEMPERATURE_LOW_CUTOFF_CHARGE_ddegC`, `SOF_TEMPERATURE_LOW_LIMIT_CHARGE_ddegC`

**描述**：系统应为充电和放电方向分别定义低温降额的开始阈值（CUTOFF，MOL 级别，降额起始点）和完全降额阈值（LIMIT，MSL 级别，电流限制为零或 limp home）。区分充放电是因为电池在低温下充电和放电的特性不同。

#### REQ-004 — 高温降额阈值

**编号** | **优先级** | **宏定义**
REQ-004 | 高 | `SOF_TEMPERATURE_HIGH_CUTOFF_DISCHARGE_ddegC`, `SOF_TEMPERATURE_HIGH_LIMIT_DISCHARGE_ddegC`, `SOF_TEMPERATURE_HIGH_CUTOFF_CHARGE_ddegC`, `SOF_TEMPERATURE_HIGH_LIMIT_CHARGE_ddegC`

**描述**：系统应为充电和放电方向分别定义高温降额的开始阈值（CUTOFF）和完全降额阈值（LIMIT）。

### 2.3 电压降额配置

#### REQ-005 — 电压降额阈值

**编号** | **优先级** | **宏定义**
REQ-005 | 高 | `SOF_VOLTAGE_CUTOFF_CHARGE_mV`, `SOF_VOLTAGE_LIMIT_CHARGE_mV`, `SOF_VOLTAGE_CUTOFF_DISCHARGE_mV`, `SOF_VOLTAGE_LIMIT_DISCHARGE_mV`

**描述**：系统应定义电压降额阈值：
- 充电方向：上限电压 CUTOFF（MOL，降额起始）和 LIMIT（RSL，充电电流为零）
- 放电方向：下限电压 CUTOFF（MOL，降额起始）和 LIMIT（RSL，放电电流为零）

### 2.4 配置结构体

#### REQ-006 — SOF_CONFIG_s 配置结构体

**编号** | **优先级** | **结构体**
REQ-006 | 高 | `SOF_CONFIG_s`

**描述**：系统应定义包含所有降额参数的配置结构体，包括：最大充放电电流、limp home 电流、4 个低温阈值、4 个高温阈值、4 个电压阈值，共 15 个字段。

#### REQ-007 — 推荐电流配置实例

**编号** | **优先级** | **变量**
REQ-007 | 高 | `sof_recommendedCurrent`

**描述**：系统应定义 `sof_recommendedCurrent` 配置常量实例，使用上述宏定义填充 `SOF_CONFIG_s` 结构体的所有字段。该实例供 `SOF_Init()` 和 `SOF_Calculation()` 使用。

## 3. 追溯矩阵

| 需求编号 | 需求描述 | 结构体/宏/变量 | 文件 |
|----------|---------|--------------|------|
| REQ-001 | 最大充放电电流配置 | `SOF_STRING_CURRENT_CONTINUOUS_*` | [sof_trapezoid_cfg.h](sof_trapezoid_cfg.h) |
| REQ-002 | Limp Home 电流配置 | `SOF_STRING_CURRENT_LIMP_HOME_mA` | [sof_trapezoid_cfg.h](sof_trapezoid_cfg.h) |
| REQ-003 | 低温降额阈值 | `SOF_TEMPERATURE_LOW_*` | [sof_trapezoid_cfg.h](sof_trapezoid_cfg.h) |
| REQ-004 | 高温降额阈值 | `SOF_TEMPERATURE_HIGH_*` | [sof_trapezoid_cfg.h](sof_trapezoid_cfg.h) |
| REQ-005 | 电压降额阈值 | `SOF_VOLTAGE_*` | [sof_trapezoid_cfg.h](sof_trapezoid_cfg.h) |
| REQ-006 | SOF_CONFIG_s 结构体 | `SOF_CONFIG_s` | [sof_trapezoid_cfg.h](sof_trapezoid_cfg.h) |
| REQ-007 | 推荐电流配置实例 | `sof_recommendedCurrent` | [sof_trapezoid_cfg.c](sof_trapezoid_cfg.c) |
