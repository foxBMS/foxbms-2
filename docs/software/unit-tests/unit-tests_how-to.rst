.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _HOW_TO_USE_UNIT_TESTS:

How to use unit tests
=====================

Verify that the unit testing framework is work is working as expected:

.. code-block:: console

    waf build_unit_test
    waf build_unit_test --coverage

Typical usage and more information on the unit tests can be found in :ref:`Unit tests <UNIT_TESTS>`.

Using ceedling directly
=======================

.. warning::
    Ceedling (the unit test runner that is used by this project) is wrapped by
    the waf-toolchain. The toolchain makes sure that the path handed to
    ceedling contains the necessary applications and files, so without the
    wrapper extensive knowledge of the project structure is helpful.

    That being said, it is normally not necessary to directly execute ceedling
    as the wrapper handles this.

The waf-script of this project executes always the complete unit test-suite.
This can take some time. When developing or debugging a unit test, it might be
beneficial to have finer control over which test is executed.

For this and especially for debugging the unit test integration it is possible
to directly access ceedling. It is therefore necessary for the shell from which
ceedling is run that all dependencies are available. Especially, it is
necessary that it is possible to find ceedling from the current ``PATH`` of the
shell in which this is attempted. This will not be covered in this manual.

Once such a shell is available it also has to be made sure that all files that
are generated or adapted on-the-fly are available in the ``build``-directory.
This can be ensured by executing the waf-task ``build_unit_test`` just until
ceedling is run by it. After this step (it does not matter whether the task
completes or is imported after it has generated the dependencies) the following
example can be executed.

.. code-block:: console
    :emphasize-lines: 3, 7, 11

    # go to the build directory of ceedling, assuming the current directory is
    # the project root (IMPORTANT, otherwise it will not work correctly)
    cd build/unit_test/

    # interact directly with ceedling (refer to ceedling manual for details)
    # this line executes test_plausibility.c with coverage
    ceedling gcov:test_plausibility.c

    # should the current shell not be able to directly follow the ceedling.cmd
    # file, it might be necessary to access the vendored ceedling directly:
    ruby ../../tools/vendor/ceedling/bin/ceedling gcov:test_plausibility.c
