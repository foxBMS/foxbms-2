.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _FOX_CLI_BASICS:

|fox-cli| Basics
================

The interface to interact with the repository is the |fox-cli|,
which is implemented in the root of the repository at ``cli/**``.
The |fox-cli| enables to e.g., build the embedded binary or run tests.
The goal of the |fox-cli| is to control and simplify the interaction with the
repository.
Therefore, it is **not** needed to

- set or change any environment variables or
- activate/deactivate any virtual Python environments.

The |fox-cli| is implemented as Python module.
To simplify its usage, the following shell wrappers are provided (also at the
root of the repository):

- ``fox.ps1`` for PowerShell (``pwsh.exe``)
- ``fox.sh`` for Bash (``bash``)

Users shall always use these shell wrappers, and not the Python module
directory.
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

Implementation Details
----------------------

For details on the |fox-cli| implementation, see :ref:`DEVELOPING_FOX_CLI`.

Usage
-----

For details on the |fox-cli| usage, see :ref:`FOX_CLI_USAGE`.

.. include:: ./../../build/docs/fox_help.txt

|fox-cli-package|
-----------------

.. note::

   Further documentation on the |fox-cli-package|:

   - :ref:`INSTALL_FOX_CLI_PACKAGE` (Installation instructions)
   - :ref:`FOX_CLI_PACKAGE` (Information on the Usage)

If only some of the |fox-cli| tools are needed and access to the
repository is not required, use the |fox-cli-package|.
To interact with the |fox-cli-package|, use the following command regardless of
the terminal used:

.. tabs::

   .. group-tab:: Windows

      .. code-block:: shell

         fox-cli

   .. group-tab:: Linux

      .. code-block:: shell

         fox-cli
