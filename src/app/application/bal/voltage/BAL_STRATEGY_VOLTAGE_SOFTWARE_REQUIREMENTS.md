# BAL — 基于电压的均衡策略软件需求规格说明

**文件**: `bal_strategy_voltage.c`
**版本**: v1.11.0
**创建日期**: 2020-05-29
**最后更新**: 2026-04-20
**所属层级**: APPLICATION
**模块前缀**: BAL
**许可证**: BSD-3-Clause

---

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 电池管理系统（Battery Management System）中基于电压的电池均衡策略（Voltage-Based Balancing Strategy）的软件需求规格。该策略模块是 BAL 均衡框架的三种具体均衡策略实现之一，负责根据电池单体电压差异决定均衡的激活与停用。

### 1.2 范围

#### 涵盖内容

- 基于电压阈值的均衡激活与停用算法
- 均衡执行状态机的四个子状态处理（ENTRY → CHECK_LOWEST_VOLTAGE → CHECK_CURRENT → ACTIVATE_BALANCING）
- 均衡条件检查与许可评估逻辑
- 均衡阈值迟滞（Hysteresis）机制
- BAL_Trigger() 状态机主调度逻辑
- BAL_SetStateRequest() 和 BAL_GetInitializationState() 的策略实现
- 单元测试支持接口

#### 不涵盖内容

- BAL 核心状态机的通用框架逻辑（属于 `bal.c` 的职责）
- 基于历史数据的均衡策略（`bal_strategy_history.c`）
- 无均衡策略（`bal_strategy_none.c`）
- AFE（Analog Front End）硬件驱动的具体操作
- 数据库的底层读写实现

### 1.3 定义与缩略语

| 术语 | 英文全称 | 说明 |
|------|----------|------|
| BAL | Balancing | 电池均衡模块 |
| FSM | Finite State Machine | 有限状态机 |
| BMS | Battery Management System | 电池管理系统 |
| mV | Millivolt | 毫伏，电压单位 |
| ddegC | Decidegree Celsius | 0.1°C，温度单位 |
| AFE | Analog Front End | 模拟前端芯片 |
| 迟滞 | Hysteresis | 防止均衡完成后过早重新激活的阈值偏移机制 |

### 1.4 参考文献

| 编号 | 文档名称 | 说明 |
|------|----------|------|
| [1] | `bal.h` | BAL 模块头文件，定义数据结构、枚举与公开 API |
| [2] | `bal.c` | BAL 模块核心实现，包含状态机通用逻辑 |
| [3] | `bal_cfg.h` / `bal_cfg.c` | BAL 模块配置参数与阈值管理 |
| [4] | `BAL_SOFTWARE_REQUIREMENTS.md` | BAL 核心模块软件需求规格说明 |
| [5] | `bms.h` | BMS 主控模块头文件，提供电池系统状态查询 |

---

## 2. 总体描述

### 2.1 产品视角

本模块是 BAL 均衡框架的策略实现层，位于 APPLICATION 层。它实现了 BAL 头文件（`bal.h`）中声明为"在均衡策略文件中实现"的三个公开函数（`BAL_Trigger()`、`BAL_SetStateRequest()`、`BAL_GetInitializationState()`），以及 `BAL_Trigger()` 调用的两个静态子函数（`BAL_ProcessStateCheckBalancing()` 和 `BAL_ProcessStateBalancing()`）。

```
┌──────────────────────────────────┐
│          BAL 核心 (bal.c)         │
│  ┌────────────────────────────┐  │
│  │  通用状态机逻辑             │  │
│  │  (SaveLastStates,          │  │
│  │   CheckReEntrance,         │  │
│  │   CheckStateRequest,       │  │
│  │   TransferStateRequest,    │  │
│  │   ProcessStateUninit/Init) │  │
│  └─────────────┬──────────────┘  │
│                │                  │
│  ┌─────────────▼──────────────┐  │
│  │   均衡策略接口              │  │
│  │   (bal_strategy_voltage.c) │  │
│  │   ┌───────────────────┐    │  │
│  │   │  BAL_Trigger()    │    │  │
│  │   │  ├─ CHECK_BAL...  │    │  │
│  │   │  └─ BALANCE       │    │  │
│  │   └───────────────────┘    │  │
│  └────────────────────────────┘  │
└──────────────────────────────────┘
```

### 2.2 工作模式

基于电压的均衡策略在 BAL 状态机的两个状态中工作：

| 状态 | 处理函数 | 说明 |
|------|----------|------|
| `BAL_FSM_CHECK_BALANCING` | `BAL_ProcessStateCheckBalancing()` | 检查均衡是否被允许，评估是否满足进入均衡的条件 |
| `BAL_FSM_BALANCE` | `BAL_ProcessStateBalancing()` | 四子状态均衡执行流程，包含安全检查与均衡激活 |

`BAL_FSM_BALANCE` 状态下的子状态流转：

```
BAL_ENTRY ──► BAL_CHECK_LOWEST_VOLTAGE ──► BAL_CHECK_CURRENT ──► BAL_ACTIVATE_BALANCING
    ▲                                                                    │
    │                                                                    │
    └──────────────────── (均衡未完成) ─────────────────────────────────┘
                                     │
                            (均衡完成) ▼
                         BAL_FSM_CHECK_BALANCING
```

其他状态（UNINITIALIZED、INITIALIZATION、INITIALIZED）由 BAL 核心模块（`bal.c`）处理，本模块仅作为状态分发的入口。

### 2.3 用户特征

本模块的使用者是 BMS 主控模块（`bms.c`）。调用方以 100ms 周期调用 `BAL_Trigger()` 驱动状态机，通过 `BAL_SetStateRequest()` 向状态机发起均衡相关请求，通过 `BAL_GetInitializationState()` 查询初始化状态。

---

## 3. 功能需求

### 3.1 均衡激活管理

#### FR-3.1.1 — 基于电压阈值的均衡激活

- **编号**: FR-3.1.1
- **优先级**: 高
- **对应函数**: `BAL_ActivateBalancing()`
- **描述**: 系统应基于最小单体电压和均衡阈值，确定需要激活均衡的电池单体。对于所有串（String）、所有模块（Module）、所有电池块（Cell Block），当电池块电压超过"该串最低单体电压 + 均衡阈值"时，激活该电池块的均衡。

**输入**

| 来源 | 描述 |
|------|------|
| 数据库 `DATA_BLOCK_ID_CELL_VOLTAGE` | 所有电池块的单体电压数据 |
| 数据库 `DATA_BLOCK_ID_MIN_MAX` | 每串的最小/最大电压和温度数据 |
| `bal_state.balancingThreshold` | 当前有效的均衡阈值（mV） |

**前置条件**

- 数据库中存在有效的单体电压数据块和最小/最大值数据块
- `bal_state.balancingThreshold` 已被初始化（默认值 `BAL_DEFAULT_THRESHOLD_mV + BAL_HYSTERESIS_mV`）

**处理流程**

1. 从数据库读取单体电压数据块（`cellVoltage`）和最小/最大值数据块（`minMax`）
2. 初始化 `finished = true`（假设无需均衡）
3. 遍历所有串：`s = 0 .. (BS_NR_OF_STRINGS - 1)`
4. 获取该串的最小电压：`min = minMax.minimumCellVoltage_mV[s]`
5. 初始化该串的已均衡电池块计数：`nrBalancedCells = 0`
6. 遍历所有模块：`m = 0 .. (BS_NR_OF_MODULES_PER_STRING - 1)`
7. 遍历所有电池块：`cb = 0 .. (BS_NR_OF_CELL_BLOCKS_PER_MODULE - 1)`
8. 比较：若 `cellVoltage.cellVoltage_mV[s][m][cb] > (min + bal_state.balancingThreshold)`：
   - 设置 `activateBalancing[s][m][cb] = true`
   - 设置 `finished = false`（存在需要均衡的电池块）
   - 首次激活时：设置 `balancingThreshold = BAL_GetBalancingThreshold_mV()`（去除迟滞）
   - 设置 `active = true`、`enableBalancing = true`
   - 递增 `nrBalancedCells`
9. 否则：设置 `activateBalancing[s][m][cb] = false`
10. 记录该串已均衡电池块数量：`nrBalancedCells[s] = nrBalancedCells`
11. 将均衡控制数据写回数据库

**后置条件**

- 超过阈值的电池块均衡已激活（`activateBalancing = true`）
- 未超过阈值的电池块均衡处于停用状态（`activateBalancing = false`）
- 若存在需要均衡的电池块：`enableBalancing = true`，`active = true`
- 首次激活均衡时：`balancingThreshold` 被更新为不带迟滞的原始阈值
- 均衡控制数据已同步到数据库

**错误处理**

- 依赖于数据库读写机制的正确性，函数本身无显式错误处理

---

#### FR-3.1.2 — 均衡阈值迟滞设置

- **编号**: FR-3.1.2
- **优先级**: 中
- **对应函数**: `BAL_ActivateBalancing()`
- **描述**: 当首次检测到需要均衡的电池块时，系统应将均衡阈值更新为不带迟滞的原始阈值（通过 `BAL_GetBalancingThreshold_mV()` 获取），使所有低于初始阈值但仍有不均衡的电池块也能一并参与均衡。此机制确保在同一均衡周期内，所有需要均衡的电池块都会被激活。

**处理流程**

1. 在首次设置 `activateBalancing[s][m][cb] = true` 的同一迭代中
2. 调用 `BAL_GetBalancingThreshold_mV()` 获取配置的原始均衡阈值
3. 将 `bal_state.balancingThreshold` 更新为该值
4. 同一函数内后续迭代将使用新的（更低的）阈值进行判断

**前置条件**

- 至少存在一个需要激活均衡的电池块（即 `cellVoltage > min + threshold`）

**后置条件**

- `bal_state.balancingThreshold` 被设置为不带迟滞的原始阈值值
- 该均衡周期内后续参与比较的电池块均使用不带迟滞的阈值

**相关需求**

- 均衡完成后的迟滞恢复参见 [FR-3.4.4](#fr-344--均衡激活子状态处理)

---

#### FR-3.1.3 — 均衡完成判断

- **编号**: FR-3.1.3
- **优先级**: 中
- **对应函数**: `BAL_ActivateBalancing()`
- **描述**: 系统应返回均衡是否已完成的标志。若本轮遍历中没有任何电池块需要激活均衡（`finished = true`），表示当前轮次的均衡已完成。该返回值被 `BAL_ProcessStateBalancing()` 用于决定是否退出均衡状态。

**返回值**

| 返回值 | 含义 |
|--------|------|
| `true` | 均衡已完成 — 所有电池块电压差均在阈值范围内 |
| `false` | 均衡未完成 — 存在至少一个电池块仍需要均衡 |

**后置条件**

- 返回值反映均衡完成状态，由调用方据此决定下一步状态转移

---

### 3.2 均衡停用管理

#### FR-3.2.1 — 全局均衡停用

- **编号**: FR-3.2.1
- **优先级**: 高
- **对应函数**: `BAL_Deactivate()`
- **描述**: 系统应停用所有串、所有模块、所有电池块的均衡功能。同时将所有电池块的累积均衡电荷量（delta charge）清零，清除均衡使能标志和激活标志，并将结果同步到数据库。

**处理流程**

1. 遍历所有串：`s = 0 .. (BS_NR_OF_STRINGS - 1)`
2. 遍历所有模块：`m = 0 .. (BS_NR_OF_MODULES_PER_STRING - 1)`
3. 遍历所有电池块：`cb = 0 .. (BS_NR_OF_CELL_BLOCKS_PER_MODULE - 1)`
4. 清除激活标志：`activateBalancing[s][m][cb] = false`
5. 清零累积电荷：`deltaCharge_mAs[s][m][cb] = 0u`
6. 清零该串的均衡计数：`nrBalancedCells[s] = 0u`
7. 清除均衡使能：`enableBalancing = false`
8. 清除激活标志：`bal_state.active = false`
9. 将均衡控制数据写回数据库

**前置条件**

- 无特殊前置条件，可在任意状态下被调用

**后置条件**

- 所有电池块的均衡标志为 `false`
- 所有累积均衡电荷量清零为 `0 mAs`
- 所有串的已均衡电池块计数为 `0`
- 均衡使能位为 `false`
- `bal_state.active = false`
- 数据已同步到数据库

**错误处理**

- 无显式错误处理，依赖于数据库写入机制

---

### 3.3 均衡条件检查

#### FR-3.3.1 — 均衡许可评估与状态转移

- **编号**: FR-3.3.1
- **优先级**: 高
- **对应函数**: `BAL_ProcessStateCheckBalancing()`
- **描述**: 系统应根据传入的状态请求和全局/局部均衡许可标志，决定是否允许进入均衡执行状态。仅当局部均衡许可（`balancingAllowed`）和全局均衡许可（`balancingGlobalAllowed`）**同时**为 `true`，且电池系统处于静置状态（`BMS_AT_REST`）时，才转移到 `BAL_FSM_BALANCE` 状态。

**输入**

| 参数 | 类型 | 描述 |
|------|------|------|
| `state_request` | `BAL_STATE_REQUEST_e` | 待处理的状态请求 |

**前置条件**

- 状态机当前处于 `BAL_FSM_CHECK_BALANCING` 状态

**处理流程**

1. 检查传入的状态请求：
   - 若 `state_request == BAL_STATE_NO_BALANCING_REQUEST`：设置 `balancingAllowed = false`
   - 若 `state_request == BAL_STATE_ALLOW_BALANCING_REQUEST`：设置 `balancingAllowed = true`
2. 设置定时器为 `BAL_FSM_SHORTTIME_100ms`（100ms）
3. 评估均衡许可条件：
   - 若 `balancingAllowed == false` **或** `balancingGlobalAllowed == false`：
     - 调用 `BAL_Deactivate()` 停用所有均衡
     - 设置 `bal_state.active = false`
     - 保持在 `BAL_FSM_CHECK_BALANCING` 状态
   - 否则（均衡被允许）：
     - 若 `BMS_GetBatterySystemState() == BMS_AT_REST`：转移到 `BAL_FSM_BALANCE` 状态，子状态为 `BAL_ENTRY`
     - 否则：保持在 `BAL_FSM_CHECK_BALANCING` 状态

**后置条件**

- 若均衡被许可且电池静置：`state = BAL_FSM_BALANCE, substate = BAL_ENTRY`
- 若均衡被禁止：所有均衡被停用，`active = false`
- 定时器被重置为 `BAL_FSM_SHORTTIME_100ms`

**错误处理**

- 无显式错误返回，不满足条件时静默保持在当前状态

---

### 3.4 均衡执行状态机

均衡执行状态机在 `BAL_ProcessStateBalancing()` 函数中实现，包含四个子状态和全局禁用检查。每个子状态执行完毕后设置定时器并返回，由下一触发周期继续执行。

#### FR-3.4.1 — 全局禁用优先处理

- **编号**: FR-3.4.1
- **优先级**: 高
- **对应函数**: `BAL_ProcessStateBalancing()`
- **描述**: 系统应在处理任何子状态之前，优先检查全局均衡许可标志（`balancingGlobalAllowed`）。若全局均衡被禁用，无论当前处于哪个子状态，应立即停用均衡并返回 `BAL_FSM_CHECK_BALANCING` 状态。

**处理流程**

1. 在子状态分发之前，首先检查 `balancingGlobalAllowed`
2. 若为 `false`：
   - 若 `bal_state.active == true`：调用 `BAL_Deactivate()`
   - 设置 `active = false`
   - 设置 `state = BAL_FSM_CHECK_BALANCING`
   - 设置 `substate = BAL_ENTRY`
   - 设置 `timer = BAL_FSM_SHORTTIME_100ms`
   - 返回（不执行后续子状态处理）

**前置条件**

- 状态机当前处于 `BAL_FSM_BALANCE` 状态

**后置条件**

- 若全局禁用：均衡停用，返回 CHECK_BALANCING 状态
- 若全局使能：继续执行子状态处理

---

#### FR-3.4.2 — ENTRY 子状态处理

- **编号**: FR-3.4.2
- **优先级**: 高
- **对应函数**: `BAL_ProcessStateBalancing()`，`case BAL_ENTRY`
- **描述**: 在 ENTRY 子状态下，系统应检查局部均衡许可标志（`balancingAllowed`）。若均衡不被允许，停用均衡并返回检查状态；若均衡被允许，进入最低电压检查子状态。

**处理流程**

1. 检查并更新 `state_request`：同 FR-3.3.1 的处理逻辑
2. 若 `balancingAllowed == false`：
   - 若 `active == true`：调用 `BAL_Deactivate()`
   - 设置 `active = false`
   - 设置 `state = BAL_FSM_CHECK_BALANCING, substate = BAL_ENTRY`
3. 若 `balancingAllowed == true`：
   - 设置 `substate = BAL_CHECK_LOWEST_VOLTAGE`
4. 设置 `timer = BAL_FSM_SHORTTIME_100ms`

**后置条件**

- 若允许均衡：`substate = BAL_CHECK_LOWEST_VOLTAGE`
- 若不允许均衡：返回 `BAL_FSM_CHECK_BALANCING`

---

#### FR-3.4.3 — CHECK_LOWEST_VOLTAGE 子状态处理

- **编号**: FR-3.4.3
- **优先级**: 高
- **对应函数**: `BAL_ProcessStateBalancing()`，`case BAL_CHECK_LOWEST_VOLTAGE`
- **描述**: 系统应检查所有串的最低单体电压和最高温度是否在安全范围内。若任一串的最低电压低于均衡电压下限（`BAL_LOWER_VOLTAGE_LIMIT_mV = 2000mV`）或最高温度超过均衡温度上限（`BAL_UPPER_TEMPERATURE_LIMIT_ddegC = 700`，即 70.0°C），应立即停止均衡。

**处理流程**

1. 从数据库读取 MIN_MAX 数据块（`checkMinMax`）
2. 遍历所有串：`s = 0 .. (BS_NR_OF_STRINGS - 1)`
3. 检查两项安全条件：
   - `minimumCellVoltage_mV[s] <= BAL_LOWER_VOLTAGE_LIMIT_mV`（电压过低）
   - `maximumTemperature_ddegC[s] >= BAL_UPPER_TEMPERATURE_LIMIT_ddegC`（温度过高）
4. 若任一条件满足：
   - 若 `active == true`：调用 `BAL_Deactivate()`
   - 设置 `state = BAL_FSM_CHECK_BALANCING, substate = BAL_ENTRY`
5. 若两项条件均不满足：转移到下一子状态（`substate = BAL_CHECK_CURRENT`）
6. 设置 `timer = BAL_FSM_BALANCING_TIME_100ms`（10 × 100ms = 1秒）

**前置条件**

- 子状态为 `BAL_CHECK_LOWEST_VOLTAGE`

**后置条件**

- 安全性检查通过：进入 `BAL_CHECK_CURRENT` 子状态
- 安全性检查失败：均衡停用，返回 `BAL_FSM_CHECK_BALANCING`

**相关配置参数**

| 宏定义 | 值 | 单位 | 说明 |
|--------|-----|------|------|
| `BAL_LOWER_VOLTAGE_LIMIT_mV` | `2000` | mV | 允许均衡的最低单体电压 |
| `BAL_UPPER_TEMPERATURE_LIMIT_ddegC` | `700` | 0.1°C | 允许均衡的最高温度（70.0°C） |

---

#### FR-3.4.4 — CHECK_CURRENT 子状态处理

- **编号**: FR-3.4.4
- **优先级**: 高
- **对应函数**: `BAL_ProcessStateBalancing()`，`case BAL_CHECK_CURRENT`
- **描述**: 系统应确认电池系统仍处于静置状态（`BMS_AT_REST`）。均衡只能在电池无显著电流流动（静置状态）时执行。若电池系统不再处于静置状态，停止均衡并返回检查状态。

**处理流程**

1. 调用 `BMS_GetBatterySystemState()` 获取当前电池系统状态
2. 若 `== BMS_AT_REST`：
   - 设置 `substate = BAL_ACTIVATE_BALANCING`（进入均衡激活子状态）
3. 若 `!= BMS_AT_REST`：
   - 若 `active == true`：调用 `BAL_Deactivate()`
   - 设置 `state = BAL_FSM_CHECK_BALANCING, substate = BAL_ENTRY`
4. 设置 `timer = BAL_FSM_BALANCING_TIME_100ms`（1秒）

**前置条件**

- 子状态为 `BAL_CHECK_CURRENT`

**后置条件**

- 电池静置：进入 `BAL_ACTIVATE_BALANCING` 子状态
- 电池非静置：均衡停用，返回 `BAL_FSM_CHECK_BALANCING`

**说明**

- 此检查确保均衡仅在电池处于静置状态时执行，避免在充电或放电过程中因电流导致电压读数偏差而做出错误的均衡决策

---

#### FR-3.4.5 — ACTIVATE_BALANCING 子状态处理

- **编号**: FR-3.4.5
- **优先级**: 高
- **对应函数**: `BAL_ProcessStateBalancing()`，`case BAL_ACTIVATE_BALANCING`
- **描述**: 系统应调用均衡激活函数执行实际的均衡操作。若均衡已完成，应用迟滞阈值以防止过早重新激活均衡，然后返回检查状态；若均衡未完成，继续保持在均衡状态进行下一轮均衡。

**处理流程**

1. 检查并更新 `state_request`：同 FR-3.3.1 的处理逻辑
2. 若 `balancingAllowed == false`：
   - 若 `active == true`：调用 `BAL_Deactivate()`
   - 设置 `active = false`
   - 设置 `state = BAL_FSM_CHECK_BALANCING, substate = BAL_ENTRY`
3. 若 `balancingAllowed == true`：
   - 调用 `BAL_ActivateBalancing()`
   - 若返回 `true`（均衡完成）：
     - 将 `balancingThreshold` 设置为 `BAL_GetBalancingThreshold_mV() + BAL_HYSTERESIS_mV`
     - 设置 `state = BAL_FSM_CHECK_BALANCING, substate = BAL_ENTRY`
     - **说明**: 施加迟滞可防止均衡刚完成后因微小电压波动而立即重新激活均衡
   - 若返回 `false`（仍有电池块需要均衡）：
     - 设置 `state = BAL_FSM_BALANCE, substate = BAL_ENTRY`
     - 保持在均衡状态继续下一轮
4. 设置 `timer = BAL_FSM_BALANCING_TIME_100ms`（1秒）

**后置条件**

| 情况 | 新状态 | 新子状态 | balancingThreshold |
|------|--------|----------|---------------------|
| 均衡完成 | `BAL_FSM_CHECK_BALANCING` | `BAL_ENTRY` | 原始阈值 + 迟滞 |
| 均衡未完成 | `BAL_FSM_BALANCE` | `BAL_ENTRY` | 保持不变 |
| 均衡被禁止 | `BAL_FSM_CHECK_BALANCING` | `BAL_ENTRY` | 保持不变 |

**相关需求**

- 与 FR-3.1.2 互补：FR-3.1.2 在均衡开始时去除迟滞，本需求在均衡完成时施加迟滞

---

### 3.5 状态机触发与调度

#### FR-3.5.1 — 重入保护检查

- **编号**: FR-3.5.1
- **优先级**: 高
- **对应函数**: `BAL_Trigger()`
- **描述**: 系统应在 `BAL_Trigger()` 函数入口处调用 `BAL_CheckReEntrance()` 进行重入检查。若返回值大于 0（检测到重入），函数应立即返回，不执行任何状态机操作。

**处理流程**

1. 调用 `BAL_CheckReEntrance(&bal_state)`
2. 若返回值 `> 0u`：`return`（立即返回，放弃本次执行）

**相关需求**

- 对应于 `bal.c` 中的 FR-3.2.1（重入检查机制）

---

#### FR-3.5.2 — 定时器倒计时管理

- **编号**: FR-3.5.2
- **优先级**: 高
- **对应函数**: `BAL_Trigger()`
- **描述**: 系统应在 `BAL_Trigger()` 中管理定时器倒计时。若定时器尚未归零（`timer > 0`），递减定时器并跳过本次状态机处理。

**处理流程**

1. 若 `bal_state.timer > 0u`：
   - 递减定时器：`--bal_state.timer`
   - 若递减后 `> 0`：递减 `triggerEntry` 并返回（跳过状态机处理）
2. 若定时器已归零（`timer == 0`）：继续执行状态机分发逻辑

**前置条件**

- 已通过重入检查

**后置条件**

- 定时器 > 0：本次不处理状态机
- 定时器 == 0：进入状态分发流程

---

#### FR-3.5.3 — 状态分发

- **编号**: FR-3.5.3
- **优先级**: 高
- **对应函数**: `BAL_Trigger()`
- **描述**: 系统应根据当前状态（`bal_state.state`）分发到对应的状态处理函数。

**处理流程**

```
switch (bal_state.state):
  case BAL_FSM_UNINITIALIZED:
    1. BAL_SaveLastStates(&bal_state)
    2. BAL_TransferStateRequest(&bal_state) → stateRequest
    3. BAL_ProcessStateUninitialized(&bal_state, stateRequest)
    break

  case BAL_FSM_INITIALIZATION:
    1. BAL_SaveLastStates(&bal_state)
    2. BAL_Init(&bal_tableBalancingControl)
    3. BAL_ProcessStateInitialization(&bal_state)
    break

  case BAL_FSM_INITIALIZED:
    1. BAL_SaveLastStates(&bal_state)
    2. BAL_ProcessStateInitialized(&bal_state)
    break

  case BAL_FSM_CHECK_BALANCING:
    1. BAL_SaveLastStates(&bal_state)
    2. BAL_TransferStateRequest(&bal_state) → stateRequest
    3. BAL_ProcessStateCheckBalancing(stateRequest)
    break

  case BAL_FSM_BALANCE:
    1. BAL_SaveLastStates(&bal_state)
    2. BAL_TransferStateRequest(&bal_state) → stateRequest
    3. BAL_ProcessStateBalancing(stateRequest)
    break

  default:
    FAS_ASSERT(FAS_TRAP)  // 非法状态，触发断言
```

**后置条件**

- 每次执行完毕后递减 `triggerEntry`（`bal_state.triggerEntry--`）

**错误处理**

- 若 `state` 为非法值（不属于上述任何枚举值）：触发 `FAS_ASSERT(FAS_TRAP)` 断言

---

#### FR-3.5.4 — 临界区保护下的状态请求设置

- **编号**: FR-3.5.4
- **优先级**: 高
- **对应函数**: `BAL_SetStateRequest()`
- **描述**: 系统应在任务临界区保护下调用 `BAL_CheckStateRequest()` 校验请求合法性。仅当合法性校验返回 `BAL_OK` 时，才将状态请求写入 `bal_state.stateRequest`。

**输入**

| 参数 | 类型 | 描述 |
|------|------|------|
| `stateRequest` | `BAL_STATE_REQUEST_e` | 待设置的状态请求 |

**处理流程**

1. 初始化 `returnValue = BAL_OK`
2. 进入任务临界区：`OS_EnterTaskCritical()`
3. 调用 `BAL_CheckStateRequest(&bal_state, stateRequest)` 校验请求
4. 赋返回值给 `returnValue`
5. 若 `returnValue == BAL_OK`：将请求写入 `bal_state.stateRequest = stateRequest`
6. 退出任务临界区：`OS_ExitTaskCritical()`
7. 返回 `returnValue`

**前置条件**

- 操作系统已初始化，临界区机制可用

**后置条件**

- 若请求合法：`bal_state.stateRequest = stateRequest`
- 若请求不合法：`bal_state.stateRequest` 保持不变

**返回值**

| 返回值 | 含义 |
|--------|------|
| `BAL_OK` | 请求合法，已写入 |
| `BAL_REQUEST_PENDING` | 存在待处理请求 |
| `BAL_ALREADY_INITIALIZED` | 已初始化，拒绝重复初始化 |
| `BAL_ILLEGAL_REQUEST` | 当前状态下请求非法 |

**相关需求**

- 依赖于 BAL 核心模块的 FR-3.4.1（直接通过类请求校验）和 FR-3.4.2（初始化请求校验）

---

### 3.6 状态查询

#### FR-3.6.1 — 初始化状态查询

- **编号**: FR-3.6.1
- **优先级**: 中
- **对应函数**: `BAL_GetInitializationState()`
- **描述**: 系统应返回当前均衡模块的初始化完成状态，供外部模块查询。

**返回值**

| 返回值 | 含义 |
|--------|------|
| `STD_OK` | 均衡初始化已完成 |
| `STD_NOT_OK` | 均衡初始化尚未完成 |

**后置条件**

- 无状态修改，纯查询操作

**相关需求**

- 对应于 BAL 核心模块的 FR-3.6.3（已初始化状态处理，设置 `initializationFinished = STD_OK`）

---

### 3.7 均衡执行状态转移图

```
BAL_FSM_CHECK_BALANCING
  │
  │ (balancingAllowed && balancingGlobalAllowed && BMS_AT_REST)
  ▼
BAL_FSM_BALANCE (substate = BAL_ENTRY)
  │
  │ [balancingAllowed == true]
  ▼
BAL_CHECK_LOWEST_VOLTAGE
  │
  │ [voltage > BAL_LOWER_VOLTAGE_LIMIT_mV && temp < BAL_UPPER_TEMPERATURE_LIMIT_ddegC]
  ▼
BAL_CHECK_CURRENT
  │
  │ [BMS_AT_REST]
  ▼
BAL_ACTIVATE_BALANCING
  │
  ├── [finished == true] ──► BAL_FSM_CHECK_BALANCING (施加迟滞)
  │
  └── [finished == false] ──► BAL_FSM_BALANCE (继续均衡)

任意子状态:
  ├── [balancingGlobalAllowed == false] ──► BAL_FSM_CHECK_BALANCING
  ├── [balancingAllowed == false] ──► BAL_FSM_CHECK_BALANCING
  ├── [voltage <= limit || temp >= limit] ──► BAL_FSM_CHECK_BALANCING (仅CHECK_LOWEST_VOLTAGE子状态)
  └── [!BMS_AT_REST] ──► BAL_FSM_CHECK_BALANCING (仅CHECK_CURRENT子状态)
```

---

## 4. 非功能需求

### 4.1 时序与性能

#### NFR-4.1.1 — 状态机触发周期

- **编号**: NFR-4.1.1
- **描述**: `BAL_Trigger()` 应以 **100ms** 为周期被 `bms.c` 定时调用。
- **依据**: `BAL_FSM_SHORTTIME_100ms = 1u`（1 × 100ms），`bal.h` 中文档说明

#### NFR-4.1.2 — 均衡检查周期

- **编号**: NFR-4.1.2
- **描述**: 均衡执行状态机（`BAL_ProcessStateBalancing()`）中各子状态（CHECK_LOWEST_VOLTAGE、CHECK_CURRENT、ACTIVATE_BALANCING）的定时器应设置为 `BAL_FSM_BALANCING_TIME_100ms = 10u`（即 10 × 100ms = **1000ms**）。
- **依据**: `bal_cfg.h` 中 `BAL_FSM_BALANCING_TIME_100ms (10u)`

#### NFR-4.1.3 — 条件检查周期

- **编号**: NFR-4.1.3
- **描述**: 均衡条件检查（`BAL_ProcessStateCheckBalancing()`）和 ENTRY 子状态的定时器应设置为 `BAL_FSM_SHORTTIME_100ms = 1u`（即 **100ms**）。
- **依据**: 两种场景均使用 `bal_state.timer = BAL_FSM_SHORTTIME_100ms`

### 4.2 内存

#### NFR-4.2.1 — 数据块静态分配

- **编号**: NFR-4.2.1
- **描述**: 用于数据库数据读取的局部数据结构体应使用 `static` 存储类声明，放置于数据段（data segment）而非栈（stack）上，避免大型结构体造成栈溢出。具体为：
  - `bal_tableBalancingControl`（全局静态变量）：`DATA_BLOCK_BALANCING_CONTROL_s`
  - `cellVoltage`（函数内静态变量）：`DATA_BLOCK_CELL_VOLTAGE_s`
- **依据**: `bal_strategy_voltage.c:70` 的 `static DATA_BLOCK_BALANCING_CONTROL_s bal_tableBalancingControl` 和 `bal_strategy_voltage.c:126` 的 `static DATA_BLOCK_CELL_VOLTAGE_s cellVoltage`

### 4.3 鲁棒性

#### NFR-4.3.1 — 临界区保护

- **编号**: NFR-4.3.1
- **描述**: 状态请求的读-校验-写操作（`BAL_SetStateRequest()`）必须在任务临界区内执行，确保在 RTOS 多任务环境下操作的原子性。具体包括：
  - `OS_EnterTaskCritical()` 进入临界区
  - 调用 `BAL_CheckStateRequest()` 校验 + 条件写入
  - `OS_ExitTaskCritical()` 退出临界区
- **依据**: `BAL_SetStateRequest()` 中使用了 `OS_EnterTaskCritical()` / `OS_ExitTaskCritical()`

#### NFR-4.3.2 — 无效状态断言保护

- **编号**: NFR-4.3.2
- **描述**: `BAL_Trigger()` 中的 `default` 分支应触发 `FAS_ASSERT(FAS_TRAP)` 断言，确保在状态机出现非法状态值（如内存损坏导致的意外值）时系统能立即停止运行，而非静默产生未定义行为。
- **依据**: `bal_strategy_voltage.c:340` 的 `FAS_ASSERT(FAS_TRAP)`

#### NFR-4.3.3 — 多级均衡保护

- **编号**: NFR-4.3.3
- **描述**: 均衡激活需经过多级安全检查，形成纵深防御（Defense in Depth）：
  1. **全局许可检查**（`balancingGlobalAllowed`）：系统级均衡总开关
  2. **局部许可检查**（`balancingAllowed`）：运行时条件判断
  3. **电池状态检查**（`BMS_AT_REST`）：确保电池处于静置状态
  4. **安全阈值检查**：最低电压下限和最高温度上限
  5. **阈值迟滞**：防止均衡完成后频繁启停

  任一检查不通过即停止均衡。
- **依据**: `BAL_ProcessStateBalancing()` 中贯穿的多级检查逻辑

### 4.4 可配置性

#### NFR-4.4.1 — 配置参数集中管理

- **编号**: NFR-4.4.1
- **描述**: 基于电压的均衡策略使用的所有可调阈值参数应集中定义在 `bal_cfg.h` 中，包括但不限于：

| 宏定义 | 默认值 | 单位 | 说明 | 关联需求 |
|--------|--------|------|------|----------|
| `BAL_DEFAULT_THRESHOLD_mV` | `200` | mV | 默认均衡阈值 | FR-3.1.1 |
| `BAL_HYSTERESIS_mV` | `200` | mV | 均衡结束迟滞 | FR-3.1.2, FR-3.4.5 |
| `BAL_LOWER_VOLTAGE_LIMIT_mV` | `2000` | mV | 均衡电压下限（2.0V） | FR-3.4.3 |
| `BAL_UPPER_TEMPERATURE_LIMIT_ddegC` | `700` | 0.1°C | 均衡温度上限（70.0°C） | FR-3.4.3 |
| `BAL_FSM_SHORTTIME_100ms` | `1u` | ×100ms | 短延时（100ms） | NFR-4.1.1, NFR-4.1.3 |
| `BAL_FSM_BALANCING_TIME_100ms` | `10u` | ×100ms | 均衡执行周期（1s） | NFR-4.1.2 |

- **依据**: `bal_cfg.h` 中所有 `#define BAL_*` 宏定义

### 4.5 可测试性

#### NFR-4.5.1 — 单元测试支持

- **编号**: NFR-4.5.1
- **描述**: 模块应通过条件编译（`#ifdef UNITY_UNIT_TEST`）暴露内部函数和静态变量，支持单元测试框架直接访问。暴露的接口包括：

| 测试接口 | 类型 | 描述 |
|----------|------|------|
| `BAL_GetState()` | 函数 | 获取状态机当前状态 |
| `TEST_BAL_GetBalancingControl()` | 函数 | 获取均衡控制数据块指针 |
| `TEST_BAL_GetBalancingState()` | 函数 | 获取均衡状态结构体指针 |

- **依据**: `bal_strategy_voltage.c:347-359` 的 `#ifdef UNITY_UNIT_TEST` 区块

---

## 5. 接口需求

### 5.1 本模块实现的公共 API

| 函数 | 返回类型 | 描述 | 关联需求 |
|------|----------|------|----------|
| `BAL_Trigger()` | `void` | 状态机主触发函数（100ms 周期） | FR-3.5.1 ~ FR-3.5.3 |
| `BAL_SetStateRequest()` | `BAL_RETURN_TYPE_e` | 向状态机发起状态请求 | FR-3.5.4 |
| `BAL_GetInitializationState()` | `STD_RETURN_TYPE_e` | 查询初始化完成状态 | FR-3.6.1 |

### 5.2 本模块调用的外部 API

| 函数 | 来源模块 | 描述 |
|------|----------|------|
| `BAL_SaveLastStates()` | `bal.c` | 保存状态历史记录 |
| `BAL_CheckReEntrance()` | `bal.c` | 重入检测 |
| `BAL_TransferStateRequest()` | `bal.c` | 转移状态请求 |
| `BAL_CheckStateRequest()` | `bal.c` | 校验状态请求合法性 |
| `BAL_Init()` | `bal.c` | 均衡模块通用初始化 |
| `BAL_ProcessStateUninitialized()` | `bal.c` | 处理未初始化状态 |
| `BAL_ProcessStateInitialization()` | `bal.c` | 处理初始化状态 |
| `BAL_ProcessStateInitialized()` | `bal.c` | 处理已初始化状态 |
| `BAL_GetBalancingThreshold_mV()` | `bal_cfg.c` | 获取当前均衡阈值 |
| `BMS_GetBatterySystemState()` | `bms.c` | 获取电池系统电流状态 |
| `DATA_READ_DATA()` | `database` | 从数据库读取数据块 |
| `DATA_WRITE_DATA()` | `database` | 将数据块写入数据库 |
| `OS_EnterTaskCritical()` | `os` | 进入任务临界区 |
| `OS_ExitTaskCritical()` | `os` | 退出任务临界区 |

### 5.3 数据库依赖

| 数据块 ID | 类型 | 读写 | 用途 |
|-----------|------|------|------|
| `DATA_BLOCK_ID_BALANCING_CONTROL` | `DATA_BLOCK_BALANCING_CONTROL_s` | 读/写 | 均衡控制数据：激活标志、delta charge、使能位 |
| `DATA_BLOCK_ID_CELL_VOLTAGE` | `DATA_BLOCK_CELL_VOLTAGE_s` | 只读 | 所有电池块的单体电压数据 |
| `DATA_BLOCK_ID_MIN_MAX` | `DATA_BLOCK_MIN_MAX_s` | 只读 | 每串的最小/最大电压和温度 |

### 5.4 调用方

| 调用模块 | 调用接口 | 场景 |
|----------|----------|------|
| `bms.c` | `BAL_Trigger()` | 以 100ms 周期触发状态机 |
| `bms.c` | `BAL_SetStateRequest()` | 发起均衡相关状态请求 |
| `bms.c` | `BAL_GetInitializationState()` | 查询均衡初始化状态 |

---

## 6. 数据结构

### 6.1 静态变量

| 变量 | 类型 | 存储类 | 描述 |
|------|------|--------|------|
| `bal_tableBalancingControl` | `DATA_BLOCK_BALANCING_CONTROL_s` | 文件级 static | 均衡控制数据库表的本地副本，包含激活标志、delta charge、使能位 |
| `bal_state` | `BAL_STATE_s` | 文件级 static | 均衡状态机的完整运行状态 |

`BAL_STATE_s` 结构体和 `DATA_BLOCK_BALANCING_CONTROL_s` 数据结构的详细定义参见 [BAL_SOFTWARE_REQUIREMENTS.md 第 6.1 节](../BAL_SOFTWARE_REQUIREMENTS.md#61-bal_state_s--均衡状态结构体)。

---

## 7. 控制流

### 7.1 BAL_Trigger() 完整流程

```
BAL_Trigger() (每 100ms 调用)
  │
  ├─ BAL_CheckReEntrance(&bal_state)           // FR-3.5.1: 重入检查
  │   └─ 若 > 0: return
  │
  ├─ 定时器管理                                 // FR-3.5.2
  │   └─ 若 timer > 0: --timer; 若仍 > 0: return
  │
  └─ 状态分发 (switch)                          // FR-3.5.3
      │
      ├─ BAL_FSM_UNINITIALIZED
      │   ├─ BAL_SaveLastStates()
      │   ├─ BAL_TransferStateRequest()
      │   └─ BAL_ProcessStateUninitialized()
      │
      ├─ BAL_FSM_INITIALIZATION
      │   ├─ BAL_SaveLastStates()
      │   ├─ BAL_Init(&bal_tableBalancingControl)
      │   └─ BAL_ProcessStateInitialization()
      │
      ├─ BAL_FSM_INITIALIZED
      │   ├─ BAL_SaveLastStates()
      │   └─ BAL_ProcessStateInitialized()
      │
      ├─ BAL_FSM_CHECK_BALANCING               // FR-3.3.1: 均衡条件检查
      │   ├─ BAL_SaveLastStates()
      │   ├─ BAL_TransferStateRequest()
      │   └─ BAL_ProcessStateCheckBalancing()
      │
      ├─ BAL_FSM_BALANCE                       // FR-3.4.x: 均衡执行状态机
      │   ├─ BAL_SaveLastStates()
      │   ├─ BAL_TransferStateRequest()
      │   └─ BAL_ProcessStateBalancing()
      │
      └─ default: FAS_ASSERT(FAS_TRAP)         // NFR-4.3.2
```

### 7.2 BAL_ProcessStateBalancing() 子状态流程

```
BAL_ProcessStateBalancing(state_request)
  │
  ├─ 更新 balancingAllowed (根据 state_request)    // 同 FR-3.3.1
  │
  ├─ [balancingGlobalAllowed == false]            // FR-3.4.1: 全局禁用优先
  │   └─ BAL_Deactivate() → CHECK_BALANCING
  │
  └─ switch(substate):
      │
      ├─ BAL_ENTRY:                                // FR-3.4.2
      │   ├─ [balancingAllowed == false]
      │   │   └─ BAL_Deactivate() → CHECK_BALANCING
      │   └─ [balancingAllowed == true]
      │       └─ substate = CHECK_LOWEST_VOLTAGE
      │
      ├─ BAL_CHECK_LOWEST_VOLTAGE:                 // FR-3.4.3
      │   ├─ 读取 MIN_MAX 数据块
      │   ├─ [voltage ≤ BAL_LOWER_VOLTAGE_LIMIT_mV || temp ≥ BAL_UPPER_TEMPERATURE_LIMIT_ddegC]
      │   │   └─ BAL_Deactivate() → CHECK_BALANCING
      │   └─ [安全条件满足]
      │       └─ substate = CHECK_CURRENT
      │
      ├─ BAL_CHECK_CURRENT:                        // FR-3.4.4
      │   ├─ [BMS_GetBatterySystemState() == BMS_AT_REST]
      │   │   └─ substate = ACTIVATE_BALANCING
      │   └─ [!BMS_AT_REST]
      │       └─ BAL_Deactivate() → CHECK_BALANCING
      │
      └─ BAL_ACTIVATE_BALANCING:                   // FR-3.4.5
          ├─ [balancingAllowed == false]
          │   └─ BAL_Deactivate() → CHECK_BALANCING
          ├─ [BAL_ActivateBalancing() == true]    // 均衡完成
          │   ├─ balancingThreshold = BAL_GetBalancingThreshold_mV() + BAL_HYSTERESIS_mV
          │   └─ → CHECK_BALANCING
          └─ [BAL_ActivateBalancing() == false]   // 均衡未完成
              └─ → BALANCE (继续下一轮)
```

---

## 8. 追溯矩阵

### 8.1 需求 → 代码位置映射

#### 功能需求（FR）

| 需求编号 | 需求简述 | 实现文件 | 函数 / 定义 |
|----------|----------|----------|-------------|
| FR-3.1.1 | 基于电压阈值的均衡激活 | `bal_strategy_voltage.c` | `BAL_ActivateBalancing()` (L122-153) |
| FR-3.1.2 | 均衡阈值迟滞设置 | `bal_strategy_voltage.c` | `BAL_ActivateBalancing()` 中 L139 |
| FR-3.1.3 | 均衡完成判断 | `bal_strategy_voltage.c` | `BAL_ActivateBalancing()` L122, L152 |
| FR-3.2.1 | 全局均衡停用 | `bal_strategy_voltage.c` | `BAL_Deactivate()` (L155-169) |
| FR-3.3.1 | 均衡许可评估与状态转移 | `bal_strategy_voltage.c` | `BAL_ProcessStateCheckBalancing()` (L171-189) |
| FR-3.4.1 | 全局禁用优先处理 | `bal_strategy_voltage.c` | `BAL_ProcessStateBalancing()` L200-209 |
| FR-3.4.2 | ENTRY 子状态处理 | `bal_strategy_voltage.c` | `BAL_ProcessStateBalancing()` L211-223 |
| FR-3.4.3 | CHECK_LOWEST_VOLTAGE 子状态 | `bal_strategy_voltage.c` | `BAL_ProcessStateBalancing()` L224-241 |
| FR-3.4.4 | CHECK_CURRENT 子状态处理 | `bal_strategy_voltage.c` | `BAL_ProcessStateBalancing()` L242-253 |
| FR-3.4.5 | ACTIVATE_BALANCING 子状态 | `bal_strategy_voltage.c` | `BAL_ProcessStateBalancing()` L254-275 |
| FR-3.5.1 | 重入保护检查 | `bal_strategy_voltage.c` | `BAL_Trigger()` L300-303 |
| FR-3.5.2 | 定时器倒计时管理 | `bal_strategy_voltage.c` | `BAL_Trigger()` L305-310 |
| FR-3.5.3 | 状态分发 | `bal_strategy_voltage.c` | `BAL_Trigger()` L312-343 |
| FR-3.5.4 | 临界区保护的状态请求设置 | `bal_strategy_voltage.c` | `BAL_SetStateRequest()` (L283-295) |
| FR-3.6.1 | 初始化状态查询 | `bal_strategy_voltage.c` | `BAL_GetInitializationState()` (L279-281) |

#### 非功能需求（NFR）

| 需求编号 | 需求简述 | 实现文件 | 函数 / 定义 |
|----------|----------|----------|-------------|
| NFR-4.1.1 | 状态机触发周期 100ms | `bal_cfg.h` | `BAL_FSM_SHORTTIME_100ms (1u)` |
| NFR-4.1.2 | 均衡检查周期 1s | `bal_cfg.h` | `BAL_FSM_BALANCING_TIME_100ms (10u)` |
| NFR-4.1.3 | 条件检查周期 100ms | `bal_strategy_voltage.c` | `BAL_ProcessStateCheckBalancing()` L179 |
| NFR-4.2.1 | 数据块静态分配 | `bal_strategy_voltage.c` | L70, L126: `static` 声明 |
| NFR-4.3.1 | 临界区保护 | `bal_strategy_voltage.c` | `BAL_SetStateRequest()` L286-292 |
| NFR-4.3.2 | 无效状态断言保护 | `bal_strategy_voltage.c` | `BAL_Trigger()` L340 |
| NFR-4.3.3 | 多级均衡保护 | `bal_strategy_voltage.c` | `BAL_ProcessStateBalancing()` 多级检查 |
| NFR-4.4.1 | 配置参数集中管理 | `bal_cfg.h` | 所有 `#define BAL_*` 宏 |
| NFR-4.5.1 | 单元测试支持 | `bal_strategy_voltage.c` | L347-359: `#ifdef UNITY_UNIT_TEST` 区块 |

### 8.2 代码 → 需求反向索引

| 函数 / 代码块 | 所在行 | 实现的需求 |
|---------------|--------|-----------|
| `BAL_ActivateBalancing()` | L122-153 | FR-3.1.1, FR-3.1.2, FR-3.1.3 |
| `BAL_Deactivate()` | L155-169 | FR-3.2.1 |
| `BAL_ProcessStateCheckBalancing()` | L171-189 | FR-3.3.1, NFR-4.1.3 |
| `BAL_ProcessStateBalancing()` — 全局禁用检查 | L200-209 | FR-3.4.1 |
| `BAL_ProcessStateBalancing()` — ENTRY | L211-223 | FR-3.4.2 |
| `BAL_ProcessStateBalancing()` — CHECK_LOWEST_VOLTAGE | L224-241 | FR-3.4.3 |
| `BAL_ProcessStateBalancing()` — CHECK_CURRENT | L242-253 | FR-3.4.4 |
| `BAL_ProcessStateBalancing()` — ACTIVATE_BALANCING | L254-275 | FR-3.4.5 |
| `BAL_Trigger()` — 重入检查 | L300-303 | FR-3.5.1 |
| `BAL_Trigger()` — 定时器管理 | L305-310 | FR-3.5.2 |
| `BAL_Trigger()` — 状态分发 | L312-343 | FR-3.5.3, NFR-4.3.2 |
| `BAL_SetStateRequest()` | L283-295 | FR-3.5.4, NFR-4.3.1 |
| `BAL_GetInitializationState()` | L279-281 | FR-3.6.1 |
| `#ifdef UNITY_UNIT_TEST` 区块 | L347-359 | NFR-4.5.1 |

---

**文档生成日期**: 2026-06-06
**生成工具**: Claude Code (write-software-requirements skill)
**适用范围**: foxBMS 2 v1.11.0
**关联文档**: [BAL_SOFTWARE_REQUIREMENTS.md](../BAL_SOFTWARE_REQUIREMENTS.md)
