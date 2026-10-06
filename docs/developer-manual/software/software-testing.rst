.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _SOFTWARE_TESTING:

Software Testing
================

.. _module_testing:

Module Testing
--------------

All source code **MUST** be written in a unit-testable way.
All source code **MUST**
be unit tested in the CI pipeline for every commit.
Low-level code that cannot be tested may be omitted, but this has to be
justified in the documentation.

The coverage report **MUST** result in a line and branch coverage of 100%.

For details see :numref:`UNIT_TESTS`.

.. _integration_testing:

Integration Testing
-------------------

The linked program **MUST** be run in a HIL test.

The coverage report of the HIL test **MUST** result in a line and branch
coverage of 100%.
For some parts of the software, this requires additional manual testing to
complement the automated tests that are run within the CI pipeline.
