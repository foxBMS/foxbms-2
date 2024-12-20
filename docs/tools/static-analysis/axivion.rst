.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _AXIVION_BAUHAUS_SUITE:

Axivion Bauhaus Suite
=====================

Static Analysis tries to uncover potential errors in software by analyzing the
source code.
In this project one of the tools that are used is the Axivion Bauhaus Suite.

.. note::

    The following part of the documentation describes setting up the tool
    Axivion Bauhaus Suite.
    This tool is a commercial tool by `Axivion <https://www.axivion.com/en/>`_.
    Please contact Axivion if you are interested in acquiring a license.

Setup
-----

- Install OpenJDK and make it available in PATH.
- Install Python 3.11 from https://www.python.org/downloads/windows/.
- Extract Axivion Bauhaus Suite from the zip-file and copy it in a versioned
  directory e.g., ``C:\Bauhaus\x.y.z`` and add ``C:\Bauhaus\x.y.z\bin`` to the
  user PATH environment variable.
- Copy the license file into ``%USERPROFILE%\.bauhaus\``.
- Name the certificate of the dashboard server ``auto.cert`` and copy it into
  the folder ``%USERPROFILE%\.bauhaus\``.
- **Optional:** If there is an Axivion Dashboard server running: Set the user
  environment variable ``AXIVION_PASSWORD=xyz`` to the user token obtained from
  the dashboard.

To verify the installation and create an initialization of the local database,
the following command has to be run once inside a terminal (please make sure
that the terminal is freshly spawned so that it inherits the previously set
environment variables):

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

         .\fox.ps1 axivion local-analysis

   .. group-tab:: Win32/cmd.exe

      .. code-block:: bat

         fox.bat axivion local-analysis

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

         ./fox.sh axivion local-analysis

   .. group-tab:: Linux

      .. code-block:: shell

         ./fox.sh axivion local-analysis

Local Builds And Results
------------------------

- Run the following command to start a local dashserver and **do not** close
  the terminal afterwards (follow the instructions printed on the terminal to
  see the results):

  .. tabs::

    .. group-tab:: Win32/PowerShell

        .. code-block:: powershell

          .\fox.ps1 axivion local-dashserver

    .. group-tab:: Win32/cmd.exe

        .. code-block:: bat

          fox.bat axivion local-dashserver

    .. group-tab:: Win32/Git bash

        .. code-block:: shell

          ./fox.sh axivion local-dashserver

    .. group-tab:: Linux

        .. code-block:: shell

          ./fox.sh axivion local-dashserver

- Run the following command to update the analysis result (the local dashboard
  will not be available during the execution of the local analysis):

  .. tabs::

    .. group-tab:: Win32/PowerShell

        .. code-block:: powershell

          .\fox.ps1 axivion local-analysis

    .. group-tab:: Win32/cmd.exe

        .. code-block:: bat

          fox.bat axivion local-analysis

    .. group-tab:: Win32/Git bash

        .. code-block:: shell

          ./fox.sh axivion local-analysis

    .. group-tab:: Linux

        .. code-block:: shell

          ./fox.sh axivion local-analysis

Advanced Usage
--------------

Race Condition Analysis
+++++++++++++++++++++++

Go into the directory `racepdfs` and execute the wrapper batch file.
This should generate a directory with call graphs to variables that are
accessed in a potential unsafe manner.
These can be used for closer investigation.


Updating the architecture
-------------------------

The current architecture is described in `architecture.gxl`.
This file is automatically embedded and verified against during the
architecture analysis run by this tool.
In order to update the architecture, it can be changed according to the
user manual of the tool in `gravis` and the exported in place of
`architecture.gxl`.

When updating the architecture, a new render should be created in order to
update the documentation at :ref:`SOFTWARE_ARCHITECTURE`.
This can be done through the export feature of `gravis`.
For the developer's convenience a helper script has been created.
Call

.. tabs::

   .. group-tab:: Win32/PowerShell

      .. code-block:: powershell

         .\fox.ps1 run-program gravis --script tests\axivion\scripts\gravis_export_architecture_svg.py

   .. group-tab:: Win32/cmd.exe

      .. code-block:: bat

         fox.bat run-program gravis --script tests\axivion\scripts\gravis_export_architecture_svg.py

   .. group-tab:: Win32/Git bash

      .. code-block:: shell

         ./fox.sh run-program gravis --script tests/axivion/scripts/gravis_export_architecture_svg.py

   .. group-tab:: Linux

      .. code-block:: shell

         ./fox.sh run-program gravis --script tests/axivion/scripts/gravis_export_architecture_svg.py

from a shell in the root of the project and the image will be
automatically updated.
