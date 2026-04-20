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

"""Base definitions for model parameter dataclasses.

This package module provides the ``BaseModel`` dataclass from which all
model-parameter classes derive. It normalizes relative source paths and
validates inputs both for regular filesystem directories and ZIP archives.

The typical workflow is:

- Construct a concrete model dataclass with ``name``, ``sources`` and the
  directory context (``_directory``).
- Let ``__post_init__()`` convert string entries in ``sources`` to paths
  relative to ``_directory`` and verify that all sources exist.
"""

import zipfile
from dataclasses import dataclass
from pathlib import Path


@dataclass
class BaseModel:
    """Base class for model parameter dataclasses.

    This dataclass provides common fields and initialization checks that are
    shared by all model parameter implementations (e.g., current limit models).

    Attributes:
        name: Human-readable model name. Must be a string.
        sources: List of data or reference sources. Elements can be:
            - absolute or relative filesystem paths (``pathlib.Path``),
            - paths inside a ZIP archive (``zipfile.Path``),
            - strings (treated as relative paths and resolved against ``_directory``).
        _directory: Root directory used to resolve relative ``sources`` when
            given as strings. Can be a real filesystem path or a ZIP virtual
            path.
    """

    name: str
    sources: list[zipfile.Path | Path | str]
    _directory: zipfile.Path | Path

    def __post_init__(self) -> None:
        """Validate attributes and normalize source paths.

        This method performs the following steps:
        - Ensures ``name`` is a string.
        - Iterates over ``sources`` and converts string entries to paths by
          joining them with ``_directory``.
        - Leaves existing ``Path`` or ``zipfile.Path`` entries unchanged.
        - Verifies that each resulting path exists.
        - Stores the normalized list back into ``self.sources``.

        Raises:
            TypeError
                If ``name`` is not a string.
            ValueError
                If at least one entry in ``sources`` does not resolve to an
                existing path.
        """
        if not isinstance(self.name, str):
            err_txt = f"Modelname '{self.name}' is not a string."
            raise TypeError(err_txt)
        new_sources = []
        for source in self.sources:
            if isinstance(source, str):
                new_source = self._directory.joinpath(source)
                new_sources.append(new_source)
            else:
                new_source = source
                new_sources.append(new_source)
            if not new_source.exists():
                err_txt = (
                    f"At least one source in '{self.sources}' is not a valid "
                    "path or does not exist."
                )
                raise ValueError(err_txt)
        self.sources = new_sources  # type: ignore[assignment]
