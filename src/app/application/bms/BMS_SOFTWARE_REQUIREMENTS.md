# BMS 驱动程序软件需求文档

**版本**: v1.11.0  
**作者**: foxBMS Team  
**日期**: 2026-04-20  
**文件**: `bms.c`  
**模块**: ENGINE  
**前缀**: BMS

---

## 1. 概述

### 1.1 文件描述
本文件实现了BMS（电池管理系统）驱动程序，包含控制BMS的状态机和相关业务逻辑。

### 1.2 主要功能
- 实现BMS状态机控制
- 电池系统状态监测与管理
- 接触器（Contactor）控制与反馈验证
- 电池预充电过程监控
- 多个电池字符串（String）管理
- 故障诊断与处理
- 开路检测（Open Wire Detection）

---

## 2. 功能需求

### 2.1 状态机管理

#### 2.1.1 状态请求处理
- **REQ-001**: 系统应能检查并验证状态请求的有效性
- **REQ-002**: 系统应将经过验证的状态请求转移到状态机中
- **REQ-003**: 系统应支持从数据库读取状态请求

#### 2.1.2 状态转换
- **REQ-004**: 系统应在不同BMS状态之间进行转换
- **REQ-005**: 系统应跟踪前一个状态和子状态
- **REQ-006**: 系统应支持状态的进入、保持和退出子状态

### 2.2 电池系统监测

#### 2.2.1 故障检测
- **REQ-007**: 系统应监测诊断模块中所有严重级别为 `DIAG_FATAL_ERROR` 的错误标志
- **REQ-008**: 系统应在检测到致命错误时设置 `minimumActiveDelay_ms` 参数
- **REQ-009**: 系统应在多个错误同时激活时使用最短延迟时间
- **REQ-010**: 系统应检测电池系统状态是否正常，并在延迟时间到期时触发接触器打开

#### 2.2.2 测量值获取
- **REQ-011**: 系统应从数据库中获取最新的测量值
- **REQ-012**: 系统应维护本地的数据库表副本，包括：
  - 最小/最大值表（MIN_MAX）
  - 开路检测表（OPEN_WIRE）
  - 电池包值表（PACK_VALUES）

#### 2.2.3 开路检测
- **REQ-013**: 系统应检测任何电压感知线路的开路状况
- **REQ-014**: 系统应在检测到开路时记录故障诊断信息

### 2.3 接触器管理

#### 2.3.1 反馈验证
- **REQ-015**: 系统应验证特定接触器的反馈有效性
- **REQ-016**: 系统应从错误标志数据库条目中读取接触器反馈状态
- **REQ-017**: 系统应支持检查指定字符串和接触器类型的反馈

#### 2.3.2 接触器控制
- **REQ-018**: 系统应追踪电源路径状态（打开/闭合）
- **REQ-019**: 系统应管理多个电池字符串的接触器状态
- **REQ-020**: 系统应记录需要打开的接触器及其对应的字符串

### 2.4 预充电过程

#### 2.4.1 预充电监控
- **REQ-021**: 系统应监控预充电过程并检测其成功/失败状态
- **REQ-022**: 系统应支持预充电超时配置
- **REQ-023**: 系统应支持预充电重试计数
- **REQ-024**: 系统应返回预充电过程的以下状态：
  - `BMS_PRECHARGING_SUCCESSFUL` - 预充电成功
  - `BMS_PRECHARGING_ONGOING` - 预充电进行中
  - `BMS_PRECHARGING_FAILED` - 预充电失败

#### 2.4.2 电流/电压限制检查
- **REQ-025**: 系统应检查预充电电流是否低于配置的限制值
- **REQ-026**: 系统应检查电池电压与DC链路电压之间的差异是否低于限制值
- **REQ-027**: 系统应在检查失败时返回 `STD_NOT_OK`

### 2.5 电池字符串管理

#### 2.5.1 字符串选择策略
- **REQ-028**: 系统应能识别具有最高总电压的字符串
  - 用于启动放电模式时选择首个要闭合的字符串
  - 支持考虑或忽略预充电可用性

- **REQ-029**: 系统应能识别与首个闭合字符串电压最接近的字符串
  - 用于在驾驶过程中闭合其他字符串
  - 支持考虑或忽略预充电可用性

- **REQ-030**: 系统应能识别具有最低总电压的字符串
  - 用于启动充电模式时选择首个要闭合的字符串
  - 支持考虑或忽略预充电可用性

- **REQ-031**: 系统应计算首个闭合字符串与其他字符串之间的电压差

#### 2.5.2 字符串状态追踪
- **REQ-032**: 系统应记录哪些字符串处于闭合状态
- **REQ-033**: 系统应记录哪些字符串的预充电接触器处于闭合状态
- **REQ-034**: 系统应记录已关闭的字符串数量
- **REQ-035**: 系统应记录已停用的字符串
- **REQ-036**: 系统应记录首个闭合字符串的标识符

#### 2.5.3 字符串开关超时
- **REQ-037**: 系统应为字符串打开操作设置超时
- **REQ-038**: 系统应为字符串关闭操作设置超时
- **REQ-039**: 系统应为下一个字符串闭合设置计时器

### 2.6 其他功能

#### 2.6.1 重进入检查
- **REQ-040**: 系统应检查状态机触发函数的重进入情况
- **REQ-041**: 系统应在重进入时返回错误状态（0xff）
- **REQ-042**: 状态机触发函数不应该被多个不同进程同时调用

#### 2.6.2 CAN请求处理
- **REQ-043**: 系统应从数据库中检查CAN总线请求
- **REQ-044**: 系统应将CAN请求转换为状态请求

#### 2.6.3 时间管理
- **REQ-045**: 系统应维护当前系统计时器值
- **REQ-046**: 系统应支持配置松弛期（Relaxation Period）
- **REQ-047**: 系统应记录当前的流动状态（`BMS_RELAXATION` 或 `BMS_ACTIVE`）
- **REQ-048**: 系统应追踪接触器断路电流超出的时间

---

## 3. 性能需求

### 3.1 定时要求
- **REQ-049**: 状态机应支持时间触发或事件触发模式
- **REQ-050**: 系统应提供 10ms 级的时间分辨率（通过 `restTimer_10ms` 和相关超时）
- **REQ-051**: 系统应支持配置单位为毫秒的延迟时间

### 3.2 资源需求
- **REQ-052**: 系统应使用静态变量存储BMS状态
- **REQ-053**: 系统应维护本地数据库表副本以提高访问效率
- **REQ-054**: 系统应支持多个电池字符串（最多 `BS_NR_OF_STRINGS` 个）

### 3.3 可靠性需求
- **REQ-055**: 系统应设置允许的错误请求计数器
- **REQ-056**: 系统应在初始化完成后设置初始化标志
- **REQ-057**: 系统应支持振荡超时检测机制

---

## 4. 接口需求

### 4.1 依赖模块
系统依赖以下模块的接口：
- **AFE** - 模拟前端（Analog Front End）
- **BAL** - 平衡模块（Balance）
- **CAN** - CAN总线通信
- **DATABASE** - 数据库模块
- **DIAG** - 诊断模块
- **IMD** - 隔离故障检测（Insulation Monitoring Device）
- **LED** - LED控制
- **MEAS** - 测量模块
- **OS** - 操作系统
- **SOA** - 操作区域（State of Array）
- **SPS** - 电源开关（Smart Power Switch）

### 4.2 数据库表
- `DATA_BLOCK_MIN_MAX_s` - 最小/最大值数据块
- `DATA_BLOCK_OPEN_WIRE_s` - 开路检测数据块
- `DATA_BLOCK_PACK_VALUES_s` - 电池包值数据块

### 4.3 状态定义
- `BMS_STATE_REQUEST_e` - 状态请求枚举
- `BMS_STATE_s` - BMS状态结构体
- `BMS_RETURN_TYPE_e` - 返回类型枚举
- `BMS_RESULT_PRECHARGE_PROCESS_e` - 预充电过程结果枚举

---

## 5. 静态函数列表

| 函数名称 | 功能描述 |
|---------|---------|
| `BMS_CheckStateRequest()` | 检查状态请求的有效性 |
| `BMS_TransferStateRequest()` | 转移状态请求到状态机 |
| `BMS_CheckReEntrance()` | 检查重进入情况 |
| `BMS_CheckCanRequests()` | 检查CAN请求 |
| `BMS_IsAnyFatalErrorFlagSet()` | 检查任何致命错误标志 |
| `BMS_IsBatterySystemStateOkay()` | 检查电池系统状态 |
| `BMS_IsContactorFeedbackValid()` | 验证接触器反馈 |
| `BMS_GetMeasurementValues()` | 获取测量值 |
| `BMS_CheckOpenSenseWire()` | 检查开路感知线 |
| `BMS_MonitorPrechargeProcess()` | 监控预充电过程 |
| `BMS_IsPrechargeCurrentBelowLimit()` | 检查预充电电流 |
| `BMS_IsPrechargeVoltageBelowLimit()` | 检查预充电电压 |
| `BMS_GetHighestString()` | 获取最高电压字符串 |
| `BMS_GetClosestString()` | 获取最接近电压字符串 |
| `BMS_GetLowestString()` | 获取最低电压字符串 |

---

## 6. 宏定义

| 宏名称 | 描述 |
|-------|------|
| `BMS_NO_ACTIVE_DELAY_TIME_ms` | 未设置的活跃延迟时间的默认值（UINT32_MAX） |
| `BMS_SAVE_LAST_STATES()` | 保存最后状态和子状态的宏 |

---

## 7. 全局变量

### 7.1 BMS状态结构
- `bms_state` (BMS_STATE_s) - 包含BMS状态机的所有状态信息

### 7.2 数据库表副本
- `bms_tableMinMax` - 最小/最大值表副本
- `bms_tableOpenWire` - 开路检测表副本
- `bms_tablePackValues` - 电池包值表副本

---

## 8. 许可证

本文件遵循 BSD-3-Clause 许可证。

```
SPDX-License-Identifier: BSD-3-Clause

Copyright © 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der 
angewandten Forschung e.V. All rights reserved.
```

---

## 9. 版本历史

| 版本 | 日期 | 描述 |
|------|------|------|
| v1.11.0 | 2026-04-20 | 最后更新日期 |
| v1.0.0 | 2020-02-24 | 创建日期 |

---

## 10. 需求追溯矩阵

下表显示了软件需求与代码实现的关联关系：

| 需求编号 | 需求描述 | 实现位置 | 状态 |
|---------|---------|--------|------|
| REQ-001 | 检查状态请求有效性 | `BMS_CheckStateRequest()` | ✓ 已实现 |
| REQ-002 | 转移状态请求到状态机 | `BMS_TransferStateRequest()` | ✓ 已实现 |
| REQ-003 | 从数据库读取状态请求 | 用户信息中处理 | ✓ 已实现 |
| REQ-004 | 状态机状态转换 | `BMS_STATE_s` 结构体 | ✓ 已实现 |
| REQ-005 | 跟踪前一个状态和子状态 | `BMS_SAVE_LAST_STATES()` 宏、`lastState`、`lastSubstate` | ✓ 已实现 |
| REQ-006 | 支持状态进入、保持、退出子状态 | `BMS_STATE_s.substate` | ✓ 已实现 |
| REQ-007 | 监测 DIAG_FATAL_ERROR 级错误 | `BMS_IsAnyFatalErrorFlagSet()` | ✓ 已实现 |
| REQ-008 | 设置 minimumActiveDelay_ms 参数 | `BMS_IsAnyFatalErrorFlagSet()` | ✓ 已实现 |
| REQ-009 | 多个错误时使用最短延迟 | `BMS_IsAnyFatalErrorFlagSet()` | ✓ 已实现 |
| REQ-010 | 检测电池系统状态和触发接触器打开 | `BMS_IsBatterySystemStateOkay()` | ✓ 已实现 |
| REQ-011 | 从数据库获取最新测量值 | `BMS_GetMeasurementValues()` | ✓ 已实现 |
| REQ-012 | 维护本地数据库表副本 | `bms_tableMinMax`、`bms_tableOpenWire`、`bms_tablePackValues` | ✓ 已实现 |
| REQ-013 | 检测任何开路感知线 | `BMS_CheckOpenSenseWire()` | ✓ 已实现 |
| REQ-014 | 记录开路检测诊断信息 | `BMS_CheckOpenSenseWire()` 中的 `DIAG_Handler()` 调用 | ✓ 已实现 |
| REQ-015 | 验证接触器反馈有效性 | `BMS_IsContactorFeedbackValid()` | ✓ 已实现 |
| REQ-016 | 从错误标志数据库读取反馈状态 | `BMS_IsContactorFeedbackValid()` | ✓ 已实现 |
| REQ-017 | 支持指定字符串和接触器类型的检查 | `BMS_IsContactorFeedbackValid(stringNumber, contactorType)` | ✓ 已实现 |
| REQ-018 | 追踪电源路径状态 | `BMS_STATE_s.powerPath` | ✓ 已实现 |
| REQ-019 | 管理多个电池字符串的接触器状态 | `BMS_STATE_s.closedStrings`、`closedPrechargeContactors` | ✓ 已实现 |
| REQ-020 | 记录需要打开的接触器 | `BMS_GetFirstContactorToBeOpened()`、`BMS_GetSecondContactorToBeOpened()` | ✓ 已实现 |
| REQ-021 | 监控预充电过程检测成功/失败 | `BMS_MonitorPrechargeProcess()` | ✓ 已实现 |
| REQ-022 | 支持预充电超时配置 | `BMS_MonitorPrechargeProcess(timeout_ms)` | ✓ 已实现 |
| REQ-023 | 支持预充电重试计数 | `BMS_STATE_s.prechargeTryCounter` | ✓ 已实现 |
| REQ-024 | 返回预充电过程状态 | `BMS_RESULT_PRECHARGE_PROCESS_e` 枚举 | ✓ 已实现 |
| REQ-025 | 检查预充电电流是否低于限制 | `BMS_IsPrechargeCurrentBelowLimit()` | ✓ 已实现 |
| REQ-026 | 检查电压差是否低于限制 | `BMS_IsPrechargeVoltageBelowLimit()` | ✓ 已实现 |
| REQ-027 | 检查失败时返回 STD_NOT_OK | `BMS_IsPrechargeCurrentBelowLimit()`、`BMS_IsPrechargeVoltageBelowLimit()` | ✓ 已实现 |
| REQ-028 | 识别最高电压字符串 | `BMS_GetHighestString()` | ✓ 已实现 |
| REQ-029 | 识别最接近电压字符串 | `BMS_GetClosestString()` | ✓ 已实现 |
| REQ-030 | 识别最低电压字符串 | `BMS_GetLowestString()` | ✓ 已实现 |
| REQ-031 | 计算字符串电压差 | `BMS_GetStringVoltageDifference()` | ✓ 已实现 |
| REQ-032 | 记录哪些字符串处于闭合状态 | `BMS_STATE_s.closedStrings` | ✓ 已实现 |
| REQ-033 | 记录哪些字符串预充电接触器闭合 | `BMS_STATE_s.closedPrechargeContactors` | ✓ 已实现 |
| REQ-034 | 记录已关闭的字符串数量 | `BMS_STATE_s.numberOfClosedStrings` | ✓ 已实现 |
| REQ-035 | 记录已停用的字符串 | `BMS_STATE_s.deactivatedStrings` | ✓ 已实现 |
| REQ-036 | 记录首个闭合字符串标识符 | `BMS_STATE_s.firstClosedString` | ✓ 已实现 |
| REQ-037 | 字符串打开操作超时 | `BMS_STATE_s.stringOpenTimeout` | ✓ 已实现 |
| REQ-038 | 字符串关闭操作超时 | `BMS_STATE_s.stringCloseTimeout` | ✓ 已实现 |
| REQ-039 | 下一个字符串闭合计时器 | `BMS_STATE_s.nextStringClosedTimer` | ✓ 已实现 |
| REQ-040 | 检查重进入情况 | `BMS_CheckReEntrance()` | ✓ 已实现 |
| REQ-041 | 重进入时返回错误状态 (0xff) | `BMS_CheckReEntrance()` 返回 0xFF | ✓ 已实现 |
| REQ-042 | 防止多进程同时调用 | `OS_EnterTaskCritical()`/`OS_ExitTaskCritical()` 保护 | ✓ 已实现 |
| REQ-043 | 检查 CAN 总线请求 | `BMS_CheckCanRequests()` | ✓ 已实现 |
| REQ-044 | 将 CAN 请求转换为状态请求 | `BMS_CheckCanRequests()` | ✓ 已实现 |
| REQ-045 | 维护当前系统计时器值 | `BMS_STATE_s.currentSystick` | ✓ 已实现 |
| REQ-046 | 支持可配置的松弛期 | `BMS_STATE_s.restTimer_10ms`、`BS_RELAXATION_PERIOD_10ms` | ✓ 已实现 |
| REQ-047 | 记录当前流动状态 | `BMS_UpdateBatterySystemState()`、`BMS_STATE_s.currentFlowState` | ✓ 已实现 |
| REQ-048 | 追踪接触器断路电流超出的时间 | `BMS_STATE_s.timeAboveContactorBreakCurrent_ms` | ✓ 已实现 |
| REQ-049 | 支持时间/事件触发模式 | `BMS_Trigger()` 函数 | ✓ 已实现 |
| REQ-050 | 提供 10ms 级时间分辨率 | `restTimer_10ms`、相关超时定义 | ✓ 已实现 |
| REQ-051 | 支持毫秒单位延迟配置 | `BMS_NO_ACTIVE_DELAY_TIME_ms` 宏、延迟参数 | ✓ 已实现 |
| REQ-052 | 使用静态变量存储BMS状态 | `static BMS_STATE_s bms_state` | ✓ 已实现 |
| REQ-053 | 维护本地数据库表副本以提高效率 | 三个本地数据库表副本 | ✓ 已实现 |
| REQ-054 | 支持多个电池字符串 | 数组大小为 `BS_NR_OF_STRINGS` | ✓ 已实现 |
| REQ-055 | 维护错误请求计数器 | `BMS_STATE_s.ErrRequestCounter` | ✓ 已实现 |
| REQ-056 | 设置初始化完成标志 | `BMS_STATE_s.initFinished` | ✓ 已实现 |
| REQ-057 | 支持振荡超时检测机制 | `BMS_STATE_s.OscillationTimeout` | ✓ 已实现 |

---

## 11. 需求实现覆盖率

- **总需求数**: 57
- **已实现**: 57
- **覆盖率**: 100%

所有软件需求都已在代码中实现并添加了追溯性注释。

---

**文档结束**
