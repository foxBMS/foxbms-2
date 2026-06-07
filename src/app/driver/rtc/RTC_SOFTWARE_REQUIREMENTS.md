# RTC — 实时时钟驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 RTC（Real-Time Clock）驱动模块的软件需求。RTC 模块通过 I2C 接口驱动 NXP PCF2131 实时时钟芯片，维护系统时间并提供时间戳服务。

### 1.2 范围
- **涵盖**：RTC IC 初始化、系统时间维护（1ms 粒度）、时间设置/读取、电池低电压检测
- **不涵盖**：I2C 底层协议

### 1.3 配置
- I2C 接口：`i2cREG1`
- I2C 地址：`0x53`

## 2. 功能需求

#### REQ-001 — RTC IC 初始化
**优先级**：高 | **函数**：`RTC_Initialize()`

模块应通过 I2C 初始化 PCF2131 芯片，包括 STOP 位清除、24 小时模式设置、电池电源管理配置和 OTPR 操作。

#### REQ-002 — 系统时间同步
**优先级**：高 | **函数**：`RTC_InitializeSystemTimeWithRtc()`

模块应在启动时从 RTC IC 读取时间并同步到软件系统定时器。

#### REQ-003 — 毫秒级系统时间递增
**优先级**：高 | **函数**：`RTC_IncrementSystemTime()`

模块应每 1ms 递增软件系统定时器（需在 1ms 任务中调用）。

#### REQ-004 — 系统时间获取
**优先级**：高 | **函数**：`RTC_GetSystemTimeRtcFormat()`

模块应返回当前系统时间的 RTC 格式数据（含百分秒、秒、分、时、日、月、年）。

#### REQ-005 — 系统时间设置
**优先级**：中 | **函数**：`RTC_SetSystemTimeRtcFormat()`

模块应支持通过 CAN 消息等外部请求设置 RTC 时间。

#### REQ-006 — 周期性触发
**优先级**：高 | **函数**：`RTC_Trigger()`

模块应检查 CAN 消息队列中的时间设置请求，执行 RTC IC 写入和系统时间调整，同时定期检查电池低电压标志。

#### REQ-007 — RTC 定时器与 IC 同步
模块应每 `RTC_TIME_BETWEEN_RTC_TIMER_ADJUSTMENT_min`（60 分钟）比较软件定时器与 RTC IC 时间，偏差超过 1 秒时进行修正。

## 3. 数据结构
```c
typedef struct {
    time_t secondsSinceEpoch;
    uint16_t milliseconds;
} RTC_SYSTEM_TIMER_EPOCH_s;
```

## 4. 接口需求（8 个公开函数）
| 函数 | 说明 |
|------|------|
| `RTC_Initialize()` | IC 初始化 |
| `RTC_InitializeSystemTimeWithRtc()` | 时间同步 |
| `RTC_IncrementSystemTime()` | 1ms 递增 |
| `RTC_GetSystemTimeRtcFormat()` | 获取时间 |
| `RTC_SetSystemTimeRtcFormat()` | 设置时间 |
| `RTC_Trigger()` | 周期性触发 |
| `RTC_GetSystemStartUpTime()` | 获取启动时间戳 |
| `RTC_IsRtcModuleInitialized()` | 初始化状态 |
