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

"""Testing file 'cli/cmd_etl/cmds/__init__.py'."""

import io
import sys
import unittest
from contextlib import redirect_stdout
from pathlib import Path
from unittest.mock import MagicMock, Mock, patch

# TODO: Why is this pylint suppression needed?!
try:
    # pylint: disable-next=import-self
    from cli.cmd_etl.cmds import (
        DEFAULT_CONFIG_FILE_FILTER,
        get_config_file_options,
        read_config,
    )
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    # pylint: disable-next=import-self
    from cli.cmd_etl.cmds import (
        DEFAULT_CONFIG_FILE_FILTER,
        get_config_file_options,
        read_config,
    )


class TestReadConfig(unittest.TestCase):
    """Test read_config method"""

    @patch("yaml.safe_load")
    @patch("builtins.open")
    def test_read_config(self, open_mock: Mock, safe_load_mock: Mock) -> None:
        """Test for the read_config function which reads the
        configurations yaml files

        :param open_mock: A mock for the file opening
        :param safe_load_mock: A mock for the yaml parser
        """
        test_path = Path("test")
        read_config(test_path)
        open_mock.assert_called_once_with(test_path, encoding="utf-8")
        safe_load_mock.assert_called_once()


@patch("cli.cmd_etl.cmds.read_config")
class TestGetConfigFileOptions(unittest.TestCase):
    """Test the configuration parser"""

    @patch("cli.cmd_etl.cmds.Path", return_value=Path("option_test"))
    def test_get_config_file_options_no_file(
        self, mock_path: Mock, _: MagicMock
    ) -> None:
        """Test when the configuration file does not exist."""
        mock_path.cwd.return_value = Path()
        ctx = MagicMock()
        config_file = MagicMock()
        config_file.is_file.return_value = False
        config_file.parent = Path()
        get_config_file_options(ctx, MagicMock(), config_file)
        self.assertDictEqual(ctx.default_map, {})

    @patch("cli.cmd_etl.cmds.Path", return_value=Path("option_test"))
    def test_get_config_file_options_file_exists_no_sampling(
        self, mock_path: Mock, m_read_config: MagicMock
    ) -> None:
        """Test when the configuration file exists."""
        mock_path.cwd.return_value = Path()
        ctx = MagicMock()
        config_file = MagicMock()
        config_file.is_file.return_value = True
        config_file.parent = Path()
        m_read_config.return_value = {"foo": "bar"}

        get_config_file_options(ctx, MagicMock(), config_file)
        self.assertDictEqual(ctx.default_map, {"foo": "bar"})

    @patch("cli.cmd_etl.cmds.Path", return_value=Path("option_test"))
    def test_get_config_file_options_file_exists_sampling(
        self,
        mock_path: Mock,
        m_read_config: MagicMock,
    ) -> None:
        """Test when the configuration file exists and sampling is provided."""
        mock_path.cwd.return_value = Path()
        ctx = MagicMock()
        config_file = MagicMock()
        config_file.is_file.return_value = True
        config_file.parent = Path()
        m_read_config.return_value = {"foo": "bar", "sampling": {"a": "b"}}

        get_config_file_options(ctx, MagicMock(), config_file)
        self.assertDictEqual(
            ctx.default_map, {"foo": "bar", "sampling": {"a": "b"}.items()}
        )

    @patch("cli.cmd_etl.cmds.Path.is_file")
    def test_get_config_file_options_default_file(
        self, m_is_file: MagicMock, m_read_config: MagicMock
    ) -> None:
        """Test when the configuration file exists."""
        ctx = MagicMock()
        config_file = DEFAULT_CONFIG_FILE_FILTER
        m_is_file.return_value = True
        m_read_config.return_value = {"foo": "bar"}
        buf = io.StringIO()
        with redirect_stdout(buf):
            get_config_file_options(ctx, MagicMock(), config_file)
        self.assertIn("Default configuration file filter.yml is used.", buf.getvalue())

    @patch("cli.cmd_etl.cmds.Path.is_file")
    @patch("cli.cmd_etl.cmds.Path.cwd")
    def test_get_config_file_options_dbc_not_found(
        self, m_cwd: MagicMock, m_is_file: MagicMock, m_read_config: MagicMock
    ) -> None:
        """Test when the dbc file cant be found"""
        ctx = MagicMock()

        m_cwd.return_value = Path("/foo")
        m_is_file.return_value = False
        m_read_config.return_value = {"dbc": "test.dbc"}
        config_file = MagicMock()
        config_file.parent = Path()
        get_config_file_options(ctx, MagicMock(), config_file)
        self.assertEqual(ctx.default_map, {"dbc": "test.dbc"})

    @patch("cli.cmd_etl.cmds.Path.is_file")
    def test_get_config_file_options_dbc_in_cfg_path(
        self, m_is_file: MagicMock, m_read_config: MagicMock
    ) -> None:
        """Test when dbc file is found in path of config"""
        ctx = MagicMock()
        config_file = MagicMock()

        m_read_config.return_value = {"dbc": "bar"}
        m_is_file.side_effect = [False, False, True]
        config_file.is_file.return_value = True
        config_file.parent = Path("foo")

        buf = io.StringIO()
        with redirect_stdout(buf):
            get_config_file_options(ctx, MagicMock(), config_file)

        self.assertEqual("", buf.getvalue())
        self.assertDictEqual(ctx.default_map, {"dbc": Path("foo/bar")})

    @patch("cli.cmd_etl.cmds.Path.is_file")
    @patch("cli.cmd_etl.cmds.Path.cwd")
    def test_get_config_file_options_in_cwd(
        self, m_cwd: MagicMock, m_is_file: MagicMock, m_read_config: MagicMock
    ) -> None:
        """Test when dbc file is found in path of cwd"""
        ctx = MagicMock()
        config_file = MagicMock()

        m_read_config.return_value = {"dbc": "bar"}
        m_is_file.side_effect = [False, True]
        m_cwd.return_value = Path("foo")
        config_file.is_file.return_value = True

        buf = io.StringIO()
        with redirect_stdout(buf):
            get_config_file_options(ctx, MagicMock(), config_file)

        self.assertEqual("", buf.getvalue())
        self.assertDictEqual(ctx.default_map, {"dbc": Path("foo/bar")})


if __name__ == "__main__":
    unittest.main()
