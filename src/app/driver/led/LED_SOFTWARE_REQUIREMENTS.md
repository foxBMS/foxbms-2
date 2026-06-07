# LED — 调试 LED 驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 LED 调试指示灯驱动模块的软件需求。LED 模块通过 MCU 引脚控制一个调试用 LED，以不同闪烁频率指示 BMS 运行状态。

### 1.2 范围
- **涵盖**：LED 开关控制、运行状态闪烁模式
- **不涵盖**：LED 硬件引脚配置

## 2. 功能需求

#### REQ-001 — LED 常亮（启动指示）
**优先级**：高 | **函数**：`LED_SetDebugLed()`

模块应在启动阶段点亮调试 LED 常亮，以指示系统正在启动。

#### REQ-002 — LED 周期性闪烁
**优先级**：高 | **函数**：`LED_Trigger()`

模块应周期性切换 LED 状态，实现闪烁效果。切换周期可通过 `LED_SetToggleTime()` 设置。

#### REQ-003 — 闪烁频率配置
**优先级**：高 | **函数**：`LED_SetToggleTime()`

模块应支持通过参数动态设置 LED 闪烁的 ON/OFF 持续时间（ms）。

#### REQ-004 — 运行状态指示模式
**优先级**：中 | **宏定义**：`LED_NORMAL_OPERATION_ON_OFF_TIME_ms`, `LED_ERROR_OPERATION_ON_OFF_TIME_ms`

模块应通过不同闪烁频率区分运行状态：
- 正常运行：500ms ON/OFF（慢闪）
- 错误状态：100ms ON/OFF（快闪）

## 3. 接口需求
| 函数 | 说明 |
|------|------|
| `LED_SetDebugLed()` | 点亮 LED（启动时调用）|
| `LED_Trigger()` | 周期性触发器 |
| `LED_SetToggleTime(ms)` | 设置闪烁周期 |
