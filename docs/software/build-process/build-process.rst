.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _BUILD_PROCESS:

Build Process
=============

This section addresses relevant steps in the build process that are important
to know when interacting with it.

|configure|\ -Step
--------------------

During the configuration step (|waf| command |configure|), the build system
checks for the availability of the required tools and libraries.
For different build variants, different so-called *build environments* are
created.
This means, e.g., for a |ti-tms570|\ -based |bms-master| build, the
|ti-arm-cgt| toolchain must be installed, successfully configured, and then
this *build environment* can be used.
This is then the same for all other build variants, e.g., for the documentation
build |sphinx|, |doxygen| etc. must be installed.
If an environment is not available, but a build variant that requires it is
run, the build system will fail with an error message indicating the
missing environment.
The solution is then to install the missing tools for this environment
(:ref:`SOFTWARE_INSTALLATION`) and re-run the |configure| command.

|build|\ -Step
----------------

The |build|\ -Step performs the actual build of one variant.
All variants are defined in the dictionary ``VARIANT_CONFIGS`` in the top-level
``wscript``; the complete list is documented in :ref:`FOX_WAF` and can be shown
with ``waf --help``.

The following description uses the variant ``app_ti_arm_cgt``, the |foxbms|
application for the |ti-tms570|\ -based |bms-master|, as an example:

.. figure:: ../../../build/docs/docs/software/build-process/build-process_app_ti_arm_cgt.svg
   :name: build-process_app_ti_arm_cgt
   :alt: Steps performed by the build command of the variant app_ti_arm_cgt

   Steps that are performed when running ``waf build_app_ti_arm_cgt``

Preparation
^^^^^^^^^^^

Before any build task is created, the build system:

- selects the build environment ``ti_arm_cgt`` that has been created during the
  |configure|\ -Step,
- checks that the version information is consistent in all places where it is
  used,
- adds the command file ``conf/cc/remarks.txt`` to the compiler options of all
  targets on the variant, and
- recurses into the variant directory ``src``, where all targets are defined.

All artifacts are written into the variant-specific output directory
``build/app_ti_arm_cgt``, the sources in the repository are not modified.

Code Generation
^^^^^^^^^^^^^^^

The first tasks of the build generate sources and headers:

1. The |ti-halcogen| project ``conf/hcg/app.hcg`` (together with
   ``conf/hcg/app.dil``) is run to generate the HAL sources and headers into
   ``build/app_ti_arm_cgt/src/app/hal``.
2. The build configuration (``app_build_cfg.c``) and the version information
   (``version.c``) are generated from the current state of the repository,
   e.g., commit information, compiler version and build configuration.
3. The configuration file ``conf/bms/bms.json`` is validated and translated into
   the configuration headers ``foxbms_config_*.h`` (e.g., algorithm, balancing,
   strategy, current sensor, IMD, redundancy, RTOS, debug, |bms-slave|).
4. The battery cell and battery system configuration (``battery_cell_cfg.c``,
   ``battery_cell_cfg.h``, ``battery_system_cfg.h``) and the diagnosis entries
   (``diag_array_cfg.c``) are validated and generated.

Compiling and Archiving
^^^^^^^^^^^^^^^^^^^^^^^
..
   cspell:ignore armasm, armar

Afterwards, all C sources are compiled with ``armcl`` and all assembler sources
with ``armasm`` into object files.
Depending on the source group, different flags are applied, i.e. the
application sources, the HAL sources and the operating system sources use
different optimization and language settings.

The object files are then archived (``armar``) into one static library per
module, e.g., ``foxbms-hal.lib``, ``foxbms-driver.lib``.

Linking
^^^^^^^

These libraries and the remaining objects (e.g., ``main.c``, ``fstartup.c``,
``fassert.c`` and the generated sources) are linked into
``build/app_ti_arm_cgt/src/app/main/foxbms.elf`` using the linker script of the
app.
In addition to the binary itself, the linker emits the map file
``foxbms.elf.map`` and the link information ``foxbms.elf.xml``.

Post-Processing
^^^^^^^^^^^^^^^
..
   cspell:ignore armhex

Based on the linked binary, the following artifacts are created:

- The CRC-64 signatures of the program are calculated and written to
  ``foxbms.crc64.csv`` and ``foxbms.crc64.json``.
- ``armhex`` creates the hex file ``foxbms.hex`` and the corresponding
  ``foxbms.hex.map`` based on the command file ``src/app/main/app_hex.cmd``.
- ``tiobj2bin`` creates the binary file ``foxbms.bin``.
- The debugger script ``build/update_program_information.cmm`` is generated,
  so that the current build can be flashed and verified with Lauterbach.

Optional Outputs
^^^^^^^^^^^^^^^^

The build command accepts additional options:

``--preprocess-files``
    Additionally creates the preprocessor outputs of all C files, i.e. the
    preprocessed sources (``.pp``), the predefined macros (``.ppm``), the
    included files (``.ppi``) and the dependencies (``.ppd``).
``--generate-listings``
    Additionally creates the cross reference, function information and
    preprocessor listings of all C files.

Incremental and Clean Builds
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The build is incremental, i.e. only tasks whose inputs, command line or
dependencies have changed are re-run.
To remove all artifacts of the variant, or to inspect its tasks, the
accompanying commands can be used:

.. code-block:: console

    waf clean_app_ti_arm_cgt
    waf list_app_ti_arm_cgt



.. _afe_library_build:

Building the Analog Front-End Library
-------------------------------------

In order to easily switch between different AFEs the |foxbms| build
system implements a mechanic for swapping implementations through a
configuration file.
The configuration file is described in :numref:`BMS_APPLICATION`.

The build system will automatically select the correct driver files depending
on the configuration.

External Libraries
------------------

A How-to is found in
:ref:`HOW_TO_BUILD_A_LIBRARY_AND_LINK_IT_IN_A_FOXBMS_2_PROJECT`.
