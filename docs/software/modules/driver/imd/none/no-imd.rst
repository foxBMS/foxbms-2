.. include:: ./../../../../../macros.txt
.. include:: ./../../../../../units.txt

.. _DUMMY_INSULATION_MEASUREMENT_DEVICE:

Dummy Insulation Measurement Device
===================================

Module Files
------------

Driver
^^^^^^

- ``src/app/driver/imd/none/no-imd.c`` (`API <../../../../_static/doxygen/src/html/no__imd_8c.html>`__, `source <../../../../_static/doxygen/src/html/no__imd_8c_source.html>`__)
- ``src/app/driver/imd/none/no-imd.h`` (`API <../../../../_static/doxygen/src/html/no__imd_8h.html>`__, `source <../../../../_static/doxygen/src/html/no__imd_8h_source.html>`__)

Configuration
^^^^^^^^^^^^^

*none*

Unit Test
^^^^^^^^^

- ``tests/unit/app/driver/imd/none/test_no-imd.c`` (`API <../../../../_static/doxygen/tests/html/test__no__imd_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__no__imd_8c_source.html>`__)

Description
-----------

The insulation measurement is always set to be valid and an insulation
resistance of :math:`10000000\ k{\Omega}`  is written to the database entry.
