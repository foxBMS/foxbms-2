# HTSENSOR — 高温湿度传感器驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 HTSENSOR（Humidity/Temperature Sensor，湿度/温度传感器）驱动模块的软件需求。HTSENSOR 模块负责通过 I2C 接口驱动 Sensirion SHT35-DIS 传感器，获取电池管理系统的环境温度和湿度数据。

### 1.2 范围
- **涵盖**：I2C 传感器测量触发、数据读取、CRC8 校验、原始值到物理量的转换
- **不涵盖**：I2C 底层驱动协议、传感器硬件引脚配置

### 1.3 定义与缩略语
| 术语 | 说明 |
|------|------|
| HTSEN | Humidity/Temperature Sensor |
| SHT35-DIS | Sensirion 数字温湿度传感器型号 |
| ddegC | 分摄氏度（deci-degree Celsius，0.1°C 分辨率）|
| CRC8 | 8 位循环冗余校验 |

### 1.4 参考文献
- Sensirion SHT35-DIS 数据手册，2019 年 2 月，版本 6

## 2. 总体描述

### 2.1 产品视角
HTSENSOR 模块位于 DRIVERS 层，通过 I2C1 接口与 SHT35-DIS 传感器通信。测量结果通过 `DATA_WRITE_DATA()` 写入数据库。

### 2.2 工作模式
以状态机方式运行：
1. **HTSEN_START_MEAS** — 发送测量命令
2. **HTSEN_READ_RESULTS** — 读取传感器数据（含重试机制）

## 3. 功能需求

### 3.1 传感器测量控制

#### REQ-001 — 测量触发状态机
**优先级**：高 | **函数**：`HTSEN_Trigger()`

模块应通过两态状态机驱动传感器：先发送单次测量命令，再读取结果，循环往复。

#### REQ-002 — I2C 测量命令发送
**优先级**：高 | **函数**：`HTSEN_Trigger()`（HTSEN_START_MEAS）

模块应通过 I2C DMA 向传感器地址 `0x44` 发送 2 字节测量命令（MSB + LSB），采用高可重复性模式。

#### REQ-003 — I2C 数据读取
**优先级**：高 | **函数**：`HTSEN_Trigger()`（HTSEN_READ_RESULTS）

模块应通过 I2C DMA 从传感器读取 6 字节数据：[温度MSB, 温度LSB, 温度CRC, 湿度MSB, 湿度LSB, 湿度CRC]。

#### REQ-004 — 读取重试机制
**优先级**：高 | **函数**：`HTSEN_Trigger()`

当传感器未就绪（I2C NACK）时，模块应在 `HTSEN_READ_TRIES`（5 次）内重试读取，超过则重新触发测量。

### 3.2 数据校验与转换

#### REQ-005 — CRC8 校验
**优先级**：高 | **函数**：`HTSEN_CalculateCrc8()`

模块应实现传感器 CRC8 校验算法（多项式 `0x31`，种子值 `0xFF`），分别校验温度和湿度的原始数据。仅在校验通过后才采用测量值。

#### REQ-006 — 温度原始值转换
**优先级**：高 | **函数**：`HTSEN_ConvertRawTemperature()`

模块应将原始温度值转换为分摄氏度（ddegC）：`T[ddegC] = 10 × (-45 + 175 × raw / 65535)`

#### REQ-007 — 湿度原始值转换
**优先级**：高 | **函数**：`HTSEN_ConvertRawHumidity()`

模块应将原始湿度值转换为百分比：`H[%] = 100 × raw / 65535`

#### REQ-008 — 数据写入数据库
**优先级**：高 | **函数**：`HTSEN_Trigger()`

CRC 校验通过后，模块应通过 `DATA_WRITE_DATA()` 将温湿度数据写入数据库。

### 3.3 配置参数

#### REQ-009 — 传感器地址与接口配置
**优先级**：中 | **宏定义**：`HTSEN_I2C_INTERFACE`, `HTSEN_I2C_ADDRESS`

传感器使用 I2C1 接口（`i2cREG1`），I2C 地址为 `0x44`。

#### REQ-010 — 时钟拉伸配置
**优先级**：低 | **宏定义**：`HTSEN_CLOCK_STRETCHING`

模块不支持时钟拉伸模式（`HTSEN_CLOCK_STRETCHING = false`），测量命令 MSB 为 `0x24`。

## 4. 非功能需求

- **REQ-011**：读写缓冲区使用共享 RAM（`.sharedRAM`）段存储
- **REQ-012**：无效状态触发 `FAS_ASSERT(FAS_TRAP)` 保护
- **REQ-013**：单元测试可访问 `TEST_HTSEN_TestCalculateCrc8()` 进行 CRC8 算法验证

## 5. 接口需求

### 5.1 公共 API
| 函数 | 说明 |
|------|------|
| `HTSEN_Trigger()` | 驱动传感器状态机执行一次测量循环 |

### 5.2 依赖项
| 依赖模块 | 用途 |
|----------|------|
| `i2c.h` | I2C DMA 读写 |
| `database.h` | 数据块写入 |

## 6. 追溯矩阵

| 需求编号 | 需求描述 | 函数/宏 | 文件 |
|----------|----------|---------|------|
| REQ-001 | 状态机控制 | `HTSEN_Trigger()` | htsensor.c:225-279 |
| REQ-002 | 测量命令发送 | `HTSEN_Trigger()` | htsensor.c:233 |
| REQ-003 | 数据读取 | `HTSEN_Trigger()` | htsensor.c:242-243 |
| REQ-004 | 读取重试 | `HTSEN_Trigger()` | htsensor.c:266-271 |
| REQ-005 | CRC8 校验 | `HTSEN_CalculateCrc8()` | htsensor.c:192-209 |
| REQ-006 | 温度转换 | `HTSEN_ConvertRawTemperature()` | htsensor.c:211-216 |
| REQ-007 | 湿度转换 | `HTSEN_ConvertRawHumidity()` | htsensor.c:218-221 |
| REQ-008 | 数据写库 | `DATA_WRITE_DATA()` | htsensor.c:263 |
| REQ-009 | 传感器地址 | `HTSEN_I2C_ADDRESS` | htsensor.c:72 |
