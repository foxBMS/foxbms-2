.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _CONFIGURATION:

Configuration
=============

|foxbms| is an embedded system and some parts of the code are near to the
hardware level.
The code relies on a Hardware Abstraction Layer (HAL), which is a set of
functions and definitions acting as an interface between the application and
the underlying hardware.
The HAL must be configured to fit the application and the hardware the software
is running on (e.g., configure IO pins for communication with the outside
world).
This configuration is done with software called |ti-halcogen|.
This is described in :ref:`HAL_CONFIGURATION`.

Another point is that |foxbms| is a development platform.
As such, many parts of the code must be configured to fit the target
application (e.g., which monitoring IC is used to monitor the battery cells).
This is explained in :ref:`BMS_APPLICATION_CONFIGURATION`.

.. _HAL_CONFIGURATION:

|ti-halcogen|
-------------

|ti-halcogen| is a graphical user interface used to configure the HAL sources.
It generates sources in the form of ``.h``, ``.c`` and ``.asm`` files.
These HAL sources are generated based on the |ti-halcogen| configuration files
(``*.hcg`` and ``*.dil``).
The respective configuration files are found

- for the application at:

  - ``conf/hcg/app.hcg``
  - ``conf/hcg/app.dil``
- for the bootloader at:

  - ``conf/hcg/bootloader.hcg``
  - ``conf/hcg/bootloader.dil``


|foxbms| uses the Waf tool
:doc:`/tools/waf_tools/waf_tools_autosummary/waf_tools.hcg`
to automatically run |ti-halcogen| and create the required sources.
Additional information on the tool can be found in
:ref:`TI_HALCOGEN_TOOL`.

.. note::

   In some cases it might be beneficial to **not** generate the HAL during the
   build step and instead use a generated version of the HAL.
   For this use case see :ref:`HOW_TO_USE_GENERATED_SOURCES_FROM_HALCOGEN`.

|ti-halcogen| ships with its own version of |freertos| and generates the
corresponding sources when running the code generator.
As |foxbms| uses its own copy of |freertos|, the generated |freertos|
files from |ti-halcogen| are removed after the code generator has run.

Furthermore, generated sources listed in ``conf/hcg/app-remove.yml``
(application) and ``conf/hcg/bootloader-remove.yml`` (bootloader) are removed
by the build integration.

The startup source check uses target-specific hash files:

- ``src/app/hal/app-startup.hash`` for the application build
- ``src/bootloader/hal/bootloader-startup.hash`` for the bootloader build

Enabling Cache
^^^^^^^^^^^^^^

The |ti-tms570lc4357| can use cache to improve performance.

- From |foxbms| v1.3.0 to v1.11.0, cache is enabled by setting the define
  ``OS_ENABLE_CACHE`` in ``src/app/task/os/os.h`` to ``true``.
  In newer versions than v1.11.0, cache is enabled by setting
  ``mcu:↳use-cache:`` to ``true`` in ``conf/bms/bms.json``.
  **Therefore, the configuration setting in HALCoGen is ignored**.
  However, as the startup code generated in ``HL_sys_startup.c`` by |ti-halcogen|
  does still change (the line to activate cache is added), consequently the
  hash in ``src/app/hal/app-startup.hash`` still needs to be updated, but no
  further changes need to be applied.
  The process to enable cache is then as follows:

  - Enable cache by applying the cache setting dependent on the |foxbms|
    version.
  - Update the file hash in ``src/app/hal/app-startup.hash``

- Before |foxbms| v1.3.0, enabling cache relied on the |ti-halcogen|
  configuration and that this setting altered the startup code in
  ``HL_sys_startup.c``.
  The cache enable setting is found at setting ``TMS570LC4357ZWT_FREERTOS``,
  sub-setting ``R5-MPU-PMU`` and then the configuration ``Cortex-R5``,
  sub-configuration ``General Configuration`` : *Enable Cache*.
  The process to enable cache is then as follows:

  - Enable cache in |ti-halcogen|
  - Update the startup code in ``fstartup.c`` with the changes from
    ``HL_sys_startup.c``.
  - Update the file hash in ``src/app/hal/app-startup.hash``

.. _BMS_APPLICATION_CONFIGURATION:

BMS Application
---------------

The project provides two very basic configuration options:

#. general options in |bms-config-file|.
   This file is case-sensitive and all settings need to be in lower-case.

Furthermore, the used battery cells and the general configuration of the
battery system that is built up need to be defined:

- for the used battery cell:

  - ``conf/bms/battery_cell_cfg.json``

- for the top level view on the battery system:

  - ``conf/bms/battery_system_cfg.json``

However, the actual behavior of the battery system in the target application is
highly dependent on the target application and can therefore not simply be
configured through some switches.
This needs to be implemented in e.g., the ``BMS_Trigger()`` function.
It is up to the developer to familiarize with the hardware, code and
documentation and adapt the source code to the application-specific
requirements.

General Options
^^^^^^^^^^^^^^^

Some BMS configurations require compiling different sources.
That applies to the operating system and the Analog Front-End.

.. note::

   Only very basic configurations can be changed via these options described
   here.
   Again, everything not mentioned here must still be **configured** by
   **programming** the application behavior in the source code.

The main keys

- ``application``,
- ``bms-slave``,
- ``debug`` (optional), and
- ``rtos``.

are described in the following subsections.

.. _CONFIGURE_APPLICATION:

``application``
"""""""""""""""

``application:↳algorithm``
''''''''''''''''''''''''''

The ``algorithms`` key requires one key: ``state-estimation``.

Supported settings:

- ``application:↳algorithm:↳state-estimation:↳soc``: ``"counting"``,
  ``"debug"``, or ``null`` to disable the SOC algorithm
- ``application:↳algorithm:↳state-estimation:↳soe``: ``"counting"``,
  ``"debug"``, or ``null`` to disable the SOE algorithm
- ``application:↳algorithm:↳state-estimation:↳sof``: ``"trapezoid"``
- ``application:↳algorithm:↳state-estimation:↳soh``: ``"debug"``, or ``null``
  to disable the SOH algorithm

``application:↳balancing-strategy``
'''''''''''''''''''''''''''''''''''

Supported settings:

- ``"voltage"``: Voltage-based balancing: Cell balancing based on voltage
  differences.
  Details are found in :ref:`BALANCING_APPLICATION_VOLTAGE_BASED_BALANCING`
- ``"history"``: History-based balancing: Cell balancing based on voltage
  history.
  Details are found in :ref:`BALANCING_APPLICATION_HISTORY_BASED_BALANCING`
- ``null``: No balancing of any cell.
  Details are found in :ref:`BALANCING_APPLICATION_NO_BALANCING`

``application:↳current-sensor``
'''''''''''''''''''''''''''''''

Currently only CAN-based current sensors are supported.

The manufacturer, model and the type of connection of the Insulation Monitoring
Device must be provided:

- ``application:↳current-sensor:↳type``: must be ``"can"``
- ``application:↳current-sensor:↳manufacturer``
- ``application:↳current-sensor:↳model``

Supported devices:

- ``isabellenhuette``: ``ivt-s``
- ``lem``: ``cab500``

``application:↳insulation-monitoring-device``
'''''''''''''''''''''''''''''''''''''''''''''

In case no Insulation Monitoring Device is used, the entire key
``application:↳insulation-monitoring-device``
needs to be removed.

The manufacturer, model and the type of connection of the Insulation Monitoring
Device must be provided:

- ``application:↳insulation-monitoring-device:↳type`` (either ``can`` or
  ``pwm``)
- ``application:↳insulation-monitoring-device:↳manufacturer``
- ``application:↳insulation-monitoring-device:↳model``

Supported devices:

- ``bender``: ``ir155``, ``iso165c``

Developer notes:

- The joint path from the values of ``manufacturer`` and ``model`` must be the
  name of the source directory in ``src/app/driver/imd/<manufacturer>/<model>``
  that includes the driver implementation.
- Every Insulation Monitoring Device needs to provide its include directories
  via a Waf target named ``{ip}imd-<manufacturer>_<model>`` using
  ``export_includes`` in the corresponding ``wscript`` file at
  ``src/app/driver/imd/<manufacturer>/<model>/wscript``.

.. _CONFIGURE_BMS_SLAVE:

``bms-slave``
"""""""""""""

``bms-slave`` requires defining ``analog-front-end`` and
``temperature-sensor``.

- ``analog-front-end``: The manufacturer and the IC must be provided.
  Valid settings are:

  - ``adi``

    - ``ades1830``

  - ``debug``

    - ``can``
    - ``default``

  - ``ltc``

    - ``6804-1``
    - ``6806``
    - ``6811-1``
    - ``6812-1``
    - ``6813-1``

  - ``maxim:``

    - ``max17852``

  - ``nxp``

    - ``mc33775a``

  - ``st``

  - ``ti``

    - ``dummy``


- ``temperature-sensor``: The manufacturer, model and the evaluation method of
  the temperature sensor must be provided.



The list of supported ICs is also found in
:numref:`SUPPORTED_ANALOG_FRONT_ENDS`.

Developer notes:

- The joint path from the values of ``manufacturer`` and ``ic`` must be the
  name of the source directory in ``src/app/driver/afe/<manufacturer>/<ic>``
  that includes the driver implementation.
- Every AFE needs to provide its include directories via a Waf target named
  ``{ip}afe-<manufacturer>_<ic>`` using ``export_includes``
  in the corresponding ``wscript`` file at
  ``src/app/driver/afe/<manufacturer>/<ic>/wscript``.
- The build process behind this configuration is documented at
  :ref:`afe_library_build`.

.. _CONFIGURE_DEBUG:

``debug``
"""""""""

Some features are only useful when debugging, and should not be enabled in the
final application build.
|uart| can be added as a debug interface as follows
(``debug:↳interfaces:↳["uart"]``).

.. _CONFIGURE_OS:

``rtos``
""""""""

The RTOS name must be defined in ``rtos:↳name:"<name>"``, where ``<name>`` is
the name of desired operating system.
Currently only |freertos| is supported (``rtos:↳name:"freertos"``).

|freertos| supports TCP/IP when the following TCP/IP addon is
enabled by adding ``freertos-plus-tcp`` to the addon list
(``rtos:↳addons:↳["freertos-plus-tcp"]``).

Developer notes:

- The name must correspond to the directory in source tree where the RTOS
  sources are (e.g., for ``freertos`` it needs to be at ``src/os/freertos``)
- The value for ``os`` must be the name of the source directory in ``src/os/``
  that includes the operating system sources.
- Every RTOS needs to provide its include directories via a Waf target named
  ``{ip}rtos-<kernel|tcp>`` using ``export_includes`` in the
  corresponding ``wscript`` file at ``src/os/freertos/<name>/wscript``.

Compiler and Linker Options and Remarks
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The details of the project compiler configuration are described in
:ref:`COMPILER_CONFIGURATION`.

Basic Battery Cell And System Configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Battery Cell Configuration
""""""""""""""""""""""""""

The basic parameters of the used battery cell of the battery system are defined
in:

- ``conf/bms/battery_cell_cfg.json``

The following parameters need to be defined (the links lead to the doxygen
documentation for the explanation of the specific parameter) in
``conf/bms/battery_cell_cfg.json``:

.. csv-table::
   :name: battery-cell-configuration
   :header-rows: 1
   :delim: ;
   :file: ../../../build/docs/docs/software/configuration/battery_cell_cfg.csv

The state of charge lookup table ``bc_stateOfChargeLookupTable`` is defined in
``conf/bms/battery_cell_soc_lookup-table.csv`` and
the state of energy lookup table ``bc_stateOfEnergyLookupTable`` is defined in
``conf/bms/battery_cell_soe_lookup-table.csv``.

Battery System Configuration
""""""""""""""""""""""""""""

The basic, top level view on the battery system configuration is defined at:

- ``conf/bms/battery_system_cfg.json``
- ``src/app/application/config/bms-slave_cfg.h``

.. csv-table::
   :name: battery-system-configuration
   :header-rows: 1
   :delim: ;
   :file: ../../../build/docs/docs/software/configuration/battery_system_cfg.csv

The symbolic names of the individual strings need to be adapted in
``BS_STRING_ID_e`` to the actual number of strings in the battery system.

The configuration array ``bs_stringsWithPrecharge`` needs to define whether a
string can be used for precharging or not.
This configuration array uses the entries in ``BS_STRING_ID_e`` for the
assignment.
