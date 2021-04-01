.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _BATCH_AND_SHELL_CODING_GUIDELINES:

Batch and Shell Coding Guidelines
=================================

These coding guidelines **MUST** be applied to all batch/shell scripts.

.. _rules_batch:

Batch
-----

The following rules generally apply and follow the naming schema
``BATCH:<ongoing-number>``.

.. _rule_batch_filenames:

Filenames (``BATCH:001``)
+++++++++++++++++++++++++

The following rules apply for filenames of batch scripts.

.. admonition:: Batch script filenames

   - The general file naming rules **MUST** be applied (see
     :numref:`rule_general_filenames`).
   - Batch scripts **MUST** use ``.bat`` as file extension.

For example the valid file names for batch scripts are

- ``hello.bat``
- ``my-script.bat``

.. _rule_batch_header:

Header (``BATCH:002``)
++++++++++++++++++++++

.. admonition:: Batch script header

   Batch scripts **MUST** start with the following header:

   .. literalinclude:: ./../../../conf/tpl/batch.bat
      :language: console
      :linenos:
      :lines: 1-37
      :caption: File header for batch scripts.
      :name: file-header-batch

.. _rules_shell:

Shell
-----

The following rules generally apply and follow the naming schema
``SHELL:<ongoing-number>``.

.. _rule_shell_filenames:

Filenames (``SHELL:001``)
+++++++++++++++++++++++++

The following rules apply for filenames of shell scripts.

.. admonition:: Shell script filenames

   - The general file naming rules **MUST** be applied (see
     :numref:`rule_general_filenames`).
   - Shell scripts **MUST** use ``.sh`` as file extension.

For example the valid file names for shell scripts are

- ``hello.sh``
- ``my-script.sh``

.. _rule_shell_header:

Header (``SHELL:002``)
++++++++++++++++++++++

.. admonition:: Shell script header

   Shell scripts **MUST** start with the following header:

   .. literalinclude:: ./../../../conf/tpl/shell.sh
      :language: console
      :linenos:
      :lines: 1-37
      :caption: File header for shell scripts.
      :name: file-header-shell


File Templates
--------------

These file templates below show how these rules are correctly applied.
They **SHOULD** be used as basis for new files.

- Batch script :download:`batch.bat <../../../conf/tpl/batch.bat>`
- Shell script :download:`shell.sh <../../../conf/tpl/shell.sh>`
