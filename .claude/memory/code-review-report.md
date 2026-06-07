---
name: code-review-report
description: MISRA C / CERT C 检查后必须生成代码审查报告并存放在源码文件所在目录的审查结果文件夹中
metadata:
  type: project
---

# 代码审查结果报告规范（项目要求）

**每次使用 [[misra-c-checklist]] 或 [[cert-c-checklist]] 检查代码后，必须生成一份代码审查结果报告。**

## 报告存放位置

在**被检查的源代码文件所在目录**下创建 `code-review-results/` 文件夹，将审查结果存放于此。

命名格式：`<源文件名>_review_<YYYY-MM-DD>.md`

示例：
```
src/app/application/bal/code-review-results/bal_review_2026-06-07.md
src/app/application/algorithm/soc/counting/code-review-results/soc_counting_review_2026-06-07.md
```

## 报告内容模板

```markdown
# 代码审查结果报告

- **审查文件**: `<文件路径>`
- **审查日期**: `<YYYY-MM-DD>`
- **审查标准**: MISRA C:2012 / CERT C 安全编码（或两者）
- **审查人**: AI 自动审查 (Claude Code)

## 审查摘要

| 类别 | 通过 | 违规 | 豁免 | 备注 |
|------|------|------|------|------|
| MISRA C 变量与类型 | | | | |
| MISRA C 条件控制 | | | | |
| MISRA C 循环控制 | | | | |
| MISRA C 函数 | | | | |
| MISRA C 条件进阶 | | | | |
| MISRA C 循环进阶 | | | | |
| MISRA C 指令 | | | | |
| CERT C 预处理器 | | | | |
| CERT C 声明 | | | | |
| CERT C 表达式 | | | | |
| CERT C 整数 | | | | |
| CERT C 浮点 | | | | |
| CERT C 数组 | | | | |
| CERT C 字符串 | | | | |
| CERT C 内存 | | | | |
| CERT C IO | | | | |
| CERT C 环境 | | | | |
| CERT C 信号 | | | | |
| CERT C 杂项 | | | | |

## 违规详情

### 违规 1
- **规则编号**: <MISRA/CERT 规则编号>
- **严重等级**: <Mandatory/Required/Advisory 或 L1/L2/L3>
- **位置**: <文件名:行号>
- **描述**: <违规内容描述>
- **修复建议**: <具体修复方式>
- **状态**: <待修复/已修复/已豁免>

### 违规 2
...

## 豁免说明（如有）
- <豁免理由、批准人、有效期>
```

## 执行原则

1. **每次必做**：使用 MISRA C 或 CERT C 检查清单审查代码后，必须生成报告，不得跳过
2. **就近存放**：报告放在被审查源文件所在目录下，便于代码与审查记录关联
3. **完整记录**：即使零违规，也应生成报告记录"全部通过"
4. **可追溯**：文件名含日期，同一文件多次审查保留历史记录

**Why:** 汽车级 BMS 开发需要完整的代码审查追溯记录。将审查报告存放在源文件同目录下，方便开发者查看、版本控制和审计追溯。

**How to apply:** 每次完成 MISRA C 或 CERT C 代码检查后，按模板生成审查报告，创建目标目录 `code-review-results/`（如不存在），写入报告文件。报告中的违规项需注明是否已修复。

相关记忆：[[misra-c-checklist]] [[cert-c-checklist]]
