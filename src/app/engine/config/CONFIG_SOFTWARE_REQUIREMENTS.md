# 诊断配置 — 软件需求规格说明

## 1. 引言

诊断配置模块定义诊断系统的所有配置参数，包括诊断 ID 枚举、严重级别、阈值灵敏度宏、延时定义、诊断条目配置结构体和配置数组。

## 2. 功能需求

### REQ-APP_ENGINE_CONFIG_DIAG_CFG_001 — 诊断 ID 枚举定义

**编号** | REQ-APP_ENGINE_CONFIG_DIAG_CFG_001
**文件** | [diag_cfg.h](diag_cfg.h)
**描述**：系统应在 `DIAG_ID_e` 枚举中定义所有诊断事件 ID（如 `DIAG_ID_OVERVOLTAGE`、`DIAG_ID_UNDERVOLTAGE` 等），以 `DIAG_ID_MAX` 作为结束标记。

### REQ-APP_ENGINE_CONFIG_DIAG_CFG_002 — 灵敏度阈值配置

**编号** | REQ-APP_ENGINE_CONFIG_DIAG_CFG_002
**文件** | [diag_cfg.h](diag_cfg.h)
**描述**：系统应提供灵敏度阈值宏（`DIAG_SEN_EVENT_1` ~ `DIAG_SEN_EVENT_500`），定义在不同事件计数后开始记录的灵敏度等级。

### REQ-APP_ENGINE_CONFIG_DIAG_CFG_003 — 错误延时配置

**编号** | REQ-APP_ENGINE_CONFIG_DIAG_CFG_003
**文件** | [diag_cfg.h](diag_cfg.h)
**描述**：系统应定义状态转移延时（`DIAG_NO_DELAY`, `DIAG_DELAY_100ms` ~ `DIAG_DELAY_1000ms`），以及废弃延时标记 `DIAG_DELAY_DISCARD`（`UINT32_MAX`）。致命错误（`DIAG_FATAL_ERROR`）不允许配置 `DIAG_DELAY_DISCARD`。

### REQ-APP_ENGINE_CONFIG_DIAG_CFG_004 — 诊断配置表

**编号** | REQ-APP_ENGINE_CONFIG_DIAG_CFG_004
**文件** | [diag_cfg.c](diag_cfg.c)
**描述**：系统应在 `diag_diagnosisIdConfiguration[]` 数组中为每个诊断 ID 定义其配置：ID、严重级别（`DIAG_SEVERITY_LEVEL_e`）、延时（ms）、事件阈值、回调函数指针、评估启用标志等。
