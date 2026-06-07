# MEAS — 测量管理模块软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 MEAS（Measurements）测量管理模块的软件需求。MEAS 模块是测量系统的顶层调度器，负责初始化测量子系统、周期性触发测量序列、以及向 AFE 状态机发送各种测量请求。

### 1.2 范围
- **涵盖**：测量系统初始化、周期性控制、AFE 测量请求（温度、均衡反馈、EEPROM、断线检测）
- **不涵盖**：AFE 底层测量实现、传感器具体通信协议

## 2. 功能需求

#### REQ-001 — 测量系统初始化
**优先级**：高 | **函数**：`MEAS_Initialize()`

模块应调用所有测量相关模块（ADC、AFE、温度传感器等）的初始化函数。

#### REQ-002 — 周期性测量控制（1ms 周期）
**优先级**：高 | **函数**：`MEAS_Control()`

模块应每 1ms 被任务调度器调用一次，驱动所有测量相关子模块的状态机。

#### REQ-003 — 首次测量周期完成检测
**优先级**：中 | **函数**：`MEAS_IsFirstMeasurementCycleFinished()`

模块应指示第一个 AFE 测量周期是否已完成。

#### REQ-004 — 启动测量
**优先级**：高 | **函数**：`MEAS_StartMeasurement()`

模块应向 AFE 状态机发送初始化请求，启动测量序列。

#### REQ-005 — 温度读取请求
**优先级**：高 | **函数**：`MEAS_RequestTemperatureRead(string)`

模块应向指定电池串的 AFE 发送外部温度传感器读取请求。

#### REQ-006 — 均衡反馈读取请求
**优先级**：中 | **函数**：`MEAS_RequestBalancingFeedbackRead(string)`

模块应向指定电池串发送均衡反馈状态读取请求。

#### REQ-007 — EEPROM 读写请求
**优先级**：中 | **函数**：`MEAS_RequestEepromRead/Write(string)`

模块应支持对从板 EEPROM 的读写请求。

#### REQ-008 — 断线检测请求
**优先级**：中 | **函数**：`MEAS_RequestOpenWireCheck(string)`

模块应支持对指定电池串的电压采集线断线检测。

## 3. 接口需求
| 函数 | 说明 |
|------|------|
| `MEAS_Initialize()` | 初始化所有测量模块 |
| `MEAS_Control()` | 1ms 周期性控制 |
| `MEAS_StartMeasurement()` | 启动测量 |
| `MEAS_Request*()` | 各类测量请求 |
