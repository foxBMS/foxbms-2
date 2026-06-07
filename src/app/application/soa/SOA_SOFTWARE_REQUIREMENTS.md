# SOA（安全操作区域）模块 — 软件需求规格说明

**文件**: `soa.c`
**版本**: v1.11.0
**创建日期**: 2020-10-14
**最后更新**: 2026-04-20
**所属层级**: APPLICATION
**模块前缀**: SOA
**许可证**: BSD-3-Clause

---

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 电池管理系统中 **SOA（Safe Operating Area，安全操作区域）模块**的软件需求规格。该模块负责实时监测电池参数（单体电压、单体温度、电流）是否在安全操作区域范围内，并在参数超出安全阈值时向诊断系统（DIAG）上报相应事件。SOA 模块是电池管理系统安全防护体系的核心组成部分。

### 1.2 范围

| 项目 | 说明 |
|------|------|
| **涵盖** | 单体电压三级安全阈值检查（过压/欠压）、单体温度三级安全阈值检查（过温/欠温，区分充放电方向）、电池串级电流过流检查、电芯级电流过流检查、电池包级总电流过流检查、开路异常电流检测、深度放电检测、诊断事件上报与清除、从站温度检查预留接口 |
| **不涵盖** | 单体电压/温度/电流的原始数据采集（由 DRIVER 层负责）、安全阈值的具体数值定义（由 `battery_cell_cfg.h` / `battery_system_cfg.h` 负责）、诊断事件的具体处理逻辑（由 DIAG 模块负责）、电流方向的判断算法（由 BMS 模块 `BMS_GetCurrentFlowDirection()` 负责） |

### 1.3 定义与缩略语

| 缩略语 | 英文全称 | 中文说明 |
|--------|---------|---------|
| SOA | Safe Operating Area | 安全操作区域 |
| MOL | Maximum Operating Limit | 最大运行限制（最外层安全边界） |
| RSL | Recommended Safety Limit | 推荐安全限制（中间层安全边界） |
| MSL | Maximum Safety Limit | 最大安全限制（最内层安全边界，最严重） |
| DIAG | Diagnostics | 诊断模块 |
| BMS | Battery Management System | 电池管理系统 |
| mV | Millivolt | 毫伏 |
| ddegC | Deci Degree Celsius | 分摄氏度（0.1°C） |
| mA | Milliampere | 毫安 |
| FAS | FoxBMS Assertion System | FoxBMS 断言系统 |

### 1.4 参考文献

| 编号 | 文档 | 说明 |
|------|------|------|
| REF-01 | [soa.h](soa.h) | SOA 模块头文件，定义公开接口 |
| REF-02 | [soa_cfg.h](../config/soa_cfg.h) | SOA 配置头文件，声明辅助检查函数 |
| REF-03 | [soa_cfg.c](../config/soa_cfg.c) | SOA 辅助检查函数实现 |
| REF-04 | [diag.h](../../engine/diag/diag.h) | 诊断模块接口（DIAG_Handler） |
| REF-05 | [bms.h](../bms/bms.h) | BMS 模块接口（电流方向判断） |
| REF-06 | [database.h](../../engine/database/database.h) | 数据库结构定义（DATA_BLOCK_MIN_MAX_s, DATA_BLOCK_PACK_VALUES_s） |
| REF-07 | [battery_cell_cfg.h](../config/battery_cell_cfg.h) | 电芯安全阈值常量定义 |
| REF-08 | [battery_system_cfg.h](../config/battery_system_cfg.h) | 电池系统配置常量 |

---

## 2. 总体描述

### 2.1 产品视角

SOA 模块位于 APPLICATION 层，是 BMS 安全防护体系的前端检查模块。它在 BMS 主状态机获取原始测量数据（电压、温度、电流）之后被调用，对数据执行三级安全阈值检查，并向 DIAG 诊断系统上报检查结果。SOA 模块本身不修改测量数据，仅执行边界检查与诊断事件上报。

```
┌──────────────────────────────────────────────────┐
│                 APPLICATION 层                     │
│  ┌─────────┐  ┌──────────┐  ┌─────────┐         │
│  │   BMS   │  │   BAL    │  │   ...   │         │
│  └────┬────┘  └──────────┘  └─────────┘         │
│       │                                            │
│  ┌────▼──────────────────────────────────┐       │
│  │            SOA (安全操作区域)           │       │
│  │  ┌──────────────────────────────────┐ │       │
│  │  │ 电压检查 / 温度检查 / 电流检查     │ │       │
│  │  │ 三级安全阈值 (MOL→RSL→MSL)        │ │       │
│  │  │ 充放电方向感知 / 深度放电检测      │ │       │
│  │  └──────────────┬───────────────────┘ │       │
│  └─────────────────┼─────────────────────┘       │
│                    │                               │
│  ┌─────────────────▼─────────────────────┐       │
│  │     DIAG / BMS / Database / Config    │       │
│  └───────────────────────────────────────┘       │
└──────────────────────────────────────────────────┘
```

### 2.2 工作模式

SOA 模块以**无状态函数库**形式工作。每个检查函数（`SOA_CheckVoltages()`、`SOA_CheckTemperatures()`、`SOA_CheckCurrent()`）均为纯函数，不维护内部状态变量。函数由 BMS 主状态机在每个测量周期调用，对传入的数据指针执行检查后直接通过 `DIAG_Handler()` 上报结果。

| 函数 | 检查对象 | 调用阶段 |
|------|---------|---------|
| `SOA_CheckVoltages()` | 单体电压 | 电压测量完成后 |
| `SOA_CheckTemperatures()` | 单体温度 | 温度测量完成后 |
| `SOA_CheckCurrent()` | 电池串电流 / 电池包电流 | 电流测量完成后 |
| `SOA_CheckSlaveTemperatures()` | 从站温度 | 预留（待实现） |

### 2.3 用户特征

本模块的使用者是 BMS 上层控制逻辑（`bms.c`）。由 BMS 状态机在数据采集完成后按固定周期调用。不直接面对外部用户或工具链。

### 2.4 关键设计决策

| 决策 | 说明 |
|------|------|
| 三级安全阈值体系 | 采用 MOL（最大运行限制）→ RSL（推荐安全限制）→ MSL（最大安全限制）三级分层结构，MOL 为最外层边界（优先级最低），MSL 为最内层边界（优先级最高）。阈值之间的嵌套关系使得各层违规可以逐级上报 |
| 故障分层上报与恢复 | 利用三级阈值的嵌套特性，违规检测从上到下逐级深入（MOL → RSL → MSL），故障恢复从下到上逐级清除（MSL → RSL → MOL）。该设计避免了在每个层级单独判断条件 |
| 充放电方向感知 | 温度和电流的 SOA 检查根据电池当前工作状态（充电/放电/浮空）选择不同的安全阈值，因为充电和放电对电池的安全性要求不同 |
| 无效数据跳过 | 电流 SOA 检查对无效测量值（`invalidStringCurrent` / `invalidPackCurrent`）跳过检查，避免基于不可靠数据进行安全判断 |
| 无状态设计 | SOA 模块不维护任何内部状态，每次检查完全基于输入参数，避免状态同步问题 |

---

## 3. 功能需求

### 3.1 电压安全操作区域检查

#### REQ-001 — 电压 SOA 全串遍历检查

| 属性 | 内容 |
|------|------|
| **编号** | REQ-001 |
| **优先级** | 高 |
| **关联函数** | `SOA_CheckVoltages()` |

**描述**：系统应对所有电池串（`s = 0..BS_NR_OF_STRINGS-1`）逐一执行单体电压安全操作区域检查。对每串获取最大电芯电压（`maximumCellVoltage_mV[s]`）用于过压检查，获取最小电芯电压（`minimumCellVoltage_mV[s]`）用于欠压检查。

**前置条件**：
- `pMinimumMaximumCellVoltages != NULL_PTR`（通过 `FAS_ASSERT` 强制校验）

**处理逻辑**：
1. 断言检查输入指针非空
2. 初始化欠压 MSL 返回值变量 `retvalUndervoltageMSL`
3. 遍历每个电池串 `s`
4. 获取该串的最大电压和最小电压
5. 执行过压检查（REQ-002）
6. 执行欠压检查（REQ-003）

---

#### REQ-002 — 过压三级安全阈值监控

| 属性 | 内容 |
|------|------|
| **编号** | REQ-002 |
| **优先级** | 高 |
| **关联函数** | `SOA_CheckVoltages()` |
| **关联诊断ID** | `DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_MOL`, `DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_RSL`, `DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_MSL` |

**描述**：系统应对每个电池串的最大电芯电压执行三级过压安全阈值检查。三级阈值满足 `BC_VOLTAGE_MAX_MOL_mV < BC_VOLTAGE_MAX_RSL_mV < BC_VOLTAGE_MAX_MSL_mV` 的嵌套关系。

**违规检测逻辑**（逐级深入）：
1. 若 `voltageMax_mV >= BC_VOLTAGE_MAX_MOL_mV`（MOL 违规）：
   - 上报 `DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_MOL` = `DIAG_EVENT_NOT_OK`
   - 若同时 `voltageMax_mV >= BC_VOLTAGE_MAX_RSL_mV`（RSL 违规）：
     - 上报 `DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_RSL` = `DIAG_EVENT_NOT_OK`
     - 若同时 `voltageMax_mV >= BC_VOLTAGE_MAX_MSL_mV`（MSL 违规）：
       - 上报 `DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_MSL` = `DIAG_EVENT_NOT_OK`

**恢复检测逻辑**（逐级退出）：
1. 若 `voltageMax_mV < BC_VOLTAGE_MAX_MSL_mV`（MSL 恢复）：
   - 上报 `DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_MSL` = `DIAG_EVENT_OK`
   - 若同时 `voltageMax_mV < BC_VOLTAGE_MAX_RSL_mV`（RSL 恢复）：
     - 上报 `DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_RSL` = `DIAG_EVENT_OK`
     - 若同时 `voltageMax_mV < BC_VOLTAGE_MAX_MOL_mV`（MOL 恢复）：
       - 上报 `DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_MOL` = `DIAG_EVENT_OK`

**诊断上报参数**：所有事件均以 `DIAG_STRING` 级别上报，携带当前电池串编号 `s`。

---

#### REQ-003 — 欠压三级安全阈值监控

| 属性 | 内容 |
|------|------|
| **编号** | REQ-003 |
| **优先级** | 高 |
| **关联函数** | `SOA_CheckVoltages()` |
| **关联诊断ID** | `DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MOL`, `DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_RSL`, `DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MSL` |

**描述**：系统应对每个电池串的最小电芯电压执行三级欠压安全阈值检查。三级阈值满足 `BC_VOLTAGE_MIN_MOL_mV > BC_VOLTAGE_MIN_RSL_mV > BC_VOLTAGE_MIN_MSL_mV` 的嵌套关系。

**违规检测逻辑**（逐级深入）：
1. 若 `voltageMin_mV <= BC_VOLTAGE_MIN_MOL_mV`（MOL 违规）：
   - 上报 `DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MOL` = `DIAG_EVENT_NOT_OK`
   - 若同时 `voltageMin_mV <= BC_VOLTAGE_MIN_RSL_mV`（RSL 违规）：
     - 上报 `DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_RSL` = `DIAG_EVENT_NOT_OK`
     - 若同时 `voltageMin_mV <= BC_VOLTAGE_MIN_MSL_mV`（MSL 违规）：
       - 上报 `DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MSL` = `DIAG_EVENT_NOT_OK`
       - 保存 MSL 返回值为后续深度放电判断使用

**恢复检测逻辑**（逐级退出）：
1. 若 `voltageMin_mV > BC_VOLTAGE_MIN_MSL_mV`（MSL 恢复）：
   - 上报 `DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MSL` = `DIAG_EVENT_OK`
   - 若同时 `voltageMin_mV > BC_VOLTAGE_MIN_RSL_mV`（RSL 恢复）：
     - 上报 `DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_RSL` = `DIAG_EVENT_OK`
     - 若同时 `voltageMin_mV > BC_VOLTAGE_MIN_MOL_mV`（MOL 恢复）：
       - 上报 `DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MOL` = `DIAG_EVENT_OK`

**诊断上报参数**：所有事件均以 `DIAG_STRING` 级别上报，携带当前电池串编号 `s`。

---

#### REQ-004 — 深度放电检测

| 属性 | 内容 |
|------|------|
| **编号** | REQ-004 |
| **优先级** | 高 |
| **关联函数** | `SOA_CheckVoltages()` |
| **关联诊断ID** | `DIAG_ID_DEEP_DISCHARGE_DETECTED` |

**描述**：系统应在欠压 MSL 故障被确认后，进一步判断是否发生了深度放电。深度放电的触发条件为：欠压 MSL 诊断事件返回错误 **且** 最小电压低于深度放电阈值。

**处理逻辑**：
1. 若 `retvalUndervoltageMSL == DIAG_HANDLER_RETURN_ERR_OCCURRED`（欠压 MSL 故障确认）
2. 且 `voltageMin_mV <= BC_VOLTAGE_DEEP_DISCHARGE_mV`（电压低于深度放电阈值）
3. 则上报 `DIAG_ID_DEEP_DISCHARGE_DETECTED` = `DIAG_EVENT_NOT_OK`

**设计说明**：深度放电意味着电池电压降至危险水平以下，可能造成电池永久性损坏。该检测仅在欠压 MSL 被确认时触发，因为深度放电阈值比 MSL 更低，只有最严重的欠压情况才需要深度放电检测。

---

### 3.2 温度安全操作区域检查

#### REQ-005 — 温度 SOA 全串遍历检查

| 属性 | 内容 |
|------|------|
| **编号** | REQ-005 |
| **优先级** | 高 |
| **关联函数** | `SOA_CheckTemperatures()` |

**描述**：系统应对所有电池串逐一执行单体温度安全操作区域检查。对每串获取最大电芯温度（`maximumTemperature_ddegC[s]`）用于过温检查，获取最小电芯温度（`minimumTemperature_ddegC[s]`）用于欠温检查。同时获取每串的电流值用于判断当前充放电方向。

**前置条件**：
- `pMinimumMaximumCellTemperatures != NULL_PTR`（通过 `FAS_ASSERT` 强制校验）
- `pCurrent != NULL_PTR`（通过 `FAS_ASSERT` 强制校验）

**处理逻辑**：
1. 断言检查两个输入指针均非空
2. 遍历每个电池串 `s`
3. 获取该串的最大温度、最小温度和电流值
4. 调用 `BMS_GetCurrentFlowDirection()` 判断电流方向
5. 根据方向选择对应的温度阈值组
6. 执行过温检查（REQ-007）
7. 执行欠温检查（REQ-008）

---

#### REQ-006 — 充放电方向感知温度阈值选择

| 属性 | 内容 |
|------|------|
| **编号** | REQ-006 |
| **优先级** | 高 |
| **关联函数** | `SOA_CheckTemperatures()` |
| **关联接口** | `BMS_GetCurrentFlowDirection()` |

**描述**：系统应根据电池串的当前电流方向，自动选择充电或放电对应的温度安全阈值组。这是因为电池在充电和放电时对温度的承受能力不同，需要使用不同的安全边界。

**阈值选择规则**：

| 电流方向 | 返回值 | 使用阈值组 |
|---------|--------|----------|
| 放电 | `BMS_DISCHARGING` | `BC_TEMPERATURE_*_DISCHARGE_*_ddegC` 系列 |
| 充电（及其他） | 非 `BMS_DISCHARGING` | `BC_TEMPERATURE_*_CHARGE_*_ddegC` 系列 |

**设计说明**：
- 代码使用 `if (BMS_GetCurrentFlowDirection(i_current) == BMS_DISCHARGING)` 判断为放电，`else` 分支涵盖充电和浮空状态
- 放电时电流从电池流出，电池发热由内阻引起；充电时电流流入电池，充电效率和安全温度边界不同于放电

---

#### REQ-007 — 过温三级安全阈值监控

| 属性 | 内容 |
|------|------|
| **编号** | REQ-007 |
| **优先级** | 高 |
| **关联函数** | `SOA_CheckTemperatures()` |
| **关联诊断ID** | `DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_MOL/RSL/MSL`, `DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_MOL/RSL/MSL` |

**描述**：系统应对每个电池串的最大电芯温度执行三级过温安全阈值检查。根据电流方向（REQ-006）使用对应的放电或充电阈值组。三级阈值满足嵌套关系：MOL < RSL < MSL（放电和充电各有独立的三级阈值）。

**违规检测逻辑**（以放电为例）：
1. 若 `temperatureMax_ddegC >= BC_TEMPERATURE_MAX_DISCHARGE_MOL_ddegC`（放电 MOL 违规）：
   - 上报对应放电 MOL 诊断事件 = `DIAG_EVENT_NOT_OK`
   - 若同时 >= 放电 RSL：上报放电 RSL 诊断事件 = `DIAG_EVENT_NOT_OK`
     - 若同时 >= 放电 MSL：上报放电 MSL 诊断事件 = `DIAG_EVENT_NOT_OK`

**恢复检测逻辑**（以放电为例）：
1. 若 `temperatureMax_ddegC < BC_TEMPERATURE_MAX_DISCHARGE_MSL_ddegC`（放电 MSL 恢复）：
   - 上报放电 MSL 诊断事件 = `DIAG_EVENT_OK`
   - 若同时 < 放电 RSL：上报放电 RSL 诊断事件 = `DIAG_EVENT_OK`
     - 若同时 < 放电 MOL：上报放电 MOL 诊断事件 = `DIAG_EVENT_OK`

**充电方向**使用 `BC_TEMPERATURE_MAX_CHARGE_*_ddegC` 系列阈值，对应的诊断 ID 为 `DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_*`，逻辑相同。

**诊断上报参数**：所有事件均以 `DIAG_STRING` 级别上报，携带当前电池串编号 `s`。

---

#### REQ-008 — 欠温三级安全阈值监控

| 属性 | 内容 |
|------|------|
| **编号** | REQ-008 |
| **优先级** | 高 |
| **关联函数** | `SOA_CheckTemperatures()` |
| **关联诊断ID** | `DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_MOL/RSL/MSL`, `DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_MOL/RSL/MSL` |

**描述**：系统应对每个电池串的最小电芯温度执行三级欠温安全阈值检查。根据电流方向（REQ-006）使用对应的放电或充电阈值组。三级阈值满足嵌套关系：MOL > RSL > MSL（放电和充电各有独立的三级阈值）。

**违规检测逻辑**（以放电为例）：
1. 若 `temperatureMin_ddegC <= BC_TEMPERATURE_MIN_DISCHARGE_MOL_ddegC`（放电 MOL 违规）：
   - 上报对应放电 MOL 诊断事件 = `DIAG_EVENT_NOT_OK`
   - 若同时 <= 放电 RSL：上报放电 RSL 诊断事件 = `DIAG_EVENT_NOT_OK`
     - 若同时 <= 放电 MSL：上报放电 MSL 诊断事件 = `DIAG_EVENT_NOT_OK`

**恢复检测逻辑**（以放电为例）：
1. 若 `temperatureMin_ddegC > BC_TEMPERATURE_MIN_DISCHARGE_MSL_ddegC`（放电 MSL 恢复）：
   - 上报放电 MSL 诊断事件 = `DIAG_EVENT_OK`
   - 若同时 > 放电 RSL：上报放电 RSL 诊断事件 = `DIAG_EVENT_OK`
     - 若同时 > 放电 MOL：上报放电 MOL 诊断事件 = `DIAG_EVENT_OK`

**充电方向**使用 `BC_TEMPERATURE_MIN_CHARGE_*_ddegC` 系列阈值，对应的诊断 ID 为 `DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_*`，逻辑相同。

**诊断上报参数**：所有事件均以 `DIAG_STRING` 级别上报，携带当前电池串编号 `s`。

---

### 3.3 电流安全操作区域检查

#### REQ-009 — 电流 SOA 检查入口

| 属性 | 内容 |
|------|------|
| **编号** | REQ-009 |
| **优先级** | 高 |
| **关联函数** | `SOA_CheckCurrent()` |

**描述**：系统应对所有电池串及电池包总电流执行多级电流安全操作区域检查。检查范围包括：电池串过流、电芯过流、电池包总过流以及开路异常电流检测。

**前置条件**：
- `pTablePackValues != NULL_PTR`（通过 `FAS_ASSERT` 强制校验）

**处理逻辑**：
1. 断言检查输入指针非空
2. **电池串级检查**：遍历每个电池串，仅对有效电流值执行：
   - 判断电流方向
   - 检查电池串过流（REQ-011）
   - 检查电芯过流（REQ-012）
   - 检查开路异常电流（REQ-014）
   - 处理电流浮空状态
3. **电池包级检查**：若电池包总电流有效，检查电池包过流（REQ-013）

---

#### REQ-010 — 电流数据有效性判断

| 属性 | 内容 |
|------|------|
| **编号** | REQ-010 |
| **优先级** | 高 |
| **关联函数** | `SOA_CheckCurrent()` |

**描述**：系统应在执行电流 SOA 检查前，判断电流测量数据的有效性。仅对有效的电流数据执行过流检查，无效数据应被跳过。

**判断规则**：

| 检查级别 | 有效性标志 | 判断逻辑 |
|---------|-----------|---------|
| 电池串级 | `invalidStringCurrent[s] == 0u` | 仅当标志为 0 时执行该串的过流检查 |
| 电池包级 | `invalidPackCurrent == 0u` | 仅当标志为 0 时执行电池包过流检查 |

**设计说明**：跳过无效电流值避免了基于故障传感器数据进行安全判断的误报风险。

---

#### REQ-011 — 电池串过流检测

| 属性 | 内容 |
|------|------|
| **编号** | REQ-011 |
| **优先级** | 高 |
| **关联函数** | `SOA_CheckCurrent()` |
| **关联辅助函数** | `SOA_IsStringCurrentLimitViolated()`（定义于 `soa_cfg.c`） |
| **关联诊断ID** | `DIAG_ID_STRING_OVERCURRENT_CHARGE_MSL`, `DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MSL` |

**描述**：系统应调用 `SOA_IsStringCurrentLimitViolated()` 判断电池串电流是否超过电池串最大电流限制（`BS_MAXIMUM_STRING_CURRENT_mA`），并根据电流方向上报对应的诊断事件。

**处理逻辑**（对每个有效电流的电池串）：
1. 调用 `SOA_IsStringCurrentLimitViolated(absStringCurrent_mA, currentDirection)` 获取过流标志
2. 若 `currentDirection == BMS_CHARGING`（充电）：
   - 过流 → 上报 `DIAG_ID_STRING_OVERCURRENT_CHARGE_MSL` = `DIAG_EVENT_NOT_OK`
   - 未过流 → 上报 `DIAG_ID_STRING_OVERCURRENT_CHARGE_MSL` = `DIAG_EVENT_OK`
3. 若 `currentDirection == BMS_DISCHARGING`（放电）：
   - 过流 → 上报 `DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MSL` = `DIAG_EVENT_NOT_OK`
   - 未过流 → 上报 `DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MSL` = `DIAG_EVENT_OK`

**辅助函数逻辑**（`SOA_IsStringCurrentLimitViolated()`）：
- 仅在充放电状态下检查，浮空状态下不判断为过流
- 使用电流绝对值与 `BS_MAXIMUM_STRING_CURRENT_mA` 比较

---

#### REQ-012 — 电芯过流检测

| 属性 | 内容 |
|------|------|
| **编号** | REQ-012 |
| **优先级** | 高 |
| **关联函数** | `SOA_CheckCurrent()` |
| **关联辅助函数** | `SOA_IsCellCurrentLimitViolated()`（定义于 `soa_cfg.c`） |
| **关联诊断ID** | `DIAG_ID_OVERCURRENT_CHARGE_CELL_MSL`, `DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MSL` |

**描述**：系统应调用 `SOA_IsCellCurrentLimitViolated()` 判断电流是否超过单体电芯的电流限制，并根据电流方向上报对应的诊断事件。

**处理逻辑**（对每个有效电流的电池串）：
1. 调用 `SOA_IsCellCurrentLimitViolated(absStringCurrent_mA, currentDirection)` 获取过流标志
2. 若 `currentDirection == BMS_CHARGING`（充电）：
   - 过流 → 上报 `DIAG_ID_OVERCURRENT_CHARGE_CELL_MSL` = `DIAG_EVENT_NOT_OK`
   - 未过流 → 上报 `DIAG_ID_OVERCURRENT_CHARGE_CELL_MSL` = `DIAG_EVENT_OK`
3. 若 `currentDirection == BMS_DISCHARGING`（放电）：
   - 过流 → 上报 `DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MSL` = `DIAG_EVENT_NOT_OK`
   - 未过流 → 上报 `DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MSL` = `DIAG_EVENT_OK`

**辅助函数逻辑**（`SOA_IsCellCurrentLimitViolated()`）：
- 充电时：比较电流与 `BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK * BC_CURRENT_MAX_CHARGE_MSL_mA`
- 放电时：比较电流与 `BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK * BC_CURRENT_MAX_DISCHARGE_MSL_mA`
- 浮空/静置时：不判断为过流

---

#### REQ-013 — 电池包过流检测

| 属性 | 内容 |
|------|------|
| **编号** | REQ-013 |
| **优先级** | 高 |
| **关联函数** | `SOA_CheckCurrent()` |
| **关联辅助函数** | `SOA_IsPackCurrentLimitViolated()`（定义于 `soa_cfg.c`） |
| **关联诊断ID** | `DIAG_ID_PACK_OVERCURRENT_CHARGE_MSL`, `DIAG_ID_PACK_OVERCURRENT_DISCHARGE_MSL` |

**描述**：系统应在电池包总电流数据有效时，调用 `SOA_IsPackCurrentLimitViolated()` 判断总电流是否超过电池包最大电流限制（`BS_MAXIMUM_PACK_CURRENT_mA`），并根据电流方向上报对应的诊断事件。

**处理逻辑**：
1. 检查 `invalidPackCurrent == 0u`（有效）
2. 获取电池包总电流的绝对值和方向
3. 调用 `SOA_IsPackCurrentLimitViolated(absPackCurrent_mA, currentDirection)`
4. 若 `currentDirection == BMS_CHARGING`（充电）：
   - 过流 → 上报 `DIAG_ID_PACK_OVERCURRENT_CHARGE_MSL` = `DIAG_EVENT_NOT_OK`
   - 未过流 → 上报 `DIAG_ID_PACK_OVERCURRENT_CHARGE_MSL` = `DIAG_EVENT_OK`
5. 若 `currentDirection == BMS_DISCHARGING`（放电）：
   - 过流 → 上报 `DIAG_ID_PACK_OVERCURRENT_DISCHARGE_MSL` = `DIAG_EVENT_NOT_OK`
   - 未过流 → 上报 `DIAG_ID_PACK_OVERCURRENT_DISCHARGE_MSL` = `DIAG_EVENT_OK`
6. 若电流浮空（非充非放）：上报所有电池包过流诊断事件 = `DIAG_EVENT_OK`

**诊断上报参数**：电池包级事件以 `DIAG_SYSTEM` 级别上报，携带参数 `0u`。

---

#### REQ-014 — 开路异常电流检测

| 属性 | 内容 |
|------|------|
| **编号** | REQ-014 |
| **优先级** | 高 |
| **关联函数** | `SOA_CheckCurrent()` |
| **关联辅助函数** | `SOA_IsCurrentOnOpenString()`（定义于 `soa_cfg.c`） |
| **关联诊断ID** | `DIAG_ID_CURRENT_ON_OPEN_STRING` |

**描述**：系统应检测接触器断开（开路）状态下电池串是否存在异常电流。当检测到有电流流过但接触器未闭合且不在预充电状态时，判定为开路异常。

**辅助函数逻辑**（`SOA_IsCurrentOnOpenString()`）：
1. 断言检查串编号合法性（`stringNumber < BS_NR_OF_STRINGS`）
2. 仅当存在充放电电流方向时（`BMS_CHARGING` 或 `BMS_DISCHARGING`）继续判断
3. 调用 `BMS_IsStringClosed()` 和 `BMS_IsStringPrecharging()` 查询接触器状态
4. 若两者均返回 `false`（接触器完全断开），返回 `true`（异常）
5. 否则返回 `false`（正常）

**处理逻辑**（对每个有效电流的电池串）：
1. 调用 `SOA_IsCurrentOnOpenString(currentDirection, s)`
2. 若返回 `false`（正常）→ 上报 `DIAG_ID_CURRENT_ON_OPEN_STRING` = `DIAG_EVENT_OK`
3. 若返回 `true`（异常）→ 上报 `DIAG_ID_CURRENT_ON_OPEN_STRING` = `DIAG_EVENT_NOT_OK`

---

#### REQ-015 — 电流浮空状态处理

| 属性 | 内容 |
|------|------|
| **编号** | REQ-015 |
| **优先级** | 中 |
| **关联函数** | `SOA_CheckCurrent()` |

**描述**：系统应在检测到电流方向为浮空（既非充电也非放电）时，将所有电流相关诊断事件设置为 `DIAG_EVENT_OK`（无故障），包括电池串过流、电芯过流、电池包过流的所有充放电方向诊断 ID。

**处理逻辑**：
- 当 `currentDirection` 既不是 `BMS_CHARGING` 也不是 `BMS_DISCHARGING` 时：
  1. 上报 `DIAG_ID_STRING_OVERCURRENT_CHARGE_MSL` = `DIAG_EVENT_OK`
  2. 上报 `DIAG_ID_OVERCURRENT_CHARGE_CELL_MSL` = `DIAG_EVENT_OK`
  3. 上报 `DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MSL` = `DIAG_EVENT_OK`
  4. 上报 `DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MSL` = `DIAG_EVENT_OK`

**设计说明**：电流浮空表示电池处于静置或弛豫状态，此时不存在过流风险，因此所有过流诊断事件应标记为正常。

---

### 3.4 输入保护与预留接口

#### REQ-016 — 输入参数空指针保护

| 属性 | 内容 |
|------|------|
| **编号** | REQ-016 |
| **优先级** | 高 |
| **关联函数** | `SOA_CheckVoltages()`, `SOA_CheckTemperatures()`, `SOA_CheckCurrent()` |

**描述**：所有 SOA 公开检查函数必须在函数入口处对输入的指针参数执行非空断言检查（`FAS_ASSERT`），确保不会因空指针解引用导致系统崩溃。

**断言检查清单**：

| 函数 | 断言内容 |
|------|---------|
| `SOA_CheckVoltages()` | `FAS_ASSERT(pMinimumMaximumCellVoltages != NULL_PTR)` |
| `SOA_CheckTemperatures()` | `FAS_ASSERT(pMinimumMaximumCellTemperatures != NULL_PTR)` 和 `FAS_ASSERT(pCurrent != NULL_PTR)` |
| `SOA_CheckCurrent()` | `FAS_ASSERT(pTablePackValues != NULL_PTR)` |

---

#### REQ-017 — 从站温度检查预留接口

| 属性 | 内容 |
|------|------|
| **编号** | REQ-017 |
| **优先级** | 低 |
| **关联函数** | `SOA_CheckSlaveTemperatures()` |

**描述**：系统应预留从站温度检查函数接口 `SOA_CheckSlaveTemperatures()`，供未来从站温度 SOA 检查功能扩展。当前函数体为空（TODO），不执行任何操作。

---

## 4. 非功能需求

### 4.1 时序与性能

| 编号 | 需求描述 | 关联 |
|------|---------|------|
| NFR-001 | 所有 SOA 检查函数应在单次 BMS 任务周期内完成执行，不包含阻塞等待或循环延迟 | 全部 extern 函数 |
| NFR-002 | 电压和温度检查对每个电池串的遍历时间复杂度为 O(N × L)，其中 N 为电池串数，L 为阈值层级数（固定为 3），无嵌套多层循环 | `SOA_CheckVoltages()`, `SOA_CheckTemperatures()` |
| NFR-003 | 电流检查的最佳路径（串电流无效时跳过检查）避免了不必要的辅助函数调用开销 | `SOA_CheckCurrent()` |

### 4.2 内存

| 编号 | 需求描述 | 关联 |
|------|---------|------|
| NFR-004 | SOA 模块不维护任何内部状态变量或静态数据副本，所有数据通过参数传递，内存占用极小 | 全部函数 |
| NFR-005 | 所有局部变量使用栈分配，函数返回后自动释放 | 全部函数 |

### 4.3 鲁棒性

| 编号 | 需求描述 | 关联 |
|------|---------|------|
| NFR-006 | 每个函数可独立调用，不依赖特定的调用顺序或先前的系统状态 | 全部 extern 函数 |
| NFR-007 | 故障检测遵循"违规逐级深入，恢复逐级退出"策略，即使传感器数据波动也不会导致诊断事件的频繁跳变（由 DIAG 模块内部去抖动机制保证） | `SOA_CheckVoltages()`, `SOA_CheckTemperatures()` |
| NFR-008 | 无效电流数据被安全跳过而非进行可能错误的过流判断，避免因传感器故障导致误报过流 | `SOA_CheckCurrent()` |

### 4.4 可配置性

| 编号 | 需求描述 | 关联 |
|------|---------|------|
| NFR-009 | 所有安全阈值应通过 `battery_cell_cfg.h` 和 `battery_system_cfg.h` 中的编译期宏定义配置，不硬编码在 SOA 代码中 | `BC_VOLTAGE_*`, `BC_TEMPERATURE_*`, `BC_CURRENT_*`, `BS_MAXIMUM_*_CURRENT_mA` |
| NFR-010 | 电池串数量（`BS_NR_OF_STRINGS`）通过配置宏定义，SOA 函数自动适配不同的电池系统拓扑 | `for (uint8_t s = 0u; s < BS_NR_OF_STRINGS; s++)` |

### 4.5 可测试性

| 编号 | 需求描述 | 关联 |
|------|---------|------|
| NFR-011 | SOA 检查函数不依赖硬件外设，所有输入通过参数传入，可直接使用模拟数据进行单元测试 | 全部 extern 函数 |
| NFR-012 | 头文件通过 `#ifdef UNITY_UNIT_TEST` 条件编译块预留了单元测试接口扩展区 | `soa.h`, `soa.c` |

---

## 5. 接口需求

### 5.1 公共 API

| 函数 | 签名 | 说明 |
|------|------|------|
| `SOA_CheckVoltages` | `void SOA_CheckVoltages(DATA_BLOCK_MIN_MAX_s *pMinimumMaximumCellVoltages)` | 检查单体电压是否在安全操作区域内 |
| `SOA_CheckTemperatures` | `void SOA_CheckTemperatures(DATA_BLOCK_MIN_MAX_s *pMinimumMaximumCellTemperatures, DATA_BLOCK_PACK_VALUES_s *pCurrent)` | 检查单体温度是否在安全操作区域内（根据电流方向选择阈值） |
| `SOA_CheckCurrent` | `void SOA_CheckCurrent(DATA_BLOCK_PACK_VALUES_s *pTablePackValues)` | 检查电流是否在安全操作区域内 |
| `SOA_CheckSlaveTemperatures` | `void SOA_CheckSlaveTemperatures(void)` | 从站温度检查（预留，未实现） |

### 5.2 依赖项

| 模块 | 头文件 | 使用的接口 |
|------|--------|----------|
| 电池电芯配置 | `battery_cell_cfg.h` | `BC_VOLTAGE_MAX_MOL_mV`, `BC_VOLTAGE_MAX_RSL_mV`, `BC_VOLTAGE_MAX_MSL_mV`, `BC_VOLTAGE_MIN_MOL_mV`, `BC_VOLTAGE_MIN_RSL_mV`, `BC_VOLTAGE_MIN_MSL_mV`, `BC_VOLTAGE_DEEP_DISCHARGE_mV`, `BC_TEMPERATURE_MAX_DISCHARGE_MOL_ddegC` 等所有温度/电流阈值宏 |
| 电池系统配置 | `battery_system_cfg.h` | `BS_NR_OF_STRINGS` |
| BMS | `bms.h` | `BMS_GetCurrentFlowDirection()` |
| DIAG | `diag.h` | `DIAG_Handler()`, `DIAG_EVENT_OK`, `DIAG_EVENT_NOT_OK`, `DIAG_STRING`, `DIAG_SYSTEM`，各类 `DIAG_ID_*` 宏 |
| 数学工具 | `foxmath.h` | `abs()` |
| SOA 配置 | `soa_cfg.h` | `SOA_IsStringCurrentLimitViolated()`, `SOA_IsCellCurrentLimitViolated()`, `SOA_IsPackCurrentLimitViolated()`, `SOA_IsCurrentOnOpenString()` |
| 标准库 | `<stdbool.h>`, `<stdint.h>` | `bool`, `uint8_t`, `int16_t`, `int32_t`, `uint32_t` |

### 5.3 调用方

| 调用模块 | 调用接口 | 场景 |
|----------|----------|------|
| `bms.c` | `SOA_CheckVoltages()` | 单体电压测量完成后 |
| `bms.c` | `SOA_CheckTemperatures()` | 单体温度测量完成后 |
| `bms.c` | `SOA_CheckCurrent()` | 电流测量完成后 |

### 5.4 诊断事件上报汇总

#### 电压相关诊断事件

| 诊断 ID | 触发事件 | 级别 |
|--------|---------|------|
| `DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_MOL` | 过压 MOL 违规/恢复 | `DIAG_STRING` |
| `DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_RSL` | 过压 RSL 违规/恢复 | `DIAG_STRING` |
| `DIAG_ID_CELL_VOLTAGE_OVERVOLTAGE_MSL` | 过压 MSL 违规/恢复 | `DIAG_STRING` |
| `DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MOL` | 欠压 MOL 违规/恢复 | `DIAG_STRING` |
| `DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_RSL` | 欠压 RSL 违规/恢复 | `DIAG_STRING` |
| `DIAG_ID_CELL_VOLTAGE_UNDERVOLTAGE_MSL` | 欠压 MSL 违规/恢复 | `DIAG_STRING` |
| `DIAG_ID_DEEP_DISCHARGE_DETECTED` | 深度放电检测 | `DIAG_STRING` |

#### 温度相关诊断事件

| 诊断 ID | 触发事件 | 级别 |
|--------|---------|------|
| `DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_MOL` | 放电过温 MOL 违规/恢复 | `DIAG_STRING` |
| `DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_RSL` | 放电过温 RSL 违规/恢复 | `DIAG_STRING` |
| `DIAG_ID_TEMP_OVERTEMPERATURE_DISCHARGE_MSL` | 放电过温 MSL 违规/恢复 | `DIAG_STRING` |
| `DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_MOL` | 充电过温 MOL 违规/恢复 | `DIAG_STRING` |
| `DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_RSL` | 充电过温 RSL 违规/恢复 | `DIAG_STRING` |
| `DIAG_ID_TEMP_OVERTEMPERATURE_CHARGE_MSL` | 充电过温 MSL 违规/恢复 | `DIAG_STRING` |
| `DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_MOL` | 放欠温 MOL 违规/恢复 | `DIAG_STRING` |
| `DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_RSL` | 放电欠温 RSL 违规/恢复 | `DIAG_STRING` |
| `DIAG_ID_TEMP_UNDERTEMPERATURE_DISCHARGE_MSL` | 放电欠温 MSL 违规/恢复 | `DIAG_STRING` |
| `DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_MOL` | 充电欠温 MOL 违规/恢复 | `DIAG_STRING` |
| `DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_RSL` | 充电欠温 RSL 违规/恢复 | `DIAG_STRING` |
| `DIAG_ID_TEMP_UNDERTEMPERATURE_CHARGE_MSL` | 充电欠温 MSL 违规/恢复 | `DIAG_STRING` |

#### 电流相关诊断事件

| 诊断 ID | 触发事件 | 级别 |
|--------|---------|------|
| `DIAG_ID_STRING_OVERCURRENT_CHARGE_MSL` | 电池串充电过流 | `DIAG_STRING` |
| `DIAG_ID_STRING_OVERCURRENT_DISCHARGE_MSL` | 电池串放电过流 | `DIAG_STRING` |
| `DIAG_ID_OVERCURRENT_CHARGE_CELL_MSL` | 电芯充电过流 | `DIAG_STRING` |
| `DIAG_ID_OVERCURRENT_DISCHARGE_CELL_MSL` | 电芯放电过流 | `DIAG_STRING` |
| `DIAG_ID_PACK_OVERCURRENT_CHARGE_MSL` | 电池包充电过流 | `DIAG_SYSTEM` |
| `DIAG_ID_PACK_OVERCURRENT_DISCHARGE_MSL` | 电池包放电过流 | `DIAG_SYSTEM` |
| `DIAG_ID_CURRENT_ON_OPEN_STRING` | 开路异常电流 | `DIAG_STRING` |

---

## 6. 控制流

### 6.1 电压 SOA 检查流程

```
SOA_CheckVoltages(pMinimumMaximumCellVoltages)
  ├─ FAS_ASSERT(ptr != NULL)
  └─ for each string s:
       ├─ 获取 maximumCellVoltage_mV[s], minimumCellVoltage_mV[s]
       ├─ 过压检查 (REQ-002):
       │   ├─ if max >= MOL → NOT_OK, 继续检查
       │   │   └─ if max >= RSL → NOT_OK, 继续检查
       │   │       └─ if max >= MSL → NOT_OK
       │   └─ if max < MSL → OK, 继续检查
       │       └─ if max < RSL → OK, 继续检查
       │           └─ if max < MOL → OK
       ├─ 欠压检查 (REQ-003):
       │   ├─ if min <= MOL → NOT_OK, 继续检查
       │   │   └─ if min <= RSL → NOT_OK, 继续检查
       │   │       └─ if min <= MSL → NOT_OK
       │   │           └─ 深度放电检测 (REQ-004):
       │   │               若 MSL 确认且 min <= DEEP_DISCHARGE → NOT_OK
       │   └─ if min > MSL → OK, 继续检查
       │       └─ if min > RSL → OK, 继续检查
       │           └─ if min > MOL → OK
       └─ 进入下一串
```

### 6.2 温度 SOA 检查流程

```
SOA_CheckTemperatures(pMinMaxCellTemps, pCurrent)
  ├─ FAS_ASSERT(两个指针均非空)
  └─ for each string s:
       ├─ 获取 temperatureMax, temperatureMin, current
       ├─ 判断电流方向 → DISCHARGING / else (CHARGING)
       ├─ 过温检查 (REQ-007):
       │   ├─ if max >= MOL → NOT_OK, 继续
       │   │   └─ if max >= RSL → NOT_OK, 继续
       │   │       └─ if max >= MSL → NOT_OK
       │   └─ if max < MSL → OK, 继续
       │       └─ if max < RSL → OK, 继续
       │           └─ if max < MOL → OK
       ├─ 欠温检查 (REQ-008):
       │   ├─ if min <= MOL → NOT_OK, 继续
       │   │   └─ if min <= RSL → NOT_OK, 继续
       │   │       └─ if min <= MSL → NOT_OK
       │   └─ if min > MSL → OK, 继续
       │       └─ if min > RSL → OK, 继续
       │           └─ if min > MOL → OK
       └─ 进入下一串
```

### 6.3 电流 SOA 检查流程

```
SOA_CheckCurrent(pTablePackValues)
  ├─ FAS_ASSERT(ptr != NULL)
  ├─ 电池串级检查 ─ for each string s:
  │   ├─ if invalidStringCurrent[s] != 0 → 跳过 (REQ-010)
  │   ├─ 获取 currentDirection, absStringCurrent
  │   ├─ stringOvercurrent = SOA_IsStringCurrentLimitViolated(...)
  │   ├─ cellOvercurrent = SOA_IsCellCurrentLimitViolated(...)
  │   ├─ if CHARGING:
  │   │   ├─ 上报 串过流充电 诊断 (REQ-011)
  │   │   └─ 上报 电芯过流充电 诊断 (REQ-012)
  │   ├─ else if DISCHARGING:
  │   │   ├─ 上报 串过流放电 诊断 (REQ-011)
  │   │   └─ 上报 电芯过流放电 诊断 (REQ-012)
  │   └─ else (浮空) (REQ-015):
  │       └─ 上报所有过流诊断 = OK
  │   └─ 开路异常检测 (REQ-014):
  │       若 SOA_IsCurrentOnOpenString() → NOT_OK / OK
  └─ 电池包级检查:
      ├─ if invalidPackCurrent != 0 → 跳过
      ├─ 获取 packCurrentDirection, absPackCurrent
      ├─ packOvercurrent = SOA_IsPackCurrentLimitViolated(...)
      ├─ if CHARGING → 上报 / else if DISCHARGING → 上报 (REQ-013)
      └─ else (浮空) → 上报所有 = OK
```

---

## 7. 数据结构

SOA 模块自身不定义数据结构，全部依赖外部数据库提供的数据结构：

| 数据结构 | 定义来源 | SOA 使用方式 |
|---------|---------|-------------|
| `DATA_BLOCK_MIN_MAX_s` | `database.h` | 通过指针读取各串的 min/max 电压和温度值 |
| `DATA_BLOCK_PACK_VALUES_s` | `database.h` | 通过指针读取各串电流、电池包电流及其有效性标志 |
| `BMS_CURRENT_FLOW_STATE_e` | `bms.h` | 电流方向枚举（`BMS_CHARGING`, `BMS_DISCHARGING` 等） |
| `DIAG_RETURNTYPE_e` | `diag.h` | DIAG_Handler 返回值类型 |

---

## 8. 配置阈值参考

以下宏定义在 `battery_cell_cfg.h` 和 `battery_system_cfg.h` 中，是 SOA 模块的核心安全边界参数：

### 8.1 电压阈值（battery_cell_cfg.h）

| 宏定义 | 说明 |
|--------|------|
| `BC_VOLTAGE_MAX_MOL_mV` | 过压最大运行限制（最外层） |
| `BC_VOLTAGE_MAX_RSL_mV` | 过压推荐安全限制（中间层） |
| `BC_VOLTAGE_MAX_MSL_mV` | 过压最大安全限制（最内层，最严重） |
| `BC_VOLTAGE_MIN_MOL_mV` | 欠压最小运行限制（最外层） |
| `BC_VOLTAGE_MIN_RSL_mV` | 欠压推荐安全限制（中间层） |
| `BC_VOLTAGE_MIN_MSL_mV` | 欠压最小安全限制（最内层，最严重） |
| `BC_VOLTAGE_DEEP_DISCHARGE_mV` | 深度放电电压阈值 |

### 8.2 温度阈值（battery_cell_cfg.h）

| 宏定义 | 说明 |
|--------|------|
| `BC_TEMPERATURE_MAX_DISCHARGE_MOL_ddegC` | 放电过温 MOL |
| `BC_TEMPERATURE_MAX_DISCHARGE_RSL_ddegC` | 放电过温 RSL |
| `BC_TEMPERATURE_MAX_DISCHARGE_MSL_ddegC` | 放电过温 MSL |
| `BC_TEMPERATURE_MAX_CHARGE_MOL_ddegC` | 充电过温 MOL |
| `BC_TEMPERATURE_MAX_CHARGE_RSL_ddegC` | 充电过温 RSL |
| `BC_TEMPERATURE_MAX_CHARGE_MSL_ddegC` | 充电过温 MSL |
| `BC_TEMPERATURE_MIN_DISCHARGE_MOL_ddegC` | 放电欠温 MOL |
| `BC_TEMPERATURE_MIN_DISCHARGE_RSL_ddegC` | 放电欠温 RSL |
| `BC_TEMPERATURE_MIN_DISCHARGE_MSL_ddegC` | 放电欠温 MSL |
| `BC_TEMPERATURE_MIN_CHARGE_MOL_ddegC` | 充电欠温 MOL |
| `BC_TEMPERATURE_MIN_CHARGE_RSL_ddegC` | 充电欠温 RSL |
| `BC_TEMPERATURE_MIN_CHARGE_MSL_ddegC` | 充电欠温 MSL |

### 8.3 电流阈值（battery_system_cfg.h / battery_cell_cfg.h）

| 宏定义 | 说明 |
|--------|------|
| `BS_MAXIMUM_STRING_CURRENT_mA` | 电池串最大电流限制 |
| `BS_MAXIMUM_PACK_CURRENT_mA` | 电池包最大电流限制 |
| `BC_CURRENT_MAX_CHARGE_MSL_mA` | 单体电芯充电最大安全电流 |
| `BC_CURRENT_MAX_DISCHARGE_MSL_mA` | 单体电芯放电最大安全电流 |
| `BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK` | 每个电芯块的并联电芯数 |

---

## 9. 追溯矩阵

### 9.1 需求 → 代码位置映射

#### 功能需求

| 需求编号 | 需求简述 | 实现文件 | 函数 / 代码段 |
|---------|---------|----------|-------------|
| REQ-001 | 电压 SOA 全串遍历检查 | `soa.c` | `SOA_CheckVoltages()` L81-146 |
| REQ-002 | 过压三级安全阈值监控 | `soa.c` | `SOA_CheckVoltages()` L89-112 |
| REQ-003 | 欠压三级安全阈值监控 | `soa.c` | `SOA_CheckVoltages()` L114-144 |
| REQ-004 | 深度放电检测 | `soa.c` | `SOA_CheckVoltages()` L126-129 |
| REQ-005 | 温度 SOA 全串遍历检查 | `soa.c` | `SOA_CheckTemperatures()` L148-269 |
| REQ-006 | 充放电方向感知温度阈值选择 | `soa.c` | `SOA_CheckTemperatures()` L160, L215 |
| REQ-007 | 过温三级安全阈值监控 | `soa.c` | `SOA_CheckTemperatures()` L160-212 |
| REQ-008 | 欠温三级安全阈值监控 | `soa.c` | `SOA_CheckTemperatures()` L215-267 |
| REQ-009 | 电流 SOA 检查入口 | `soa.c` | `SOA_CheckCurrent()` L271-351 |
| REQ-010 | 电流数据有效性判断 | `soa.c` | `SOA_CheckCurrent()` L277, L328 |
| REQ-011 | 电池串过流检测 | `soa.c` | `SOA_CheckCurrent()` L282-310 |
| REQ-012 | 电芯过流检测 | `soa.c` | `SOA_CheckCurrent()` L283-310 |
| REQ-013 | 电池包过流检测 | `soa.c` | `SOA_CheckCurrent()` L328-349 |
| REQ-014 | 开路异常电流检测 | `soa.c` | `SOA_CheckCurrent()` L319-323 |
| REQ-015 | 电流浮空状态处理 | `soa.c` | `SOA_CheckCurrent()` L310-316, L345-349 |
| REQ-016 | 输入参数空指针保护 | `soa.c` | 各函数入口 `FAS_ASSERT()` |
| REQ-017 | 从站温度检查预留接口 | `soa.c` | `SOA_CheckSlaveTemperatures()` L353-354 |

#### 非功能需求

| 需求编号 | 需求简述 | 关联 |
|---------|---------|------|
| NFR-001 | 单周期内完成 | 全部 extern 函数 |
| NFR-002 | O(N×3) 遍历复杂度 | `SOA_CheckVoltages()`, `SOA_CheckTemperatures()` |
| NFR-003 | 无效数据跳过优化 | `SOA_CheckCurrent()` |
| NFR-004 | 无状态设计，零静态内存 | 全部函数 |
| NFR-005 | 栈分配局部变量 | 全部函数 |
| NFR-006 | 函数独立可调用 | 全部 extern 函数 |
| NFR-007 | 故障恢复逐级退出 | `SOA_CheckVoltages()`, `SOA_CheckTemperatures()` |
| NFR-008 | 无效电流安全跳过 | `SOA_CheckCurrent()` |
| NFR-009 | 阈值宏定义配置 | `battery_cell_cfg.h`, `battery_system_cfg.h` |
| NFR-010 | BS_NR_OF_STRINGS 自动适配 | 所有 for 循环 |
| NFR-011 | 无硬件依赖可测试 | 全部 extern 函数 |
| NFR-012 | 单元测试接口预留 | `#ifdef UNITY_UNIT_TEST` 区块 |

### 9.2 三级安全阈值体系总览

```
过压 / 过温 (上限):
  ┌──────────────────────────────────────┐
  │         MSL (最大安全限制)             │  ← 最内层，最严重
  │    ┌─────────────────────────────┐   │
  │    │    RSL (推荐安全限制)         │   │  ← 中间层
  │    │   ┌────────────────────┐    │   │
  │    │   │  MOL (最大运行限制) │    │   │  ← 最外层，预警
  │    │   │    正常范围         │    │   │
  │    │   └────────────────────┘    │   │
  │    └─────────────────────────────┘   │
  └──────────────────────────────────────┘

欠压 / 欠温 (下限):
  ┌──────────────────────────────────────┐
  │    正常范围                           │
  │    ┌────────────────────┐           │
  │    │  MOL (最小运行限制) │           │  ← 最外层，预警
  │    └────────────────────┘           │
  │         ┌─────────────────────┐      │
  │         │  RSL (推荐安全限制)  │      │  ← 中间层
  │         └─────────────────────┘      │
  │              ┌────────────────────┐  │
  │              │ MSL (最小安全限制) │  │  ← 最内层，最严重
  │              └────────────────────┘  │
  │                   ┌──────────────┐   │
  │                   │  深度放电     │   │  ← 仅欠压适用
  │                   └──────────────┘   │
  └──────────────────────────────────────┘
```

---

**文档生成日期**: 2026-06-06
**生成工具**: Claude Code (write-software-requirements skill)
**适用范围**: foxBMS 2 v1.11.0
