# DATA — 数据库引擎模块软件需求规格说明

## 1. 引言

### 1.1 目的

本文档定义 foxBMS 2 数据库引擎模块（DATA）的软件需求。DATA 模块是 BMS 数据管理的核心，负责所有模块数据的统一存储、读写访问和时间戳管理。

### 1.2 范围

- **涵盖**：数据库初始化、数据读写访问（队列机制）、时间戳管理、数据块类型定义、数据有效性辅助判断、内置自检（BIST）、索引映射工具函数
- **不涵盖**：各业务模块如何使用数据库的具体逻辑

### 1.3 定义与缩略语

| 缩略语 | 说明 |
|--------|------|
| DATA | 数据库引擎模块（Database Engine） |
| BIST | 内置自检（Built-In Self-Test） |
| uniqueId | 数据库条目标识符，全局唯一 |

### 1.4 参考文献

- [database.h](database.h) — 数据库模块头文件
- [database.c](database.c) — 数据库模块实现
- [database_helper.h](database_helper.h) — 数据库辅助函数头文件
- [database_helper.c](database_helper.c) — 数据库辅助函数实现
- [database_cfg.h](../config/database_cfg.h) — 数据库配置头文件

---

## 2. 总体描述

### 2.1 产品视角

DATA 模块是 BMS 的中央数据管理模块，位于 ENGINE 层。所有模块通过 FreeRTOS 队列向 database task 提交读写请求，database task 是唯一直接操作数据结构的实体。该设计保证了数据访问的线程安全性。

### 2.2 工作模式

DATA 模块采用生产者-消费者模式：
- **生产者（调用方）**：调用 `DATA_READ_DATA()` / `DATA_WRITE_DATA()` 宏，将读写请求入队
- **消费者（DATA_Task）**：从队列取出请求，执行实际的数据拷贝操作

每次访问最多支持 4 个数据块（`DATA_MAX_ENTRIES_PER_ACCESS = 4`）。

### 2.3 数据流

```text
[模块A] → DATA_WRITE_DATA(&block) → Queue → DATA_Task() → memcpy → data_database[]
[模块B] → DATA_READ_DATA(&block)  → Queue → DATA_Task() → memcpy ← data_database[]
```

---

## 3. 功能需求

### 3.1 数据库初始化

#### REQ-APP_ENGINE_DATABASE_001 — 数据库初始化

**编号** | REQ-APP_ENGINE_DATABASE_001
**优先级** | 高
**函数** | `DATA_Initialize()`

**描述**：系统应在启动阶段初始化数据库模块，包括：
1. 验证数据库队列已创建（`ftsk_allQueuesCreated == true`）
2. 静态断言验证数据库数组已定义且长度匹配 `DATA_BLOCK_ID_MAX`
3. 遍历所有数据库条目，将除 `uniqueId` 外的内容清零
4. 建立 `uniqueId` 到数据库数组索引的映射表 `data_uniqueIdToDatabaseEntry[]`
5. 验证队列指针非空

**前置条件**：数据库队列已通过 FreeRTOS 创建

**后置条件**：所有数据库条目（除 uniqueId 外）清零，ID 映射表建立完成

**错误处理**：
- 数据库数组未定义：编译时静态断言失败
- 队列未创建：返回 `STD_NOT_OK`
- uniqueId 或索引越界：返回 `STD_NOT_OK`

---

### 3.2 数据写访问

#### REQ-APP_ENGINE_DATABASE_002 — 单数据块写入

**编号** | REQ-APP_ENGINE_DATABASE_002
**优先级** | 高
**函数** | `DATA_Write1DataBlock()`, `DATA_Write2DataBlocks()`, `DATA_Write3DataBlocks()`, `DATA_Write4DataBlocks()`, `DATA_WRITE_DATA()`

**描述**：系统应提供写入 1-4 个数据块到数据库的接口。每个写入函数构造 `DATA_QUEUE_MESSAGE_s` 消息并通过 FreeRTOS 队列发送。变参宏 `DATA_WRITE_DATA()` 根据参数数量自动选择对应函数。写入操作在 database task 中异步执行，包含以下处理：
1. 更新数据块头部的 `previousTimestamp` 为当前的 `timestamp`
2. 更新 `timestamp` 为当前系统 tick 计数
3. 将传入的数据通过 `memcpy` 拷贝到数据库对应条目

**前置条件**：数据库已初始化，传入的第一个数据块指针非空

**后置条件**：数据库对应条目被更新，时间戳被刷新

**错误处理**：
- 队列满且超时：返回 `STD_NOT_OK`
- 首指针为空：由 `DATA_AccessDatabaseEntries` 断言检测

---

#### REQ-APP_ENGINE_DATABASE_003 — 多数据块写入（2-4块）

**编号** | REQ-APP_ENGINE_DATABASE_003
**优先级** | 高
**函数** | `DATA_Write2DataBlocks()`, `DATA_Write3DataBlocks()`, `DATA_Write4DataBlocks()`

**描述**：系统应支持批量写入 2-4 个数据块，减少多次入队的开销。所有指针均需非空（`FAS_ASSERT` 检查）。内部均委托 `DATA_AccessDatabaseEntries()` 统一处理，未使用的槽位填充 `NULL_PTR`。

---

### 3.3 数据读访问

#### REQ-APP_ENGINE_DATABASE_004 — 单数据块读取

**编号** | REQ-APP_ENGINE_DATABASE_004
**优先级** | 高
**函数** | `DATA_Read1DataBlock()`, `DATA_Read2DataBlocks()`, `DATA_Read3DataBlocks()`, `DATA_Read4DataBlocks()`, `DATA_READ_DATA()`

**描述**：系统应提供读取 1-4 个数据块的接口。变参宏 `DATA_READ_DATA()` 根据参数数量自动选择对应函数。读取操作在 database task 中异步执行，通过 `memcpy` 将数据库条目内容拷贝到调用方传入的结构体中。

**前置条件**：数据库已初始化，传入的数据块指针非空

**后置条件**：调用方结构体被填充为数据库当前值

---

### 3.4 数据库任务

#### REQ-APP_ENGINE_DATABASE_005 — 数据库任务处理

**编号** | REQ-APP_ENGINE_DATABASE_005
**优先级** | 高
**函数** | `DATA_Task()`

**描述**：系统应在 database task 中循环从队列接收消息并处理数据库读写请求：
1. 从队列 `ftsk_databaseQueue` 以 1ms 超时接收消息
2. 断言验证第一条目指针非空（作为消息有效性的基本检查）
3. 调用 `DATA_IterateOverDatabaseEntries()` 遍历处理队列中的所有非空条目
4. 对每个非空条目：解析 uniqueId → 查找数据库索引 → 获取数据长度 → 执行 `DATA_CopyData()`

**处理流程**：
1. 检查队列指针非空
2. 初始化接收消息（所有条目设为 NULL_PTR）
3. 以 1ms 阻塞超时从队列接收
4. 断言首条目非空
5. 遍历并处理所有条目

---

### 3.5 数据拷贝

#### REQ-APP_ENGINE_DATABASE_006 — 数据库读写拷贝

**编号** | REQ-APP_ENGINE_DATABASE_006
**优先级** | 高
**函数** | `DATA_CopyData()`

**描述**：系统应实现数据库与用户结构体之间的双向数据拷贝：
- **写访问**（`DATA_WRITE_ACCESS`）：先更新传入结构体的时间戳（previousTimestamp ← timestamp, timestamp ← 当前 tick），再将完整结构体拷贝到数据库
- **读访问**（`DATA_READ_ACCESS`）：将数据库内容直接拷贝到用户结构体
- 无效访问类型触发 `FAS_ASSERT(FAS_TRAP)`

---

### 3.6 内置自检

#### REQ-APP_ENGINE_DATABASE_007 — 数据库自检

**编号** | REQ-APP_ENGINE_DATABASE_007
**优先级** | 高
**函数** | `DATA_ExecuteDataBist()`

**描述**：系统应提供数据库模块的内置自检功能：
1. 构造测试数据（member1 = UINT8_MAX, member2 = 0xAA 交替位模式）
2. 写入数据库自检条目
3. 读回并逐字段断言验证写入值与读出值一致
4. 任一步骤失败通过断言捕获

---

### 3.7 数据库辅助函数

#### REQ-APP_ENGINE_DATABASE_008 — 数据条目更新判断

**编号** | REQ-APP_ENGINE_DATABASE_008
**优先级** | 中
**函数** | `DATA_DatabaseEntryUpdatedAtLeastOnce()`

**描述**：系统应提供判断数据库条目是否至少被更新过一次的函数。通过检查 `timestamp` 和 `previousTimestamp` 是否同时为 0 来判断——二者同时为 0 表示从未被更新。

---

#### REQ-APP_ENGINE_DATABASE_009 — 指定时间间隔内更新判断

**编号** | REQ-APP_ENGINE_DATABASE_009
**优先级** | 中
**函数** | `DATA_EntryUpdatedWithinInterval()`

**描述**：系统应提供判断数据库条目最后更新时间是否在指定时间间隔内的函数。使用无符号整数算术处理定时器溢出（wrap-around）。

---

#### REQ-APP_ENGINE_DATABASE_010 — 周期性更新判断

**编号** | REQ-APP_ENGINE_DATABASE_010
**优先级** | 中
**函数** | `DATA_EntryUpdatedPeriodicallyWithinInterval()`

**描述**：系统应提供判断数据库条目是否在指定时间间隔内周期性更新的函数。需同时满足：
1. 最后更新时间距今 ≤ 时间间隔
2. 两次更新之间的时间差 ≤ 时间间隔
3. 条目至少被更新过一次

---

### 3.8 索引映射工具

#### REQ-APP_ENGINE_DATABASE_011 — 电压/温度索引映射

**编号** | REQ-APP_ENGINE_DATABASE_011
**优先级** | 中
**函数** | `DATA_GetStringNumberFromVoltageIndex()`, `DATA_GetModuleNumberFromVoltageIndex()`, `DATA_GetCellNumberFromVoltageIndex()`, `DATA_GetStringNumberFromTemperatureIndex()`, `DATA_GetModuleNumberFromTemperatureIndex()`, `DATA_GetSensorNumberFromTemperatureIndex()`

**描述**：系统应提供从全局电压/温度传感器索引到物理位置（串号、模组号、单体号/传感器号）的映射函数：
- 串号 = 索引 / 每串单体数（或每串传感器数）
- 模组号 = (索引 / 每模组单体数) - (串号 × 每串模组数)
- 单体号 = 索引 % 每模组单体数
- 传感器号 = 索引 % 每模组传感器数

所有映射函数应在入口处对索引范围进行断言检查。

---

## 4. 非功能需求

### 4.1 时序与性能

| 需求编号 | 描述 |
|----------|------|
| REQ-APP_ENGINE_DATABASE_NFR_001 | 数据读写请求应通过队列异步处理，避免在调用方阻塞 |
| REQ-APP_ENGINE_DATABASE_NFR_002 | 队列发送超时时间 = 10ms / OS_TICK_RATE_MS（约 10 个 tick） |
| REQ-APP_ENGINE_DATABASE_NFR_003 | `DATA_Task()` 的队列接收超时为 1ms（阻塞等待） |

### 4.2 内存

| 需求编号 | 描述 |
|----------|------|
| REQ-APP_ENGINE_DATABASE_NFR_004 | 数据库条目数量 `DATA_BLOCK_ID_MAX` 应小于 `UINT8_MAX`（255），由编译期静态断言保证 |
| REQ-APP_ENGINE_DATABASE_NFR_005 | 数据库条目在初始化时（除 uniqueId 外）应清零，确保无未定义初始值 |
| REQ-APP_ENGINE_DATABASE_NFR_006 | 每个数据块头部必须包含 `DATA_BLOCK_HEADER_s`（uniqueId + timestamp + previousTimestamp） |

### 4.3 鲁棒性

| 需求编号 | 描述 |
|----------|------|
| REQ-APP_ENGINE_DATABASE_NFR_007 | 所有公开读写函数入口处应对第一个数据指针进行非空断言检查 |
| REQ-APP_ENGINE_DATABASE_NFR_008 | `DATA_Task()` 应在处理消息前断言首个条目指针非空 |
| REQ-APP_ENGINE_DATABASE_NFR_009 | uniqueId 应在 `DATA_BLOCK_ID_MAX` 范围内，由运行时断言 `FAS_ASSERT` 保证 |
| REQ-APP_ENGINE_DATABASE_NFR_010 | 数据库数组长度编译期静态断言确保与 `DATA_BLOCK_ID_MAX` 一致 |

### 4.4 C99 标准兼容

| 需求编号 | 描述 |
|----------|------|
| REQ-APP_ENGINE_DATABASE_NFR_011 | 变参宏 `DATA_READ_DATA()` / `DATA_WRITE_DATA()` 应通过 `GET_MACRO` + `DATA_DummyFunction()` 实现 ISO C99 标准兼容 |


## 5. 接口需求

### 5.1 公共 API

| 函数/宏 | 说明 |
|----------|------|
| `DATA_Initialize()` | 初始化数据库 |
| `DATA_Task()` | 数据库任务处理函数 |
| `DATA_READ_DATA(...)` | 变参宏：读取 1-4 个数据块 |
| `DATA_WRITE_DATA(...)` | 变参宏：写入 1-4 个数据块 |
| `DATA_Read1DataBlock()` ~ `DATA_Read4DataBlocks()` | 读取 1-4 个数据块 |
| `DATA_Write1DataBlock()` ~ `DATA_Write4DataBlocks()` | 写入 1-4 个数据块 |
| `DATA_ExecuteDataBist()` | 数据库内置自检 |
| `DATA_DatabaseEntryUpdatedAtLeastOnce()` | 判断条目是否更新过 |
| `DATA_EntryUpdatedWithinInterval()` | 判断条目在时间间隔内是否更新 |
| `DATA_EntryUpdatedPeriodicallyWithinInterval()` | 判断条目在时间间隔内是否周期性更新 |
| `DATA_GetStringNumberFromVoltageIndex()` | 电压索引→串号 |
| `DATA_GetModuleNumberFromVoltageIndex()` | 电压索引→模组号 |
| `DATA_GetCellNumberFromVoltageIndex()` | 电压索引→单体号 |
| `DATA_GetStringNumberFromTemperatureIndex()` | 温度索引→串号 |
| `DATA_GetModuleNumberFromTemperatureIndex()` | 温度索引→模组号 |
| `DATA_GetSensorNumberFromTemperatureIndex()` | 温度索引→传感器号 |

### 5.2 依赖项

| 被依赖模块 | 调用目的 |
|------------|----------|
| `OS` (FreeRTOS) | 队列操作（SendToBackOfQueue, ReceiveFromQueue） |
| `ftask` | 队列句柄 `ftsk_databaseQueue` |

### 5.3 调用方

所有需要读写数据库的模块（BMS、SYS、BAL、SE、DIAG 等）均通过 `DATA_READ_DATA()` / `DATA_WRITE_DATA()` 访问数据库。

---

## 6. 数据结构

### 6.1 核心结构体

| 结构体 | 说明 |
|--------|------|
| `DATA_BASE_s` | 数据库配置条目（指针 + 数据长度） |
| `DATA_BLOCK_HEADER_s` | 数据块头部（uniqueId + timestamp + previousTimestamp） |
| `DATA_QUEUE_MESSAGE_s` | 队列消息（访问类型 + 最多4个数据块指针） |
| `DATA_BASE_HEADER_s` | 数据库设备配置（条目数 + 数据库指针） |

### 6.2 数据库块类型

数据库支持 40+ 种数据块类型（`DATA_BLOCK_ID_e`），包括：
- 单体电压 (`DATA_BLOCK_CELL_VOLTAGE_s`)
- 单体温度 (`DATA_BLOCK_CELL_TEMPERATURE_s`)
- 极值 (`DATA_BLOCK_MIN_MAX_s`)
- 电池包测量值 (`DATA_BLOCK_PACK_VALUES_s`)
- 电流测量 (`DATA_BLOCK_CURRENT_s`)
- SOC/SOH/SOE/SOF
- 错误状态 (`DATA_BLOCK_ERROR_STATE_s`)
- MSL/RSL/MOL 安全限值标志
- 绝缘监测 (`DATA_BLOCK_INSULATION_s`)
- 均衡控制/反馈 (`DATA_BLOCK_BALANCING_CONTROL_s`)
- 等等

---

## 7. 追溯矩阵

| 需求编号 | 源文件 | 函数/宏 |
|----------|--------|---------|
| REQ-APP_ENGINE_DATABASE_001 | database.c | `DATA_Initialize()` |
| REQ-APP_ENGINE_DATABASE_002 | database.c, database.h | `DATA_Write1DataBlock()`, `DATA_WRITE_DATA()` |
| REQ-APP_ENGINE_DATABASE_003 | database.c, database.h | `DATA_Write2DataBlocks()` ~ `DATA_Write4DataBlocks()` |
| REQ-APP_ENGINE_DATABASE_004 | database.c, database.h | `DATA_Read1DataBlock()` ~ `DATA_Read4DataBlocks()`, `DATA_READ_DATA()` |
| REQ-APP_ENGINE_DATABASE_005 | database.c | `DATA_Task()`, `DATA_IterateOverDatabaseEntries()` |
| REQ-APP_ENGINE_DATABASE_006 | database.c | `DATA_CopyData()` |
| REQ-APP_ENGINE_DATABASE_007 | database.c | `DATA_ExecuteDataBist()` |
| REQ-APP_ENGINE_DATABASE_008 | database_helper.c | `DATA_DatabaseEntryUpdatedAtLeastOnce()` |
| REQ-APP_ENGINE_DATABASE_009 | database_helper.c | `DATA_EntryUpdatedWithinInterval()` |
| REQ-APP_ENGINE_DATABASE_010 | database_helper.c | `DATA_EntryUpdatedPeriodicallyWithinInterval()` |
| REQ-APP_ENGINE_DATABASE_011 | database_helper.c | `DATA_Get*Index()` 映射函数 |
