# DIAG CBS — 诊断回调函数模块软件需求规格说明

## 1. 引言

诊断回调函数（CBS）模块提供各诊断事件触发时执行的具体业务逻辑。每个回调函数对应一类诊断事件（如过压、欠压、过温、通信故障等），由 `DIAG_Handler()` 在事件状态变更时调用。

## 2. 回调函数清单

| 回调函数 | 对应诊断事件 | 源文件 |
|----------|-------------|--------|
| `DIAG_DummyCallback()` | 默认/占位回调 | diag_cbs_dummy.c |
| `DIAG_ErrorOvervoltage()` | 过压事件 | diag_cbs_voltage.c |
| `DIAG_ErrorUndervoltage()` | 欠压事件 | diag_cbs_voltage.c |
| `DIAG_ErrorOvertemperatureCharge()` | 充电过温 | diag_cbs_temperature.c |
| `DIAG_ErrorOvertemperatureDischarge()` | 放电过温 | diag_cbs_temperature.c |
| `DIAG_ErrorUndertemperatureCharge()` | 充电欠温 | diag_cbs_temperature.c |
| `DIAG_ErrorUndertemperatureDischarge()` | 放电欠温 | diag_cbs_temperature.c |
| `DIAG_ErrorOvercurrentCharge()` | 充电过流 | diag_cbs_current.c |
| `DIAG_ErrorOvercurrentDischarge()` | 放电过流 | diag_cbs_current.c |
| `DIAG_ErrorCurrentOnOpenString()` | 开路串电流 | diag_cbs_current.c |
| `DIAG_ErrorCurrentMeasurement()` | 电流测量故障 | diag_cbs_current-sensor.c |
| `DIAG_ErrorHighVoltageMeasurement()` | 高压测量故障 | diag_cbs_current-sensor.c |
| `DIAG_ErrorPowerMeasurement()` | 功率测量故障 | diag_cbs_power-measurement.c |
| `DIAG_ErrorSystemMonitoring()` | 系统监控违规 | diag_cbs_sys-mon.c |
| `DIAG_ErrorInterlock()` | 互锁故障 | diag_cbs_interlock.c |
| `DIAG_ErrorCanTiming()` | CAN 时序违规 | diag_cbs_can.c |
| `DIAG_ErrorCanRxQueueFull()` | CAN 接收队列满 | diag_cbs_can.c |
| `DIAG_ErrorCanTxQueueFull()` | CAN 发送队列满 | diag_cbs_can.c |
| `DIAG_ErrorAfeDriver()` | AFE 驱动故障 | diag_cbs_afe.c |
| `DIAG_ErrorAfe()` | AFE 模块故障 | diag_cbs_afe.c |
| `DIAG_ErrorCurrentSensor()` | 电流传感器故障 | diag_cbs_current-sensor.c |
| `DIAG_Sbc()` | SBC 故障 | diag_cbs_sbc.c |
| `DIAG_ErrorPlausibility()` | 合理性检查故障 | diag_cbs_plausibility.c |
| `DIAG_StringContactorFeedback()` | 串接触器反馈故障 | diag_cbs_contactor.c |
| `DIAG_PrechargeContactorFeedback()` | 预充电接触器反馈故障 | diag_cbs_contactor.c |
| `DIAG_PlausibilityCheck()` | 合理性检查 | diag_cbs_plausibility.c |
| `DIAG_ErrorDeepDischarge()` | 深度放电 | diag_cbs_deep-discharge.c |
| `DIAG_Insulation()` | 绝缘监测故障 | diag_cbs_insulation.c |
| `DIAG_I2c()` | I2C 通信故障 | diag_cbs_i2c.c |
| `DIAG_FramError()` | FRAM 故障 | diag_cbs_fram.c |
| `DIAG_Rtc()` | RTC 故障 | diag_cbs_rtc.c |
| `DIAG_AlertFlag()` | ALERT 标志 | diag_cbs_bms.c |
| `DIAG_PrechargeProcess()` | 预充电流程故障 | diag_cbs_contactor.c |
| `DIAG_AerosolAlert()` | 气溶胶警报 | diag_cbs_aerosol-sensor.c |
| `DIAG_SupplyVoltageClamp30c()` | 30C 供电电压故障 | diag_cbs_clamp30c.c |

## 3. 通用接口规范

所有回调函数遵循统一的函数签名：

```c
void DIAG_Callback(
    DIAG_ID_e diagId,
    DIAG_EVENT_e event,
    const DIAG_DATABASE_SHIM_s *const kpkDiagShim,
    uint32_t data);
```

| 参数 | 说明 |
|------|------|
| `diagId` | 诊断事件 ID |
| `event` | 事件类型（`DIAG_EVENT_OK` / `DIAG_EVENT_NOT_OK` / `DIAG_EVENT_RESET`） |
| `kpkDiagShim` | 数据库访问垫片（Shim），提供对数据库条目的只读访问 |
| `data` | 附加信息（如串号等） |

## 4. 功能需求

### REQ-APP_ENGINE_DIAG_CBS_001 — 诊断回调统一签名

**编号** | REQ-APP_ENGINE_DIAG_CBS_001
**优先级** | 高
**文件** | [diag_cbs.h](diag_cbs.h)，所有 diag_cbs_*.c

**描述**：所有诊断回调函数应遵循统一的函数签名，接收 `diagId`、`event`、`kpkDiagShim`、`data` 四个参数，由 `DIAG_Handler()` 在检测到事件状态变化时调用。

### REQ-APP_ENGINE_DIAG_CBS_002 — 占位回调

**编号** | REQ-APP_ENGINE_DIAG_CBS_002
**优先级** | 低
**函数** | `DIAG_DummyCallback()`
**文件** | [diag_cbs_dummy.c](diag_cbs_dummy.c)

**描述**：系统应提供一个占位回调函数，对所有输入参数进行断言有效性检查，但不执行任何实际业务逻辑。用于未配置特定回调的诊断条目。

### REQ-APP_ENGINE_DIAG_CBS_003 — 安全限值回调

**编号** | REQ-APP_ENGINE_DIAG_CBS_003
**优先级** | 高
**文件** | diag_cbs_voltage.c, diag_cbs_temperature.c, diag_cbs_current.c

**描述**：系统应为电压、温度、电流相关的安全限值违规提供回调函数，在事件触发时更新对应的 MSL/RSL/MOL 标志位（通过数据库写入）。回调函数应区分串级（STRING）事件和系统级（SYSTEM）事件。
