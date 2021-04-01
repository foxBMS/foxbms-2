.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _CHANGELOG:

#########
Changelog
#########

..
    Comments:
    slaveplausibility is the old name of a plausibility module
    JUnit is a test system (originally for Java)

.. spelling::
    slaveplausibility
    JUnit


All notable changes to this project will be documented in this file.

The format is based on
`Keep a Changelog <https://keepachangelog.com/en/1.0.0/>`__.

|foxbms| uses the following versioning pattern ``MAJOR.MINOR.PATCH``
where ``MAJOR``, ``MINOR`` and ``PATCH`` are numbers.

Versioning follows then these rules:

- increasing ``MAJOR`` introduces incompatible changes
- increasing ``MINOR`` adds functionality in a backwards compatible manner
- increasing ``PATCH`` fixes bugs in a backwards compatible manner

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
  only validated values. Do not directly use current sensor measurements anymore.
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
  files in the repository for style guide rule compliance. If a rule is violated
  the check guidelines and style guide process throws an error.
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
  Bender iso165c driver has been added (driver fails to build).
- Added a driver for Smart Power Switches that is encapsulated by the contactor
  module. The contactor module handles the communication with the Smart Power
  Switch driver and manages the ICs.
- Added a state of energy |SOE| estimation implementation based on counting
  energy.
- Added documentation of the hardware development flow.
- Enabled the FreeRTOS idle hook in user code. That means that the function
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
  has now an option to validate that it is working as intended. The conda
  environment can be tested by running
  ``tools\utils\cmd\run-python-script.bat tests\env\packages_test.py -f tests\env\conda_env_win32.json``
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
- Updated FreeRTOS to version 10.4.3 (from 10.3.0).
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
- Updated the unit test framework ``ceedling`` to version ``0.31.`` and
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
