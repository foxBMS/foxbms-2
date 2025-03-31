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

- the Python `venv` environment,
- the compiler toolchain,
- and a debugger.

Compilation
-----------

Details on this step are described in :ref:`BUILDING_THE_APPLICATION`.
The following steps should pass and result in the application being built.

.. note::

   |ti-halcogen| is not available on Linux, and therefore the code generator
   tool can not run on Linux.
   Therefore, in order to build on Linux, the build process needs to be run on
   Windows first, and then the entire directory
   ``build/app_embedded/src/app/hal`` needs to be copied to the Linux machine.
   These files should be converted to LF.
   After that Linux builds are possible.
   This procedure needs to be repeated everytime the HAL configuration files
   (``conf/hcg/app.hcg``, ``conf/hcg/app.dil``) are changed.

.. tabs::

  .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

        .\fox.ps1 waf configure build_app_embedded

  .. group-tab:: Win32/Git bash

      .. code-block:: shell

        ./fox.ps1 waf configure build_app_embedded

  .. group-tab:: Linux

      .. code-block:: shell

        ./fox.ps1 waf configure build_app_embedded

If any error messages occurs, they have to be investigated before continuing.

.. _hardware_setup:

Hardware Setup
--------------

For this first setup, it is enough to connect a power supply and a debugger to
the |bms-master|.
Optionally, a CAN-interface can be connected to the CAN1 connector.
To debug or flash the binary file into the hardware, a debugger must also
be connected to the |bms-master|.
The connection is shown in :numref:`bms-master-connectors` and the
accompanying :numref:`bms-master-connectors-description`.
More details on the position and type of connectors can be found in
:ref:`CONNECTORS`.

.. drawio-figure:: ./img/bms-master-connectors.drawio
   :format: svg
   :alt: |bms-master| connectors
   :name: bms-master-connectors
   :width: 1040px

   |bms-master| connectors

.. csv-table:: |bms-master| connectors description
   :file: ./bms-master-connectors-description.csv
   :name: bms-master-connectors-description
   :header-rows: 1
   :delim: ;
   :widths: 10 45 45

.. _flash_the_application_using_the_debugger:

Flash/Debug the Application Using the Debugger
----------------------------------------------

Start the TRACE32 software, and click the **"Up" button (A)** to set the
|bms-master| to its "Up" state.
After that, click the **flash button (B)** to flash the |foxbms| application.

After the binary is successfully flashed, the user can start the application
and debug it using the features of the debugger software.
More details on setting up the debugger toolchain can be found in
:ref:`DEBUGGING_THE_APPLICATION`.
