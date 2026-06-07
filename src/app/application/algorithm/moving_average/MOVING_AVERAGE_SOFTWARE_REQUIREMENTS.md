# MOVING_AVERAGE — 软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 移动平均（Moving Average）算法模块的软件需求规格。该模块实现了对电池包总电流和总功率的滑动窗口移动平均计算，支持多种时间窗口（1秒、5秒、10秒、30秒、60秒及可配置时长）。

### 1.2 范围

- **涵盖**：电流移动平均计算、功率移动平均计算、多时间窗口管理、环形缓冲区管理、数据有效性校验、数据库读写
- **不涵盖**：电流/功率原始数据的采集（由 ISA 传感器模块负责）

### 1.3 定义与缩略语

| 术语 | 说明 |
|------|------|
| Moving Average | 移动平均/滑动平均，一种时间序列平滑算法 |
| Circular Buffer | 环形缓冲区，用于存储历史数据点的 FIFO 队列 |
| ISA | Insulation Monitoring & Sensor Acquisition，电流/功率数据来源 |

### 1.4 参考文献

- [moving_average.c](moving_average.c) — 移动平均算法实现
- [moving_average.h](moving_average.h) — 移动平均算法头文件

## 2. 总体描述

### 2.1 产品视角

MOVING_AVERAGE 模块是 foxBMS 2 算法框架中的一个具体算法实现，通过 `ALGO_MovingAverage()` 函数注册到算法调度框架中。该模块周期性地从数据库读取电流和功率数据，使用滑动窗口移动平均算法计算多种时间跨度的平均值，并将结果写回数据库。

### 2.2 工作模式

模块通过检测数据库中 `newCurrent` 和 `newPower` 计数器的变化来判断是否有新数据。当检测到新数据时，将数据加入环形缓冲区，使用增量式移动平均算法更新各时间窗口的平均值。窗口填充阶段（初始阶段）仅累加新值，窗口填满后同时加入新值和减去最旧值。

### 2.3 用户特征

本模块的结果供其他 BMS 模块（如 SOC 估算、SOE 估算等）通过数据库读取使用。

## 3. 功能需求

### 3.1 电流移动平均计算

#### REQ-001 — 多时间窗口电流移动平均

**编号** | **优先级** | **函数**
REQ-001 | 高 | `ALGO_MovingAverage()`

**描述**：系统应计算电池包总电流在 6 种时间窗口下的移动平均值：1秒（`movingAverageCurrent1sInterval_mA`）、5秒、10秒、30秒、60秒和可配置时长（`movingAverageCurrentConfigurableInterval_mA`，默认 3 秒）。

**处理流程**：
1. 检测电流数据更新（`curCounter != algo_tableCurrent.newCurrent`）
2. 汇总所有电池串的电流值得到总电流
3. 将新电流值写入环形缓冲区
4. 对各时间窗口：新值加入平均（`+= newValue / windowSize`）
5. 若窗口已填满：减去最旧值（`-= oldestValue / windowSize`）
6. 推进各窗口指针

#### REQ-002 — 功率移动平均计算

**编号** | **优先级** | **函数**
REQ-002 | 高 | `ALGO_MovingAverage()`

**描述**：系统应计算电池包总功率在 6 种时间窗口下的移动平均值：1秒（`movingAveragePower1sInterval_mA`）、5秒、10秒、30秒、60秒和可配置时长（`movingAveragePowerConfigurableInterval_mA`，默认 3 秒）。计算逻辑与电流移动平均对称。

### 3.2 数据有效性校验

#### REQ-003 — 测量数据有效性检查

**编号** | **优先级** | **函数**
REQ-003 | 高 | `ALGO_MovingAverage()`

**描述**：系统应在计算移动平均前检查所有电池串的电流/功率测量是否有效（`invalidMeasurement[s] == 0`）。任一电池串存在无效测量时，该周期的数据不纳入移动平均计算。

**处理流程**：
1. 遍历所有电池串（`0` 到 `BS_NR_OF_STRINGS - 1`）
2. 若 `invalidMeasurement[s] != 0`，设置 `validValues = false`
3. 仅当 `validValues == true` 时才进行移动平均计算

### 3.3 环形缓冲区管理

#### REQ-004 — 环形缓冲区存储

**编号** | **优先级** | **函数**
REQ-004 | 高 | `ALGO_MovingAverage()`

**描述**：系统应使用外部 SDRAM 中的静态数组作为环形缓冲区，存储电流和功率的历史数据。数组长度取 60 秒窗口大小和可配置窗口大小中的较大值，再加 1。

**处理流程**：
- 电流缓冲区 `curValues[]` 长度 = `max(60000/周期, 可配置时长/周期) + 1`
- 功率缓冲区 `powValues[]` 长度 = `max(60000/周期, 可配置时长/周期) + 1`
- 周期取 `ALGO_TICK_ms` 和 ISA 传感器周期中的较大值

#### REQ-005 — 指针回绕（Overflow Protection）

**编号** | **优先级** | **函数**
REQ-005 | 中 | `ALGO_MovingAverage()`

**描述**：系统应在每次指针递增后检查是否超出缓冲区末尾，若超出则回绕到缓冲区起始位置，确保环形缓冲区的正确操作。

**处理流程**：对 7 个电流指针（`pNew`, `_1s`, `_5s`, `_10s`, `_30s`, `_60s`, `_cfg`）和 7 个功率指针分别执行回绕检查。

### 3.4 窗口初始化

#### REQ-006 — 窗口填充阶段管理

**编号** | **优先级** | **函数**
REQ-006 | 中 | `ALGO_MovingAverage()`

**描述**：系统应在各时间窗口填满数据后才开始减去最旧值。窗口填充状态通过位标志（`curInit` / `powInit`）管理：bit0=1s, bit1=5s, bit2=10s, bit3=30s, bit4=60s, bit5=cfg。窗口未填满时仅累加新值。

**处理流程**：
1. 检查对应位标志是否置位
2. 若已置位：正常执行加减法
3. 若未置位：检查指针是否到达窗口边界，到达则置位

### 3.5 数据库交互

#### REQ-007 — 数据库读写

**编号** | **优先级** | **函数**
REQ-007 | 高 | `ALGO_MovingAverage()`

**描述**：系统应在每次执行开始时从数据库读取电流数据块（`DATA_BLOCK_ID_CURRENT`）、功率数据块（`DATA_BLOCK_ID_POWER`）和移动平均数据块（`DATA_BLOCK_ID_MOVING_AVERAGE`）。仅当有新的有效数据被处理（`newValues == 1`）时才将移动平均结果写回数据库。

### 3.6 配置参数

#### REQ-008 — 窗口大小配置

**编号** | **优先级** | **宏定义**
REQ-008 | 中 | `MOVING_AVERAGE_DURATION_CURRENT_CONFIG_ms`, `MOVING_AVERAGE_DURATION_POWER_CONFIG_ms`

**描述**：系统应提供可配置的电流和功率移动平均窗口时长（默认均为 3000ms），以及 ISA 传感器数据周期配置（`ISA_CURRENT_CYCLE_TIME_ms`、`ISA_POWER_CYCLE_TIME_ms`，默认均为 200ms）。

#### REQ-009 — 窗口样本数量计算

**编号** | **优先级** | **宏定义**
REQ-009 | 中 | `ALGO_NUMBER_AVERAGE_VALUES_CUR_*`, `ALGO_NUMBER_AVERAGE_VALUES_POW_*`

**描述**：系统应在编译期根据算法 Tick 周期和 ISA 传感器周期中较大的值，计算各时间窗口所需的样本数量。例如：1秒窗口样本数 = 1000ms / max(ALGO_TICK_ms, ISA周期)。

## 4. 非功能需求

### 4.1 时序与性能

#### REQ-010 — 增量式计算

**编号** | **优先级**
REQ-010 | 高

**描述**：系统应使用增量式移动平均算法（加入新值/除数、减去最旧值/除数），每个新数据点仅需 O(1) 时间复杂度，而非 O(N) 的全窗口重新求和。

### 4.2 内存

#### REQ-011 — 外部 SDRAM 存储

**编号** | **优先级**
REQ-011 | 中

**描述**：系统应将环形缓冲区分配在外部 SDRAM（`MEM_EXT_SDRAM`）中，以减少内部 SRAM 占用。电流和功率各一个缓冲区，最大长度由配置决定。

### 4.3 鲁棒性

#### REQ-012 — 无效数据隔离

**编号** | **优先级**
REQ-012 | 中

**描述**：当存在无效测量时，该周期的数据完全不参与计算，但已有的移动平均值保持不变。

## 5. 接口需求

### 5.1 公共 API

| 函数 | 用途 | 调用方 |
|------|------|--------|
| `ALGO_MovingAverage()` | 移动平均计算主函数 | 算法调度框架（`ALGO_MainFunction`） |

### 5.2 依赖项

| 依赖模块 | 用途 |
|----------|------|
| `database.h` | 读取电流/功率数据，写入移动平均结果 |
| `algorithm_cfg.h` | 算法调度框架集成 |
| `math.h` | 浮点数运算 |

### 5.3 调用方

- 算法调度框架（通过 `algo_algorithms[]` 注册）

## 6. 数据结构

### 6.1 环形缓冲区配置

| 缓冲区 | 元素类型 | 最大长度 | 存储位置 |
|--------|---------|---------|----------|
| `curValues[]` | `float_t` | `max(60s, cfg) / period + 1` | 外部 SDRAM |
| `powValues[]` | `float_t` | `max(60s, cfg) / period + 1` | 外部 SDRAM |

### 6.2 窗口填充标志

| 标志 | bit0 | bit1 | bit2 | bit3 | bit4 | bit5 |
|------|------|------|------|------|------|------|
| `curInit` | 1s | 5s | 10s | 30s | 60s | cfg |
| `powInit` | 1s | 5s | 10s | 30s | 60s | cfg |

## 7. 控制流

```
ALGO_MovingAverage() 被调用
    │
    ├── 从数据库读取电流、功率、移动平均数据块
    │
    ├── 电流数据处理
    │   ├── 检测新数据 (curCounter vs newCurrent)
    │   ├── 检查所有电池串测量有效性
    │   ├── 汇总所有电池串电流值
    │   ├── 写入环形缓冲区
    │   ├── 更新 6 个时间窗口的移动平均值
    │   └── 检查所有指针回绕
    │
    ├── 功率数据处理（与电流对称）
    │
    └── 若有新数据 → 将移动平均结果写回数据库
```

## 8. 追溯矩阵

| 需求编号 | 需求描述 | 函数/宏 | 文件 |
|----------|---------|--------|------|
| REQ-001 | 多时间窗口电流移动平均 | `ALGO_MovingAverage()` | [moving_average.c](moving_average.c) |
| REQ-002 | 功率移动平均计算 | `ALGO_MovingAverage()` | [moving_average.c](moving_average.c) |
| REQ-003 | 测量数据有效性检查 | `ALGO_MovingAverage()` | [moving_average.c](moving_average.c) |
| REQ-004 | 环形缓冲区存储 | `curValues[]`, `powValues[]` | [moving_average.c](moving_average.c) |
| REQ-005 | 指针回绕 | `ALGO_MovingAverage()` | [moving_average.c](moving_average.c) |
| REQ-006 | 窗口填充阶段管理 | `curInit`, `powInit` | [moving_average.c](moving_average.c) |
| REQ-007 | 数据库读写 | `DATA_READ_DATA()`, `DATA_WRITE_DATA()` | [moving_average.c](moving_average.c) |
| REQ-008 | 窗口大小配置 | `MOVING_AVERAGE_DURATION_*` | [moving_average.h](moving_average.h) |
| REQ-009 | 窗口样本数量计算 | `ALGO_NUMBER_AVERAGE_VALUES_*` | [moving_average.c](moving_average.c) |
| REQ-010 | 增量式计算 | `ALGO_MovingAverage()` | [moving_average.c](moving_average.c) |
| REQ-011 | 外部 SDRAM 存储 | `MEM_EXT_SDRAM` | [moving_average.c](moving_average.c) |
| REQ-012 | 无效数据隔离 | `ALGO_MovingAverage()` | [moving_average.c](moving_average.c) |
