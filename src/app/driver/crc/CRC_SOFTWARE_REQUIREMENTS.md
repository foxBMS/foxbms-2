# CRC — 循环冗余校验驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 CRC（Cyclic Redundancy Check，循环冗余校验）驱动模块的软件需求。CRC 模块利用 MCU 硬件 CRC 引擎对数据流进行完整性校验计算。

### 1.2 范围
- **涵盖**：硬件 CRC 初始化（种子值配置、模式切换）、64 位数据分组输入、剩余字节处理、CRC 结果读取
- **不涵盖**：CRC 多项式配置（由 HAL 层负责硬件初始化）、软件 CRC 计算

### 1.3 定义与缩略语
| 术语 | 说明 |
|------|------|
| CRC | Cyclic Redundancy Check，循环冗余校验 |
| PSA | Parallel Signature Analysis，并行签名分析寄存器 |
| 种子值（Seed） | CRC 计算的初始值 |
| Full-CPU Mode | CPU 完全控制模式，写入数据即开始 CRC 计算 |
| Data Capture Mode | 数据捕获模式，用于写入种子值而不触发计算 |

### 1.4 参考文献
- MCU 硬件 CRC 模块技术参考手册
- `HL_crc.h` HAL 层 CRC 寄存器定义

## 2. 总体描述

### 2.1 产品视角
CRC 模块位于 DRIVERS 层，直接操作 MCU 的 CRC 硬件寄存器（`crcREG1`）。提供单一公开接口 `CRC_CalculateCrc()` 供上层模块（如数据库完整性校验）调用。

### 2.2 工作模式
1. **Data Capture Mode**：写入 CRC 种子值
2. **Full-CPU Mode**：写入数据触发 CRC 硬件计算

CRC 模块通过 `crcCalls` 计数器确保单次调用互斥（不可重入）。

### 2.3 用户特征
供 foxBMS 内部模块调用，主要用于数据块完整性校验。

## 3. 功能需求

### 3.1 CRC 硬件计算

#### REQ-001 — CRC 硬件计算
**编号** | **优先级** | **函数**
REQ-001 | 高 | `CRC_CalculateCrc()`

**描述**：模块应利用 MCU 硬件 CRC 引擎计算给定数据流的 CRC 校验值。输入为字节数组，输出为 64 位 CRC 结果。

**输入**：
| 参数 | 类型 | 说明 |
|------|------|------|
| pCrc | uint64_t* | 指向 CRC 计算结果存储位置的指针 |
| pData | uint8_t* | 指向待计算数据流的起始指针 |
| lengthInBytes | uint32_t | 数据流长度（字节数） |

**返回值**：`STD_OK` 表示计算成功，`STD_NOT_OK` 表示 CRC 硬件正忙。

**前置条件**：
- `pCrc` 和 `pData` 均不得为 NULL
- CRC 硬件当前未被占用（`crcCalls == 0`）

**处理流程**：
1. 断言检查输入指针非空
2. 检查 crcCalls 计数器，确保不重入
3. 设置 CRC 为 Data Capture Mode，写入 64 位种子值
4. 切换为 Full-CPU Mode
5. 按 64 位（8 字节）分组处理输入数据，处理大小端字节序
6. 处理剩余不足 64 位的尾部数据（补零填充）
7. 若仅有低 32 位数据，单独写入 PSA 寄存器触发计算
8. 读取 64 位 CRC 结果
9. 重置 crcCalls 计数器

**后置条件**：CRC 结果写入 `*pCrc`，CRC 硬件恢复可用状态。

**错误处理**：
- 若 `pCrc == NULL` 或 `pData == NULL`，触发 `FAS_ASSERT` 断言
- 若 CRC 硬件正忙（`crcCalls != 0`），返回 `STD_NOT_OK`

### 3.2 配置与参数

#### REQ-002 — CRC 种子值配置
**编号** | **优先级** | **宏定义**
REQ-002 | 中 | `CRC_SEED_HIGH`, `CRC_SEED_LOW`

**描述**：模块应通过宏定义提供 CRC 计算的 64 位种子值：
- 高 32 位：`0xBADC0DED`
- 低 32 位：`0xDEADBEEF`

#### REQ-003 — CRC 硬件模式切换
**编号** | **优先级** | **宏定义**
REQ-003 | 高 | `CRC_DATA_CAPTURE_MODE_CLEAR_MASK`, `CRC_FULL_CPU_MODE_SET_MASK`

**描述**：模块应通过配置寄存器位掩码在 Data Capture Mode（写入种子）和 Full-CPU Mode（写入数据触发计算）之间切换。

#### REQ-004 — CRC 数据输入大小端处理
**编号** | **优先级** | **函数**
REQ-004 | 高 | `CRC_CalculateCrc()`（数据打包循环）

**描述**：模块应在将数据写入 CRC 硬件寄存器之前进行字节序反转处理，以适配硬件的大端（Big Endian）要求。每个 32 位数据块中的 4 个字节按逆序排列。

#### REQ-005 — CRC 剩余字节处理
**编号** | **优先级** | **函数**
REQ-005 | 高 | `CRC_CalculateCrc()`（剩余字节处理循环）

**描述**：当输入数据长度不是 64 位的整数倍时，模块应将剩余字节打包为 32 位数据并补零填充后写入 CRC 硬件。

#### REQ-006 — CRC 单次调用互斥
**编号** | **优先级** | **静态变量**
REQ-006 | 高 | `crcCalls`

**描述**：模块应通过 `crcCalls` 静态计数器确保 `CRC_CalculateCrc()` 函数不可重入。在首次调用完成之前，后续调用应返回 `STD_NOT_OK`。

## 4. 非功能需求

### 4.1 时序与性能
- **REQ-007**：CRC 硬件计算为同步操作，不阻塞其他模块。每次 `CRC_CalculateCrc()` 调用的时间复杂度为 O(n)，其中 n 为数据长度。

### 4.2 内存
- **REQ-008**：模块不应使用动态内存分配。所有变量使用栈分配或静态分配。

### 4.3 鲁棒性
- **REQ-009**：模块应对输入指针进行非空断言检查（`FAS_ASSERT`），防止空指针导致的未定义行为。

### 4.4 可配置性
- **REQ-010**：种子值（`CRC_SEED_HIGH`/`CRC_SEED_LOW`）应在编译时通过宏定义配置。

### 4.5 可测试性
- **REQ-011**：模块应在 `UNITY_UNIT_TEST` 条件下：
  - 外部化 `crcCalls` 变量供测试代码访问
  - 提供 `TEST_CRC_SetCrcCalls()` 函数设置调用计数
  - 使用模拟寄存器（`test_register`）替代硬件寄存器

## 5. 接口需求

### 5.1 公共 API
| 函数 | 签名 | 说明 |
|------|------|------|
| `CRC_CalculateCrc` | `extern STD_RETURN_TYPE_e CRC_CalculateCrc(uint64_t *pCrc, uint8_t *pData, uint32_t lengthInBytes)` | 计算数据流的 CRC 校验值 |

### 5.2 依赖项
| 依赖模块 | 头文件 | 用途 |
|----------|--------|------|
| HAL CRC | `HL_crc.h` | CRC 硬件寄存器定义 |
| 标准类型 | `fstd_types.h` | `STD_RETURN_TYPE_e` 定义 |
| 断言 | `fassert.h` | `FAS_ASSERT` 运行时断言 |
| 标准库 | `<stdint.h>` | 定长整数类型 |

### 5.3 调用方
数据库完整性校验模块、数据块验证模块。

## 6. 数据结构

### CRC_REGISTER_SIDE_e（寄存器侧枚举）
```c
typedef enum {
    CRC_REGISTER_LOW,   // 低 32 位寄存器待写入
    CRC_REGISTER_HIGH,  // 高 32 位寄存器待写入
} CRC_REGISTER_SIDE_e;
```

### 单元测试模拟寄存器
```c
#ifdef UNITY_UNIT_TEST
crcBASE_t test_register;
#define crcREG1 ((crcBASE_t *)&test_register)
#endif
```

## 7. 状态机 / 控制流

```
输入：pData[], lengthInBytes
  │
  ├─ 断言检查 (pCrc != NULL, pData != NULL)
  ├─ 重入检查 (crcCalls == 0?)
  │   └─ 否 → 返回 STD_NOT_OK
  ├─ 设置 Data Capture Mode → 写入种子值
  ├─ 设置 Full-CPU Mode
  ├─ 循环处理 64 位数据块（含字节序反转）
  ├─ 处理剩余字节（< 8 字节）
  ├─ 若仅低 32 位可用 → 写入触发 CRC 计算
  ├─ 读取 64 位 CRC 结果 → 写入 *pCrc
  └─ 重置 crcCalls → 返回 STD_OK
```

## 8. 追溯矩阵

| 需求编号 | 需求描述 | 函数/宏 | 文件 |
|----------|----------|---------|------|
| REQ-001 | CRC 硬件计算 | `CRC_CalculateCrc()` | crc.c:75-168 |
| REQ-002 | 种子值配置 | `CRC_SEED_HIGH`, `CRC_SEED_LOW` | crc.h:69,71 |
| REQ-003 | 硬件模式切换 | `CRC_DATA_CAPTURE_MODE_CLEAR_MASK`, `CRC_FULL_CPU_MODE_SET_MASK` | crc.h:73,75 |
| REQ-004 | 大小端处理 | `CRC_CalculateCrc()`（数据打包） | crc.c:111-128 |
| REQ-005 | 剩余字节处理 | `CRC_CalculateCrc()`（尾部处理） | crc.c:133-152 |
| REQ-006 | 单次调用互斥 | `crcCalls` | crc.c:80,91,161 |
| REQ-007 | O(n) 时间复杂度 | `CRC_CalculateCrc()` | crc.c:75-168 |
| REQ-008 | 无动态内存分配 | 全部 | crc.c |
| REQ-009 | 空指针断言 | `FAS_ASSERT` | crc.c:76-77 |
| REQ-010 | 编译时配置 | `CRC_SEED_HIGH`/`CRC_SEED_LOW` | crc.h:69,71 |
| REQ-011 | 单元测试支持 | `TEST_CRC_SetCrcCalls`, `test_register` | crc.h:98-100,123 |
