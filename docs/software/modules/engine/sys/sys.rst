.. include:: ../../../../macros.txt
.. include:: ../../../../units.txt

.. _SYSTEM_MODULE:

System Module
=============

Module Files
------------

Driver
^^^^^^

- ``src/app/engine/sys/reset.c``
- ``src/app/engine/sys/reset.h``
- ``src/app/engine/sys/sys.c``
- ``src/app/engine/sys/sys.h``

Configuration
^^^^^^^^^^^^^

- ``src/app/engine/config/sys_cfg.c``
- ``src/app/engine/config/sys_cfg.h``

Unit Test
^^^^^^^^^

- ``tests/unit/app/engine/config/test_sys_cfg.c``
- ``tests/unit/app/engine/sys/test_sys.c``

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
