# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概览

FoxBMS 2 是 Fraunhofer IISB 开发的开源模块化电池管理系统 (BMS) 开发平台（v1.11.0），许可证 BSD-3-Clause。支持锂电池、钠离子电池、超级电容、液流电池和燃料电池等多种储能系统。

**目标平台**: TI TMS570LC4357 (ARM Cortex-R5F) + FreeRTOS
**Host 工具**: Python CLI (`cli/`)，waf 构建系统 (`tools/waf`)，Sphinx + Doxygen 文档

## 常用命令

所有命令在仓库根目录执行。务必通过 shell wrapper 调用，它会激活正确的 Python 虚拟环境：

**Windows (PowerShell 7+)**:

```powershell
.\fox.ps1 waf configure          # 配置项目（生成构建变体）
.\fox.ps1 waf build_app_embedded # 编译嵌入式固件
.\fox.ps1 waf build_docs         # 构建文档
.\fox.ps1 waf clean              # 清理构建产物
.\fox.ps1 ceedling --project app # 嵌入式单元测试 (Ceedling)
.\fox.ps1 cli-unittest           # CLI 工具的 Python 单元测试
.\fox.ps1 pre-commit run --all-files  # 代码风格检查/格式化
.\fox.ps1 waf <subcommand>       # 执行任意 waf 命令
```

**Linux / Git Bash**:

```bash
./fox.sh waf configure
./fox.sh waf build_app_embedded
./fox.sh ceedling --project app
./fox.sh cli-unittest
```

**`fox.py` CLI 命令组**:

- `waf` — waf 构建系统（configure, build_app_embedded, build_docs, clean 等）
- `ceedling` — 嵌入式单元测试
- `cli-unittest` — CLI 工具自身的单元测试
- `bootloader` — 启动加载器/CAN 固件更新
- `bms` — BMS 交互式 shell
- `pre-commit` — 运行 pre-commit hooks
- `gui` — 图形界面（绘图、构建、模拟）
- `install` — 安装管理

Waf 是多步构建系统——先 `configure`，再 `build_app_embedded`。多个 waf 子命令可以串联执行。

## 高层架构

### 分层结构

```text
src/
├── app/
│   ├── application/    # BMS 应用逻辑（算法、均衡、BMS 状态机、SOA、合理性检查、冗余）
│   ├── driver/         # 硬件驱动（AFE、CAN、ADC、SPI、I2C、GPIO、PWM 等）
│   ├── engine/         # 数据管理引擎（database、diag、hw_info、sys_mon、sys）
│   ├── hal/            # 硬件抽象层（TI HALCoGen 生成）
│   ├── main/           # 入口：main.c，启动序列，断言
│   └── task/           # FreeRTOS 任务定义
├── bootloader/         # 基于 CAN 的启动加载器
├── os/                 # FreeRTOS 内核 + FreeRTOS-plus-tcp
├── portable/           # 处理器特定代码（TI TMS570）
└── opt/                # 可选组件（电池参数等）
```

### 关键架构模式

**策略模式（可替换实现）**: 多个模块支持运行时通过配置切换不同实现，每个策略一个子目录：

- **状态估算** (`algorithm/state_estimation/`): SOC (counting/lookup-table/debug/none)、SOE (counting/debug/none)、SOF (trapezoid)、SOH (debug/none)
- **电池均衡** (`bal/`): voltage / history / none
- **AFE 驱动** (`driver/afe/`): ADI (ADES1830) / NXP (MC33775A) / TI (dummy) / LTC (LTC68xx) / debug

**模块配置模式**: 每个模块通常有配套的 `config/` 子目录，包含 `*_cfg.c` / `*_cfg.h` 文件，以及一个 `wscript` 构建文件。

### 任务系统（FreeRTOS）

任务定义在 `src/app/task/ftask/ftask.h`：

- **Cyclic 1ms**: 高速周期性任务（传感器读取、AFE 通信）
- **Cyclic 10ms**: 中速周期性任务（CAN 通信、数据库更新）
- **Cyclic 100ms**: 低速周期性任务（系统监控、诊断）
- **Cyclic Algorithm 100ms**: 算法任务（状态估算、均衡决策）
- **Task Engine**: 数据库事件处理任务（高优先级）
- **Task I2C / AFE / UART / EMAC**: 异步通信任务

任务间通信通过 FreeRTOS 队列（`OS_QUEUE`），队列声明在 `ftask.h`。

### 配置系统

`conf/bms/bms.json` 定义 BMS 硬件和应用配置：

- 选择 AFE 芯片和制造商
- 选择 SOC/SOE/SOF/SOH 算法
- 选择均衡策略
- 电流传感器类型、温度传感器类型、IMD 类型

`conf/cc/` 定义编译器配置，`conf/hcg/` 定义 HALCoGen 配置。

### 数据库引擎

`src/app/engine/database/` 实现了核心数据管理层，所有模块通过队列向 database task 提交读写请求，database task 是唯一直接操作数据结构的实体。

## 代码规范

- **命名**: 文件名小写+下划线 (`soc_counting.c`)。每个模块有唯一大写前缀，函数名 `<PREFIX>_<Name>`。
- **Include guard**: `FOXBMS__<MODULE>_H_`（双下划线）
- **格式化**: `.clang-format`，C++ 基础风格，`AlignAfterOpenBracket: AlwaysBreak`，连续赋值对齐
- **文件头**: BSD-3-Clause 许可证 + Doxygen 文档块
- **代码分段**: 每段以分隔注释标记——`/*========== Includes ==========*/` 等

### 需求文档关联

每个模块源码目录下可有 `*_SOFTWARE_REQUIREMENTS.md`（中文），需求编号为 `REQ-<PATH_PREFIX>_NNN`。路径前缀从 `src/` 起每层大写并用下划线连接。如 `src/app/application/bal/bal.c` → `REQ-APP_APPLICATION_BAL_001`。源码中通过 `@requirements`（文件级）、`@req`（函数级）、行内注释关联。

### 代码生成后安全检查清单（强制）

**每次生成或修改 C 代码后，必须对照以下两项安全检查清单逐项验证：**

1. **MISRA C:2012 检查清单** — 汽车行业嵌入式 C 代码质量标准
   - 完整文档：[.claude/skills/MISRA_C_CheckList.md](.claude/skills/MISRA_C_CheckList.md)
   - 项目记忆：[[misra-c-checklist]]
   - 涵盖：143 条规则 + 16 条指令，六大章节（变量与类型、条件控制、循环控制、函数、条件进阶、循环进阶）

2. **CERT C 安全编码检查清单** — 卡内基梅隆大学 SEI 安全编码标准
   - 完整文档：[.claude/skills/CERT_C_CheckList.md](.claude/skills/CERT_C_CheckList.md)
   - 项目记忆：[[cert-c-checklist]]
   - 涵盖：13 大类（预处理器 PRE、声明 DCL、表达式 EXP、整数 INT、浮点 FLP、数组 ARR、字符串 STR、内存 MEM、IO FIO、临时文件 TMP、环境 ENV、信号 SIG、杂项 MSC）

**检查执行原则：**

- MISRA C 侧重代码质量和可移植性，CERT C 侧重安全漏洞防护，两者互补
- 优先确保高等级（L1/L2）规则和强制（Mandatory/Required）规则无违规
- 发现违规立即修正，不得在未处理违规的情况下提交代码
- 建议结合静态分析工具（Cppcheck、Clang Static Analyzer）自动化检测

**代码审查结果报告（强制）：**

每次使用 MISRA C 或 CERT C 检查清单审查代码后，**必须生成审查结果报告**：

- 报告存放位置：被审查源文件所在目录下创建 `code-review-results/` 文件夹
- 命名格式：`<源文件名>_review_<YYYY-MM-DD>.md`
- 报告模板和详细规范见：[.claude/memory/code-review-report.md](.claude/memory/code-review-report.md)
- 项目记忆：[[code-review-report]]
- 即使零违规也必须生成报告，确保完整的审查追溯链

## 编译工具链

- **编译器**: TI Code Composer Studio (CCS)，使用 TI ARM 编译器（`armcl`）
- **代码生成**: TI HALCoGen（外设配置 `.hcg` / `.dil` 文件）
- **构建系统**: waf（`tools/waf`），由 `wscript` 文件驱动，每个源模块一个 `wscript`
- **嵌入式测试**: Ceedling（基于 Ruby，使用 mingw-w64 编译）
- **目标二进制**: 通过硬件调试器（Lauterbach）烧录到 TI TMS570
- **文档**: Sphinx (RST) + Doxygen (C API)

## 自动记忆

在对话过程中，自动将重要的项目信息、架构决策、反馈和参考资料保存到项目记忆文件中。

**记忆分为两级：**

1. **项目记忆** — 位于 `.claude/memory/`，随仓库版本控制，供团队共享
2. **个人偏好** — 位于 `~/.claude/projects/E--Sandbox-FoxBMS2/memory/`，不提交到仓库

### 记忆格式

- 每个记忆一个 `.md` 文件，kebab-case 文件名
- 包含 frontmatter：`name`、`description`、`metadata`
- 在 `MEMORY.md` 中添加一行索引
- 用 `[[name]]` 链接相关记忆

### 注意事项

- 不要保存可从代码、git 历史或已有文档直接推导的信息
- 发现现有记忆有误时更新或删除，不创建重复记忆
