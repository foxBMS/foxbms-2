# Redundancy（测量冗余校验）模块 — 软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 电池管理系统中 **Redundancy（测量冗余校验）模块**的软件需求规格。该模块负责对电池管理系统中的冗余测量值（单体电压、单体温度、电流、电压、功率等）进行综合校验、时间戳有效性检查、超时检测以及最终测量值的确定，是保障测量数据可靠性的核心模块。

### 1.2 范围

| 项目 | 说明 |
|------|------|
| **涵盖** | 单体电压冗余校验与验证、单体温度冗余校验与验证、电流测量验证、电池包总电压验证、高压总线电压验证、功率测量验证、测量时间戳有效性检查、测量超时检测、最小值/最大值/平均值计算、诊断事件上报、数据库读写操作 |
| **不涵盖** | AFE 硬件原始数据采集（由 DRIVER 层负责）、单体电压/温度的具体合理性比较算法（由 Plausibility 模块负责）、BMS 状态机控制逻辑（由 BMS 模块负责）、电芯平衡控制策略 |

### 1.3 定义与缩略语

| 缩略语 | 英文全称 | 中文说明 |
|--------|---------|---------|
| MRC | Measurement Redundancy Check | 测量冗余校验模块前缀 |
| AFE | Analog Front-End | 模拟前端采集芯片 |
| SRS | Software Requirements Specification | 软件需求规格说明 |
| BMS | Battery Management System | 电池管理系统 |
| DIAG | Diagnostics | 诊断模块 |
| PL | Plausibility | 合理性校验模块 |
| ddegC | Deci Degree Celsius | 分摄氏度（0.1°C） |
| HV | High Voltage | 高压 |
| mA | Milliampere | 毫安 |
| mV | Millivolt | 毫伏 |
| W | Watt | 瓦特 |
| ms | Millisecond | 毫秒 |

### 1.4 参考文献

| 编号 | 文档 | 说明 |
|------|------|------|
| REF-01 | [bms.h](../bms/bms.h) | BMS 状态接口（电芯串连接状态） |
| REF-02 | [database.h](../../engine/database/database.h) | 数据库读写接口 |
| REF-03 | [diag_cfg.h](../../engine/config/diag_cfg.h) | 诊断事件 ID 定义 |
| REF-04 | [plausibility.h](../plausibility/plausibility.h) | 合理性校验接口 |
| REF-05 | [database_cfg.h](../../engine/config/database_cfg.h) | 数据库结构定义 |
| REF-06 | [battery_system_cfg.h](../config/battery_system_cfg.h) | 电池系统配置常量 |

---

## 2. 总体描述

### 2.1 产品视角

Redundancy 模块（前缀 MRC）位于 APPLICATION 层，是 BMS 应用层的一个核心子模块。它位于测量数据采集（AFE/电流传感器）与上层应用（BMS 状态机、均衡控制等）之间，负责对原始测量数据进行可靠性验证后输出可信的测量结果。

```
┌──────────────────────────────────────────────────┐
│                 APPLICATION 层                     │
│  ┌─────────┐  ┌──────────┐  ┌─────────┐         │
│  │   BMS   │  │   BAL    │  │   ...   │         │
│  └────┬────┘  └──────────┘  └─────────┘         │
│       │                                            │
│  ┌────▼──────────────────────────────────┐       │
│  │       Redundancy (MRC)                 │       │
│  │  ┌──────────────────────────────────┐ │       │
│  │  │ AFE测量验证 / 电池包测量验证        │ │       │
│  │  │ 时间戳校验 / 超时检测 / 统计计算    │ │       │
│  │  └──────────────┬───────────────────┘ │       │
│  └─────────────────┼─────────────────────┘       │
│                    │                               │
│  ┌─────────────────▼─────────────────────┐       │
│  │  Database / DIAG / Plausibility / OS  │       │
│  └───────────────────────────────────────┘       │
└──────────────────────────────────────────────────┘
```

### 2.2 工作模式

该模块以**函数库**形式工作，由上层 BMS 控制模块按固定周期调用。模块内部维护静态状态变量（`mrc_state`）和本地数据库表副本（`mrc_table*`），不暴露内部状态。

模块提供三个公开接口，按调用顺序可分为两个阶段：

| 调用阶段 | 调用函数 | 说明 |
|---------|---------|------|
| 初始化阶段 | `MRC_Initialize()` | 初始化所有内部状态和数据库表项，将测量值标记为无效 |
| AFE 测量验证 | `MRC_ValidateAfeMeasurement()` | 校验冗余单体电压和温度测量值，计算最小/最大/平均值 |
| 电池包测量验证 | `MRC_ValidatePackMeasurement()` | 校验电流、电芯串电压、电池电压、高压总线电压、功率 |

### 2.3 用户特征

本模块的使用者是 BMS 上层控制逻辑，通过 `BMS_StateMachine()` 中的测量验证流程按序调用。不直接面对外部用户或工具链。

### 2.4 关键设计决策

| 决策 | 说明 |
|------|------|
| 静态本地数据库副本 | 模块内部维护数据库表的静态本地副本（`static` 变量），避免在堆栈上分配大型数据结构，同时保证数据持久性 |
| 冗余降级策略 | 当冗余测量不可用时（超时或从未更新），自动降级为单源验证模式，使用可用测量值并上报相应诊断事件 |
| 时间戳单调性判断 | 利用无符号整数回绕（wrap-around）特性判断时间戳更新，无需特殊处理定时器溢出 |
| 数据库 Header 保护 | 在整体拷贝数据库条目时，先保存目标条目的 header 再恢复，保证数据库元数据不被意外覆盖 |

---

## 3. 功能需求

### 3.1 模块初始化与状态管理

#### REQ-001 — 冗余模块初始化

| 属性 | 内容 |
|------|------|
| **编号** | REQ-001 |
| **优先级** | 高 |
| **关联函数** | `MRC_Initialize()` |
| **关联数据结构** | `mrc_tableCellVoltages`, `mrc_tableCellTemperatures`, `mrc_tablePackValues` |

**描述**：系统应在冗余模块启动时，将所有内部数据库表副本中的测量值初始化为无效状态。具体包括：遍历所有电芯串的所有模组的所有电芯块，将单体电压标记为无效；遍历所有电芯串的所有模组的所有温度传感器，将单体温度标记为无效；将所有电芯串级和电池包级的电压/电流/功率有效标志设置为无效。初始化完成后将数据写入数据库。

**处理逻辑**：
1. 遍历所有电芯串（`s = 0..BS_NR_OF_STRINGS-1`）
2. 对每个电芯串，设置 `invalidStringVoltage[s] = true`
3. 遍历所有模组（`m = 0..BS_NR_OF_MODULES_PER_STRING-1`）
4. 遍历所有电芯块（`cb = 0..BS_NR_OF_CELL_BLOCKS_PER_MODULE-1`），设置 `invalidCellVoltage[s][m][cb] = true`
5. 设置 `invalidModuleVoltage[s][m] = true`
6. 遍历所有温度传感器（`ts = 0..BS_NR_OF_TEMP_SENSORS_PER_MODULE-1`），设置 `invalidCellTemperature[s][m][ts] = true`
7. 设置所有电芯串级别的无效标志：`invalidStringVoltage`, `invalidStringCurrent`, `invalidStringPower`
8. 设置所有电池包级别的无效标志：`invalidPackCurrent`, `invalidBatteryVoltage`, `invalidHvBusVoltage`, `invalidPackPower`
9. 调用 `DATA_WRITE_DATA()` 将初始化后的数据写入数据库

**返回状态**：
| 返回值 | 含义 |
|--------|------|
| `STD_OK` | 数据库写入成功 |
| `STD_NOT_OK` | 数据库写入失败 |

---

#### REQ-002 — 冗余模块内部状态维护

| 属性 | 内容 |
|------|------|
| **编号** | REQ-002 |
| **优先级** | 中 |
| **关联数据结构** | `MRC_STATE_s` |
| **关联变量** | `mrc_state` |

**描述**：系统应在模块内部维护一个静态状态结构体 `mrc_state`，用于记录各组测量值的上一次时间戳。该结构体包含：上一次基础单体电压测量时间戳、上一次冗余单体电压测量时间戳、上一次基础单体温度测量时间戳、上一次冗余单体温度测量时间戳、各电芯串上一次电流测量时间戳数组、各电芯串上一次功率测量时间戳数组。这些时间戳用于判断测量值自上次 MRC 调用以来是否已更新。

**状态结构体字段**：

| 字段 | 类型 | 说明 |
|------|------|------|
| `lastBaseCellVoltageTimestamp` | `uint32_t` | 上一次基础单体电压测量时间戳 |
| `lastRedundancy0CellVoltageTimestamp` | `uint32_t` | 上一次冗余0单体电压测量时间戳 |
| `lastBaseCellTemperatureTimestamp` | `uint32_t` | 上一次基础单体温度测量时间戳 |
| `lastRedundancy0CellTemperatureTimestamp` | `uint32_t` | 上一次冗余0单体温度测量时间戳 |
| `lastStringCurrentTimestamp[]` | `uint32_t[]` | 各电芯串上一次电流测量时间戳 |
| `lastStringPowerTimestamp[]` | `uint32_t[]` | 各电芯串上一次功率测量时间戳 |

---

### 3.2 时间戳校验

#### REQ-003 — 测量数据至少更新一次检查

| 属性 | 内容 |
|------|------|
| **编号** | REQ-003 |
| **优先级** | 高 |
| **关联函数** | `MRC_MeasurementUpdatedAtLeastOnce()` |

**描述**：系统应提供时间戳检查函数，判断测量数据是否至少被更新过一次。判断规则：仅当时间戳和上次时间戳**同时为零**时，判定测量从未被更新；其他任何情况下，判定测量至少曾被更新过一次。

**处理逻辑**：
1. 若 `(timestamp == 0) && (previousTimestamp == 0)`，返回 `false`（从未更新）
2. 否则返回 `true`（至少更新过一次）

**设计原理**：时间戳和上次时间戳同时为零仅可能发生在系统启动后、测量尚未首次完成的时刻。一旦测量发生，至少其中一个值会变为非零。

---

#### REQ-004 — 测量数据近期更新检查（含超时判断）

| 属性 | 内容 |
|------|------|
| **编号** | REQ-004 |
| **优先级** | 高 |
| **关联函数** | `MRC_MeasurementUpdatedRecently()` |
| **关联配置** | `MRC_CURRENT_MEASUREMENT_PERIOD_TIMEOUT_ms`, `MRC_AFE_MEASUREMENT_PERIOD_TIMEOUT_ms`, `MRC_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms` |

**描述**：系统应提供带超时参数的测量更新检查函数。该函数获取当前系统时钟滴答数，计算当前时间与测量时间戳的差值。若差值不超过指定的时间间隔**且**测量至少曾被更新过一次，则判定测量在近期内已更新，返回 `STD_OK`；否则返回 `STD_NOT_OK`。

**处理逻辑**：
1. 获取当前系统时钟滴答数：`currentTimestamp = OS_GetTickCount()`
2. 计算时间差：`currentTimestamp - timestamp`（无符号整数运算，自动处理定时器溢出回绕）
3. 若 `(currentTimestamp - timestamp) <= timeInterval` 且 `MRC_MeasurementUpdatedAtLeastOnce() == true`，返回 `STD_OK`
4. 否则返回 `STD_NOT_OK`

**设计原理**：无符号整数减法的回绕特性使得即使 `currentTimestamp` 因定时器溢出而小于 `timestamp`，差值计算仍然正确，无需使用 `abs()`。

---

### 3.3 AFE 测量验证（综合入口）

#### REQ-005 — AFE 测量综合验证

| 属性 | 内容 |
|------|------|
| **编号** | REQ-005 |
| **优先级** | 高 |
| **关联函数** | `MRC_ValidateAfeMeasurement()` |

**描述**：系统应在每次 AFE 测量周期中，从数据库读取基础（Base）和冗余0（Redundancy0）的单体电压和温度测量数据，依次执行单体电压验证和单体温度验证，并根据验证结果将更新后的数据写回数据库。该函数是 AFE 测量冗余校验的顶层入口。

**处理逻辑**：
1. 声明静态本地数据库表副本（`mrc_tableCellVoltageBase`, `mrc_tableCellVoltageRedundancy0`, `mrc_tableCellTemperatureBase`, `mrc_tableCellTemperatureRedundancy0`），使用 `static` 存储类别避免堆栈分配
2. 调用 `DATA_READ_DATA()` 从数据库读取基础及冗余的单体电压和温度测量数据
3. 调用 `MRC_ValidateCellVoltageMeasurement()` 执行单体电压验证
4. 调用 `MRC_ValidateCellTemperatureMeasurement()` 执行单体温度验证
5. 根据验证结果选择性写入数据库：
   - 若电压和温度均有更新：写入 `mrc_tableCellVoltages`, `mrc_tableCellTemperatures`, `mrc_tableMinimumMaximumValues`
   - 若仅有电压更新：写入 `mrc_tableCellVoltages`, `mrc_tableMinimumMaximumValues`
   - 若仅有温度更新：写入 `mrc_tableCellTemperatures`, `mrc_tableMinimumMaximumValues`
   - 若均无更新：返回 `STD_NOT_OK`

**返回状态**：
| 返回值 | 含义 |
|--------|------|
| `STD_OK` | 至少一项测量被成功验证并写入数据库 |
| `STD_NOT_OK` | 无任何测量被更新 |

---

### 3.4 单体电压测量验证

#### REQ-006 — 单体电压测量验证主流程

| 属性 | 内容 |
|------|------|
| **编号** | REQ-006 |
| **优先级** | 高 |
| **关联函数** | `MRC_ValidateCellVoltageMeasurement()` |
| **关联配置** | `MRC_AFE_MEASUREMENT_PERIOD_TIMEOUT_ms` |

**描述**：系统应对基础（Base）和冗余0（Redundancy0）两个通道的单体电压测量数据进行综合验证。验证流程包括：判断是否使用冗余测量、检查基础测量时间戳（超时检测）、检查冗余测量时间戳（超时检测）、根据两通道数据可用性选择验证策略、执行验证并计算结果。验证完成后计算最小/最大/平均单体电压并执行电压分布检查。

**处理流程**：

1. **判断是否使用冗余测量**：若冗余通道数据库条目至少曾被更新过一次，则启用冗余验证模式；否则仅使用基础测量。

2. **基础测量时间戳检查**：
   - 若基础测量在 `MRC_AFE_MEASUREMENT_PERIOD_TIMEOUT_ms` 内更新 → 清除超时标志，上报 `DIAG_EVENT_OK`
   - 否则 → 设置超时标志，上报 `DIAG_EVENT_NOT_OK`（诊断 ID：`DIAG_ID_BASE_CELL_VOLTAGE_MEASUREMENT_TIMEOUT`）

3. **冗余测量时间戳检查**：
   - 若冗余测量超时**且**冗余功能已启用 → 设置超时标志，上报诊断错误
   - 否则 → 清除超时标志，上报 `DIAG_EVENT_OK`（诊断 ID：`DIAG_ID_REDUNDANCY0_CELL_VOLTAGE_MEASUREMENT_TIMEOUT`）

4. **验证策略选择**：

   | 场景 | 冗余启用 | 基础更新 | 冗余更新 | 处理方式 |
   |------|---------|---------|---------|---------|
   | A | 是 | 是 | 是 | 两通道冗余比较验证（`MRC_ValidateCellVoltage()`） |
   | B | 是 | 是 | 否 | 若冗余超时，使用基础测量单源验证 |
   | C | 是 | 否 | 是 | 若基础超时，使用冗余测量单源验证 |
   | D | 是 | 否 | 否 | 不做任何处理 |
   | E | 否 | 是 | — | 使用基础测量单源验证 |
   | F | 否 | 否 | — | 不做任何处理 |

5. **统计计算**：若有任何测量值被验证更新，计算最小/最大/平均单体电压（`MRC_CalculateCellVoltageMinMaxAverage()`），随后执行电压分布检查（`PL_CheckVoltageSpread()`）。若分布检查发现异常，重新计算最小/最大/平均值以排除被标记为无效的电芯。

**前置条件**：
- `pCellVoltageBase != NULL_PTR`
- `pCellVoltageRedundancy0 != NULL_PTR`

**返回状态**：
| 返回值 | 含义 |
|--------|------|
| `true` | 已验证的电压数据库条目已更新 |
| `false` | 无需更新 |

---

#### REQ-007 — 单体电压冗余比较验证

| 属性 | 内容 |
|------|------|
| **编号** | REQ-007 |
| **优先级** | 高 |
| **关联函数** | `MRC_ValidateCellVoltage()` |

**描述**：系统应遍历所有电芯串的所有模组的所有电芯块，对基础测量和冗余0测量的单体电压进行逐对比较验证。验证规则如下：

- **两通道均有效**：调用 `PL_CheckCellVoltage()` 进行合理性校验，若合理则使用校验后的值并标记为有效；若不合理则标记为无效。
- **仅基础通道有效**：直接使用基础测量值，标记为有效。
- **仅冗余通道有效**：直接使用冗余测量值，标记为有效。
- **两通道均无效**：取两通道测量值的算术平均值作为填充值，标记为无效。

**处理逻辑**：
1. 断言三个输入指针均非空
2. 嵌套遍历所有电芯串 × 模组 × 电芯块
3. 对每个电芯块执行上述四路分支判断
4. 每个电芯串处理完毕后：
   - 记录该串的有效电压数量：`nrValidCellVoltages[s]`
   - 累加该串的总电压：`stringVoltage_mV[s]`
   - 重置有效计数器和合理性标志
   - 调用 `DIAG_CheckEvent()` 上报该串的单体电压合理性诊断事件（诊断 ID：`DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE`）

**前置条件**：
- `pCellVoltageBase != NULL_PTR`
- `pCellVoltageRedundancy0 != NULL_PTR`
- `pValidatedVoltages != NULL_PTR`

**后条件**：
- `pValidatedVoltages` 中包含每个电芯块的最终验证电压值及有效性标志
- `pValidatedVoltages->nrValidCellVoltages[s]` 记录了每个电芯串的有效电压数
- `pValidatedVoltages->stringVoltage_mV[s]` 记录了每个电芯串的总电压

**返回状态**：
| 返回值 | 含义 |
|--------|------|
| `STD_OK` | 所有电芯块的电压均被成功验证 |
| `STD_NOT_OK` | 至少有一个电芯块的电压无法被验证（两通道均无效或合理性校验失败） |

---

#### REQ-008 — 单体电压单源更新验证

| 属性 | 内容 |
|------|------|
| **编号** | REQ-008 |
| **优先级** | 中 |
| **关联函数** | `MRC_UpdateCellVoltageValidation()` |

**描述**：当只有一个测量源（基础通道或冗余通道）可用时，系统应将该测量源的整个数据库条目拷贝到已验证电压数据库条目中，同时保持目标条目的数据库 header 不变，以确保数据库元数据不被覆盖。

**处理逻辑**：
1. 断言两个输入指针均非空
2. 保存目标条目的 header：`tmpHeader = pValidatedVoltages->header`
3. 整体拷贝源条目到目标条目：`*pValidatedVoltages = *pCellVoltage`
4. 恢复目标条目的 header：`pValidatedVoltages->header = tmpHeader`

**前置条件**：
- `pCellVoltage != NULL_PTR`
- `pValidatedVoltages != NULL_PTR`

**后条件**：
- `pValidatedVoltages` 的内容与 `pCellVoltage` 相同，但 header 保持不变

---

#### REQ-009 — 单体电压最小/最大/平均值计算

| 属性 | 内容 |
|------|------|
| **编号** | REQ-009 |
| **优先级** | 高 |
| **关联函数** | `MRC_CalculateCellVoltageMinMaxAverage()` |

**描述**：系统应遍历所有电芯串的所有模组的所有电芯块，仅统计标记为有效的单体电压，计算每个电芯串的最小电压、最大电压、平均电压，并记录最小值和最大值对应的模组编号和电芯块编号。

**处理逻辑**：
1. 断言输入指针非空
2. 对每个电芯串：
   - 初始化：`min = INT16_MAX`, `max = INT16_MIN`, `sum = 0`, `nrValidCellVoltages = 0`
   - 遍历所有模组的所有电芯块
   - 仅处理 `invalidCellVoltage == false` 的电芯
   - 累加电压值，更新最小/最大值及其位置索引
3. 若该串有效电压数 > 0，计算平均值：`average = sum / nrValidCellVoltages`
4. 若该串有效电压数 == 0，设置平均值为 0，返回 `STD_NOT_OK`

**后条件**：
- `pMinMaxAverageValues` 中填充每个电芯串的最小/最大/平均电压及其位置信息
- `pMinMaxAverageValues->validMeasuredCellVoltages[s]` 记录各串有效电压数量

**返回状态**：
| 返回值 | 含义 |
|--------|------|
| `STD_OK` | 所有电芯串均有至少一个有效电压，统计计算成功 |
| `STD_NOT_OK` | 至少有一个电芯串的所有电压均无效 |

---

### 3.5 单体温度测量验证

#### REQ-010 — 单体温度测量验证主流程

| 属性 | 内容 |
|------|------|
| **编号** | REQ-010 |
| **优先级** | 高 |
| **关联函数** | `MRC_ValidateCellTemperatureMeasurement()` |
| **关联配置** | `MRC_AFE_MEASUREMENT_PERIOD_TIMEOUT_ms` |

**描述**：系统应对基础（Base）和冗余0（Redundancy0）两个通道的单体温度测量数据进行综合验证。验证流程与单体电压验证主流程（REQ-006）结构相同，但操作对象为温度数据。验证完成后计算最小/最大/平均单体温度并执行温度分布检查。

**处理流程**：

1. **判断是否使用冗余测量**：若冗余通道温度数据库条目至少曾被更新过一次，则启用冗余验证模式。

2. **基础测量时间戳检查**：超时时上报诊断事件（诊断 ID：`DIAG_ID_BASE_CELL_TEMPERATURE_MEASUREMENT_TIMEOUT`）。

3. **冗余测量时间戳检查**：超时时上报诊断事件（诊断 ID：`DIAG_ID_REDUNDANCY0_CELL_TEMPERATURE_MEASUREMENT_TIMEOUT`）。

4. **验证策略选择**：与 REQ-006 相同逻辑，将电压替换为温度。

5. **统计计算**：若有任何测量值被验证更新，计算最小/最大/平均单体温度（`MRC_CalculateCellTemperatureMinMaxAverage()`），随后执行温度分布检查（`PL_CheckTemperatureSpread()`）。若分布检查发现异常，重新计算最小/最大/平均值。

**前置条件**：
- `pCellTemperatureBase != NULL_PTR`
- `pCellTemperatureRedundancy0 != NULL_PTR`

**返回状态**：
| 返回值 | 含义 |
|--------|------|
| `true` | 已验证的温度数据库条目已更新 |
| `false` | 无需更新 |

---

#### REQ-011 — 单体温度冗余比较验证

| 属性 | 内容 |
|------|------|
| **编号** | REQ-011 |
| **优先级** | 高 |
| **关联函数** | `MRC_ValidateCellTemperature()` |

**描述**：系统应遍历所有电芯串的所有模组的所有温度传感器，对基础测量和冗余0测量的单体温度进行逐对比较验证。验证规则与单体电压冗余比较验证（REQ-007）相同：
- 两通道均有效 → 调用 `PL_CheckCellTemperature()` 合理性校验
- 仅一通道有效 → 直接使用该通道值
- 两通道均无效 → 取平均值作为填充值，标记为无效

每个电芯串处理完毕后记录有效温度数量和合理性诊断结果（诊断 ID：`DIAG_ID_PLAUSIBILITY_CELL_TEMP`）。

**前置条件**：
- `pCellTemperatureBase != NULL_PTR`
- `pCellTemperatureRedundancy0 != NULL_PTR`
- `pValidatedTemperatures != NULL_PTR`

**后条件**：
- `pValidatedTemperatures` 中包含每个温度传感器的最终验证温度值及有效性标志
- `pValidatedTemperatures->nrValidTemperatures[s]` 记录了每个电芯串的有效温度数量

**返回状态**：
| 返回值 | 含义 |
|--------|------|
| `STD_OK` | 所有温度传感器的温度均被成功验证 |
| `STD_NOT_OK` | 至少有一个温度传感器的温度无法被验证 |

---

#### REQ-012 — 单体温度单源更新验证

| 属性 | 内容 |
|------|------|
| **编号** | REQ-012 |
| **优先级** | 中 |
| **关联函数** | `MRC_UpdateCellTemperatureValidation()` |

**描述**：当只有一个温度测量源可用时，系统应将该测量源的整个数据库条目拷贝到已验证温度数据库条目中，同时保持目标条目的数据库 header 不变。逻辑与 REQ-008 相同，操作对象为温度数据。

---

#### REQ-013 — 单体温度最小/最大/平均值计算

| 属性 | 内容 |
|------|------|
| **编号** | REQ-013 |
| **优先级** | 高 |
| **关联函数** | `MRC_CalculateCellTemperatureMinMaxAverage()` |

**描述**：系统应遍历所有电芯串的所有模组的所有温度传感器，仅统计标记为有效的温度值，计算每个电芯串的最小温度、最大温度、平均温度，并记录最小值和最大值对应的模组编号和传感器编号。

**处理逻辑**：
1. 断言输入指针非空
2. 对每个电芯串：
   - 初始化：`min = INT16_MAX`, `max = INT16_MIN`, `sum_ddegC = 0.0f`, `nrValidCellTemperatures = 0`
   - 遍历所有模组的所有温度传感器
   - 仅处理 `invalidCellTemperature == false` 的传感器
   - 以浮点数累加温度值（`float_t`），更新最小/最大值及其位置索引
3. 若该串有效温度数 > 0，计算平均值：`average = sum_ddegC / (float_t)nrValidCellTemperatures`
4. 若该串有效温度数 == 0，设置平均值为 0.0f，返回 `STD_NOT_OK`

**后条件**：
- `pMinMaxAverageValues` 中填充每个电芯串的最小/最大/平均温度及其位置信息
- `pMinMaxAverageValues->validMeasuredCellTemperatures[s]` 记录各串有效温度数量

**返回状态**：
| 返回值 | 含义 |
|--------|------|
| `STD_OK` | 所有电芯串均有至少一个有效温度，统计计算成功 |
| `STD_NOT_OK` | 至少有一个电芯串的所有温度均无效 |

---

### 3.6 电池包测量验证

#### REQ-014 — 电池包测量综合验证

| 属性 | 内容 |
|------|------|
| **编号** | REQ-014 |
| **优先级** | 高 |
| **关联函数** | `MRC_ValidatePackMeasurement()` |

**描述**：系统应在每次电池包测量周期中，从数据库读取电流、电芯串电压（系统电压1）、高压总线电压（系统电压3）和功率测量数据，依次执行五项子验证：电流验证、电芯串电压验证、电池电压验证、高压总线电压验证、功率验证。所有验证完成后将电池包级别数据写回数据库。

**处理逻辑**：
1. 声明本地数据库表副本变量
2. 调用 `DATA_READ_DATA()` 读取四张表的数据
3. 依次调用：
   - `MRC_ValidateCurrentMeasurement()`
   - `MRC_ValidateStringVoltageMeasurement()`（传入已验证的单体电压表）
   - `MRC_ValidateBatteryVoltageMeasurement()`
   - `MRC_ValidateHighVoltageBusMeasurement()`
   - `MRC_ValidatePowerMeasurement()`
4. 调用 `DATA_WRITE_DATA()` 将 `mrc_tablePackValues` 写入数据库

---

#### REQ-015 — 电流测量验证

| 属性 | 内容 |
|------|------|
| **编号** | REQ-015 |
| **优先级** | 高 |
| **关联函数** | `MRC_ValidateCurrentMeasurement()` |
| **关联配置** | `MRC_CURRENT_MEASUREMENT_PERIOD_TIMEOUT_ms` (250ms) |

**描述**：系统应遍历所有电芯串，对每个电芯串的电流测量进行超时检查和有效性验证。若测量在超时时间内更新且数据有效，则采用该测量值并标记为有效；若超时或数据无效，则标记为无效。电池包总电流为所有有效电芯串电流之和，若任一电芯串电流无效，则电池包总电流亦标记为无效。

**处理逻辑**：
1. 断言输入指针非空
2. 初始化：`packCurrent_mA = 0`，清除 `invalidPackCurrent`
3. 遍历每个电芯串：
   - 调用 `MRC_MeasurementUpdatedRecently()` 检查是否超时
   - 上报超时诊断事件（诊断 ID：`DIAG_ID_CURRENT_MEASUREMENT_TIMEOUT`）
   - 若未超时且时间戳已更新：更新本地保存的时间戳和电流值
     - 若原始测量有效（`invalidMeasurement[s] == 0`）：标记该串电流有效
     - 若原始测量无效：标记该串电流无效，上报测量错误（诊断 ID：`DIAG_ID_CURRENT_MEASUREMENT_ERROR`）
   - 若超时：标记该串电流无效
   - 若该串电流有效，累加到 `packCurrent_mA`；否则设置 `invalidPackCurrent = 1`
4. 将最终总电流写入 `mrc_tablePackValues.packCurrent_mA`

---

#### REQ-016 — 电芯串电压测量验证

| 属性 | 内容 |
|------|------|
| **编号** | REQ-016 |
| **优先级** | 高 |
| **关联函数** | `MRC_ValidateStringVoltageMeasurement()` |
| **关联配置** | `MRC_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms` (300ms), `MRC_ALLOWED_NUMBER_OF_INVALID_CELL_VOLTAGES` (5) |

**描述**：系统应遍历所有电芯串，对每个电芯串的电流传感器高压测量值与 AFE 测量的电芯串总电压进行合理性校验。根据电流传感器和 AFE 测量数据的可用性，采用不同的电压确定策略。

**处理逻辑**：

对每个电芯串执行以下判断：

1. **电流传感器超时检查**：调用 `MRC_MeasurementUpdatedRecently()` 检查电流传感器测量是否在超时时间内更新，上报诊断事件（诊断 ID：`DIAG_ID_CURRENT_SENSOR_V1_MEASUREMENT_TIMEOUT`）。

2. **最优路径 — 双源合理性校验**：
   - 条件：电流传感器未超时 **且** 电流传感器测量有效 **且** AFE 所有电芯块电压均有效
   - 操作：调用 `PL_CheckStringVoltage()` 比较 AFE 总电压与电流传感器高压值
   - 采用电流传感器测量值作为电芯串电压
   - 若合理性校验通过 → 标记为有效；否则 → 标记为无效

3. **次优路径 A — 仅电流传感器有效**：
   - 条件：合理性校验不可行 **且** 电流传感器未超时 **且** 测量有效
   - 操作：直接采用电流传感器测量值，标记为有效

4. **次优路径 B — 仅 AFE 有效**：
   - 条件：合理性校验不可行 **且** AFE 所有电芯块电压均有效
   - 操作：直接采用 AFE 电芯串总电压，标记为有效

5. **降级路径 — 估算构造**：
   - 条件：电流传感器和 AFE 均不可靠
   - 操作：使用 AFE 电芯串总电压 + 平均单体电压 × 无效电芯数 进行估算
   - 若无效电芯数 ≤ `MRC_ALLOWED_NUMBER_OF_INVALID_CELL_VOLTAGES`(5) → 标记为有效
   - 若无效电芯数 > 5 → 标记为无效

---

#### REQ-017 — 电池电压测量验证

| 属性 | 内容 |
|------|------|
| **编号** | REQ-017 |
| **优先级** | 高 |
| **关联函数** | `MRC_ValidateBatteryVoltageMeasurement()` |

**描述**：系统应根据电芯串连接状态和电芯串电压有效性，计算电池包总电压。电池包总电压为所有有效且连接的电芯串电压的平均值。

**处理逻辑**：
1. 获取已连接电芯串数量：`numberOfConnectedStrings = BMS_GetNumberOfConnectedStrings()`
2. **有电芯串连接时**：遍历所有电芯串，仅累加同时满足"电压有效"和"电芯串已闭合"条件的电芯串电压
3. **无电芯串连接时**：遍历所有电芯串，累加所有电压有效的电芯串电压（不判断连接状态）
4. 若至少一个电芯串电压有效：计算平均值 `batteryVoltage_mV = sum / count`，标记电池电压有效
5. 若无有效电芯串电压：设置 `batteryVoltage_mV = INT32_MAX`，标记电池电压无效

**返回状态**：通过 `mrc_tablePackValues.invalidBatteryVoltage` 体现

---

#### REQ-018 — 高压总线电压测量验证

| 属性 | 内容 |
|------|------|
| **编号** | REQ-018 |
| **优先级** | 中 |
| **关联函数** | `MRC_ValidateHighVoltageBusMeasurement()` |
| **关联配置** | `MRC_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms` (300ms) |

**描述**：系统应遍历所有电芯串，对每个电芯串的高压总线电压测量（系统电压3）进行超时检查和有效性验证。高压总线电压为所有有效且已连接（闭合或预充电中）的电芯串的高压测量平均值。

**处理逻辑**：
1. 断言输入指针非空
2. 遍历所有电芯串：
   - 调用 `MRC_MeasurementUpdatedRecently()` 检查电流传感器测量是否超时
   - 上报超时诊断事件（诊断 ID：`DIAG_ID_CURRENT_SENSOR_V3_MEASUREMENT_TIMEOUT`）
   - 若电芯串闭合或预充电中 **且** 测量未超时 **且** 测量值有效 → 累加电压值，递增有效计数
3. 若有效计数 > 0：计算平均值 `highVoltageBusVoltage_mV = sum / validVoltages`，标记有效
4. 若有效计数 == 0：标记高压总线电压无效

---

#### REQ-019 — 功率测量验证

| 属性 | 内容 |
|------|------|
| **编号** | REQ-019 |
| **优先级** | 中 |
| **关联函数** | `MRC_ValidatePowerMeasurement()` |
| **关联配置** | `MRC_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms` (300ms) |

**描述**：系统应遍历所有电芯串，对每个电芯串的功率测量进行超时检查和有效性验证。若功率测量值有效且未超时，直接采用；若功率测量无效或超时，但电流和电芯串电压均有效，则通过 `P = I × V` 计算功率作为降级替代值。电池包总功率为所有有效电芯串功率之和。

**处理逻辑**：
1. 断言输入指针非空
2. 初始化：`packPower_W = 0`，清除 `invalidPackPower`
3. 遍历每个电芯串：
   - 检查功率测量是否超时（诊断 ID：`DIAG_ID_CURRENT_SENSOR_POWER_MEASUREMENT_TIMEOUT`）
   - 若未超时且时间戳已更新：
     - 若测量有效 → 直接采用，标记有效
     - 若测量无效 → 设置计算标志 `calculatePower = true`
   - 若超时 → 设置无效标志，设置计算标志 `calculatePower = true`
   - 若 `calculatePower == true` 且电流有效且电芯串电压有效：
     - 计算：`stringPower_W = (stringCurrent_mA / 1000.0f) × (stringVoltage_mV / 1000.0f)`
     - 标记有效，上报 `DIAG_EVENT_OK`
   - 若该串功率有效 → 累加到 `packPower_W`；否则设置 `invalidPackPower = 1`
   - 上报功率测量错误诊断事件（诊断 ID：`DIAG_ID_POWER_MEASUREMENT_ERROR`）
4. 将最终总功率写入 `mrc_tablePackValues.packPower_W`

---

### 3.7 诊断事件上报

#### REQ-020 — 测量超时诊断事件上报

| 属性 | 内容 |
|------|------|
| **编号** | REQ-020 |
| **优先级** | 高 |
| **关联函数** | `MRC_ValidateCellVoltageMeasurement()`, `MRC_ValidateCellTemperatureMeasurement()`, `MRC_ValidateCurrentMeasurement()`, `MRC_ValidateStringVoltageMeasurement()`, `MRC_ValidateHighVoltageBusMeasurement()`, `MRC_ValidatePowerMeasurement()` |

**描述**：系统应在各类测量数据超时时，通过 `DIAG_Handler()` 或 `DIAG_CheckEvent()` 向诊断系统上报相应的诊断事件。涉及以下诊断事件 ID：

| 诊断 ID | 触发条件 | 上报方式 | 事件级别 |
|--------|---------|---------|---------|
| `DIAG_ID_BASE_CELL_VOLTAGE_MEASUREMENT_TIMEOUT` | 基础单体电压测量超时 | `DIAG_Handler()` | `DIAG_SYSTEM` |
| `DIAG_ID_REDUNDANCY0_CELL_VOLTAGE_MEASUREMENT_TIMEOUT` | 冗余0单体电压测量超时 | `DIAG_Handler()` | `DIAG_SYSTEM` |
| `DIAG_ID_BASE_CELL_TEMPERATURE_MEASUREMENT_TIMEOUT` | 基础单体温度测量超时 | `DIAG_Handler()` | `DIAG_SYSTEM` |
| `DIAG_ID_REDUNDANCY0_CELL_TEMPERATURE_MEASUREMENT_TIMEOUT` | 冗余0单体温度测量超时 | `DIAG_Handler()` | `DIAG_SYSTEM` |
| `DIAG_ID_CURRENT_MEASUREMENT_TIMEOUT` | 电流测量超时 | `DIAG_CheckEvent()` | `DIAG_STRING` |
| `DIAG_ID_CURRENT_SENSOR_V1_MEASUREMENT_TIMEOUT` | 电流传感器系统电压1超时 | `DIAG_CheckEvent()` | `DIAG_STRING` |
| `DIAG_ID_CURRENT_SENSOR_V3_MEASUREMENT_TIMEOUT` | 电流传感器系统电压3超时 | `DIAG_CheckEvent()` | `DIAG_STRING` |
| `DIAG_ID_CURRENT_SENSOR_POWER_MEASUREMENT_TIMEOUT` | 电流传感器功率测量超时 | `DIAG_CheckEvent()` | `DIAG_STRING` |

---

#### REQ-021 — 测量及合理性异常诊断事件上报

| 属性 | 内容 |
|------|------|
| **编号** | REQ-021 |
| **优先级** | 高 |
| **关联函数** | `MRC_ValidateCellVoltage()`, `MRC_ValidateCellTemperature()`, `MRC_ValidateCurrentMeasurement()`, `MRC_ValidateStringVoltageMeasurement()`, `MRC_ValidatePowerMeasurement()` |

**描述**：系统应在发现测量值异常或合理性校验失败时，上报相应诊断事件：

| 诊断 ID | 触发条件 | 上报方式 | 事件级别 |
|--------|---------|---------|---------|
| `DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE` | 电芯串中存在单体电压合理性校验失败 | `DIAG_CheckEvent()` | `DIAG_STRING` |
| `DIAG_ID_PLAUSIBILITY_CELL_TEMP` | 电芯串中存在单体温度合理性校验失败 | `DIAG_CheckEvent()` | `DIAG_STRING` |
| `DIAG_ID_PLAUSIBILITY_PACK_VOLTAGE` | 电芯串总电压合理性校验失败或无法执行 | `DIAG_CheckEvent()` | `DIAG_STRING` |
| `DIAG_ID_CURRENT_MEASUREMENT_ERROR` | 电芯串电流测量值无效 | `DIAG_Handler()` | `DIAG_STRING` |
| `DIAG_ID_POWER_MEASUREMENT_ERROR` | 电芯串功率测量值无效 | `DIAG_Handler()` | `DIAG_STRING` |

---

### 3.8 配置参数

#### REQ-022 — 测量超时阈值配置

| 属性 | 内容 |
|------|------|
| **编号** | REQ-022 |
| **优先级** | 中 |
| **关联宏** | `MRC_CURRENT_MEASUREMENT_PERIOD_TIMEOUT_ms`, `MRC_AFE_MEASUREMENT_PERIOD_TIMEOUT_ms`, `MRC_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms` |

**描述**：系统应通过以下编译期宏定义配置各类测量的超时阈值：

| 宏定义 | 值 | 说明 |
|--------|-----|------|
| `MRC_CURRENT_MEASUREMENT_PERIOD_TIMEOUT_ms` | 250ms | 电流测量更新超时时间 |
| `MRC_AFE_MEASUREMENT_PERIOD_TIMEOUT_ms` | 250ms | AFE 基础/冗余测量更新超时时间 |
| `MRC_CURRENT_SENSOR_MEASUREMENT_TIMEOUT_ms` | 300ms | 电流传感器（高压/功率）测量更新超时时间 |

**设计说明**：AFE 和电流测量使用 250ms 超时（与测量周期匹配），电流传感器测量使用较长的 300ms 超时（考虑传感器通信延迟）。

---

#### REQ-023 — 无效单体电压容许数量配置

| 属性 | 内容 |
|------|------|
| **编号** | REQ-023 |
| **优先级** | 低 |
| **关联宏** | `MRC_ALLOWED_NUMBER_OF_INVALID_CELL_VOLTAGES` |

**描述**：系统应定义在电芯串电压估算降级路径中允许的最大无效单体电压数量。当无效电芯数不超过 `MRC_ALLOWED_NUMBER_OF_INVALID_CELL_VOLTAGES`（默认值 5）时，估算结果仍标记为有效；超过此数量时标记为无效。

---

## 4. 非功能需求

### 4.1 时序与性能

| 编号 | 需求描述 | 关联 |
|------|---------|------|
| NFR-001 | 所有验证函数应在单次 BMS 任务周期内完成执行，不包含阻塞等待 | `MRC_ValidateAfeMeasurement()`, `MRC_ValidatePackMeasurement()` |
| NFR-002 | 时间戳检查使用无符号整数运算，无需调用 `abs()` 处理定时器溢出，保证 O(1) 时间复杂度 | `MRC_MeasurementUpdatedRecently()` |
| NFR-003 | 最小/最大/平均值计算应一次遍历完成所有统计（min/max/sum/count），避免多次遍历 | `MRC_CalculateCellVoltageMinMaxAverage()`, `MRC_CalculateCellTemperatureMinMaxAverage()` |

### 4.2 内存

| 编号 | 需求描述 | 关联 |
|------|---------|------|
| NFR-004 | 大型数据库表副本（`DATA_BLOCK_CELL_VOLTAGE_s`, `DATA_BLOCK_CELL_TEMPERATURE_s`）必须使用 `static` 存储类别声明，放置在数据段（data segment）而非堆栈上 | `MRC_ValidateAfeMeasurement()` |
| NFR-005 | 模块状态结构体 `MRC_STATE_s` 应使用 `static` 存储类别，避免外部直接访问 | `mrc_state` |

### 4.3 鲁棒性

| 编号 | 需求描述 | 关联 |
|------|---------|------|
| NFR-006 | 所有接收指针参数的函数必须在入口处使用 `FAS_ASSERT` 进行非空断言检查 | 全部静态函数 |
| NFR-007 | 数据库条目整体拷贝时必须保护和恢复目标条目的 `header` 字段，防止元数据被覆盖 | `MRC_UpdateCellVoltageValidation()`, `MRC_UpdateCellTemperatureValidation()` |
| NFR-008 | 除法运算前必须检查除数不为零（如有效电压/温度计数为零时跳过平均值计算） | `MRC_CalculateCellVoltageMinMaxAverage()`, `MRC_CalculateCellTemperatureMinMaxAverage()` |
| NFR-009 | 当基础测量和冗余测量均失效时，取两通道平均值作为填充值，而非留空 | `MRC_ValidateCellVoltage()`, `MRC_ValidateCellTemperature()` |

### 4.4 可配置性

| 编号 | 需求描述 | 关联 |
|------|---------|------|
| NFR-010 | 所有超时阈值应通过宏定义配置，便于针对不同硬件平台调整 | `MRC_*_TIMEOUT_ms` 系列宏 |
| NFR-011 | 无效单体电压容许数量应通过宏定义配置 | `MRC_ALLOWED_NUMBER_OF_INVALID_CELL_VOLTAGES` |

### 4.5 可测试性

| 编号 | 需求描述 | 关联 |
|------|---------|------|
| NFR-012 | 所有内部静态函数应在 `UNITY_UNIT_TEST` 编译条件下暴露为 `TEST_` 前缀的外部函数，支持单元测试直接调用 | `#ifdef UNITY_UNIT_TEST` 块 |

---

## 5. 接口需求

### 5.1 公共 API

| 函数 | 签名 | 说明 |
|------|------|------|
| `MRC_Initialize` | `STD_RETURN_TYPE_e MRC_Initialize(void)` | 初始化冗余模块，将所有内部数据标记为无效 |
| `MRC_ValidateAfeMeasurement` | `STD_RETURN_TYPE_e MRC_ValidateAfeMeasurement(void)` | 验证冗余 AFE 测量值（单体电压+温度） |
| `MRC_ValidatePackMeasurement` | `STD_RETURN_TYPE_e MRC_ValidatePackMeasurement(void)` | 验证电池包级别测量值（电流/电压/功率） |

### 5.2 依赖项

| 模块 | 头文件 | 使用的接口 |
|------|--------|----------|
| BMS | `bms.h` | `BMS_GetNumberOfConnectedStrings()`, `BMS_IsStringClosed()`, `BMS_IsStringPrecharging()` |
| Database | `database.h` | `DATA_READ_DATA()`, `DATA_WRITE_DATA()`, `DATA_DatabaseEntryUpdatedAtLeastOnce()`, `DATA_EntryUpdatedWithinInterval()` |
| DIAG | `diag.h` | `DIAG_Handler()`, `DIAG_CheckEvent()` |
| Plausibility | `plausibility.h` | `PL_CheckCellVoltage()`, `PL_CheckCellTemperature()`, `PL_CheckVoltageSpread()`, `PL_CheckTemperatureSpread()`, `PL_CheckStringVoltage()` |
| OS | `os.h` | `OS_GetTickCount()` |
| Math | `foxmath.h`, `math.h` | `UNIT_CONVERSION_FACTOR_1000_FLOAT`, `float_t` 类型 |

### 5.3 调用方

本模块由 BMS 状态机（`BMS_StateMachine()`）调用。调用顺序为：先调用 `MRC_Initialize()` 初始化，然后在每个测量周期依次调用 `MRC_ValidateAfeMeasurement()` 和 `MRC_ValidatePackMeasurement()`。

### 5.4 数据库表交互

| 数据库表 ID | 交互方式 | 说明 |
|------------|---------|------|
| `DATA_BLOCK_ID_CELL_VOLTAGE` | 写入 | 存储已验证的单体电压值 |
| `DATA_BLOCK_ID_CELL_TEMPERATURE` | 写入 | 存储已验证的单体温度值 |
| `DATA_BLOCK_ID_MIN_MAX` | 写入 | 存储最小/最大/平均电压和温度 |
| `DATA_BLOCK_ID_PACK_VALUES` | 写入 | 存储电池包级别的电流/电压/功率 |
| `DATA_BLOCK_ID_CELL_VOLTAGE_BASE` | 读取 | 读取基础通道单体电压 |
| `DATA_BLOCK_ID_CELL_VOLTAGE_REDUNDANCY0` | 读取 | 读取冗余0通道单体电压 |
| `DATA_BLOCK_ID_CELL_TEMPERATURE_BASE` | 读取 | 读取基础通道单体温度 |
| `DATA_BLOCK_ID_CELL_TEMPERATURE_REDUNDANCY0` | 读取 | 读取冗余0通道单体温度 |
| `DATA_BLOCK_ID_CURRENT` | 读取 | 读取电流测量值 |
| `DATA_BLOCK_ID_SYSTEM_VOLTAGE_1` | 读取 | 读取电流传感器系统电压1 |
| `DATA_BLOCK_ID_SYSTEM_VOLTAGE_3` | 读取 | 读取电流传感器系统电压3 |
| `DATA_BLOCK_ID_POWER` | 读取 | 读取功率测量值 |

---

## 6. 数据结构

### 6.1 MRC_STATE_s

```c
typedef struct {
    uint32_t lastBaseCellVoltageTimestamp;             /**< 上一次基础单体电压时间戳 */
    uint32_t lastRedundancy0CellVoltageTimestamp;      /**< 上一次冗余0单体电压时间戳 */
    uint32_t lastBaseCellTemperatureTimestamp;         /**< 上一次基础单体温度时间戳 */
    uint32_t lastRedundancy0CellTemperatureTimestamp;  /**< 上一次冗余0单体温度时间戳 */
    uint32_t lastStringCurrentTimestamp[BS_NR_OF_STRINGS]; /**< 各串上次电流时间戳 */
    uint32_t lastStringPowerTimestamp[BS_NR_OF_STRINGS];   /**< 各串上次功率时间戳 */
} MRC_STATE_s;
```

### 6.2 静态本地数据库副本

| 变量 | 类型 | 数据库 ID | 说明 |
|------|------|----------|------|
| `mrc_tableMinimumMaximumValues` | `DATA_BLOCK_MIN_MAX_s` | `DATA_BLOCK_ID_MIN_MAX` | 最小/最大/平均值 |
| `mrc_tableCellVoltages` | `DATA_BLOCK_CELL_VOLTAGE_s` | `DATA_BLOCK_ID_CELL_VOLTAGE` | 已验证单体电压 |
| `mrc_tableCellTemperatures` | `DATA_BLOCK_CELL_TEMPERATURE_s` | `DATA_BLOCK_ID_CELL_TEMPERATURE` | 已验证单体温度 |
| `mrc_tablePackValues` | `DATA_BLOCK_PACK_VALUES_s` | `DATA_BLOCK_ID_PACK_VALUES` | 已验证电池包数据 |

---

## 7. 控制流

### 7.1 初始化流程

```
MRC_Initialize()
  ├─ 遍历所有电芯串
  │   ├─ 设置 invalidStringVoltage[s] = true
  │   └─ 遍历所有模组
  │       ├─ 遍历所有电芯块: invalidCellVoltage[s][m][cb] = true
  │       ├─ invalidModuleVoltage[s][m] = true
  │       └─ 遍历所有温度传感器: invalidCellTemperature[s][m][ts] = true
  ├─ 设置电芯串级无效标志 (voltage/current/power)
  ├─ 设置电池包级无效标志 (current/voltage/hvBus/power)
  └─ DATA_WRITE_DATA() → 写入数据库
```

### 7.2 AFE 测量验证流程

```
MRC_ValidateAfeMeasurement()
  ├─ DATA_READ_DATA() ← 读取 Base 和 Redundancy0 的电压/温度数据
  ├─ MRC_ValidateCellVoltageMeasurement()
  │   ├─ 检查是否启用冗余
  │   ├─ 检查基础测量时间戳 → 超时则上报 DIAG
  │   ├─ 检查冗余测量时间戳 → 超时则上报 DIAG
  │   ├─ 根据数据可用性选择策略:
  │   │   ├─ 两通道均更新 → MRC_ValidateCellVoltage() [冗余比较]
  │   │   ├─ 仅一通道更新 + 另一通道超时 → MRC_UpdateCellVoltageValidation() [单源]
  │   │   └─ 无更新 → 跳过
  │   ├─ MRC_CalculateCellVoltageMinMaxAverage() [统计计算]
  │   └─ PL_CheckVoltageSpread() [分布检查]
  │       └─ 若有异常 → MRC_CalculateCellVoltageMinMaxAverage() [重算]
  └─ MRC_ValidateCellTemperatureMeasurement()
      └─ (同上结构的温度版本)
```

### 7.3 电池包测量验证流程

```
MRC_ValidatePackMeasurement()
  ├─ DATA_READ_DATA() ← 读取电流、电压、功率数据
  ├─ MRC_ValidateCurrentMeasurement()        [电流验证]
  ├─ MRC_ValidateStringVoltageMeasurement()  [电芯串电压验证]
  ├─ MRC_ValidateBatteryVoltageMeasurement() [电池电压验证]
  ├─ MRC_ValidateHighVoltageBusMeasurement() [高压总线电压验证]
  ├─ MRC_ValidatePowerMeasurement()          [功率验证]
  └─ DATA_WRITE_DATA() → 写入 mrc_tablePackValues
```

---

## 8. 追溯矩阵

| 需求编号 | 需求名称 | 关联函数/宏 | 优先级 |
|---------|---------|------------|--------|
| REQ-001 | 冗余模块初始化 | `MRC_Initialize()` | 高 |
| REQ-002 | 冗余模块内部状态维护 | `MRC_STATE_s`, `mrc_state` | 中 |
| REQ-003 | 测量数据至少更新一次检查 | `MRC_MeasurementUpdatedAtLeastOnce()` | 高 |
| REQ-004 | 测量数据近期更新检查 | `MRC_MeasurementUpdatedRecently()` | 高 |
| REQ-005 | AFE 测量综合验证 | `MRC_ValidateAfeMeasurement()` | 高 |
| REQ-006 | 单体电压测量验证主流程 | `MRC_ValidateCellVoltageMeasurement()` | 高 |
| REQ-007 | 单体电压冗余比较验证 | `MRC_ValidateCellVoltage()` | 高 |
| REQ-008 | 单体电压单源更新验证 | `MRC_UpdateCellVoltageValidation()` | 中 |
| REQ-009 | 单体电压最小/最大/平均值计算 | `MRC_CalculateCellVoltageMinMaxAverage()` | 高 |
| REQ-010 | 单体温度测量验证主流程 | `MRC_ValidateCellTemperatureMeasurement()` | 高 |
| REQ-011 | 单体温度冗余比较验证 | `MRC_ValidateCellTemperature()` | 高 |
| REQ-012 | 单体温度单源更新验证 | `MRC_UpdateCellTemperatureValidation()` | 中 |
| REQ-013 | 单体温度最小/最大/平均值计算 | `MRC_CalculateCellTemperatureMinMaxAverage()` | 高 |
| REQ-014 | 电池包测量综合验证 | `MRC_ValidatePackMeasurement()` | 高 |
| REQ-015 | 电流测量验证 | `MRC_ValidateCurrentMeasurement()` | 高 |
| REQ-016 | 电芯串电压测量验证 | `MRC_ValidateStringVoltageMeasurement()` | 高 |
| REQ-017 | 电池电压测量验证 | `MRC_ValidateBatteryVoltageMeasurement()` | 高 |
| REQ-018 | 高压总线电压测量验证 | `MRC_ValidateHighVoltageBusMeasurement()` | 中 |
| REQ-019 | 功率测量验证 | `MRC_ValidatePowerMeasurement()` | 中 |
| REQ-020 | 测量超时诊断事件上报 | 6 个验证函数中的超时检查 | 高 |
| REQ-021 | 测量及合理性异常诊断事件上报 | 5 个验证函数中的异常检查 | 高 |
| REQ-022 | 测量超时阈值配置 | `MRC_*_TIMEOUT_ms` (3 个宏) | 中 |
| REQ-023 | 无效单体电压容许数量配置 | `MRC_ALLOWED_NUMBER_OF_INVALID_CELL_VOLTAGES` | 低 |
