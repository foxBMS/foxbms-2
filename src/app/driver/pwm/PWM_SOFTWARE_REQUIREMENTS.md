# PWM — 脉宽调制驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 PWM（Pulse Width Modulation）驱动模块的软件需求。PWM 模块使用 TMS570LC43 的 ePWM 外设生成 PWM 信号，并通过 eCAP 捕获外部 PWM 信号参数。

### 1.2 范围
- **涵盖**：ePWM 初始化与启停、占空比设置、eCAP 捕获测量
- **不涵盖**：HET PWM 外设

## 2. 功能需求

#### REQ-001 — PWM 初始化
**优先级**：高 | **函数**：`PWM_Initialize()`

模块应初始化 ePWM 和 eCAP 模块。需要在调用前先执行 `etpwmInit()`。

#### REQ-002 — PWM 启动/停止
**优先级**：高 | **函数**：`PWM_StartPwm()`, `PWM_StopPwm()`

模块应启动或停止所有已配置的 ePWM 通道。

#### REQ-003 — 占空比设置
**优先级**：高 | **函数**：`PWM_SetDutyCycle(dutyCycle_perm)`

模块应以千分比（permill）为单位设置 PWM 占空比（当前仅支持通道 1A）。超出范围的值应被限制到阈值。

#### REQ-004 — PWM 输入信号捕获
**优先级**：中 | **函数**：`PWM_GetPwmData()`

模块应通过 eCAP 模块捕获外部 PWM 信号的占空比和频率，返回 `PWM_SIGNAL_s` 结构体。

#### REQ-005 — eCAP 模块状态
**优先级**：低 | **函数**：`PWM_IsEcapModuleInitialized()`

## 3. 数据结构
```c
typedef struct {
    float_t dutyCycle_perc;   // 占空比 (%)
    float_t frequency_Hz;     // 频率 (Hz)
} PWM_SIGNAL_s;
```

## 4. 接口需求
| 函数 | 说明 |
|------|------|
| `PWM_Initialize()` | 初始化 ePWM/eCAP |
| `PWM_StartPwm/StopPwm()` | 启停 PWM |
| `PWM_SetDutyCycle(perm)` | 设置占空比（千分比）|
| `PWM_GetPwmData()` | 获取输入 PWM 参数 |
