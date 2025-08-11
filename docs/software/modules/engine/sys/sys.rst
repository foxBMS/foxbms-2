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

The state machine is shown in :numref:`state-machine-system-module`.

.. drawio-figure:: sys_state-machine.drawio
   :format: svg
   :alt: State machine of the System Module
   :name: state-machine-system-module
   :width: 512px

   State machine of the System Module
