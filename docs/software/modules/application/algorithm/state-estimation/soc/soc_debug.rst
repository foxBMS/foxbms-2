.. include:: ./../../../../../../macros.txt
.. include:: ./../../../../../../units.txt

.. _SOC__DEBUG_DEFAULT:

SOC: Debug Default
==================

Module Files
------------

Driver
^^^^^^

- ``src/app/application/algorithm/state_estimation/soc/debug/soc_debug.c`` (`API <./../../../../../../_static/doxygen/src/html/soc__debug_8c.html>`__, `source <./../../../../../../_static/doxygen/src/html/soc__debug_8c_source.html>`__)
- ``src/app/application/algorithm/state_estimation/soc/debug/soc_debug.h`` (`API <./../../../../../../_static/doxygen/src/html/soc__debug_8h.html>`__, `source <./../../../../../../_static/doxygen/src/html/soc__debug_8h_source.html>`__)

Configuration
^^^^^^^^^^^^^

- ``src/app/application/algorithm/state_estimation/soc/debug/soc_debug_cfg.c`` (`API <./../../../../../../_static/doxygen/src/html/soc__debug__cfg_8c.html>`__, `source <./../../../../../../_static/doxygen/src/html/soc__debug__cfg_8c_source.html>`__)
- ``src/app/application/algorithm/state_estimation/soc/debug/soc_debug_cfg.h`` (`API <./../../../../../../_static/doxygen/src/html/soc__debug__cfg_8h.html>`__, `source <./../../../../../../_static/doxygen/src/html/soc__debug__cfg_8h_source.html>`__)

Unit Test
^^^^^^^^^

- ``tests/unit/app/application/algorithm/state_estimation/soc/debug/test_soc_debug.c`` (`API <./../../../../../../_static/doxygen/tests/html/test__soc__debug_8c.html>`__, `source <./../../../../../../_static/doxygen/tests/html/test__soc__debug_8c_source.html>`__)
- ``tests/unit/app/application/algorithm/state_estimation/soc/debug/test_soc_debug_cfg.c`` (`API <./../../../../../../_static/doxygen/tests/html/test__soc__debug__cfg_8c.html>`__, `source <./../../../../../../_static/doxygen/tests/html/test__soc__debug__cfg_8c_source.html>`__)

Detailed Description
--------------------

The debug implementation is a fake implementation that continuously outputs
a SOC of 50%.
