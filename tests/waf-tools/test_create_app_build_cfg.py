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

"""Testing file 'tools/waf-tools/create_app_build_cfg.py'."""

import sys
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import MagicMock, patch

ROOT = Path(__file__).parents[2]
TEMPERATURE_SENSOR_CSV = ROOT / "docs/software/modules/driver/ts/ts-short-names.csv"
FIXTURES = ROOT / "tests/waf-tools/fixtures/create_app_build_cfg"
FIXED_DATE = "2026-03-23"

for i in [
    ROOT / "tools/waf-tools",
    ROOT / "tools/waf3-2.1.6-6a38d8c49406d2fef32d6f6600c8f033",  # Windows
    ROOT / "tools/waf.3-2.1.6-6a38d8c49406d2fef32d6f6600c8f033",  # Linux
]:
    if i.exists():
        sys.path.insert(0, str(i))

# pylint: disable=wrong-import-position

from create_app_build_cfg import (  # noqa:E402
    create_app_build_cfg_c,
    get_afe,
    get_imd,
    get_temperature_sensor,
)

# pylint: enable


class _FakeNode:  # pylint: disable=too-few-public-methods
    """Minimal node wrapper exposing abspath() like waf nodes."""

    def __init__(self, path: Path):
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


class LookupTests(unittest.TestCase):
    """Tests for configuration lookup helpers."""

    def test_get_afe_returns_expected_token(self):
        """Resolve known AFE manufacturer/IC pair."""
        ctx = MagicMock()
        ctx.env = SimpleNamespace(
            FOXBMS_BMS_SLAVE_AFE_MANUFACTURER="debug",
            FOXBMS_BMS_SLAVE_AFE_IC="default",
        )
        self.assertEqual(get_afe(ctx), "DEBUG_DEFAULT")

    def test_get_afe_calls_fatal_on_unknown(self):
        """Call ctx.fatal for unknown AFE combinations."""
        ctx = MagicMock()
        ctx.env = SimpleNamespace(
            FOXBMS_BMS_SLAVE_AFE_MANUFACTURER="unknown",
            FOXBMS_BMS_SLAVE_AFE_IC="foo",
        )
        result = get_afe(ctx)
        self.assertEqual(result, "")
        ctx.fatal.assert_called_once()

    def test_get_temperature_sensor_reads_csv(self):
        """Resolve sensor short-name from CSV content."""
        ctx = _make_ctx()

        self.assertEqual(get_temperature_sensor(ctx), "FAK00")

    def test_get_temperature_sensor_calls_fatal_when_csv_missing(self):
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

    def test_get_temperature_sensor_calls_fatal_on_unknown_sensor(self):
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

    def test_get_imd_returns_only_manufacturer_when_model_none(self):
        """Return manufacturer only when IMD model is configured as none."""
        ctx = MagicMock()
        ctx.env = SimpleNamespace(
            FOXBMS_IMD_MANUFACTURER="none",
            FOXBMS_IMD_MODEL="none",
        )

        self.assertEqual(get_imd(ctx), "NONE")

    def test_get_imd_returns_manufacturer_and_model(self):
        """Return MANUFACTURER_MODEL when IMD model is explicitly configured."""
        ctx = MagicMock()
        ctx.env = SimpleNamespace(
            FOXBMS_IMD_MANUFACTURER="iso",
            FOXBMS_IMD_MODEL="i123",
        )

        self.assertEqual(get_imd(ctx), "ISO_I123")


class CreateAppBuildCfgCTests(unittest.TestCase):
    """Fixture-based tests for create_app_build_cfg_c."""

    @patch("create_app_build_cfg.datetime")
    def test_render_matches_expected_fixture(self, mock_datetime: MagicMock):
        """Render using real c template and compare against committed golden output."""
        mock_datetime.now.return_value.date.return_value.strftime.return_value = (
            FIXED_DATE
        )

        ctx = _make_ctx()
        template = (ROOT / "conf/tpl/c.c").read_text(encoding="utf-8")
        expected = (FIXTURES / "expected-app_build_cfg.c").read_text(encoding="utf-8")
        result = create_app_build_cfg_c(ctx, template)
        self.assertMultiLineEqual(expected, result)


if __name__ == "__main__":
    unittest.main()
