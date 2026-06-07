# MCU — 微控制器工具函数软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 MCU 工具模块的软件需求。MCU 模块提供微控制器级别的通用工具函数，包括微秒级延时、自由运行计数器读取、超时检测和设备 ID 读取。

### 1.2 范围
- **涵盖**：微秒延时、FRC0 计数器访问、超时判断、设备 ID 读取
- **不涵盖**：MCU 时钟配置、外设初始化

### 1.3 定义
| 术语 | 说明 |
|------|------|
| FRC0 | Free Running Counter 0，32 位自由运行计数器 |
| RTI | Real-Time Interrupt，实时中断模块 |

## 2. 功能需求

#### REQ-001 — 微秒级延时
**优先级**：高 | **函数**：`MCU_Delay_us()`

模块应通过 FRC0 计数器实现阻塞式微秒级延时。

#### REQ-002 — 自由运行计数器读取
**优先级**：高 | **函数**：`MCU_GetFreeRunningCount()`

模块应读取 FRC0 计数器当前值，供超时检测使用。

#### REQ-003 — 计数器差值时间转换
**优先级**：中 | **函数**：`MCU_ConvertFrcDifferenceToTimespan_us()`

模块应将 FRC0 计数器差值转换为微秒时间。

#### REQ-004 — 超时检测
**优先级**：高 | **函数**：`MCU_IsTimeElapsed()`

模块应检查从起始计数值起是否已超出指定超时时间（μs）。

#### REQ-005 — 设备 ID 读取
**优先级**：低 | **函数**：`MCU_GetDeviceRegister()`, `MCU_GetDieIdHigh()`, `MCU_GetDieIdLow()`

模块应读取 MCU 的唯一设备标识寄存器。

## 3. 配置宏
- `MCU_LARGEST_PIN_NUMBER` = 31（TMS570LC4357 最大引脚号）
- `MCU_ADC1_MAX_NR_CHANNELS` = 32（ADC1 最大通道数）
- `MCU_RTI_CNT0_FRC0_REG` = 0xFFFFFC10（FRC0 寄存器地址）

## 4. 接口需求
| 函数 | 说明 |
|------|------|
| `MCU_Delay_us(us)` | 微秒延时 |
| `MCU_GetFreeRunningCount()` | 获取 FRC0 值 |
| `MCU_ConvertFrcDifferenceToTimespan_us(cnt)` | 计数器差值→微秒 |
| `MCU_IsTimeElapsed(start, timeout_us)` | 超时判断 |
| `MCU_GetDeviceRegister/DieIdHigh/DieIdLow()` | 设备 ID |
