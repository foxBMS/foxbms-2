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

"""Validate BMS configuration files"""

from dataclasses import dataclass, field, fields


KEY_MAP = {
    # 1. level keys
    "application": "application",
    "rtos": "rtos",
    "bms-slave": "bms-slave",
    # 2. level application keys
    "insulation-monitoring-device": "application/insulation-monitoring-device",
    "algorithm": "application/algorithm",
    "current-sensor": "application/current-sensor",
    "state-estimation": "application/algorithm/state-estimation",
    "soc": "application/algorithm/state-estimation/soc",
    "soe": "application/algorithm/state-estimation/soe",
    "sof": "application/algorithm/state-estimation/sof",
    "soh": "application/algorithm/state-estimation/soh",
    # 2. level bms-slave keys
    "analog-front-end": "bms-slave/analog-front-end",
    "temperature-sensor": "bms-slave/temperature-sensor",
}


class InvalidConfigurationError(RuntimeError):
    """Generate a detailed error for an invalid configuration."""


def to_define(name: str) -> str:
    """Create a macro from a provided string"""
    return "".join([c if c.isalnum() else "_" for c in name.upper()])


@dataclass(kw_only=True)
class StateEstimation:
    """State-of-X estimation methods for battery management."""

    soc: str = "none"
    soe: str = "none"
    sof: str = "trapezoid"
    soh: str = "none"

    def __post_init__(self):
        # stringify 'None' as we need that to find the and validate
        for f in fields(self):
            val = getattr(self, f.name)
            if isinstance(val, str):
                val = val.lower()
            setattr(self, f.name, val)

        if self.soc not in ("counting", "debug", "lookup-table", "none"):
            err_msg = f"Invalid 'soc' configuration: {self.soc}"
            raise InvalidConfigurationError(err_msg)
        if self.soe not in ("counting", "debug", "none"):
            err_msg = f"Invalid 'soe' configuration: {self.soe}"
            raise InvalidConfigurationError(err_msg)
        if self.sof not in ("trapezoid",):
            err_msg = f"Invalid 'sof' configuration: {self.sof}"
            raise InvalidConfigurationError(err_msg)
        if self.soh not in ("debug", "none"):
            err_msg = f"Invalid 'soh' configuration: {self.soh}"
            raise InvalidConfigurationError(err_msg)


@dataclass
class Algorithm:
    """Algorithm configuration containing state estimation settings."""

    state_estimation: StateEstimation


@dataclass
class AerosolSensor:
    """Aerosol sensor configuration."""

    manufacturer: str | None
    model: str | None
    type: str | None

    def __post_init__(self):
        values = [getattr(self, f.name) for f in fields(self)]
        any_empty = any(v is None for v in values)
        all_empty = all(v is None for v in values)
        if any_empty and not all_empty:
            err_msg = (
                "AerosolSensor can only be instantiated with either all "
                "fields provided or none of them"
            )
            raise ValueError(err_msg)

        if not self.manufacturer:
            self.manufacturer = "none"
            self.model = "none"
            self.type = "ignore"
        elif self.manufacturer == "honeywell":
            if self.model not in ("bas6c-x00",):
                err_msg = "Unsupported aerosol sensor."
                raise InvalidConfigurationError(err_msg)

            if self.model != "bas6c-x00" and self.type != "can":
                err_msg = "Invalid type choice for the aerosol sensor."
                raise InvalidConfigurationError(err_msg)


@dataclass
class CurrentSensor:
    """Current sensor configuration."""

    manufacturer: str
    model: str
    type: str

    def __post_init__(self):
        class UnsupportedCurrentSensorError(InvalidConfigurationError):
            def __init__(self):
                super().__init__("Unsupported current sensor")

        class InvalidTypeChoiceForCurrentSensorError(InvalidConfigurationError):
            def __init__(self):
                super().__init__("Invalid type choice for current sensor")

        for f in fields(self):
            val = getattr(self, f.name)
            setattr(self, f.name, val.lower())

        if self.manufacturer == "isabellenhuette":
            if self.model not in ("ivt-s",):
                raise UnsupportedCurrentSensorError

            if self.model == "ivt-s" and self.type != "can":
                raise InvalidTypeChoiceForCurrentSensorError

        elif self.manufacturer == "lem":
            if self.model not in ("cab500",):
                raise UnsupportedCurrentSensorError

            if self.model == "cab500" and self.type != "can":
                raise InvalidTypeChoiceForCurrentSensorError

        else:
            raise UnsupportedCurrentSensorError


@dataclass
class InsulationMonitoringDevice:
    """Insulation monitoring device configuration."""

    manufacturer: str
    model: str = "none"
    type: str = "ignore"

    def __post_init__(self):
        class UnsupportedInsulationMonitoringDeviceError(InvalidConfigurationError):
            def __init__(self):
                super().__init__("Unsupported insulation monitoring device")

        class InvalidTypeChoiceForInsulationMonitoringDeviceError(
            InvalidConfigurationError
        ):
            def __init__(self):
                super().__init__("Invalid type choice for insulation monitoring device")

        if self.manufacturer == "none":
            pass
        elif self.manufacturer == "bender":
            if self.model not in ("ir155", "iso165c"):
                raise UnsupportedInsulationMonitoringDeviceError
            if self.model == "ir155" and self.type != "pwm":
                raise InvalidTypeChoiceForInsulationMonitoringDeviceError
            if self.model == "iso165c" and self.type != "can":
                raise InvalidTypeChoiceForInsulationMonitoringDeviceError
        else:
            raise UnsupportedInsulationMonitoringDeviceError


@dataclass
class Application:
    """Application configuration, including sensors and algorithms."""

    algorithm: Algorithm
    aerosol_sensor: AerosolSensor
    balancing_strategy: str
    current_sensor: CurrentSensor
    insulation_monitoring_device: InsulationMonitoringDevice

    def __post_init__(self):
        class InvalidTypeChoiceForBalancingStrategyError(InvalidConfigurationError):
            def __init__(self):
                super().__init__("Invalid type choice for balancing strategy")

        if self.balancing_strategy not in ("voltage", "history", "none"):
            raise InvalidTypeChoiceForBalancingStrategyError


@dataclass
class RTOS:
    """Real-Time Operating System configuration."""

    name: str
    addons: list[str] = field(default_factory=list)

    def __post_init__(self):
        class InvalidRtosChoiceError(InvalidConfigurationError):
            def __init__(self):
                super().__init__("Invalid RTOS choice")

        self.name = self.name.lower()
        for i, val in enumerate(self.addons):
            self.addons[i] = val.lower()
        if self.name not in ("freertos",):
            raise InvalidRtosChoiceError


@dataclass
class AnalogFrontEnd:
    """Analog front-end IC configuration."""

    ic: str
    manufacturer: str

    def __post_init__(self):
        class UnsupportedAfeError(InvalidConfigurationError):
            def __init__(self):
                super().__init__("Unsupported AFE IC")

        for f in fields(self):
            val = getattr(self, f.name)
            setattr(self, f.name, val.lower())
        if self.manufacturer == "adi":
            if self.ic not in ("ades1830",):
                raise UnsupportedAfeError
        elif self.manufacturer == "debug":
            if self.ic not in ("can", "default"):
                raise UnsupportedAfeError
        elif self.manufacturer == "ltc":
            if self.ic not in ("6804-1", "6806", "6811-1", "6812-1", "6813-1"):
                raise UnsupportedAfeError
        elif self.manufacturer == "maxim":
            if self.ic not in ("max17852",):
                raise UnsupportedAfeError
        elif self.manufacturer == "nxp":
            if self.ic not in ("mc33775a",):
                raise UnsupportedAfeError
        elif self.manufacturer == "ti":
            if self.ic not in ("dummy",):
                raise UnsupportedAfeError
        else:
            raise UnsupportedAfeError


@dataclass
class TemperatureSensor:
    """Temperature sensor configuration."""

    manufacturer: str
    method: str
    model: str

    class UnsupportedTemperatureSensorError(InvalidConfigurationError):
        """Unsupported temperature sensor"""

        def __init__(self):
            super().__init__("Unsupported temperature sensor")

    class UnsupportedTemperatureSensorEvaluationMethodError(InvalidConfigurationError):
        """Unsupported temperature sensor evaluation method"""

        def __init__(self):
            super().__init__("Unsupported temperature sensor evaluation method")

    def __post_init__(self):
        for f in fields(self):
            val = getattr(self, f.name)
            setattr(self, f.name, val.lower())

        if self.manufacturer == "epcos":
            self._validate_epcos()
        elif self.manufacturer == "fake":
            self._validate_fake()
        elif self.manufacturer == "murata":
            self._validate_murata()
        elif self.manufacturer == "semitec":
            self._validate_semitec()
        elif self.manufacturer == "tdk":
            self._validate_tdk()
        elif self.manufacturer == "vishay":
            self._validate_vishay()
        else:
            raise self.UnsupportedTemperatureSensorError

    def _validate_epcos(self) -> None:
        if self.model not in ("b57251v5103j060", "b57861s0103f045", "b57332v5103f360"):
            raise self.UnsupportedTemperatureSensorError
        if self.model == "b57251v5103j060" and self.method not in (
            "polynomial",
            "lookup-table",
        ):
            raise self.UnsupportedTemperatureSensorEvaluationMethodError
        if self.model == "b57861s0103f045" and self.method not in (
            "polynomial",
            "lookup-table",
        ):
            raise self.UnsupportedTemperatureSensorEvaluationMethodError
        if self.model == "b57332v5103f360" and self.method not in (
            "polynomial",
            "lookup-table",
        ):
            raise self.UnsupportedTemperatureSensorEvaluationMethodError

    def _validate_fake(self) -> None:
        if self.model not in ("fake",):
            raise self.UnsupportedTemperatureSensorError
        if self.model == "fake" and self.method not in ("polynomial", "lookup-table"):
            raise self.UnsupportedTemperatureSensorEvaluationMethodError

    def _validate_murata(self) -> None:
        if self.model not in ("ncxxxxh103",):
            raise self.UnsupportedTemperatureSensorError
        if self.model == "ncxxxxh103" and self.method not in (
            "polynomial",
            "lookup-table",
        ):
            raise self.UnsupportedTemperatureSensorEvaluationMethodError

    def _validate_semitec(self) -> None:
        if self.model not in ("103jt",):
            raise self.UnsupportedTemperatureSensorError
        if self.model == "103jt" and self.method not in ("polynomial", "lookup-table"):
            raise self.UnsupportedTemperatureSensorEvaluationMethodError

    def _validate_tdk(self) -> None:
        if self.model not in ("ntcgs103jf103ft8", "ntcg163jx103dt1s"):
            raise self.UnsupportedTemperatureSensorError
        if self.model == "ntcgs103jf103ft8" and self.method not in ("lookup-table",):
            raise self.UnsupportedTemperatureSensorEvaluationMethodError
        if self.model == "ntcg163jx103dt1s" and self.method not in ("lookup-table",):
            raise self.UnsupportedTemperatureSensorEvaluationMethodError

    def _validate_vishay(self) -> None:
        if self.model not in ("ntcalug01a103g", "ntcle317e4103sba"):
            raise self.UnsupportedTemperatureSensorError
        if self.model == "ntcalug01a103g" and self.method not in (
            "polynomial",
            "lookup-table",
        ):
            raise self.UnsupportedTemperatureSensorEvaluationMethodError
        if self.model == "ntcle317e4103sba" and self.method not in (
            "polynomial",
            "lookup-table",
        ):
            raise self.UnsupportedTemperatureSensorEvaluationMethodError


@dataclass
class BMSSlave:
    """BMS-Slave configuration."""

    analog_front_end: AnalogFrontEnd
    temperature_sensor: TemperatureSensor


@dataclass
class Debug:
    """Debug configuration."""

    interfaces: list[str] = field(default_factory=list)

    def __post_init__(self):
        for i, val in enumerate(self.interfaces):
            self.interfaces[i] = val.lower()
            if val not in ("uart",):
                err_msg = f"Invalid choice: 'debug:interfaces' -> {val}"
                raise InvalidConfigurationError(err_msg)


@dataclass
class System:
    """Complete configuration for the system."""

    application: Application
    rtos: RTOS
    bms_slave: BMSSlave
    debug: Debug


def _parse_state_estimation(config: dict) -> StateEstimation:
    algorithm = config.get("algorithm")
    if not algorithm:
        err = f"Key '{KEY_MAP['algorithm']}' is required."
        raise InvalidConfigurationError(err)

    state_estimation = algorithm.get("state-estimation")
    if not state_estimation:
        err = f"Key '{KEY_MAP['state-estimation']}' is required."
        raise InvalidConfigurationError(err)

    algo_setup = {}
    for algo in ("soc", "soe", "sof", "soh"):
        tmp = state_estimation.get(algo)
        if not tmp:
            err = f"Key '{KEY_MAP[algo]}' is required."
            raise InvalidConfigurationError(err)
        algo_setup[algo] = tmp

    tmp = state_estimation.get("soh")
    algo_setup["soh"] = tmp

    return StateEstimation(**algo_setup)


def _parse_aerosol_sensor(config: dict) -> AerosolSensor:
    device = config.get("aerosol-sensor")
    if not device:
        # Aerosol sensor is optional
        return AerosolSensor(None, None, None)

    try:
        manufacturer = device["manufacturer"]
        model = device["model"]
        _type = device["type"]
    except KeyError:
        msg = "'aerosol-sensor' requires keys 'manufacturer', 'model', 'type'"
        raise InvalidConfigurationError(msg) from None

    return AerosolSensor(manufacturer, model, _type)


def _parse_balancing_strategy(config: dict) -> str:
    return config.get("balancing-strategy", "none")


def _parse_current_sensor(config: dict) -> CurrentSensor:
    device = config.get("current-sensor")
    if not device:
        err = f"Key '{KEY_MAP['current-sensor']}' is required."
        raise InvalidConfigurationError(err)

    try:
        manufacturer = device["manufacturer"]
        model = device["model"]
        _type = device["type"]
    except KeyError:
        msg = f"'{KEY_MAP[device]}' requires keys 'manufacturer', 'model', 'type'"
        raise InvalidConfigurationError(msg) from None

    return CurrentSensor(manufacturer, model, _type)


def _parse_insulation_monitoring_device(config: dict) -> InsulationMonitoringDevice:
    device = config.get("insulation-monitoring-device")
    if not device or device == "none":
        # Insulation monitoring device is optional
        return InsulationMonitoringDevice("none")

    try:
        manufacturer = device["manufacturer"]
        model = device["model"]
        _type = device["type"]
    except KeyError:
        msg = (
            f"'{KEY_MAP['insulation-monitoring-device']}' requires keys "
            "'manufacturer', 'model', 'type'"
        )
        raise InvalidConfigurationError(msg) from None

    return InsulationMonitoringDevice(manufacturer, model, _type)


def _parse_rtos(config: dict) -> RTOS:
    rtos = config.get("rtos")
    if not rtos:
        err = f"Key '{KEY_MAP['rtos']}' is required."
        raise InvalidConfigurationError(err)

    try:
        rtos_name = rtos["name"]
    except KeyError:
        msg = f"'{KEY_MAP[rtos]}' requires key 'name'"
        raise InvalidConfigurationError(msg) from None

    try:
        rtos_addons = rtos["addons"]
    except KeyError:
        rtos_addons = []

    return RTOS(name=rtos_name, addons=rtos_addons)


def _parse_analog_front_end(config: dict) -> AnalogFrontEnd:
    afe = config.get("analog-front-end")
    if not afe:
        err = f"Key '{KEY_MAP['analog-front-end']}' is required."
        raise InvalidConfigurationError(err)

    try:
        manufacturer = afe["manufacturer"]
        ic = afe["ic"]
    except KeyError:
        msg = "'analog-front-end' requires keys 'manufacturer', 'model'"
        raise InvalidConfigurationError(msg) from None

    return AnalogFrontEnd(manufacturer=manufacturer, ic=ic)


def _parse_temperature_sensor(config: dict) -> TemperatureSensor:
    device = config.get("temperature-sensor")
    if not device:
        err = f"Key '{KEY_MAP['temperature-sensor']}' is required."
        raise InvalidConfigurationError(err)

    try:
        manufacturer = device["manufacturer"]
        model = device["model"]
        method = device["method"]
    except KeyError:
        msg = "'analog-front-end' requires keys 'manufacturer', 'model', 'method"
        raise InvalidConfigurationError(msg) from None
    return TemperatureSensor(manufacturer=manufacturer, model=model, method=method)


def _parse_debug_interfaces(config: dict) -> list[str]:
    # debug is entirely optional
    debug = config.get("debug")
    if not debug:
        return []

    return debug.get("interfaces", [])


def parse_config_to_system(config: dict) -> System:
    """Parse a json configuration to a object-oriented, validated system configuration"""
    application = _parse_application(config)
    rtos = _parse_rtos(config)
    bms_slave = _parse_bms_slave(config)
    debug = _parse_debug(config)
    return System(
        application=application,
        rtos=rtos,
        bms_slave=bms_slave,
        debug=debug,
    )


def _parse_bms_slave(config: dict) -> BMSSlave:
    bms_slave = config.get("bms-slave")
    if not bms_slave:
        err = f"Key '{KEY_MAP['bms-slave']}' is required."
        raise InvalidConfigurationError(err)

    analog_front_end = _parse_analog_front_end(bms_slave)
    temperature_sensor = _parse_temperature_sensor(bms_slave)
    return BMSSlave(
        analog_front_end=analog_front_end,
        temperature_sensor=temperature_sensor,
    )


def _parse_debug(config: dict) -> Debug:
    interfaces = _parse_debug_interfaces(config)
    return Debug(
        interfaces=interfaces,
    )


def _parse_application(config: dict) -> Application:
    application = config.get("application")
    if not application:
        err_msg = f"Key '{KEY_MAP['application']}' is required"
        raise InvalidConfigurationError(err_msg)

    state_estimation = _parse_state_estimation(application)
    aerosol_sensor = _parse_aerosol_sensor(application)
    balancing_strategy = _parse_balancing_strategy(application)
    current_sensor = _parse_current_sensor(application)
    insulation_monitoring_device = _parse_insulation_monitoring_device(application)
    return Application(
        algorithm=Algorithm(state_estimation=state_estimation),
        aerosol_sensor=aerosol_sensor,
        balancing_strategy=balancing_strategy,
        current_sensor=current_sensor,
        insulation_monitoring_device=insulation_monitoring_device,
    )
