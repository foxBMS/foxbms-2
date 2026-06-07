# PHY — 以太网物理层驱动软件需求规格说明

## 1. 引言

### 1.1 目的
本文档定义 foxBMS 2 中 PHY（DP83869HM）以太网物理层驱动模块的软件需求。PHY 模块负责配置和管理 TI DP83869HM 以太网 PHY 芯片，通过 MDIO 接口进行寄存器操作。

### 1.2 范围
- **涵盖**：PHY 初始化与复位、自协商、链路状态检测、MII 模式设置、回环测试
- **不涵盖**：EMAC 上层驱动、TCP/IP 协议栈

## 2. 功能需求

#### REQ-001 — PHY 初始化
**优先级**：高 | **函数**：`PHY_Initialize()`

#### REQ-002 — PHY ID 读取
**优先级**：中 | **函数**：`PHY_GetId()`

模块应通过 MDIO 读取 PHY 芯片 ID（预期值 `0x2000A0F1`）用于验证硬件连接。

#### REQ-003 — 软件/硬件复位
**优先级**：高 | **函数**：`PHY_ResetSoftware()`, `PHY_ResetHardware()`

#### REQ-004 — 自协商
**优先级**：高 | **函数**：`PHY_AutoNegotiate()`

模块应发起与链路伙伴的自协商，支持 100BTX_FD、100BTX、10BT_FD、10BT 能力组合。

#### REQ-005 — 链路状态检测
**优先级**：高 | **函数**：`PHY_GetLinkStatus()`

#### REQ-006 — 伙伴能力读取
**优先级**：中 | **函数**：`PHY_GetPartnerAbility()`

#### REQ-007 — MII 模式设置
**优先级**：中 | **函数**：`PHY_SetMiiMode()`

模块应将 PHY 从默认 RGMII 模式切换到 MII 模式。

#### REQ-008 — 回环测试
**优先级**：低 | **函数**：`PHY_EnableLoopback()`

#### REQ-009 — 操作模式获取
**优先级**：中 | **函数**：`PHY_GetOperationMode()`

模块应读取 PHY Strap Status Register 获取上电时由引脚配置的操作模式。

## 3. 数据结构
- `PHY_OPERATION_MODE_s`：操作模式（RGMII→MII 使能、自协商使能、PHY 地址、镜像模式等）
- `PHY_SPEED_SEL_e`：速度选择（10/100/1000 Mbps）

## 4. 接口需求（10 个公开函数）
| 函数 | 说明 |
|------|------|
| `PHY_Initialize/GetId/Reset*` | 初始化与复位 |
| `PHY_AutoNegotiate/GetLinkStatus/GetPartnerAbility` | 自协商与链路 |
| `PHY_SetMiiMode/ResetMii/EnableLoopback` | 模式与测试 |
| `PHY_GetOperationMode` | 操作模式 |

## 5. 来源说明
本模块基于 TI HALCoGen 生成的 DP83640 驱动修改，以支持 DP83869HM PHY。
