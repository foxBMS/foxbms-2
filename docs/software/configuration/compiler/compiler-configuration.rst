:orphan:

.. include:: ./../../../macros.txt
.. include:: ./../../../units.txt

..
   cspell:ignore armnm,armhex

.. _COMPILER_CONFIGURATION:

Compiler Configuration
======================

The general compiler and linker flags are directly defined in the main
``wscript`` and are typically not required to be changed.

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
        bld(
            features="c cstlib",
            source=source,
            includes=includes,
            cflags=cflags,
            target=target,
            cmd_files=
            [bld.path.find_node("path/to/some/remark/file.txt").abspath()],
        )

.. warning::

   If remarks should be disabled, the option ``--issue_remarks`` needs to be
   removed in the main ``wscript`` and the project needs to be re-configured.
   Furthermore all command files that specify remarks need to be checked and
   all diagnosis related commands need to be removed or the severity level
   needs to be set to ``--diag_remark=...`` to avoid compile errors.

   The default remark settings are relatively strict to avoid common mistakes.
   **Changing them is generally not recommended**.

.. note::

   It is possible to add all kinds of compiler flags in command files, this is
   not only related to remarks.
