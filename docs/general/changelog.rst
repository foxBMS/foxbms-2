.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _CHANGELOG:

#########
Changelog
#########

..
    Comments:
    Axivion is the company that builds the Axivion Bauhaus suite
    PEC stands for pack error check
    slaveplausibility is the old name of a plausibility module
    sym is the file extension of a symbol file
    JUnit is a test system (originally for Java)
    matcher as in `problem matcher`
    Cppcheck is a static program analysis tools
    xxxx is the dummy date for x.y.z

.. spelling::
    Axivion
    PEC
    slaveplausibility
    sym
    JUnit
    matcher
    Cppcheck
    xxxx


All notable changes to this project will be documented in this file.

|foxbms| uses the following versioning pattern ``MAJOR.MINOR.PATCH``
where ``MAJOR``, ``MINOR`` and ``PATCH`` are numbers.

Versioning follows then these rules:

- increasing ``MAJOR`` introduces significant changes, that require some
  larger work to update a project to this version.
- increasing ``MINOR`` introduces changes, that require less work than a major
  update and most of the work should be straight forward in order to update a
  project to this version.
- increasing ``PATCH`` introduces minor changes, that only require minor and
  straight forward work to update a project to this version.

********************
[1.5.1] - 2023-02-23
********************

|foxbms| now ships with a new conda environment ``2023-02-fennec-fox`` and the
local conda environment needs to be updated (see :numref:`conda_env_update`).

Added
=====

Changed
=======

- Update the static code analysis to Axivion 7.5.2 (from 7.5.0).
- Simplified the conda development environment.

Deprecated
==========

Removed
=======

Fixed
=====

- Renamed struct member `packVoltage_mV` to `stringVoltage_mV` for cell voltage
  database entry to adhere to our naming convention.

********************
[1.5.0] - 2023-02-03
********************

Added
=====

- Added hardware design files (e.g., schematics, layout, BOM and STEP file
  etc.) for the NXP MC33775A monitoring IC.
- Added driver for the PCF2131 RTC.
  The driver uses I2C to communicate with the IC.
- A basic GUI to interact with the |foxbms| via CAN from a PC.
- The software version and the die ID can be requested via CAN.
- The FRAM can now be initialized/reinitialized via CAN.
- Stub code for the software reset via CAN has been implemented.
- The precharge abort reason (voltage or current) is now evaluated and the
  result is transmitted via CAN.
- Now CAN messages with extended identifier can be transmitted and received.
  Four CAN mailboxes (61-63) are reserved for the reception of messages with
  extended identifiers on each CAN.
  The HALCoGen configuration for these mailboxes is overwritten during the
  initialization phase.
- The I2C driver API was reworked.
  There are now three types of communication:
  read, write and write followed by repeated start followed by read.
- The I2C communication now uses DMA and is made in a separate task.
- Basic documentation of defines that can be used to define the application
  behavior.

Changed
=======

- The default interface configuration for BMS-Slaves is the interface 1 on the
  |bms-interface|\ s:

  - for LTC-based interfaces: hardware chip select 1 on SPI 1
  - for NXP-based interfaces: hardware chip select 1 on SPI 1

- The chip select configuration has been unified.
  The previous implementation worked like this:

  - for software chip select the SPI configuration has been
    configured directly in the configuration struct by setting the member
    ``csPin`` to a certain pin and the member ``csType`` to
    ``SPI_CHIP_SELECT_SOFTWARE``in a variable of type
    ``SPI_INTERFACE_CONFIG_s``.
  - For hardware chip select, i.e., ``csType`` set to
    ``SPI_CHIP_SELECT_HARDWARE``, the configuration in ``csPin`` was ignored,
    and the pin configuration in the member ``pConfig`` was evaluated.

  Now, the SPI configuration is done by setting the chip select type via the
  member ``csType`` and the pin to be used via the member ``csPin`` in a
  variable of type ``SPI_INTERFACE_CONFIG_s``.
  Therefore there is no need anymore the adapt the ``spiDAT1_t`` configuration,
  when another chip select should be used.
- The members in `DATA_BLOCK_ERROR_STATE_s` have been renamed to better reflect
  what each entry actually indicates.
- The type of all members in `DATA_BLOCK_ERROR_STATE_s` have been changed to
  `bool`.
- The order of members in `DATA_BLOCK_ERROR_STATE_s` has been changed, so that
  the entries are logically grouped.
  This change is transparent, as long as the application does **not** relay on
  the memory layout.
- Changed data type for error flags in database entries to boolean.
- Applied the *include-what-you-use* paradigm to the |foxbms| sources.
  The :ref:`C_CODING_GUIDELINES` have been adapted accordingly and generally
  been improved.
- Added dedicated enum for error flags to .dbc respectively .sym file.
- Rewrote the CRC implementation for the LTC ICs from scratch and added
  unit tests for the new implementation.
- Update the static code analysis to Axivion 7.5.0 (from 7.4.6).
- Software version is now also saved in FRAM version struct. As this is The
  the first FRAM entry, this changes the memory layout and makes FRAM entries
  existing prior to this version invalid.
- A successful boot is now indicated via CAN by sending

  - the magic start sequence,
  - the software version information,
  - the MCU die ID,
  - the MCU lot number,
  - the MCU wafer number and
  - the magic end sequence.

  See ``src/app/engine/config/sys_cfg.c`` for implementation details.

- Updated the documentation section about Lauterbach debug probes.
- Removed define ``BS_MAX_SUPPORTED_CELLS`` from file ``battery_system_cfg.h``
  and moved the definition to the individual AFE implementations.
- Use ``float_t`` instead of ``float``.
- Aligned the hardware documentation for BMS-Master, BMS-Interface and
  BMS-Slave (see :ref:`MASTER_OVERVIEW`, :ref:`INTERFACES_OVERVIEW`, and
  :ref:`SLAVES_OVERVIEW`)
- Updated the documentation section about the |foxbms| platform
- The functionality to switch to user mode is now implemented as an inline
  function instead of a macro.
- The default optimization level is set to -O0 (see
  ``conf/cc/cc-options.yaml``).

Deprecated
==========

Removed
=======

Fixed
=====

- Fixed the hardware readme as it referenced the wrong |master| version.
- Fixed the regular expression for checking the validity of macro names.
- Fixed wrong configured chip select pin for MCU SBC. Chip select 1 instead of
  chip select was configured.

********************
[1.4.1] - 2022-10-27
********************

|foxbms| updated to TI ARM CGT 20.2.6.LTS.
Installation instructions are found at :numref:`css_install`.

Added
=====

- The new default compiler set in ``conf/env/paths_win32.txt`` and
  ``conf/env/paths_linux.txt`` is now TI ARM CGT v20.2.6.LTS (shipped with CCS
  ``12.0.0``).
- Added driver for *Murata NCU15XH103F6Sxx* temperature sensor

Changed
=======

- Updated the hardware design files (e.g., schematics, layout, BOM and STEP
  file etc.) of the |bms-master|  to ``v1.1.5`` (see
  :ref:`CHANGELOG_FOR_MASTER_TMS570_V1_1_5`).
- Updated documentation of the interface using the |max17841b| transceiver
  chip.
- The implementation of the CAN driver has been changed to simplify adding
  new messages.
  Furthermore it is now possible to force a certain style on how CAN messages
  are defined in the symbol/dbc files as well as in the source code.
  For further information see :ref:`HOW_TO_USE_THE_CAN_MODULE`.
- Updated version of ``.sym`` file for the definition of the CAN messages and
  signals to v6.0 (from 5.0).
- Update the static code analysis to Axivion 7.4.6 (from 7.4.0).

Deprecated
==========

Removed
=======

- Unused calculation of MOL, RSL and MSL curves for trapezoid algorithm in
  ``SOF`` module.

Fixed
=====

- In ``ADC`` module, corrected formula making the conversion from raw ADC
  reading to voltage in mV.
- The address of two diagnostic registers in the Smart Power Switch driver
  was wrong.
- Endianness definition throughout the CAN signals was inconsistent. Not all
  signals were correctly defined as big-endian.

********************
[1.4.0] - 2022-07-29
********************

Added
=====

- |foxbms| has been certified as open source hardware by the
  Open Source Hardware Association under the
  OSHWA UID `DE000128 <https://certification.oshwa.org/de000128.html>`_.
  This information has been added to the README and license documentation
  (see :ref:`OPEN_SOURCE_HARDWARE_CERTIFICATION`).
- Added Axivion configuration (see ``tests/axivion/addon``) to check for

  - source files to be decodable with a specific encoding
  - POSIX 3.206
  - filename uniqueness in the build process
  - file level doxygen comment
  - doxygen comments for extern and static variables, and typedefs
  - the |foxbms| license header
  - literal suffixes for numerical assignments
- Added test cases for the Axivion rules (see ``tests/axivion/addon-test``)
- The foxBMS GUI wrapper is now implemented as a module.
- Added a driver for the NXP MC33775A monitoring IC.
  The driver measures cell voltages, cell temperatures via an I2C multiplexer
  and controls cell balancing.

Changed
=======

- Updated the hardware design files (e.g., schematics, layout, BOM and STEP
  file etc.) of the |bms-master|  to ``v1.1.3`` (see
  :ref:`CHANGELOG_FOR_MASTER_TMS570_V1_1_3`).
- Function names are now checked by Axivion to conform to the guidelines.
- Use correct prefix in the

  - ``version`` module (:ref:`VERSION_MODULE`)
  - ``assert`` module (:ref:`FASSERT_MODULE`)
- The behavior of the ``build_bin`` command, which creates the binary, has been
  improved.
  If the BMS configuration file (``conf/bms/bms.json``) has been changed, the
  project needs to be reconfigured (by running ``waf configure``, see
  :ref:`BUILDING_THE_APPLICATION`).
- The following settings in BMS configuration file (``conf/bms/bms.json``) have
  been changed:

  - The AFE key for the IC has been renamed from ``chip`` to ``ic`` in the
    ``slave-unit/analog-front-end`` section.
  - The balancing strategy has been moved into the ``application`` section.
  - The operating system key has been renamed from ``operating-system`` to
    ``rtos``.
  - The SOF needs to be configured in the
    ``application/algorithm/state-estimation`` section.
    Currently ``"sof": "trapezoid"`` is the only valid option.

- Several variables and functions have been renamed to conform to the |foxbms|
  style guide (:ref:`STYLE_GUIDE`).
  There will be additional changes in future releases as the task of having
  a fully style guide conforming repository is an iterative and timely process.

Deprecated
==========

Removed
=======

Fixed
=====

- The macro ``SOC_STRING_CAPACITY_As`` was incorrectly braced, as it had less
  closing than opening braces
  (``src/app/application/algorithm/state_estimation/soc/counting/soc_counting_cfg.h``).
  This however did not reveal as bug, as the calling function fixed the
  incorrect bracing by adding the missing brace correctly
  (``src/app/application/algorithm/state_estimation/soc/counting/soc_counting.c``).
- The delay between chip select activation and start of SPI transmission and the
  delay between end of SPI transmission and chip select deactivation have been
  increased to avoid SPI transmission errors for AFEs.
- The CAN2 message boxes were not activated within the HALCoGen configuration.
- Fix several unit testing warnings due to a misconfiguration with respect to
  ``BS_NR_OF_STRINGS``.

********************
[1.3.0] - 2022-05-30
********************

|foxbms| updated to TI ARM CGT 20.2.5.LTS.
Installation instructions are found at :numref:`css_install`.

|foxbms| updated to LLVM 13.0.0.
Installation instructions are found at :numref:`llvm_install`.

Added
=====

- The new default compiler set in ``conf/env/paths_win32.txt`` and
  ``conf/env/paths_linux.txt`` is now TI ARM CGT v20.2.5.LTS (shipped with CCS
  ``11.0.0``).
- Add support for the environment variable ``FOXBMS_2_CCS_VERSION_STRICT`` to
  check for the exact CCS version.
- Added hardware specifications for the LTC based slaves to the documentation.
- Added a superimposed state machine for all insulation monitoring device
  drivers to control the subordinate, device specific driver implementations.
- Added driver implementation for Bender IR155 insulation monitoring device.
- Transmit the unique ID of the MCU on which |foxbms| is running on startup
  via CAN.
- Added a mechanism to the ``algorithm`` module that allows to reinitialize an
  algorithm.
- Added an API to the ``MCU`` module that allows to measure microseconds with
  the ``RTI`` module.
- Added a can helper that transforms a boolean to an integer value (fitting to
  the CAN messages).
- The Axivion configuration has been updated to ``7.2.3`` in the release
  ``1.1.2``, however the version setting in the configuration file has not.
  The versions of the Axivion Suite and the configuration files have been
  aligned to ``7.2.3``.
- Added the version information to the header section of all ``C`` source and
  header files.
- Added a function to compute CRCs with the MCU hardware.
- The ``FRAM`` module now writes a CRC of the data written to the FRAM. It
  also reads the stored CRC and compares it with the CRC calculated on the
  read data. This makes FRAM entries existing prior to this version invalid.
- A ``DIAG`` entry was added for FRAM errors.
- Added documentation for the ``FRAM`` and the ``CRC`` modules.
- Integrated interaction with the diagnostic module into the Maxim ``AFE``
  driver.
- Added information on debug adapters to the documentation.
- Documented the rule that pointer parameters to functions must indicate in
  their Doxygen documentation whether they are used as input, output or both.
- Added a continuously running task with same priority as the 1ms task.
  It is suspended by default if unused.
- Added documentation on the Maxim driver.

Changed
=======

- The linker step now uses ``--start-group`` and ``--end-group`` to re-read
  libraries until all symbols are resolved instead of ``--reread_libs``.
  This change simplifies the integration of the build into Axivion.
- |freertos| configuration is now validated in
  ``os_freertos_config-validation.h``.
- Updated the documentation of the of the LTC-based interface |ltc6820| version
  1.0.3 (see :ref:`INTERFACE_LTC_6820___V1_0_3__`).
- Updated LLVM to version 13.0.0.
- Updated the pinout documentation for the |bms-master| version 1.1.1
  (see :ref:`MASTER_TMS570___V1_1_1__`).
- Update the static code analysis to Axivion 7.4.0.
- Increased the size of the variable that describes stack sizes for |freertos|,
  allowing for the fully supported stack size of the port.
- The diagnosis entries for the ``LTC`` module have been renamed in order to
  reflect that they are relevant for all ``AFE`` implementations.
- The schema file for the ``bms.json`` configuration file has been split into
  several files for readability and |code| has been made aware of the schema.
- Added a style check that constant values should be on the right hand side of
  a comparison.
- Updated |freertos| to version 10.4.5 (from 10.4.3).
- Cleaned up the CAN callback for receiving commands.
- Extended the script for updating the doxygen headers in source files so that
  it fetches first and then compares to the tracking branch.
- Updated the documentation of the system monitoring module and implemented
  the recording of violations to the persistent memory.
- Split the definition of CAN callback functions into separate definitions for
  received and transmitted messages in order to better reflect the actual
  implementation and prevent wrong usage.
- Number of strings now configured to one by default.
- Renamed the ``FRAM`` functions to adhere to *verb-noun* style.
- Extended the Axivion configuration so that parameters without an assertion
  can be suppressed.
- The threshold above which balancing is started for a single cell has been
  made configurable by the |foxbms| CAN messages that are already defined.
- MCU specific defines have been moved from ``general.h`` to ``mcu.h``.
- The static assert macro ``static_assert`` has been renamed to
  ``f_static_assert`` to work around compiler limitations.
- The following defines have been renamed to be aligned with
  :ref:`NAMING_CONVENTIONS`:

  - ``BS_NR_OF_MODULES`` to ``BS_NR_OF_MODULES_PER_STRING``,
  - ``BS_NR_OF_CELLS_PER_MODULE`` to ``BS_NR_OF_CELL_BLOCKS_PER_MODULE`` and
  - ``BS_NR_OF_BAT_CELLS`` to ``BS_NR_OF_CELL_BLOCKS_PER_STRING``.
- Loop correctly over ``BS_NR_OF_MODULES_PER_STRING``,
  ``BS_NR_OF_CELL_BLOCKS_PER_MODULE`` and ``BS_NR_OF_CELL_BLOCKS_PER_STRING``.

- Function names are now checked by Axivion to conform to the guidelines.
- Improved the MISRA-C conformance of of the database module
  (:ref:`DATABASE_MODULE`).
- Fixed the pack current check in ``SOA_CheckCurrent``.
- Fixed LLVM install script to check for permissions before running the script.

Deprecated
==========

Removed
=======

- Removed unused function ``OS_SystemTickHandler()``.
- The Cppcheck support and its configuration files as well as the respective
  build and clean commands have been removed.
  The static program analysis of the embedded software is done by Axivion
  Bauhaus Suite (see :ref:`AXIVION_BAUHAUS_SUITE`).

Fixed
=====

- Fixed a bug where the ``update_doxygen_header.py`` script tried to
  ``git add`` files that did not exist.
- The HALCoGen SPI interface chip select configuration is overwritten and all
  hardware chip selects are disabled when using a software chip select.
  This causes all SPI communication using hardware chip selects to fail for
  this SPI interface.
  Now the SPI chip select register are correctly configured before starting a
  SPI transaction.
- Suppress a superfluous warning during unit test builds concerning the number
  of bytes per word (as this information is used for task sizing).
- Fixed "unsafe" macros that had not parentheses and added this violation to
  violations that fail the CI.
- Fixed undesired attempt of installation of git hooks even when the project
  was not stored in a git repository.
- Fixed import order in Python files.
- Fixed a static assertion in the Maxim driver (that prevented in some
  constellations compilation due to a logic error).
- Fixed a timing violation in the Maxim driver (a computationally costly
  operation has been moved out of the 1ms-task).
- Some defines where invisible to the preprocessor due to missing includes.
  This has been resolved and measures taken that this situation cannot return.
- The HALCoGen generated startup file ``HL_sys_startup.c`` does not need to be
  compiled as |foxbms| implements the startup in ``fstartup.c``.

********************
[1.2.1] - 2021-12-08
********************

|foxbms| now ships with a new conda environment ``2021-11-fennec-fox`` and the
local conda environment needs to be updated (see :numref:`conda_env_update`).

Added
=====

- Added enum typedef for GPIO pin state.
- Add driver for the temperature sensor *NTCLE317E4103SBA* from *Vishay*.
- Added documentation for Coulomb-counting and extended the documentation of
  the state-estimation-module.
- Added a define for ``NULL``.
- Make |git| a required software installation (see :ref:`git_install`).
- If a system-wide installation of |git| is not found in the build process,
  check for a user installation.
- Added an API to check whether a certain amount of milliseconds has passed.
- Added an API to the ``spi``-module that allows to check if the interface
  is available.
- Added a python script that can help with searching for regressions after
  resolving a complete violation category in Axivion.
- Automatically update the ``@updated`` doxygen comment before committing when
  pre-commit hooks are activated.
- Added documentation for the LTC based interface and slaves.
- In the ``spi``-module, hardware Chip Select pins can now be used.
  Until now only software Chip Select pins could be used.

Changed
=======

- Rewrote the linker script: the sections lengths are no longer hard-coded.
- The memory layout of ``OS_TASK_DEFINITION_s`` has been changed.
- The ``os``-module wrapper for task definition now also contains the
  ``pvParameters``.
- Static analysis cleanup of the ``os``-module and ``ftask``-module wrappers.
- Tasks are now marked as requiring FPU context, to prevent memory corruption
  during task switching.
- The Chip Select pin used to drive the external flash memory over SPI is now
  set as output with initial level high, corresponding to a default inactive
  state for the SPI line.
- Removed a hotfix that has been introduced in v1.0.0: the CAN driver used a
  critical section in order to avoid computation issues when scaling values.
  This has been shown to be connected with the FPU context and can now be
  removed as the root cause has been resolved.
- Updated the Axivion configuration to use version ``7.2.6``.
- Create the Axivion build directory before running the analysis.
- The documentation build now treats warnings as errors.
- Updated the doxygen configuration files to match the doxygen version that
  is shipped in the conda environment.
- Switched the used C standard to C11.
- The |freertos| specific implementations for the task and queue creation were
  moved into ``os_freertos.c`` and ``ftask_freertos.c``.
- Greatly enhanced the robustness and code quality of the driver for Maxim
  analog front ends.

Deprecated
==========

Removed
=======

Fixed
=====

- The error message when ``bms.json`` had an invalid configuration always
  indicated an invalid *analog front-end* configuration, independently of what
  option was actually invalid configured.
  The error message now correctly points to the setting, that sets an invalid
  configuration.
- Fixed several MISRA-C violations and style guide violations.
- Fixed the conda environment shell update script.
- The signature of the |freertos| tasks was not correct and has been fixed to
  match ``void TaskName(void *pvParameters)``.
- Explicitly cast the variables that are passed to the |freertos| task creation
  function.
- Make ``NULL_PTR`` explicitly *unsigned*.
- Fixed product number for the used trace probe in our test setup.
- Fixed incorrect values for GPIO measurement times in LTC module.
- Cleaned up ``spi``-module and added documentation.
  Removed unused enum (``SPI_INTERFACE_e``).

********************
[1.2.0] - 2021-10-21
********************

Added
=====

- Added stubs for the Bender IR155 insulation monitoring device driver (i.e.
  **driver is non-functional**).
- Extended the documentation with a render of the architecture against which
  the foxBMS implementation is checked.
- Added driver for I2C communication for MCU.
- Added driver for I2C port expander.
- Added driver for I2C humidity/temperature sensor.
- Implemented feedback through auxiliary contacts for the contactor driver.
- Debug LED is now toggled depending on system state (slow: okay, fast: error)
- Added an option to install a pre-commit hook in the repository.
  The pre-commit hook runs the guidelines check (see
  :ref:`WAF_TOOL_GIT_HOOKS`).
- Added a driver module that allows to use the enhanced PWM features of the
  MCU.
- Adapted CAN module to receive/transmit messages either via CAN1 or CAN2.
- Annotate maximum stack size in |freertos| so that debugger can catch this
  information.
- Updated the hardware design files (e.g., schematics, layout, BOM and STEP
  file etc.) of the |bms-master|  to ``v1.1.1``.
- Added information on citing the project in ``CFF`` format.
- Added driver for interlock.

Changed
=======

- The measurement IC (``MIC``) module has been renamed to Analog Front-End
  (``AFE``).
- ``build_all`` and ``clean_all`` need to run the Axivion related commands as
  last steps.
- The file content of the linker script for the elf file is now hashed using
  Unix-style line endings as this is more robust (e.g., downloaded zip files
  from GitHub releases).
- Sort the SPI formats so that there are no conflicts between the configuration
  on SPI1 and SPI4 (Both are connected to the interface connector).
- Control the pins of the interface for LTC AFEs with the port expander on the
  |foxbms| master board.
- The documentation build defaults now only to ``html`` and ``spelling``,
  .i.e., ``linkcheck`` has been removed.
- Increased stack size of 10ms task to 5120 bytes (from 4096).
- Declared large database entries as static, so that they are placed in the
  data segment to reduce the stack usage of the respective task.

Deprecated
==========

Removed
=======

- Removed unused code in main build script.

Fixed
=====

- Fixed variable name typos in ``moving_average.c``.
- A specific order of build/clean commands related to Axivion build and clean
  commands resulted in build errors.
  Now the build is early aborted if a wrong order is supplied and a
  descriptive error message is printed to stderr.
- The documentation included a wrong statement about how to configure
  |freertos|.
- Battery voltage is now transmitted correctly via CAN (ID: 0x222).

********************
[1.1.2] - 2021-09-03
********************

Added
=====

- A basic block diagram and description of a battery system and the voltages.
  and the currents that need to be measured have been included in the
  documentation.
- Add helper script to run the library test build
  (``tests/variants/lib-build/lib-build.bat``).
- Updated the Axivion configuration to use version ``7.2.3``.
- Added minimal documentation for Axivion setup.
- Improved the Axivion configuration:

  * use ``FAS_ASSERT`` as assert macro in order to be compliant with the style
    guide
  * fix some includes (library-inclusions and unnecessary inclusions)
  * adds the Axivion example for race condition analysis and a minimal
    configuration of entry points
  * updates ``.axivion.preinc`` with missing symbols
  * makes cafeCC point to the right compiler library
  * excludes vendored code from analysis
  * disables all naming conventions (as they are currently not configured and
    thus horribly noisy)
  * disables the NoImplicitTypeConversion check as it is very noisy and better
    done with appropriate MISRA rules
  * detect unsafe variable access by defining task priorities
  * make Axivion less noisy, by disabling unused style-checks.
  * Made rules for loop-counter variables more strict.
  * enabling more detailed computation of findings (Abstract Interpretation in
    Static Semantic Analysis).

Changed
=======

- Improved the code quality of the module ``foxmath`` so that it is
  MISRA-compliant (:numref:`FOXMATH`).
- Refactored ``cc-options`` parsing to separate tool to simplify the TI ARM CGT
  compiler tool.

Deprecated
==========

Removed
=======

Fixed
=====

- When HALCoGen was not available, the waf tool nevertheless tried to set the
  respective include path, which lead to exception in Python. This has been
  fixed by not trying to set the include path when HALCoGen is not available.

********************
[1.1.1] - 2021-08-06
********************

Added
=====

- All measurement values are now invalidated after startup to prevent invalid
  data on CAN.
- Add a script for VS Code detection.
- Added signal data for string related CAN messages

Changed
=======

Deprecated
==========

Removed
=======

Fixed
=====

- fixed bug, that multiplexed cell voltages 2+3 were transmitted incorrectly
  via CAN (``foxBMS_CellVoltage``)
- fixed bug, that current limits were transmitted incorrectly via CAN
  (``foxBMS_LimitValues``)

********************
[1.1.0] - 2021-07-29
********************

|foxbms| now defaults to |code-composer-studio| 10.3.1 in order to build
the binaries. If an older version of the compiler should be used, it can be
configured in ``conf/env/paths_win32.txt``. For installation instructions
see :numref:`css_install`.

|foxbms| now ships with a new conda environment ``2021-08-arctic-fox`` and the
local conda environment needs to be updated (see :numref:`conda_env_update`).

Added
=====

- Diagnosis entries can now be configured specifically for each LTC6813-1
  instance.
- Endianness is now configurable in CAN module.
- Add number of balanced cells in each string to database entry
  ``DATA_BLOCK_ID_BALANCING_CONTROL``.
- Added a built-in self-test for the database module that writes and reads an
  entry during startup of the system.
- Added a helper tool for the maintenance of the list of licenses in the conda
  environment.
- Added a helper macro for the generation of array initializers
  (``REPEAT_U()``).
- Documented how to use Ozone to show the location of a failed assertion.
- Cell voltages and temperatures are each stored in one array containing all
  values. Added database helper functions to get string, module or cell/sensor
  number from index in the global array.
- Added a block diagram of the |foxbms| master hardware.
- Added a SPI API function that allows to switch the functional state (GIO or
  SPI) of a SPI pin of the MCU.
- Added testing for different versions of TI CCS (versions ``10.2.1``,
  ``10.3.0``, ``10.3.1`` and ``10.4.0``).
- Add fallback compiler detection.
- Added a link to foxBMS project website with acknowledgment information:

  - List of funded projects foxBMS has received funding from
  - Instructions on how to reference to |foxbms|

- Added handling of I2C multiplexers to the Maxim monitoring IC driver.
- Added a flash tool to the waf toolchain. It is based on SEGGER J-Flash and
  can be invoked by calling ``waf install_bin`` or running the ``Flash:Binary``
  task in |code|.
- Added a short note on the installation of PEAK-Drivers (for CAN
  communication) to the installation manual.
- Added a section on testing the setup of the toolchain.
- Added configurable reaction type, delay and severity for each diagnosis
  entry.
- Configuration of failures, that lead to a transition to the error state is
  now done by configuring the severity of the respective diagnosis entry to
  ``DIAG_FATAL_ERROR``. The handling of the timing delay to transition into
  error state is done by the BMS module.
- Implemented CAN messages defined in .dbc file. Only stubs are implemented for
  some CAN messages respectively some signals.
- Documented workarounds that have to be applied to HALCoGen.

Changed
=======

- The schematic of the |bms-master| has been updated (``v1.0.2``).
- The new default compiler set in ``conf/env/paths_win32.txt`` is now
  TI ARM CGT v20.2.4.LTS (shipped with CCS ``10.3.1``).
- Reorganized the compiler tests in ``tests/ccs/*``.
- Updated the development conda environment to ``2021-06-arctic-fox``. Please
  run the ``conda-update-env.bat`` script in order to update your local
  environment (see :numref:`conda_env_update`).
- CAN callback functions are now defined in separate files to increase
  readability.
- Ring buffer for CAN RX was replaced by a |freertos| queue. A DIAG entry was
  added that detects if the queue is full when trying to add an element.
- Updated the black configuration to match the new python version
  (``Python 3.9.5``).
- Improved the Ozone configuration so that an data breakpoint is set
  automatically on the symbol ``fas_assertLocation.pc``.
- Run VS Code build tasks as process.
- Use posix paths in VS Code configuration on all host platforms.
- Improved the MISRA compliance of the monitoring driver for the family of
  Maxim ICs.
- Pause the terminal when running ``ide.bat``, ``waf.bat`` and ``waf.sh`` in
  case the conda base or development environment could not be found. This is
  done to clearly point the user to the error message.
- Moved the struct storing the version of the software to a fixed location
  (address ``0x003FFF40``) in flash memory.
- Replace all access to registers through the ``IO_`` functions with an access
  that keeps the volatile keyword on the register.
- Dropped the requirement for an internal library (``pyhameg``) in the HIL
  test.
- Updated the unit test framework ``ceedling`` to version ``0.31.1``.
- Simplified the configuration of the Cppcheck tool.
- Improved several function names in the ``ftask`` module.
- All required Queues are now created in one function (``FTSK_CreateQueues``)
  before the scheduler starts.
- Split the waf compiler tool into smaller sub-tools to simplify maintenance
  and tests.

Deprecated
==========

Removed
=======

- Completely removed dynamic allocation from |freertos| and reduced the (unused)
  heap size to 0.
- Removed deprecated option ``COLS_IN_ALPHA_INDEX`` from the doxygen
  configurations.
- Disabled FEE driver in the HALCoGen project as it is currently unused.
- Removed the hardware design files from the ``hardware`` directory and
  provided a URL under which the files can be found.

Fixed
=====

- The implementation of ``DIAG_ErrorCurrentOnOpenString()`` had an error where
  the wrong enum has been used leading to an assertion.
- Fixed bug in SOC/SOE counting module that extrapolated for cell voltages
  below lookup-table range
- Fixed bug that caused current sensor scaling values to be only recalibrated
  after startup and not while recalibrating SOC/SOE values via LUT when BMS is
  at rest.
- A race condition between the SOC and SOE calculation lead to a mutual
  overwriting of the state estimation values.
- Added documentation for the CAN module.
- The regex to parse the ``linkcheck`` output in case of a broken URL did not
  match and raised an exception. The regex is now simpler and has a fallback
  option.
- ``void`` functions used unnecessary ``return;`` at the end of the function.
  These returns have been removed.
- Sometimes ``black`` would fail to write to the cache pickle due to concurrent
  write processes of multiple ``black`` processes. This issue has been fixed by
  passing all files, that should be checked, at once to ``black``.
- Fixed comparison error in SOF module that compared mA with A.
- Some C-language guideline checks did not properly take C language global
  excludes into account.
- Updated links in :numref:`HALCOGEN_TOOL_DOCUMENTATION`.
- Fixed construction of the path variable in batch scripts.

********************
[1.0.2] - 2021-04-30
********************

Added
=====

- Added number of valid cell voltages and temperatures used for calculating
  minimum, maximum and average values to database entry
  ``DATA_BLOCK_ID_MIN_MAX``.
- Added the build option ``--skip-doxygen`` to enable faster documentation
  builds, if an API documentation is not required/does not need to be updated.

Changed
=======

- Do no longer hard code the EOL for the files in the VS Code settings, instead
  just use the system EOL instead.

Deprecated
==========

Removed
=======

Fixed
=====

- Fixed typo in changelog. The LTC 6804-1 is basically an older version of the
  LTC 6811-1 (previously it stated wrongly LTC 6812-1).
- ``linkcheck`` and ``spelling`` were not run when the documentation was built.
- The GUI searched for the wrong license file during startup routine and
  therefore failed to start successfully.
- Improved the readability of the engine block schematic in the general
  software documentation.
- Fixed an issue where the ``waf.sh``-wrapper would not print the error message
  when the conda environment could not be found.
- Removed unused variables in build scripts.
- Fixed readmes in the tools directory and its subdirectories.

********************
[1.0.1] - 2021-04-16
********************

Added
=====

- Added links to built versions of the documentation to the project README.
- Added testing for different versions of TI CSS (versions 10.0.0 and 10.1.1).
- Added a readme to the hardware directory.
- Added support for LTC 6804-1 (basically an older version of the LTC 6811-1).
- Added the updated design files of the |foxbms| master v1.0.1.

Changed
=======

- Activated URL availability checking for https://github.com/foxBMS/foxbms-2
  when building the documentation.
- Moved code examples in developer manual to separate source files.
- The shebang in Python files has been updated from ``python`` to ``python3``.
- Updated waf to version 2.0.22 (from 2.0.21).
- Updated database documentation.
- Updated information on tracing with Segger J-Trace probes.

Deprecated
==========

Removed
=======

- Removed external quality check in CI. This functionality has been replaced
  by the "guidelines" feature that is shipped with this repository.

Fixed
=====

- Use code page 850 as default when using the ``waf.sh``/``waf.bat`` wrappers.
- ``without-halcogen_diff-includes.diff`` was unintentionally excluded by
  the regex in ``.gitignore``.
- Fixed repository name in the documentation.
- Fixed available waf commands and put the table in a separate csv file.
- Fixed comments in LTC driver build scripts
- Includes directories were not correctly set for the drivers *LTC 6811-1* and
  *LTC 6812-1*.
- Clarification and correction of the license namings (no changes in the
  licenses themselves).
  The license of the |foxbms| software is the *BSD 3-Clause License*. This
  license is also formally known as *BSD 3-Clause "New" or "Revised" License*.
  As of this release |foxbms| added the SPDX short identifier to the software
  sources (``SPDX-License-Identifier: BSD-3-Clause``) according to
  `SPDX Specification Version 2.2 <https://spdx.dev/specifications/>`_.
  The license text of the
  `Creative Commons Attribution 4.0 International License <https://creativecommons.org/licenses/by/4.0/>`_
  has been added to the repository. The SPDX short name of this license is
  ``CC-BY-4.0``. For more information see :ref:`LICENSE`.
- Fixed the names of variables in bash scripts on Windows.

********************
[1.0.0] - 2021-04-01
********************

Added
=====

- Added automated installer script for miniconda3.
- Added automated installer script for MinGW64.
- Added automated installer script for LLVM.

Changed
=======

- Finalize red-fox conda environment ``2021-04-red-fox`` based on
  ``red-fox-devel-020``. No packages are updated, just finalized the name.
- Removed dependency of configured power lines from overcurrent check in SOA
  module. The overcurrent is now checked against allowed cell, string and pack
  current limits.
- C source files, that are not vendored, are now ``ASCII`` encoded.
- Improved the code quality by adding assertions in various locations and
  removing unused code.
- Base decisions in ``bms`` on pack value database entry as this entry contains
  only validated values. Do not directly use current sensor measurements
  anymore.
- Fixed function names in the system monitoring module.
- Replace remaining ``TRUE`` and ``FALSE`` in embedded sources with ``true``
  and ``false`` respectively from ``stdbool.h``.
- Updated LLVM to version 11.0.1
- Only compile |foxbms| source and operating system sources in ``strict_ansi``
  mode

Deprecated
==========

Removed
=======

- Removed several unused defines.

Fixed
=====

- Style guide check did not properly exclude files when checking ``*.rst``
  files.
- Fix ``@prefix`` check in Doxygen comments.
- Fix ``@prefix`` for modules ``epcos_b57251v5103j060``,
  ``epcos_b57861s0103f045`` and ``vishay_ntcalug01a103g``.
- Do not add ``__TIME__`` and ``__DATE__`` macros to list of predefined defines
  in VS Code.
- Create all build-time created include directories VS Code knows about in
  ``c_cpp_properties.json`` to remove workspace warnings.
- Add back ``Previous`` and ``Next`` buttons back in rendered documentation.
- Updated getting started information.
- Fixed function names and variable names of database structs in the ``bms``
  module.
- Fixed function names and variable names of database structs in the ``adc``
  module.
- Fixed CAN driver problem when cache is enabled.
- ``conda-init.bat`` used a wrong path to write the ``.condarc`` file if the
  user name contained whitespace.

********************
[0.3.0] - 2021-03-16
********************

Added
=====

- Added coverage reports for the unit tests of the waf-tools.
- Added general safety information.
- Added a rudimentary hardware change process to the hardware developer manual.
- Added information on the criticality of used tools.
- Added the calculation of pack values (current, power, high voltage) to the
  redundancy module.
- Added a manual on using debugger by Segger or Lauterbach in conjunction with
  the platform.
- Added a problem matcher to the ``build_docs`` task in Visual Studio Code
  so that the IDE can parse warnings printed from Doxygen
- Added documentation stubs for all embedded software modules
- Added interface description for CAN communication (.dbc and .sym)

Changed
=======

- SBC no longer monitors input on FCCU pins as usage of these pins is currently
  not supported
- The MIC modules are now able to define their own threshold values for
  plausible cell voltage measurements. In order to implement this, the MIC
  has to declare a struct of type ``MIC_PLAUSIBILITY_VALUES_s`` and pass it to
  the plausibility-functions of the MIC.
- Improves deployment speed with the internal deployment script.
- Simplifies the macro that records the program counter in ``fassert.h`` with
  input from static analysis.
- Disables the static analysis finding for dead branches in the expansion of
  the ``FAS_ASSERT`` assertion macro as the assertion should not be hit by
  normal operating code.
- Clarified the internal merge request template.
- Asserts that the SPI function ``SPI_TransmitReceiveDataDma()`` does not
  receive null pointer as parameter.
- Improved minor code quality issues in the ``DIAG_Initialize()`` function.
- Improved the documentation on installing dependencies.
- Removed function ``DIAG_configAssert()`` as it is replaced by ``FAS_ASSERT``.
- The diagnosis module has been restructured. The callbacks for each type of
  diagnosis event are now implemented in type specific files, e.g., for the
  current sensor in ``diag_cbs_current-sensor.c``. These files are in
  ``src/app/engine/diag/cbs/``. Unit tests have been added.
- Fixed typos related to battery cell and battery system.
- Enhanced the documentation section on the build process.
- Improved the documentation of the temperature sensor interface (TSI).
- Some structs in the database did not conform to the naming conventions.
- Renamed the hardware directory from ``hw`` to ``hardware``.
- ``clang-format``, ``black`` and ``pylint`` tools have been moved into the
  guidelines tool. Configuration is now done via ``rules.json`` as all other
  guidelines.

Deprecated
==========

Removed
=======

Fixed
=====

- Corrected the function ``LTC_I2CCheckACK()`` that did not check the
  acknowledge signal of multiplexer over I2C communication.
- Corrected the function ``LTC_SetMUXChCommand()`` because message content for
  I2C communication with multiplexers was wrong.
- Corrected several instances in the ``LTC6813-1``-driver where the
  loop-counter variable has been declared outside of the loop.
- Renamed function names in ``redundancy`` module to adhere to style guide
  rules.
- Fixed wording and replaced *isolation* by *insulation*.
- Replaced references to LG INR18650MJ1 by generic battery cell configuration.
- Tasks were not initialized correctly. The IDLE task never run.

********************
[0.2.0] - 2021-02-02
********************

Added
=====

- A parser to validate the linker output was added. The parser analyzes the
  output of the linking process to validate that symbols were pulled from the
  specified source. If the symbol is pulled from another source the build
  process throws an error.
- A parser to validate the style guide rules was added. The parser analyzes the
  files in the repository for style guide rule compliance. If a rule is
  violated the check guidelines and style guide process throws an error.
- Updated the development conda environment to ``red-fox-devel-020`` with
  Python version 3.8, updated packages and new packages: ``mypy``, ``rope``,
  ``seaborn``, ``statsmodels``, ``scikit-learn``, ``filterpy``, ``dask`` and
  ``scipy``.
- Documentation of the development workflow has been added as a draft.
- Discard return value of ``memcpy()`` in ``DATA_Task()`` as the return value
  is not further used.
- Added first draft of ``state estimation`` module. This module is located
  within the ``algorithm`` module and responsible for calculating state of
  charge |SOC|, state of energy |SOF|, state of health |SOH| and state of
  function |SOF|. Different calculation methods for each state estimation
  algorithm can be selected via configuration file ``bms.json``.
- Added a dummy insulation monitoring device driver. Non-functional
  Bender iso165c drivers have been added (driver fail to build).
- Added a driver for Smart Power Switches that is encapsulated by the contactor
  module. The contactor module handles the communication with the Smart Power
  Switch driver and manages the ICs.
- Added a state of energy |SOE| estimation implementation based on counting
  energy.
- Added documentation of the hardware development flow.
- Enabled the |freertos| idle hook in user code. That means that the function
  ``FTSK_UserCode_Idle()`` will be called every time the OS goes into idle
  state.
- Extended the conda environment with a sphinx-plugin that provides
  BiBTeX-capabilities. Added this plugin to the documentation build
  in order to generate bibliographies.
- Extended the API of the Temperature Sensor Interface (TSI) in order to
  return maximum and minimum limits for plausible measurement values.
- Extended the documentation with a manual for writing unit tests and a
  reference to the C coding standard the we use.
- Added an initialization function to ``sys_mon`` module to detect possible
  configuration errors
- Added more files to the unit test coverage analysis in order to have a more
  complete image of the project and its unit test coverage.
- The conda environment configuration that is distributed within the
  has now an option to validate that it is working as intended.
  The conda environment can be tested by running
  ``tools/utils/cmd/run-python-script.bat tests/env/packages_test.py -f tests/env/conda_env_win32.json``
  from the repository root.

Changed
=======

- The ``algorithm``-implementation is now able to handle calling a function
  cyclically and calling an additional initialization function. The
  implementation of the algorithm does not need to interact with the
  ``algorithm``-API anymore as this is now handled by the module.
  In order to not block the rest of the system with an algorithm that might
  take longer for computation, the algorithm module has been moved into its own
  task with a lower priority.
- Updated |freertos| to version 10.4.3 (from 10.3.0).
- Updated waf to version 2.0.21 (from 2.0.20).
- Consequently uses deci degree Celsius as integer-value (instead of degree
  Celsius floats) in the Temperature Sensor Interface (TSI).
- Moved the ``slaveplausibility``-modules into one central module in the MIC
  and renamed it to ``MIC plausibility``.
- Initialize runner variables in loops correctly as unsigned.
- Renamed directory ``measurement-ic`` to ``mic``.
- Renamed directory ``temperature-sensors`` to ``ts``.
- Changed linker script prefix from ``ldscript_`` to ``linker_script_``.
- Renamed all references in the build toolchain and documentation from
  ``linkerscript`` to ``linker_script``.
- Source files are no longer needed to be added manually to the documentation,
  instead sources are linked into the documentation based on the Doxygen
  generated documentation.
- Sets ``UNITY_INCLUDE_EXEC_TIME`` in the ceedling configuration in order to
  tell unity to track the time that a single test takes. This information is
  aggregated in the JUnit test report that is generated.
- Updated the unit test framework ``ceedling`` to version ``0.31.0`` and
  vendored the ``ceedling`` release with the repository in order to have more
  control over the executed version. Simplified the installation of Ruby in
  order to to reflect these changes. Simplified the ``f_unit_test`` waf-tool.

Deprecated
==========

Removed
=======

- Removed the deprecated Anaconda extension from the list of recommended
  Visual Studio Code plugins that is shipped with the project.

Fixed
=====

- If the runtime support library was missing on a machine, the configure step
  failed, although the runtime support library was successfully built by the
  compiler. Now a warning is printed that the runtime support library is
  missing and that it will be built. An error is no longer generated when the
  build succeeds.
- The Maxim driver did not properly map multiple modules onto internal data
  structures for multi-string systems. This has been fixed.
- Fixes various comparisons between incompatible enum types.
- Fixed erroneous behavior of the GUI. The GUI implementation is now ``black``
  and ``pylint`` compliant.
- Fixes erroneous hard-coded PEC-values in the LTC driver. A regression is
  prevented by added unit tests.
- The project version number was missing in the documentation build. The
  version information is now properly passed to the documentation build.
- The top of every documentation page began with ``orphan:``.
- Fixed implicitly defined functions.
- Fixed the unit test documentation on how to use ceedling directly. The old
  version mentioned a wrong path for the ``project.yml`` used by ceedling.

********************
[0.1.0] - 2020-11-17
********************

Initial version
