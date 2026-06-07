---
name: software-requirements-docs
description: foxBMS 2 软件需求文档规范和编写流程
metadata: 
  type: project
---

# 软件需求文档规范

## 文档结构

每个主要模块在源码目录中有对应的 `*_SOFTWARE_REQUIREMENTS.md` 文件，使用中文编写。

**命名**: `<MODULE>_SOFTWARE_REQUIREMENTS.md` (如 `BMS_SOFTWARE_REQUIREMENTS.md`)

**文件头**:
```markdown
# <模块名> 驱动程序软件需求文档

**版本**: v1.11.0
**作者**: foxBMS Team
**日期**: 2026-04-20
**文件**: `<源文件>.c`
**模块**: <ENGINE/DRIVER>
**前缀**: <大写前缀>
```

## 标准章节

1. **概述** — 文件描述和主要功能列表
2. **功能需求** — 按功能域分组
   - 每个需求编号格式：`REQ-<路径前缀>_NNN` (如 `REQ-APP_APPLICATION_BAL_001`)
   - **路径前缀**由源文件在 `src/` 下的目录路径推导，各层级大写，下划线连接
   - 示例映射：
     | 源文件路径 | 前缀 | 首个需求编号 |
     |-----------|------|-------------|
     | `src/app/application/bal/bal.c` | `APP_APPLICATION_BAL` | `REQ-APP_APPLICATION_BAL_001` |
     | `src/app/application/bms/bms.c` | `APP_APPLICATION_BMS` | `REQ-APP_APPLICATION_BMS_001` |
     | `src/app/application/algorithm/state_estimation/soc/counting/soc_counting.c` | `APP_APPLICATION_ALGORITHM_STATE_ESTIMATION_SOC_COUNTING` | `REQ-APP_APPLICATION_ALGORITHM_STATE_ESTIMATION_SOC_COUNTING_001` |
     | `src/app/driver/afe/ti/dummy/ti_dummy.c` | `APP_DRIVER_AFE_TI_DUMMY` | `REQ-APP_DRIVER_AFE_TI_DUMMY_001` |
   - 需求描述采用规范句式："系统应能..."
3. **数据流程图** — 可选，描述模块间的数据流
4. **配置参数** — 可选，配置选项说明
5. **错误处理** — 可选，错误/诊断处理机制

## 当前状态

项目正在系统性地为模块补充需求文档。已有需求文档的模块包括:
- BMS 主状态机
- 状态估算 (SOC counting/debug/lookup-table/none, SOE counting/debug/none, SOF trapezoid, SOH debug/none)
- 电池均衡 (BAL, BAL voltage/history/none)
- 多类驱动 (ADC, CRC, DMA, FRAM, I2C, IO, SPI, LED, MCU, PWM, RTC, UART, CAN, etc.)
- 以太网、冗余、安全操作区 (SOA)、合理性检查 (plausibility)

## Doxygen 关联

源代码中通过 `@requirements` 标签关联需求，需求编号须使用完整路径前缀:
```c
/**
 * @requirements REQ-APP_APPLICATION_BAL_003, REQ-APP_APPLICATION_BAL_005
 */
```

### 注释层级

| 层级 | 位置 | 标签 | 格式 |
|------|------|------|------|
| 文件级 | 文件头注释块 | `@requirements` | `REQ-<前缀>_NNN` 逗号分隔 |
| 函数级 | Doxygen 注释块 | `@req` | `REQ-<前缀>_NNN` |
| 代码块级 | 行内注释 | `/* */` | `/* REQ-<前缀>_NNN: 说明 */` |
| 宏/变量级 | 行尾注释 | `/**< */` | `/**< REQ-<前缀>_NNN: 说明 */` |

[[code-conventions]] | [[architecture-modules]]
