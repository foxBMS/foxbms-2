# STATE_ESTIMATION — 软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义了 foxBMS 2 状态估计（State Estimation）模块的软件需求规格。该模块是 SOC（State of Charge，荷电状态）、SOE（State of Energy，能量状态）和 SOH（State of Health，健康状态）估计算法的包装器（Wrapper），负责统一的初始化、周期性计算和数据库管理。

### 1.2 范围

- **涵盖**：SOC/SOE/SOH 的初始化包装、周期性计算调度、数据库读写管理、算法 API 接口定义
- **不涵盖**：具体的 SOC/SOE/SOH 估算算法实现（由各自子模块实现）

### 1.3 定义与缩略语

| 术语 | 说明 |
|------|------|
| SE | State Estimation，状态估计 |
| SOC | State of Charge，荷电状态（0% ~ 100%） |
| SOE | State of Energy，能量状态（0% ~ 100%） |
| SOH | State of Health，健康状态（0% ~ 100%） |

### 1.4 参考文献

- [state_estimation.c](state_estimation.c) — 状态估计包装器实现
- [state_estimation.h](state_estimation.h) — 状态估计头文件（含 API 接口定义）

## 2. 总体描述

### 2.1 产品视角

STATE_ESTIMATION 模块位于 foxBMS 2 的应用层，不实现具体的估计算法，而是作为不同算法实现（库仑计数、查表法、调试模式、空实现等）的统一调度入口。各子模块实现 `state_estimation.h` 中定义的 API 接口函数，由本模块统一调用。

### 2.2 工作模式

模块维护三个数据库表副本（SOC、SOE、SOH），在初始化阶段根据配置选择具体算法进行初始化，在运行阶段依次调用各算法进行计算，并将结果批量写回数据库。

### 2.3 用户特征

本模块的使用者是系统初始化流程（调用初始化函数）和算法调度框架（通过周期性任务调用 `SE_RunStateEstimations`）。

## 3. 功能需求

### 3.1 SOC 初始化与计算

#### REQ-001 — SOC 初始化包装

**编号** | **优先级** | **函数**
REQ-001 | 高 | `SE_InitializeSoc(bool ccPresent, uint8_t stringNumber)`

**描述**：系统应提供 SOC 初始化的包装函数，验证电池串编号有效性后，调用具体 SOC 算法的初始化函数 `SE_InitializeStateOfCharge()`，并将初始化结果写入数据库。

**前置条件**：`stringNumber < BS_NR_OF_STRINGS`
**后置条件**：SOC 数据库表已更新

#### REQ-002 — SOC 计算

**编号** | **优先级** | **函数**
REQ-002 | 高 | `SE_CalculateStateOfCharge(DATA_BLOCK_SOC_s *pSocValues)`

**描述**：系统应在 `state_estimation.h` 中声明 SOC 周期计算接口 `SE_CalculateStateOfCharge()`，由具体 SOC 算法子模块实现。该函数接收 SOC 数据库块指针，更新 SOC 估算值。

### 3.2 SOE 初始化与计算

#### REQ-003 — SOE 初始化包装

**编号** | **优先级** | **函数**
REQ-003 | 高 | `SE_InitializeSoe(bool ec_present, uint8_t stringNumber)`

**描述**：系统应提供 SOE 初始化的包装函数，验证电池串编号有效性后，调用具体 SOE 算法的初始化函数 `SE_InitializeStateOfEnergy()`，并将初始化结果写入数据库。

**前置条件**：`stringNumber < BS_NR_OF_STRINGS`
**后置条件**：SOE 数据库表已更新

#### REQ-004 — SOE 计算

**编号** | **优先级** | **函数**
REQ-004 | 高 | `SE_CalculateStateOfEnergy(DATA_BLOCK_SOE_s *pSoeValues)`

**描述**：系统应在 `state_estimation.h` 中声明 SOE 周期计算接口 `SE_CalculateStateOfEnergy()`，由具体 SOE 算法子模块实现。

### 3.3 SOH 初始化与计算

#### REQ-005 — SOH 初始化包装

**编号** | **优先级** | **函数**
REQ-005 | 高 | `SE_InitializeSoh(uint8_t stringNumber)`

**描述**：系统应提供 SOH 初始化的包装函数，验证电池串编号有效性后，调用具体 SOH 算法的初始化函数 `SE_InitializeStateOfHealth()`，并将初始化结果写入数据库。

**前置条件**：`stringNumber < BS_NR_OF_STRINGS`
**后置条件**：SOH 数据库表已更新

#### REQ-006 — SOH 计算

**编号** | **优先级** | **函数**
REQ-006 | 高 | `SE_CalculateStateOfHealth(DATA_BLOCK_SOH_s *pSohValues)`

**描述**：系统应在 `state_estimation.h` 中声明 SOH 周期计算接口 `SE_CalculateStateOfHealth()`，由具体 SOH 算法子模块实现。

### 3.4 统一调度执行

#### REQ-007 — 统一状态估计执行

**编号** | **优先级** | **函数**
REQ-007 | 高 | `SE_RunStateEstimations(void)`

**描述**：系统应提供 `SE_RunStateEstimations()` 函数，依次调用 SOC、SOE、SOH 的计算函数，并将三个数据块的更新结果一次性批量写回数据库。

**处理流程**：
1. 调用 `SE_CalculateStateOfCharge(&se_tableSocEstimation)`
2. 调用 `SE_CalculateStateOfEnergy(&se_tableSoeEstimation)`
3. 调用 `SE_CalculateStateOfHealth(&se_tableSohEstimation)`
4. 批量写入数据库：`DATA_WRITE_DATA(&soc, &soh, &soe)`

### 3.5 API 接口定义

#### REQ-008 — 状态估计 API 接口规范

**编号** | **优先级** | **结构/函数**
REQ-008 | 高 | `state_estimation.h` 中声明的 6 个 API 函数

**描述**：系统应在 `state_estimation.h` 中定义状态估计子模块必须实现的完整 API 接口，包括：
- `SE_InitializeStateOfCharge()` — SOC 初始化
- `SE_CalculateStateOfCharge()` — SOC 周期计算
- `SE_GetStateOfChargeFromVoltage()` — 电压→SOC 查表
- `SE_InitializeStateOfEnergy()` — SOE 初始化
- `SE_CalculateStateOfEnergy()` — SOE 周期计算
- `SE_InitializeStateOfHealth()` — SOH 初始化
- `SE_CalculateStateOfHealth()` — SOH 周期计算

## 4. 非功能需求

### 4.1 内存

#### REQ-009 — 静态数据库表副本

**编号** | **优先级**
REQ-009 | 中

**描述**：系统应使用静态局部变量维护 SOC（`DATA_BLOCK_SOC_s`）、SOE（`DATA_BLOCK_SOE_s`）、SOH（`DATA_BLOCK_SOH_s`）三个数据库表副本，避免每次调用时重新分配。

### 4.2 鲁棒性

#### REQ-010 — 参数有效性断言

**编号** | **优先级**
REQ-010 | 中

**描述**：系统应在所有初始化包装函数中使用 `FAS_ASSERT` 验证 `stringNumber < BS_NR_OF_STRINGS`，确保电池串索引不越界。

### 4.3 可扩展性

#### REQ-011 — 算法可替换性

**编号** | **优先级**
REQ-011 | 中

**描述**：系统应通过统一的 API 接口设计，支持在编译期替换不同的 SOC/SOE/SOH 算法实现（如计数法、查表法、调试模式、空实现），而无需修改本包装器模块的代码。

## 5. 接口需求

### 5.1 公共 API

| 函数 | 用途 | 调用方 |
|------|------|--------|
| `SE_InitializeSoc(bool, uint8_t)` | SOC 初始化包装 | 系统初始化流程 |
| `SE_InitializeSoe(bool, uint8_t)` | SOE 初始化包装 | 系统初始化流程 |
| `SE_InitializeSoh(uint8_t)` | SOH 初始化包装 | 系统初始化流程 |
| `SE_RunStateEstimations(void)` | 统一调度执行 | 算法调度框架 |

### 5.2 子模块 API（由各算法实现）

| 函数 | 用途 |
|------|------|
| `SE_InitializeStateOfCharge()` | SOC 初始化实现 |
| `SE_CalculateStateOfCharge()` | SOC 周期计算实现 |
| `SE_GetStateOfChargeFromVoltage()` | 电压→SOC 查表 |
| `SE_InitializeStateOfEnergy()` | SOE 初始化实现 |
| `SE_CalculateStateOfEnergy()` | SOE 周期计算实现 |
| `SE_InitializeStateOfHealth()` | SOH 初始化实现 |
| `SE_CalculateStateOfHealth()` | SOH 周期计算实现 |

### 5.3 依赖项

| 依赖模块 | 用途 |
|----------|------|
| `database.h` | 数据库读写（`DATA_WRITE_DATA`） |
| `fassert.h` | 断言检查（`FAS_ASSERT`） |

## 6. 数据结构

### 6.1 静态数据库副本

| 变量 | 类型 | 数据库 ID |
|------|------|-----------|
| `se_tableSocEstimation` | `DATA_BLOCK_SOC_s` | `DATA_BLOCK_ID_SOC` |
| `se_tableSohEstimation` | `DATA_BLOCK_SOH_s` | `DATA_BLOCK_ID_SOH` |
| `se_tableSoeEstimation` | `DATA_BLOCK_SOE_s` | `DATA_BLOCK_ID_SOE` |

## 7. 控制流

```
SE_InitializeSoc / SE_InitializeSoe / SE_InitializeSoh
    │
    ├── FAS_ASSERT(stringNumber < BS_NR_OF_STRINGS)
    ├── 调用具体算法的初始化函数
    └── DATA_WRITE_DATA() → 持久化到数据库

SE_RunStateEstimations()
    │
    ├── SE_CalculateStateOfCharge(&soc)
    ├── SE_CalculateStateOfEnergy(&soe)
    ├── SE_CalculateStateOfHealth(&soh)
    └── DATA_WRITE_DATA(&soc, &soh, &soe) → 批量写回数据库
```

## 8. 追溯矩阵

| 需求编号 | 需求描述 | 函数 | 文件 |
|----------|---------|------|------|
| REQ-001 | SOC 初始化包装 | `SE_InitializeSoc()` | [state_estimation.c](state_estimation.c) |
| REQ-002 | SOC 计算接口 | `SE_CalculateStateOfCharge()` | [state_estimation.h](state_estimation.h) |
| REQ-003 | SOE 初始化包装 | `SE_InitializeSoe()` | [state_estimation.c](state_estimation.c) |
| REQ-004 | SOE 计算接口 | `SE_CalculateStateOfEnergy()` | [state_estimation.h](state_estimation.h) |
| REQ-005 | SOH 初始化包装 | `SE_InitializeSoh()` | [state_estimation.c](state_estimation.c) |
| REQ-006 | SOH 计算接口 | `SE_CalculateStateOfHealth()` | [state_estimation.h](state_estimation.h) |
| REQ-007 | 统一状态估计执行 | `SE_RunStateEstimations()` | [state_estimation.c](state_estimation.c) |
| REQ-008 | API 接口规范 | 6 个 API 函数声明 | [state_estimation.h](state_estimation.h) |
| REQ-009 | 静态数据库表副本 | `se_tableSocEstimation` 等 | [state_estimation.c](state_estimation.c) |
| REQ-010 | 参数有效性断言 | `FAS_ASSERT` | [state_estimation.c](state_estimation.c) |
| REQ-011 | 算法可替换性 | API 接口设计 | [state_estimation.h](state_estimation.h) |
