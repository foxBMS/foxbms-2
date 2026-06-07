---
name: code-conventions
description: foxBMS 2 C 代码规范、命名约定和文档标准
metadata: 
  type: project
---

# 代码规范与约定

## 文件头格式

每个源文件须包含：
1. **BSD-3-Clause 版权声明** — 标准三条款BSD许可证文本
2. **Doxygen 文件文档块** — 包含：
   - `@file` — 文件名
   - `@author` — foxBMS Team
   - `@date` — 创建日期
   - `@updated` — 最后更新日期
   - `@version` — 当前版本 (v1.11.0)
   - `@ingroup` — 所属 Doxygen 分组
   - `@prefix` — 模块前缀 (如 ALGO, BMS, TIDUM)
   - `@brief` — 简要描述
   - `@details` — 详细描述
   - `@requirements` — 关联需求编号 (如 REQ-003, REQ-005)
3. **Include guard**: `FOXBMS__<MODULE>_H_` 格式 (双下划线)

## 命名约定

- **文件名**: 小写 + 下划线 (`soc_counting.c`, `ti_dummy.h`)
- **模块前缀**: 每个模块有唯一大写前缀，用于函数和类型
  - ALGO (Algorithm), BMS (Battery Management), SOC, SOE, SOF, SOH, BAL
  - TIDUM (TI Dummy AFE), ADI (Analog Devices AFE), NXP (NXP AFE)
  - DIAG (Diagnostic), CAN, SPI, I2C, UART, ADC, PWM
- **函数命名**: `<PREFIX>_<FunctionName>` (如 `ALGO_MainFunction`, `BMS_Trigger`)
- **全局常量/宏**: 大写 + 下划线 (`BS_NR_OF_STRINGS`)

## 代码格式化

- 使用 **clang-format** (配置在 `.clang-format`)
- C++ 风格 (Language: Cpp)
- 连续赋值对齐、连续宏对齐
- `AccessModifierOffset: -2`, `AlignAfterOpenBracket: AlwaysBreak`

## 代码结构

```
/*========== Includes =======================================================*/
/* clang-format off */
#include <标准库>
/* clang-format on */
#include "自定义头文件"

/*========== Macros and Definitions =========================================*/

/*========== Extern Constant and Variable Declarations ======================*/

/*========== Extern Function Prototypes =====================================*/
```

## 函数文档规范

```c
/**
 * @brief   简要描述
 * @param   参数名 参数说明
 * @return  返回值说明
 * @req     REQ-xxx
 */
extern void PREFIX_FunctionName(void);
```

[[project-overview]] | [[architecture-modules]] | [[software-requirements-docs]]
