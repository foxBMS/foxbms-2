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

"""Testing file 'tools/waf_tools/app_build_config_generate.py'."""

import hashlib
import sys
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import MagicMock, patch

ROOT = Path(__file__).parents[2]
TEMPERATURE_SENSOR_CSV = ROOT / "docs/software/modules/driver/ts/ts-short-names.csv"
FIXTURES = ROOT / "tests/waf_tools/fixtures/app_build_config_generate"
FIXED_DATE = "2026-03-23"

for i in [
    ROOT / "tools/waf_tools",
    ROOT / "tools/waf3-2.1.9-beba77c244731800bf15a003232e7040",  # Windows
    ROOT / "tools/.waf3-2.1.9-beba77c244731800bf15a003232e7040",  # Linux
]:
    if i.exists():
        sys.path.insert(0, str(i))

# pylint: disable=wrong-import-position

from app_build_config_generate import (  # noqa: E402
    _build_configuration_entry,
    _to_snake_case,
    _to_upper_snake_case,
    app_build_config_generate,
    app_build_config_generate_c,
    app_build_config_generate_source,
    get_afe,
    get_build_configuration,
    get_imd,
    get_temperature_sensor,
)
from waflib.Task import RUN_ME, SKIP_ME  # noqa: E402

# pylint: enable


class _FakeNode:  # pylint: disable=too-few-public-methods
    """Minimal node wrapper exposing abspath() like waf nodes."""

    def __init__(self, path: Path) -> None:
        self._path = path

    def abspath(self) -> str:
        """Return absolute path to the underlying file."""
        return str(self._path)


def _make_ctx() -> MagicMock:
    """Build a minimal ctx mock with all env values required by tested functions."""
    ctx = MagicMock()
    ctx.env = SimpleNamespace(
        VERSION="1.2.3",
        FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOC="counting",
        FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOE="counting",
        FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOF="trapezoid",
        FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOH="none",
        FOXBMS_BALANCING_STRATEGY="none",
        FOXBMS_RTOS_NAME="freertos",
        FOXBMS_IMD_MANUFACTURER="none",
        FOXBMS_IMD_MODEL="none",
        FOXBMS_BMS_SLAVE_AFE_MANUFACTURER="debug",
        FOXBMS_BMS_SLAVE_AFE_IC="can",
        FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_MANUFACTURER="fake",
        FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_MODEL="none",
        FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_METHOD="lookup-table",
    )
    ctx.srcnode.find_node.return_value = _FakeNode(TEMPERATURE_SENSOR_CSV)
    return ctx


def _make_task() -> app_build_config_generate_source:
    """Build a app_build_config_generate_source task with minimal mocked waf context."""
    task = app_build_config_generate_source.__new__(app_build_config_generate_source)
    task.generator = SimpleNamespace(bld=MagicMock())
    task.outputs = [MagicMock()]
    task.inputs = [MagicMock()]
    return task


class LookupTests(unittest.TestCase):
    """Tests for configuration lookup helpers."""

    def test_get_afe_returns_expected_token(self) -> None:
        """Resolve known AFE manufacturer/IC pair."""
        ctx = MagicMock()
        ctx.env = SimpleNamespace(
            FOXBMS_BMS_SLAVE_AFE_MANUFACTURER="debug",
            FOXBMS_BMS_SLAVE_AFE_IC="default",
        )
        self.assertEqual(get_afe(ctx), "DEBUG_DEFAULT")

    def test_get_afe_calls_fatal_on_unknown(self) -> None:
        """Call ctx.fatal for unknown AFE combinations."""
        ctx = MagicMock()
        ctx.env = SimpleNamespace(
            FOXBMS_BMS_SLAVE_AFE_MANUFACTURER="unknown",
            FOXBMS_BMS_SLAVE_AFE_IC="foo",
        )
        result = get_afe(ctx)
        self.assertEqual(result, "")
        ctx.fatal.assert_called_once()

    def test_get_temperature_sensor_reads_csv(self) -> None:
        """Resolve sensor short-name from CSV content."""
        ctx = _make_ctx()

        self.assertEqual(get_temperature_sensor(ctx), "FAK00")

    def test_get_temperature_sensor_calls_fatal_when_csv_missing(self) -> None:
        """Call ctx.fatal when CSV definition file cannot be found."""
        ctx = MagicMock()
        ctx.env = SimpleNamespace(
            FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_MANUFACTURER="fake",
            FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_MODEL="none",
        )
        ctx.srcnode.find_node.return_value = None
        ctx.fatal.side_effect = RuntimeError

        with self.assertRaises(RuntimeError):
            get_temperature_sensor(ctx)
        tmp = TEMPERATURE_SENSOR_CSV.relative_to(ROOT).as_posix()
        err_msg = f"Temperature sensor short name CSV file not found: {tmp}"
        ctx.fatal.assert_called_once_with(err_msg)

    def test_get_temperature_sensor_calls_fatal_on_unknown_sensor(self) -> None:
        """Call ctx.fatal when configured sensor cannot be found in CSV."""
        ctx = _make_ctx()
        ctx.env = SimpleNamespace(
            FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_MANUFACTURER="unknown",
            FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_MODEL="does-not-exist",
        )
        result = get_temperature_sensor(ctx)
        err_msg = (
            "Could not find a matching temperature sensor short name for the "
            "configured sensor."
        )
        ctx.fatal.assert_called_once_with(err_msg)
        self.assertEqual(result, "")


class ImdTests(unittest.TestCase):
    """Tests for IMD identifier generation."""

    def test_get_imd_returns_only_manufacturer_when_model_none(self) -> None:
        """Return manufacturer only when IMD model is configured as none."""
        ctx = MagicMock()
        ctx.env = SimpleNamespace(
            FOXBMS_IMD_MANUFACTURER="none",
            FOXBMS_IMD_MODEL="none",
        )

        self.assertEqual(get_imd(ctx), "NONE")

    def test_get_imd_returns_manufacturer_and_model(self) -> None:
        """Return MANUFACTURER_MODEL when IMD model is explicitly configured."""
        ctx = MagicMock()
        ctx.env = SimpleNamespace(
            FOXBMS_IMD_MANUFACTURER="iso",
            FOXBMS_IMD_MODEL="i123",
        )

        self.assertEqual(get_imd(ctx), "ISO_I123")


class HelperTests(unittest.TestCase):
    """Tests for helper formatting functions."""

    def test_to_upper_snake_case(self) -> None:
        """Convert camelCase field names to enum-style tokens."""
        _in, _out = "socAlgorithm", "SOC_ALGORITHM"
        self.assertEqual(_to_upper_snake_case(_in), _out)

    def test_to_snake_case(self) -> None:
        """Convert camelCase identifiers to snake_case."""
        _in, _out = "temperatureSensorMethod", "temperature_sensor_method"
        self.assertEqual(_to_snake_case(_in), _out)

    def test_build_configuration_entry(self) -> None:
        """Build a single C struct assignment line."""
        __in = _build_configuration_entry("socAlgorithm", "COUNTING")
        _in, _out = (__in, "    .socAlgorithm = SOC_ALGORITHM_COUNTING,")
        self.assertEqual(_in, _out)


class BuildConfigurationTests(unittest.TestCase):
    """Tests for assembling BuildConfiguration values."""

    @patch("app_build_config_generate.get_temperature_sensor", return_value="FAK00")
    @patch("app_build_config_generate.get_afe", return_value="DEBUG_CAN")
    @patch("app_build_config_generate.get_imd", return_value="NONE")
    def test_get_build_configuration_populates_expected_values(
        self, *_: MagicMock
    ) -> None:
        """Uppercase environment values and normalize sensor method token."""
        ctx = MagicMock()
        ctx.env = SimpleNamespace(
            FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOC="counting",
            FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOE="counting",
            FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOF="trapezoid",
            FOXBMS_ALGORITHM_STATE_ESTIMATOR_SOH="none",
            FOXBMS_BALANCING_STRATEGY="none",
            FOXBMS_RTOS_NAME="freertos",
            FOXBMS_BMS_SLAVE_TEMPERATURE_SENSOR_METHOD="lookup-table",
        )

        cfg = get_build_configuration(ctx)

        self.assertEqual(cfg.soc_algorithm, "COUNTING")
        self.assertEqual(cfg.soe_algorithm, "COUNTING")
        self.assertEqual(cfg.sof_algorithm, "TRAPEZOID")
        self.assertEqual(cfg.soh_algorithm, "NONE")
        self.assertEqual(cfg.imd, "NONE")
        self.assertEqual(cfg.balancing_strategy, "NONE")
        self.assertEqual(cfg.rtos, "FREERTOS")
        self.assertEqual(cfg.afe, "DEBUG_CAN")
        self.assertEqual(cfg.temperature_sensor, "FAK00")
        self.assertEqual(cfg.temperature_sensor_method, "LOOKUP_TABLE")


class CreateAppBuildCfgSourceCreateConfigHash(unittest.TestCase):
    """Tests for app_build_config_generate_source Task class methods."""

    @patch("app_build_config_generate.get_build_configuration")
    def test_create_config_hash_hashes_build_configuration(
        self, mock: MagicMock
    ) -> None:
        """Create deterministic SHA256 digest from build configuration repr."""
        mock.return_value = "BUILD_CFG"
        expected_hash = hashlib.sha256(repr("BUILD_CFG").encode("utf-8")).hexdigest()
        task = _make_task()
        result = task.create_config_hash()
        self.assertEqual(result, expected_hash)
        mock.assert_called_once_with(task.generator.bld)


class CreateAppBuildCfgSourceRunnableStatus(unittest.TestCase):
    """Tests for app_build_config_generate_source Task class methods."""

    @patch("app_build_config_generate.Task.runnable_status", return_value=SKIP_ME)
    def test_runnable_status_forwards_non_run_me(self, _: MagicMock) -> None:
        """Return super() state unchanged when task is not ready to run."""
        task = _make_task()
        task.create_config_hash = MagicMock()

        self.assertEqual(task.runnable_status(), SKIP_ME)
        task.create_config_hash.assert_not_called()

    @patch("app_build_config_generate.Task.runnable_status", return_value=RUN_ME)
    def test_runnable_status_returns_skip_when_hash_unchanged(
        self, _: MagicMock
    ) -> None:
        """Skip execution when stored hash matches current configuration hash."""
        task = _make_task()
        task.create_config_hash = MagicMock(return_value="same-hash")
        task.outputs[
            0
        ].parent.find_or_declare.return_value.read.return_value = "same-hash"

        self.assertEqual(task.runnable_status(), SKIP_ME)
        task.outputs[0].parent.make_node.assert_not_called()

    @patch("app_build_config_generate.Task.runnable_status", return_value=RUN_ME)
    def test_runnable_status_writes_hash_and_runs_when_hash_differs(
        self, _: MagicMock
    ) -> None:
        """Write hash marker and run task when configuration hash changed."""
        task = _make_task()
        task.create_config_hash = MagicMock(return_value="new-hash")
        task.outputs[
            0
        ].parent.find_or_declare.return_value.read.side_effect = FileNotFoundError()

        self.assertEqual(task.runnable_status(), RUN_ME)
        task.outputs[0].parent.make_node.assert_called_once_with("app_build_cfg.hash")
        task.outputs[0].parent.make_node.return_value.write.assert_called_once_with(
            "new-hash", encoding="utf-8"
        )


class CreateAppBuildCfg(unittest.TestCase):
    """Tests for app_build_config_generate task-generator"""

    def _make_taskgen(self) -> SimpleNamespace:
        src_node = object()
        app_build_cfg_node = object()
        return SimpleNamespace(
            version=True,
            app_build_cfg=True,
            env=SimpleNamespace(PROJECT_ROOT=["."]),
            path=SimpleNamespace(
                ctx=SimpleNamespace(
                    root=SimpleNamespace(find_node=MagicMock(return_value=src_node))
                ),
                find_or_declare=MagicMock(return_value=app_build_cfg_node),
            ),
            create_task=MagicMock(
                return_value=SimpleNamespace(outputs=[app_build_cfg_node])
            ),
            source=["main.c"],
        )

    def test_app_build_config_generate_returns_early_without_version(self) -> None:
        """Do nothing when task generator has no version attribute enabled."""
        taskgen = self._make_taskgen()
        taskgen.version = False

        app_build_config_generate(taskgen)

        taskgen.create_task.assert_not_called()
        self.assertEqual(taskgen.source, ["main.c"])

    def test_app_build_config_generate_adds_generated_source(self) -> None:
        """Create task from fixed template source and fixed app_build_cfg.c target."""
        taskgen = self._make_taskgen()

        app_build_config_generate(taskgen)

        taskgen.path.ctx.root.find_node.assert_called_once_with("./conf/tpl/c.c")
        taskgen.path.find_or_declare.assert_called_once_with("app_build_cfg.c")
        taskgen.create_task.assert_called_once_with(
            "app_build_config_generate_source",
            src=taskgen.path.ctx.root.find_node.return_value,
            tgt=[taskgen.path.find_or_declare.return_value],
        )
        self.assertEqual(
            taskgen.source, ["main.c", taskgen.path.find_or_declare.return_value]
        )

    def test_app_build_config_generate_skips_task_when_disabled(self) -> None:
        """Do not create generation task when app_build_cfg flag is disabled."""
        taskgen = self._make_taskgen()
        taskgen.app_build_cfg = False

        app_build_config_generate(taskgen)

        taskgen.create_task.assert_not_called()
        self.assertEqual(taskgen.source, ["main.c"])

    def test_app_build_config_generate_handles_non_list_source(self) -> None:
        """Fallback path: convert scalar source to list before appending generated file."""
        taskgen = self._make_taskgen()
        taskgen.source = "main.c"
        app_build_config_generate(taskgen)
        self.assertEqual(
            taskgen.source, ["main.c", taskgen.path.find_or_declare.return_value]
        )


class CreateAppBuildCfgCTests(unittest.TestCase):
    """Fixture-based tests for app_build_config_generate_c."""

    @patch("app_build_config_generate.datetime")
    def test_render_matches_expected_fixture(self, mock_datetime: MagicMock) -> None:
        """Render using real c template and compare against committed golden output."""
        mock_datetime.now.return_value.date.return_value.strftime.return_value = (
            FIXED_DATE
        )

        ctx = _make_ctx()
        template = (ROOT / "conf/tpl/c.c").read_text(encoding="utf-8")
        expected = (FIXTURES / "expected-app_build_cfg.c").read_text(encoding="utf-8")
        result = app_build_config_generate_c(ctx, template)
        self.assertMultiLineEqual(expected, result)


if __name__ == "__main__":
    unittest.main()
