# MAIN 模块 — 软件需求规格说明

**版本：** v1.11.0  
**日期：** 2026-06-06  
**文件：** `main.c`, `fassert.c`, `fstartup.c` 及对应头文件  
**所属层级：** APP（应用层）  
**模块编号：** MAIN  

---

## 1. 引言

### 1.1 目的

本文档描述 foxBMS 2 系统中 MAIN 模块（主控启动模块）的软件需求规格。MAIN 模块是系统的启动入口，负责硬件初始化、系统启动流程、断言系统和通用基础设施定义。本文档旨在为开发、测试和验证提供需求追溯基准。

### 1.2 范围

**涵盖：**

- `main.c` / `main.h` — 主程序入口与硬件初始化
- `fassert.c` / `fassert.h` — 运行时和编译时断言系统
- `fstartup.c` / `fstartup.h` — CPU 复位向量与启动例程
- `general.h` — 通用位操作宏、重复宏、括号剥离宏及平台定义
- `fstd_types.h` — foxBMS 标准数据类型定义
- `fsystem.h` — 系统权限模式切换（用户/特权）及 ISR 任务切换
- `infinite-loop-helper.h` — 可测试的无限循环辅助宏
- `app_build_cfg.h` — 应用构建配置枚举与结构体

**不涵盖：**

- OS 层（FreeRTOS/SafeRTOS 内部实现）
- 驱动层（ADC、SPI、I2C、DMA、PWM 等驱动实现）
- HAL 层（`HL_*` 硬件抽象层）
- 应用算法模块（SOC/SOH/SOE/SOF 算法实现）

### 1.3 定义与缩略语

| 缩略语 | 英文全称 | 中文说明 |
|--------|---------|----------|
| AFE | Analog Front End | 模拟前端采集芯片 |
| CCM | Cortex-R5 Cache Coherency Module | 缓存一致性模块 |
| ECC | Error Correcting Code | 纠错码 |
| ESM | Error Signaling Module | 错误信号模块 |
| IRQ | Interrupt Request | 中断请求 |
| ISR | Interrupt Service Routine | 中断服务例程 |
| MPU | Memory Protection Unit | 内存保护单元 |
| PLL | Phase-Locked Loop | 锁相环 |
| RTOS | Real-Time Operating System | 实时操作系统 |
| SPSR | Saved Program Status Register | 保存程序状态寄存器 |
| VIM | Vectored Interrupt Manager | 向量中断管理器 |
| SOC/SOH/SOE/SOF | State of Charge/Health/Energy/Function | 荷电/健康/能量/功能状态 |

### 1.4 参考文献

- TI ARM CGT 编译器手册 SPNU151V
- foxBMS 2 系统架构文档
- MISRA-C:2012 编码规范

---

## 2. 总体描述

### 2.1 产品视角

MAIN 模块处于 foxBMS 2 软件栈的最底层应用层位置，是系统启动后首先执行的模块。系统复位后由 `_c_int00()` 启动例程完成 CPU 级初始化，随后调用 `main()` 进行外设初始化和 RTOS 启动。

### 2.2 工作模式

- **上电复位（Power-On Reset）**：完整初始化，包括内存初始化和系统时钟配置
- **外部复位（External Reset）**：内存初始化 + 系统时钟配置 + ESM/VIM 初始化
- **调试复位（Debug Reset）**：与外部复位相同
- **看门狗复位（Watchdog Reset）**：不执行完整初始化（保持已配置外设状态）
- **软件复位（SW Reset）**：不执行完整初始化
- **CPU核心复位（CPU0 Reset）**：仅使能 CPU 事件总线导出
- **振荡器故障复位（OSC Failure Reset）**：不执行初始化

### 2.3 用户特征

本模块面向嵌入式固件开发人员、测试工程师和系统集成工程师。使用前需理解 ARM Cortex-R5 架构、TI HALCoGen 代码生成工具以及 foxBMS 2 构建配置系统。

---

## 3. 功能需求

### 3.1 主程序入口（main.c / main.h）

#### REQ-001 — 系统硬件初始化序列

**编号** | **优先级** | **实现函数**
REQ-001 | 高 | `main()`

**描述：**  
系统应按预定顺序初始化所有硬件外设模块，包括：引脚复用（Pin Muxing）→ GPIO → SPI → ADC → HET → eTPWM → CRC → 调试 LED → I2C → DMA → UART（条件编译）→ PWM → 诊断模块 → 数学自检 → 以太网（条件编译）。

**前置条件：**
- CPU 已由 `_c_int00()` 完成内核级初始化
- 系统时钟已配置

**处理流程：**
1. 调用 `MINFO_SetResetSource(getResetSource())` 记录复位源
2. 依次初始化各外设模块
3. 条件编译模块（UART、TCP/Ethernet）仅在对应宏定义时初始化
4. 调用 `MATH_StartupSelfTest()` 执行数学库自检
5. 完成外设初始化后进入 OS 初始化阶段

**后置条件：**
- 所有使能的外设模块处于就绪状态
- 调试 LED 已点亮

**错误处理：**
- 各驱动初始化函数内部处理错误（如配置验证）

---

#### REQ-002 — 操作系统初始化与启动

**编号** | **优先级** | **实现函数**
REQ-002 | 高 | `main()`

**描述：**  
系统应在硬件初始化完成后，通过 OS 时间自检验证后启动实时操作系统（RTOS）调度器。启动 RTOS 调度器后程序不应返回；若返回，则应返回非零错误码。

**前置条件：**
- 所有外设模块初始化完成
- `OS_CheckTimeHasPassedSelfTest()` 返回 `STD_OK`

**处理流程：**
1. 调用 `OS_CheckTimeHasPassedSelfTest()` 执行 OS 时间自检
2. 通过 `FAS_ASSERT` 断言自检结果
3. 调用 `OS_InitializeOperatingSystem()` 初始化 OS
4. 调用 `_enable_IRQ_interrupt_()` 使能 IRQ 中断
5. 检查 OS 启动状态（`os_boot` 必须为 `OS_INIT_PRE_OS`）
6. 记录调度器启动时间戳 `os_schedulerStartTime`
7. 调用 `OS_StartScheduler()` 启动调度器

**后置条件：**
- RTOS 调度器接管控制权，进入多任务运行状态
- 如调度器启动失败，系统不应继续执行

**错误处理：**
- `OS_CheckTimeHasPassedSelfTest()` 失败 → `FAS_ASSERT` 断言触发
- OS 初始化失败（`os_boot != OS_INIT_PRE_OS`）→ `FAS_ASSERT(FAS_TRAP)` 触发

---

#### REQ-003 — 复位源信息记录

**编号** | **优先级** | **实现函数**
REQ-003 | 中 | `main()`

**描述：**  
系统应在 main 函数入口处读取并记录系统复位源信息，供后续诊断使用。

**处理流程：**
1. 调用 `getResetSource()` 获取复位源
2. 将复位源传递给 `MINFO_SetResetSource()` 记录到主信息模块

**错误处理：**
- 无（复位源读取为硬件寄存器操作）

---

#### REQ-004 — 中断使能时序控制

**编号** | **优先级** | **实现函数**
REQ-004 | 高 | `main()`

**描述：**  
系统应在 AFE 任务创建之后才使能 IRQ 中断，以防止 DMA 中断在 AFE 任务句柄为 NULL 时被触发而导致的空指针访问。

**前置条件：**
- `OS_InitializeOperatingSystem()` 调用完成，AFE 任务已创建

**处理流程：**
1. 在 `OS_InitializeOperatingSystem()` 之后调用 `_enable_IRQ_interrupt_()`

**后置条件：**
- 中断系统已使能，DMA 中断可安全触发

---

#### REQ-005 — 条件编译外设支持

**编号** | **优先级** | **实现函数**
REQ-005 | 中 | `main()`

**描述：**  
系统应支持通过编译宏 `FOXBMS_UART_SUPPORT` 和 `FOXBMS_TCP_SUPPORT` 条件性地使能 UART 和以太网外设的初始化，以适应不同硬件配置。

**前置条件：**
- 构建系统中已定义对应的配置宏
- 相应初始化函数（`UART_Initialize()`, `ETH_Initialize()`）已链接

**处理流程：**
1. 预处理阶段检查 `#if (defined(FOXBMS_UART_SUPPORT) && (FOXBMS_UART_SUPPORT == 1))`
2. 条件性调用 `UART_Initialize()`
3. 预处理阶段检查 `#if (defined(FOXBMS_TCP_SUPPORT) && (FOXBMS_TCP_SUPPORT == 1))`
4. 条件性调用 `ETH_Initialize()`

---

#### REQ-006 — 单元测试构建兼容

**编号** | **优先级** | **实现函数**
REQ-006 | 中 | `main()` / `main.h`

**描述：**  
系统应在单元测试构建模式下将 `main()` 函数重命名为 `unit_test_main()`，以允许测试框架定义自己的 `main()` 入口，使模块可独立进行单元测试。

**处理流程：**
1. 预处理阶段检查 `#ifndef UNITY_UNIT_TEST`
2. 嵌入式构建时函数名为 `main`
3. 单元测试构建时函数名为 `unit_test_main`

---

### 3.2 断言系统（fassert.c / fassert.h）

#### REQ-007 — FAS_ASSERT 运行时断言

**编号** | **优先级** | **实现宏**
REQ-007 | 高 | `FAS_ASSERT(x)`

**描述：**  
系统应提供运行时断言宏 `FAS_ASSERT(x)`，用于检查程序运行时应始终为真的条件。断言失败时，系统应记录断言位置并根据当前断言级别采取相应措施（进入无限循环/禁用中断/无操作）。在单元测试构建模式下，断言失败应抛出 CException 异常以支持断言失败的测试验证。

**前置条件：**
- `FAS_ASSERT_LEVEL` 宏已定义（若未定义则使用默认值）

**输入：**

| 参数 | 类型 | 说明 |
|------|------|------|
| `x` | `bool` 可求值表达式 | 应被断言为真的条件表达式 |

**处理流程：**
1. 求值条件表达式 `x`
2. 若 `x` 为假：
   - 调用 `FAS_ASSERT_RECORD()` 记录断言位置
   - 调用 `FAS_InfiniteLoop()` 进入指定行为
3. 若 `x` 为真：无操作，程序继续执行

**后置条件：**
- 断言通过：程序正常继续
- 断言失败（嵌入式模式）：程序停止在无限循环中
- 断言失败（单元测试模式）：抛出异常供测试捕获

---

#### REQ-008 — 断言级别配置

**编号** | **优先级** | **实现函数/宏**
REQ-008 | 高 | `FAS_InfiniteLoop()`, `FAS_ASSERT_LEVEL_*`

**描述：**  
系统应支持三种断言行为级别，通过 `FAS_ASSERT_LEVEL` 宏配置：  

| 级别 | 值 | 行为 |
|------|-----|------|
| `FAS_ASSERT_LEVEL_INF_LOOP_AND_DISABLE_INTERRUPTS` | 0 | 禁用中断后进入无限循环（触发看门狗复位） |
| `FAS_ASSERT_LEVEL_INF_LOOP_FOR_DEBUG` | 1 | 进入无限循环（调试模式，可触发看门狗复位） |
| `FAS_ASSERT_LEVEL_NO_OPERATION` | 2 | 无操作（仅记录断言位置） |

若 `FAS_ASSERT_LEVEL` 未在构建系统中定义，系统应默认使用 `FAS_ASSERT_LEVEL_INF_LOOP_AND_DISABLE_INTERRUPTS`（级别0）。若定义了无效值，编译器应产生错误。

**处理流程：**
1. 编译时通过 `#if` 预处理器指令选择 `FAS_InfiniteLoop()` 实现
2. 无效级别时触发 `#error` 编译错误

**错误处理：**
- 无效的 `FAS_ASSERT_LEVEL` 值 → 编译错误

---

#### REQ-009 — 断言位置记录

**编号** | **优先级** | **实现宏/函数**
REQ-009 | 中 | `FAS_ASSERT_RECORD()`, `FAS_StoreAssertLocation()`

**描述：**  
系统应在每次断言触发时记录断言发生的位置信息，包括程序计数器（PC）地址和源代码行号。记录的信息存储在 `FAS_ASSERT_LOCATION_s` 结构体中。此记录功能在所有断言级别下均执行。

**数据结构：**

| 字段 | 类型 | 说明 |
|------|------|------|
| `pc` | `uint32_t *` | 断言触发时的程序计数器地址 |
| `line` | `uint32_t` | 断言触发的源代码行号 |

**处理流程：**
1. 通过 `__curpc()` 获取当前程序计数器值
2. 通过 `__LINE__` 获取当前行号
3. 调用 `FAS_StoreAssertLocation(pc, line)` 存储位置信息

**后置条件：**
- `FAS_ASSERT_LOCATION_s` 结构体被填充为最新断言的位置

---

#### REQ-010 — FAS_TRAP 无条件断言失败

**编号** | **优先级** | **实现宏**
REQ-010 | 中 | `FAS_TRAP`

**描述：**  
系统应提供 `FAS_TRAP` 宏，其求值结果为布尔假值（`0u == 1u`）。当作为参数传递给 `FAS_ASSERT()` 时，将无条件触发断言失败。该宏用于标记代码中不应到达的逻辑路径。

**处理流程：**
1. `FAS_TRAP` 展开为表达式 `(0u == 1u)`，始终为 `false`
2. 传递给 `FAS_ASSERT(FAS_TRAP)` 即触发断言失败

---

#### REQ-011 — FAS_STATIC_ASSERT 静态断言

**编号** | **优先级** | **实现宏**
REQ-011 | 中 | `FAS_STATIC_ASSERT(cond, msg)`

**描述：**  
系统应提供编译时静态断言宏 `FAS_STATIC_ASSERT(cond, msg)`。当编译器支持 `_Static_assert`（C11/C17 及 TI 编译器 C99 模式）时，宏映射到标准 `_Static_assert`；当编译器不支持时（C89/C90、普通 C99、C94），宏展开为空并产生编译警告。

**处理流程：**
1. 检查 `__STDC_VERSION__` 确定 C 标准版本
2. C11(`201112L`) 或 C17(`201710L`)：映射到 `_Static_assert(cond, msg)`
3. C99(`199901L`) + TI 编译器：映射到 `_Static_assert(cond, msg)`
4. 其他情况：展开为空，输出 `#warning` 提示

**错误处理：**
- 不支持的 C 标准版本 → 编译警告并忽略静态断言

---

#### REQ-012 — 中断禁用机制

**编号** | **优先级** | **实现函数**
REQ-012 | 高 | `FAS_DisableInterrupts()`

**描述：**  
系统应通过汇编实现的 `FAS_DisableInterrupts()` 函数禁用所有中断。该函数通过 `#pragma SWI_ALIAS` 映射到汇编实现，写入 SPSR 寄存器的控制域掩码字节 PSR[7:0] 来禁用中断。

**处理流程：**
1. `FAS_ASSERT_LEVEL` 为 0 级别时，断言失败调用 `FAS_DisableInterrupts()`
2. 汇编函数写入 SPSR 寄存器以禁用中断

**后置条件：**
- 所有中断被禁用，系统进入无限循环等待看门狗复位

---

### 3.3 启动例程（fstartup.c / fstartup.h）

#### REQ-013 — CPU 复位入口（_c_int00）

**编号** | **优先级** | **实现函数**
REQ-013 | 高 | `_c_int00()`

**描述：**  
系统复位后应首先执行 `_c_int00()` 函数作为复位向量入口。该函数负责 CPU 内核初始化、复位原因识别及分级初始化，最终调用应用主函数 `main()`。

**前置条件：**
- 硬件触发复位信号

**处理流程：**
1. 调用 `_coreInitRegisters_()` 初始化内核寄存器
2. 调用 `_coreInitStackPointer_()` 初始化堆栈指针
3. 调用 `STU_GetResetSourceWithoutFlagReset()` 读取复位源（不清除标志位）
4. 根据复位源类型执行分级初始化
5. 调用 `_mpuInit_()` 初始化内存保护单元
6. 调用 `__TI_auto_init()` 初始化全局变量和构造函数
7. 调用 `main()` 进入应用入口
8. `main()` 正常返回后调用 `exit(0)`

**后置条件：**
- 应用主函数 `main()` 被调用

---

#### REQ-014 — 复位源检测（不清除标志）

**编号** | **优先级** | **实现函数**
REQ-014 | 高 | `STU_GetResetSourceWithoutFlagReset()`

**描述：**  
系统应能从 SYSESR（系统异常状态寄存器）中读取复位原因，且不立即清除对应标志位。复位源优先级从高到低依次为：上电复位 → 振荡器故障复位 → 看门狗复位 → 看门狗2复位 → 软件复位 → 外部复位 → 调试复位 → CPU0复位。

**输入：** 无

**返回值：**

| 返回值 | 含义 |
|--------|------|
| `POWERON_RESET` | 上电复位 |
| `EXT_RESET` | 外部复位 |
| `OSC_FAILURE_RESET` | 振荡器故障复位 |
| `WATCHDOG_RESET` | 看门狗复位 |
| `WATCHDOG2_RESET` | 看门狗2复位 |
| `SW_RESET` | 软件复位 |
| `DEBUG_RESET` | 调试复位请求 |
| `CPU0_RESET` | CPU0复位 |
| `NO_RESET` | 未发生复位 |

**处理流程：**
1. 检查 `SYS_EXCEPTION & POWERON_RESET`
2. 按优先级链逐级检查各复位标志
3. 返回检测到的最高优先级复位源

---

#### REQ-015 — 上电复位初始化

**编号** | **优先级** | **实现函数**
REQ-015 | 高 | `_c_int00()`

**描述：**  
系统在上电复位（POWERON_RESET）时应执行完整的初始化流程：内存初始化（避免 ECC 错误）→ CPU 事件总线导出使能 → ESM 组3错误检查 → 系统时钟配置（含 eFuse 自检，最多重试5次）→ IRQ VIC 偏移使能 → VIM 表初始化 → ESM 初始化。

**前置条件：**
- 复位源为 `POWERON_RESET`

**处理流程：**
1. `_memInit_()` 初始化 L2RAM
2. `_coreEnableEventBusExport_()` 使能 CPU 事件总线导出
3. 检查 `esmREG->SR1[2]`：若非零，调用 `esmGroup3Notification()`
4. `systemInit()` 初始化系统时钟和 Flash（含 eFuse 自检）
5. `_coreEnableIrqVicOffset_()` 使能 IRQ VIC 偏移
6. `vimInit()` 初始化向量中断表
7. 再次检查 ESM 组3错误并调用 `esmInit()`

**错误处理：**
- ESM 组3错误 → `esmGroup3Notification()` 报警（设备不可靠）
- `systemInit()` 内部 PLL 锁定重试最多 5 次（宏 `STU_PLL_RETRIES`）

---

#### REQ-016 — ESM 错误检测与通知

**编号** | **优先级** | **实现函数**
REQ-016 | 高 | `_c_int00()`

**描述：**  
系统应在启动过程中检查 ESM（Error Signaling Module）组3状态寄存器，检测上电期间是否发生 ECC 错误。如果检测到错误，应调用 `esmGroup3Notification()` 通知系统。

**处理流程：**
1. 读取 `esmREG->SR1[2]`，检查是否非零
2. 若检测到错误，调用 `esmGroup3Notification(esmREG, esmREG->SR1[2])`

---

#### REQ-017 — CPU 事件总线导出使能

**编号** | **优先级** | **实现函数**
REQ-017 | 高 | `_c_int00()`

**描述：**  
系统应在复位初始化过程中使能 CPU 事件总线导出功能。该功能使 CPU 能检测到程序 Flash 或数据 RAM 访问中的单比特或双比特 ECC 错误，并将错误信号导出到 ESM。

**前置条件：**
- 复位源为 `POWERON_RESET`、`EXT_RESET`、`DEBUG_RESET` 或 `CPU0_RESET`

---

#### REQ-018 — MPU 和 VIM 初始化

**编号** | **优先级** | **实现函数**
REQ-018 | 高 | `_c_int00()`

**描述：**  
系统应在所有复位类型的处理完成后调用 `_mpuInit_()` 初始化内存保护单元（MPU）。在上电复位、外部复位和调试复位类型下，额外调用 `vimInit()` 初始化向量中断管理器（VIM）。

**前置条件：**
- 对应复位类型的初始化流程已执行完毕

**处理流程：**
1. 各复位分支执行完毕
2. 调用 `_mpuInit_()`（所有复位类型均执行）
3. 调用 `__TI_auto_init()` 初始化全局变量

---

### 3.4 通用宏定义（general.h）

#### REQ-019 — 寄存器位操作宏

**编号** | **优先级** | **实现宏**
REQ-019 | 中 | `GEN_SET_BIT()`, `GEN_CLEAR_BIT()`

**描述：**  
系统应提供 `GEN_SET_BIT(register, bit)` 和 `GEN_CLEAR_BIT(register, bit)` 宏，用于对寄存器进行位设置（置1）和位清除（置0）操作。操作应保证无符号整型运算，避免有符号移位导致的未定义行为。

**输入：**

| 参数 | 类型 | 说明 |
|------|------|------|
| `register` | `uint32` | 目标寄存器 |
| `bit` | uint32 可求值表达式 | 要设置或清除的位编号 |

**处理流程：**
- `GEN_SET_BIT`：`(register) |= (uint32)((uint32)1u << (bit))`
- `GEN_CLEAR_BIT`：`(register) &= ~(uint32)((uint32)1u << (bit))`

---

#### REQ-020 — 令牌重复宏

**编号** | **优先级** | **实现宏**
REQ-020 | 中 | `GEN_REPEAT_U(x, n)`, `GEN_REPEAT_U*u(x)`

**描述：**  
系统应提供 `GEN_REPEAT_U(x, n)` 宏，用于在数组初始化器中生成指定数量的重复字面量序列。该宏支持最多 99 次重复（`GEN_REPEAT_MAXIMUM_REPETITIONS`），通过链式宏展开实现。典型用法为初始化动态大小的数组为非零默认值。

**输入：**

| 参数 | 类型 | 说明 |
|------|------|------|
| `x` | 令牌 | 要重复的令牌（如 `true`、`false`） |
| `n` | 无符号整数字面量 | 重复次数（≤99） |

**使用示例：**
```c
#define ARRAY_SIZE (4u)
bool variable[ARRAY_SIZE] = {GEN_REPEAT_U(false, GEN_STRIP(ARRAY_SIZE))};
// 展开为：bool variable[4u] = {false, false, false, false};
```

---

#### REQ-021 — 括号剥离宏

**编号** | **优先级** | **实现宏**
REQ-021 | 低 | `GEN_STRIP(x)`

**描述：**  
系统应提供 `GEN_STRIP(x)` 宏，用于剥离宏参数外层的括号。该宏配合 `GEN_REPEAT_U` 使用，允许以带括号的宏（如 `(4u)`）作为重复次数参数。

**处理流程：**
1. `GEN_STRIP(x)` → `GEN_STRIP_PARENS(GEN_GET_ARGS x)`
2. `GEN_GET_ARGS` 展开 `__VA_ARGS__` 获取括号内容
3. `GEN_STRIP_PARENS` 剥离外层括号

---

#### REQ-022 — 基础数据类型运行时验证

**编号** | **优先级** | **实现宏**
REQ-022 | 中 | `FAS_STATIC_ASSERT()`（在 general.h 中使用）

**描述：**  
系统应在编译时通过静态断言验证基础数据类型定义的正确性，包括：
- `false == 0`
- `true == 1`
- `true != false`
- `STD_OK == 0`
- `STD_NOT_OK == 1`
- `STD_OK != STD_NOT_OK`

**错误处理：**
- 静态断言失败 → 编译错误

---

#### REQ-023 — 平台字长定义

**编号** | **优先级** | **实现宏**
REQ-023 | 中 | `GEN_BYTES_PER_WORD`

**描述：**  
系统应定义平台的字长（每字的字节数）宏 `GEN_BYTES_PER_WORD`。对于 TI ARM 编译器（`__TI_COMPILER_VERSION__` + `__ARM_32BIT_STATE` + `__TMS470__`）、单元测试和文档生成模式，字长均为 4 字节。对于未指定的平台，默认使用 4 字节并产生编译警告。

---

### 3.5 标准类型定义（fstd_types.h）

#### REQ-024 — 标准返回类型定义

**编号** | **优先级** | **实现枚举**
REQ-024 | 中 | `STD_RETURN_TYPE_e`

**描述：**  
系统应定义标准返回类型枚举 `STD_RETURN_TYPE_e`，包含 `STD_OK`（操作成功）和 `STD_NOT_OK`（操作失败）两个值，供所有模块统一使用。

**枚举值：**

| 值 | 含义 |
|----|------|
| `STD_OK` | 操作成功完成 |
| `STD_NOT_OK` | 操作失败 |

---

#### REQ-025 — 数字引脚状态类型定义

**编号** | **优先级** | **实现枚举**
REQ-025 | 低 | `STD_PIN_STATE_e`

**描述：**  
系统应定义数字引脚状态枚举 `STD_PIN_STATE_e`，包含 `STD_PIN_LOW`（低电平）、`STD_PIN_HIGH`（高电平）和 `STD_PIN_UNDEFINED`（未定义）三种状态。

---

#### REQ-026 — 空指针宏定义

**编号** | **优先级** | **实现宏**
REQ-026 | 中 | `NULL`, `NULL_PTR`

**描述：**  
系统应定义 `NULL` 和 `NULL_PTR` 空指针宏。`NULL` 定义为 `((void *)0u)`，`NULL_PTR` 定义为 `((void *)(0u))`。两者均在未被预先定义时才定义（通过 `#ifndef` 守卫）。

---

### 3.6 系统权限控制（fsystem.h）

#### REQ-027 — 特权模式提升

**编号** | **优先级** | **实现函数**
REQ-027 | 高 | `FSYS_RaisePrivilege()`

**描述：**  
系统应提供 `FSYS_RaisePrivilege()` 函数，通过汇编实现将当前处理器执行模式从用户模式提升到特权模式。该函数通过 `#pragma SWI_ALIAS` 映射到汇编实现，操作 SPSR 寄存器的控制域掩码字节 PSR[7:0]。

**返回值：**

| 返回值 | 含义 |
|--------|------|
| 0 | 调用者原先处于用户模式 |
| 非0 | 调用者原先已处于特权模式 |

**前置条件：**
- 使用特权模式后必须调用 `FSYS_SwitchToUserMode()` 切换回用户模式

**后置条件：**
- 处理器处于特权执行模式

---

#### REQ-028 — 用户模式切换

**编号** | **优先级** | **实现函数/宏**
REQ-028 | 高 | `FSYS_SwitchToUserMode()`

**描述：**  
系统应提供 `FSYS_SwitchToUserMode()` 函数/宏，用于从特权模式切换回用户模式。在嵌入式目标平台上，该函数通过内联汇编 `CPS #0x10` 实现；在单元测试模式下，该宏展开为空。不使用此函数可能导致意外的系统行为。

**处理流程：**
1. 嵌入式模式：执行内联汇编指令 `CPS #0x10`
2. 单元测试模式：无操作

---

#### REQ-029 — ISR 中的任务切换

**编号** | **优先级** | **实现宏**
REQ-029 | 中 | `FSYS_PORT_YIELD_FROM_ISR(x)`

**描述：**  
系统应提供 `FSYS_PORT_YIELD_FROM_ISR(x)` 宏，在中断服务例程中触发任务调度切换。当参数 `x` 不等于 `pdFALSE` 时，执行 FreeRTOS 的 `portYIELD_FROM_ISR(x)` 操作。在单元测试模式下，该宏展开为空。

---

### 3.7 可测试无限循环（infinite-loop-helper.h）

#### REQ-030 — 可测试的无限循环辅助宏

**编号** | **优先级** | **实现宏**
REQ-030 | 中 | `FOREVER()`

**描述：**  
系统应提供 `FOREVER()` 宏，在嵌入式目标平台展开为 `true`（用于 `while(FOREVER())` 无限循环），在单元测试模式下映射到外部函数 `FOREVER()`，允许测试框架控制循环退出，从而使包含无限循环的代码可被单元测试覆盖。

**处理流程：**
1. 嵌入式模式：`#define FOREVER() (true)`
2. 单元测试模式：`extern bool FOREVER(void);`（由测试框架实现）

---

### 3.8 构建配置定义（app_build_cfg.h）

#### REQ-031 — 模拟前端（AFE）类型枚举

**编号** | **优先级** | **实现枚举**
REQ-031 | 中 | `VER_AFE_e`

**描述：**  
系统应定义 AFE 类型枚举 `VER_AFE_e`，涵盖 foxBMS 2 支持的全部模拟前端采集芯片型号。编码方案为：高二位十六进制数字表示制造商，低二位数字表示具体型号。

**支持的 AFE 类型：**

| 制造商 | 型号枚举 | 编码 |
|--------|----------|------|
| DEBUG | `AFE_DEBUG_DEFAULT` | 0x0000 |
| DEBUG | `AFE_DEBUG_CAN` | 0x0001 |
| LTC | `AFE_LTC_LTC6804_1` | 0x0100 |
| LTC | `AFE_LTC_LTC6806` | 0x0101 |
| LTC | `AFE_LTC_LTC6811_1` | 0x0102 |
| LTC | `AFE_LTC_LTC6812_1` | 0x0103 |
| LTC | `AFE_LTC_LTC6813_1` | 0x0104 |
| NXP | `AFE_NXP_MC33775A` | 0x0200 |
| MAXIM | `AFE_MAXIM_MAX17852` | 0x0300 |
| ADI | `AFE_ADI_ADES1830` | 0x0400 |
| TI | `AFE_TI_DUMMY` | 0x0501 |

---

#### REQ-032 — 算法配置枚举

**编号** | **优先级** | **实现枚举**
REQ-032 | 中 | `VER_SOC_ALGORITHM_e`, `VER_SOE_ALGORITHM_e`, `VER_SOF_ALGORITHM_e`, `VER_SOH_ALGORITHM_e`, `VER_BALANCING_STRATEGY_e`, `VER_IMD_e`, `VER_RTOS_e`, `VER_TEMPERATURE_SENSOR_e`, `VER_TEMPERATURE_SENSOR_METHOD_e`

**描述：**  
系统应定义构建配置所需的全部算法和组件类型枚举，覆盖 SOC/SOH/SOE/SOF 算法、均衡策略、绝缘监测装置（IMD）、RTOS 类型、温度传感器型号及方法。

**各枚举选项：**

| 枚举 | 选项 |
|------|------|
| SOC 算法 | `NONE`, `DEBUG`, `COUNTING`, `LOOKUP_TABLE`, `INVALID` |
| SOE 算法 | `NONE`, `DEBUG`, `COUNTING`, `INVALID` |
| SOF 算法 | `TRAPEZOID`, `INVALID` |
| SOH 算法 | `NONE`, `DEBUG`, `INVALID` |
| 均衡策略 | `NONE`, `VOLTAGE`, `HISTORY` |
| IMD | `NONE`, `BENDER_ISO165C`, `BENDER_IR155` |
| RTOS | `FREERTOS`, `SAFERTOS` |
| 温度传感器方法 | `POLYNOMIAL`, `LOOKUP_TABLE` |
| 温度传感器 | `FAK00`, `EPC00`, `EPC01`, `EPC02`, `MUR00`, `SEM00`, `VIS00`, `VIS01`, `VIS02`, `TDK00`, `TDK01` |

---

#### REQ-033 — 构建配置结构体

**编号** | **优先级** | **实现结构体**
REQ-033 | 中 | `VER_BUILD_CONFIGURATION_s`

**描述：**  
系统应定义构建配置结构体 `VER_BUILD_CONFIGURATION_s`，聚合所有编译时确定的系统配置信息。该结构体实例 `ver_foxbmsBuildConfiguration` 由构建工具从 `conf/bms/bms.json` 中提取生成，所有字段为 `const` 只读。

**数据结构字段：**

| 字段 | 类型 | 说明 |
|------|------|------|
| `socAlgorithm` | `VER_SOC_ALGORITHM_e` | SOC 状态估计算法 |
| `soeAlgorithm` | `VER_SOE_ALGORITHM_e` | SOE 状态估计算法 |
| `sofAlgorithm` | `VER_SOF_ALGORITHM_e` | SOF 状态估计算法 |
| `sohAlgorithm` | `VER_SOH_ALGORITHM_e` | SOH 状态估计算法 |
| `imd` | `VER_IMD_e` | 绝缘监测装置类型 |
| `balancingStrategy` | `VER_BALANCING_STRATEGY_e` | 均衡策略 |
| `rtos` | `VER_RTOS_e` | RTOS 类型 |
| `afe` | `VER_AFE_e` | AFE 采集芯片型号 |
| `temperatureSensor` | `VER_TEMPERATURE_SENSOR_e` | 温度传感器型号 |
| `temperatureSensorMethod` | `VER_TEMPERATURE_SENSOR_METHOD_e` | 温度传感器计算方法 |

---

## 4. 非功能需求

### 4.1 时序与性能

#### NFR-001 — PLL 锁定重试时限

**编号** | **优先级** | **关联宏**
NFR-001 | 高 | `STU_PLL_RETRIES (5u)`

**描述：**  
系统在 `systemInit()` 中等待 PLL 锁定的重试次数最多为 5 次。超过此次数后 PLL 仍未锁定，系统应进入故障处理状态。

---

#### NFR-002 — 复位后首次中断使能的时序

**编号** | **优先级** | **关联函数**
NFR-002 | 高 | `main()`

**描述：**  
系统上电复位到 IRQ 中断使能的总耗时取决于各外设初始化耗时之和。中断必须在 AFE 任务创建后才使能，确保 DMA 中断回调函数可安全访问有效的 AFE 任务句柄。

---

### 4.2 内存

#### NFR-003 — 断言位置记录内存开销

**编号** | **优先级** | **关联结构体**
NFR-003 | 低 | `FAS_ASSERT_LOCATION_s`

**描述：**  
`FAS_ASSERT_LOCATION_s` 结构体大小为 8 字节（32位平台），存储为静态局部变量（位于栈中），不占用持久内存。

---

### 4.3 鲁棒性

#### NFR-004 — 无效复位源处理

**编号** | **优先级** | **关联函数**
NFR-004 | 中 | `_c_int00()`

**描述：**  
当检测到未知复位源时，`_c_int00()` 应通过 `default` 分支安全退出 switch 语句，执行 MPU 初始化和全局变量初始化后正常进入应用。

---

#### NFR-005 — 断言保护完整性

**编号** | **优先级** | **关联宏**
NFR-005 | 高 | `FAS_ASSERT(x)`

**描述：**  
断言宏在嵌入式模式下通过 `do { ... } while(false)` 包装，确保在条件语句中使用时不会因宏展开导致控制流错误。断言失败时在记录位置信息后才进入无限循环，保证调试信息不丢失。

---

### 4.4 可配置性

#### NFR-006 — 编译时构建配置

**编号** | **优先级** | **关联文件**
NFR-006 | 中 | `app_build_cfg.h`

**描述：**  
所有系统配置（AFE 类型、算法选择、传感器类型等）应在编译时通过 `conf/bms/bms.json` 确定，以 `const` 结构体的形式嵌入固件，运行时不可修改。

---

### 4.5 可测试性

#### NFR-007 — 单元测试隔离

**编号** | **优先级** | **关联宏**
NFR-007 | 高 | `UNITY_UNIT_TEST`

**描述：**  
MAIN 模块所有文件应支持通过 `UNITY_UNIT_TEST` 宏进行单元测试隔离。在单元测试模式下：
- `main()` 重命名为 `unit_test_main()`
- `FAS_ASSERT()` 映射为 CException 异常抛出
- 静态函数可外部化以支持直接测试
- `FOREVER()` 映射为外部可控制的模拟函数
- `FSYS_SwitchToUserMode()` 和 `FSYS_PORT_YIELD_FROM_ISR()` 展开为空

---

## 5. 接口需求

### 5.1 公共 API

| 函数/宏 | 文件 | 访问性 | 说明 |
|---------|------|--------|------|
| `main()` / `unit_test_main()` | main.c | 全局 | 应用入口点 |
| `_c_int00()` | fstartup.c | 全局 | 复位向量入口 |
| `STU_GetResetSourceWithoutFlagReset()` | fstartup.c | 静态（UT时外部化） | 读取复位源 |
| `FAS_ASSERT(x)` | fassert.h | 宏 | 运行时断言 |
| `FAS_STATIC_ASSERT(cond, msg)` | fassert.h | 宏 | 静态断言 |
| `FAS_TRAP` | fassert.h | 宏 | 无条件断言失败 |
| `FAS_StoreAssertLocation()` | fassert.c | 全局 | 记录断言位置 |
| `FAS_DisableInterrupts()` | fassert.h | 全局 | 禁用中断（ASM实现） |
| `FSYS_RaisePrivilege()` | fsystem.h | 全局 | 提升特权模式（ASM实现） |
| `FSYS_SwitchToUserMode()` | fsystem.h | 静态内联/宏 | 切换用户模式 |
| `FSYS_PORT_YIELD_FROM_ISR(x)` | fsystem.h | 宏 | ISR中任务切换 |
| `GEN_SET_BIT()` / `GEN_CLEAR_BIT()` | general.h | 宏 | 位操作 |
| `GEN_REPEAT_U()` | general.h | 宏 | 令牌重复 |
| `GEN_STRIP()` | general.h | 宏 | 括号剥离 |
| `FOREVER()` | infinite-loop-helper.h | 宏/函数 | 无限循环辅助 |
| `__TI_auto_init()` | fstartup.h | 全局 | TI库：全局变量初始化 |
| `VER_BUILD_CONFIGURATION_s` / `ver_foxbmsBuildConfiguration` | app_build_cfg.h | 全局只读 | 构建配置 |

### 5.2 依赖项

| 依赖模块 | 类型 | 说明 |
|----------|------|------|
| `HL_sys_common.h` | HAL | 系统通用功能 |
| `HL_system.h` | HAL | 系统时钟与Flash配置 |
| `HL_sys_vim.h` | HAL | 向量中断管理器 |
| `HL_sys_core.h` | HAL | CPU内核初始化 |
| `HL_esm.h` | HAL | 错误信号模块 |
| `HL_adc.h` | HAL | ADC驱动 |
| `HL_crc.h` | HAL | CRC驱动 |
| `HL_etpwm.h` | HAL | eTPWM驱动 |
| `HL_gio.h` | HAL | GPIO驱动 |
| `HL_het.h` | HAL | HET驱动 |
| `HL_pinmux.h` | HAL | 引脚复用 |
| `adc.h` | DRIVER | ADC驱动层 |
| `diag.h` | DRIVER | 诊断模块 |
| `dma.h` | DRIVER | DMA驱动 |
| `i2c.h` | DRIVER | I2C驱动 |
| `led.h` | DRIVER | 调试LED |
| `pwm.h` | DRIVER | PWM驱动 |
| `spi.h` | DRIVER | SPI驱动 |
| `os.h` | OS | RTOS接口 |
| `foxmath.h` | 库 | 数学自检 |
| `master_info.h` | APP | 主信息模块 |
| `fassert.h` | MAIN | 断言系统 |
| `fstd_types.h` | MAIN | 标准类型 |
| `CException.h` | 测试 | CException 测试框架 |

### 5.3 调用方

| 调用方 | 调用函数/宏 | 说明 |
|--------|-----------|------|
| 系统复位 | `_c_int00()` | 硬件复位向量直接跳转 |
| `_c_int00()` | `main()` | 启动完成后调用应用入口 |
| 所有模块 | `FAS_ASSERT()` / `FAS_TRAP` / `FAS_STATIC_ASSERT()` | 断言检查 |
| 所有模块 | `STD_RETURN_TYPE_e` / `STD_OK` / `STD_NOT_OK` | 返回值约定 |
| 所有模块 | `GEN_SET_BIT()` / `GEN_CLEAR_BIT()` | 寄存器操作 |
| 驱动层 | `NULL` / `NULL_PTR` | 空指针检查 |
| OS 层 | `FSYS_RaisePrivilege()` / `FSYS_SwitchToUserMode()` | 权限切换 |
| OS 层 | `FOREVER()` | 无限循环任务 |

---

## 6. 数据结构

### 6.1 FAS_ASSERT_LOCATION_s

```c
typedef struct {
    uint32_t *pc;   /*!< 程序计数器值 */
    uint32_t line;  /*!< 断言触发的行号 */
} FAS_ASSERT_LOCATION_s;
```

### 6.2 VER_BUILD_CONFIGURATION_s

```c
typedef struct {
    const VER_SOC_ALGORITHM_e socAlgorithm;
    const VER_SOE_ALGORITHM_e soeAlgorithm;
    const VER_SOF_ALGORITHM_e sofAlgorithm;
    const VER_SOH_ALGORITHM_e sohAlgorithm;
    const VER_IMD_e imd;
    const VER_BALANCING_STRATEGY_e balancingStrategy;
    const VER_RTOS_e rtos;
    const VER_AFE_e afe;
    const VER_TEMPERATURE_SENSOR_e temperatureSensor;
    const VER_TEMPERATURE_SENSOR_METHOD_e temperatureSensorMethod;
} VER_BUILD_CONFIGURATION_s;
```

### 6.3 STD_RETURN_TYPE_e

```c
typedef enum {
    STD_OK,     /* 操作成功 */
    STD_NOT_OK, /* 操作失败 */
} STD_RETURN_TYPE_e;
```

### 6.4 STD_PIN_STATE_e

```c
typedef enum {
    STD_PIN_LOW,       /* 引脚低电平 */
    STD_PIN_HIGH,      /* 引脚高电平 */
    STD_PIN_UNDEFINED, /* 引脚状态未定义 */
} STD_PIN_STATE_e;
```

---

## 7. 控制流

### 7.1 系统启动总体流程

```
硬件复位
  │
  ▼
_c_int00() ── 内核寄存器初始化 ── 堆栈指针初始化 ── 复位源检测
  │                                                      │
  │                    ┌─────────────────────────────────┤
  │                    ▼                                 ▼
  │           POWERON_RESET/EXT_RESET/DEBUG_RESET    OSC_FAILURE/WATCHDOG/
  │                    │                            CPU0/SW_RESET/其他
  │           ┌───────┴───────┐                     ── 跳过初始化
  │           ▼               ▼
  │     POWERON_RESET    EXT/DEBUG_RESET
  │     _memInit_()      _memInit_()
  │     时钟配置          时钟配置
  │     VIM初始化         VIM初始化
  │     ESM初始化         ESM初始化
  │           │               │
  │           └───────┬───────┘
  │                   ▼
  └───────────────── _mpuInit_() ── __TI_auto_init() ── main()
                                                             │
                                                             ▼
                                                     硬件外设初始化
                                                     按序初始化所有外设
                                                             │
                                                             ▼
                                                     OS时间自检 ── OS初始化
                                                             │
                                                             ▼
                                                     使能IRQ中断 ── 检查OS启动状态
                                                             │
                                                             ▼
                                                     OS_StartScheduler()
                                                     （不应返回）
```

### 7.2 断言控制流

```
FAS_ASSERT(condition)
  │
  ├── condition 为真 ── 无操作，继续执行
  │
  └── condition 为假
        │
        ├── 单元测试模式 ── Throw(0) ── CException异常
        │
        └── 嵌入式模式
              │
              ├── FAS_ASSERT_RECORD() ── 记录PC和行号
              │
              └── FAS_InfiniteLoop()
                    │
                    ├── 级别0: FAS_DisableInterrupts() → while(true)
                    ├── 级别1: while(true)
                    └── 级别2: 无操作（返回）
```

---

## 8. 需求追溯矩阵

| 需求编号 | 需求标题 | 实现位置 | 类型 |
|----------|---------|---------|------|
| REQ-001 | 系统硬件初始化序列 | `main.c:98-143` `main()` | 功能 |
| REQ-002 | 操作系统初始化与启动 | `main.c:123-143` `main()` | 功能 |
| REQ-003 | 复位源信息记录 | `main.c:103` `main()` | 功能 |
| REQ-004 | 中断使能时序控制 | `main.c:131` `main()` | 功能 |
| REQ-005 | 条件编译外设支持 | `main.c:70-83,114-122` | 功能 |
| REQ-006 | 单元测试构建兼容 | `main.c:97-101`, `main.h:74-78` | 功能 |
| REQ-007 | FAS_ASSERT 运行时断言 | `fassert.h:248-261` `FAS_ASSERT(x)` | 功能 |
| REQ-008 | 断言级别配置 | `fassert.h:167-201` | 功能 |
| REQ-009 | 断言位置记录 | `fassert.h:221-230`, `fassert.c:71-82` | 功能 |
| REQ-010 | FAS_TRAP 无条件断言 | `fassert.h:128` `FAS_TRAP` | 功能 |
| REQ-011 | FAS_STATIC_ASSERT 静态断言 | `fassert.h:263-285` | 功能 |
| REQ-012 | 中断禁用机制 | `fassert.h:116-117` `FAS_DisableInterrupts()` | 功能 |
| REQ-013 | CPU 复位入口 | `fstartup.c:158-258` `_c_int00()` | 功能 |
| REQ-014 | 复位源检测 | `fstartup.c:112-148` | 功能 |
| REQ-015 | 上电复位初始化 | `fstartup.c:174-214` `_c_int00()` | 功能 |
| REQ-016 | ESM 错误检测与通知 | `fstartup.c:192-198,209-212` | 功能 |
| REQ-017 | CPU事件总线导出使能 | `fstartup.c:190,227` | 功能 |
| REQ-018 | MPU和VIM初始化 | `fstartup.c:207,238` | 功能 |
| REQ-019 | 寄存器位操作宏 | `general.h:72-78` | 功能 |
| REQ-020 | 令牌重复宏 | `general.h:112-243` | 功能 |
| REQ-021 | 括号剥离宏 | `general.h:250-254` | 功能 |
| REQ-022 | 基础数据类型验证 | `general.h:95-101` | 功能 |
| REQ-023 | 平台字长定义 | `general.h:258-269` | 功能 |
| REQ-024 | 标准返回类型定义 | `fstd_types.h:82-85` | 功能 |
| REQ-025 | 引脚状态类型定义 | `fstd_types.h:88-92` | 功能 |
| REQ-026 | 空指针宏定义 | `fstd_types.h:66-78` | 功能 |
| REQ-027 | 特权模式提升 | `fsystem.h:115-116` `FSYS_RaisePrivilege()` | 功能 |
| REQ-028 | 用户模式切换 | `fsystem.h:130-136` `FSYS_SwitchToUserMode()` | 功能 |
| REQ-029 | ISR 任务切换 | `fsystem.h:143-146` | 功能 |
| REQ-030 | 可测试无限循环 | `infinite-loop-helper.h:62-66` | 功能 |
| REQ-031 | AFE 类型枚举 | `app_build_cfg.h:71-93` | 功能 |
| REQ-032 | 算法配置枚举 | `app_build_cfg.h:99-193` | 功能 |
| REQ-033 | 构建配置结构体 | `app_build_cfg.h:195-206` | 功能 |
| NFR-001 | PLL 锁定重试时限 | `fstartup.c:90` `STU_PLL_RETRIES` | 非功能 |
| NFR-002 | 中断使能时序约束 | `main.c:131` `main()` | 非功能 |
| NFR-003 | 断言位置记录内存开销 | `fassert.c:77-81` `FAS_ASSERT_LOCATION_s` | 非功能 |
| NFR-004 | 无效复位源处理 | `fstartup.c:234-235` `_c_int00()` | 非功能 |
| NFR-005 | 断言保护完整性 | `fassert.h:254-261` `FAS_ASSERT(x)` | 非功能 |
| NFR-006 | 编译时构建配置 | `app_build_cfg.h` | 非功能 |
| NFR-007 | 单元测试隔离 | 所有文件 `UNITY_UNIT_TEST` | 非功能 |

---

> **文档生成信息：** 本文档由 Claude 自动生成，基于 foxBMS 2 源代码分析。  
> **生成日期：** 2026-06-06  
> **覆盖文件：** `main.c`, `main.h`, `fassert.c`, `fassert.h`, `fstartup.c`, `fstartup.h`, `general.h`, `fstd_types.h`, `fsystem.h`, `infinite-loop-helper.h`, `app_build_cfg.h`
