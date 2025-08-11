.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _FOX_CLI:

|fox-cli|
=========

.. note::

   The goal of the |fox-cli| is to simplify the interaction with the
   repository.
   Therefore, it is **not** needed to

   - set or change any environment variables or
   - activate/deactivate any virtual Python environments.

The command line interface to interact with the repository is the |fox-cli|.
|fox-py|, in the root of the repository, is the tool that implements the tool
to interact with the repository, to e.g., build the embedded binary or run
tests.

For details of the |fox-py| implementation see :ref:`FOX_PY` .

To simplify the usage of this Python module, the following shell wrappers are
provided (also at the root of the repository):

- ``fox.ps1`` for PowerShell (``pwsh.exe``)
- ``fox.sh`` for Bash (``bash``)

The basic usage of the respective wrapper is defined by the used terminal:

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

         .\fox.ps1

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

         ./fox.sh

   .. group-tab:: Linux

      .. code-block:: shell

         ./fox.sh
