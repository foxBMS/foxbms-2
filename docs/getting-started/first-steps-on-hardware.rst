.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _FIRST_STEPS_ON_HARDWARE:

First Steps on Hardware
=======================

This section gives a brief walk-through on setting up a debugger connection to
a |bms-master|.
Aim of this section is to check that all crucial parts of the toolchain work
and to recap the process of setting up the toolchain.

.. note::

    In order to be able to develop software for the BMS and deploy it to the
    embedded target, it is absolutely necessary to have these three
    components in place:

        * the embedded device itself,
        * a power supply,
        * a debugger.

Setup
-----

It is assumed, that all required software has been installed.
Details on the software installation can be found in
:ref:`SOFTWARE_INSTALLATION`.
Most notable parts of the toolchain are:

  * the conda environment,
  * the compiler toolchain,
  * and a debugger.

Compilation
-----------

Details on this step are described in :ref:`BUILDING_THE_APPLICATION`.
The following steps should pass and result in the application being built.

.. code-block:: console

     C:\Users\vulpes\Documents\foxbms-2>waf configure build_bin

If any error messages occur, they have to be investigated before continuing.

Hardware setup
--------------

For this first setup, it is enough to connect a power supply and a debug
connection to the |bms-master|.
Optionally, a CAN-interface can be connected to the CAN1 connector.
Details on the position and type of connectors can be found in
:ref:`CONNECTORS`.

Debug session
-------------

Details on setting up the debugger toolchain can be found in
:ref:`DEBUGGING_THE_APPLICATION`.

The following steps go through the most important parts of the workflow.
If they work as expected, it can be assumed that the debugger toolchain is
functional.

  * Connect the debugger to the target.
  * Supply power to the target.
  * Attach the debugger in the debug tool to the target and download the
    built binary to the target.
  * Start the target.

After these steps, it should be possible to halt and re-start the target.
The variable ``os_timer`` should show incremented values when halting the
target.
Central data structures such as ``bms_state``, ``sys_state``,
``cont_contactorStates`` and ``ltc_cellVoltage`` (for
:ref:`AFEs <ANALOG_FRONT_END_API>` of the LTC family).
