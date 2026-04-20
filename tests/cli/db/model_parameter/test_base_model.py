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

"""Testing file 'cli/db/model_parameter/base_model.py'."""

# cspell:ignore zpath

import io
import sys
import tempfile
import unittest
import zipfile
from pathlib import Path

try:
    from cli.db.model_parameter import BaseModel
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[4]))
    from cli.db.model_parameter import BaseModel


class TestBaseModelPostInit(unittest.TestCase):
    """Tests for BaseModel.__post_init__ normalization and validation."""

    def test_valid_filesystem_sources_string_and_path(self) -> None:
        """Should resolve string sources against _directory and keep Path entries unchanged."""
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            a = directory / "a.txt"
            b = directory / "b.txt"
            a.write_text("A", encoding="utf-8")
            b.write_text("B", encoding="utf-8")

            # Mix of string and Path sources
            sources = ["a.txt", b]
            model = BaseModel(name="ok", sources=sources, _directory=directory)

            # Strings are resolved against _directory; Paths are preserved
            self.assertEqual(len(model.sources), 2)
            self.assertIsInstance(model.sources[0], Path)
            self.assertEqual(model.sources[0], a)
            self.assertIs(model.sources[1], b)  # same object preserved
            self.assertTrue(model.sources[0].exists())
            self.assertTrue(model.sources[1].exists())

    def test_valid_zipfile_resolution_from_string(self) -> None:
        """Should resolve string sources against a zipfile.Path _directory and verify existence."""
        with tempfile.TemporaryDirectory() as tmp:
            zpath = Path(tmp) / "db.zip"
            # Create zip with a file at root/data/a.txt
            with zipfile.ZipFile(zpath, "w") as zf:
                zf.writestr("root/data/a.txt", io.BytesIO(b"hello").getvalue())

            with zipfile.ZipFile(zpath) as zf:
                root = zipfile.Path(zf) / "root" / "data"
                # Provide a string source that must be joined to root
                model = BaseModel(name="in-zip", sources=["a.txt"], _directory=root)

                self.assertEqual(len(model.sources), 1)
                src = model.sources[0]
                # zipfile.Path is expected
                self.assertIsInstance(src, zipfile.Path)
                self.assertTrue(src.exists())
                self.assertEqual(src.name, "a.txt")

    def test_invalid_name_type_raises_typeerror(self) -> None:
        """Should raise TypeError if name is not a string."""
        with self.assertRaises(TypeError) as ctx:
            BaseModel(name=123, sources=[], _directory=Path.cwd())
        self.assertIn("not a string", str(ctx.exception))

    def test_missing_string_source_raises_valueerror(self) -> None:
        """Should raise ValueError if a string source resolved against _directory does not exist."""
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            with self.assertRaises(ValueError) as ctx:
                BaseModel(name="x", sources=["missing.txt"], _directory=directory)
            self.assertIn("At least one source", str(ctx.exception))

    def test_missing_path_source_raises_valueerror(self) -> None:
        """Should raise ValueError if a Path source does not exist."""
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            missing = directory / "nope.txt"
            with self.assertRaises(ValueError) as ctx:
                BaseModel(name="x", sources=[missing], _directory=directory)
            self.assertIn("At least one source", str(ctx.exception))

    def test_non_string_sources_are_preserved_by_identity(self) -> None:
        """Should keep non-str sources (Path/zipfile.Path) unchanged by identity."""
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            p = directory / "p.txt"
            p.write_text("ok", encoding="utf-8")

            # Filesystem Path identity preserved
            model_fs = BaseModel(name="fs", sources=[p], _directory=directory)
            self.assertIs(model_fs.sources[0], p)

            # zipfile.Path identity preserved
            zpath = Path(tmp) / "db.zip"
            with zipfile.ZipFile(zpath, "w") as zf:
                zf.writestr("root/q.txt", b"z")
            with zipfile.ZipFile(zpath) as zf:
                q = zipfile.Path(zf) / "root" / "q.txt"
                model_zip = BaseModel(name="zip", sources=[q], _directory=q.parent)
                self.assertIs(model_zip.sources[0], q)

    def test_sources_list_is_replaced_with_normalized_entries(self) -> None:
        """Should replace sources with the normalized list (e.g., strings -> Paths)."""
        with tempfile.TemporaryDirectory() as tmp:
            directory = Path(tmp)
            (directory / "a.txt").write_text("A", encoding="utf-8")
            m = BaseModel(name="normalize", sources=["a.txt"], _directory=directory)
            self.assertEqual(m.sources, [directory / "a.txt"])


if __name__ == "__main__":
    unittest.main()
