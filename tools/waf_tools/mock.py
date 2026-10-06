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

"""CMock and Unity test runner integration for the Waf build system.

This tool automates the generation of mock source files (via CMock)
and test-runner source files (via Unity's ``generate_test_runner.rb``)
for C unit tests. It is designed to work together with the ``preprocess``
and ``gcov`` tools in this toolchain.

Usage
-----

1. Load the tool during ``configure``

   .. code-block:: python

      def configure(ctx):
          ctx.find_program("ruby", var="RUBY")
          ctx.load("mock", tooldir=TOOLDIR)

2. Declare test targets in a ``test.json`` file (one per test directory)

   .. code-block:: python

      [
          {
              "source": [
                  "src/app/abc/abc.c"
              ],
              "test": "test_abc.c",
              "includes": [
                  "src/app/abc",
                  "src/app/def"
              ],
              "mocks": [
                  "src/app/def/def.h"
              ],
              "defines": [
                  "EXAMPLE_DEFINE=1u"
              ],
              "use": [
                  "M"
              ]
          }
      ]

3. In the variant's build script (``tests/unit/wscript``), load and
   instantiate the test configurations

   .. code-block:: python

      def build(bld):
          bld.load_test_configs()
          for i, cfg in enumerate(bld.test_configs):
              bld(
                  features=cfg["features"],
                  target=cfg["target"],
                  source=cfg["source"],
                  includes=cfg["includes"],
                  mocks=cfg["mocks"] + ["test-framework", f"{var}-c-test-constants"],
                  cmock_config=cfg["cmock_config"],
                  uselib=cfg["COVERAGE"],
                  defines=cfg["defines"],
                  cflags=cfg["cflags"],
                  path=cfg["path"],
                  idx=i,
              )

Dependencies
------------

- Ruby interpreter (stored in ``env.RUBY``)
- CMock (bundled under ``tools/cmock/``)
- Unity (bundled as CMock vendor dependency)
- ``preprocess`` waf tool (this toolchain)

"""

from typing import Literal

import preprocess
from waflib import Utils
from waflib.Build import BuildContext
from waflib.Configure import ConfigurationContext, conf
from waflib.Node import Node
from waflib.Task import Task
from waflib.TaskGen import after_method, before_method, feature, task_gen


def configure(ctx: ConfigurationContext) -> None:
    """Configure paths to CMock and the Unity test runner generator.

    Stores the absolute paths to ``cmock.rb`` and ``generate_test_runner.rb``
    in ``ctx.env`` for use by the :class:`CMock` and :class:`Runner` tasks.
    """
    ctx.env.CMOCK = f"{ctx.srcnode}/tools/cmock/lib/cmock.rb"
    ctx.env.RUNNER = (
        f"{ctx.srcnode}/tools/cmock/vendor/unity/auto/generate_test_runner.rb"
    )


class CMock(Task):
    """Task that generates CMock mock source files from header files.

    Invokes ``cmock.rb`` to produce ``Mock<Header>.h`` and ``Mock<Header>.c``
    from preprocessed and cleaned header files.
    """

    def scan(self) -> tuple[list[Node], list[Node]]:
        """Register the global CMock configuration file as a dependency.

        Returns:
            A tuple of (dependency nodes, empty list). The global
            ``bld.cmock_cfg`` node is included so that changes to the CMock
            configuration trigger a rebuild.
        """
        cfg = getattr(self.generator.bld, "cmock_cfg", None)
        if not cfg:
            return ([], [])
        return ([cfg], [])

    def run(self) -> None:
        """Execute ``cmock.rb`` to generate mock files.

        Builds the command from ``env.RUBY``, ``env.CMOCK``, and the global
        plus optional per-target configuration files. Executes in the output
        directory of the first output node.
        """
        cwd = self.outputs[0].parent
        cwd.mkdir()

        bld = self.generator.bld
        cfg = bld.cmock_cfg
        if not cfg:
            bld.fatal("No cmock config found")

        add_config_node = getattr(self, "cmock_config_node", None)
        cmd = bld.env.RUBY + [
            bld.env.CMOCK,
            f"-o{cfg}",
        ]

        if add_config_node:
            add_cmock_config = add_config_node.abspath()
            cmd.append(f"-o{add_cmock_config}")

        self.exec_command(
            cmd + [i.abspath() for i in self.inputs],
            cwd=cwd.abspath(),
        )


class Runner(Task):
    """Task that generates Unity test runner source files.

    Invokes ``generate_test_runner.rb`` to produce a ``<test>_Runner.c`` file
    containing the Unity ``main()`` entry point that discovers and runs
    all test functions in the test source file.
    """

    color = "GREY"

    def scan(self) -> tuple[list[Node], list[Node]]:
        """Register the global Unity configuration file as a dependency.

        Returns:
            A tuple of (dependency nodes, empty list). The global
            ``bld.unity_cfg`` node is included so that changes to the
            Unity configuration trigger a rebuild.
        """
        cfg = getattr(self.generator.bld, "unity_cfg", None)
        if not cfg:
            return ([], [])
        return ([cfg], [])

    def run(self) -> int:
        """Execute ``generate_test_runner.rb`` to generate the runner file.

        Returns:
            The exit code of the runner generation command.
        """
        bld = self.generator.bld
        cmd = bld.env.RUBY + [bld.env.RUNNER]

        cfg = getattr(self.generator.bld, "unity_cfg", None)
        if cfg:
            cmd.append(cfg.abspath())

        return self.exec_command(
            cmd
            + [
                self.inputs[0].abspath(),
                self.outputs[0].abspath(),
            ]
        )

    def keyword(self) -> Literal["Creating"]:  # noqa: D102
        return "Creating"

    def __str__(self) -> str:
        """Return the generated output path."""
        return str(self.outputs[0].path_from(self.outputs[0].ctx.launch_node()))


def find_test_file(self: task_gen) -> "Node | None":
    """Find the test source file associated with the current target.

    Scans ``self.source`` for a node whose name contains ``test_``.

    Returns:
        The test source file node, or ``None`` if none is found.
    """
    source_nodes = self.to_nodes(self.source)
    for source_node in source_nodes:
        if "test_" in source_node.name:
            return source_node
    return None


def create_mocks(
    self: task_gen, header_nodes: list[Node], test_file: Node
) -> list[Node]:
    """Declare mock output nodes for each header to be mocked.

    Creates ``Mock<Header>.h`` and ``Mock<Header>.c`` output nodes under
    ``mocks/<test_name>/`` in the build directory.

    Args:
        self: The task generator instance.
        header_nodes: Header file nodes to generate mocks for.
        test_file: The test source file (used to derive the output
            subdirectory name).

    Returns:
        A list of declared output nodes (headers and sources interleaved).
    """
    test_name = test_file.name.replace(test_file.suffix(), "")
    mock_output_nodes = []
    for header_node in header_nodes:
        if header_node:
            header_base_name = header_node.name.replace(".h", "")
            outputs = [
                self.bld.path.find_or_declare(
                    f"mocks/{test_name}/Mock{header_base_name}.h"
                ),
                self.bld.path.find_or_declare(
                    f"mocks/{test_name}/Mock{header_base_name}.c"
                ),
            ]
            mock_output_nodes.extend(outputs)
    return mock_output_nodes


def create_runners(self: task_gen, test_file: Node) -> list[Node]:
    """Declare the runner output node under ``runner/``.

    Args:
        self: The task generator instance.
        test_file: The test source file (used to derive the runner file name).

    Returns:
        A single-element list containing the declared runner output node.
    """
    test_name = test_file.name.replace(test_file.suffix(), "")
    return [self.bld.path.find_or_declare(f"runner/{test_name}_Runner.c")]


def create_result(bld: BuildContext) -> None:
    """Create aggregated unit-test result files per target from the stored test results."""
    test_results = getattr(bld, "utest_results", [])  # cspell:ignore utest
    if not test_results:
        return

    results_by_target = extract_targets_from_results(bld, test_results)
    results_dir = bld.bldnode.make_node("results")
    results_dir.mkdir()

    for target_name, target_results in results_by_target.items():
        result_node = results_dir.make_node(f"{target_name}_result.txt")
        output_text = "".join(result.out.decode() for result in target_results)
        if target_results:
            result_node.write(output_text, encoding="utf-8")


def extract_targets_from_results(
    bld: BuildContext, test_results: list
) -> dict[str, list]:
    """Group unit-test results by target name.

    The target name is derived from the parent directory of the test binary.
    """
    results_by_target: dict[str, list] = {}
    for result in test_results:
        target_node = bld.root.find_node(result.test_path)
        results_by_target.setdefault(target_node.parent.name, []).append(result)
    return results_by_target


@conf
def load_test_configs(self: BuildContext) -> None:
    """Load test configurations from JSON files.

    Discovers all ``test.json`` files for the active variant (``app``
    or ``bootloader``), parses them, resolves all paths to waf Nodes,
    and populates ``self.test_configs`` - a list of dictionaries ready
    to be passed to ``bld()``.

    Supported config keys in each ``test.json``:
        - **platform (str)**: Optional host platform (``"win32"`` or
            ``"linux"``) on which the config is enabled.
        - **source (str | list)**: Source file paths to compile.
        - **test (str)**: Path to the test source file (relative to the JSON file).
        - **includes (list)**: Include directory paths.
        - **export_includes (list)**: Exported include directory paths.
        - **mocks (list)**: Mock header file paths.
        - **cmock_config (str)**: An optional per-target CMock configuration.
        - **use (str | list)**: Dependencies to link against.
        - **defines (list)**: Compile-time definitions.
        - **cflags (list)**: Additional C compiler flags.
        - **features (str)**: Build features (default: ``"c cprogram test"``).
    """
    self.test_configs = []
    self.all_test_configs = []
    self.test_config_nodes = []
    host_platform = "win32" if Utils.is_win32 else "linux"

    var = self.variant.split("_", maxsplit=1)[0]
    json_nodes = list(self.path.ant_glob(f"{var}/**/test.json"))
    if var == "app":
        json_nodes += list(self.path.ant_glob("os/**/test.json"))

    for json_node in json_nodes:
        self.test_config_nodes.append(json_node)
        configs = json_node.read_json()
        if isinstance(configs, dict):
            configs = [configs]

        for config in configs:
            test = config.get("test", None)
            if not test:
                self.fatal(f"{json_node.relpath()}: 'test' missing in {config}")

            test_node = json_node.parent.find_node(test)
            if not test_node:
                self.fatal(
                    f"{json_node.relpath()} [{test}]: Testfile not found: {test}"
                )

            self.all_test_configs.append({"path": json_node.parent, "test": test_node})

            platform = config.get("platform")
            if platform and platform != host_platform:
                continue

            source = []
            for src in Utils.to_list(config.get("source", [])):
                node = resolve_node(self, src)
                if node:
                    source.append(node)
                else:
                    self.fatal(
                        f"{json_node.relpath()} [{test}]: Source not found: {src}"
                    )

            source.append(test_node)

            includes = []
            for incl in config.get("includes", []):
                node = resolve_node(self, incl)
                if node:
                    includes.append(node)
                else:
                    self.fatal(
                        f"{json_node.relpath()} [{test}]: Include not found: {incl}"
                    )

            mocks = []
            for mock in config.get("mocks", []):
                node = resolve_node(self, mock)
                if node:
                    mocks.append(node)
                else:
                    self.fatal(
                        f"{json_node.relpath()} [{test}]: Mock not found: {mock}"
                    )

            self.test_configs.append(
                {
                    "features": config.get("features", "c cprogram test"),
                    "target": test.removesuffix(".c"),
                    "source": source,
                    "includes": includes,
                    "mocks": mocks,
                    "cmock_config": config.get("cmock_config", ""),
                    "use": Utils.to_list(config.get("use", [])),
                    "defines": Utils.to_list(config.get("defines", [])),
                    "cflags": Utils.to_list(config.get("cflags", [])),
                    "path": json_node.parent,
                }
            )


def resolve_node(self: BuildContext, path: str) -> Node:
    """Resolve a path string to a waf Node.

    Handles special cases for HAL generated sources (``HL_`` prefix),
    application config paths, and diagnostic directory layouts by checking
    multiple root locations.

    Args:
        self: The active build context.
        path: The path string to resolve.

    Returns:
        The resolved waf Node, or ``None`` if not found.
    """
    if "HL_" in path:
        return self.bldnode.find_or_declare(f"{self.env.HAL_DIR[0]}/{path}")
    if "src/app/application/config" in path:
        return self.srcnode.find_node(path) or self.bldnode.find_or_declare(path)
    if "ades183x/diag" in path or "ades1830/diag" in path:
        return self.srcnode.find_node(path) or self.srcnode.make_node(path)

    return (
        self.srcnode.find_node(path)
        or self.bldnode.find_node(path)
        or self.root.find_node(path)
    )


@feature("test")
@before_method("process_source")
def setup(self: task_gen) -> None:
    """Set up mock generation and test runner creation for a test target.

    For each task generator with the ``test`` feature:

    1. Preprocesses mock header files (via the ``preprocess`` tool).
    2. Creates :class:`CMock` tasks to generate ``Mock<Header>.h/.c`` from the
       cleaned headers.
    3. Creates a :class:`Runner` task to generate a ``<test>_Runner.c`` file
       containing the Unity ``main()`` entry point.
    4. Extends ``self.source`` with the generated mock and runner files.
    """
    if not getattr(self, "source", None):
        return
    if not isinstance(self.source, list):
        self.source = [self.source]

    test_file = find_test_file(self)

    if not hasattr(self, "includes"):
        self.includes = []

    if hasattr(self, "mocks"):
        mock_header_nodes = getattr(self, "mocks", [])
        preprocessed_nodes = preprocess.create_preprocessed_files(
            self, mock_header_nodes, test_file
        )
        mock_output_nodes = create_mocks(self, mock_header_nodes, test_file)
        if mock_output_nodes:
            clean_header_nodes = []
            for preproc_node in preprocessed_nodes:
                clean_header_node = preprocess.create_clean_header(self, preproc_node)
                clean_header_nodes.append(clean_header_node)

            mock_tasks = self.create_task(
                "CMock", src=clean_header_nodes, tgt=mock_output_nodes
            )

            self.source.extend([i for i in mock_tasks.outputs if i.suffix() == ".c"])
            self.includes.extend([mock_tasks.outputs[0].parent])

            add_config_name = getattr(self, "cmock_config", None)
            if add_config_name:
                add_config_node = test_file.parent.find_node(add_config_name)
                if add_config_node:
                    mock_tasks.dep_nodes.append(add_config_node)
                    mock_tasks.cmock_config_node = add_config_node

    runner_output_node = create_runners(self, test_file)

    runner_task = self.create_task("Runner", test_file, runner_output_node)
    self.source.extend(runner_task.outputs)


@feature("test")
@after_method("process_source")
def add_mock_cflags(self: task_gen) -> None:
    """Append mock-specific warning-suppression flags to mock compilation tasks."""
    if not getattr(self, "mocks", None):
        return
    config_cflags = list(getattr(self, "cflags", []))
    cc_version = self.bld.env.CC_VERSION
    gcc_major = int(cc_version[0]) if cc_version else 0
    for task in self.tasks:
        if task.__class__.__name__ == "c" and task.inputs:
            if "Mock" in task.inputs[0].name:
                if config_cflags:
                    task.env.append_value("CFLAGS", config_cflags)
                # -Wno-array-parameter is only needed for GCC 11+,
                # as the warning was first released in that version.
                # Passing it to older versions would cause an
                # "unrecognized command line option" error.
                if gcc_major >= 11:
                    task.env.append_unique(
                        "CFLAGS",
                        [
                            "-Wno-array-parameter",
                            "-Wno-unused-variable",
                            "-Wno-error=int-conversion",
                        ],
                    )
