---
name: architecture-modules
description: foxBMS 2 软件架构、模块分层和目录组织
metadata: 
  type: project
---

# 架构与模块组织

## 分层架构

```
src/
├── app/                    # 应用层
│   ├── application/        # BMS 核心应用逻辑
│   │   ├── algorithm/      # 状态估计算法 (SOC/SOE/SOF/SOH)
│   │   ├── bal/            # 电池均衡 (balancing)
│   │   ├── bms/            # BMS 主状态机
│   │   ├── config/         # 电池系统配置
│   │   ├── plausibility/   # 合理性检查 (plausibility)
│   │   ├── redundancy/     # 冗余管理
│   │   ├── soa/            # 安全操作区 (Safe Operating Area)
│   │   └── ethernet/       # 以太网通信
│   ├── driver/             # 驱动层
│   │   ├── adc/            # ADC 驱动
│   │   ├── afe/            # 模拟前端 (AFE) — ADI/NXP/TI/debug
│   │   ├── can/            # CAN 通信
│   │   ├── contactor/      # 接触器驱动
│   │   ├── crc/            # CRC 校验
│   │   ├── dma/            # DMA 驱动
│   │   ├── fram/           # FRAM (铁电存储器)
│   │   ├── interlock/      # 互锁
│   │   ├── io/             # GPIO
│   │   ├── i2c/            # I2C 总线
│   │   ├── meas/           # 测量
│   │   ├── mcu/            # MCU 驱动
│   │   ├── pex/            # 引脚扩展 (Pin Extension)
│   │   ├── phy/            # 以太网 PHY
│   │   ├── pwm/            # PWM
│   │   ├── rtc/            # 实时时钟
│   │   ├── sbc/            # 系统基础芯片 (System Basis Chip)
│   │   ├── spi/            # SPI 总线
│   │   ├── sps/            # 安全电源开关
│   │   ├── uart/           # UART
│   │   └── ...             # 其他驱动
│   ├── main/               # 主程序入口
│   ├── task/               # FreeRTOS 任务
│   └── doxygen_app.h       # Doxygen 主文档
├── bootloader/             # 启动加载器 (CAN 升级)
├── os/                     # FreeRTOS + FreeRTOS-plus-tcp
├── portable/               # TI C2000 处理器特定代码
└── opt/                    # 可选代码 (如特定电池参数)
```

## 策略模式设计

多个模块采用"策略模式"实现可替换配置：

**状态估算 (state_estimation)**:
- `soc/` — SOC 估算: `counting/` (库仑计数), `lookup-table/` (查表法), `debug/`, `none/`
- `soe/` — SOE 估算: `counting/`, `debug/`, `none/`
- `sof/` — SOF 估算: `trapezoid/` (梯形积分)
- `soh/` — SOH 估算: `debug/`, `none/`

**均衡 (bal)**:
- `history/` — 历史数据均衡策略
- `voltage/` — 电压均衡策略
- `none/` — 无均衡

**AFE 驱动 (afe)**:
- `adi/` — ADI 芯片 (ADES1830)
- `nxp/` — NXP 芯片 (MC33775A)
- `ti/` — TI 芯片 (dummy 实现)
- `debug/` — 调试用虚拟 AFE

## 配置文件模式

每个模块通常有对应的配置层:
- `config/` 子目录包含 `*_cfg.c` / `*_cfg.h`
- 电池配置: `battery_cell_cfg`, `battery_system_cfg`
- `wscript` 文件用于 waf 构建系统

[[project-overview]] | [[code-conventions]] | [[build-system-tools]]
