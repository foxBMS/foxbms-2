# BMS 驱动程序 - 需求可追溯性映射

**文件**: `bms.c`  
**版本**: v1.11.0  
**日期**: 2026-04-20

---

## 文档说明

本文档提供了软件需求与源代码实现之间的详细映射。每一行都显示了需求、对应的代码位置以及该代码如何实现该需求。

此文档与 `BMS_SOFTWARE_REQUIREMENTS.md` 配合使用，为开发者和审核人员提供完整的需求追溯关联。

---

## 需求到代码映射表

### 1. 状态机管理需求

#### 状态请求处理
| 需求编号 | 需求描述 | 代码位置 | 实现方式 |
|---------|--------|--------|--------|
| REQ-001 | 检查状态请求有效性 | `BMS_CheckStateRequest()` | 函数检查请求类型和当前状态，返回有效性结果 |
| REQ-002 | 转移状态请求到状态机 | `BMS_TransferStateRequest()` | 函数从状态变量读取请求并清除请求 |
| REQ-003 | 从数据库读取状态请求 | `BMS_CheckCanRequests()` | 函数从 CAN 数据库读取状态请求 |

#### 状态转换
| 需求编号 | 需求描述 | 代码位置 | 实现方式 |
|---------|--------|--------|--------|
| REQ-004 | 状态转换 | `bms_state.state` 字段 | 全局状态变量在 BMS_Trigger() 中更新 |
| REQ-005 | 跟踪前一个状态和子状态 | `BMS_SAVE_LAST_STATES()` 宏、`lastState`、`lastSubstate` | 宏保存当前状态到前一个状态 |
| REQ-006 | 支持进入、保持、退出子状态 | `bms_state.substate` 字段 | 子状态在状态机中管理 |

---

### 2. 电池系统监测需求

#### 故障检测
| 需求编号 | 需求描述 | 代码位置 | 实现方式 |
|---------|--------|--------|--------|
| REQ-007 | 监测 DIAG_FATAL_ERROR 错误 | `BMS_IsAnyFatalErrorFlagSet()` | 遍历诊断模块中的所有致命错误条目 |
| REQ-008 | 设置 minimumActiveDelay_ms 参数 | `BMS_IsAnyFatalErrorFlagSet()` | 比较所有错误的延迟，存储最小值 |
| REQ-009 | 多个错误时使用最短延迟 | `BMS_IsAnyFatalErrorFlagSet()` | `if (bms_state.minimumActiveDelay_ms > kDelay_ms)` |
| REQ-010 | 检测电池系统状态和触发打开 | `BMS_IsBatterySystemStateOkay()` | 检查延迟并在到期时返回 STD_NOT_OK |

#### 测量值获取
| 需求编号 | 需求描述 | 代码位置 | 实现方式 |
|---------|--------|--------|--------|
| REQ-011 | 获取最新测量值 | `BMS_GetMeasurementValues()` | 调用 `DATA_READ_DATA()` 从数据库读取 |
| REQ-012 | 维护本地数据库表副本 | `bms_tableMinMax`, `bms_tableOpenWire`, `bms_tablePackValues` | 三个静态全局变量存储本地副本 |

#### 开路检测
| 需求编号 | 需求描述 | 代码位置 | 实现方式 |
|---------|--------|--------|--------|
| REQ-013 | 检测任何开路感知线 | `BMS_CheckOpenSenseWire()` | 遍历所有字符串和模块检查开路 |
| REQ-014 | 记录开路检测诊断信息 | `BMS_CheckOpenSenseWire()` | 调用 `DIAG_Handler()` 记录错误 |

---

### 3. 接触器管理需求

#### 反馈验证
| 需求编号 | 需求描述 | 代码位置 | 实现方式 |
|---------|--------|--------|--------|
| REQ-015 | 验证接触器反馈有效性 | `BMS_IsContactorFeedbackValid()` | 读取错误状态数据库条目 |
| REQ-016 | 从错误标志数据库读取反馈 | `BMS_IsContactorFeedbackValid()` | `DATA_READ_DATA(&tableErrorFlags)` |
| REQ-017 | 支持指定字符串和接触器类型 | `BMS_IsContactorFeedbackValid(stringNumber, contactorType)` | 参数化函数支持多种检查 |

#### 接触器控制
| 需求编号 | 需求描述 | 代码位置 | 实现方式 |
|---------|--------|--------|--------|
| REQ-018 | 追踪电源路径状态 | `bms_state.powerPath` | 状态变量记录路径状态 |
| REQ-019 | 管理多个字符串接触器状态 | `bms_state.closedStrings`, `closedPrechargeContactors` | 数组字段管理每个字符串的接触器 |
| REQ-020 | 记录需要打开的接触器 | `BMS_GetFirstContactorToBeOpened()`, `BMS_GetSecondContactorToBeOpened()` | 函数返回应该打开的接触器类型 |

---

### 4. 预充电过程需求

#### 预充电监控
| 需求编号 | 需求描述 | 代码位置 | 实现方式 |
|---------|--------|--------|--------|
| REQ-021 | 监控预充电过程 | `BMS_MonitorPrechargeProcess()` | 检查电流和电压条件，超时检查 |
| REQ-022 | 支持预充电超时配置 | `BMS_MonitorPrechargeProcess(timeout_ms)` | `if (bms_state.currentSystick - bms_state.startOfPrecharging > timeout_ms)` |
| REQ-023 | 支持预充电重试计数 | `bms_state.prechargeTryCounter` | 状态变量计数重试次数 |
| REQ-024 | 返回预充电过程状态 | `BMS_RESULT_PRECHARGE_PROCESS_e` 枚举 | SUCCESSFUL/ONGOING/FAILED 三种状态 |

#### 电流/电压限制检查
| 需求编号 | 需求描述 | 代码位置 | 实现方式 |
|---------|--------|--------|--------|
| REQ-025 | 检查预充电电流限制 | `BMS_IsPrechargeCurrentBelowLimit()` | `MATH_AbsInt32_t(packCurrent) < BMS_PRECHARGE_CURRENT_THRESHOLD_mA` |
| REQ-026 | 检查预充电电压限制 | `BMS_IsPrechargeVoltageBelowLimit()` | 计算电压差并与阈值比较 |
| REQ-027 | 检查失败返回 STD_NOT_OK | 两个检查函数 | 返回 `STD_OK` 或 `STD_NOT_OK` |

---

### 5. 电池字符串管理需求

#### 字符串选择策略
| 需求编号 | 需求描述 | 代码位置 | 实现方式 |
|---------|--------|--------|--------|
| REQ-028 | 识别最高电压字符串 | `BMS_GetHighestString()` | 遍历所有字符串找最大电压 |
| REQ-029 | 识别最接近电压字符串 | `BMS_GetClosestString()` | 计算与首个闭合字符串的电压差 |
| REQ-030 | 识别最低电压字符串 | `BMS_GetLowestString()` | 遍历所有字符串找最小电压 |
| REQ-031 | 计算字符串电压差 | `BMS_GetStringVoltageDifference()` | `abs(stringVoltage - firstClosedStringVoltage)` |

#### 字符串状态追踪
| 需求编号 | 需求描述 | 代码位置 | 实现方式 |
|---------|--------|--------|--------|
| REQ-032 | 记录闭合字符串状态 | `bms_state.closedStrings[]` | 布尔数组记录每个字符串状态 |
| REQ-033 | 记录预充电接触器状态 | `bms_state.closedPrechargeContactors[]` | 布尔数组记录预充电接触器状态 |
| REQ-034 | 记录闭合字符串数量 | `bms_state.numberOfClosedStrings` | 计数器变量 |
| REQ-035 | 记录停用字符串 | `bms_state.deactivatedStrings[]` | 布尔数组记录停用状态 |
| REQ-036 | 记录首个闭合字符串 | `bms_state.firstClosedString` | 变量存储字符串索引 |

#### 字符串开关超时
| 需求编号 | 需求描述 | 代码位置 | 实现方式 |
|---------|--------|--------|--------|
| REQ-037 | 字符串打开超时 | `bms_state.stringOpenTimeout` | 在 BMS_Trigger() 中递减 |
| REQ-038 | 字符串关闭超时 | `bms_state.stringCloseTimeout` | 在 BMS_Trigger() 中递减 |
| REQ-039 | 下一字符串闭合计时器 | `bms_state.nextStringClosedTimer` | 在 BMS_Trigger() 中递减 |

---

### 6. 其他功能需求

#### 重进入检查
| 需求编号 | 需求描述 | 代码位置 | 实现方式 |
|---------|--------|--------|--------|
| REQ-040 | 检查重进入 | `BMS_CheckReEntrance()` | 增加触发计数器并在关键区保护 |
| REQ-041 | 重进入返回 0xff | `BMS_CheckReEntrance()` | `retval = 0xFF` 当重进入时 |
| REQ-042 | 防止多进程调用 | `OS_EnterTaskCritical()`/`OS_ExitTaskCritical()` | 关键区保护状态访问 |

#### CAN 请求处理
| 需求编号 | 需求描述 | 代码位置 | 实现方式 |
|---------|--------|--------|--------|
| REQ-043 | 检查 CAN 总线请求 | `BMS_CheckCanRequests()` | 从状态请求数据块读取 |
| REQ-044 | 转换 CAN 请求 | `BMS_CheckCanRequests()` | 映射 CAN 值到状态请求 |

#### 时间管理
| 需求编号 | 需求描述 | 代码位置 | 实现方式 |
|---------|--------|--------|--------|
| REQ-045 | 维护系统计时器值 | `bms_state.currentSystick` | `bms_state.currentSystick = OS_GetTickCount()` 在 BMS_Trigger() |
| REQ-046 | 支持可配置松弛期 | `bms_state.restTimer_10ms`, `BS_RELAXATION_PERIOD_10ms` | 定时递减计时器 |
| REQ-047 | 记录流动状态 | `BMS_UpdateBatterySystemState()`, `bms_state.currentFlowState` | 根据电流更新状态 |
| REQ-048 | 追踪接触器断路电流时间 | `bms_state.timeAboveContactorBreakCurrent_ms` | 状态变量记录时间 |

---

### 7. 性能需求

#### 定时要求
| 需求编号 | 需求描述 | 代码位置 | 实现方式 |
|---------|--------|--------|--------|
| REQ-049 | 支持时间/事件触发 | `BMS_Trigger()` 函数 | 主要触发函数 |
| REQ-050 | 提供 10ms 时间分辨率 | 各种 10ms 定时器 | `restTimer_10ms` 等 |
| REQ-051 | 支持毫秒延迟配置 | `BMS_NO_ACTIVE_DELAY_TIME_ms` 宏 | 延迟参数使用 ms 单位 |

#### 资源需求
| 需求编号 | 需求描述 | 代码位置 | 实现方式 |
|---------|--------|--------|--------|
| REQ-052 | 使用静态变量存储状态 | `static BMS_STATE_s bms_state` | 全局静态变量 |
| REQ-053 | 维护本地数据库副本 | 三个本地表副本 | 提高访问效率 |
| REQ-054 | 支持多个电池字符串 | 数组大小 `BS_NR_OF_STRINGS` | 可配置的字符串数量 |

#### 可靠性需求
| 需求编号 | 需求描述 | 代码位置 | 实现方式 |
|---------|--------|--------|--------|
| REQ-055 | 错误请求计数器 | `bms_state.ErrRequestCounter` | 计数器变量 |
| REQ-056 | 初始化完成标志 | `bms_state.initFinished` | 布尔状态 |
| REQ-057 | 振荡超时检测 | `bms_state.OscillationTimeout` | 在 BMS_Trigger() 中递减 |

---

## 代码到需求反向映射表

### 关键函数与需求关联

#### `BMS_CheckStateRequest()`
- **实现需求**: REQ-001, REQ-055
- **功能**: 验证状态请求的有效性

#### `BMS_TransferStateRequest()`
- **实现需求**: REQ-002
- **功能**: 将状态请求转移到状态机

#### `BMS_GetMeasurementValues()`
- **实现需求**: REQ-011, REQ-012
- **功能**: 从数据库获取最新测量值

#### `BMS_CheckOpenSenseWire()`
- **实现需求**: REQ-013, REQ-014
- **功能**: 检测开路和记录诊断信息

#### `BMS_IsAnyFatalErrorFlagSet()`
- **实现需求**: REQ-007, REQ-008, REQ-009
- **功能**: 检测致命错误并设置最小延迟

#### `BMS_IsBatterySystemStateOkay()`
- **实现需求**: REQ-010
- **功能**: 检查电池系统状态和延迟

#### `BMS_IsContactorFeedbackValid()`
- **实现需求**: REQ-015, REQ-016, REQ-017
- **功能**: 验证接触器反馈

#### `BMS_MonitorPrechargeProcess()`
- **实现需求**: REQ-021, REQ-022, REQ-023, REQ-024
- **功能**: 监控预充电过程

#### `BMS_IsPrechargeCurrentBelowLimit()`
- **实现需求**: REQ-025, REQ-027
- **功能**: 检查预充电电流

#### `BMS_IsPrechargeVoltageBelowLimit()`
- **实现需求**: REQ-026, REQ-027
- **功能**: 检查预充电电压

#### `BMS_GetHighestString()`
- **实现需求**: REQ-028
- **功能**: 识别最高电压字符串

#### `BMS_GetClosestString()`
- **实现需求**: REQ-029
- **功能**: 识别最接近电压字符串

#### `BMS_GetLowestString()`
- **实现需求**: REQ-030
- **功能**: 识别最低电压字符串

#### `BMS_GetStringVoltageDifference()`
- **实现需求**: REQ-031
- **功能**: 计算字符串电压差

#### `BMS_UpdateBatterySystemState()`
- **实现需求**: REQ-047
- **功能**: 更新电池系统流动状态

#### `BMS_GetFirstContactorToBeOpened()`
- **实现需求**: REQ-020
- **功能**: 确定首个要打开的接触器

#### `BMS_GetSecondContactorToBeOpened()`
- **实现需求**: REQ-018, REQ-019, REQ-020
- **功能**: 确定第二个要打开的接触器

#### `BMS_CheckReEntrance()`
- **实现需求**: REQ-040, REQ-041, REQ-042
- **功能**: 检查重进入和防护

#### `BMS_CheckCanRequests()`
- **实现需求**: REQ-043, REQ-044
- **功能**: 检查和转换 CAN 请求

#### `BMS_Trigger()`
- **实现需求**: REQ-004, REQ-005, REQ-011, REQ-013, REQ-040, REQ-041, REQ-045, REQ-047, REQ-049
- **功能**: 主状态机触发函数

---

## 使用本映射文档

### 按需求查找代码
1. 在顶部的"需求到代码映射表"中找到需求编号
2. 查看"代码位置"列获得函数名称
3. 在 `bms.c` 中搜索该函数名称

### 按代码查找需求
1. 在"代码到需求反向映射表"中找到函数名称
2. 查看"实现需求"列获得相关的需求编号
3. 在"BMS_SOFTWARE_REQUIREMENTS.md"中查看需求描述

### 验证覆盖率
1. 检查两个表中的所有 57 个需求是否都有对应的代码位置
2. 运行代码分析工具验证所有需求编号标记

---

**文档版本**: 1.0  
**最后更新**: 2026-06-05  
**维护者**: foxBMS Team
