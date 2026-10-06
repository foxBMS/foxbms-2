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

"""Check or fix ordering of `includes` lists in `wscript` files.

Entries are ordered by origin: `bld.bldnode` entries first, local entries
second, and `bld.srcnode` entries last. Paths are sorted component-wise within
each origin group.
"""

import argparse
import ast
import posixpath
import sys
from collections.abc import Sequence
from pathlib import Path

# The required include-group order: bldnode, local, then srcnode.
_BLDNODE_SORT_GROUP = 0
_LOCAL_SORT_GROUP = 1
_SRCNODE_SORT_GROUP = 2


class InvalidWscriptError(Exception):
    """Raised when a `wscript` file cannot be parsed as Python."""


def extract_path(expression: ast.AST, source_text: str) -> str:
    """Extract a sortable path from one `includes` list expression.

    Args:
        expression: AST expression representing one element of an `includes`
            list.
        source_text: Complete `wscript` source text used to recover source
            text for unsupported expressions.

    Returns:
        The path from a string literal or a static `find_node` argument, `"."`
        for a bare supported Waf node, or the original source representation
        when no path can be extracted. Returns an empty string when no source
        representation is available.

    """
    # A literal include such as "include/foo" is represented by ast.Constant.
    if isinstance(expression, ast.Constant):
        # ast.Constant can also represent numbers or None. Only strings provide
        # a static path that can be used for sorting.
        if isinstance(expression.value, str):
            return expression.value

    # Calls such as bld.srcnode.find_node("src/foo") are ast.Call nodes.
    if isinstance(expression, ast.Call):
        # A method call is represented by an ast.Attribute function node.
        if isinstance(expression.func, ast.Attribute):
            # Only find_node receives the relative include path as an argument.
            if expression.func.attr == "find_node":
                # A call without positional arguments cannot provide a path.
                if expression.args:
                    path_argument = expression.args[0]

                    # Only static string arguments can be sorted reliably.
                    if isinstance(path_argument, ast.Constant) and isinstance(
                        path_argument.value, str
                    ):
                        return path_argument.value

    # A bare Waf node such as bld.bldnode, bld.path, or bld.srcnode has no
    # relative child path. "." sorts the root before child paths in its group.
    if isinstance(expression, ast.Attribute):
        # The attribute must be directly rooted at the variable named bld.
        # This prevents unrelated expressions such as other.srcnode from being
        # treated as Waf source-node paths.
        if isinstance(expression.value, ast.Name) and expression.value.id == "bld":
            # These are the Waf node roots supported by this hook.
            if expression.attr in ("bldnode", "path", "srcnode"):
                return "."

    # Unsupported expressions remain deterministic by using their source text.
    return ast.get_source_segment(source_text, expression) or ""


def _offset_map(source: str) -> list[int]:
    """Build a mapping from source line numbers to character offsets.

    Args:
        source: Complete source text of the processed `wscript` file.

    Returns:
        Character offsets for the start of every source line, followed by the
        end-of-file offset. Index `line_number - 1` identifies a line start.

    """
    offsets = [0]
    pos = 0
    for line in source.splitlines(keepends=True):
        pos += len(line)
        offsets.append(pos)
    return offsets


def _find_includes_lists(tree: ast.Module) -> list[ast.List]:
    """Find literal lists assigned to the variable name `includes`.

    Args:
        tree: Parsed AST of a `wscript` file.

    Returns:
        List AST nodes from assignments of the form `includes = [...]`.
        Assignments with non-list values or different assignment targets are
        ignored.

    """
    lists: list[ast.List] = []

    for node in ast.walk(tree):
        # Only assignment statements can define `includes = [...]`.
        if not isinstance(node, ast.Assign):
            continue

        for target in node.targets:
            # Assignment targets can be names, attributes, or unpacking
            # expressions. Only a plain name can be the `includes` variable.
            if not isinstance(target, ast.Name):
                continue

            # Ignore assignments to names other than `includes`.
            if target.id != "includes":
                continue

            # Only literal lists can safely be reordered and rewritten.
            if isinstance(node.value, ast.List):
                lists.append(node.value)

    return lists


def _sorted_element_texts(list_node: ast.List, source: str) -> list[str]:
    """Return source text for list elements in canonical include order.

    Args:
        list_node: AST list node containing the `includes` entries to sort.
        source: Complete source text used to preserve each list element's
            original source representation.

    Returns:
        Source text of all list elements, sorted by include origin group and
        component-wise normalized path.

    """
    sorted_elements = sorted(
        list_node.elts,
        key=lambda element: _path_sort_key(element, source),
    )
    return [
        ast.get_source_segment(source, element) or "" for element in sorted_elements
    ]


def _compute_changes(
    list_node: ast.List,
    source: str,
    offsets: list[int],
) -> tuple[int, int, str] | None:
    """Compute a source replacement for one unsorted `includes` list.

    Args:
        list_node: AST list node assigned to `includes`.
        source: Complete source text of the processed `wscript` file.
        offsets: Character offsets for source line starts, as produced by
            `_offset_map`.

    Returns:
        A tuple containing the absolute start offset, absolute end offset, and
        replacement text for an unsorted list. Returns `None` when the list is
        empty or already in canonical order.

    """
    # An empty list has only one possible order.
    if not list_node.elts:
        return None

    keys = [_path_sort_key(element, source) for element in list_node.elts]

    # If the current order equals the canonical sorted order, no diagnostic or
    # rewrite is necessary.
    if keys == sorted(keys):
        return None

    start_line = list_node.lineno
    start_col = list_node.col_offset
    end_line = list_node.end_lineno or start_line
    end_col = list_node.end_col_offset or 0

    start_abs = _absolute_offset(source, offsets, start_line, start_col)
    end_abs = _absolute_offset(source, offsets, end_line, end_col)

    line_start = offsets[start_line - 1]
    line_prefix = source[line_start:start_abs]

    # Preserve the indentation of the assignment line and indent elements by
    # one additional indentation level.
    base_indent = line_prefix[: len(line_prefix) - len(line_prefix.lstrip(" \t"))]
    element_indent = f"{base_indent}    "

    sorted_texts = _sorted_element_texts(list_node, source)
    new_list = (
        "[\n"
        + "".join(f"{element_indent}{element},\n" for element in sorted_texts)
        + f"{base_indent}]"
    )

    return start_abs, end_abs, new_list


def _bld_root_attribute(expression: ast.AST) -> str | None:
    """Find the root attribute of a `bld.<root>...` expression.

    Args:
        expression: AST expression representing one `includes` list element.

    Returns:
        The attribute immediately following `bld`, such as `"bldnode"`,
        `"path"`, or `"srcnode"`. Returns `None` when the expression is not
        rooted at `bld`.

    """
    node = expression

    # Waf node expressions consist of calls and attribute accesses. Follow
    # only the receiver chain to avoid classifying unrelated child nodes.
    while isinstance(node, (ast.Attribute, ast.Call)):
        # An ast.Call wraps the expression being called. For
        # bld.srcnode.find_node("src"), this exposes bld.srcnode.find_node.
        if isinstance(node, ast.Call):
            node = node.func
            continue

        # At this point, node is an ast.Attribute. An ast.Name receiver means
        # this is the innermost attribute, for example bld.srcnode.
        if isinstance(node.value, ast.Name):
            # Only expressions rooted in the variable named bld belong to a
            # Waf node group. For example, other.srcnode is not a srcnode path.
            if node.value.id == "bld":
                return node.attr
            return None

        # This is still an outer attribute, for example find_node in
        # bld.srcnode.find_node. Continue towards its receiver.
        node = node.value

    # String literals and unsupported expressions have no bld root attribute.
    return None


def _include_sort_group(expression: ast.AST) -> int:
    """Return the required origin-based sort group for one include entry.

    Args:
        expression: AST expression representing one `includes` list element.

    Returns:
        `_BLDNODE_SORT_GROUP` for entries rooted at `bld.bldnode`,
        `_SRCNODE_SORT_GROUP` for entries rooted at `bld.srcnode`, or
        `_LOCAL_SORT_GROUP` for string literals, `bld.path`, and unsupported
        expressions.

    """
    root_attribute = _bld_root_attribute(expression)

    # bld.bldnode points into the build directory. These entries sort first.
    if root_attribute == "bldnode":
        return _BLDNODE_SORT_GROUP

    # bld.srcnode points into the source directory. These entries sort last.
    if root_attribute == "srcnode":
        return _SRCNODE_SORT_GROUP

    # String literals, bld.path entries, and unsupported expressions are local
    # includes. They sort between bldnode and srcnode entries.
    return _LOCAL_SORT_GROUP


def _path_sort_key(expression: ast.AST, source: str) -> tuple[int, tuple[str, ...]]:
    """Build the canonical sort key for one include-list expression.

    Args:
        expression: AST expression representing one `includes` list element.
        source: Complete source text used to extract the expression path.

    Returns:
        A tuple whose first item is the include origin group and whose second
        item contains normalized path components. Tuple comparison therefore
        sorts by origin group before sorting paths within that group.

    """
    normalized_path = posixpath.normpath(extract_path(expression, source))

    # Python compares tuple items from left to right. The group is therefore
    # compared first, before path components are compared within that group.
    return (
        _include_sort_group(expression),
        tuple(normalized_path.split("/")),
    )


def _absolute_offset(
    source: str,
    offsets: list[int],
    line_no: int,
    byte_column: int,
) -> int:
    """Convert an AST UTF-8 byte column to a source character offset.

    Args:
        source: Complete source text containing the AST node.
        offsets: Character offsets for source line starts, as produced by
            `_offset_map`.
        line_no: One-based source line number reported by the AST.
        byte_column: Zero-based UTF-8 byte column reported by the AST.

    Returns:
        Absolute character offset into `source` corresponding to the supplied
        AST line and byte-column location.

    """
    line_start = offsets[line_no - 1]

    # The offset map contains an end-of-file entry after the final line start.
    # Use it as the line end when the requested line is known.
    line_end = offsets[line_no] if line_no < len(offsets) else len(source)
    line = source[line_start:line_end]

    # AST columns are UTF-8 byte offsets, while Python string indexes count
    # Unicode characters. Convert the byte prefix back to character length.
    return line_start + len(line.encode("utf-8")[:byte_column].decode("utf-8"))


def process_file(file_path: Path, fix: bool) -> tuple[int, str]:
    """Check or fix all `includes` lists in one `wscript` file.

    Args:
        file_path: Path to the `wscript` file to process.
        fix: Whether unsorted lists should be rewritten in canonical order.

    Returns:
        A tuple containing the number of unsorted lists found and an optional
        error message. The message is non-empty when the file cannot be parsed
        as Python. In fix mode, unsorted lists are rewritten before returning.

    """
    source = file_path.read_text(encoding="utf-8")

    try:
        tree = ast.parse(source)
    except SyntaxError as exc:
        msg = f"{file_path.as_posix()}: invalid Python syntax: {exc.msg}"
        return 1, msg

    lists = _find_includes_lists(tree)
    offsets = _offset_map(source)

    changes: list[tuple[int, int, str]] = []
    for list_node in lists:
        change = _compute_changes(list_node, source, offsets)
        if change:
            changes.append(change)

    # No replacements means that there are no includes lists or all lists
    # already follow the required ordering.
    if not changes:
        return 0, ""

    if fix:
        # Apply replacements from the end of the file backwards so earlier
        # offsets remain valid after later text has been replaced.
        new_source = source
        for start, end, replacement in sorted(changes, reverse=True):
            new_source = new_source[:start] + replacement + new_source[end:]

        file_path.write_text(new_source, encoding="utf-8")
        print(
            f"{file_path.as_posix()}: fixed {len(changes)} unsorted includes list(s).",
            file=sys.stderr,
        )
        return len(changes), ""

    # Check mode reports every unsorted list without changing the file.
    for start, _, _ in changes:
        line_no = source.count("\n", 0, start) + 1
        print(
            f"{file_path.absolute().as_posix()}:{line_no}: "
            "includes list is not sorted by origin and path components",
            file=sys.stderr,
        )

    return len(changes), ""


def main(argv: Sequence[str] | None = None) -> int:
    """Check or fix sorting of `includes` lists in `wscript` files.

    Args:
        argv: Optional command-line arguments containing file paths and the
            optional `--fix` flag.

    Returns:
        In check mode, the number of unsorted lists found, capped at `255`.
        In fix mode, `0` when no errors or changes were found, otherwise `1`
        after rewriting files or reporting errors.

    """
    parser = argparse.ArgumentParser(
        description="Check or fix sorting of 'includes' lists in wscript files."
    )
    parser.add_argument("files", nargs="*", help="wscript files to process")
    parser.add_argument(
        "--fix",
        action="store_true",
        help="Sort and rewrite unsorted includes lists.",
    )
    args = parser.parse_args(argv)

    total_errors = 0

    for file_str in args.files:
        file_path = Path(file_str)

        # Pre-commit can pass deleted or otherwise unavailable paths. Ignore
        # paths that are not regular files.
        if not file_path.is_file():
            continue

        err_count, message = process_file(file_path, args.fix)

        # Syntax errors are returned as messages so that processing can
        # continue with further files.
        if message:
            print(message, file=sys.stderr)

        total_errors += err_count

    # A fixing pre-commit hook must fail after modifying files so users can
    # inspect and stage the generated changes.
    if args.fix:
        return 1 if total_errors else 0

    return min(total_errors, 255)


if __name__ == "__main__":
    raise SystemExit(main())
