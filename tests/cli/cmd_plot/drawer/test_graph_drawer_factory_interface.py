#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Testing file 'cli/cmd_plot/drawer/graph_drawer_factory_interface.py'."""

import io
import sys
import unittest
from contextlib import redirect_stderr
from pathlib import Path

try:
    from cli.cmd_plot.drawer.graph_drawer_factory import GraphDrawerFactoryInterface
    from cli.cmd_plot.drawer.graph_types import GraphTypes
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.cmd_plot.drawer.graph_drawer_factory import GraphDrawerFactoryInterface
    from cli.cmd_plot.drawer.graph_types import GraphTypes


class TestGetGraphType(unittest.TestCase):
    """Class to test the get_graph_type method of the Executor class"""

    def test_graph_config_not_dict(self) -> None:
        """Tests the get_graph_type method with graph_config parameter not as dict"""
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            GraphDrawerFactoryInterface._get_graph_type("test")  # pylint: disable=protected-access
        self.assertEqual(cm.exception.code, 1)
        self.assertIn(
            "Plot configuration is not a list of dictionaries.", buf.getvalue()
        )

    def test_valid_graph_type(self) -> None:
        """Tests the get_graph_type method with a valid graph type"""
        graph_type = GraphDrawerFactoryInterface._get_graph_type({"type": "LINE"})  # pylint: disable=protected-access
        self.assertEqual(graph_type, GraphTypes["LINE"])

    def test_invalid_graph_type(self) -> None:
        """Tests the get_graph_type method with a invalid graph type"""
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            GraphDrawerFactoryInterface._get_graph_type({"type": "test"})  # pylint: disable=protected-access
        self.assertEqual(cm.exception.code, 1)
        self.assertIn("is not valid", buf.getvalue())

    def test_missing_type(self) -> None:
        """Tests the get_graph_type method with a invalid graph type"""
        buf = io.StringIO()
        with redirect_stderr(buf), self.assertRaises(SystemExit) as cm:
            GraphDrawerFactoryInterface._get_graph_type({"test": "test"})  # pylint: disable=protected-access
        self.assertEqual(cm.exception.code, 1)
        self.assertIn("does not contain the manditory key 'type", buf.getvalue())


if __name__ == "__main__":
    unittest.main()
