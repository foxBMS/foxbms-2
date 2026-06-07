# BAL_STRATEGY_HISTORY — 基于历史电荷差的均衡策略软件需求规格说明

**文件**: `bal_strategy_history.c`
**版本**: v1.11.0
**创建日期**: 2020-05-29
**最后更新**: 2026-04-20
**所属层级**: APPLICATION
**模块前缀**: BAL
**许可证**: BSD-3-Clause

---

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 电池管理系统（Battery Management System）中基于历史电荷差（History-Based Delta Charge）的电池均衡策略的软件需求规格。该策略通过累积电池单体间电压差所对应的电荷量（单位：mAs），将其作为均衡判据，决定哪些电池单体需要被均衡。

本模块是 BAL 均衡框架下的一个具体均衡策略实现，与基于电压的均衡策略（`bal_strategy_voltage.c`）和无均衡策略（`bal_strategy_none.c`）并列。

### 1.2 范围

#### 涵盖内容

- BAL 状态机触发函数 `BAL_Trigger()` 的顶层调度逻辑
- 均衡检查状态（`BAL_FSM_CHECK_BALANCING`）的子状态机处理
- 均衡执行状态（`BAL_FSM_BALANCE`）的子状态机处理
- 基于历史电荷差（Delta Charge, mAs）的不均衡计算逻辑
- 均衡激活与去激活的具体实现
- 均衡执行过程中的安全保护条件（电压下限、温度上限）
- 状态请求的外部接口（`BAL_SetStateRequest()`）
- 初始化状态查询接口（`BAL_GetInitializationState()`）

#### 不涵盖内容

- BAL 框架的通用状态机操作（`BAL_SaveLastStates()`、`BAL_CheckReEntrance()`、`BAL_TransferStateRequest()`、`BAL_CheckStateRequest()`）——属于 `bal.c` 的职责
- 均衡模块通用初始化（`BAL_Init()`）——属于 `bal.c` 的职责
- 未初始化/初始化/已初始化状态的处理（`BAL_ProcessStateUninitialized()` 等）——属于 `bal.c` 的职责
- 均衡阈值的管理（`BAL_SetBalancingThreshold()` / `BAL_GetBalancingThreshold_mV()`）——属于 `bal_cfg.c` 的职责
- 基于电压的均衡策略——属于 `bal_strategy_voltage.c` 的职责
- 硬件 AFE（Analog Front End）的具体操作

### 1.3 定义与缩略语

| 术语 | 英文全称 | 说明 |
|------|----------|------|
| BAL | Balancing | 电池均衡模块 |
| FSM | Finite State Machine | 有限状态机 |
| Delta Charge | Delta Charge | 电荷差累积量，单位为 mAs（毫安·秒） |
| DOD | Depth of Discharge | 放电深度 |
| SOC | State of Charge | 荷电状态 |
| BMS | Battery Management System | 电池管理系统 |
| AFE | Analog Front End | 模拟前端芯片 |
| ddegC | Deci-Degree Celsius | 0.1°C（温度单位） |

### 1.4 参考文献

| 编号 | 文档名称 | 说明 |
|------|----------|------|
| [1] | `bal.h` | BAL 模块头文件，定义数据结构与接口 |
| [2] | `bal.c` | BAL 模块核心实现 |
| [3] | `BAL_SOFTWARE_REQUIREMENTS.md` | BAL 模块通用软件需求规格说明 |
| [4] | `bal_cfg.h` | BAL 模块配置参数定义 |
| [5] | `battery_cell_cfg.h` | 电池电芯容量配置（BC_CAPACITY_mAh） |
| [6] | `bms-slave_cfg.h` | BMS 从板配置（均衡电阻等） |
| [7] | `database.h` | 数据库访问接口 |
| [8] | `state_estimation.h` | 状态估计算法接口（SE_GetStateOfChargeFromVoltage） |

---

## 2. 总体描述

### 2.1 产品视角

本模块是 BAL 均衡框架的**历史电荷差策略**实现。它被 BAL 框架通过以下三个 public 接口调用：

- `BAL_Trigger()` — 每 100ms 由 BMS 主控调用，驱动状态机
- `BAL_SetStateRequest()` — 接收外部状态请求
- `BAL_GetInitializationState()` — 供外部查询初始化状态

模块内部实现了均衡检查（`BAL_FSM_CHECK_BALANCING`）和均衡执行（`BAL_FSM_BALANCE`）两个运行状态的完整子状态机逻辑。

```
┌─────────────────────────────────────────────┐
│              BAL 框架 (bal.c)                │
│  ┌───────────────────────────────────────┐  │
│  │  BAL_Trigger() 顶层调度               │  │
│  │    ├─ UNINITIALIZED → bal.c           │  │
│  │    ├─ INITIALIZATION → bal.c          │  │
│  │    ├─ INITIALIZED → bal.c             │  │
│  │    ├─ CHECK_BALANCING → 本模块 ★      │  │
│  │    ├─ BALANCE → 本模块 ★              │  │
│  │    └─ 其他状态 → (未实现)              │  │
│  └───────────────────────────────────────┘  │
└─────────────────────────────────────────────┘
```

### 2.2 工作模式

本模块基于子状态机（Sub-FSM）运行，均衡检查状态（`BAL_FSM_CHECK_BALANCING`）和均衡执行状态（`BAL_FSM_BALANCE`）各自包含多个子状态：

**均衡检查状态（CHECK_BALANCING）子状态：**

| 子状态 | 说明 |
|--------|------|
| `BAL_ENTRY` | 入口 — 检查全局均衡是否被允许 |
| `BAL_CHECK_IMBALANCES` | 检查是否存在未消除的电荷差 |
| `BAL_COMPUTE_IMBALANCES` | 在 BMS 静置状态下计算各单体的电荷差 |

**均衡执行状态（BALANCE）子状态：**

| 子状态 | 说明 |
|--------|------|
| `BAL_ENTRY` | 入口 — 检查全局均衡是否被允许 |
| `BAL_ACTIVATE_BALANCING` | 执行均衡激活，扣除累积电荷并更新数据库 |

### 2.3 均衡策略原理

历史电荷差策略的核心思想是：

1. **累积电荷差**：当 BMS 处于静置状态（`BMS_AT_REST`）时，以各串中电压最低的电池单体为基准，计算其他单体的电压差
2. **转换为电荷量**：通过 SOC-电压映射关系（`SE_GetStateOfChargeFromVoltage()`），将电压差转换为放电深度（DOD）差，进而计算电荷差（单位：mAs）
3. **累积消耗**：在均衡激活期间，根据均衡电流和均衡时间，逐步扣除已均衡的电荷量，直至电荷差清零
4. **重新计算条件**：仅在 BMS 静置状态下重新计算电荷差

---

## 3. 功能需求

### 3.1 均衡检查子状态机

#### REQ-001 — 均衡检查入口子状态处理

- **编号**: REQ-001
- **优先级**: 高
- **对应函数**: `BAL_ProcessStateCheckBalancing()`（BAL_ENTRY 分支）
- **描述**: 系统应在均衡检查的入口子状态下，根据全局均衡允许标志决定后续走向：
  - 若全局均衡未被允许（`balancingGlobalAllowed == false`）：若有活跃均衡则先停用，保持在入口子状态
  - 若全局均衡被允许：切换到"检查不均衡"子状态

**处理流程**

1. 若 `bal_state.balancingGlobalAllowed == false`：
   - 若 `bal_state.active == true`，调用 `BAL_Deactivate()` 停用均衡
   - 保持 `substate = BAL_ENTRY`
2. 否则：
   - 设置 `substate = BAL_CHECK_IMBALANCES`
3. 设置定时器 `timer = BAL_FSM_SHORTTIME_100ms`（100ms）

**错误处理**

- 当全局均衡被禁用时，自动清理活跃均衡状态以确保安全

---

#### REQ-002 — 不均衡存在性检查

- **编号**: REQ-002
- **优先级**: 高
- **对应函数**: `BAL_ProcessStateCheckBalancing()`（BAL_CHECK_IMBALANCES 分支）
- **描述**: 系统应在"检查不均衡"子状态下，检查是否存在未消除的电荷差。若存在，直接进入均衡执行状态；若不存在，切换到"计算不均衡量"子状态等待 BMS 静置后重新计算。

**处理流程**

1. 若 `bal_state.active == true`，先调用 `BAL_Deactivate()` 停用均衡
2. 调用 `BAL_CheckImbalances()`：
   - 若返回 `true`（存在待均衡的电荷差）：设置 `state = BAL_FSM_BALANCE`，`substate = BAL_ENTRY`
   - 若返回 `false`：设置 `substate = BAL_COMPUTE_IMBALANCES`
3. 设置定时器 `timer = BAL_FSM_SHORTTIME_100ms`（100ms）

**前置条件**

- 存在有效的电荷差数据（已完成至少一次不均衡计算）

**错误处理**

- 无显式错误处理；若无电荷差则进入计算子状态重新评估

---

#### REQ-003 — BMS 静置状态下重新计算不均衡量

- **编号**: REQ-003
- **优先级**: 高
- **对应函数**: `BAL_ProcessStateCheckBalancing()`（BAL_COMPUTE_IMBALANCES 分支）
- **描述**: 系统应在"计算不均衡量"子状态下，检查 BMS 是否处于静置状态（`BMS_AT_REST`）。仅在静置状态下执行不均衡量计算，计算完成后进入均衡执行状态。

**处理流程**

1. 检查 `BMS_GetBatterySystemState() == BMS_AT_REST`：
   - 若是：调用 `BAL_ComputeImbalances()` 计算不均衡量，设置 `state = BAL_FSM_BALANCE`，`substate = BAL_ENTRY`
   - 若否：设置 `substate = BAL_CHECK_IMBALANCES`（回到上一子状态等待）
2. 设置定时器 `timer = BAL_FSM_SHORTTIME_100ms`（100ms）

**前置条件**

- BMS 必须处于静置状态才能执行不均衡计算

**后置条件**

- 若 BMS 静置：数据库中的 `deltaCharge_mAs` 已更新为新的电荷差值
- 若 BMS 非静置：电路状态未改变，等待下次触发

---

### 3.2 均衡执行子状态机

#### REQ-004 — 均衡执行入口子状态处理

- **编号**: REQ-004
- **优先级**: 高
- **对应函数**: `BAL_ProcessStateBalancing()`（BAL_ENTRY 分支）
- **描述**: 系统应在均衡执行的入口子状态下，根据全局均衡允许标志决定后续走向：
  - 若全局均衡未被允许：停用均衡，退出均衡执行状态，回到均衡检查状态
  - 若全局均衡被允许：切换到"激活均衡"子状态

**处理流程**

1. 若 `bal_state.balancingGlobalAllowed == false`：
   - 若 `bal_state.active == true`，调用 `BAL_Deactivate()` 停用均衡
   - 设置 `bal_state.active = false`
   - 设置 `substate = (BAL_FSM_SUB_e)BAL_FSM_CHECK_BALANCING`
2. 否则：
   - 设置 `substate = BAL_ACTIVATE_BALANCING`
3. 设置定时器 `timer = BAL_FSM_SHORTTIME_100ms`（100ms）

**后置条件**

- 若全局均衡被禁用：状态机已切换到均衡检查状态，均衡已停用

---

#### REQ-005 — 均衡激活与安全条件检查

- **编号**: REQ-005
- **优先级**: 高
- **对应函数**: `BAL_ProcessStateBalancing()`（BAL_ACTIVATE_BALANCING 分支）
- **描述**: 系统应在"激活均衡"子状态下，对每条电池串检查以下安全条件，**全部通过**后方可激活均衡：
  - 最低单体电压不低于下限阈值（`BAL_LOWER_VOLTAGE_LIMIT_mV`）
  - 最高温度不超过上限阈值（`BAL_UPPER_TEMPERATURE_LIMIT_ddegC`）
  - 存在未消除的电荷差（`BAL_CheckImbalances() == true`）
  - 全局均衡允许标志为 `true`

**处理流程**

1. 从数据库读取最小/最大值数据块（`DATA_BLOCK_ID_MIN_MAX`）
2. 设置定时器为 `BAL_FSM_BALANCING_TIME_100ms`（均衡执行时间，1s）
3. 遍历每条电池串（`s = 0 ~ BS_NR_OF_STRINGS`）：
   - 若 `minimumCellVoltage_mV[s] <= BAL_LOWER_VOLTAGE_LIMIT_mV`（电压过低），停用均衡并退出
   - 若 `maximumTemperature_ddegC[s] >= BAL_UPPER_TEMPERATURE_LIMIT_ddegC`（温度过高），停用均衡并退出
   - 若 `BAL_CheckImbalances() == false`（无不均衡），停用均衡并退出
   - 若 `bal_state.balancingGlobalAllowed == false`（全局禁用），停用均衡并退出
4. 若所有串全部通过上述检查，调用 `BAL_ActivateBalancing()` 执行一次均衡周期

**前置条件**

- 数据库中存在有效的最小/最大值数据（`DATA_BLOCK_ID_MIN_MAX`）
- 均衡定时器设置为 1s，每个均衡周期持续 1s

**后置条件**

- 若所有安全检查通过：`BAL_ActivateBalancing()` 已被调用，均衡电流已计算并扣除了对应电荷量
- 若任一安全检查失败：均衡已停用，状态机已回到均衡检查状态

**错误处理**

| 条件 | 触发动作 |
|------|----------|
| 任意串电压低于下限 | 停用均衡 → 回到 CHECK_BALANCING |
| 任意串温度高于上限 | 停用均衡 → 回到 CHECK_BALANCING |
| 不存在不均衡 | 停用均衡 → 回到 CHECK_BALANCING |
| 全局均衡被禁用 | 停用均衡 → 回到 CHECK_BALANCING |

---

### 3.3 均衡激活实现

#### REQ-006 — 基于电荷差的均衡激活控制

- **编号**: REQ-006
- **优先级**: 高
- **对应函数**: `BAL_ActivateBalancing()`
- **描述**: 系统应在均衡激活函数中，遍历所有电池串/模组/电芯块，对每个电芯块根据以下条件决定是否激活均衡：
  - 若局部 `balancingAllowed == false`：无条件停用该电芯块的均衡
  - 若该电芯块的 `deltaCharge_mAs > 0`：激活均衡，计算本周期消耗的电荷量并扣除
  - 若该电芯块的 `deltaCharge_mAs == 0`：停用该电芯块的均衡

**输入**

| 参数 | 类型 | 来源 | 描述 |
|------|------|------|------|
| `bal_balancing` | `DATA_BLOCK_BALANCING_CONTROL_s` | 数据库 | 均衡控制数据块 |
| `bal_cellVoltage` | `DATA_BLOCK_CELL_VOLTAGE_s` | 数据库 | 单体电压数据块 |

**处理流程**

1. 从数据库读取 `bal_balancing` 和 `bal_cellVoltage` 数据块
2. 遍历每条电池串（`s`）：
   1. 初始化该串已均衡电芯计数 `nrBalancedCells = 0`
   2. 遍历每个模组（`m`）：
      1. 遍历每个电芯块（`cb`）：
         - 若 `bal_state.balancingAllowed == false`：设置 `activateBalancing[s][m][cb] = false`
         - 否则若 `deltaCharge_mAs[s][m][cb] > 0`：
           - 设置 `activateBalancing[s][m][cb] = true`
           - `nrBalancedCells++`
           - 计算均衡电流：`cellBalancingCurrent = cellVoltage_mV / SLV_BALANCING_RESISTANCE_ohm`
           - 计算本周期扣除电荷量：`difference = (BAL_FSM_BALANCING_TIME_100ms / 10u) * cellBalancingCurrent`
           - 设置 `bal_state.active = true`、`enableBalancing = true`
           - 若 `difference > deltaCharge_mAs`：设置 `deltaCharge_mAs = 0`（防止下溢）
           - 否则：`deltaCharge_mAs -= difference`（扣除本周期消耗）
         - 否则：设置 `activateBalancing[s][m][cb] = false`
   3. 记录该串的已均衡电芯数：`nrBalancedCells[s] = nrBalancedCells`
3. 将更新的 `bal_balancing` 写回数据库

**后置条件**

- 数据库中的 `deltaCharge_mAs` 已按本周期均衡量更新
- `activateBalancing` 标志反映各电芯块的均衡激活状态
- `enableBalancing` 和 `active` 标志反映全局均衡激活状态

**错误处理**

- 使用无符号整数运算，`deltaCharge_mAs -= difference` 前检查是否会发生下溢（`difference > deltaCharge_mAs`），若会则直接将 `deltaCharge_mAs` 置零

---

### 3.4 均衡停用

#### REQ-007 — 均衡全局停用

- **编号**: REQ-007
- **优先级**: 高
- **对应函数**: `BAL_Deactivate()`
- **描述**: 系统应提供均衡全局停用功能，遍历所有电池串/模组/电芯块，将所有电芯块的均衡激活标志置为 `false`，将所有电荷差值清零，并将全局均衡使能标志和活跃标志置为 `false`。

**处理流程**

1. 遍历每条电池串（`s`）：
   1. 遍历每个模组（`m`）：
      1. 遍历每个电芯块（`cb`）：
         - 设置 `activateBalancing[s][m][cb] = false`
         - 设置 `deltaCharge_mAs[s][m][cb] = 0u`
   2. 设置 `nrBalancedCells[s] = 0u`
2. 设置 `enableBalancing = false`
3. 设置 `bal_state.active = false`
4. 将更新的 `bal_balancing` 写回数据库

**后置条件**

- 所有电芯块的均衡已全部停用
- 所有电荷差数据已清零
- 均衡使能标志和活跃标志已清除

**错误处理**

- 无显式错误处理；此函数为无条件全部停用

---

### 3.5 不均衡检测与计算

#### REQ-008 — 不均衡存在性检测

- **编号**: REQ-008
- **优先级**: 高
- **对应函数**: `BAL_CheckImbalances()`
- **描述**: 系统应提供不均衡存在性检测函数，遍历所有电池串/模组/电芯块，若任意电芯块的 `deltaCharge_mAs` 大于 0，则返回 `true`，否则返回 `false`。

**处理流程**

1. 初始化 `returnValue = false`
2. 遍历每条电池串（`s`）：
   1. 遍历每个模组（`m`）：
      1. 遍历每个电芯块（`cb`）：
         - 若 `bal_balancing.deltaCharge_mAs[s][m][cb] > 0`：设置 `returnValue = true`
3. 返回 `returnValue`

**返回值**

| 返回值 | 含义 |
|--------|------|
| `true` | 存在至少一个电芯块有未消除的电荷差 |
| `false` | 所有电芯块的电荷差均已清零 |

---

#### REQ-009 — 基于电压差和 SOC 映射的不均衡量计算

- **编号**: REQ-009
- **优先级**: 高
- **对应函数**: `BAL_ComputeImbalances()`
- **描述**: 系统应在 BMS 静置状态下，对每条电池串执行以下不均衡量计算流程：
  1. 找到该串中电压最低的电芯块（作为基准）
  2. 通过 SOC-电压映射函数获取基准电芯块的 SOC，计算最大放电深度 `maxDOD`
  3. 更新有效均衡阈值（回读最新的配置阈值并加上迟滞）
  4. 对该串中其他电芯块，若其电压高于基准电压加均衡阈值之和，则计算其与基准之间的电荷差（`deltaCharge_mAs = maxDOD - DOD`）

**输入**

| 参数 | 类型 | 来源 | 描述 |
|------|------|------|------|
| `bal_cellVoltage.cellVoltage_mV` | `uint16_t[][]` | 数据库 | 所有电芯块的电压值 |
| `BC_CAPACITY_mAh` | 宏 | `battery_cell_cfg.h` | 电芯额定容量（mAh） |

**处理流程**

1. 从数据库读取 `bal_balancing` 和 `bal_cellVoltage` 数据块
2. 遍历每条电池串（`s`）：
   1. 初始化基准电压 `voltageMin_mV = INT16_MAX`（32767）
   2. 遍历所有电芯块，找到电压最低的电芯块：
      - 记录其电压值 `voltageMin_mV`
      - 记录其模组索引 `minVoltageModuleIndex`
      - 记录其电芯块索引 `minVoltageModuleCellBlockIndex`
   3. 通过基准电芯块电压计算 SOC：
      - `SOC = SE_GetStateOfChargeFromVoltage(voltageMin_mV / 1000.0f)`
   4. 计算最大放电深度：
      - `maxDOD = BC_CAPACITY_mAh * (1.0f - SOC) * 3600.0f`
   5. 将基准电芯块的 `deltaCharge_mAs` 置为零（基准不均衡量为零）
   6. 更新有效均衡阈值：
      - `bal_state.balancingThreshold = BAL_GetBalancingThreshold_mV() + BAL_HYSTERESIS_mV`
   7. 遍历该串中其他电芯块（非基准电芯块）：
      - 若 `cellVoltage_mV >= voltageMin_mV + balancingThreshold`：
        - `SOC = SE_GetStateOfChargeFromVoltage(cellVoltage_mV / 1000.0f)`
        - `DOD = BC_CAPACITY_mAh * (1.0f - SOC) * 3600.0f`
        - `deltaCharge_mAs = maxDOD - DOD`（基准与当前电芯块的电荷差）
3. 将更新的 `bal_balancing` 写回数据库

**前置条件**

- BMS 处于静置状态（由调用方 `BAL_ProcessStateCheckBalancing()` 在 `BAL_COMPUTE_IMBALANCES` 子状态中保证）
- 数据库中存在有效的单体电压数据

**后置条件**

- 数据库中的 `deltaCharge_mAs` 已按计算结果更新
- 基准电芯块的 `deltaCharge_mAs` 为零
- `bal_state.balancingThreshold` 已更新为最新配置值加迟滞

**错误处理**

- 无显式错误处理；若 SOC 计算失败，`SE_GetStateOfChargeFromVoltage()` 函数的内部处理将生效

---

### 3.6 状态机顶层调度

#### REQ-010 — BAL 状态机顶层触发

- **编号**: REQ-010
- **优先级**: 高
- **对应函数**: `BAL_Trigger()`
- **描述**: 系统应在 `BAL_Trigger()` 函数中执行 BAL 状态机的完整事件序列，每 100ms 被调用一次。函数应执行以下步骤：
  1. 重入检查（`BAL_CheckReEntrance()`）——检测到重入则放弃本次执行
  2. 定时器管理——若定时器未到期则递减并返回（跳过状态机处理）
  3. 状态分发——根据当前状态调用对应的处理函数

**输入**

| 参数 | 类型 | 描述 |
|------|------|------|
| 无 | void | 所有状态信息通过模块内静态变量 `bal_state` 维护 |

**处理流程**

1. 调用 `BAL_CheckReEntrance(&bal_state)` 进行重入检查
   - 若返回值 `> 0`（0xFF），立即返回（放弃本次执行）
2. 定时器递减：
   - 若 `bal_state.timer > 0`：递减定时器
     - 若递减后仍 `> 0`：递减 `triggerEntry`（退出重入保护）并返回
3. 状态分发（`switch(bal_state.state)`）：
   | 当前状态 | 调用的处理函数 | 实现位置 |
   |----------|---------------|----------|
   | `BAL_FSM_UNINITIALIZED` | `BAL_ProcessStateUninitialized()` | `bal.c` |
   | `BAL_FSM_INITIALIZATION` | `BAL_Init()` + `BAL_ProcessStateInitialization()` | `bal.c` |
   | `BAL_FSM_INITIALIZED` | `BAL_ProcessStateInitialized()` | `bal.c` |
   | `BAL_FSM_CHECK_BALANCING` | `BAL_ProcessStateCheckBalancing()` | **本模块** |
   | `BAL_FSM_BALANCE` | `BAL_ProcessStateBalancing()` | **本模块** |
   | `default` | `FAS_ASSERT(FAS_TRAP)` | 非法状态陷阱 |
4. 递减 `triggerEntry`（退出重入保护）

**前置条件**

- 状态机已完成初始化，`bal_state.state` 为有效状态值

**后置条件**

- 根据当前状态执行了对应的子状态机处理逻辑
- `triggerEntry` 已递减，允许下次重入

**错误处理**

- 若检测到重入（`BAL_CheckReEntrance() > 0`）：放弃本次执行
- 若状态机处于非法状态（`default` 分支）：触发 `FAS_ASSERT(FAS_TRAP)` 致命断言

---

### 3.7 状态请求接口

#### REQ-011 — 状态请求设置

- **编号**: REQ-011
- **优先级**: 高
- **对应函数**: `BAL_SetStateRequest()`
- **描述**: 系统应向外部模块提供状态请求设置接口，在任务临界区内原子地校验请求合法性并写入状态请求。

**输入**

| 参数 | 类型 | 描述 |
|------|------|------|
| `stateRequest` | `BAL_STATE_REQUEST_e` | 期望设置的状态请求 |

**处理流程**

1. 初始化 `returnValue = BAL_OK`
2. 进入任务临界区（`OS_EnterTaskCritical()`）
3. 调用 `BAL_CheckStateRequest(&bal_state, stateRequest)` 校验请求合法性
4. 若返回 `BAL_OK`：将 `bal_state.stateRequest = stateRequest`
5. 退出任务临界区（`OS_ExitTaskCritical()`）
6. 返回校验结果

**返回值**

| 返回值 | 含义 |
|--------|------|
| `BAL_OK` | 请求合法，已写入 |
| `BAL_REQUEST_PENDING` | 存在待处理请求 |
| `BAL_ALREADY_INITIALIZED` | 状态机已初始化 |
| `BAL_ILLEGAL_REQUEST` | 非法请求 |

**错误处理**

- 若请求非法，不修改 `stateRequest`，返回对应的错误码给调用方处理

---

#### REQ-012 — 初始化状态查询

- **编号**: REQ-012
- **优先级**: 中
- **对应函数**: `BAL_GetInitializationState()`
- **描述**: 系统应向外部模块提供初始化状态查询接口，返回当前状态机的初始化完成标志。

**处理流程**

1. 返回 `bal_state.initializationFinished`

**返回值**

| 返回值 | 含义 |
|--------|------|
| `STD_OK` | 均衡模块已完成初始化 |
| `STD_NOT_OK` | 均衡模块尚未完成初始化 |

---

### 3.8 状态机状态转移图（历史策略视角）

```
BAL_Trigger() 每 100ms 触发

┌─────────────────────────────────────┐
│         BAL_FSM_CHECK_BALANCING     │
│  ┌──────────────────────────────┐   │
│  │ BAL_ENTRY                    │   │
│  │ ───balancingGlobalAllowed?──▶│   │
│  │   false → stay, deactivate   │   │
│  │   true  → CHECK_IMBALANCES   │   │
│  ├──────────────────────────────┤   │
│  │ BAL_CHECK_IMBALANCES         │   │
│  │ ───CheckImbalances()?────────▶   │
│  │   true  → BAL_FSM_BALANCE    │   │
│  │   false → COMPUTE_IMBALANCES │   │
│  ├──────────────────────────────┤   │
│  │ BAL_COMPUTE_IMBALANCES       │   │
│  │ ───BMS_AT_REST?─────────────▶   │
│  │   true  → Compute + BALANCE  │   │
│  │   false → CHECK_IMBALANCES   │   │
│  └──────────────────────────────┘   │
└─────────────────────────────────────┘
              │
              ▼
┌─────────────────────────────────────┐
│         BAL_FSM_BALANCE             │
│  ┌──────────────────────────────┐   │
│  │ BAL_ENTRY                    │   │
│  │ ───balancingGlobalAllowed?──▶│   │
│  │   false → CHECK_BALANCING    │   │
│  │   true  → ACTIVATE_BALANCING │   │
│  ├──────────────────────────────┤   │
│  │ BAL_ACTIVATE_BALANCING       │   │
│  │ ───安全条件检查?────────────▶│   │
│  │   全部通过 → ActivateBal..() │   │
│  │   任一失败 → CHECK_BALANCING │   │
│  └──────────────────────────────┘   │
└─────────────────────────────────────┘
```

---

## 4. 非功能需求

### 4.1 时序与性能

#### REQ-013 — 状态机触发周期

- **编号**: REQ-013
- **描述**: BAL 状态机应以 **100ms** 为周期被 `BAL_Trigger()` 函数定时触发。
- **依据**: 所有子状态处理将 `timer` 设置为 `BAL_FSM_SHORTTIME_100ms (1u)`，即 1 × 100ms

#### REQ-014 — 均衡执行周期

- **编号**: REQ-014
- **描述**: 均衡激活周期（电荷扣除周期）应为 **1 秒**（`BAL_FSM_BALANCING_TIME_100ms = 10u`，即 10 × 100ms）。
- **依据**: `BAL_ProcessStateBalancing()` 中 `timer = BAL_FSM_BALANCING_TIME_100ms`

#### REQ-015 — 电荷量计算精度

- **编号**: REQ-015
- **描述**: 每周期扣除的电荷量应基于实际均衡电流计算（`cellVoltage / SLV_BALANCING_RESISTANCE_ohm`），并转换为每 100ms 对应的 mAs 值。计算公式为 `difference = (BAL_FSM_BALANCING_TIME_100ms / 10u) * cellBalancingCurrent`，其中 `BAL_FSM_BALANCING_TIME_100ms / 10u = 1`（即每 100ms 扣除一次电流对应的电荷量）。
- **依据**: `BAL_ActivateBalancing()` 中 `difference = (BAL_FSM_BALANCING_TIME_100ms / 10u) * (uint32_t)(cellBalancingCurrent)`

### 4.2 内存

#### REQ-016 — 模块本地数据存储

- **编号**: REQ-016
- **描述**: 模块应使用静态局部变量存储数据库表的本地副本（`bal_balancing`、`bal_cellVoltage`）和状态机状态（`bal_state`），避免频繁的堆内存分配。
- **依据**: 三个静态变量均为模块级静态存储

### 4.3 鲁棒性

#### REQ-017 — 无符号整数下溢保护

- **编号**: REQ-017
- **描述**: 在扣除电荷量时，系统应先判断 `difference > deltaCharge_mAs`，若扣除量大于剩余量则直接清零，防止无符号整数下溢。
- **依据**: `BAL_ActivateBalancing()` 中 `if (difference > bal_balancing.deltaCharge_mAs[s][m][cb])` 分支

#### REQ-018 — 均衡安全保护条件

- **编号**: REQ-018
- **描述**: 均衡激活前必须通过以下安全条件检查，任一条件不满足即停用均衡：
  - 最低单体电压 > `BAL_LOWER_VOLTAGE_LIMIT_mV`（2V）
  - 最高温度 < `BAL_UPPER_TEMPERATURE_LIMIT_ddegC`（70°C）
  - 存在实际不均衡（`BAL_CheckImbalances() == true`）
  - 全局均衡允许标志为 `true`
- **依据**: `BAL_ProcessStateBalancing()` 中 `BAL_ACTIVATE_BALANCING` 子状态的安全检查逻辑

#### REQ-019 — 临界区保护

- **编号**: REQ-019
- **描述**: 状态请求的写入操作必须在任务临界区内执行，确保并发安全。
- **依据**: `BAL_SetStateRequest()` 中使用 `OS_EnterTaskCritical()` / `OS_ExitTaskCritical()` 包围校验与写入操作

### 4.4 可配置性

#### REQ-020 — 可配置参数依赖

- **编号**: REQ-020
- **描述**: 模块应依赖以下可配置参数（在 `bal_cfg.h` 中集中定义）：

| 宏定义 | 默认值 | 单位 | 说明 |
|--------|--------|------|------|
| `BAL_FSM_SHORTTIME_100ms` | `1u` | ×100ms | 短延时（100ms） |
| `BAL_FSM_BALANCING_TIME_100ms` | `10u` | ×100ms | 均衡执行时间（1s） |
| `BAL_LOWER_VOLTAGE_LIMIT_mV` | `2000` | mV | 均衡电压下限 |
| `BAL_UPPER_TEMPERATURE_LIMIT_ddegC` | `700` | 0.1°C | 均衡温度上限（70°C） |
| `BAL_DEFAULT_THRESHOLD_mV` | `200` | mV | 默认均衡阈值 |
| `BAL_HYSTERESIS_mV` | `200` | mV | 均衡迟滞值 |
| `BC_CAPACITY_mAh` | 电芯相关 | mAh | 电芯额定容量 |
| `SLV_BALANCING_RESISTANCE_ohm` | 硬件相关 | Ω | 均衡电阻值 |

### 4.5 可测试性

#### REQ-021 — 单元测试支持

- **编号**: REQ-021
- **描述**: 模块应通过条件编译（`#ifdef UNITY_UNIT_TEST`）暴露内部函数和静态变量，支持单元测试框架直接访问。
- **依据**: `bal_strategy_history.c` 末尾的 `UNITY_UNIT_TEST` 区块暴露了：
  - `BAL_GetState()` — 获取当前状态
  - `TEST_BAL_GetBalancingControl()` — 获取均衡控制数据块
  - `TEST_BAL_GetBalancingState()` — 获取均衡状态结构体

---

## 5. 接口需求

### 5.1 公共 API

| 函数 | 返回类型 | 描述 | 需求编号 |
|------|----------|------|----------|
| `BAL_Trigger()` | `void` | 状态机顶层触发函数 | REQ-010 |
| `BAL_SetStateRequest()` | `BAL_RETURN_TYPE_e` | 设置状态机请求 | REQ-011 |
| `BAL_GetInitializationState()` | `STD_RETURN_TYPE_e` | 获取初始化状态 | REQ-012 |

### 5.2 依赖项

| 依赖模块 | 头文件 | 使用功能 |
|----------|--------|----------|
| BAL 框架 | `bal.h` | `BAL_SaveLastStates()`、`BAL_CheckReEntrance()`、`BAL_TransferStateRequest()`、`BAL_CheckStateRequest()`、`BAL_Init()`、`BAL_ProcessStateUninitialized()`、`BAL_ProcessStateInitialization()`、`BAL_ProcessStateInitialized()` |
| BAL 配置 | `bal_cfg.h` | 时间常量、阈值宏 |
| BMS 主控 | `bms.h` | `BMS_GetBatterySystemState()` — 获取 BMS 当前状态 |
| 数据库 | `database.h` | `DATA_READ_DATA()`、`DATA_WRITE_DATA()`、`DATA_BLOCK_BALANCING_CONTROL_s`、`DATA_BLOCK_CELL_VOLTAGE_s`、`DATA_BLOCK_MIN_MAX_s` |
| 操作系统 | `os.h` | `OS_EnterTaskCritical()`、`OS_ExitTaskCritical()` |
| 状态估算 | `state_estimation.h` | `SE_GetStateOfChargeFromVoltage()` — 电压→SOC 映射 |
| 电芯配置 | `battery_cell_cfg.h` | `BC_CAPACITY_mAh` — 电芯额定容量 |
| BMS 从板配置 | `bms-slave_cfg.h` | `SLV_BALANCING_RESISTANCE_ohm`、`BS_NR_OF_STRINGS`、`BS_NR_OF_MODULES_PER_STRING`、`BS_NR_OF_CELL_BLOCKS_PER_MODULE` |
| 标准库 | `<math.h>`、`<stdbool.h>`、`<stdint.h>` | 浮点运算、布尔类型、整数类型 |

### 5.3 调用方

| 调用模块 | 调用接口 | 场景 |
|----------|----------|------|
| `bms.c` | `BAL_Trigger()` | 以 100ms 周期触发状态机 |
| `bms.c` | `BAL_SetStateRequest()` | 发起均衡相关状态请求 |
| `bms.c` | `BAL_GetInitializationState()` | 查询均衡初始化状态 |

---

## 6. 数据结构

### 6.1 本地静态变量

| 变量名 | 类型 | 描述 | 需求编号 |
|--------|------|------|----------|
| `bal_balancing` | `DATA_BLOCK_BALANCING_CONTROL_s` | 均衡控制数据块本地副本 | REQ-016 |
| `bal_cellVoltage` | `DATA_BLOCK_CELL_VOLTAGE_s` | 单体电压数据块本地副本 | REQ-016 |
| `bal_state` | `BAL_STATE_s` | 均衡状态机状态结构体 | REQ-016 |

#### bal_state 初始值

| 字段 | 初始值 | 说明 |
|------|--------|------|
| `timer` | `0` | 定时器初始为零 |
| `stateRequest` | `BAL_STATE_NO_REQUEST` | 无待处理请求 |
| `state` | `BAL_FSM_UNINITIALIZED` | 未初始化状态 |
| `substate` | `BAL_ENTRY` | 入口子状态 |
| `lastState` | `BAL_FSM_UNINITIALIZED` | 上一状态初始值 |
| `lastSubstate` | `0` | 上一子状态初始值 |
| `triggerEntry` | `0` | 重入保护计数初始为零 |
| `errorRequestCounter` | `0` | 错误计数初始为零 |
| `initializationFinished` | `STD_NOT_OK` | 初始化尚未完成 |
| `active` | `false` | 均衡未激活 |
| `balancingThreshold` | `BAL_DEFAULT_THRESHOLD_mV + BAL_HYSTERESIS_mV`（400mV） | 有效均衡阈值 |
| `balancingAllowed` | `true` | 局部允许均衡 |
| `balancingGlobalAllowed` | `false` | 全局禁止均衡 |

### 6.2 数据库表依赖

| 数据库表 ID | 类型 | 读写模式 | 使用函数 |
|-------------|------|----------|----------|
| `DATA_BLOCK_ID_BALANCING_CONTROL` | `DATA_BLOCK_BALANCING_CONTROL_s` | 读写 | `BAL_ActivateBalancing()`、`BAL_Deactivate()`、`BAL_ComputeImbalances()` |
| `DATA_BLOCK_ID_CELL_VOLTAGE` | `DATA_BLOCK_CELL_VOLTAGE_s` | 只读 | `BAL_ActivateBalancing()`、`BAL_ComputeImbalances()` |
| `DATA_BLOCK_ID_MIN_MAX` | `DATA_BLOCK_MIN_MAX_s` | 只读 | `BAL_ProcessStateBalancing()` |

---

## 7. 控制流

### 7.1 主状态机触发流程

```
BAL_Trigger()                                    [每 100ms]
  ├─ BAL_CheckReEntrance(&bal_state)             → 若重入则返回
  ├─ if timer > 0 → timer-- → 若仍 > 0 返回       [定时器管理]
  ├─ switch(bal_state.state):
  │   ├─ UNINITIALIZED:
  │   │     BAL_SaveLastStates()
  │   │     BAL_TransferStateRequest()
  │   │     BAL_ProcessStateUninitialized()       [bal.c]
  │   ├─ INITIALIZATION:
  │   │     BAL_SaveLastStates()
  │   │     BAL_Init(&bal_balancing)              [bal.c]
  │   │     BAL_ProcessStateInitialization()      [bal.c]
  │   ├─ INITIALIZED:
  │   │     BAL_SaveLastStates()
  │   │     BAL_ProcessStateInitialized()         [bal.c]
  │   ├─ CHECK_BALANCING:
  │   │     BAL_SaveLastStates()
  │   │     BAL_ProcessStateCheckBalancing()      ★ 本模块
  │   ├─ BALANCE:
  │   │     BAL_SaveLastStates()
  │   │     BAL_ProcessStateBalancing()           ★ 本模块
  │   └─ default → FAS_ASSERT(FAS_TRAP)
  └─ triggerEntry--
```

### 7.2 均衡检查子状态机流程

```
BAL_ProcessStateCheckBalancing()
  ├─ [BAL_ENTRY]
  │     ├─ globalAllowed==false → deactivate, stay ENTRY
  │     └─ globalAllowed==true  → goto CHECK_IMBALANCES
  │     timer = 100ms
  │
  ├─ [BAL_CHECK_IMBALANCES]
  │     ├─ if active → BAL_Deactivate()
  │     ├─ BAL_CheckImbalances()
  │     │     ├─ true  → state=BALANCE, substate=ENTRY
  │     │     └─ false → substate=COMPUTE_IMBALANCES
  │     timer = 100ms
  │
  └─ [BAL_COMPUTE_IMBALANCES]
        ├─ BMS_GetBatterySystemState() == BMS_AT_REST?
        │     ├─ yes → BAL_ComputeImbalances(), state=BALANCE
        │     └─ no  → substate=CHECK_IMBALANCES
        timer = 100ms
```

### 7.3 均衡执行子状态机流程

```
BAL_ProcessStateBalancing()
  ├─ [BAL_ENTRY]
  │     ├─ globalAllowed==false
  │     │     → deactivate, state=CHECK_BALANCING
  │     └─ globalAllowed==true
  │           → substate=ACTIVATE_BALANCING
  │     timer = 100ms
  │
  └─ [BAL_ACTIVATE_BALANCING]
        ├─ READ DATA_BLOCK_MIN_MAX
        │     timer = BAL_FSM_BALANCING_TIME_100ms (1s)
        │
        ├─ for each string s:
        │     ├─ minVoltage[s] <= BAL_LOWER_VOLTAGE_LIMIT_mV?
        │     ├─ maxTemp[s]    >= BAL_UPPER_TEMPERATURE_LIMIT_ddegC?
        │     ├─ BAL_CheckImbalances() == false?
        │     └─ globalAllowed == false?
        │
        ├─ any condition FAILED:
        │     → deactivate, state=CHECK_BALANCING
        │
        └─ all conditions PASSED:
              → BAL_ActivateBalancing()
```

### 7.4 不均衡计算流程

```
BAL_ComputeImbalances()
  ├─ READ bal_balancing, bal_cellVoltage
  │
  ├─ for each string s:
  │     1. FIND cell with min voltage → (minModule, minCb)
  │     2. SOC_min = SE_GetStateOfChargeFromVoltage(V_min / 1000)
  │     3. maxDOD = BC_CAPACITY_mAh * (1-SOC_min) * 3600
  │     4. deltaCharge[minCell] = 0
  │     5. balancingThreshold = BAL_GetBalancingThreshold_mV() + HYSTERESIS_mV
  │     6. for each other cell:
  │           if V_cell >= V_min + balancingThreshold:
  │             SOC_cell = SE_GetStateOfChargeFromVoltage(V_cell / 1000)
  │             DOD_cell = BC_CAPACITY_mAh * (1-SOC_cell) * 3600
  │             deltaCharge[cell] = maxDOD - DOD_cell
  │
  └─ WRITE bal_balancing
```

---

## 8. 追溯矩阵

### 8.1 需求 → 代码位置映射

| 需求编号 | 需求简述 | 实现文件 | 函数 / 定义 |
|----------|----------|----------|-------------|
| REQ-001 | 均衡检查入口子状态处理 | `bal_strategy_history.c` | `BAL_ProcessStateCheckBalancing()` — BAL_ENTRY 分支 |
| REQ-002 | 不均衡存在性检查 | `bal_strategy_history.c` | `BAL_ProcessStateCheckBalancing()` — BAL_CHECK_IMBALANCES 分支 |
| REQ-003 | BMS 静置状态下重新计算不均衡量 | `bal_strategy_history.c` | `BAL_ProcessStateCheckBalancing()` — BAL_COMPUTE_IMBALANCES 分支 |
| REQ-004 | 均衡执行入口子状态处理 | `bal_strategy_history.c` | `BAL_ProcessStateBalancing()` — BAL_ENTRY 分支 |
| REQ-005 | 均衡激活与安全条件检查 | `bal_strategy_history.c` | `BAL_ProcessStateBalancing()` — BAL_ACTIVATE_BALANCING 分支 |
| REQ-006 | 基于电荷差的均衡激活控制 | `bal_strategy_history.c` | `BAL_ActivateBalancing()` |
| REQ-007 | 均衡全局停用 | `bal_strategy_history.c` | `BAL_Deactivate()` |
| REQ-008 | 不均衡存在性检测 | `bal_strategy_history.c` | `BAL_CheckImbalances()` |
| REQ-009 | 基于电压差和 SOC 映射的不均衡量计算 | `bal_strategy_history.c` | `BAL_ComputeImbalances()` |
| REQ-010 | BAL 状态机顶层触发 | `bal_strategy_history.c` | `BAL_Trigger()` |
| REQ-011 | 状态请求设置 | `bal_strategy_history.c` | `BAL_SetStateRequest()` |
| REQ-012 | 初始化状态查询 | `bal_strategy_history.c` | `BAL_GetInitializationState()` |
| REQ-013 | 状态机触发周期 100ms | `bal_cfg.h` | `BAL_FSM_SHORTTIME_100ms (1u)` |
| REQ-014 | 均衡执行周期 1s | `bal_cfg.h` | `BAL_FSM_BALANCING_TIME_100ms (10u)` |
| REQ-015 | 电荷量计算精度 | `bal_strategy_history.c` | `BAL_ActivateBalancing()` — difference 计算公式 |
| REQ-016 | 模块本地数据存储 | `bal_strategy_history.c` | 静态变量 `bal_balancing`、`bal_cellVoltage`、`bal_state` |
| REQ-017 | 无符号整数下溢保护 | `bal_strategy_history.c` | `BAL_ActivateBalancing()` — `if (difference > deltaCharge)` |
| REQ-018 | 均衡安全保护条件 | `bal_strategy_history.c` | `BAL_ProcessStateBalancing()` — 四条安全检查 |
| REQ-019 | 临界区保护 | `bal_strategy_history.c` | `BAL_SetStateRequest()` — `OS_EnterTaskCritical()` |
| REQ-020 | 可配置参数依赖 | `bal_cfg.h` / `battery_cell_cfg.h` / `bms-slave_cfg.h` | 各宏定义 |
| REQ-021 | 单元测试支持 | `bal_strategy_history.c` | `#ifdef UNITY_UNIT_TEST` 区块 |

### 8.2 代码 → 需求反向索引

| 函数 | 需求编号 |
|------|----------|
| `BAL_ActivateBalancing()` | REQ-006, REQ-015, REQ-017 |
| `BAL_Deactivate()` | REQ-007 |
| `BAL_ProcessStateCheckBalancing()` | REQ-001, REQ-002, REQ-003 |
| `BAL_ProcessStateBalancing()` | REQ-004, REQ-005, REQ-018 |
| `BAL_CheckImbalances()` | REQ-008 |
| `BAL_ComputeImbalances()` | REQ-009 |
| `BAL_Trigger()` | REQ-010 |
| `BAL_SetStateRequest()` | REQ-011, REQ-019 |
| `BAL_GetInitializationState()` | REQ-012 |

---

**文档生成日期**: 2026-06-06
**生成工具**: Claude Code (write-software-requirements skill)
**适用范围**: foxBMS 2 v1.11.0
**关联文档**: [BAL_SOFTWARE_REQUIREMENTS.md](../BAL_SOFTWARE_REQUIREMENTS.md)
