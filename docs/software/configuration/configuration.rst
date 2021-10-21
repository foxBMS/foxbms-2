.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _CONFIGURATION:

Configuration
=============

|foxbms| is an embedded system and some parts of the code are near to the
hardware level. The code relies on a Hardware Abstraction Layer
(HAL), which is a set of functions and definitions acting as an interface
between the application and the underlying hardware.
The HAL must be configured to fit the application and the hardware the
software is running on (e.g., configure IO pins for communication with
the outside world). This configuration is done through a software called
|halcogen|. This is described in :ref:`HAL_CONFIGURATION`.

Another point is that |foxbms| is a development platform. As such, many parts
of the code must be configured to fit the target application (e.g., which
monitoring IC is used to monitor the battery cells). This is explained in
:ref:`BMS_APPLICATION`.

.. _HAL_CONFIGURATION:

|halcogen|
----------

|halcogen| is a graphic user interface used to configure the HAL sources. It
generates sources in form of ``.h`` and ``.c`` and ``.asm`` files. These HAL
sources are generated based on the |halcogen| configuration files (``*.hcg``
and ``*.dil``). |foxbms| uses the waf tool :ref:`WAF_TOOL_HALCOGEN` to
automatically run |halcogen| and create the required sources. Additional
information on the tool can be found in :ref:`HALCOGEN_TOOL_DOCUMENTATION`.

.. note::

   In some cases it might be beneficial to **not** generate the HAL during the
   build step and instead use a generated version of the HAL. For this use
   case see :ref:`HOW_TO_USE_GENERATED_SOURCES_FROM_HALCOGEN`.

|halcogen| ships with its own version of FreeRTOS and generates the
corresponding sources when running the code generator. As |foxbms| uses its
different own copy of FreeRTOS, the generated FreeRTOS files from |halcogen|
are removed after the code generator has run.

|halcogen| creates the source file ``HL_sys_startup.c`` which implements
(a weak implementation of) the function ``_c_int00`` (the system's startup
routine). |foxbms| provides its own **non**-weak implementation of ``_c_int00``
in ``fstartup.c``. The |foxbms| implementation of ``_c_int00`` must be coupled
to the the current |halcogen| configuration. Most changes in the |halcogen|
project do not alter the startup behavior and no further action needs to be
taken into account. However there are settings that alter the startup behavior
like e.g., enabling the cache (Setting: ``TMS570LC4357ZWT_FREERTOS``,
sub-setting ``R5-MPU-PMU`` and then the configuration ``Cortex-R5``,
sub-configuration ``General Configuration``: *Enable Cache*). Such settings
need to be ported to ``fstartup.c`` as this non-weak implementation of
``_c_int00`` outweighs the generated,  new version of ``_c_int00`` in
``HL_sys_startup.c``. Otherwise the startup function used by |foxbms| would not
reflect the |halcogen| configuration. The :ref:`WAF_TOOL_HALCOGEN` provides a
mechanism to detected such changes. The hash of the current
``HL_sys_startup.c`` implementation is stored in ``src/hal/startup.hash`` and
compared to the actual hash of the generated ``HL_sys_startup.c`` file. If
these are not the same, the build aborts with the following message:

.. literalinclude:: fstartup.c-check.txt
   :language: console
   :caption: Error message on hash mismatch of ``HL_sys_startup.c``
   :name: halcogen-configuration-error

The build aborts as the expected hash is
``b'e2e61496edd65f44d7cc811b504ad1f2'`` while the actual hash is
``b'1something-other'``. Next, the function ``_c_int00`` in the two files
(``fstartup.c``) and ``HL_sys_startup.c``
needs to be compared by the developer and the developer needs to update the
``_c_int00`` implementation in the file ``fstartup.c`` to reflect the
|halcogen| startup routine. The concluding step is to update the hash value in
``src/hal/startup.hash`` with ``1something-other``. Now the build toolchain
knows, that the changes applied in the |halcogen| are reflected in the
dependencies and the build will not abort after the HAL sources are generated.

The process is illustrated in figure
:numref:`halcogen-configuration`.

   .. figure:: img/halcogen_configuration.png
      :alt: HALCoGen configuration
      :name: halcogen-configuration
      :width: 100 %
      :align: center

      HALCoGen configuration change detection process

.. _BMS_APPLICATION:

BMS application
---------------

The project provides two basic configuration options:

- source options in ``conf/bms/bms.json``
- compiler options in ``conf/cc/cc-options.yaml`` (path is an option, see
  :py:meth:`f_ti_arm_cgt.options`) and compiler
  remarks and remark severity level in ``conf/cc/remarks.txt``

Source Options
^^^^^^^^^^^^^^

Some BMS configurations require compiling different sources. That applies to
the Operating System and the measurement
IC.

.. note::

   Only very basic configurations can be changed via these options described
   here. Everything not mentioned here must still be *configured* by
   programming the application behavior in the sources.

Operating System
""""""""""""""""

The Operating System is configured in ``conf/bms/bms.json``. The value for
``os`` must be the name of the source directory in ``src/os/`` that includes
the Operating System sources. Currently only FreeRTOS is supported
(option: ``"os": "freertos"``).

Analog Front-End
""""""""""""""""

The AFE is configured in ``conf/bms/bms.json``. The joint path
from the values of ``manufacturer`` and ``chip`` must be the name of the
source directory in ``src/app/driver/afe/<manufacturer>/<chip>`` that includes
the driver implementation. A list of supported ICs is found in
:numref:`SUPPORTED_ANALOG_FRONT_ENDS`.

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

The following subsections describe the options in ``conf/cc/cc-options.yaml``.

``INCLUDE_PATHS``
"""""""""""""""""

Additional ``INCLUDE_PATHS`` that are not standard compiler includes.
Standard compiler includes are derived in the configure step of the compiler.

``LIBRARY_PATHS``
"""""""""""""""""

Additional ``LIBRARY_PATHS`` that are not standard compiler library search
paths. Standard compiler library search paths are derived in the configure
step of the compiler.

``LIBRARIES``
"""""""""""""

Libraries that are used when linking.

.. _CONFIGURATION_CFLAGS:

``CFLAGS``
""""""""""

``CFLAGS`` are configured differently or the BMS application, the Operating
System and the Hardware Abstraction Layer.

- ``common``: options are applied to all sources (BMS, OS, HAL).
- ``common_compile_only``: options are applied to all sources (BMS, OS, HAL),
  but only for the compile step, not for the preprocessor build steps.
  The build tool automatically adds that the options
  ``--gen_cross_reference_listing``,  ``--gen_func_info_listing``,
  ``--gen_preprocessor_listing``. These options control the generation of
  the ``*.aux``, ``*.crl`` and ``*.rl`` files.
- ``foxbms``: ``CFLAGS`` that should only be applied to the BMS application
  sources (``src/app/*``).
- ``hal``: ``CFLAGS`` that should only be applied to the generated hardware
  abstraction layer sources (``src/hal/*``).
- ``operating_system``: ``CFLAGS`` that should only be applied to the
  operating system sources (``src/os/*``).


``LINKFLAGS``
"""""""""""""

Flags that are passed to the compiler when linking (Note: The compiler is
used as linker when run with the argument
``--run_linker``). Flags here do typically not needed to be changed except
for ``--heap_size=0x800``,
``--stack_size=0x800`` or the optimization flag ``-oN`` where ``N`` is the
level of optimization.

``HEXGENFLAGS``
"""""""""""""""

Flags passed to hex file generation tool ``armhex`` (Note: hex files are only
generated when passing a node by keyword ``linker_script_hex`` in
``bld.tiprogram(..., linker_script_hex=some_node, ...)``).

``NMFLAGS``
"""""""""""

Flags passed to hex file generation tool ``armnm``. Flags here do
typically not needed to be changed.

Remarks
"""""""

Compiler remarks help to find potential problems at an early stage of
development. The file ``conf/cc/remarks.txt`` allows to list remarks and how
they should be handled. Global remarks are set in ``conf/cc/remarks.txt``.
Remarks are re-loaded before compiling. Remarks can be added to a single build
step as shown in :numref:`cmd-file-for-single-build-step`

.. code-block:: python
    :caption: Adding command-file that includes remarks to a single build step
    :name: cmd-file-for-single-build-step
    :linenos:

    def build(bld):
        bld.stlib(
            source=source,
            includes=includes,
            cflags=cflags,
            target=target,
            cmd_files=
            [bld.path.find_node("path/to/some/remark/file.txt").abspath()],
        )

.. warning::

   If remarks should be disabled, the option ``--issue_remarks`` needs to be
   removed in ``conf/cc/cc-options.yaml``. Furthermore all command files that
   specify remarks need to be checked and all diagnosis related commands need
   to be removed or the severity level needs to be set to ``--diag_remark=...``
   to avoid compile errors.

   The default remark settings are relatively strict to avoid common mistakes.
   **Changing them is generally not recommended**.


.. note::

   It is possible to add all kinds of compiler flags in command files, this is
   not only related to remarks.

Linker Output Processing
""""""""""""""""""""""""

.. note::

    Linker output validation only works if ``--scan_libraries`` is specified.

The linker output is processed in order to validate that the correct symbols
are linked into the binary.

If a symbol is defined in multiple places the linker decided which symbol to
use. This is described in *TI ARM assembly tools manual* in section
*Exhaustively Read and Search Libraries*.

Consider the following linker output in :numref:`output-of-linking`.

.. literalinclude:: linker_pulls/example_linker_output.txt
    :language: console
    :caption: Linker output
    :name: output-of-linking
    :linenos:

If the symbol ``_c_int00`` should be pulled from
``src/app/main/fstartup.c.1.obj``, the linking step should be treated as
successful. If the symbol is pulled from somewhere else an error must be
generated. To tell this to the linker output parser a ``json`` file that
indicates which symbol should be pulled from where needs to be defined (see
:numref:`pulls-config-file`). In this file use Unix-separator and specify the
linked source as seen from the build directory.


.. literalinclude:: linker_pulls/example_linker_pulls.json
    :language: json
    :caption: Linker pulls file ``linker_pulls.json``
    :name: pulls-config-file
    :linenos:


This file needs to be specified when a program is built (see
:numref:`linker-pulls-build-example`). If the symbol ``_c_int00`` would not be
pulled from ``src/app/main/fstartup.c.1.obj`` an error would be generated.

.. literalinclude:: linker_pulls/wscript
    :language: python
    :caption: Checking the linker output to use the correct symbols
    :name: linker-pulls-build-example
    :linenos:
    :emphasize-lines: 14
    :lines: 43-

For implementation details see :py:meth:`f_ti_arm_cgt.cprogram.parse_output`.
