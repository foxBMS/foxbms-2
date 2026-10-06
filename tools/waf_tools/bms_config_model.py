#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# We kindly request you to use one or more of the following phrases to refer to
# foxBMS in your hardware, software, documentation or advertising materials:
#
# - "This product uses parts of foxBMS®"
# - "This product includes parts of foxBMS®"
# - "This product is derived from foxBMS®"

"""Data model and parser for the foxBMS BMS configuration (``bms.json``).

This module defines the typed dataclass hierarchy that represents the complete
foxBMS battery management system configuration. Validation is performed by
pydantic: field types and ``Literal`` choices are checked when the model is
validated.

The dataclass hierarchy mirrors the JSON structure:

- :class:`System` (root)

    - :class:`Application` - Sensors, algorithms, balancing, redundancy.

        - :class:`Algorithm` / :class:`StateEstimation`
        - :class:`AerosolSensor`
        - :class:`CurrentSensor`
        - :class:`InsulationMonitoringDevice`

    - :class:`RTOS` - Real-time OS name and addons.
    - :class:`BMSSlave` - Analog front-end and temperature sensor.

        - :class:`AnalogFrontEnd`
        - :class:`TemperatureSensor`

    - :class:`Debug` - Debug interface selection.
    - :class:`MCU` - MCU selection.

The validators raise ``ValueError``. Pydantic collects them into a single
``ValidationError``, which :func:`parse_config_to_system` converts into one
:class:`InvalidConfigurationError` listing all invalid entries.

Usage
-----
Parse a configuration dictionary (typically read from ``bms.json``)::

    from bms_config_model import parse_config_to_system

    system = parse_config_to_system(config_dict)

The returned :class:`System` instance is then consumed by
:mod:`waf_tools.bms_config_generate` to set environment variables and generate
configuration headers.

Dependencies
------------
- Consumed by :mod:`waf_tools.bms_config_validate` and
  :mod:`waf_tools.bms_config_generate`.
"""

from collections.abc import Mapping
from dataclasses import field
from typing import Annotated, Literal

from pydantic import (
    BeforeValidator,
    ConfigDict,
    Field,
    TypeAdapter,
    ValidationError,
    ValidationInfo,
    field_validator,
    model_validator,
)
from pydantic.dataclasses import dataclass


class InvalidConfigurationError(RuntimeError):
    """Raised when the BMS configuration contains invalid or unsupported values.

    Raised by :func:`parse_config_to_system` after a pydantic
    ``ValidationError``. The message lists all invalid entries: missing keys,
    unsupported device/sensor combinations, and inconsistent type choices.
    """


def _to_field_name(name: str) -> str:
    return name.replace("_", "-")


def _to_lower(value: object) -> object:
    return value.lower() if isinstance(value, str) else value


BMS_CONFIG = ConfigDict(
    alias_generator=_to_field_name,
    populate_by_name=True,
)

Lower = BeforeValidator(_to_lower)
LowerStr = Annotated[str, Lower, Field(strict=True)]


AEROSOL_SENSOR_DISABLED: dict[str, str] = {
    "manufacturer": "none",
    "model": "none",
    "type": "ignore",
}
SUPPORTED_AEROSOL_SENSORS: dict[tuple[str, str], str] = {
    ("none", "none"): "ignore",
    ("honeywell", "bas6c-x00"): "can",
}

SUPPORTED_CURRENT_SENSORS: dict[tuple[str, str], str] = {
    ("isabellenhuette", "ivt-s"): "can",
    ("lem", "cab500"): "can",
}

IMD_DISABLED: dict[str, str] = {
    "manufacturer": "none",
    "model": "none",
    "type": "ignore",
}
SUPPORTED_INSULATION_MONITORING_DEVICES: dict[tuple[str, str], str] = {
    ("none", "none"): "ignore",
    ("bender", "ir155"): "pwm",
    ("bender", "iso165c"): "can",
}

SUPPORTED_ANALOG_FRONT_ENDS_ICS: dict[str, tuple[str, ...]] = {
    "adi": ("ades1830",),
    "debug": ("can", "default"),
    "ltc": ("6804-1", "6806", "6811-1", "6812-1", "6813-1"),
    "maxim": ("max17852",),
    "nxp": ("mc33775a",),
    "ti": ("dummy",),
}

_POLYNOMIAL_AND_LOOKUP_TABLE = ("polynomial", "lookup-table")
_LOOKUP_TABLE_ONLY = ("lookup-table",)
SUPPORTED_TEMPERATURE_SENSORS: dict[tuple[str, str], tuple[str, ...]] = {
    ("epcos", "b57251v5103j060"): _POLYNOMIAL_AND_LOOKUP_TABLE,
    ("epcos", "b57861s0103f045"): _POLYNOMIAL_AND_LOOKUP_TABLE,
    ("epcos", "b57332v5103f360"): _POLYNOMIAL_AND_LOOKUP_TABLE,
    ("fake", "fake"): _POLYNOMIAL_AND_LOOKUP_TABLE,
    ("murata", "ncxxxxh103"): _POLYNOMIAL_AND_LOOKUP_TABLE,
    ("semitec", "103jt"): _POLYNOMIAL_AND_LOOKUP_TABLE,
    ("tdk", "ntcgs103jf103ft8"): _LOOKUP_TABLE_ONLY,
    ("tdk", "ntcg163jx103dt1s"): _LOOKUP_TABLE_ONLY,
    ("tdk", "ntcb57334v5103f360"): _LOOKUP_TABLE_ONLY,
    ("vishay", "ntcalug01a103g"): _POLYNOMIAL_AND_LOOKUP_TABLE,
    ("vishay", "ntcle317e4103sba"): _POLYNOMIAL_AND_LOOKUP_TABLE,
}

_DISABLED_DEVICES: dict[str, Mapping[str, str]] = {
    "aerosol_sensor": AEROSOL_SENSOR_DISABLED,
    "insulation_monitoring_device": IMD_DISABLED,
}


def _check_device(
    manufacturer: str,
    model: str,
    device_type: str,
    supported: Mapping[tuple[str, str], str],
    device: str,
) -> None:
    expected_type = supported.get((manufacturer, model))
    if not expected_type:
        combinations = ", ".join(f"'{man}, {mod}'" for man, mod in supported)
        err_msg = (
            f"unsupported {device} '{manufacturer} {model}', "
            f"supported are: {combinations}"
        )
        raise ValueError(err_msg)
    if device_type != expected_type:
        err_msg = (
            f"invalid type '{device_type}' for {device} "
            f"'{manufacturer} {model}', expected '{expected_type}'"
        )
        raise ValueError(err_msg)


def _models_of(manufacturer: str, supported: Mapping[tuple[str, str], object]) -> str:
    models = [mod for man, mod in supported if man == manufacturer]
    return ", ".join(models)


def _is_absent(value: object) -> bool:
    if not value:
        return True
    return isinstance(value, str) and value.strip().lower() in ("", "none")


@dataclass(config=BMS_CONFIG, frozen=True)
class StateEstimation:
    """State-of-X estimation method selection for battery management.

    Configures which algorithm variant is used for each state estimation
    domain. Values are validated against a fixed set of supported methods.

    Supported methods:
        - **SOC**: ``"counting"``, ``"debug"``, ``"lookup-table"``, ``"none"``
        - **SOE**: ``"counting"``, ``"debug"``, ``"none"``
        - **SOF**: ``"trapezoid"``
        - **SOH**: ``"debug"``, ``"none"``

    Attributes:
        soc: State-of-charge estimation method
        soe: State-of-energy estimation method
        sof: State-of-function estimation method
        soh: State-of-health estimation method
    """

    soc: Annotated[Literal["counting", "debug", "lookup-table", "none"], Lower]
    soe: Annotated[Literal["counting", "debug", "none"], Lower]
    sof: Annotated[Literal["trapezoid"], Lower]
    soh: Annotated[Literal["debug", "none"], Lower]


@dataclass(config=BMS_CONFIG, frozen=True)
class Algorithm:
    """Algorithm configuration container.

    Wraps the :class:`StateEstimation` configuration under the ``algorithm``
    JSON key.

    Attributes:
        state_estimation: The state estimation method configuration.
    """

    state_estimation: StateEstimation


@dataclass(config=BMS_CONFIG, frozen=True)
class AerosolSensor:
    """Aerosol sensor hardware configuration.

    All fields are required. The sensor is disabled by omitting the
    ``aerosol-sensor`` key in ``bms.json``, see ``_replace_disabled_device``.

    Attributes:
        manufacturer: Sensor manufacturer.
        model: Sensor model identifier.
        _type: Communication type.

    Raises:
        ValueError: If the manufacturer/model/type combination is unsupported
            or inconsistent.
    """

    manufacturer: LowerStr
    model: LowerStr
    type: LowerStr

    @model_validator(mode="after")
    def _check_supported(self) -> "AerosolSensor":
        _check_device(
            self.manufacturer,
            self.model,
            self.type,
            SUPPORTED_AEROSOL_SENSORS,
            "aerosol sensor",
        )
        return self


@dataclass(config=BMS_CONFIG, frozen=True)
class CurrentSensor:
    """Current sensor hardware configuration.

    Defines which current sensor is used in the system and its communication
    interface type.

    Attributes:
        manufacturer: Sensor manufacturer.
        model: Sensor model identifier.
        _type: Communication type.

    Raises:
        ValueError: If the manufacturer/model/type combination is unsupported
            or inconsistent.
    """

    manufacturer: LowerStr
    model: LowerStr
    type: LowerStr

    @model_validator(mode="after")
    def _check_supported(self) -> "CurrentSensor":
        _check_device(
            self.manufacturer,
            self.model,
            self.type,
            SUPPORTED_CURRENT_SENSORS,
            "current sensor",
        )
        return self


@dataclass(config=BMS_CONFIG, frozen=True)
class InsulationMonitoringDevice:
    """Insulation monitoring device configuration.

    All fields are required. Insulation monitoring is disabled by omitting the
    ``insulation-monitoring-device`` key in ``bms.json`` or by configuring the
    combination ``none``/``none``/``ignore`` explicitly.

    Attributes:
        manufacturer: IMD manufacturer.
        model: IMD model identifier.
        _type: Communication type.

    Raises:
        ValueError: If the manufacturer/model/type combination is unsupported
            or inconsistent.
    """

    manufacturer: LowerStr
    model: LowerStr
    type: LowerStr

    @model_validator(mode="after")
    def _check_supported(self) -> "InsulationMonitoringDevice":
        _check_device(
            self.manufacturer,
            self.model,
            self.type,
            SUPPORTED_INSULATION_MONITORING_DEVICES,
            "insulation monitoring device",
        )
        return self


def _disabled_aerosol_sensor() -> AerosolSensor:
    return AerosolSensor(**AEROSOL_SENSOR_DISABLED)


def _disabled_insulation_monitoring_device() -> InsulationMonitoringDevice:
    return InsulationMonitoringDevice(**IMD_DISABLED)


@dataclass(config=BMS_CONFIG, frozen=True)
class Application:
    """Application-level configuration, including sensors and algorithms.

    Groups all application-domain configuration: state estimation algorithms,
    sensor selections, balancing strategy, and redundancy settings.

    ``aerosol_sensor`` and ``insulation_monitoring_device`` are optional. If the
    key is omitted, ``null``, ``{}`` or ``"none"``, the device is replaced by
    its disabled default configuration.

    Attributes:
        algorithm: Algorithm and state estimation configuration.
        aerosol_sensor: Aerosol sensor hardware selection (default: disabled).
        balancing_strategy: Active balancing strategy name.
        current_sensor: Current sensor hardware selection.
        insulation_monitoring_device: IMD hardware selection (default:
            disabled).
        redundant_v_t_measurement: Whether redundant voltage and temperature
            measurement paths are enabled (default: ``True``).
    """

    algorithm: Algorithm
    current_sensor: CurrentSensor
    aerosol_sensor: AerosolSensor = field(default_factory=_disabled_aerosol_sensor)
    insulation_monitoring_device: InsulationMonitoringDevice = field(
        default_factory=_disabled_insulation_monitoring_device
    )
    balancing_strategy: Annotated[Literal["voltage", "history", "none"], Lower] = "none"
    redundant_v_t_measurement: bool = False

    @field_validator("aerosol_sensor", "insulation_monitoring_device", mode="before")
    @classmethod
    def _replace_disabled_device(cls, value: object, info: ValidationInfo) -> object:
        if _is_absent(value):
            return dict(_DISABLED_DEVICES[info.field_name])
        return value


@dataclass(config=BMS_CONFIG, frozen=True)
class RTOS:
    """Real-Time Operating System configuration.

    Specifies which RTOS kernel and optional addons are used by the
    foxBMS firmware.

    Attributes:
        name: RTOS kernel name.
        addons: List of addon identifiers.
    """

    name: Annotated[Literal["freertos"], Lower]
    addons: list[LowerStr] = field(default_factory=list)


@dataclass(config=BMS_CONFIG, frozen=True)
class AnalogFrontEnd:
    """Analog front-end IC configuration.

    Defines the battery monitoring IC manufacturer and model used on the
    BMS-Slave board.

    Attributes:
        ic: IC model identifier.
        manufacturer: IC manufacturer name.

    Raises:
        ValueError: If the manufacturer/IC combination is unsupported.
    """

    ic: LowerStr
    manufacturer: Annotated[
        Literal["adi", "debug", "ltc", "maxim", "nxp", "st", "ti"], Lower
    ]

    @model_validator(mode="after")
    def _check_supported(self) -> "AnalogFrontEnd":
        supported = SUPPORTED_ANALOG_FRONT_ENDS_ICS[self.manufacturer]
        if self.ic not in supported:
            err_msg = (
                f"unsupported AFE IC '{self.ic}' for manufacturer "
                f"'{self.manufacturer}', supported are: {', '.join(supported)}"
            )
            raise ValueError(err_msg)
        return self


@dataclass(config=BMS_CONFIG, frozen=True)
class TemperatureSensor:
    """Temperature sensor configuration for the BMS-Slave board.

    Defines the NTC thermistor used for cell temperature measurement and
    the evaluation method.

    Attributes:
        manufacturer: Sensor manufacturer name.
        method: Evaluation method.
        model: Sensor model/part number.

    Raises:
        ValueError: If the manufacturer/model/method combination is unsupported.
    """

    manufacturer: Annotated[
        Literal["epcos", "fake", "murata", "semitec", "tdk", "vishay"], Lower
    ]
    method: LowerStr
    model: LowerStr

    @model_validator(mode="after")
    def _check_supported(self) -> "TemperatureSensor":
        methods = SUPPORTED_TEMPERATURE_SENSORS.get((self.manufacturer, self.model))
        if not methods:
            models = _models_of(self.manufacturer, SUPPORTED_TEMPERATURE_SENSORS)
            err_msg = (
                "unsupported temperature sensor "
                f"'{self.manufacturer} {self.model}', supported are: {models}"
            )
            raise ValueError(err_msg)
        if self.method not in methods:
            err_msg = (
                f"unsupported evaluation method '{self.method}' for "
                f"'{self.manufacturer} {self.model}', "
                f"supported are: {', '.join(methods)}"
            )
            raise ValueError(err_msg)
        return self


@dataclass(config=BMS_CONFIG, frozen=True)
class BMSSlave:
    """BMS-Slave board configuration.

    Groups the analog front-end IC and temperature sensor configuration
    for the slave measurement board.

    Attributes:
        analog_front_end: AFE IC selection and manufacturer.
        temperature_sensor: NTC thermistor selection and evaluation method.
    """

    analog_front_end: AnalogFrontEnd
    temperature_sensor: TemperatureSensor


@dataclass(config=BMS_CONFIG, frozen=True)
class MCU:
    """MCU configuration.

    Groups the MCU configuration for the BMS-Master.

    Attributes:
        use_cache: Whether to use the MCU's cache feature or not.
    """

    use_cache: bool = False


@dataclass(config=BMS_CONFIG, frozen=True)
class Debug:
    """Debug interface configuration.

    Specifies which debug communication interfaces are enabled in the firmware
    build.

    Attributes:
        interfaces: List of enabled debug interface names.
    """

    interfaces: list[Annotated[Literal["uart"], Lower]] = field(default_factory=list)


@dataclass(config=BMS_CONFIG, frozen=True)
class System:
    """Root configuration object representing the complete BMS system.

    Aggregates all configuration domains (application, RTOS, BMS-Slave, debug)
    into a single validated instance. This is the object returned by
    :func:`parse_config_to_system` and consumed by build tools.

    Attributes:
        application: Application-level configuration (sensors, algorithms,
            balancing, redundancy).
        rtos: Real-time operating system selection and addons.
        bms_slave: BMS-Slave board hardware configuration.
        mcu: MCU configuration
        debug: Debug interface configuration.
    """

    application: Application
    rtos: RTOS
    bms_slave: BMSSlave
    mcu: MCU
    debug: Debug = field(default_factory=Debug)


_SYSTEM_ADAPTER: TypeAdapter[System] = TypeAdapter(System)


def _format_validation_error(error: ValidationError) -> str:
    count = error.error_count()
    entry = "entry" if count == 1 else "entries"
    lines = [f"{count} invalid {entry} in the BMS configuration:"]
    for err in error.errors(include_url=False):
        key = " -> ".join(str(part) for part in err["loc"]) or "<System>"
        msg = err["msg"].removeprefix("Value error, ")
        if err["type"] == "missing":
            lines.append(f"  {key}: required key is missing")
        elif isinstance(err["input"], (dict, list)):
            lines.append(f"  {key}: {msg}")
        else:
            lines.append(f"  {key}: {msg} (got {err['input']})")
    return "\n".join(lines)


def parse_config_to_system(config: dict) -> System:
    """Parse a JSON configuration into a validated system configuration.

    Maps the raw configuration dictionary (the content of
    ``bms.json``) to the typed dataclass hierarchy defined in this module.
    Validation is performed by the module-level ``TypeAdapter``.
    This is the entry point of this module and is called by
    :func:`waf_tools.bms_config_validate.validate_bms_configuration`.

    Args:
        config: Parsed JSON content of ``bms.json``, expected to contain the
            sections ``application``, ``rtos``, ``bms-slave`` and optionally
            ``debug``.

    Returns:
        A fully populated and validated :class:`System` instance.

    Raises:
        InvalidConfigurationError: If a required key is missing or a
            configuration value is unsupported or inconsistent. The message is
            produced by ``_format_validation_error()``.
    """
    try:
        return _SYSTEM_ADAPTER.validate_python(config)
    except ValidationError as e:
        raise InvalidConfigurationError(_format_validation_error(e)) from e
