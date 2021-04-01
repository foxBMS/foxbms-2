.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _SOA_MODULE:

SOA Module
==========

Module Files
------------

Driver
^^^^^^

- ``src/app/application/soa/soa.c`` (`API <../../../../_static/doxygen/src/html/soa_8c.html>`__, `source <../../../../_static/doxygen/src/html/soa_8c_source.html>`__)
- ``src/app/application/soa/soa.h`` (`API <../../../../_static/doxygen/src/html/soa_8h.html>`__, `source <../../../../_static/doxygen/src/html/soa_8h_source.html>`__)

Configuration
^^^^^^^^^^^^^

- ``src/app/application/config/soa_cfg.c`` (`API <../../../../_static/doxygen/src/html/soa__cfg_8c.html>`__, `source <../../../../_static/doxygen/src/html/soa__cfg_8c_source.html>`__)
- ``src/app/application/config/soa_cfg.h`` (`API <../../../../_static/doxygen/src/html/soa__cfg_8h.html>`__, `source <../../../../_static/doxygen/src/html/soa__cfg_8h_source.html>`__)

Unit Test
^^^^^^^^^
- ``tests/unit/app/application/soa/test_soa_cfg.c``  (`API <../../../../_static/doxygen/tests/html/test__soa__cfg_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__soa__cfg_8c_source.html>`__)
- ``tests/unit/app/application/soa/test_soa.c``  (`API <../../../../_static/doxygen/tests/html/test__soa_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__soa_8c_source.html>`__)

Detailed Description
--------------------

Keeping the battery cells in the safe operating area (|soa|) is a safety goal of
the BMS. The |soa| includes cell voltage, cell temperature and battery current.

|foxbms| provides three different error levels. The first error level is called
Maximum operating limit (|mol|), the second error level is called Recommended
Safety Limit (|rsl|) and the third error level is called Maximum Safety Limit
(|msl|). The first two error levels indicate that a parameter is reaching the
limits of the recommended operating area and counter measures should be
initiated to prevent an unwanted opening of the contactors. A violation of a
|msl| means the safety of the system and the persons cannot be guaranteed anymore
and leads to an opening of the contactors.
