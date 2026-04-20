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

"""Helper functions to create model objects and load JSON files.

This module provides utilities to:
- construct model parameter objects from JSON dictionary definitions,
- read JSON either from a filesystem path or a ZIP virtual path.
"""

import sys
import zipfile
from collections.abc import Sequence
from json import load
from pathlib import Path

from ..helpers.click_helpers import recho
from .model_parameter import BaseModel
from .model_parameter.limits import CurrentLimitModel


def creates_models(
    models_dicts: dict, directory: zipfile.Path | Path
) -> Sequence[BaseModel]:
    """Create model parameter objects from JSON dictionaries.

    This function iterates over the provided model definitions and
    instantiates the corresponding model classes. Currently supported:
    - "current limits" -> `CurrentLimitModel`.

    Args:
        models_dicts: Iterable of model definition dictionaries (typically a
            list of dicts). Each dict must contain at least the key ``"name"``.
        directory: Path to the cell directory; passed through to models for optional
            file access.

    Returns:
        Sequence[BaseModel]
            Constructed model parameter objects.

    Raises:
        ValueError
            If a model ``name`` is unknown.
    """
    model_parameters = []
    for model_dict in models_dicts:
        model_dict["_directory"] = directory
        model_name = model_dict["name"]
        match model_name:
            case "current limits":
                model_parameters.append(CurrentLimitModel(**model_dict))
            case _:
                err_txt = f"Model with name '{model_name}' is not known."
                raise ValueError(err_txt)
    return model_parameters


def read_json(json_file: zipfile.Path | Path, directory: zipfile.Path | Path) -> dict:
    """Read a JSON file from a filesystem path or ZIP virtual path.

    Args:
        json_file: Path to the JSON file inside a directory or ZIP archive.
        directory: The parent directory, used only for reporting in error messages.

    Returns:
        Parsed JSON content as dictionary.

    Raises:
        SystemExit: If the JSON file does not exist.
        JSONDecodeError: Propagated if the file content is not valid JSON.
    """
    if json_file.exists():
        if isinstance(json_file, zipfile.Path):
            with json_file.open(encoding="utf-8") as f:
                return load(f)
        else:
            with open(json_file, encoding="utf-8") as f:
                return load(f)
    else:
        recho(f"File '{json_file.name} in {directory} is missing.")
        sys.exit(1)
