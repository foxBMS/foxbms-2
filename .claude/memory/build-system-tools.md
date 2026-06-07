---
name: build-system-tools
description: foxBMS 2 构建系统、开发工具链和环境配置
metadata: 
  type: project
---

# 构建系统与工具链

## 核心构建

- **编译器**: TI Code Composer Studio (CCS) — 必须安装
- **代码生成**: TI HALCoGen — 用于外设配置代码生成
- **构建脚本**: wscript (waf 构建系统)，每个模块目录下有 `wscript`
- **Python CLI**: `cli/` 目录下的统一命令行工具

## Python CLI 工具

主入口: `cli/__main__.py`
命令模块:
- `cmd_build` — 构建管理
- `cmd_bootloader` — 启动加载器 (CAN 固件更新)
- `cmd_bms` — BMS 交互 shell
- `cmd_com` — 串口通信
- `cmd_db` — 数据库操作
- `cmd_embedded_ut` — 嵌入式单元测试
- `cmd_etl` — ETL (Extract/Transform/Load) CAN 数据处理
- `cmd_gui` — 图形界面 (数据绘图、引导加载程序、构建、模拟)
- `cmd_ide` — IDE 集成
- `cmd_install` — 安装管理
- `cmd_log` — 日志查看

## 开发环境

- **首选 IDE**: VS Code
- **Shell**: PowerShell 7+ (Windows) 或 Bash (Linux)
- **安装前缀**: Windows 上为 `C:\foxbms`
- **路径**: 不向 PATH 添加工具，保持隔离

## 测试工具

- **Ruby + Gems** — 单元测试框架
- **mingw-w64** — 单元测试编译
- **嵌入式测试**: 在目标硬件上运行
- **变体测试**: `tests/variants/` — 验证不同配置组合可编译

## 文档工具

- **Doxygen** — API 文档生成
- **Graphviz** — 图表
- **drawio-desktop** — 流程图
- **文档源**: `docs/` 目录 (.rst 格式)

## 硬件工具

- **调试器**: Lauterbach (`tools/debugger/lauterbach/`)
- **CAN 工具**: DBC 文件 (`tools/dbc/`)
- **CRC**: CRC 计算工具 (`tools/crc/`)

[[project-overview]] | [[architecture-modules]]
