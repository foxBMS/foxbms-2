---
name: misra-c-checklist
description: MISRA C:2012 代码规范检查清单 — 代码生成后必须对照检查的强制性项目要求
metadata:
  type: project
---

# MISRA C:2012 规范检查清单（项目要求）

**所有新生成或修改的 C 代码必须通过此清单检查。** 完整清单文档位于 [.claude/skills/MISRA_C_CheckList.md](.claude/skills/MISRA_C_CheckList.md)，共 143 条规则 + 16 条指令（Directives）。

## 规则体系

- **Mandatory（强制）**：必须遵守，不可例外
- **Required（必需）**：必须遵守，特殊情况可经例外批准
- **Advisory（建议）**：推荐遵守

## 六大规则章节

### 一、变量与类型（Rule 1.1–1.9）
- 变量必须显式初始化（Mandatory）
- 使用固定宽度整数类型 `uint8_t`/`uint16_t`/`uint32_t`（Mandatory）
- 避免隐式/危险类型转换，signed/unsigned 不混用（Required）
- 使用 `const` 或枚举替代 `#define` 魔法数字（Required）
- 结构体字段必须显式初始化（Required）
- 跨文件变量通过 getter/setter 接口访问（Required）
- 避免变量隐藏 shadowing（Required）
- 命名反映用途和作用域，全局加 `g_`，静态加 `s_`（Advisory）

### 二、条件控制（Rule 2.1–2.18）
- 禁止条件中赋值 `if(a=b)`（Mandatory）
- 条件必须明确，`if((status & FLAG) != 0)`（Mandatory）
- if/else 必须用花括号（Required）
- switch 必须有 default，禁止 fall-through（Required）
- 条件中禁止魔法数字，禁止副作用（Required）
- 复杂条件拆分为子表达式（Advisory）

### 三、循环控制（Rule 3.1–3.22）
- 循环终止条件必须可达（Mandatory）
- 循环计数器固定宽度类型（Mandatory）
- 循环体加花括号，禁止内部修改外部依赖（Required）
- 避免死循环，必要时加 Watchdog 超时（Required）
- 循环中禁止动态内存分配、禁止浮点变量（Required）
- 嵌套深度 ≤2，函数调用注意执行时间（Advisory）

### 四、函数（Rule 4.1–4.30）
- 函数功能单一，名称清晰（Mandatory）
- 参数固定宽度类型，只读参数用 `const`（Required）
- 禁止返回局部变量地址、禁止递归（Required）
- 函数内动态内存分配禁止（Required）
- 返回值必须检查，异常路径必须处理（Mandatory）
- 长度 ≤50 行，参数数量 ≤4（Advisory）

### 五、条件控制进阶（Rule 5.1–5.40）
- 条件边界检查 `if((index>=0) && (index<ARRAY_SIZE))`（Mandatory）
- 嵌套深度 ≤3 层（Required）
- 使用括号明确优先级，避免短路副作用（Required）
- switch 覆盖所有枚举值（Advisory）

### 六、循环控制进阶（Rule 6.1–6.23）
- 循环中禁止修改计数器（Required）
- 循环变量类型匹配，避免溢出（Mandatory）
- 禁止空循环体，禁止阻塞调用（Required）
- 嵌套深度 >3 应拆分（Required）

## 16 条指令（Directives）
涵盖：可移植性、头文件自包含、注释规范、代码可读性、预处理器使用、编译器依赖、可测试性、错误处理、函数接口、变量类型、循环控制、宏定义、静态分析兼容、库函数、接口文档、安全审查。

## 补充规范
- **空格规范**：关键字后加空格、二元运算符两边空格、一元运算符紧贴操作数、逗号后空格、函数名与括号间无空格
- **复杂条件多行分解**：运算符置于行首（推荐），或使用临时布尔变量提升可读性

**Why:** foxBMS 2 是汽车级 BMS 嵌入式系统，运行于 TI TMS570LC4357 (ARM Cortex-R5F)，代码安全性和可靠性至关重要。MISRA C:2012 是汽车行业嵌入式 C 语言的事实标准。

**How to apply:** 每次生成或修改 C 代码后，逐章对照检查清单验证合规性。优先确保 Mandatory 和 Required 级别规则无违规，Advisory 级别尽量满足。发现违规立即修正后再提交。**检查完成后必须生成代码审查结果报告**，存放于源文件所在目录的 `code-review-results/` 文件夹中。

相关记忆：[[cert-c-checklist]] [[code-review-report]]
