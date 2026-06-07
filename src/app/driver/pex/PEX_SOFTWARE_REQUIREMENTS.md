# PEX — 端口扩展器驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 PEX（Port Expander）端口扩展器驱动模块的软件需求。PEX 模块通过 I2C 接口控制 NXP PCA9539 端口扩展器芯片，扩展 MCU 的可用 I/O 引脚。

### 1.2 范围
- **涵盖**：端口扩展器引脚方向配置、输出控制、输入读取、极性设置
- **不涵盖**：PCA9539 硬件初始化（由配置层负责）

## 2. 功能需求

#### REQ-001 — PEX 初始化
**优先级**：高 | **函数**：`PEX_Initialize()`

模块应初始化端口扩展器的本地状态变量。

#### REQ-002 — 引脚输出控制
**优先级**：高 | **函数**：`PEX_SetPin()`, `PEX_ResetPin()`

模块应通过 I2C 设置/复位指定端口扩展器的指定引脚。

#### REQ-003 — 引脚状态读取
**优先级**：高 | **函数**：`PEX_GetPin()`

模块应返回指定引脚的高低状态（`PEX_PIN_HIGH`/`PEX_PIN_LOW`）。

#### REQ-004 — 引脚方向配置
**优先级**：高 | **函数**：`PEX_SetPinDirectionInput()`, `PEX_SetPinDirectionOutput()`

模块应配置指定引脚为输入或输出模式。

#### REQ-005 — 引脚极性配置
**优先级**：中 | **函数**：`PEX_SetPinPolarityInverted()`, `PEX_SetPinPolarityRetained()`

模块应配置指定引脚的极性（反相/保持）。

#### REQ-006 — 周期性同步触发
**优先级**：高 | **函数**：`PEX_Trigger()`

模块应通过 I2C 总线将本地状态写入硬件寄存器，并读取输入状态到本地变量。

## 3. 宏定义
- `PEX_PIN_LOW` = 0, `PEX_PIN_HIGH` = 1

## 4. 接口需求
| 函数 | 说明 |
|------|------|
| `PEX_Initialize()` | 初始化 |
| `PEX_SetPin/ResetPin(pex, pin)` | 引脚输出控制 |
| `PEX_GetPin(pex, pin)` | 引脚输入读取 |
| `PEX_SetPinDirectionInput/Output(pex, pin)` | 方向配置 |
| `PEX_SetPinPolarityInverted/Retained(pex, pin)` | 极性配置 |
| `PEX_Trigger()` | 同步触发 |
