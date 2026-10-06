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

"""Code Generator for foxBMS configuration C/H source and CSV files.

This waf tool implements the build tasks and task generators that transform
JSON configuration files (e.g. ``battery_cell_cfg.json``,
``battery_system_cfg.json``, ``diag_array_cfg.json``) into generated C
source/header files and documentation CSV files. It uses template files
located in ``conf/bms/templates/`` and resolves define values, CSV lookup
tables, and diagnostic array initializers at build time.

The tool provides two waf features:
    - **codegen_config** - Generates C/H files from JSON configuration.
      For each file entry in the JSON, a :class:`CreateCfgFile` task is created
      that resolves typed values (bool, int, uint, float), CSV data, and
      diagnostic arrays, then renders them into the corresponding template.
      An optional ``validator`` callback is invoked to validate the JSON
      before code generation.
    - **codegen_csv** - Generates documentation CSV files summarizing the
      configuration defines for use in Sphinx documentation.

Usage
-----
Declare task generators in a waf build script::

    bld(
        features="codegen_config",
        json=bld.srcnode.find_node("conf/bms/battery_cell_cfg.json"),
        validator=battery_cell_config_validate.validate_battery_cell_configuration,
        target=f"{cp}battery_cell_cfg",
    )

    bld(
        features="codegen_csv",
        json=bld.srcnode.find_node(f"conf/bms/battery_cell_cfg.json"),
    )

Dependencies
------------
- :mod:`waf_tools.c_codegen_template` - Template rendering engine.
- :mod:`waf_tools.diag_array_config_validate` - Diagnostic array rendering.
- :mod:`waf_tools.vcs` - Version information for file headers.
"""

import csv
import math
from dataclasses import dataclass
from datetime import UTC, datetime
from pathlib import Path
from typing import Literal

from c_codegen_template import render_template
from diag_array_config_validate import render_diag_array
from vcs import VcsInformation
from waflib.Build import BuildContext
from waflib.Node import Node
from waflib.Task import Task
from waflib.TaskGen import after_method, feature, task_gen


class CreateCfgFile(Task):
    """Waf task that generates a C/H file from a JSON configuration.

    Reads the ``files`` entry addressed by ``env.IDX`` from the JSON
    configuration, resolves all define values (typed literals, CSV lookup
    tables, diagnostic arrays) and renders them into the corresponding
    template from ``conf/bms/templates``.

    A ``RuntimeError`` raised by the validator is not propagated: it is stored
    in ``err_msg`` and the task fails with return code 1, so that waf reports
    the configuration error instead of a Python traceback.

    Attributes:
        ext_out: Output extensions used by waf for task ordering.
        color: Console color used when displaying the task.
        err_msg: Error message reported by waf when validation failed.
    """

    ext_out = [".c", ".h"]
    color = "GREY"
    err_msg: str

    def run(self) -> int:  # noqa: D102
        # Run the validator on the JSON configuration only once to avoid
        # redundant validation when the same generator produces
        # multiple outputs (e.g. .h and .c)
        validator = getattr(self.generator, "validator", None)
        if validator and self.env.IDX == 0:
            try:
                validator(self.generator.bld, self.inputs[0])
            except RuntimeError as e:
                self.err_msg = (
                    f"Invalid configuration in {self.inputs[0].relpath()}:\n{e}"
                )
                return 1

        txt = create_config(self.generator.bld, self.inputs[0], self.env.IDX)
        self.outputs[0].write(txt, encoding="utf-8")
        return 0

    def keyword(self) -> Literal["Creating"]:  # noqa: D102
        return "Creating"

    def __str__(self) -> str:
        """Return the generated output path."""
        return str(self.outputs[0].path_from(self.generator.bld.path))


@feature("codegen_config")
@after_method("process_rule")
def create_cfg_task(self: task_gen) -> None:
    """Process the ``codegen_config`` feature on a task generator.

    Reads the JSON configuration node, and creates one :class:`CreateCfgFile`
    task per entry in its ``files`` list. The output files are declared
    relative to the configuration's ``path`` in the build directory.
    Referenced CSV files and the used template are registered as additional
    dependencies so that the task is re-run when they change.

    Required task generator attributes:
        - **json (Node)**: The JSON configuration source node (e.g.
          ``battery_cell_cfg.json``).

    Optional task generator attributes:
        - **validator (Callable)**: Callback invoked as
          ``validator(bld, json_node)`` to validate the configuration before
          code generation.
    """
    json_node = getattr(self, "json", None)
    if not json_node:
        return
    content = json_node.read_json()
    out_dir = content["path"]
    if not out_dir:
        return

    for idx, file_cfg in enumerate(content["files"]):
        file_name = file_cfg["file_name"]
        out = self.bld.bldnode.find_or_declare(f"{out_dir}/{file_name}")

        dep_nodes = []
        for define in file_cfg.get("defines", []):
            if _is_csv(define.get("value", "")):
                csv_node = json_node.parent.find_node(define["value"])
                if csv_node:
                    dep_nodes.append(csv_node)

        template_node = _get_template(self.bld, file_name)
        dep_nodes.append(template_node)

        task = self.create_task("CreateCfgFile", json_node, out)
        task.env.IDX = idx
        task.dep_nodes = dep_nodes


class CreateCsvFile(Task):
    """Waf task that generates a documentation CSV file from a JSON configuration.

    Depending on the input file, either the diagnosis entries
    (``diag_array_cfg.json``) or the configuration defines (battery cell and
    battery system configuration) are rendered into a semicolon-separated CSV
    file for inclusion in the Sphinx documentation.

    Attributes:
        file_cfg: The entry of the configuration's ``files`` list that is
            rendered by this task.
    """

    file_cfg: dict

    def run(self) -> None:  # noqa: D102
        if self.inputs[0].name == "diag_array_cfg.json":
            txt = self.render_diag_array_cfg_csv()
        else:
            txt = self.render_battery_cfg_csv()
        self.outputs[0].write(txt, encoding="utf-8")

    def render_diag_array_cfg_csv(self) -> str:
        """Render the CSV content for the diagnosis configuration.

        Creates one row per entry of ``array_entries``, mapping the diagnosis
        event to its description.

        Returns:
            The CSV content as a string, including the header row.
        """
        lines = []
        lines.append("Diagnosis Entry; Description")
        lines.extend(
            f"``{e['event']}``; {e['description']}"
            for e in self.file_cfg["array_entries"]
        )
        return "\n".join(lines)

    def render_battery_cfg_csv(self) -> str:
        """Render the CSV content for a battery configuration.

        Creates one row per define, mapping the description to the
        corresponding define name.

        Returns:
            The CSV content as a string, including the header row.
        """
        lines = []
        lines.append("Setting; Details (define name)")
        lines.extend(
            f"{d['description']}; ``{d['name']}``" for d in self.file_cfg["defines"]
        )
        return "\n".join(lines)

    def keyword(self) -> Literal["Creating"]:  # noqa: D102
        return "Creating"

    def __str__(self) -> str:
        """Return the generated output path."""
        return str(self.outputs[0].path_from(self.generator.bld.path))


@feature("codegen_csv")
@after_method("process_rule")
def create_csv_task(self: task_gen) -> None:
    """Process the ``codegen_csv`` feature on a task generator.

    Creates a :class:`CreateCsvFile` task for each relevant entry of the
    configuration's ``files`` list. The output file is named after the JSON
    configuration file (e.g. ``battery_cell_cfg.csv``). Entries for
    ``battery_cell_cfg.c`` are skipped, since they only contain lookup table
    references that are not documented.

    Required task generator attributes:
        - **json (Node)**: The JSON configuration source node.
    """
    json_node = getattr(self, "json", None)
    if not json_node:
        return
    file_name = Path(json_node.relpath())
    content = json_node.read_json()

    for file_cfg in content["files"]:
        if file_cfg["file_name"] == "battery_cell_cfg.c":
            continue
        out = self.path.find_or_declare(f"{file_name.stem}.csv")
        task = self.create_task("CreateCsvFile", json_node, out)
        task.file_cfg = file_cfg


def _read_csv(json_node: Node, csv_file: str) -> list[str]:
    """Read a CSV file."""
    lines = []
    csv_node = json_node.parent.find_node(csv_file)
    if not csv_node:
        msg = f"Missing CSV file: {Path(json_node.parent.relpath()).as_posix()}/{csv_file}"
        raise FileNotFoundError(msg)
    with open(csv_node.abspath(), encoding="utf-8") as f:
        reader = csv.reader(f, delimiter=";")
        next(reader)
        lines.extend(f"{{{line[0]}, {line[1]}}}," for line in reader)
    lines = [f"    {line}" for line in lines]
    return "\n".join(lines)


def _is_csv(value: object) -> bool:
    """Check whether a value is a string referencing a CSV file."""
    return isinstance(value, str) and value.endswith(".csv")


def _is_diag_array(define: dict) -> bool:
    return define.get("type") == "diag_array"


def _format_bool_literal(value: bool) -> str:
    return "true" if value else "false"


def _format_int_literal(value: int) -> str:
    # Keep plain int literals for 32-bit range and use LL for larger values.
    if -(2**31) <= value <= (2**31 - 1):
        return str(value)
    return f"{value}LL"


def _format_uint_literal(value: int) -> str:
    # Use standard unsigned suffix for 32-bit values and uLL beyond that.
    if value <= 0xFFFFFFFF:
        return f"{value}u"
    return f"{value}uLL"


def _format_float_literal(value: float) -> str:
    literal = repr(float(value))
    if "." not in literal and "e" not in literal.lower():
        literal = f"{literal}.0"
    return f"{literal}f"


@dataclass(frozen=True)
class NumericLimits:
    """Min/max limits for numeric types.

    Used to range-check define values against their declared C type before
    they are rendered as literals.

    Attributes:
        min_value: Smallest value representable by the type.
        max_value: Largest value representable by the type.
    """

    min_value: int | float
    max_value: int | float


INT_TYPE_LIMITS: dict[str, NumericLimits] = {
    "int8_t": NumericLimits(min_value=-(2**7), max_value=2**7 - 1),
    "int16_t": NumericLimits(min_value=-(2**15), max_value=2**15 - 1),
    "int32_t": NumericLimits(min_value=-(2**31), max_value=2**31 - 1),
    "int64_t": NumericLimits(min_value=-(2**63), max_value=2**63 - 1),
}

UINT_TYPE_LIMITS: dict[str, NumericLimits] = {
    "uint8_t": NumericLimits(min_value=0, max_value=2**8 - 1),
    "uint16_t": NumericLimits(min_value=0, max_value=2**16 - 1),
    "uint32_t": NumericLimits(min_value=0, max_value=2**32 - 1),
    "uint64_t": NumericLimits(min_value=0, max_value=2**64 - 1),
}

FLOAT_TYPE_LIMITS: dict[str, NumericLimits] = {
    "float32_t": NumericLimits(
        min_value=-3.4028234663852886e38, max_value=3.4028234663852886e38
    ),
    "float64_t": NumericLimits(
        min_value=-1.7976931348623157e308, max_value=1.7976931348623157e308
    ),
}


def _format_typed_int_literal(name: str, value: object, value_type: str) -> str:
    if isinstance(value, bool) or not isinstance(value, int):
        msg = f"Expected integer for define '{name}', but got {type(value).__name__}"
        raise TypeError(msg)

    if value_type in INT_TYPE_LIMITS:
        limits = INT_TYPE_LIMITS[value_type]
        if not limits.min_value <= value <= limits.max_value:
            msg = f"Value {value} out of range for {value_type} in define '{name}'"
            raise ValueError(msg)
        if value_type == "int64_t":
            return f"{value}LL"
        return str(value)

    if value_type in UINT_TYPE_LIMITS:
        limits = UINT_TYPE_LIMITS[value_type]
        if not limits.min_value <= value <= limits.max_value:
            msg = f"Value {value} out of range for {value_type} in define '{name}'"
            raise ValueError(msg)
        if value_type == "uint64_t":
            return f"{value}uLL"
        return f"{value}u"

    msg = f"Unsupported integer value_type '{value_type}' for define '{name}'"
    raise ValueError(msg)


def _format_typed_float_literal(name: str, value: object, value_type: str) -> str:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        msg = f"Expected float for define '{name}', but got {type(value).__name__}"
        raise TypeError(msg)

    float_value = float(value)
    if not math.isfinite(float_value):
        msg = f"Non-finite float value {value} for define '{name}'"
        raise ValueError(msg)

    limits = FLOAT_TYPE_LIMITS[value_type]
    if not limits.min_value <= float_value <= limits.max_value:
        msg = f"Value {value} out of range for {value_type} in define '{name}'"
        raise ValueError(msg)

    if value_type == "float32_t":
        return _format_float_literal(float_value)
    if value_type == "float64_t":
        return repr(float_value)
    msg = f"Unsupported float value_type '{value_type}' for define '{name}'"
    raise ValueError(msg)


def _render_define_value(define: dict) -> str:
    """Render the value of a define based on its type.

    This only handles the following types: bool, int*_t, uint*_t, float*_t.

    Args:
        define (dict): A dictionary containing the define's properties,
            including 'name', 'value', and 'value_type'.

    Returns:
        str: The rendered value as a string suitable for C code.
    """
    name = define["name"]
    value = define.get("value", "")
    value_type = define.get("value_type")

    if not value_type:
        msg = f"Missing value_type for define '{name}'"
        raise ValueError(msg)

    if value_type == "bool":
        if not isinstance(value, bool):
            msg = f"Expected bool for define '{name}'"
            raise ValueError(msg)
        return _format_bool_literal(value)

    if value_type in INT_TYPE_LIMITS or value_type in UINT_TYPE_LIMITS:
        return _format_typed_int_literal(name, value, value_type)

    if value_type in FLOAT_TYPE_LIMITS:
        return _format_typed_float_literal(name, value, value_type)

    msg = f"Unknown value_type '{value_type}' for define '{name}'"
    raise ValueError(msg)


def resolve_values(json_node: Node, file_cfg: dict) -> dict:
    """Resolve all values in a file configuration.

    Builds the substitution dictionary for the template renderer: ``defines``
    entries are converted into C literals, CSV references are expanded into
    initializer lists, and ``diag_array`` defines are rendered from the
    configuration's ``array_entries``. All other keys are passed through
    unchanged.

    Args:
        json_node: Node of the JSON configuration file.
        file_cfg: A single entry of the configuration's ``files`` list.

    Returns:
        A dictionary mapping template placeholder names to their rendered
        string values.

    Raises:
        FileNotFoundError: If a referenced CSV file does not exist.
        TypeError: If a define value does not match its declared type.
        ValueError: If a ``value_type`` is missing, unsupported, or a value
            is out of range.
    """
    values = {}
    for key, value in file_cfg.items():
        if key == "defines":
            for define in value:
                name = define["name"]
                val = define.get("value", "")
                if _is_diag_array(define):
                    values[name] = render_diag_array(file_cfg["array_entries"])
                elif _is_csv(val):
                    values[name] = _read_csv(json_node, val)
                else:
                    values[name] = _render_define_value(define)
        else:
            values[key] = value
    return values


def _get_template(bld: BuildContext, file_name: str) -> Node:
    file_path = Path(file_name)
    template_path = f"conf/bms/templates/{file_path.stem}_template{file_path.suffix}"
    template_node = bld.srcnode.find_node(template_path)
    if not template_node:
        bld.fatal(f"Template file not found: '{template_path}'")
    return template_node


def create_config(bld: BuildContext, json_node: Node, idx: str) -> str:
    """Create the C/H code from a JSON configuration and template.

    Resolves all values of the ``idx``-th entry of the configuration's
    ``files`` list, adds the generation metadata (creation date, date of last
    update, and foxBMS version obtained from
    :class:`~waf_tools.vcs.VcsInformation`), and renders the corresponding
    template.

    Args:
        bld: The active waf build context.
        json_node: Node pointing to the JSON configuration file.
        idx: Index of the file entry within the configuration's ``files`` list.

    Returns:
        The fully rendered file content as a string.

    Raises:
        FileNotFoundError: If a referenced CSV file does not exist.
        TypeError: If a define value does not match its declared type.
        ValueError: If a define value is invalid or out of range.
        SystemExit: Indirectly via ``bld.fatal()`` if the template file is
            missing.
    """
    config = json_node.read_json()
    file_cfg = config["files"][idx]
    file_name = file_cfg["file_name"]

    template_node = _get_template(bld, file_name)

    template_txt = template_node.read()

    today = datetime.now(tz=UTC).date().strftime("%Y-%m-%d")
    version: VcsInformation = bld.gather_and_validate_version_info()

    values = resolve_values(json_node, file_cfg)
    values.update(
        {
            "date": f"{today} (date of creation)",
            "updated": f"{today} (date of last update)",
            "version": f"v{version.major}.{version.minor}.{version.patch}",
        }
    )

    return render_template(template_txt, values)
