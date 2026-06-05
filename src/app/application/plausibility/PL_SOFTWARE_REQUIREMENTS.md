# Plausibility（合理性校验）模块 — 软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 电池管理系统中 **Plausibility（合理性校验）模块**的软件需求规格。该模块负责对电池单体电压和温度测量值进行合理性校验（Plausibility Check），确保测量数据的可信度，是电池管理系统安全运行的关键环节。

### 1.2 范围

| 项目 | 说明 |
|------|------|
| **涵盖** | 电池包总电压一致性校验、单体电压冗余校验、单体温度冗余校验、电压分布检查、温度分布检查、诊断事件上报 |
| **不涵盖** | 电压/温度传感器硬件原始数据采集（由 DRIVER 层负责）、电芯平衡控制策略、SOC/SOH 估算算法 |

### 1.3 定义与缩略语

| 缩略语 | 英文全称 | 中文说明 |
|--------|---------|---------|
| AFE | Analog Front-End | 模拟前端采集芯片 |
| PL | Plausibility | 合理性校验模块前缀 |
| SRS | Software Requirements Specification | 软件需求规格说明 |
| RTM | Requirements Traceability Matrix | 需求可追溯性矩阵 |
| BMS | Battery Management System | 电池管理系统 |
| dK | Deci Kelvin | 分开尔文（0.1°C） |
| DIAG | Diagnostics | 诊断模块 |

### 1.4 参考文献

| 编号 | 文档 | 说明 |
|------|------|------|
| REF-01 | [battery_system_cfg.h](../config/battery_system_cfg.h) | 电池系统配置 |
| REF-02 | [plausibility_cfg.h](../config/plausibility_cfg.h) | 合理性校验配置 |
| REF-03 | [diag_cfg.h](../../engine/config/diag_cfg.h) | 诊断事件配置 |
| REF-04 | [database_cfg.h](../../engine/config/database_cfg.h) | 数据库结构定义 |

---

## 2. 总体描述

### 2.1 产品视角

Plausibility 模块位于 APPLICAITON 层，是 BMS 应用层的一个子模块。其位置和作用如下：

```
┌─────────────────────────────────────┐
│           APPLICATION 层             │
│  ┌─────────┐  ┌─────────┐          │
│  │  BMS    │  │   BAL   │  ...     │
│  └────┬────┘  └─────────┘          │
│       │                              │
│  ┌────▼──────────────────────┐      │
│  │     Plausibility (PL)      │      │
│  │  ┌──────────────────────┐ │      │
│  │  │ 冗余校验 / 分布检查    │ │      │
│  │  └──────────┬───────────┘ │      │
│  └─────────────┼─────────────┘      │
│                │                     │
│  ┌─────────────▼─────────────┐      │
│  │  Database / DIAG / Config  │      │
│  └───────────────────────────┘      │
└─────────────────────────────────────┘
```

### 2.2 工作模式

该模块以**函数库**形式工作，不包含状态机，由上层 BMS 控制模块按需调用：

| 调用阶段 | 调用函数 | 说明 |
|---------|---------|------|
| 电压测量后 | `PL_CheckCellVoltage()` | 对每对冗余单体电压进行校验 |
| 温度测量后 | `PL_CheckCellTemperature()` | 对每对冗余温度进行校验 |
| 电压数据汇总后 | `PL_CheckVoltageSpread()` | 全电芯电压分布检查 |
| 温度数据汇总后  | `PL_CheckTemperatureSpread()` | 全传感器温度分布检查 |
| 电池包总压读取后 | `PL_CheckStringVoltage()` | AFE 与电流传感器总压校验 |

### 2.3 用户特征

本模块的使用者是 BMS 上层控制逻辑（通过 `BMS_StateMachine()` 调用），不直接面对外部用户或工具链。

---

## 3. 功能需求

### 3.1 电池包总电压一致性校验

#### REQ-001 — 电池包总电压一致性校验

| 属性 | 内容 |
|------|------|
| **编号** | REQ-001 |
| **优先级** | 高 |
| **关联函数** | `PL_CheckStringVoltage()` |
| **关联配置** | `PL_STRING_VOLTAGE_TOLERANCE_mV` |

**描述**：系统应对 AFE 测量的电池包总电压与电流传感器测量的电池包总电压进行一致性校验。当两者偏差的绝对值小于配置容差阈值时，判定为合理；否则判定为不合理。

**处理逻辑**：
1. 计算 AFE 电压与电流传感器电压的差值：`diff_mV = voltageAfe_mV - voltageCurrentSensor_mV`
2. 若 `|diff_mV| < PL_STRING_VOLTAGE_TOLERANCE_mV`，返回 `STD_OK`
3. 否则返回 `STD_NOT_OK`

**返回状态**：
| 返回值 | 含义 |
|--------|------|
| `STD_OK` | 电池包总电压合理 |
| `STD_NOT_OK` | 电池包总电压不合理 |

---

### 3.2 单体电压/温度冗余校验

#### REQ-002 — 单体电压冗余校验与平均值输出

| 属性 | 内容 |
|------|------|
| **编号** | REQ-002 |
| **优先级** | 高 |
| **关联函数** | `PL_CheckCellVoltage()` |
| **关联配置** | `PL_CELL_VOLTAGE_TOLERANCE_mV` |

**描述**：系统应对两个冗余通道（base 和 redundancy0）测量的同一单体电压进行一致性校验。当偏差的绝对值超过配置容差阈值时，判定不合理。校验完成后无论结果如何，均将两通道测量的算术平均值写入输出指针。

**处理逻辑**：
1. 断言输出指针 `pCellVoltage` 非空
2. 计算偏差：`|baseCellVoltage - redundancy0CellVoltage|`
3. 若偏差 `> PL_CELL_VOLTAGE_TOLERANCE_mV`，返回 `STD_NOT_OK`
4. 计算平均值：`*pCellVoltage = (baseCellVoltage + redundancy0CellVoltage) / 2`
5. 若偏差未超阈值，返回 `STD_OK`

**前条件**：`pCellVoltage` 不为 `NULL_PTR`

**后条件**：`*pCellVoltage` 被写入两个冗余通道测量的算术平均值

---

#### REQ-003 — 单体温度冗余校验与平均值输出

| 属性 | 内容 |
|------|------|
| **编号** | REQ-003 |
| **优先级** | 高 |
| **关联函数** | `PL_CheckCellTemperature()` |
| **关联配置** | `PL_CELL_TEMPERATURE_TOLERANCE_dK` |

**描述**：系统应对两个冗余通道（base 和 redundancy0）测量的同一温度传感器值进行一致性校验。当偏差的绝对值超过配置容差阈值时，判定不合理。校验完成后无论结果如何，均将两通道测量的算术平均值写入输出指针。

**处理逻辑**：
1. 断言输出指针 `pCellTemperature` 非空
2. 计算偏差：`|baseCellTemperature - redundancy0CellTemperature|`
3. 若偏差 `> PL_CELL_TEMPERATURE_TOLERANCE_dK`，返回 `STD_NOT_OK`
4. 计算平均值：`*pCellTemperature = (baseCellTemperature + redundancy0CellTemperature) / 2`
5. 若偏差未超阈值，返回 `STD_OK`

**前条件**：`pCellTemperature` 不为 `NULL_PTR`

**后条件**：`*pCellTemperature` 被写入两个冗余通道测量的算术平均值（单位：ddegC）

---

### 3.3 电压/温度分布检查

#### REQ-004 — 单体电压分布检查与无效电压标记

| 属性 | 内容 |
|------|------|
| **编号** | REQ-004 |
| **优先级** | 高 |
| **关联函数** | `PL_CheckVoltageSpread()` |
| **关联配置** | `PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV` |

**描述**：系统应遍历所有电芯串（String）中的所有模组（Module）中的所有电芯块（Cell Block），对每个有效的单体电压，检查其与所属电芯串平均电压的偏差。若偏差的绝对值超过配置的分布容差阈值，则将该单体电压标记为无效并返回不合理状态。

**处理逻辑**：
1. 断言输入指针 `pCellVoltages` 和 `pMinMaxAverageValues` 非空
2. 遍历：`string (0..BS_NR_OF_STRINGS-1)` → `module (0..BS_NR_OF_MODULES_PER_STRING-1)` → `cellBlock (0..BS_NR_OF_CELL_BLOCKS_PER_MODULE-1)`
3. 跳过已标记为无效的电压（`invalidCellVoltage[s][m][cb] == true`）
4. 计算偏差：`|cellVoltage_mV[s][m][cb] - averageCellVoltage_mV[s]|`
5. 若偏差 `> PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV`：
   - 设置电芯串级异常标志 `plausibilityIssueDetected = STD_NOT_OK`
   - 设置函数返回值 `retval = STD_NOT_OK`
   - 将该电压标记为无效：`invalidCellVoltage[s][m][cb] = true`

**前条件**：
- `pCellVoltages` 不为 `NULL_PTR`
- `pMinMaxAverageValues` 不为 `NULL_PTR`

**后条件**：
- 偏差超限的电芯在 `invalidCellVoltage` 中被标记为 `true`
- 若有任何电芯被标记，返回 `STD_NOT_OK`

---

#### REQ-005 — 单体温度分布检查与无效温度标记

| 属性 | 内容 |
|------|------|
| **编号** | REQ-005 |
| **优先级** | 高 |
| **关联函数** | `PL_CheckTemperatureSpread()` |
| **关联配置** | `PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK` |

**描述**：系统应遍历所有电芯串中的所有模组中的所有温度传感器，对每个有效的温度值，检查其与所属电芯串平均温度的偏差。若偏差的绝对值超过配置的分布容差阈值，则将该温度传感器标记为无效并返回不合理状态；若偏差未超阈值，则递增有效温度计数。

**处理逻辑**：
1. 断言输入指针 `pCellTemperatures` 和 `pMinMaxAverageValues` 非空
2. 遍历：`string (0..BS_NR_OF_STRINGS-1)` → `module (0..BS_NR_OF_MODULES_PER_STRING-1)` → `tempSensor (0..BS_NR_OF_TEMP_SENSORS_PER_MODULE-1)`
3. 跳过已标记为无效的温度（`invalidCellTemperature[s][m][ts] == true`）
4. 计算偏差：`|cellTemperature_ddegC[s][m][ts] - averageTemperature_ddegC[s]|`（注意 `averageTemperature_ddegC` 需类型转换为 `int16_t`）
5. 若偏差 `> PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK`：
   - 设置电芯串级异常标志 `plausibilityIssueDetected = STD_NOT_OK`
   - 设置函数返回值 `retval = STD_NOT_OK`
   - 将该温度标记为无效：`invalidCellTemperature[s][m][ts] = true`
6. 若偏差未超阈值：
   - 递增有效温度计数：`nrValidTemperatures[s]++`

**前条件**：
- `pCellTemperatures` 不为 `NULL_PTR`
- `pMinMaxAverageValues` 不为 `NULL_PTR`

**后条件**：
- 偏差超限的传感器在 `invalidCellTemperature` 中被标记为 `true`
- 偏差未超限的传感器对应的 `nrValidTemperatures[s]` 递增
- 若有任何传感器被标记，返回 `STD_NOT_OK`

---

### 3.4 诊断事件上报

#### REQ-006 — 电压分布异常诊断事件上报

| 属性 | 内容 |
|------|------|
| **编号** | REQ-006 |
| **优先级** | 中 |
| **关联函数** | `PL_CheckVoltageSpread()` |
| **关联诊断ID** | `DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE_SPREAD` |

**描述**：系统应在完成每个电芯串的电压分布检查后，根据该电芯串的检查结果（`plausibilityIssueDetected`）调用 `DIAG_CheckEvent()` 上报诊断事件。诊断事件以电芯串为单位（参数 `s`），诊断级别为 `DIAG_WARNING`。

**处理逻辑**：
1. 对每个电芯串完成电压分布遍历后，调用 `DIAG_CheckEvent(plausibilityIssueDetected, DIAG_ID_PLAUSIBILITY_CELL_VOLTAGE_SPREAD, DIAG_STRING, s)`
2. 其中 `s` 为当前电芯串编号

---

#### REQ-007 — 温度分布异常诊断事件上报

| 属性 | 内容 |
|------|------|
| **编号** | REQ-007 |
| **优先级** | 中 |
| **关联函数** | `PL_CheckTemperatureSpread()` |
| **关联诊断ID** | `DIAG_ID_PLAUSIBILITY_CELL_TEMPERATURE_SPREAD` |

**描述**：系统应在完成每个电芯串的温度分布检查后，根据该电芯串的检查结果（`plausibilityIssueDetected`）调用 `DIAG_CheckEvent()` 上报诊断事件。诊断事件以电芯串为单位（参数 `s`），诊断级别为 `DIAG_WARNING`。

**处理逻辑**：
1. 对每个电芯串完成温度分布遍历后，调用 `DIAG_CheckEvent(plausibilityIssueDetected, DIAG_ID_PLAUSIBILITY_CELL_TEMPERATURE_SPREAD, DIAG_STRING, s)`
2. 其中 `s` 为当前电芯串编号

---

### 3.5 输入安全校验

#### REQ-008 — 输出指针参数有效性断言

| 属性 | 内容 |
|------|------|
| **编号** | REQ-008 |
| **优先级** | 高 |
| **关联函数** | `PL_CheckCellVoltage()`, `PL_CheckCellTemperature()` |

**描述**：系统应在函数入口处对输出指针参数进行非空断言检查（`FAS_ASSERT`），防止空指针解引用导致的未定义行为。若断言失败，系统进入安全状态（FAS 断言处理器介入）。

**断言项**：
| 函数 | 断言条件 |
|------|---------|
| `PL_CheckCellVoltage()` | `pCellVoltage != NULL_PTR` |
| `PL_CheckCellTemperature()` | `pCellTemperature != NULL_PTR` |

---

#### REQ-009 — 数据库指针参数有效性断言

| 属性 | 内容 |
|------|------|
| **编号** | REQ-009 |
| **优先级** | 高 |
| **关联函数** | `PL_CheckVoltageSpread()`, `PL_CheckTemperatureSpread()` |

**描述**：系统应在函数入口处对数据库结构体指针进行非空断言检查（`FAS_ASSERT`），防止对无效内存区域的访问。

**断言项**：
| 函数 | 断言条件 |
|------|---------|
| `PL_CheckVoltageSpread()` | `pCellVoltages != NULL_PTR` 且 `pMinMaxAverageValues != NULL_PTR` |
| `PL_CheckTemperatureSpread()` | `pCellTemperatures != NULL_PTR` 且 `pMinMaxAverageValues != NULL_PTR` |

---

#### REQ-010 — 跳过无效电压数据

| 属性 | 内容 |
|------|------|
| **编号** | REQ-010 |
| **优先级** | 中 |
| **关联函数** | `PL_CheckVoltageSpread()` |

**描述**：系统在进行电压分布检查时，应跳过已标记为无效（`invalidCellVoltage[s][m][cb] == true`）的单体电压，仅对当前有效的数据执行分布检查。此设计确保已由先前校验步骤（如冗余校验）标记为不可信的电压数据不会参与分布计算和二次标记。

---

#### REQ-011 — 跳过无效温度数据并计数有效温度

| 属性 | 内容 |
|------|------|
| **编号** | REQ-011 |
| **优先级** | 中 |
| **关联函数** | `PL_CheckTemperatureSpread()` |

**描述**：系统在进行温度分布检查时，应跳过已标记为无效（`invalidCellTemperature[s][m][ts] == true`）的温度传感器数据。对于通过分布检查的有效温度传感器，应递增该电芯串的有效温度计数器（`nrValidTemperatures[s]`），供其他模块（如温度状态判断）使用。

---

### 3.6 可配置性

#### REQ-012 — 电池包总电压容差可配置

| 属性 | 内容 |
|------|------|
| **编号** | REQ-012 |
| **优先级** | 中 |
| **关联宏** | `PL_STRING_VOLTAGE_TOLERANCE_mV` |

**描述**：系统电池包总电压一致性校验的容差阈值应通过配置宏 `PL_STRING_VOLTAGE_TOLERANCE_mV` 进行定义，默认值为 3000 mV，允许范围为 [0, 10000] mV。

---

#### REQ-013 — 冗余单体电压容差可配置

| 属性 | 内容 |
|------|------|
| **编号** | REQ-013 |
| **优先级** | 中 |
| **关联宏** | `PL_CELL_VOLTAGE_TOLERANCE_mV` |

**描述**：系统冗余单体电压校验的容差阈值应通过配置宏 `PL_CELL_VOLTAGE_TOLERANCE_mV` 进行定义，默认值为 10 mV，允许范围为 [0, 10000] mV。

---

#### REQ-014 — 冗余单体温度容差可配置

| 属性 | 内容 |
|------|------|
| **编号** | REQ-014 |
| **优先级** | 中 |
| **关联宏** | `PL_CELL_TEMPERATURE_TOLERANCE_dK` |

**描述**：系统冗余单体温度校验的容差阈值应通过配置宏 `PL_CELL_TEMPERATURE_TOLERANCE_dK` 进行定义，默认值为 50 dK（即 5°C），允许范围为 [0, 100] dK。

---

#### REQ-015 — 电压分布容差可配置

| 属性 | 内容 |
|------|------|
| **编号** | REQ-015 |
| **优先级** | 中 |
| **关联宏** | `PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV` |

**描述**：系统电压分布检查的容差阈值应通过配置宏 `PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV` 进行定义，默认值为 300 mV，允许范围为 [0, 10000] mV。

---

#### REQ-016 — 温度分布容差可配置

| 属性 | 内容 |
|------|------|
| **编号** | REQ-016 |
| **优先级** | 中 |
| **关联宏** | `PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK` |

**描述**：系统温度分布检查的容差阈值应通过配置宏 `PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK` 进行定义，默认值为 100 dK（即 10°C），允许范围为 [0, 100] dK。

---

## 4. 非功能需求

### 4.1 时序与性能

| 编号 | 描述 |
|------|------|
| NFR-4.1.1 | `PL_CheckStringVoltage()` 应在 O(1) 时间内完成，仅执行一次减法、一次绝对值计算和一次比较 |
| NFR-4.1.2 | `PL_CheckCellVoltage()` 和 `PL_CheckCellTemperature()` 应在 O(1) 时间内完成 |
| NFR-4.1.3 | `PL_CheckVoltageSpread()` 的时间复杂度为 O(S × M × C)，其中 S=电芯串数、M=模组数、C=电芯块数，当前默认值为 O(1×1×18)=O(1) |
| NFR-4.1.4 | `PL_CheckTemperatureSpread()` 的时间复杂度为 O(S × M × T)，其中 T=温度传感器数，当前默认值为 O(1×1×8)=O(1) |

### 4.2 内存

| 编号 | 描述 |
|------|------|
| NFR-4.2.1 | 模块不持有任何静态变量，所有状态通过传入的数据库指针操作，无额外静态内存占用 |
| NFR-4.2.2 | 函数均为纯计算逻辑，不进行动态内存分配 |

### 4.3 鲁棒性

| 编号 | 描述 |
|------|------|
| NFR-4.3.1 | 输出指针参数通过 `FAS_ASSERT` 进行运行时检查，空指针会触发断言机制 |
| NFR-4.3.2 | `PL_CheckCellVoltage()` 对输入参数 `baseCellVoltage` 和 `redundancy0CellVoltage` 不进行范围断言（接受全范围 `int16_t` 值），由调用方确保输入数据有效 |
| NFR-4.3.3 | `PL_CheckCellTemperature()` 同理接受全范围 `int16_t` 输入值 |
| NFR-4.3.4 | 分布检查函数在遍历过程中即使检测到异常也不会提前退出，而是完整遍历所有数据点以确保诊断信息的完整性 |

### 4.4 可配置性

| 编号 | 描述 |
|------|------|
| NFR-4.4.1 | 所有容差阈值均以宏形式定义在 `plausibility_cfg.h` 中，编译期可调整 |
| NFR-4.4.2 | 电芯串/模组/电芯块数量通过 `battery_system_cfg.h` 中的宏配置，分布检查的遍历范围自动适配 |

### 4.5 可测试性

| 编号 | 描述 |
|------|------|
| NFR-4.5.1 | 所有公开函数均为无状态纯函数（输入决定输出），便于单元测试 |
| NFR-4.5.2 | 通过 `UNITY_UNIT_TEST` 宏支持外部化静态函数用于单元测试 |

---

## 5. 接口需求

### 5.1 公共 API

| 函数 | 接口类型 | 说明 |
|------|---------|------|
| `PL_CheckStringVoltage()` | Public | 电池包总电压一致性校验 |
| `PL_CheckCellVoltage()` | Public | 单体电压冗余校验 |
| `PL_CheckCellTemperature()` | Public | 单体温度冗余校验 |
| `PL_CheckVoltageSpread()` | Public | 电压分布检查 |
| `PL_CheckTemperatureSpread()` | Public | 温度分布检查 |

### 5.2 依赖项

| 依赖模块 | 依赖内容 | 说明 |
|---------|---------|------|
| `plausibility_cfg.h` | 配置宏 | 容差阈值定义 |
| `battery_system_cfg.h` | 配置宏 | 电芯串/模组/电芯块数量、温度传感器数量 |
| `database.h` | 数据结构 | `DATA_BLOCK_CELL_VOLTAGE_s`, `DATA_BLOCK_CELL_TEMPERATURE_s`, `DATA_BLOCK_MIN_MAX_s` |
| `diag.h` | 诊断函数 | `DIAG_CheckEvent()`, `DIAG_ID_*` |
| `foxmath.h` | 数学工具 | 提供 `abs()` 整数绝对值函数 |
| `<stdint.h>` | 标准库 | 固定宽度整数类型 |

### 5.3 调用方

| 调用方模块 | 调用函数 | 场景 |
|-----------|---------|------|
| BMS 主控模块 | 全部五个公开函数 | 状态机运行循环中的测量验证阶段 |
| DIAG 回调模块 | `PL_CheckCellVoltage()`, `PL_CheckCellTemperature()` | 诊断事件的合理性校验回调 |

---

## 6. 数据结构

本模块不定义自有数据结构。操作的数据结构由数据库模块（`database.h`）定义：

| 数据结构 | 用途 | 涉及函数 |
|---------|------|---------|
| `DATA_BLOCK_CELL_VOLTAGE_s` | 单体电压数据块（含电压值、无效标记） | `PL_CheckVoltageSpread()` |
| `DATA_BLOCK_CELL_TEMPERATURE_s` | 单体温度数据块（含温度值、无效标记、有效计数） | `PL_CheckTemperatureSpread()` |
| `DATA_BLOCK_MIN_MAX_s` | 统计值数据块（含平均电压、平均温度） | `PL_CheckVoltageSpread()`, `PL_CheckTemperatureSpread()` |

---

## 7. 配置参数

| 配置宏 | 默认值 | 范围 | 单位 | 用于需求 |
|--------|--------|------|------|---------|
| `PL_STRING_VOLTAGE_TOLERANCE_mV` | 3000 | [0, 10000] | mV | REQ-001 |
| `PL_CELL_VOLTAGE_TOLERANCE_mV` | 10 | [0, 10000] | mV | REQ-002 |
| `PL_CELL_TEMPERATURE_TOLERANCE_dK` | 50 | [0, 100] | dK (0.1°C) | REQ-003 |
| `PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV` | 300 | [0, 10000] | mV | REQ-004 |
| `PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK` | 100 | [0, 100] | dK (0.1°C) | REQ-005 |

---

## 8. 需求追溯矩阵

| 需求编号 | 需求描述 | 关联函数/宏 | 所在文件 | 验证方法 |
|---------|---------|------------|---------|---------|
| REQ-001 | 电池包总电压一致性校验 | `PL_CheckStringVoltage()` | plausibility.c:77 | 单元测试：注入偏差值验证返回值 |
| REQ-002 | 单体电压冗余校验与平均值输出 | `PL_CheckCellVoltage()` | plausibility.c:89 | 单元测试：注入冗余值验证返回和平均值 |
| REQ-003 | 单体温度冗余校验与平均值输出 | `PL_CheckCellTemperature()` | plausibility.c:107 | 单元测试：注入冗余值验证返回和平均值 |
| REQ-004 | 电压分布检查与无效电压标记 | `PL_CheckVoltageSpread()` | plausibility.c:126 | 单元测试：构造超限电压验证无效标记 |
| REQ-005 | 温度分布检查与无效温度标记 | `PL_CheckTemperatureSpread()` | plausibility.c:158 | 单元测试：构造超限温度验证无效标记 |
| REQ-006 | 电压分布异常诊断上报 | `DIAG_CheckEvent()` 调用 | plausibility.c:153 | 集成测试：验证诊断事件 ID 和参数 |
| REQ-007 | 温度分布异常诊断上报 | `DIAG_CheckEvent()` 调用 | plausibility.c:188 | 集成测试：验证诊断事件 ID 和参数 |
| REQ-008 | 输出指针参数有效性断言 | `FAS_ASSERT(pCellVoltage/Temperature)` | plausibility.c:96,114 | 单元测试：传入 NULL 指针验证断言 |
| REQ-009 | 数据库指针参数有效性断言 | `FAS_ASSERT(pCellVoltages/Temperatures)` | plausibility.c:130-131,162-163 | 单元测试：传入 NULL 指针验证断言 |
| REQ-010 | 跳过无效电压数据 | `invalidCellVoltage` 检查 | plausibility.c:141 | 单元测试：预设无效标记验证跳过行为 |
| REQ-011 | 跳过无效温度并计数有效温度 | `invalidCellTemperature` + `nrValidTemperatures` | plausibility.c:173,183 | 单元测试：验证有效计数累加行为 |
| REQ-012 | 总电压容差可配置 | `PL_STRING_VOLTAGE_TOLERANCE_mV` | plausibility_cfg.h:69 | 代码审查：验证宏定义和默认值 |
| REQ-013 | 冗余电压容差可配置 | `PL_CELL_VOLTAGE_TOLERANCE_mV` | plausibility_cfg.h:77 | 代码审查：验证宏定义和默认值 |
| REQ-014 | 冗余温度容差可配置 | `PL_CELL_TEMPERATURE_TOLERANCE_dK` | plausibility_cfg.h:86 | 代码审查：验证宏定义和默认值 |
| REQ-015 | 电压分布容差可配置 | `PL_CELL_VOLTAGE_SPREAD_TOLERANCE_mV` | plausibility_cfg.h:95 | 代码审查：验证宏定义和默认值 |
| REQ-016 | 温度分布容差可配置 | `PL_CELL_TEMPERATURE_SPREAD_TOLERANCE_dK` | plausibility_cfg.h:104 | 代码审查：验证宏定义和默认值 |
