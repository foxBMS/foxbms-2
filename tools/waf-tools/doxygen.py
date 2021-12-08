#! /usr/bin/env python
# encoding: UTF-8
# Thomas Nagy 2008-2010 (ita)

# changed by the foxBMS Team:
# - applied black to have a uniform style guide with in the foxBMS project
# - fixed things pylint found
# - some variables/methods etc. were renamed
# - parsing the doxygen file has been simplified to our needs
# Compare to
# https://gitlab.com/ita1024/waf/-/blob/waf-2.0.20/waflib/extras/doxygen.py
# if you want to see all changes in detail.

"""Implements a waf tool to run doxygen

:numref:`doxygen-usage` shows how to use this tool.

.. code-block:: python
    :caption: doxygen.py
    :name: doxygen-usage
    :linenos:

    def configure(conf):
        conf.load("doxygen")

    def build(bld):
        bld(features="doxygen", doxygen_conf="doxygen.conf")

"""

import os
import os.path
import re

from waflib import Context, Node, Task, TaskGen


class doxygen(Task.Task):  # pylint: disable=invalid-name
    """Doxygen task"""

    color = "BLUE"
    ext_in = [".c", ".h"]

    run_str = "${DOXYGEN} ${SRC[0].abspath()}"

    def runnable_status(self):
        for i in self.run_after:
            if not i.hasrun:
                return Task.ASK_LATER
        # all options must be on a single line
        doxygen_full_config = self.inputs[0].read().splitlines()
        self.doxygen_config = {}  # pylint: disable=attribute-defined-outside-init
        for i in doxygen_full_config:
            if not i.startswith("#") and not i == "":
                i_txt = re.sub(" +", " ", i).replace(" = ", "=")
                try:
                    key, val = i_txt.split("=")
                    self.doxygen_config[key] = val.split(" ")
                except ValueError:
                    # not simple parseable, ignore, as we are only interested
                    # in a few simple values
                    pass
        self.doxygen_input = []  # pylint: disable=attribute-defined-outside-init
        for node in self.doxygen_config["INPUT"]:
            self.doxygen_input.append(self.generator.bld.root.find_node(node))

        # Output path is always an absolute path as it was transformed above.
        # W0201 is 'attribute-defined-outside-init'
        out_dir = self.doxygen_config["OUTPUT_DIRECTORY"][0]
        path = self.generator.bld.path
        self.output_dir = path.find_or_declare(out_dir)  # pylint: disable=W0201
        self.output_dir.mkdir()
        self.signature()
        return Task.Task.runnable_status(self)

    def scan(self):
        """gets all sources from the INPUT directory based on the FILE_PATTERNS
        and excludes everything found by EXCLUDE_PATTERNS"""
        exclude_patterns_config = self.doxygen_config.get("EXCLUDE_PATTERNS", [])
        exclude_patterns = [
            pattern.replace("*/", "**/") for pattern in exclude_patterns_config
        ]
        file_patterns = self.doxygen_config.get("FILE_PATTERNS", ["*.c", "*.h"])
        if self.doxygen_config.get("RECURSIVE", ["YES"]) == ["YES"]:
            file_patterns = [f"**/{pattern}" for pattern in file_patterns]
        nodes = []
        names = []
        for node in self.doxygen_input:
            if os.path.isdir(node.abspath()):
                for i in node.ant_glob(incl=file_patterns, excl=exclude_patterns):
                    nodes.append(i)
            else:
                nodes.append(node)
        return (nodes, names)

    def post_run(self):
        nodes = self.output_dir.ant_glob("**/*", quiet=True)
        for node in nodes:
            self.generator.bld.node_sigs[node] = self.uid()
        self.outputs += nodes
        return Task.Task.post_run(self)


@TaskGen.feature("doxygen")
def process_doxygen(self):
    """creates the doxygen task"""
    if not getattr(self, "doxygen_conf", None):
        self.generator.bld.fatal("No doxygen configuration file supplied.")
    if not isinstance(self.doxygen_conf, Node.Node):
        self.generator.bld.fatal("'doxygen_conf' must be a Node.")

    self.create_task(
        "doxygen",
        self.doxygen_conf,
        cwd=self.bld.path.get_bld().abspath(),
        output=Context.STDERR,
    )


def configure(conf):
    """Check if doxygen and dot are available.

    Dot is needed to draw the diagrams"""
    conf.find_program("doxygen", var="DOXYGEN")
    conf.find_program("dot", var="DOT")
