# AFE — 模拟前端驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 AFE（Analog Front End，模拟前端）驱动模块的软件需求。AFE 模块是 BMS 的核心测量子系统，通过 SPI 总线管理电池监控芯片（多供应商支持），实现电池电压、温度和 GPIO 的采集，以及被动均衡控制。

### 1.2 范围
- **涵盖**：AFE 状态机控制、电池电压/温度采集、均衡控制、GPIO 操作、断线检测、EEPROM 访问、PEC/CRC 校验
- **不涵盖**：SPI 底层通信、电池管理算法

## 2. 模块结构（117 个文件，5 个供应商）

```
afe/
├── api/
│   ├── afe.h                     # AFE 统一接口
│   ├── afe_dma.h                 # DMA 传输接口
│   └── afe_plausibility.c/.h     # 数据合理性校验
├── adi/                          # ADI (Analog Devices) 驱动
│   ├── ades1830/                 # ADES1830 系列
│   └── common/ades183x/          # ADES183x 通用代码
│       ├── adi_ades183x.c/h      #   核心驱动
│       ├── adi_ades183x_voltages.c/h    # 电压采集
│       ├── adi_ades183x_temperatures.c/h # 温度采集
│       ├── adi_ades183x_balancing.c/h    # 均衡控制
│       ├── adi_ades183x_commands.c/h     # SPI 命令
│       ├── adi_ades183x_buffers.c/h      # 数据缓冲
│       ├── adi_ades183x_helpers.c/h      # 工具函数
│       ├── adi_ades183x_pec.c/h          # PEC 校验
│       ├── api/adi_ades183x_afe.c        # AFE API 适配
│       └── config/                       # 配置表
├── ltc/                          # ADI/LTC (Linear Technology) 驱动
│   ├── api/ltc_afe.c             #   AFE API 适配
│   ├── common/                   #   通用代码
│   │   ├── ltc_afe_dma.c/h       #   DMA 通信
│   │   └── ltc_pec.c/h           #   PEC 校验
│   ├── 6806/                     #   LTC6806 系列
│   └── 6813-1/                   #   LTC6813-1 系列
├── maxim/                        # Maxim (美信) 驱动
│   ├── api/mxm_afe.c             #   AFE API 适配
│   ├── common/                   #   通用代码
│   │   ├── mxm_17841b.c/h        #   MAX17841B 桥接芯片
│   │   ├── mxm_1785x.c/h         #   MAX1785x 系列
│   │   ├── mxm_battery_management.c/h  # 电池管理
│   │   ├── mxm_crc8.c/h          #   CRC8 校验
│   │   └── ...
│   └── max17852/mxm_17852.c      #   MAX17852
├── nxp/                          # NXP 驱动
│   ├── api/nxp_afe.c/h           #   AFE API 适配
│   ├── common/mc3377x/           #   MC3377x 通用代码
│   │   ├── nxp_mc3377x.c/h       #   核心驱动
│   │   ├── nxp_mc3377x_measurement.c/h # 测量
│   │   ├── nxp_mc3377x_balancing.c/h   # 均衡
│   │   ├── nxp_mc3377x_database.c/h    # 数据库
│   │   └── ...
│   └── mc33775a/                 #   MC33775A 系列
│       ├── nxp_mc33775a_measurement.c  # 测量
│       ├── nxp_mc33775a_balancing.c    # 均衡
│       └── ...
├── ti/                           # TI (德州仪器) 驱动
│   ├── api/ti_afe.c/h            #   AFE API 适配
│   ├── common/api/               #   BQ79xxx 通用 DMA
│   └── dummy/                    #   模拟占位实现
└── debug/                        # 调试用 AFE
    ├── can/                      #   CAN 调试 AFE
    └── default/                  #   默认调试 AFE
```

## 3. 核心功能需求

### 3.1 AFE 统一接口 (afe.h)

#### REQ-001 — AFE 初始化
**优先级**：高 | **函数**：`AFE_Initialize()`

模块应初始化所选 AFE 驱动、配置 SPI 通信参数、检测菊花链上的芯片。

#### REQ-002 — AFE 状态机触发
**优先级**：高 | **函数**：`AFE_TriggerIc()`

模块应周期性驱动 AFE 状态机，执行测量、读取、均衡等操作序列。

#### REQ-003 — 测量启动
**优先级**：高 | **函数**：`AFE_StartMeasurement()`

模块应向 AFE 状态机发送初始化请求以启动测量序列。

#### REQ-004 — 首次测量周期检测
**优先级**：中 | **函数**：`AFE_IsFirstMeasurementCycleFinished()`

#### REQ-005 — 温度读取请求
**优先级**：高 | **函数**：`AFE_RequestTemperatureRead(string)`

模块应通过 AFE 从板的 I2C 接口读取外部温度传感器数据。

#### REQ-006 — 均衡反馈读取
**优先级**：中 | **函数**：`AFE_RequestBalancingFeedbackRead(string)`

#### REQ-007 — EEPROM 访问
**优先级**：中 | **函数**：`AFE_RequestEepromRead/Write(string)`

#### REQ-008 — 断线检测
**优先级**：高 | **函数**：`AFE_RequestOpenWireCheck(string)`

模块应每 `AFE_ERROR_OPEN_WIRE_PERIOD_ms`（30000ms）触发一次断线检测。

#### REQ-009 — GPO 输出控制
**优先级**：中 | **函数**：`AFE_SetGPOStateViaMask()`

#### REQ-010 — AFE 芯片识别
**优先级**：中 | **函数**：`AFE_IdentifyAfes()`

模块应返回菊花链上所有 AFE 芯片的序列 ID。

### 3.2 数据合理性校验

**函数**：`afe_plausibility.c/h`

模块应实现电压和温度数据的合理性校验，检测异常值。

### 3.3 各供应商 AFE 通用功能

所有 AFE 驱动实现以下通用功能：
- **电压采集**：读取各通道（cell）的电压值
- **温度采集**：通过 GPIO/AUX 通道读取 NTC 温度
- **均衡控制**：配置放电开关，实现被动均衡
- **故障诊断**：断线检测、通信错误检测
- **PEC/CRC 校验**：SPI 通信数据完整性保护
- **DMA 通信**：高效批量数据传输

## 4. 非功能需求

- **REQ-011**：无效测量值使用 `AFE_DEFAULT_CELL_VOLTAGE_INVALID_VALUE`（INT16_MAX）标记
- **REQ-012**：所有 AFE 通信使用 PEC/CRC 保护数据完整性
- **REQ-013**：通过编译时宏选择 AFE 供应商（`FOXBMS_AFE_DRIVER_ADI/LTC/MAXIM/NXP/TI`）

## 5. 接口需求

### 5.1 统一 API
| 函数 | 说明 |
|------|------|
| `AFE_Initialize()` | 初始化 |
| `AFE_TriggerIc()` | 状态机触发 |
| `AFE_StartMeasurement()` | 启动测量 |
| `AFE_RequestTemperatureRead(s)` | 请求温度 |
| `AFE_RequestBalancingFeedbackRead(s)` | 请求均衡反馈 |
| `AFE_RequestEepromRead/Write(s)` | EEPROM 访问 |
| `AFE_RequestOpenWireCheck(s)` | 断线检测 |
| `AFE_SetGPOStateViaMask(mask, s)` | GPO 控制 |
| `AFE_IdentifyAfes()` | 芯片识别 |

### 5.2 依赖项
- SPI 驱动（`spi.h`）
- DMA 驱动（`dma.h`）
- 各供应商驱动模块

## 6. 均衡管理

被动均衡通过 AFE 芯片的 GPIO 引脚控制放电电阻开关，由应用层 BAL（Balancing）模块通过 AFE API 下发均衡配置。
