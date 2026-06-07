# 驱动层总览 — Driver Layer Overview

## 1. 概述

foxBMS 2 的 DRIVER 层位于应用层（APP）和硬件抽象层（HAL）之间，提供对外设和外部芯片的标准化驱动接口。驱动层按功能分为以下子系统。

## 2. 驱动模块清单

### 2.1 app/driver（应用驱动层）

| 模块 | 前缀 | 文件数 | 功能描述 | 需求文档 |
|------|------|--------|----------|----------|
| **ADC** | ADC | 2 | 模数转换器驱动，状态机控制电压采集 | [ADC](adc/ADC_SOFTWARE_REQUIREMENTS.md) |
| **AFE** | AFE | 117 | 模拟前端驱动（多供应商：ADI/LTC/Maxim/NXP/TI） | [AFE](afe/AFE_SOFTWARE_REQUIREMENTS.md) |
| **CAN** | CAN | 44 | CAN 总线通信，含 CBS 消息调度器 | [CAN](can/CAN_SOFTWARE_REQUIREMENTS.md) |
| **Config** | CFG | 33 | 各外设配置表（CAN/SPI/DMA/FRAM等） | （配置文件） |
| **Contactor** | CONT | 2 | 接触器控制与反馈诊断 | [CONTACTOR](contactor/CONTACTOR_SOFTWARE_REQUIREMENTS.md) |
| **CRC** | CRC | 2 | 硬件 CRC 校验 | [CRC](crc/CRC_SOFTWARE_REQUIREMENTS.md) |
| **DMA** | DMA | 2 | 直接存储器访问，SPI/I2C/UART DMA 传输 | [DMA](dma/DMA_SOFTWARE_REQUIREMENTS.md) |
| **EMAC** | EMAC | 4 | 以太网 MAC 驱动（DP83869 PHY） | [EMAC](emac/EMAC_SOFTWARE_REQUIREMENTS.md) |
| **Foxmath** | MATH/UTIL | 4 | 数学工具库（插值、字节序、随机数） | [FOXMATH](foxmath/FOXMATH_SOFTWARE_REQUIREMENTS.md) |
| **FRAM** | FRAM | 2 | 铁电存储器 SPI 读写（含 CRC 校验） | [FRAM](fram/FRAM_SOFTWARE_REQUIREMENTS.md) |
| **HTSENSOR** | HTSEN | 2 | SHT35-DIS I2C 温湿度传感器 | [HTSENSOR](htsensor/HTSENSOR_SOFTWARE_REQUIREMENTS.md) |
| **I2C** | I2C | 2 | I²C 总线驱动（阻塞+DMA 模式） | [I2C](i2c/I2C_SOFTWARE_REQUIREMENTS.md) |
| **IMD** | IMD | 12 | 绝缘监测（Bender IR155/ISO165C） | [IMD](imd/IMD_SOFTWARE_REQUIREMENTS.md) |
| **Interlock** | ILCK | 2 | 高压互锁回路检测 | [INTERLOCK](interlock/INTERLOCK_SOFTWARE_REQUIREMENTS.md) |
| **IO** | IO | 2 | 通用 I/O 引脚操作 | [IO](io/IO_SOFTWARE_REQUIREMENTS.md) |
| **LED** | LED | 2 | 调试 LED 状态指示 | [LED](led/LED_SOFTWARE_REQUIREMENTS.md) |
| **MCU** | MCU | 2 | MCU 工具函数（延时、超时、ID 读取） | [MCU](mcu/MCU_SOFTWARE_REQUIREMENTS.md) |
| **MEAS** | MEAS | 2 | 测量系统顶层调度 | [MEAS](meas/MEAS_SOFTWARE_REQUIREMENTS.md) |
| **PEX** | PEX | 2 | NXP PCA9539 端口扩展器 I2C 驱动 | [PEX](pex/PEX_SOFTWARE_REQUIREMENTS.md) |
| **PHY** | PHY | 2 | DP83869HM 以太网 PHY 驱动 | [PHY](phy/PHY_SOFTWARE_REQUIREMENTS.md) |
| **PWM** | PWM | 2 | ePWM 输出 + eCAP 输入捕获 | [PWM](pwm/PWM_SOFTWARE_REQUIREMENTS.md) |
| **RTC** | RTC | 2 | NXP PCF2131 实时时钟 I2C 驱动 | [RTC](rtc/RTC_SOFTWARE_REQUIREMENTS.md) |
| **SBC** | SBC | 11 | NXP FS85xx 系统基础芯片（看门狗、电压监控） | [SBC](sbc/SBC_SOFTWARE_REQUIREMENTS.md) |
| **SPI** | SPI | 3 | 多路 SPI 总线驱动（阻塞+DMA） | [SPI](spi/SPI_SOFTWARE_REQUIREMENTS.md) |
| **SPS** | SPS | 3 | 智能功率开关（接触器驱动） | [SPS](sps/SPS_SOFTWARE_REQUIREMENTS.md) |
| **TS** | TSI | 44 | 温度传感器（10 种 NTC 型号，查找表+多项式） | [TS](ts/TS_SOFTWARE_REQUIREMENTS.md) |
| **UART** | UART | 2 | RS232 串行通信（SCI+DMA，流控制） | [UART](uart/UART_SOFTWARE_REQUIREMENTS.md) |

### 2.2 bootloader/driver（引导加载器驱动层）

| 模块 | 功能 |
|------|------|
| **CAN** | 引导加载器 CAN 通信（含版本信息） |
| **CRC** | 硬件 CRC 校验（含 CRC64） |
| **Flash** | Flash 存储器编程/擦除 |
| **Foxmath** | 数学工具库（与 APP 相同） |
| **IO** | I/O 引脚操作 |
| **RTI** | 实时中断定时器 |

## 3. 驱动层架构层次

```
┌──────────────────────────────────────────────┐
│                 APPLICATION                   │
│  (algorithm, bal, bms, charger, ...)         │
├──────────────────────────────────────────────┤
│              ENGINE (engine/)                 │
│  (数据库、任务调度、状态机、诊断)              │
├──────────────────────────────────────────────┤
│              DRIVER (driver/)                 │  ← 本文档涵盖
│  ┌──────┐ ┌──────┐ ┌──────┐ ┌──────────┐   │
│  │ ADC  │ │ CAN  │ │ SPI  │ │   AFE    │   │
│  │ CRC  │ │ DMA  │ │ I2C  │ │  (ADI,   │   │
│  │ FRAM │ │ UART │ │ PEX  │ │   LTC,   │   │
│  │  IO  │ │ MCU  │ │ ...  │ │  Maxim,  │   │
│  │      │ │      │ │      │ │ NXP, TI) │  │   │
│  └──────┘ └──────┘ └──────┘ └──────────┘   │
├──────────────────────────────────────────────┤
│              HAL (HALCoGen)                   │
│  (HL_adc, HL_can, HL_spi, HL_i2c, ...)      │
├──────────────────────────────────────────────┤
│              HARDWARE (TMS570LC4357)           │
└──────────────────────────────────────────────┘
```

## 4. 需求编号规则

各模块需求文档使用 `REQ-NNN` 简单编号方案，在各自模块内独立编号。需求编号通过 `@requirements` / `@req` 注释标注在源代码中。

## 5. 共 27 个 SOFTWARE_REQUIREMENTS.md 文档

所有文档位于各自模块的源文件目录下。总计涵盖 200+ 个源文件（.c 和 .h）。
