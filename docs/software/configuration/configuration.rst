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
This configuration is done through a software called |halcogen|.
This is described in :ref:`HAL_CONFIGURATION`.

Another point is that |foxbms| is a development platform.
As such, many parts of the code must be configured to fit the target
application (e.g., which monitoring IC is used to monitor the battery cells).
This is explained in :ref:`BMS_APPLICATION`.

.. _HAL_CONFIGURATION:

|halcogen|
----------

|halcogen| is a graphic user interface used to configure the HAL sources.
It generates sources in form of ``.h`` and ``.c`` and ``.asm`` files.
These HAL sources are generated based on the |halcogen| configuration files
(``*.hcg`` and ``*.dil``).
|foxbms| uses the waf tool :ref:`WAF_TOOL_HALCOGEN` to automatically run
|halcogen| and create the required sources.
Additional information on the tool can be found in
:ref:`HALCOGEN_TOOL_DOCUMENTATION`.

.. note::

   In some cases it might be beneficial to **not** generate the HAL during the
   build step and instead use a generated version of the HAL.
   For this use case see :ref:`HOW_TO_USE_GENERATED_SOURCES_FROM_HALCOGEN`.

|halcogen| ships with its own version of |freertos| and generates the
corresponding sources when running the code generator.
As |foxbms| uses its different own copy of |freertos|, the generated |freertos|
files from |halcogen| are removed after the code generator has run.

|halcogen| creates the source file ``HL_sys_startup.c`` which implements
(a weak implementation of) the function ``_c_int00`` (the system's startup
routine). |foxbms| provides its own **non**-weak implementation of ``_c_int00``
in ``fstartup.c``.
The |foxbms| implementation of ``_c_int00`` must be coupled to the the current
|halcogen| configuration.
Most changes in the |halcogen| project do not alter the startup behavior and
no further action needs to be taken into account.
However there are settings that alter the startup behavior.
Such settings need to be ported to ``fstartup.c`` as this non-weak
implementation of ``_c_int00`` outweighs the generated, new version of
``_c_int00`` in ``HL_sys_startup.c``.
Otherwise the startup function used by |foxbms| would not reflect the
|halcogen| configuration.
The :ref:`WAF_TOOL_HALCOGEN` provides a mechanism to detected such changes.
The hash of the current ``HL_sys_startup.c`` implementation is stored in
``src/hal/startup.hash`` and compared to the actual hash of the generated
``HL_sys_startup.c`` file.
If these are not the same, the build aborts with the following message:

.. literalinclude:: fstartup.c-check.txt
   :language: console
   :caption: Example error message on hash mismatch of ``HL_sys_startup.c``
   :name: halcogen-configuration-error

The build aborts as the expected hash is
``b'e2e61496edd65f44d7cc811b504ad1f2'`` while the actual hash is
``b'1something-other234'``.
Next, the function ``_c_int00`` in the two files (``fstartup.c``) and
``HL_sys_startup.c`` needs to be compared by the developer and the developer
needs to update the ``_c_int00`` implementation in the file ``fstartup.c`` to
reflect the |halcogen| startup routine.
The concluding step is to update the hash value in ``src/hal/startup.hash``
with ``1something-other``.
Now the build toolchain knows, that the changes applied in the |halcogen| are
reflected in the dependencies and the build will not abort after the HAL
sources are generated.

The process is illustrated in :numref:`halcogen-configuration`.

   .. figure:: img/halcogen_configuration.png
      :alt: HALCoGen configuration
      :name: halcogen-configuration
      :width: 100 %
      :align: center

      HALCoGen configuration change detection process


Enabling Cache
^^^^^^^^^^^^^^

The |tms570lc4357| can use cache to improve performance.

- Since |foxbms| v1.3.0, cache is enabled by setting the define
  ``OS_ENABLE_CACHE`` in ``src/app/task/os/os.h`` to ``true``.
  **Therefore, the configuration setting in HALCoGen is ignored**.
  However, as the startup code generated in ``HL_sys_startup.c`` by HALCoGen
  does still change (the line to activate cache is added), consequently the
  hash in ``src/hal/startup.hash`` still needs to be updated, but no further
  changes need to be applied.
  The process to enable cache is then as follows:

  - Enable cache by setting ``OS_ENABLE_CACHE`` to ``true`` in
    ``src/app/task/os/os.h``.
  - Update the file hash in ``src/hal/startup.hash``

- Before |foxbms| v1.3.0, enabling cache relied on the HALCoGen configuration
  and that this setting altered the startup code in ``HL_sys_startup.c``.
  The cache enable setting is found at setting ``TMS570LC4357ZWT_FREERTOS``,
  sub-setting ``R5-MPU-PMU`` and then the configuration ``Cortex-R5``,
  sub-configuration ``General Configuration`` : *Enable Cache*.
  The process to enable cache is then as follows:

  - Enable cache in HALCoGen
  - Update the startup code in ``fstartup.c`` with the changes from
    ``HL_sys_startup.c``.
  - Update the file hash in ``src/hal/startup.hash``

.. _BMS_APPLICATION:

BMS Application
---------------

The project provides two very basic configuration options:

#. general options in ``conf/bms/bms.json``
#. compiler options in ``conf/cc/cc-options.yaml`` (path is an option, see
   :py:meth:`f_ti_arm_cgt.options`) and compiler
   remarks and remark severity level in ``conf/cc/remarks.txt``

Furthermore, the used battery cells and the general configuration of the
battery system that is built up need to be defined:

- for the used battery cell:

  - ``src/app/application/config/battery_cell_cfg.c``
  -  ``src/app/application/config/battery_cell_cfg.h``

- for the top level view on the battery system:

  - ``src/app/application/config/battery_system_cfg.c``
  - ``src/app/application/config/battery_system_cfg.h``

However, the actual behavior of the battery system in the target application is
highly dependent on the target application and can therefore not simply be
configured through some switches.
This needs to be implemented in e.g., the
`BMS_Trigger() <./../../_static/doxygen/src/html/bms_8c.html#a954a05b20669dc00663c4f0f2a729fcd>`__
function.
It is up to the developer to familiarize with the hardware, code and
documentation and adapt the source code to the application specific
requirements.

General Options
^^^^^^^^^^^^^^^

Some BMS configurations require compiling different sources.
That applies to the operating system and the Analog Front-End.

.. note::

   Only very basic configurations can be changed via these options described
   here.
   Again, everything not mentioned here must still be *configured* by
   programming the application behavior in the source code.

Operating System
""""""""""""""""

The operating system is configured in ``conf/bms/bms.json``.
The value for ``os`` must be the name of the source directory in ``src/os/``
that includes the operating system sources.
Currently only |freertos| is supported (option: ``"os": "freertos"``).

Analog Front-End
""""""""""""""""

The AFE is configured in ``conf/bms/bms.json``.
The joint path from the values of ``manufacturer`` and ``ic`` must be the
name of the source directory in ``src/app/driver/afe/<manufacturer>/<ic>``
that includes the driver implementation.
A list of supported ICs is found in :numref:`SUPPORTED_ANALOG_FRONT_ENDS`.

The build process behind this configuration is documented at
:ref:`afe_library_build`.

Balancing Strategy
""""""""""""""""""

|foxbms| supports three different balancing strategies:

- Voltage-based balancing: Cell balancing based on voltage differences
  (key-value: ``voltage``). Details are found in
  :ref:`BALANCING_MODULE_VOLTAGE_BASED_BALANCING`
- History-based balancing: Cell balancing based on voltage history
  (key-value: ``history``). Details are found in
  :ref:`BALANCING_MODULE_HISTORY_BASED_BALANCING`
- No balancing: No balancing of any cell (key-value: ``none``). Details are
  found in   :ref:`BALANCING_MODULE_NO_BALANCING`

Compiler and Linker Options and Remarks
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

All options from ``conf/cc/cc-options.yaml`` are passed verbatim into the
build process. Compiler options are set during configuration time, that means
changing values in ``conf/cc/cc-options.yaml`` needs to be followed by
``waf configure``.

See the TI compiler manual before changing the flags in
``conf/cc/cc-options.yaml``.
The details of the project compiler configuration are described in
:ref:`COMPILER_CONFIGURATION`.

Basic Battery Cell And System Configuration
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Battery Cell Configuration
""""""""""""""""""""""""""

The basic parameters of the used battery cell of the battery system are defined
in:

- ``src/app/application/config/battery_cell_cfg.c`` `🔗 <./../../_static/doxygen/src/html/battery__cell__cfg_8c.html>`__
- ``src/app/application/config/battery_cell_cfg.h`` `🔗 <./../../_static/doxygen/src/html/battery__cell__cfg_8h.html>`__

The following parameters need to be defined (the links lead to the doxygen
documentation for the explanation of the specific parameter) in
``src/app/application/config/battery_cell_cfg.h``:

.. csv-table::
   :name: battery-cell-configuration
   :header-rows: 1
   :delim: ;
   :file: ./battery-cell-configuration.csv

The lookup tables for the state of charge ``bc_stateOfChargeLookupTable``
and for the state of energy ``bc_stateOfEnergyLookupTable`` need to be defined
in ``src/app/application/config/battery_cell_cfg.c``.

Battery System Configuration
""""""""""""""""""""""""""""

The basic, top level view on the battery system configuration is defined at:

- ``src/app/application/config/battery_system_cfg.c`` `🔗 <./../../_static/doxygen/src/html/battery__system__cfg_8c.html>`__
- ``src/app/application/config/battery_system_cfg.h`` `🔗 <./../../_static/doxygen/src/html/battery__system__cfg_8h.html>`__

.. csv-table::
   :name: battery-system-configuration
   :header-rows: 1
   :delim: ;
   :file: ./battery-system-configuration.csv


The symbolic names of the individual strings needs to be adapted in
``BS_STRING_ID_e`` to the actual number of strings in the battery system.

The configuration array ``bs_stringsWithPrecharge`` need to define whether a
string can used for precharging or not.
This configuration array uses of the entries in ``BS_STRING_ID_e`` for the
assignment.
