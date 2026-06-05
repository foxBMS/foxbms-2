# SOC 库仑计数模块 — 软件需求规格说明

## 1. 引言

### 1.1 目的

本文档规定了 foxBMS 2 电池管理系统中 **SOC 库仑计数**模块
（[soc_counting.c](soc_counting.c)）的功能和非功能需求。
该模块实现库仑计数（电流积分）算法，用于实时估算每串电池的荷电状态（SOC）。

### 1.2 范围

SOC 库仑计数模块涵盖以下内容：

- 上电时从非易失存储器（FRAM）初始化 SOC
- 通过库仑计数（原始电流积分）或库仑计数器硬件传感器积分进行周期性 SOC 更新
- 电池静置时通过电压-SOC 查表进行 SOC 重校准
- 电压-SOC 查表插值
- SOC 范围限制及持久化保存至非易失存储器
- 充放电吞吐量累积

本模块**不**涵盖以下内容：

- 能量状态（SOE）或健康状态（SOH）估算
- 电流传感器或电压测量硬件驱动
- FRAM 硬件抽象层
- 数据库基础设施

### 1.3 定义与缩略语

| 术语 | 定义 |
|------|------|
| **SOC** | 荷电状态（State-of-Charge），以电池串标称容量的百分比 [0%, 100%] 表示 |
| **CC** | 库仑计数器（Coulomb Counter）—— 一种硬件电流积分器，提供以 As 为单位的累积电荷 |
| **LUT** | 查表（Lookup Table）—— 将电池电压（mV）映射到 SOC（%） |
| **FRAM** | 铁电随机存取存储器 —— 用于持久化存储 SOC 的非易失存储器 |
| **BMS** | 电池管理系统（Battery Management System） |
| **NVM** | 非易失存储器（Non-Volatile Memory） |
| **BMS_AT_REST** | 电池系统静置状态，此时无显著电流流动 |
| **BS_NR_OF_STRINGS** | 系统中电池串的数量 |
| **String（串）** | 一组串联的电池单体或电池块 |

### 1.4 参考文献

| 编号 | 文档 / 文件 |
|------|-------------|
| [R1] | [state_estimation.h](../state_estimation.h) —— 公共 API 声明 |
| [R2] | [soc_counting.c](soc_counting.c) —— 模块实现 |
| [R3] | [soc_counting_cfg.h](soc_counting_cfg.h) —— 配置宏 |
| [R4] | [state_estimation.c](../state_estimation.c) —— 状态估算封装层 |
| [R5] | [database.h](../../../../engine/database/database.h) —— 数据库基础设施 |

---

## 2. 总体描述

### 2.1 产品视角

SOC 库仑计数模块是 foxBMS 2 中几种可互换的 SOC 算法实现之一。其他实现包括基于简单查表的方法
（[soc_lookup-table.c](../lookup-table/)）和调试变体（[soc_debug.c](../debug/)）。
该模块通过构建配置（wscript）在编译时选定。它由状态估算封装层
（[state_estimation.c](../state_estimation.c)）调用，封装层持有 SOC 数据库块
并调用库仑计数专用的函数。

### 2.2 工作模式

该模块根据电池系统状态以两种不同的模式运行：

1. **活跃模式（有电流流动）：** 通过积分测量电流随时间的变化（库仑计数），
   或通过读取硬件库仑计数器传感器的累积电荷来周期性更新 SOC。

2. **静置模式（无显著电流）：** 电池已充分静置，其开路电压（OCV）是 SOC 的可靠指标。
   在此模式下，通过电压-SOC 查表，使用最小、最大和平均电池电压进行 SOC 重校准。

### 2.3 用户特征

本模块供将 foxBMS 2 集成到电池系统中的固件开发人员使用。主要用户包括：

- BMS 应用开发人员，负责配置电池容量和电池串拓扑
- 测试工程师，负责验证 SOC 估算精度
- 集成工程师，负责在编译时选择合适的 SOC 算法

---

## 3. 功能需求

### 3.1 SOC 初始化

#### FR-3.1.1 — 上电时从非易失存储器初始化

**编号：** FR-3.1.1  
**优先级：** 高  
**函数：** `SE_InitializeStateOfCharge()`

**描述：**  
系统上电时，模块应将指定电池串的 SOC 值（平均值、最小值、最大值）、
充电吞吐量和放电吞吐量从 FRAM 非易失存储器恢复到提供的 SOC 数据库块中。

**输入：**

| 参数 | 类型 | 描述 |
|------|------|------|
| `pSocValues` | `DATA_BLOCK_SOC_s *` | 指向待填充的 SOC 数据库块的指针 |
| `ccPresent` | `bool` | 如果硬件库仑计数器传感器可用则为 `true`；否则为 `false` |
| `stringNumber` | `uint8_t` | 电池串索引 [0, `BS_NR_OF_STRINGS` - 1] |

**前置条件：**

- `pSocValues` 不得为 NULL
- `stringNumber` 必须小于 `BS_NR_OF_STRINGS`
- FRAM 中必须包含有效的 SOC 数据（由上一个运行周期写入）

**处理流程：**

1. 将电流表和电流计数器数据库表读入本地副本。
2. 通过 `FRAM_ReadData(FRAM_BLOCK_ID_SOC)` 从 FRAM 读取 SOC 数据。
3. 如果 `ccPresent` 为 `true`：
   - 设置 `sensorCcUsed[stringNumber] = true`
   - 将当前库仑计数器值存储为基准值
   - 通过将 FRAM SOC 值与基于 CC 的电荷偏移量结合，计算并存储 CC 缩放偏移量（平均值、最小值、最大值）
4. 如果 `ccPresent` 为 `false`：
   - 设置 `sensorCcUsed[stringNumber] = false`
   - 将当前原始电流时间戳存储为基准值
5. 将 FRAM SOC 值和吞吐量值复制到 `pSocValues` 中
6. 通过 `SOC_CheckDatabaseSocPercentageLimits()` 施加 SOC 百分比限值（0% 至 100%）
7. 设置 `socInitialized = true`

**后置条件：**

- `pSocValues->averageSoc_perc[stringNumber]` 包含恢复的平均 SOC
- `pSocValues->minimumSoc_perc[stringNumber]` 包含恢复的最小 SOC
- `pSocValues->maximumSoc_perc[stringNumber]` 包含恢复的最大 SOC
- `pSocValues->chargeThroughput_As[stringNumber]` 包含恢复的充电吞吐量
- `pSocValues->dischargeThroughput_As[stringNumber]` 包含恢复的放电吞吐量
- SOC 模块已标记为针对指定电池串已完成初始化
- 所有 SOC 值被钳位在 [0.0%, 100.0%] 范围内

**错误处理：**

- 如果 `pSocValues` 为 NULL，断言失败
- 如果 `stringNumber >= BS_NR_OF_STRINGS`，断言失败

---

### 3.2 SOC 计算

#### FR-3.2.1 — 周期性 SOC 更新

**编号：** FR-3.2.1  
**优先级：** 高  
**函数：** `SE_CalculateStateOfCharge()`

**描述：**  
模块应周期性计算所有电池串的更新 SOC 值。计算方法取决于电池系统状态
和可用的电流测量硬件。

**输入：**

| 参数 | 类型 | 描述 |
|------|------|------|
| `pSocValues` | `DATA_BLOCK_SOC_s *` | 指向待更新的 SOC 数据库块的指针 |

**前置条件：**

- `pSocValues` 不得为 NULL
- SOC 模块必须已完成初始化（`socInitialized == true`）

**处理流程 — 守卫检查：**

1. 断言 `pSocValues` 不为 NULL
2. 如果 `socInitialized == false`，立即返回，不做任何处理

**处理流程 — 静置重校准：**

3. 如果电池系统处于 `BMS_AT_REST` 状态：
   - 从数据库读取当前最小/最大/平均电池电压
   - 通过 `SE_GetStateOfChargeFromVoltage()` 将每个电压转换为 SOC 百分比
   - 通过 `SOC_SetValue()` 为每串电池设置转换后的 SOC 值
   - 将更新后的值写入 FRAM

**处理流程 — 活跃模式（非 CC 传感器）：**

4. 如果电池系统**未**静置，且电流传感器**不是**硬件库仑计数器
   （`sensorCcUsed[s] == false`）：
   - 如果当前测量时间戳相比上一次迭代未发生变化，跳过该电池串
   - 根据时间戳差值计算时间步长 Δt（秒）
   - 如果 Δt > 0：
     - 计算 ΔSOC = (电流 × Δt) / (电池串容量 mAs) × 100%
     - 施加电流方向符号因子
     - 从平均值、最小值、最大值 SOC 中减去 ΔSOC
       （在 BAT 约定下，充入电池 = 负电流 → SOC 增加 = 减去负的增量）
     - 计算转移电荷量（As）：`|电流 × Δt / 1000|`
     - 根据电流方向累积充电或放电吞吐量
     - 将 SOC 值钳位在 [0%, 100%] 范围内
     - 更新 NVM 影子副本
   - 存储当前时间戳供下一次迭代使用

**处理流程 — 活跃模式（CC 传感器）：**

5. 如果电池系统**未**静置，且使用了硬件库仑计数器
   （`sensorCcUsed[s] == true`）：
   - 如果 CC 计数器时间戳未发生变化，跳过该电池串
   - 计算 ΔSOC = (累积电荷 As) / (电池串容量 As) × 100%
   - 施加电流方向符号因子
   - 从各自的 CC 缩放基准值中减去 ΔSOC，设置平均值/最小值/最大值 SOC
   - 计算电荷差值的绝对值（As）用于吞吐量累积
   - 根据电流方向累积充电或放电吞吐量
   - 将 SOC 值钳位在 [0%, 100%] 范围内
   - 更新 NVM 影子副本
   - 存储当前 CC 值和时间戳供下一次迭代使用

6. 处理完所有电池串后，将 SOC 数据写入 FRAM

**后置条件：**

- 对于每个有新数据的电池串，`pSocValues` 中的 SOC 值已更新
- 充电和放电吞吐量值已累积
- 更新后的值已持久化保存至 FRAM

**错误处理：**

- 如果 `pSocValues` 为 NULL，断言失败
- 如果 `socInitialized == false`，函数立即返回（优雅空操作）

---

#### FR-3.2.2 — 库仑计数 SOC 公式

**编号：** FR-3.2.2  
**优先级：** 高

**描述：**  
对于原始电流传感器积分，SOC 增量应按如下公式计算：

```
ΔSOC [%] = (I [mA] × Δt [s]) / (SOC_STRING_CAPACITY_mAs) × 100
```

其中：
- `I` 是电池串电流，单位为 mA（在 BAT 约定下，正值 = 放电）
- `Δt` 是时间步长，单位为秒
- `SOC_STRING_CAPACITY_mAs` = `BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK × BC_CAPACITY_mAh × 3600`

新的 SOC 为：`SOC_new = SOC_old - (ΔSOC × BS_CURRENT_DIRECTION_FLOAT)`

对于硬件库仑计数器积分，公式为：

```
ΔSOC [%] = Q_accumulated [As] / SOC_STRING_CAPACITY_As × 100
```

SOC 从 CC 缩放基准重新导出：`SOC_new = ccScaling[X] - ΔSOC`

---

#### FR-3.2.3 — 充放电吞吐量累积

**编号：** FR-3.2.3  
**优先级：** 中

**描述：**  
模块应分别累积每串电池的总充电吞吐量（流入电池的库仑数）和
总放电吞吐量（流出电池的库仑数）。这些值以 As（安培秒）为单位存储，
并持久化保存至 FRAM。

**处理流程：**
- 如果电流流动方向为 `BMS_CHARGING`，该时段内转移的绝对电荷量
  累加到 `chargeThroughput_As`
- 否则（放电或静置），转移的绝对电荷量累加到 `dischargeThroughput_As`

> **设计说明：** 当电池处于静置状态（`BMS_AT_REST`）但系统未处于静置重校准路径时，
> 任何残余小电流被归类为放电吞吐量。这显然是有意的设计选择。

**后置条件：**
- `pSocValues->chargeThroughput_As[s]` 和
  `pSocValues->dischargeThroughput_As[s]` 单调递增
- 吞吐量值持久化保存至 FRAM

---

### 3.3 SOC 重校准

#### FR-3.3.1 — 静置时通过查表进行重校准

**编号：** FR-3.3.1  
**优先级：** 高  
**函数：** `SOC_RecalibrateViaLookupTable()`

**描述：**  
当电池系统进入静置状态时，模块应使用测量的电池电压重新校准 SOC。
最小、最大和平均电池电压分别通过电压-SOC 查表转换为 SOC 百分比，
每串电池得到三个重校准后的 SOC 值。

**输入：**
| 参数 | 类型 | 描述 |
|------|------|------|
| `pTableSoc` | `DATA_BLOCK_SOC_s *` | 指向待更新的 SOC 数据库块的指针 |

**前置条件：**
- `pTableSoc` 不得为 NULL
- 电池必须处于静置状态（OCV ≈ 电池电压）
- 电池电压查表（`bc_stateOfChargeLookupTable`）必须已配置有效的电压-SOC 数据对

**处理流程：**
1. 从数据库读取最小/最大/平均电池电压
2. 对于每串电池：
   - 通过 LUT 将最小电池电压映射为最小 SOC
   - 通过 LUT 将最大电池电压映射为最大 SOC
   - 通过 LUT 将平均电池电压映射为平均 SOC
   - 调用 `SOC_SetValue()` 应用这些值
3. 将所有值写入 FRAM

**后置条件：**
- SOC 值反映了所有电池串基于开路电压（OCV）的 SOC
- 如果使用 CC 传感器，CC 缩放基准已更新
- 所有值已持久化保存至 FRAM

---

### 3.4 电压-SOC 查表

#### FR-3.4.1 — LUT 插值

**编号：** FR-3.4.1  
**优先级：** 中  
**函数：** `SE_GetStateOfChargeFromVoltage()`

**描述：**  
给定一个以 mV 为单位的电池电压，模块应通过在预配置的查表中进行线性插值，
返回对应的 SOC 百分比。LUT 以电压降序存储电压-SOC 数据对。

**输入：**
| 参数 | 类型 | 描述 |
|------|------|------|
| `voltage_mV` | `int16_t` | 电池电压，单位为毫伏 |

**返回值：**
| 类型 | 描述 |
|------|------|
| `float_t` | SOC 百分比，范围为 [0.0, 100.0] |

**处理流程：**
1. 初始化 `soc_perc` 为 0.50（50%）
2. 扫描 LUT（从索引 1 开始，因为不希望对最高电压以上的情况进行外推），
   找到包围输入电压的一对 LUT 条目：
   - 如果 `voltage_mV < LUT[i].voltage_mV`：设置 `between_low = i + 1`，
     `between_high = i`
3. 确定插值情形：
   - **插值情形** — 如果找到有效包围区间（`between_high` 和 `between_low`
     均非零，且 `between_low` 在 LUT 范围内）：
     在两个包围的 LUT 点之间执行线性插值
   - **低于最小值情形** — 如果 `between_low >= LUT_length`（电池电压
     低于 LUT 最低电压）：返回 **0.0%**
   - **高于最大值情形** — 否则（电池电压超过 LUT 最高电压，
     即两个索引均为 0）：返回 **100.0%**

> **设计说明：** 第 417 行的 `else if` 分支检查
> `between_low >= bc_stateOfChargeLookupTableLength`，该分支同时处理
> "电池电压 < LUT 最低电压"的情形以及第 409-410 行插值守卫条件的下坠情形。
> 然而，"高于最大值"的逻辑存在一个微妙之处。当 `voltage_mV` 超过所有 LUT 电压时，
> 条件 `voltage_mV < LUT[i].voltage_mV` 对所有 `i` 从未成立（因为 LUT 电压降序排列），
> 因此 `between_high` 和 `between_low` 均保持为 0。
> 第 409 行的第一个守卫条件 `(between_high == 0u) && (between_low == 0u)` 捕获了这种情况，
> 第 420 行的最终 `else` 分支返回 100.0%。这依赖于 LUT 按**降序**排列电压。

**后置条件：**
- 返回值钳位在 [0.0, 100.0] 范围内
- 不进行 LUT 端点之外的外推

**错误条件：**
- 如果 LUT 为空或只有 1 个条目，行为取决于 LUT 长度
  （第 401 行的循环使用 `bc_stateOfChargeLookupTableLength`）

---

### 3.5 SOC 值管理

#### FR-3.5.1 — SOC 百分比范围限制

**编号：** FR-3.5.1  
**优先级：** 高  
**函数：** `SOC_CheckDatabaseSocPercentageLimits()`

**描述：**  
指定电池串的所有 SOC 值（平均值、最小值、最大值）应被钳位在 [0.0%, 100.0%] 范围内。
超过 100.0% 的值应设置为 100.0%；低于 0.0% 的值应设置为 0.0%。

**输入：**
| 参数 | 类型 | 描述 |
|------|------|------|
| `pTableSoc` | `DATA_BLOCK_SOC_s *` | 指向 SOC 数据库块的指针 |
| `stringNumber` | `uint8_t` | 电池串索引 |

**前置条件：**
- `pTableSoc` 不得为 NULL
- `stringNumber` 必须小于 `BS_NR_OF_STRINGS`

**处理流程：**
- 分别检查 `averageSoc_perc`、`minimumSoc_perc`、`maximumSoc_perc`
  中的每一个，对照 `SOC_MAXIMUM_SOC_perc`（100.0%）和
  `SOC_MINIMUM_SOC_perc`（0.0%）
- 将任何超出范围的值钳位到最近的限值

**后置条件：**
- 该电池串的三个 SOC 值均在 [0.0%, 100.0%] 范围内

---

#### FR-3.5.2 — 非易失存储器持久化

**编号：** FR-3.5.2  
**优先级：** 高  
**函数：** `SOC_UpdateNvmValues()`

**描述：**  
每当 SOC 或吞吐量值更新时，FRAM 影子结构（`fram_soc`）中相应的值也应更新。
这确保了下一次上电周期能够从非易失存储器中恢复最近的 SOC。

**处理流程：**
- 将指定电池串在数据库块中的 `averageSoc_perc`、`minimumSoc_perc`、
  `maximumSoc_perc`、`chargeThroughput_As` 和 `dischargeThroughput_As`
  复制到 `fram_soc` 结构中

**后置条件：**
- FRAM 影子副本与该电池串的活跃 SOC 数据库值保持一致
- 实际的 FRAM 写操作在调用方调用 `FRAM_WriteData(FRAM_BLOCK_ID_SOC)` 时执行

---

#### FR-3.5.3 — CC 缩放因子管理

**编号：** FR-3.5.3  
**优先级：** 中  
**函数：** `SOC_SetValue()`、`SE_InitializeStateOfCharge()`

**描述：**  
当使用硬件库仑计数器（CC）传感器时，模块为每串电池维护一组缩放因子
（平均值、最小值、最大值）。这些缩放因子将 FRAM 中持久化的 SOC 值与
基于 CC 的电荷偏移量结合，使后续的 SOC 更新可以从 CC 累积电荷相对于
缩放基准值推导得出。

**处理流程：**
1. 根据 CC 绝对累积电荷值计算以 SOC 百分比表示的 CC 偏移量
2. 施加电流方向符号因子
3. 对于 X ∈ {平均值, 最小值, 最大值}，存储 `ccScaling[X] = current_SOC[X] + ccOffset`

**后置条件：**
- 指定电池串的 `soc_state.ccScalingAverage[stringNumber]`、
  `soc_state.ccScalingMinimum[stringNumber]` 和
  `soc_state.ccScalingMaximum[stringNumber]` 已设置

---

### 3.6 电荷-SOC 转换

#### FR-3.6.1 — 电荷（As）转 SOC 百分比

**编号：** FR-3.6.1  
**优先级：** 低  
**函数：** `SOC_GetStringSocPercentageFromCharge()`

**描述：**  
将绝对电荷值（以安培秒 As 为单位）转换为相对于配置的电池串容量的 SOC 百分比。

**公式：**
```
SOC [%] = 100 × (Q [mAs] / SOC_STRING_CAPACITY_mAs)
        = 100 × (Q [As] × 1000) / SOC_STRING_CAPACITY_mAs
```

**输入：**
| 参数 | 类型 | 描述 |
|------|------|------|
| `charge_As` | `uint32_t` | 电荷量，单位为安培秒（无符号） |

**返回值：**
| 类型 | 描述 |
|------|------|
| `float_t` | 对应的 SOC 百分比 |

---

## 4. 非功能需求

### 4.1 时序与性能

#### NFR-4.1.1 — 周期性执行

**编号：** NFR-4.1.1  
**优先级：** 中

`SE_CalculateStateOfCharge()` 由状态估算任务通过
`SE_RunStateEstimations()` 周期性调用。模块应在分配的任务时间片内完成计算。
每串电池的计算量很小：
- 每串电池几个浮点乘除运算
- 每次调用一次 FRAM 写入（在所有电池串处理完毕后批量执行）
- 每次调用一次电流表/计数器表的数据库读取

#### NFR-4.1.2 — 无新数据时跳过

**编号：** NFR-4.1.2  
**优先级：** 中

当没有新的电流测量数据可用（时间戳未变化）时，模块应跳过该电池串的 SOC 更新。
这避免了不必要的计算和 FRAM 磨损。

---

### 4.2 内存

#### NFR-4.2.1 — 静态内存占用

**编号：** NFR-4.2.1  
**优先级：** 低

模块使用单个静态实例的 `SOC_STATE_s` 和两个数据库表
（`DATA_BLOCK_CURRENT_s`、`DATA_BLOCK_CURRENT_COUNTER_s`）的本地副本。
内存使用量随 `BS_NR_OF_STRINGS`（结构体内部的数组）线性增长。
不使用动态内存分配。

---

### 4.3 鲁棒性

#### NFR-4.3.1 — 基于断言的输入校验

**编号：** NFR-4.3.1  
**优先级：** 高

所有公开和内部函数应使用 `FAS_ASSERT()` 校验指针和索引参数。
空指针解引用和数组越界访问应通过断言来防止，断言在调试构建中会中止执行。

#### NFR-4.3.2 — 除零保护

**编号：** NFR-4.3.2  
**优先级：** 高

原始电流积分路径在执行 SOC 增量计算之前应验证 `timeStep_s > 0.0f`，
防止时间戳差值为零或负数（例如由于时间戳回绕）时的除零错误。

#### NFR-4.3.3 — 初始化前优雅守卫

**编号：** NFR-4.3.3  
**优先级：** 高

如果在 `SE_InitializeStateOfCharge()` 完成之前调用
`SE_CalculateStateOfCharge()`，函数应立即返回，不修改任何 SOC 值。

---

### 4.4 可配置性

#### NFR-4.4.1 — 电池串容量

**编号：** NFR-4.4.1  
**优先级：** 中

用于 SOC 计算的电池串容量应在编译时从以下参数推导得出：
- `BS_NR_OF_PARALLEL_CELLS_PER_CELL_BLOCK` —— 每个电池块的并联电池数
- `BC_CAPACITY_mAh` —— 电池标称容量，单位为 mAh

这产生了模块中使用的 `SOC_STRING_CAPACITY_mAh`、`SOC_STRING_CAPACITY_mAs`
和 `SOC_STRING_CAPACITY_As` 宏。

#### NFR-4.4.2 — 电流方向符号约定

**编号：** NFR-4.4.2  
**优先级：** 中

电流方向符号因子（`BS_CURRENT_DIRECTION_FLOAT`）应可配置，
以匹配电池包的电流传感器极性约定。该因子应用于所有 ΔSOC 计算和 CC 偏移量计算。

#### NFR-4.4.3 — LUT 配置

**编号：** NFR-4.4.3  
**优先级：** 中

电压-SOC 查表（`bc_stateOfChargeLookupTable`）应可配置特定于电池化学特性的
电压-SOC 数据对。该表必须按电压降序排列，以使 LUT 搜索算法正常工作。

---

### 4.5 可测试性

#### NFR-4.5.1 — 单元测试钩子

**编号：** NFR-4.5.1  
**优先级：** 低

当使用 `UNITY_UNIT_TEST` 编译时，内部静态函数
`SOC_CheckDatabaseSocPercentageLimits()` 和 `SOC_UpdateNvmValues()`
应通过测试包装函数（`TEST_SOC_*`）暴露，以实现直接的单元测试。

---

## 5. 接口需求

### 5.1 公共 API

| 函数 | 可见性 | 描述 |
|------|--------|------|
| `SE_InitializeStateOfCharge()` | 外部（API） | 上电时从 NVM 初始化 SOC |
| `SE_CalculateStateOfCharge()` | 外部（API） | 周期性 SOC 计算 |
| `SE_GetStateOfChargeFromVoltage()` | 外部（API） | 电压-SOC LUT 插值 |

### 5.2 依赖项

| 模块 | 依赖类型 | 用途 |
|------|----------|------|
| `database` | 必需 | 读写电流表、计数器表和 SOC 数据块 |
| `fram` | 必需 | SOC 和吞吐量值的非易失存储 |
| `foxmath` | 必需 | 线性插值函数（`MATH_LinearInterpolation`） |
| `bms` | 必需 | 电池系统状态（`BMS_GetBatterySystemState()`、`BMS_GetCurrentFlowDirection()`） |
| `state_estimation` | 必需 | 公共 API 声明 |
| `battery_cell_cfg` | 必需 | 电池容量（`BC_CAPACITY_mAh`） |
| `battery_system_cfg` | 必需 | 电池串拓扑、电流方向符号 |

### 5.3 调用方

| 调用方 | 调用的函数 | 上下文 |
|--------|------------|--------|
| `state_estimation.c` | `SE_InitializeStateOfCharge()` | 上电时通过 `SE_InitializeSoc()` 调用 |
| `state_estimation.c` | `SE_CalculateStateOfCharge()` | 周期性任务中通过 `SE_RunStateEstimations()` 调用 |
| `state_estimation.c`（及其他） | `SE_GetStateOfChargeFromVoltage()` | 电压-SOC 映射工具函数 |

---

## 6. 数据结构

### 6.1 SOC_STATE_s

模块内部状态，静态分配。

| 字段 | 类型 | 描述 |
|------|------|------|
| `socInitialized` | `bool` | 模块初始化完成标志 |
| `sensorCcUsed[]` | `bool[BS_NR_OF_STRINGS]` | 每串电池是否使用库仑计数器传感器 |
| `ccScalingAverage[]` | `float_t[BS_NR_OF_STRINGS]` | CC 缩放基准 —— 平均 SOC |
| `ccScalingMinimum[]` | `float_t[BS_NR_OF_STRINGS]` | CC 缩放基准 —— 最小 SOC |
| `ccScalingMaximum[]` | `float_t[BS_NR_OF_STRINGS]` | CC 缩放基准 —— 最大 SOC |
| `chargeThroughput_As[]` | `float_t[BS_NR_OF_STRINGS]` | 累积充电吞吐量（As） |
| `dischargeThroughput_As[]` | `float_t[BS_NR_OF_STRINGS]` | 累积放电吞吐量（As） |
| `previousCurrentCountingValue_As[]` | `float_t[BS_NR_OF_STRINGS]` | 上次 CC 累积值（As） |
| `previousTimestamp[]` | `uint32_t[BS_NR_OF_STRINGS]` | 上次测量时间戳（ms） |

---

## 7. 状态机 / 控制流

### 7.1 初始化状态

```
上电
  └─► SE_InitializeSoc() [封装层]
        └─► SE_InitializeStateOfCharge()
              ├─ 读取 FRAM SOC
              ├─ 如果 ccPresent：设置 CC 缩放基准
              ├─ 如果非 ccPresent：设置电流时间戳基准
              ├─ 将 FRAM 值复制 → pSocValues
              ├─ 钳位到 [0%, 100%]
              └─ 设置 socInitialized = true
```

### 7.2 运行时状态

```
SE_RunStateEstimations() [周期性]
  └─► SE_CalculateStateOfCharge()
        ├─ 守卫：socInitialized？
        │   └─ 否 → 返回
        ├─ 电池静置？
        │   └─ 是 → SOC_RecalibrateViaLookupTable()
        │           ├─ 读取最小/最大/平均电池电压
        │           ├─ SE_GetStateOfChargeFromVoltage() × 每串 3 次
        │           └─ 写入 FRAM
        └─ 否 → 对每串电池：
                  ├─ 时间戳是否变化？
                  │   └─ 否 → 下一串
                  ├─ CC 传感器？
                  │   ├─ 是 → 基于 CC 的 ΔSOC 计算
                  │   └─ 否 → 电流积分 ΔSOC 计算
                  ├─ 累积充放电吞吐量
                  ├─ 将 SOC 钳位到 [0%, 100%]
                  ├─ 更新 NVM 影子副本
                  └─ 更新时间戳 / CC 基准
```

---

## 8. 追溯矩阵

| 需求 | 函数 | 验证方法 |
|------|------|----------|
| FR-3.1.1 | `SE_InitializeStateOfCharge()` | 单元测试：验证 FRAM 值被正确恢复 |
| FR-3.2.1 | `SE_CalculateStateOfCharge()` | 单元测试 + 集成测试：验证已知电流下的 SOC 增量 |
| FR-3.2.2 | `SE_CalculateStateOfCharge()` | 单元测试：使用已知 I、Δt 验证 ΔSOC 公式 |
| FR-3.2.3 | `SE_CalculateStateOfCharge()` | 单元测试：验证吞吐量累积方向 |
| FR-3.3.1 | `SOC_RecalibrateViaLookupTable()` | 单元测试：Mock LUT，验证重校准后的 SOC |
| FR-3.4.1 | `SE_GetStateOfChargeFromVoltage()` | 单元测试：验证在 LUT 数据点上和数据点之间的插值 |
| FR-3.5.1 | `SOC_CheckDatabaseSocPercentageLimits()` | 单元测试：验证在限值处和超限值处的钳位行为 |
| FR-3.5.2 | `SOC_UpdateNvmValues()` | 单元测试：验证更新后 fram_soc 与数据库镜像一致 |
| FR-3.5.3 | `SOC_SetValue()`、`SE_InitializeStateOfCharge()` | 单元测试：验证 CC 缩放因子计算 |
| FR-3.6.1 | `SOC_GetStringSocPercentageFromCharge()` | 单元测试：验证电荷-SOC 转换 |
| NFR-4.3.2 | `SE_CalculateStateOfCharge()` | 单元测试：验证零/负 Δt 被妥善处理 |
| NFR-4.3.3 | `SE_CalculateStateOfCharge()` | 单元测试：验证未初始化时提前返回 |

---

*文档由 [soc_counting.c](soc_counting.c) v1.11.0 生成。*
*最后更新：2026-06-05*
