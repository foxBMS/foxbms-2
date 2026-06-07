# ADC — 模数转换器驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 ADC（Analog-to-Digital Converter，模数转换器）驱动模块的软件需求。ADC 模块负责控制 MCU 内置 ADC 外设，完成模拟电压信号的采集、转换和数据存储。

### 1.2 范围
- **涵盖**：ADC 转换状态机、原始数据到电压值的转换、数据写入数据库
- **不涵盖**：ADC 硬件初始化（由 HAL 层 `HL_adc.h` 负责）、ADC 引脚配置

### 1.3 定义与缩略语
| 术语 | 说明 |
|------|------|
| ADC | Analog-to-Digital Converter，模数转换器 |
| VREF | 电压参考源（Voltage Reference） |
| 12-bit 转换 | 12 位分辨率，量化为 4096 级 |
| 数据块（Data Block） | foxBMS 数据库框架中的数据结构 |

### 1.4 参考文献
- Technical Reference Manual SPNU563A, March 2018, Equation 28 (page 852)
- foxBMS 数据库框架 (`database.h`)

## 2. 总体描述

### 2.1 产品视角
ADC 模块位于 DRIVERS 层，依赖 HAL 层 ADC 驱动（`HL_adc.h`）和数据库框架（`database.h`）。上层应用通过周期性调用 `ADC_Control()` 来驱动 ADC 状态机。

### 2.2 工作模式
ADC 模块以状态机方式运行，包含三种状态：
1. **ADC_START_CONVERSION** — 启动一次 ADC 转换
2. **ADC_WAIT_CONVERSION_FINISHED** — 等待转换完成
3. **ADC_CONVERSION_FINISHED** — 读取转换结果并存储

### 2.3 用户特征
本模块仅供 foxBMS 内部引擎层（ENGINE）调用，不对外部用户暴露接口。

## 3. 功能需求

### 3.1 ADC 转换控制

#### REQ-001 — ADC 转换状态机
**编号** | **优先级** | **函数**
REQ-001 | 高 | `ADC_Control()`

**描述**：模块应实现一个三态 ADC 转换状态机，循环执行"启动转换 → 等待完成 → 读取结果"的测量序列。

**处理流程**：
1. 在 `ADC_START_CONVERSION` 状态，调用 `adcStartConversion()` 启动 ADC 转换
2. 切换到 `ADC_WAIT_CONVERSION_FINISHED` 状态
3. 在等待状态，轮询 `adcIsConversionComplete()` 检查转换是否完成
4. 转换完成后切换到 `ADC_CONVERSION_FINISHED` 状态
5. 在完成状态，读取数据并写回 `ADC_START_CONVERSION` 状态继续下一次转换

**错误处理**：无效状态触发 `FAS_ASSERT(FAS_TRAP)` 断言保护。

#### REQ-002 — ADC 转换完成检测
**编号** | **优先级** | **函数**
REQ-002 | 高 | `ADC_Control()`（ADC_WAIT_CONVERSION_FINISHED 分支）

**描述**：模块应通过检查 `ADC_CONVERSION_ENDBIT`（第 8 位）来确定 ADC 转换是否已完成。

#### REQ-003 — ADC 原始数据读取
**编号** | **优先级** | **函数**
REQ-003 | 高 | `ADC_Control()`（ADC_CONVERSION_FINISHED 分支）、`adcGetData()`

**描述**：模块应在转换完成后调用 `adcGetData()` 读取所有 ADC 通道的原始数值，存储到 `adc_adc1RawVoltages[]` 数组中。

#### REQ-004 — ADC 电压值转换
**编号** | **优先级** | **函数**
REQ-004 | 高 | `ADC_ConvertVoltage()`

**描述**：模块应将 ADC 原始计数值（counts）转换为以 mV 为单位的电压值。转换公式引用 Technical Reference Manual SPNU563A Equation 28：
```
result_mV = ((adcCounts + 0.5) * (VREFHIGH - VREFLOW)) / 4096 + VREFLOW
```

**输入**：
| 参数 | 类型 | 说明 |
|------|------|------|
| adcCounts | uint16_t | ADC 原始数字值 |

**返回值**：float_t 类型的电压值（mV）

#### REQ-005 — ADC 数据写入数据库
**编号** | **优先级** | **函数**
REQ-005 | 高 | `ADC_Control()`（ADC_CONVERSION_FINISHED 分支）、`DATA_WRITE_DATA()`

**描述**：模块应将转换后的电压值通过 `DATA_WRITE_DATA()` 写入数据库 `adc_adc1Voltages` 数据块（数据块 ID: `DATA_BLOCK_ID_ADC_VOLTAGE`）。

### 3.2 配置与参数

#### REQ-006 — ADC 参考电压配置
**编号** | **优先级** | **宏定义**
REQ-006 | 中 | `ADC_VREFHIGH_mV`, `ADC_VREFLOW_mV`

**描述**：模块应通过宏定义提供 ADC 参考电压的配置，高参考电压为 5000mV，低参考电压为 0mV。

#### REQ-007 — ADC 转换分辨率配置
**编号** | **优先级** | **宏定义**
REQ-007 | 中 | `ADC_CONVERSION_FACTOR_12BIT`, `ADC_CONVERSION_FACTOR_10BIT`

**描述**：模块应提供 12 位和 10 位两种分辨率的转换因子宏，当前使用 12 位转换（4096 级）。

## 4. 非功能需求

### 4.1 时序与性能
- **REQ-008**：模块应通过非阻塞状态机实现 ADC 控制，不在等待转换完成时阻塞 CPU。每次调用 `ADC_Control()` 应在 O(n) 时间内完成（n = 通道数）。

### 4.2 内存
- **REQ-009**：模块应为每个 ADC 通道分配独立的原始数据存储空间（`adc_adc1RawVoltages[MCU_ADC1_MAX_NR_CHANNELS]`）和一个数据块结构体（`adc_adc1Voltages`）。

### 4.3 鲁棒性
- **REQ-010**：模块应在状态机遇到无效状态时触发 `FAS_ASSERT(FAS_TRAP)` 陷阱，防止未定义行为。

### 4.4 可测试性
- **REQ-011**：模块应通过 `UNITY_UNIT_TEST` 条件编译暴露内部静态函数和变量，支持单元测试：
  - `TEST_ADC_ConvertVoltage()` — 测试电压转换函数
  - `TEST_ADC_SetAdcConversionState()` / `TEST_ADC_GetAdcConversionState()` — 测试状态机状态
  - `TEST_ADC_GetAdc1Voltages()` — 获取电压数据块指针

## 5. 接口需求

### 5.1 公共 API
| 函数 | 签名 | 说明 |
|------|------|------|
| `ADC_Control` | `extern void ADC_Control(void)` | 驱动 ADC 状态机执行一次测量循环 |

### 5.2 依赖项
| 依赖模块 | 头文件 | 用途 |
|----------|--------|------|
| HAL ADC | `HL_adc.h` | ADC 硬件寄存器操作 |
| 数据库 | `database.h` | 数据块读写 |
| 电池系统配置 | `battery_system_cfg.h` | ADC 通道数配置 |
| 标准库 | `<math.h>`, `<stdbool.h>`, `<stdint.h>` | 数学运算、类型定义 |

### 5.3 调用方
本模块被引擎层（ENGINE）周期性调用。

## 6. 数据结构

### ADC_STATE_e（ADC 状态枚举）
```c
typedef enum {
    ADC_START_CONVERSION,          // 启动转换
    ADC_WAIT_CONVERSION_FINISHED,  // 等待转换完成
    ADC_CONVERSION_FINISHED,       // 转换完成
} ADC_STATE_e;
```

### adcData_t（ADC 原始数据，由 HAL 定义）
存储单个 ADC 通道的原始转换结果。

### DATA_BLOCK_ADC_VOLTAGE_s（ADC 电压数据块）
| 字段 | 类型 | 说明 |
|------|------|------|
| header.uniqueId | - | 数据块标识 `DATA_BLOCK_ID_ADC_VOLTAGE` |
| adc1ConvertedVoltages_mV[] | float_t[] | 转换后的电压值数组 |

## 7. 状态机 / 控制流

```
START_CONVERSION → WAIT_CONVERSION_FINISHED → CONVERSION_FINISHED
      ↑                                                    |
      └────────────────────────────────────────────────────┘
```

## 8. 追溯矩阵

| 需求编号 | 需求描述 | 函数/宏 | 文件 |
|----------|----------|---------|------|
| REQ-001 | ADC 状态机控制 | `ADC_Control()` | adc.c:118-151 |
| REQ-002 | 转换完成检测 | `ADC_Control()` | adc.c:127-135 |
| REQ-003 | 原始数据读取 | `ADC_Control()`, `adcGetData()` | adc.c:139 |
| REQ-004 | 电压值转换 | `ADC_ConvertVoltage()` | adc.c:105-114 |
| REQ-005 | 数据写入数据库 | `ADC_Control()`, `DATA_WRITE_DATA()` | adc.c:143 |
| REQ-006 | 参考电压配置 | `ADC_VREFHIGH_mV`, `ADC_VREFLOW_mV` | adc.c:69,71 |
| REQ-007 | 分辨率配置 | `ADC_CONVERSION_FACTOR_12BIT/10BIT` | adc.c:73,75 |
| REQ-008 | 非阻塞状态机 | `ADC_Control()` | adc.c:118-151 |
| REQ-009 | 数据存储分配 | `adc_adc1RawVoltages[]`, `adc_adc1Voltages` | adc.c:88,90 |
| REQ-010 | 无效状态保护 | `FAS_ASSERT(FAS_TRAP)` | adc.c:148 |
| REQ-011 | 单元测试支持 | `TEST_ADC_*` 函数 | adc.c:155-167 |
