# FRAM — 铁电存储器驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 FRAM（Ferroelectric Random Access Memory，铁电随机存储器）驱动模块的软件需求。FRAM 模块负责通过 SPI 接口对外部铁电存储器进行读写操作，并提供 CRC 数据完整性校验。

### 1.2 范围
- **涵盖**：FRAM 初始化、数据块写入（含 CRC 计算）、数据块读取（含 CRC 校验）、SPI 总线锁定管理
- **不涵盖**：FRAM 硬件引脚配置、SPI 底层传输协议

### 1.3 定义与缩略语
| 术语 | 说明 |
|------|------|
| FRAM | Ferroelectric RAM，铁电随机存储器 |
| CRC | Cyclic Redundancy Check，循环冗余校验 |
| Block ID | 数据块标识符，对应 FRAM 中存储的每条数据 |
| CS | Chip Select，片选信号 |

### 1.4 参考文献
- FRAM 芯片数据手册
- foxBMS 数据库框架 (`fram_cfg.h`)

## 2. 总体描述

### 2.1 产品视角
FRAM 模块位于 DRIVERS 层，通过 SPI 接口与外部 FRAM 芯片通信。依赖 CRC 模块进行数据完整性校验，依赖 SPI 模块进行总线仲裁。

### 2.2 工作模式
- **初始化模式**：计算并分配各数据块在 FRAM 中的地址
- **写入模式**：先写入使能 → 发送写命令 + 地址 + CRC + 数据
- **读取模式**：发送读命令 + 地址 → 读取 CRC + 数据 → CRC 校验

## 3. 功能需求

### 3.1 FRAM 初始化与地址管理

#### REQ-001 — FRAM 地址初始化
**优先级**：高 | **函数**：`FRAM_Initialize()`

模块应根据 FRAM 配置表中的数据块长度，自动计算并分配每个数据块的存储地址。首块从地址 0 开始，后续块地址 = 前一块地址 + 数据长度 + CRC 头大小。

#### REQ-002 — FRAM 容量校验
**优先级**：高 | **函数**：`FRAM_Initialize()`

模块应在初始化时断言检查所有数据块的总大小不超过 FRAM 最大地址（`FRAM_MAX_ADDRESS = 0x03FFFF`）。

#### REQ-003 — FRAM 版本信息读取
**优先级**：中 | **函数**：`FRAM_Initialize()`

模块应在初始化时读取 FRAM 版本信息数据块，若 CRC 校验失败则触发诊断错误。

### 3.2 FRAM 数据写入

#### REQ-004 — FRAM 数据写入（含 CRC）
**优先级**：高 | **函数**：`FRAM_WriteData()`

模块应将指定数据块写入 FRAM，写入流程为：
1. 计算数据 CRC 校验值
2. 锁定 SPI 总线
3. 发送写使能命令（`0x06`）+ 延时 5μs
4. 发送写命令（`0x02`）+ 24 位地址（高→中→低字节）
5. 发送 8 字节 CRC
6. 发送数据内容
7. 释放片选，解锁 SPI 总线

**前置条件**：`blockId < FRAM_BLOCK_MAX`，FRAM 使用软件片选模式。

**错误处理**：
- 若 CRC 计算模块忙 → 返回 `FRAM_ACCESS_CRC_BUSY`
- 若 SPI 总线被锁定 → 返回 `FRAM_ACCESS_SPI_BUSY`

#### REQ-005 — FRAM 全量重写
**优先级**：中 | **函数**：`FRAM_ReinitializeAllEntries()`

模块应支持重新初始化所有 FRAM 数据块：先更新版本信息，再逐个写入所有数据块。任一写入失败返回 `STD_NOT_OK`。

### 3.3 FRAM 数据读取

#### REQ-006 — FRAM 数据读取（含 CRC 校验）
**优先级**：高 | **函数**：`FRAM_ReadData()`

模块应读取指定数据块并进行完整性校验：
1. 锁定 SPI 总线
2. 发送读命令（`0x03`）+ 24 位地址
3. 读取 8 字节 CRC
4. 读取数据内容
5. 释放片选，解锁 SPI
6. 重新计算 CRC 并与读取的 CRC 比较
7. 不匹配时触发诊断错误

**错误处理**：
- CRC 校验失败 → 返回 `FRAM_ACCESS_CRC_ERROR`，触发 `DIAG_ID_FRAM_READ_CRC_ERROR`
- CRC 模块忙 → 返回 `FRAM_ACCESS_CRC_BUSY`
- SPI 忙 → 返回 `FRAM_ACCESS_SPI_BUSY`

### 3.4 FRAM 命令定义

#### REQ-007 — FRAM 操作码配置
**优先级**：中 | **宏定义**：`FRAM_WRITE_COMMAND`, `FRAM_READ_COMMAND`, `FRAM_WRITE_ENABLE_COMMAND`

模块应通过宏定义提供 FRAM 操作码：
- 写使能：`0x06`
- 读数据：`0x03`
- 写数据：`0x02`

## 4. 非功能需求

- **REQ-008**：FRAM 使用软件片选（`SPI_CHIP_SELECT_SOFTWARE`），禁止硬件片选
- **REQ-009**：写使能后需等待 `FRAM_DELAY_AFTER_WRITE_ENABLE_US`（5μs）
- **REQ-010**：最大 FRAM 地址空间为 `0x03FFFF`（256KB）

## 5. 接口需求

### 5.1 公共 API
| 函数 | 说明 |
|------|------|
| `FRAM_Initialize()` | 初始化 FRAM 地址表和版本信息 |
| `FRAM_ReinitializeAllEntries()` | 重写所有 FRAM 数据块 |
| `FRAM_WriteData(blockId)` | 写入指定数据块 |
| `FRAM_ReadData(blockId)` | 读取并校验指定数据块 |

### 5.2 依赖项
| 依赖模块 | 用途 |
|----------|------|
| `fram_cfg.h` | FRAM 数据块配置表 |
| `crc.h` | CRC 硬件计算 |
| `spi.h` | SPI 总线传输和锁定 |
| `io.h` | 片选引脚控制 |
| `diag.h` | 诊断错误报告 |

## 6. 追溯矩阵

| 需求编号 | 需求描述 | 函数 | 文件 |
|----------|----------|------|------|
| REQ-001 | 地址初始化 | `FRAM_Initialize()` | fram.c:99-117 |
| REQ-002 | 容量校验 | `FRAM_Initialize()` | fram.c:111 |
| REQ-003 | 版本信息读取 | `FRAM_Initialize()` | fram.c:114-116 |
| REQ-004 | 数据写入（含 CRC） | `FRAM_WriteData()` | fram.c:136-208 |
| REQ-005 | 全量重写 | `FRAM_ReinitializeAllEntries()` | fram.c:119-134 |
| REQ-006 | 数据读取（含 CRC 校验） | `FRAM_ReadData()` | fram.c:210-288 |
| REQ-007 | 操作码配置 | 宏定义 | fram.c:79-81 |
