.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _SOFTWARE_VERIFICATION:

Software Verification
=====================

Test Plans
----------

The test plan includes module and integration tests (see
:numref:`module_testing` and :numref:`integration_testing`).
These tests are automatically run in the CI pipeline for every revision of the
software.

Failing tests lead to a rejection of the feature branch until the test suite
passes (see :numref:`SOFTWARE_DEVELOPMENT_PROCESS`).

Software Analysis
-----------------

The source code is subject to a static program analysis. These tests are
performed by Axivion Suite and Cppcheck (see :numref:`AXIVION_BAUHAUS_SUITE`
and :numref:`CPPCHECK`).

The architecture is verified by the Axivion Suite (see
:numref:`AXIVION_BAUHAUS_SUITE`)
