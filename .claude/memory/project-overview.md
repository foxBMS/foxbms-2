---
name: project-overview
description: foxBMS 2 项目概览 — 开源电池管理系统开发平台
metadata: 
  type: project
---

# foxBMS 2 项目概览

foxBMS 2 (v1.11.0) 是 Fraunhofer-Gesellschaft 开发的开源模块化电池管理系统 (BMS) 开发平台，支持锂离子/固态电池、钠离子电池、超级电容、液流电池和燃料电池等多种储能系统。

**许可证**: 软件 BSD-3-Clause，硬件和文档 CC-BY-4.0。OSHWA 认证 (UID: DE000128)。

## 仓库结构

| 目录 | 说明 |
|------|------|
| `src/` | 嵌入式 BMS 软件源代码 (C + FreeRTOS) |
| `cli/` | Python CLI 工具 (构建、烧录、CAN通信、调试) |
| `conf/` | 高级配置文件 |
| `docs/` | 文档源文件 |
| `hardware/` | 硬件原理图和布局 |
| `tests/` | 测试 (单元测试、集成测试、变体测试) |
| `tools/` | 构建和开发工具 (CRC、DBC、调试器) |

**目标平台**: TI C2000 微控制器 + TI HALCoGen 代码生成
**在线文档**: https://iisb-foxbms.iisb.fraunhofer.de/foxbms/gen2/docs/html/latest/

**近期版本历史**: v1.8.0 → v1.9.0 → v1.10.0 → v1.11.0 (当前)

[[code-conventions]] | [[architecture-modules]] | [[build-system-tools]]
