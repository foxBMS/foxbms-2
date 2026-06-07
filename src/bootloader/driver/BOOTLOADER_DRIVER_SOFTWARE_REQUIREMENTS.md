# Bootloader 驱动层软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 Bootloader（引导加载器）驱动层的软件需求。Bootloader 驱动层提供引导加载器运行所需的最小外设驱动集。

### 1.2 范围
Bootloader 驱动层仅包含固件更新所需的核心驱动，不包括完整的 BMS 测量和控制功能。

## 2. 模块清单

| 模块 | 文件 | 功能 |
|------|------|------|
| **CAN** | can.c/h, can_helper.c/h, can_bootloader-version-info.c/h | CAN 通信，引导加载器协议消息收发 |
| **Config** | can_cfg.c/h, flash_cfg.c/h | CAN 消息和 Flash 配置表 |
| **CRC** | crc.c/h, vendor/crc64_calc.c/h | CRC32/CRC64 校验 |
| **Flash** | flash.c/h | 应用固件 Flash 编程/擦除/校验 |
| **Foxmath** | foxmath.c/h, utils.c/h | 数学工具库 |
| **IO** | io.c/h | I/O 引脚操作 |
| **MCU** | mcu.h | MCU 定义 |
| **RTI** | rti.c/h | 实时中断定时器 |

## 3. 核心功能需求

### 3.1 CAN 通信

#### REQ-BT-001 — CAN 消息发送
**优先级**：高 | **函数**：`CAN_SendMessage()`

模块应通过 CAN 总线发送 8 字节数据。

#### REQ-BT-002 — 引导消息发送
**优先级**：高 | **函数**：`CAN_SendBootMessage()`

模块应在启动时发送引导序列：Magic 起始 → 版本信息 → Commit Hash → Magic 结束。

#### REQ-BT-003 — 确认消息
**优先级**：高 | **函数**：`CAN_SendAcknowledgeMessage()`

模块应发送对主机命令的确认消息（含状态码和 yes/no 标志）。

#### REQ-BT-004 — 版本/信息发送
**优先级**：中 | **函数**：`CAN_SendBootloaderVersionInfo/CommitHash/Info()` , `CAN_SendDataTransferInfo()`

### 3.2 Flash 编程

#### REQ-BT-005 — Flash Bank 准备
**优先级**：高 | **函数**：`FLASH_PrepareFlashBank()`

激活指定的 Flash Bank 并使能相关扇区。

#### REQ-BT-006 — Flash 扇区擦除
**优先级**：高 | **函数**：`FLASH_EraseFlashSector()`, `FLASH_EraseFlashForApp()`

模块应擦除应用扇区（扇区 7~31）并为擦除后的区域计算 ECC（填充 0xFF）。

#### REQ-BT-007 — Flash 写入（含 ECC 自动生成）
**优先级**：高 | **函数**：`FLASH_WriteFlashWithEccAutoGeneration()`

模块应将数据写入 Flash 并自动生成 ECC 校验码。

#### REQ-BT-008 — Flash 扇区写入
**优先级**：高 | **函数**：`FLASH_WriteFlashSector()`

模块应通过扇区缓冲区（地址 0x08030000，大小 256KB）写入数据到 Flash。

#### REQ-BT-009 — Flash 编程校验
**优先级**：高 | **函数**：`FLASH_FlashProgramCheck()`

模块应在写入后校验 Flash 内容与源数据一致。

#### REQ-BT-010 — 扇区查询
**优先级**：中 | **函数**：`FLASH_GetFlashSector()`

### 3.3 CRC 校验

Bootloader CRC 模块支持 CRC64 计算（vendor/crc64_calc.c/h），用于固件完整性校验。

### 3.4 异常处理

Flash 操作异常通过 `EXCEPTION_CODES_e` 枚举定义：
- `INITIALIZATION_FAILED` / `FLASH_SECTOR_INVALID` / `WRONG_SECTOR_SIZE`
- `PREPARE_FLASH_BANK_FAILED` / `ERASING/WRITING_FLASH_SECTOR_FAILED` / `FLASH_CHECK_FAILED`

## 4. 配置参数

| 参数 | 值 |
|------|-----|
| 应用起始扇区 | 7 |
| 应用结束扇区 | 31 |
| 扇区缓冲区地址 | 0x08030000 |
| 扇区缓冲区大小 | 256 KB |
| 写入循环增量 | 4 字节 |

## 5. 接口需求

### CAN API
| 函数 | 说明 |
|------|------|
| `CAN_Initialize()` | 初始化 CAN 收发器 |
| `CAN_SendMessage(msg)` | 发送 CAN 消息 |
| `CAN_SendBootMessage()` | 发送引导序列 |
| `CAN_SendAcknowledgeMessage()` | 发送确认 |
| `CAN_SendBootloaderVersionInfo/CommitHash/Info()` | 版本信息 |
| `CAN_SendDataTransferInfo()` | 数据传输信息 |

### Flash API
| 函数 | 说明 |
|------|------|
| `FLASH_PrepareFlashBank()` | 准备 Flash Bank |
| `FLASH_EraseFlashSector()/FLASH_EraseFlashForApp()` | 擦除扇区 |
| `FLASH_WriteFlashWithEccAutoGeneration()` | 写入（含 ECC）|
| `FLASH_WriteFlashSector()` | 扇区写入 |
| `FLASH_FlashProgramCheck()` | 写入校验 |
| `FLASH_GetFlashSector()` | 扇区查询 |
