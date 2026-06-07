---
name: write-software-requirements
description: 根据源代码文件自动生成标准化的软件需求规格文档和需求可追溯性映射文档。适用于嵌入式 C 项目（foxBMS 2）。支持 simple/standard/detailed 三种详细程度。
category: documentation
tools: Read, Write, Edit, Grep, Glob, Bash
---

# 软件需求文档编写技能

根据 foxBMS 2 项目代码自动生成标准化的软件需求规格文档（SRS）和需求可追溯性映射文档（RTM）。

## 触发方式

```
/write-software-requirements <源文件路径> [选项]
```

或在对话中直接说"为 xxx.c 生成软件需求文档"、"分析 xxx 模块的需求"。

## 输入参数

| 参数 | 必需 | 说明 |
|------|------|------|
| `source_file` | ✅ | 要分析的源代码文件路径（如 `bms.c`、`soc_counting.c`） |
| `mode` | ❌ | 生成模式，默认 `full`：`full`（需求文档+追溯映射）、`requirements`（仅需求文档）、`traceability`（仅追溯映射） |
| `detail` | ❌ | 详细程度，默认 `standard`：`simple`（简化版）、`standard`（标准版）、`detailed`（详细版含输入/输出/前置/后置条件） |

## 执行流程

### 第一步：源代码分析

阅读目标源文件，提取以下信息：

1. **文件元信息**：文件名、版本号、创建日期、最后更新日期、许可证
2. **模块定位**：所属层级（APP/ENGINE/DRIVER）、前缀、主要用途
3. **函数清单**：
   - 公开函数（extern）及其参数、返回值
   - 静态函数及其功能描述
   - 各函数的实现逻辑概览
4. **宏定义**：配置宏、计算宏、保护宏
5. **数据结构/全局变量**：状态结构体、数据表副本、静态变量
6. **依赖关系**：`#include` 的模块、调用的外部函数、数据库表依赖
7. **控制流**：状态机结构、初始化流程、运行时流程
8. **错误处理**：断言检查、错误返回、故障检测机制

### 第二步：提取需求

从代码分析中提取需求，遵循以下规则：

1. **需求编号**：
   - 编号格式：`REQ-<路径前缀>_NNN`（如 `REQ-APP_APPLICATION_BAL_001`）
   - **路径前缀**：由源文件在 `src/` 下的目录路径推导，各层级大写，用下划线连接
     - 格式：`<层级1>_<层级2>_<层级3>_<模块名>`
     - 示例：`src/app/application/bal/bal.c` → 前缀 `APP_APPLICATION_BAL`
     - 示例：`src/app/application/algorithm/state_estimation/soc/counting/soc_counting.c` → 前缀 `APP_APPLICATION_ALGORITHM_STATE_ESTIMATION_SOC_COUNTING`
     - 示例：`src/app/driver/afe/ti/dummy/ti_dummy.c` → 前缀 `APP_DRIVER_AFE_TI_DUMMY`
   - 自增编号 `NNN` 从 `001` 开始，每个文件内独立递增
   - 详细模式额外支持子编号：`FR-X.Y.Z` / `NFR-X.Y.Z` 置于前缀之后
   
2. **需求描述规范**：
   - 使用"系统应……"或"模块应……"句式
   - 每个需求独立、可测试、无歧义
   - 功能需求描述"做什么"而非"怎么做"
   
3. **需求分类**：
   - 按功能模块分组（如：状态机管理、数据采集、故障检测等）
   - 功能需求在前，非功能需求在后

4. **代码映射**：每个需求必须关联到具体的函数名、变量名或宏名

### 第三步：生成需求文档

根据 `detail` 参数，生成对应详细程度的需求文档。

#### 详细版 (detailed)

适用于核心算法模块。

```markdown
# [模块名] — 软件需求规格说明

## 1. 引言
### 1.1 目的
### 1.2 范围（涵盖/不涵盖）
### 1.3 定义与缩略语
### 1.4 参考文献

## 2. 总体描述
### 2.1 产品视角
### 2.2 工作模式
### 2.3 用户特征

## 3. 功能需求
### 3.1 [功能分组]
#### FR-3.1.1 — [需求标题]
**编号** | **优先级** | **函数**
**描述**
**输入**（参数表格）
**前置条件**
**处理流程**（编号步骤）
**后置条件**
**错误处理**

## 4. 非功能需求
### 4.1 时序与性能
### 4.2 内存
### 4.3 鲁棒性
### 4.4 可配置性
### 4.5 可测试性

## 5. 接口需求
### 5.1 公共 API
### 5.2 依赖项
### 5.3 调用方

## 6. 数据结构
## 7. 状态机 / 控制流
## 8. 追溯矩阵
```

### 第四步：为代码文件增加需求编号的注释，形成需求和代码对应

**直接在 `.C` 和 `.H` 源文件中添加需求编号注释**，而非单独生成映射说明文档。这样需求与代码的对应关系始终与代码共存，不会因文档分离而脱节。

#### 4.1 注释添加规则

1. **文件头注释**：在文件头部注释块中增加一行，列出本文件实现的所有需求编号。

   ```c
   /**
    * @brief   BMS 主控制模块
    * @file    bms.c
    * @requirements REQ-APP_APPLICATION_BMS_001, REQ-APP_APPLICATION_BMS_002, REQ-APP_APPLICATION_BMS_003
    */
   ```

2. **函数级注释**：在每个函数的 Doxygen 注释块中增加 `@req` 标签，标注该函数实现的需求编号。

   ```c
   /**
    * @brief   初始化 BMS 状态机
    * @req     REQ-APP_APPLICATION_BMS_001, REQ-APP_APPLICATION_BMS_002
    * @param   p_bms 指向 BMS 状态结构体的指针
    * @return  初始化结果状态
    */
   STD_RETURN_TYPE_e BMS_Initialize(BMS_STATE_s *p_bms) { ... }
   ```

3. **关键代码块内联注释**：在函数内部的关键逻辑分支处，以行内注释标注对应的需求编号。

   ```c
   if (voltage > threshold) {
       /* REQ-APP_APPLICATION_BMS_008: 过压检测 → 触发故障保护 */
       BMS_TriggerFault(FAULT_OVERVOLTAGE);
   }
   ```

4. **宏定义/配置常量注释**：在宏定义旁以行内注释标注对应的需求编号。

   ```c
   #define BMS_OVERVOLTAGE_THRESHOLD_mV  4200u  /**< REQ-APP_APPLICATION_BMS_008: 过压保护阈值 (mV) */
   #define BMS_UNDERVOLTAGE_THRESHOLD_mV 2800u  /**< REQ-APP_APPLICATION_BMS_009: 欠压保护阈值 (mV) */
   ```

#### 4.2 注释层级规范

| 层级 | 位置 | 标签格式 | 示例 |
| ---- | ---- | -------- | ---- |
| 文件级 | 文件头注释块 | `@requirements` | `@requirements REQ-APP_APPLICATION_BAL_001, REQ-APP_APPLICATION_BAL_002` |
| 函数级 | Doxygen 注释块 | `@req` | `@req REQ-APP_APPLICATION_BAL_001, REQ-APP_APPLICATION_BAL_002` |
| 代码块级 | 行内注释 | `/* REQ-<前缀>_NNN: ... */` | `/* REQ-APP_APPLICATION_BMS_005: 状态切换 */` |
| 宏/变量级 | 行尾注释 | `/**< REQ-<前缀>_NNN: ... */` | `/**< REQ-APP_APPLICATION_BAL_012: 最大重试次数 */` |

#### 4.3 操作步骤

1. 使用 `Read` 工具读取目标 `.C` 和 `.H` 文件
2. 使用 `Edit` 工具在相应位置插入需求编号注释
3. 确保注释格式与文件现有风格一致（对齐、缩进、注释符风格）
4. 不修改任何代码逻辑，仅添加注释

#### 4.4 注意事项

- 仅修改需求文档对应的 `.C` 和 `.H` 文件，不修改其他文件
- 保持原有注释内容不变，仅在适当位置补充需求编号
- 如果一个函数实现多个需求，用逗号分隔所有编号
- 需求编号必须与第二步中生成的需求文档中的编号完全一致

### 第五步：输出位置

生成的文件放置在源文件所在目录：
- 需求文档：`<源文件目录>/<模块前缀>_SOFTWARE_REQUIREMENTS.md` 或 `<源文件基础名>_requirements.md`

## 需求编写规范

### 需求编号规则

**前缀推导**: 从源文件在 `src/` 下的路径提取目录层级，大写，下划线连接。

| 源文件路径 | 前缀 | 需求编号示例 |
|-----------|------|-------------|
| `src/app/application/bal/bal.c` | `APP_APPLICATION_BAL` | `REQ-APP_APPLICATION_BAL_001` |
| `src/app/application/bms/bms.c` | `APP_APPLICATION_BMS` | `REQ-APP_APPLICATION_BMS_001` |
| `src/app/application/algorithm/state_estimation/soc/counting/soc_counting.c` | `APP_APPLICATION_ALGORITHM_STATE_ESTIMATION_SOC_COUNTING` | `REQ-APP_APPLICATION_ALGORITHM_STATE_ESTIMATION_SOC_COUNTING_001` |
| `src/app/driver/afe/ti/dummy/ti_dummy.c` | `APP_DRIVER_AFE_TI_DUMMY` | `REQ-APP_DRIVER_AFE_TI_DUMMY_001` |
| `src/app/driver/can/can.c` | `APP_DRIVER_CAN` | `REQ-APP_DRIVER_CAN_001` |

**编号模式**:

| 模式 | 编号格式 | 示例 |
|------|---------|------|
| simple | `REQ-<前缀>_NNN` | `REQ-APP_APPLICATION_BAL_001` |
| standard | `REQ-<前缀>_NNN`（模块内递增） | `REQ-APP_APPLICATION_BAL_057` |
| detailed | `REQ-<前缀>_NNN` + `FR-X.Y.Z` / `NFR-X.Y.Z` | `REQ-APP_APPLICATION_BMS_FR-3.2.1` |

### 需求质量准则

1. **原子性**：每个需求描述一个且仅一个功能点
2. **可测试性**：需求描述应能直接映射为测试用例
3. **无歧义性**：避免"可能""应该""大约"等模糊词汇
4. **可追溯性**：每个需求必须能追溯到至少一个代码位置
5. **完整性**：覆盖所有公开函数、所有错误路径、所有配置参数

### 中文写作规范

- 所有文档内容使用中文
- 技术术语首次出现时标注英文原文，如"库仑计数（Coulomb Counting）"
- 代码标识符保持原样，不翻译
- 使用"系统应……"句式描述功能性需求

### 代码分析深度

从源代码中提取需求时，应关注：

1. **函数级别**：每个函数的职责、输入输出、边界条件
2. **分支级别**：if/else 分支对应的业务规则
3. **循环级别**：循环中体现的检查/处理逻辑
4. **宏定义**：配置参数对应的业务含义
5. **断言**：系统假设和前置条件
6. **注释**：开发者意图和设计决策
7. **TODO/FIXME**：已知缺陷和待实现功能