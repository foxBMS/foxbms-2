.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _BATCH_AND_SHELL_CODING_GUIDELINES:

Batch and Shell Coding Guidelines
=================================

These coding guidelines **MUST** be applied to all batch, pwsh and shell
scripts.

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

   .. literalinclude:: ./../../../conf/tpl/batch_script.bat
      :language: console
      :linenos:
      :lines: 1-37
      :caption: File header for batch scripts.
      :name: file-header-batch

.. _rules_pwsh:

PowerShell
----------

The following rules generally apply and follow the naming schema
``POWERSHELL:<ongoing-number>``.

.. _rule_powershell_filenames:

Filenames (``POWERSHELL:001``)
++++++++++++++++++++++++++++++

The following rules apply for filenames of PowerShell scripts.

.. admonition:: PowerShell script filenames

   - The general file naming rules **MUST** be applied (see
     :numref:`rule_general_filenames`).
   - Shell scripts **MUST** use ``.ps1`` as file extension.

For example the valid file names for shell scripts are

- ``hello.ps1``
- ``my-script.ps1``

.. _rule_powershell_header:

Header (``POWERSHELL:002``)
+++++++++++++++++++++++++++

.. admonition:: PowerShell script header

   PowerShell scripts **MUST** start with the following header:

   .. literalinclude:: ./../../../conf/tpl/pwsh_script.ps1
      :language: console
      :linenos:
      :lines: 1-37
      :caption: File header for PowerShell scripts.
      :name: file-header-powershell

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

   .. literalinclude:: ./../../../conf/tpl/shell_script.sh
      :language: console
      :linenos:
      :lines: 1-37
      :caption: File header for shell scripts.
      :name: file-header-shell

File Templates
--------------

These file templates below show how these rules are correctly applied.
They **SHOULD** be used as basis for new files.

- Batch script :download:`batch_script.bat <../../../conf/tpl/batch_script.bat>`
- Shell script :download:`pwsh_script.ps1 <../../../conf/tpl/pwsh_script.ps1>`
- Shell script :download:`shell_script.sh <../../../conf/tpl/shell_script.sh>`
