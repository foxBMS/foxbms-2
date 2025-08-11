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

"""Testing file 'foo.py'."""

# start-include-in-docs-imports
import io  # when stdout/stderr need to be captured
import unittest
from contextlib import redirect_stderr, redirect_stdout  # to capture stdout/stderr

from foo import Foo  # module under test

# stop-include-in-docs-imports


# start-include-in-docs-instantiation
# Separate unit test class per function, starting with the object instantiation
class TestFooInstantiation(unittest.TestCase):
    """Test 'Foo'-object instantiation."""

    def test_foo_instantiation_ok(self):
        """The object can be instantiated."""
        Foo(1)
        # nothing to assert for in this case.

    def test_foo_instantiation_wrong_initialization_value(self):
        """The object can not be instantiated because some reason."""
        # as the instantiation throws an exception, we need to capture it and
        # check that we raise the expected exception
        err = io.StringIO()
        out = io.StringIO()
        # always capture stderr/stdout for later assert, to make sure we really
        # get the output we expect
        with redirect_stderr(err), redirect_stdout(out):
            with self.assertRaises(SystemExit) as cm:
                Foo(0)
        # assert that the correct exception is thrown
        self.assertEqual(cm.exception.code, "foo")
        self.assertEqual(err.getvalue(), "")
        self.assertEqual(out.getvalue(), "")
        # stop-include-in-docs-instantiation


# start-include-in-docs-add-two
# Next, test some method of the class
class TestFooAddTwo(unittest.TestCase):
    """Test 'add_two' method of the 'Foo' class."""

    def test_add_two(self):
        """Calling 'add_two' on an 'Foo' instance shall add 2 to its 'attr'
        attribute."""
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            # call the function under test and store the return value in a
            # variable called 'ret'
            ret = Foo(1).add_two()
        # assert on the return value
        self.assertEqual(3, ret)
        self.assertEqual(err.getvalue(), "")
        self.assertEqual(out.getvalue(), "")
        # stop-include-in-docs-add-two


# start-include-in-docs-print-attr
# Next, capture output to stdout and compare it, i.e., running tests shall not
# write to the console
class TestFooPrintAttr(unittest.TestCase):
    """Test 'print_attr' method of the 'Foo' class."""

    def test_print_attr(self):
        """The string-representation of the 'attr' attribute shall be printed
        to stdout."""
        bla = Foo(1)
        err = io.StringIO()
        out = io.StringIO()
        with redirect_stderr(err), redirect_stdout(out):
            # call the function under test and store the return value in a
            # variable called 'ret'
            bla.print_attr()
        # assert on stdout
        self.assertEqual(err.getvalue(), "")
        self.assertEqual(out.getvalue(), "1\n")
        # stop-include-in-docs-print-attr


# make tests runnable as script
if __name__ == "__main__":
    unittest.main()
