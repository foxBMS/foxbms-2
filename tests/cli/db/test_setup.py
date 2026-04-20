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

"""Testing file 'cli/db/setup.py'."""

# cspell:ignore zpath

import io
import json
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path
from unittest.mock import patch

try:
    from cli.db import setup
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.db import setup


class TestCreatesModels(unittest.TestCase):
    """Tests for creates_models(). One TestCase per function."""

    def test_creates_current_limit_model(self) -> None:
        """Test CurrentLimitModel instantiation

        Test details: 'current limits' and pass directory via _directory.
        """
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            model_dicts = [{"name": "current limits", "foo": 123}]

            # Patch the CurrentLimitModel class in the module under test
            with patch("cli.db.setup.CurrentLimitModel") as mock_model:
                instance = object()
                mock_model.return_value = instance

                result = setup.creates_models(model_dicts, directory)

                # Verify: returns a sequence with the created instance
                self.assertEqual(result, [instance])

                # Verify: CurrentLimitModel called with our fields incl. injected _directory
                mock_model.assert_called_once()
                kwargs = mock_model.call_args.kwargs
                self.assertEqual(kwargs["name"], "current limits")
                self.assertEqual(kwargs["foo"], 123)
                self.assertEqual(kwargs["_directory"], directory)

                # Verify: original dict got mutated to include _directory
                self.assertIn("_directory", model_dicts[0])
                self.assertEqual(model_dicts[0]["_directory"], directory)

    def test_raises_on_unknown_model_name(self) -> None:
        """Should raise ValueError for an unknown model name."""
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            model_dicts = [{"name": "unknown-model"}]

            with self.assertRaises(ValueError) as ctx:
                setup.creates_models(model_dicts, directory)

            self.assertIn("is not known", str(ctx.exception))


class TestReadJson(unittest.TestCase):
    """Tests for read_json(). One TestCase per function."""

    def test_reads_from_filesystem_path(self) -> None:
        """Should read JSON from a regular filesystem Path."""
        with tempfile.TemporaryDirectory() as tmp:
            tmp_path = Path(tmp)
            data = {"a": 1, "b": [1, 2, 3]}
            json_file = tmp_path / "test.json"
            json_file.write_text(json.dumps(data), encoding="utf-8")

            out = setup.read_json(json_file, tmp_path)
            self.assertEqual(out, data)

    def test_reads_from_zipfile_path(self) -> None:
        """Should read JSON from a zipfile.Path."""
        with tempfile.TemporaryDirectory() as tmp:
            zpath = Path(tmp) / "db.zip"

            # Create a zip with a JSON file inside
            with zipfile.ZipFile(zpath, "w") as zf:
                # Put JSON into the archive
                payload = {"x": 42, "y": "ok"}
                buf = io.BytesIO(json.dumps(payload).encode("utf-8"))
                zf.writestr("root/test.json", buf.getvalue())

            # Re-open the archive and create zipfile.Path objects
            with zipfile.ZipFile(zpath) as zf:
                root = zipfile.Path(zf) / "root"
                jf = root / "test.json"

                out = setup.read_json(jf, root)
                self.assertEqual(out, {"x": 42, "y": "ok"})

    def test_missing_file_exits_and_logs(self) -> None:
        """Should call recho and sys.exit(1) when file is missing."""
        with tempfile.TemporaryDirectory() as tmp:
            tmp_path = Path(tmp)
            missing = tmp_path / "missing.json"

            with patch("cli.db.setup.recho") as mock_recho:
                with self.assertRaises(SystemExit) as ctx:
                    setup.read_json(missing, tmp_path)

                # SystemExit with code 1
                self.assertEqual(ctx.exception.code, 1)

                # Ensure recho was called with some message (exact text is not strictly asserted)
                mock_recho.assert_called()


if __name__ == "__main__":
    unittest.main()
