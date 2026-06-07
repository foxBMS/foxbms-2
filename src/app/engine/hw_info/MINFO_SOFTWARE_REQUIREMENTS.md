# MINFO — 硬件信息模块软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义 foxBMS 2 硬件信息模块（MINFO）的软件需求。MINFO 模块负责收集和管理 foxBMS-Master 板级硬件信息，包括复位源、调试探针连接状态、供电电压监测等。

### 1.2 参考文献

- [master_info.h](master_info.h) — 硬件信息模块头文件
- [master_info.c](master_info.c) — 硬件信息模块实现

---

## 2. 功能需求

### 2.1 复位源管理

#### REQ-APP_ENGINE_HW_INFO_001 — 复位源设置与查询

**编号** | REQ-APP_ENGINE_HW_INFO_001
**优先级** | 中
**函数** | `MINFO_SetResetSource()`, `MINFO_GetResetSource()`

**描述**：系统应提供设置和查询最近一次系统复位源（`resetSource_t`）的接口。复位源信息在启动阶段由启动代码写入，供上层模块查询。

---

### 2.2 调试探针检测

#### REQ-APP_ENGINE_HW_INFO_002 — 调试探针连接状态

**编号** | REQ-APP_ENGINE_HW_INFO_002
**优先级** | 中
**函数** | `MINFO_SetDebugProbeConnectionState()`, `MINFO_GetDebugProbeConnectionState()`

**描述**：系统应提供设置和查询调试探针（Debug Probe）连接状态的接口。状态枚举值包括 `MINFO_DEBUG_PROBE_NOT_CONNECTED` 和 `MINFO_DEBUG_PROBE_CONNECTED`。

---

### 2.3 供电电压监测

#### REQ-APP_ENGINE_HW_INFO_003 — 30C 供电电压监测

**编号** | REQ-APP_ENGINE_HW_INFO_003
**优先级** | 高
**函数** | `MINFO_CheckSupplyVoltageClamp30c()`

**描述**：系统应监测 30C 供电电压（Clamp 30C 电源轨）：
1. 从数据库读取 ADC 电压测量值
2. 根据电阻分压器参数（R1=10kΩ, R2=866Ω）将 ADC 电压换算为实际供电电压：`U_supply = ((R1+R2)/R2) × ADC_voltage`
3. 若供电电压 ≥ 欠压阈值（5000mV）→ 报告 `DIAG_EVENT_OK`
4. 若供电电压 < 欠压阈值 → 报告 `DIAG_EVENT_NOT_OK`（诊断 ID: `DIAG_ID_SUPPLY_VOLTAGE_CLAMP_30C_LOST`）
5. 将计算得到的供电电压值存入状态结构体

---

## 3. 数据结构

### 3.1 MINFO_MASTER_STATE_s

| 成员 | 类型 | 说明 |
|------|------|------|
| `resetSource` | `resetSource_t` | 上次复位源 |
| `debugProbe` | `MINFO_DEBUG_PROBE_CONNECTION_STATE_e` | 调试探针连接状态 |
| `supplyVoltageClamp30c_mV` | `uint32_t` | 实测 30C 供电电压（mV） |

---

## 4. 追溯矩阵

| 需求编号 | 源文件 | 函数 |
|----------|--------|------|
| REQ-APP_ENGINE_HW_INFO_001 | master_info.c | `MINFO_SetResetSource()`, `MINFO_GetResetSource()` |
| REQ-APP_ENGINE_HW_INFO_002 | master_info.c | `MINFO_SetDebugProbeConnectionState()`, `MINFO_GetDebugProbeConnectionState()` |
| REQ-APP_ENGINE_HW_INFO_003 | master_info.c | `MINFO_CheckSupplyVoltageClamp30c()` |
