.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _FOX_PY:

``fox.py``
==========

``fox.py``, in the root of the repository, is the tool to interact with the
repository, to e.g., build the embedded binary or run tests.

To simplify the usage of this Python module, three shell wrappers are provided
(also at the root of the repository):

- ``fox.bat`` for Command Prompt (``cmd.exe``)
- ``fox.ps1`` for PowerShell (``pwsh.exe``)
- ``fox.sh`` for Bash (``bash``)

To simplify and increase the readability of the documentation, the
documentation will use the PowerShell script in example code.
The notation for PowerShell is ``.\fox.ps1``.

- If you are using ``cmd.exe`` use ``fox.bat`` instead of ``.\fox.ps1``.
- If you are using ``bash`` use ``./fox.sh`` instead of ``.\fox.ps1``.
