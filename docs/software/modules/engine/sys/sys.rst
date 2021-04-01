.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _SYSTEM_MODULE:

System Module
=============

Module Files
------------

Driver
^^^^^^

- ``src/app/engine/sys/sys.c`` (`API <../../../../_static/doxygen/src/html/sys_8c.html>`__, `source <../../../../_static/doxygen/src/html/sys_8c_source.html>`__)
- ``src/app/engine/sys/sys.h`` (`API <../../../../_static/doxygen/src/html/sys_8h.html>`__, `source <../../../../_static/doxygen/src/html/sys_8h_source.html>`__)

Configuration
^^^^^^^^^^^^^

- ``src/app/engine/config/sys_cfg.c`` (`API <../../../../_static/doxygen/src/html/sys__cfg_8c.html>`__, `source <../../../../_static/doxygen/src/html/sys__cfg_8c_source.html>`__)
- ``src/app/engine/config/sys_cfg.h`` (`API <../../../../_static/doxygen/src/html/sys__cfg_8h.html>`__, `source <../../../../_static/doxygen/src/html/sys__cfg_8h_source.html>`__)

Unit Test
^^^^^^^^^

- ``tests/unit/app/engine/config/test_sys_cfg.c`` (`API <../../../../_static/doxygen/tests/html/test__sys__cfg_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__sys__cfg_8c_source.html>`__)
- ``tests/unit/app/engine/sys/test_sys.c`` (`API <../../../../_static/doxygen/tests/html/test__sys_8c.html>`__, `source <../../../../_static/doxygen/tests/html/test__sys_8c_source.html>`__)

Detailed Description
--------------------

After all initialization steps are successfully run, the system driver is
in initialization operation mode.

The top level state diagram of the system state machine is shown in
:numref:`sys-state-machine-diagram-top-view`.

The detailed state diagram of the system state machine is shown in
:numref:`sys-state-machine-diagram-detailed`.

.. graphviz:: sys_state-machine-top-view.dot
    :caption: Top level view of the system state machine
    :name: sys-state-machine-diagram-top-view

.. graphviz:: sys_state-machine-detailed.dot
    :caption: Detailed view of the system state machine
    :name: sys-state-machine-diagram-detailed
