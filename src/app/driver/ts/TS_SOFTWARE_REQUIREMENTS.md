# TS/TSI — 温度传感器接口软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 TS（Temperature Sensor）温度传感器模块的软件需求。TS 模块负责将通过 ADC 测量的 NTC 热敏电阻电压值转换为温度值（分摄氏度/ddegC），支持多种传感器型号和两种转换方法。

### 1.2 范围
- **涵盖**：TSI 温度转换接口、多传感器型号支持、查找表法和多项式法
- **不涵盖**：ADC 电压测量、传感器硬件选型

### 1.3 转换方法
| 方法 | 说明 |
|------|------|
| 查找表法 (Lookup Table) | 基于预计算的电压-温度映射表进行二分查找 |
| 多项式法 (Polynomial) | 使用拟合多项式函数计算温度 |

## 2. 模块结构（44 个文件）

```
ts/
├── api/
│   ├── tsi.h                        # TSI 统一接口
│   └── tsi_limits.c                 # 温度合理性限值
├── beta.c/h                          # Beta 参数法（备用）
├── epcos/
│   ├── b57251v5103j060/              # EPCOS B57251V5103J060
│   ├── b57332v5103f360/              # EPCOS B57332V5103F360
│   └── b57861s0103f045/              # EPCOS B57861S0103F045
├── fake/none/                        # 模拟传感器（测试用）
├── murata/ncxxxxh103/                # Murata NCxxxxH103
├── semitec/103jt/                    # Semitec 103JT
├── tdk/
│   ├── ntcg163jx103dt1s/             # TDK NTCG163JX103DT1S
│   └── ntcgs103jf103ft8/             # TDK NTCGS103JF103FT8
└── vishay/
    ├── ntcalug01a103g/               # Vishay NTCALUG01A103G
    ├── ntcle317e4103sba/             # Vishay NTCLE317E4103SBA
    └── ntcle413e2103f102l/           # Vishay NTCLE413E2103F102L
```

## 3. 功能需求

### 3.1 TSI 统一接口

#### REQ-001 — 电压→温度转换
**优先级**：高 | **函数**：`TSI_GetTemperature(adcVoltage_mV)`

模块应将 ADC 测量的电压（mV）转换为温度值（ddegC），根据编译配置选择对应的传感器实现。

#### REQ-002 — 温度合理性上限
**优先级**：中 | **函数**：`TSI_GetMaximumPlausibleTemperature()`

模块应返回所选传感器可返回的最大合理温度值。

#### REQ-003 — 温度合理性下限
**优先级**：中 | **函数**：`TSI_GetMinimumPlausibleTemperature()`

#### REQ-004 — 温度合理性限值表
**优先级**：中 | **文件**：`tsi_limits.c`

模块应维护各温度传感器型号对应的合理性限值配置表。

### 3.2 传感器实现模式

每个传感器型号支持两种实现：
- **查找表法** (`polynomial/*_polynomial.c`)：使用预计算表 + 线性插值
- **多项式法** (`lookup-table/*_lookup-table.c`)：使用 Steinhart-Hart 多项式或其简化形式

### 3.3 支持的 NTC 传感器型号

| 供应商 | 型号 | B 值 |
|--------|------|------|
| EPCOS | B57251V5103J060 | - |
| EPCOS | B57332V5103F360 | - |
| EPCOS | B57861S0103F045 | - |
| Murata | NCxxxxH103 | - |
| Semitec | 103JT | - |
| TDK | NTCG163JX103DT1S | - |
| TDK | NTCGS103JF103FT8 | - |
| Vishay | NTCALUG01A103G | - |
| Vishay | NTCLE317E4103SBA | - |
| Vishay | NTCLE413E2103F102L | - |
| Fake/None | 模拟/无传感器 | 测试用 |

## 4. 非功能需求

- **REQ-005**：温度输出使用 int16_t 类型，单位为分摄氏度（ddegC，0.1°C 分辨率）
- **REQ-006**：查找表法使用二分查找，时间复杂度 O(log n)

## 5. 接口需求

| 函数 | 说明 |
|------|------|
| `TSI_GetTemperature(adcVoltage_mV)` | 电压→温度转换 |
| `TSI_GetMaximumPlausibleTemperature()` | 最大合理温度 |
| `TSI_GetMinimumPlausibleTemperature()` | 最小合理温度 |

## 6. Beta 参数法
`beta.c/h` 提供基于 NTC Beta 参数的转换方法作为备用方案。
