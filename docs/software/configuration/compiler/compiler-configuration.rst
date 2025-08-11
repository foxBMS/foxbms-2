:orphan:

.. include:: ./../../../macros.txt
.. include:: ./../../../units.txt

.. _COMPILER_CONFIGURATION:

Compiler Configuration
======================

The following subsections describe the projects compiler options that
are available in ``conf/cc/cc-options.yaml``.

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
  abstraction layer sources (``src/app/hal/*``).
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

Flags passed to the ``armnm`` tool.
The ``armnm`` tool lists the symbols contained in an object files.
Flags here do typically not needed to be changed.

Remarks
"""""""

Compiler remarks help to find potential problems at an early stage of
development.
The file ``conf/cc/remarks.txt`` allows to list remarks and how they should be
handled.
Global remarks are set in ``conf/cc/remarks.txt``.
Remarks are re-loaded before compiling.
Remarks can be added to a single build step as shown in
:ref:`cmd-file-for-single-build-step`

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
   removed in ``conf/cc/cc-options.yaml``.
   Furthermore all command files that specify remarks need to be checked and
   all diagnosis related commands need to be removed or the severity level
   needs to be set to ``--diag_remark=...`` to avoid compile errors.

   The default remark settings are relatively strict to avoid common mistakes.
   **Changing them is generally not recommended**.

.. note::

   It is possible to add all kinds of compiler flags in command files, this is
   not only related to remarks.
