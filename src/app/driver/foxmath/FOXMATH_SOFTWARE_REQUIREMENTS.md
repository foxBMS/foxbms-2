# FOXMATH — 数学工具库软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 Foxmath 数学工具库模块的软件需求。Foxmath 提供项目常用的数学函数，包括线性插值、字节序交换、最小值/绝对值计算、伪随机数生成和字符串处理等工具函数。

### 1.2 范围
- **涵盖**：线性插值、字节序交换（16/32/64 位）、最小值计算、绝对值计算、伪随机数生成、字符串字符提取、启动自检
- **不涵盖**：浮点运算硬件细节、CRC 计算

### 1.3 定义与缩略语
| 术语 | 说明 |
|------|------|
| FOXMATH | FoxBMS Math Library |
| UTIL | 通用工具函数（Utilities） |
| 线性插值 | Linear Interpolation，通过两点确定直线上第三点的值 |

## 2. 总体描述

Foxmath 是 foxBMS 的基础数学工具库，由 `foxmath.c/h` 和 `utils.c/h` 两个文件组成。位于 DRIVERS 层，供所有上层模块调用。

## 3. 功能需求

### 3.1 Foxmath 核心函数

#### REQ-001 — 线性插值
**优先级**：高 | **函数**：`MATH_LinearInterpolation()`

模块应根据给定的两点 (x1,y1) 和 (x2,y2)，计算 x_interpolate 处的线性插值/外推值。

#### REQ-002 — 16 位字节序交换
**优先级**：中 | **函数**：`MATH_SwapBytesUint16()`

模块应将 uint16_t 值的高字节与低字节交换：`0x1234 → 0x3412`。

#### REQ-003 — 32 位字节序交换
**优先级**：中 | **函数**：`MATH_SwapBytesUint32()`

模块应完全反转 uint32_t 值的字节顺序：`0x12345678 → 0x78563412`。

#### REQ-004 — 64 位字节序交换
**优先级**：中 | **函数**：`MATH_SwapBytesUint64()`

模块应完全反转 uint64_t 值的字节顺序。

#### REQ-005 — 浮点最小值
**优先级**：中 | **函数**：`MATH_MinimumOfTwoFloats()`

模块应返回两个 float_t 值中的较小值。

#### REQ-006 — 整数最小值
**优先级**：中 | **函数**：`MATH_MinimumOfTwoUint8_t()`, `MATH_MinimumOfTwoUint16_t()`

模块应返回两个无符号整数中的较小值。

#### REQ-007 — 整数绝对值
**优先级**：中 | **函数**：`MATH_AbsInt32_t()`, `MATH_AbsInt64_t()`

模块应返回有符号整数的绝对值。对于 INT32_MIN/INT64_MIN（无对应正数），返回 INT32_MAX/INT64_MAX。

#### REQ-008 — 启动自检
**优先级**：低 | **函数**：`MATH_StartupSelfTest()`

模块应在启动时可调用自检函数，验证数学函数的假设条件，违反时触发断言。

### 3.2 Utils 工具函数

#### REQ-009 — 字符串字符提取
**优先级**：中 | **函数**：`UTIL_ExtractCharactersFromString()`

模块应从字符串中连续提取最多 8 个字符写入 64 位变量中。若到达字符串末尾则提前结束。

#### REQ-010 — 伪随机数种子设置
**优先级**：低 | **函数**：`UTIL_SeedRandomNumber()`

模块应设置伪随机数生成器的种子值。

#### REQ-011 — 伪随机数生成
**优先级**：低 | **函数**：`UTIL_GetPseudoRandomNumber()`

模块应生成伪随机数。首次调用前必须先设置种子值。

### 3.3 单位转换宏

#### REQ-012 — 单位转换因子宏
**优先级**：中 | **宏定义**

模块应提供常用的单位转换因子宏：`UNIT_CONVERSION_FACTOR_*`（1e6, 1000, 100, 85, 0.001, 0.01, 0.1, 1, 10, 100, 1000）。

## 4. 非功能需求

- **REQ-013**：所有数学函数应为纯函数（无副作用），不修改全局状态
- **REQ-014**：函数时间复杂度为 O(1)

## 5. 接口需求

### 5.1 公共 API
| 函数 | 签名 | 说明 |
|------|------|------|
| `MATH_StartupSelfTest` | `extern void MATH_StartupSelfTest(void)` | 启动自检 |
| `MATH_LinearInterpolation` | `extern float_t MATH_LinearInterpolation(x1, y1, x2, y2, x)` | 线性插值 |
| `MATH_SwapBytesUint16/32/64` | 字节序交换 | 字节反转 |
| `MATH_MinimumOfTwoFloats` | `extern float_t MATH_MinimumOfTwoFloats(v1, v2)` | 浮点最小值 |
| `MATH_MinimumOfTwoUint8_t/16_t` | 整数最小值 | 整数比较 |
| `MATH_AbsInt32_t/64_t` | 整数的绝对值 | 绝对值 |
| `UTIL_ExtractCharactersFromString` | 字符串提取 | 字符提取 |
| `UTIL_SeedRandomNumber` / `UTIL_GetPseudoRandomNumber` | 伪随机数 | 随机数 |

### 5.2 依赖项
| 依赖模块 | 用途 |
|----------|------|
| `<math.h>`, `<float.h>`, `<stdlib.h>` | 标准数学库 |

## 6. 追溯矩阵

| 需求编号 | 需求描述 | 函数/宏 | 文件 |
|----------|----------|---------|------|
| REQ-001 | 线性插值 | `MATH_LinearInterpolation()` | foxmath.h:110-115 |
| REQ-002 | 16位字节交换 | `MATH_SwapBytesUint16()` | foxmath.h:121 |
| REQ-003 | 32位字节交换 | `MATH_SwapBytesUint32()` | foxmath.h:127 |
| REQ-004 | 64位字节交换 | `MATH_SwapBytesUint64()` | foxmath.h:133 |
| REQ-005 | 浮点最小值 | `MATH_MinimumOfTwoFloats()` | foxmath.h:141 |
| REQ-006 | 整数最小值 | `MATH_MinimumOfTwoUint8/16_t()` | foxmath.h:149,157 |
| REQ-007 | 整数绝对值 | `MATH_AbsInt32/64_t()` | foxmath.h:164,171 |
| REQ-008 | 启动自检 | `MATH_StartupSelfTest()` | foxmath.h:97 |
| REQ-009 | 字符串提取 | `UTIL_ExtractCharactersFromString()` | utils.h:98-103 |
| REQ-010 | 随机数种子 | `UTIL_SeedRandomNumber()` | utils.h:110 |
| REQ-011 | 伪随机数 | `UTIL_GetPseudoRandomNumber()` | utils.h:116 |
| REQ-012 | 单位转换宏 | `UNIT_CONVERSION_FACTOR_*` | foxmath.h:74-84 |
