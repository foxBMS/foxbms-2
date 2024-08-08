#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Testing the 'axivion' command"""

import sys
import unittest
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).parent.parent.parent.parent))
# pylint: disable=wrong-import-position
from cli.cmd_axivion import axivion_impl
from cli.helpers.spr import SubprocessResult
# pylint: enable=wrong-import-position

# Tests possible -
# For export_arch
# 1. gravis not found
# 2. Check for verbosity, gravis only supports -v ? Should I do this test?
# 3. More possible test - default or alternate path found

# check_architecture


class TestArchitecture(unittest.TestCase):
    """Test architecture"""

    @patch("cli.cmd_axivion.axivion_impl.shutil.which", return_value=None)
    def test_gravis_not_found(self, mock_which):  # pylint:disable=unused-argument
        """Test for no gravis path found"""
        # Call the function under test
        result = axivion_impl.export_architecture(verbosity=0)
        # Check if the result is as expected
        self.assertEqual(result, SubprocessResult(1))

    @patch("cli.cmd_axivion.axivion_impl.shutil.which")
    @patch("cli.cmd_axivion.axivion_impl.run_process")
    def test_gravis_found_default_path_with_verbosity(
        self, mock_run_process, mock_which
    ):
        """Simple check"""
        mock_which.return_value = "/path/to/gravis"
        mock_run_process.return_value = SubprocessResult(0)

        result = axivion_impl.export_architecture(verbosity=1)

        # Check if shutil.which was called exactly once with argument 'gravis'
        mock_which.assert_called_once_with("gravis")
        mock_run_process.assert_called_once_with(
            cmd=[
                "/path/to/gravis",
                "--script",
                str(axivion_impl.GRAVIS_EXPORT_SCRIPT),
                "-v",
            ],
            cwd=axivion_impl.PROJECT_ROOT,
            stderr=None,
            stdout=None,
        )
        self.assertEqual(result, SubprocessResult(0))


if __name__ == "__main__":
    unittest.main()
