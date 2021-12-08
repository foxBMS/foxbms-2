.. include:: ./../../macros.txt
.. include:: ./../../units.txt

.. _AXIVION_BAUHAUS_SUITE:

Axivion Bauhaus Suite
=====================

..
    Comments:
    Axivion is the company that builds the Axivion Bauhaus suite
    cafeCC is the Axivion compiler
    dashserver is the command to run the Axivion Bauhaus Suite dashserver

.. spelling::
    Axivion
    cafeCC
    dashserver

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
- Install Python 3.8 from https://www.python.org/downloads/windows/. Use the
  user installer and make sure to install the Python launcher. Leave all other
  options as they are selected.
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

.. code-block:: console

    C:\Users\vulpes\Documents\foxbms-2>tests\axivion\start_local_analysis.bat

Local Builds And Results
------------------------

- Run the following command to start a local dashserver and **do not** close
  the terminal afterwards (follow the instructions printed on the terminal to
  see the results):

  .. code-block:: console

     C:\Users\vulpes\Documents\foxbms-2>tests\axivion\start_local_dashserver.bat

- Run the following command to update the analysis result (the local dashboard
  will not be available during the execution of the local analysis):

  .. code-block:: console

    C:\Users\vulpes\Documents\foxbms-2>tests\axivion\start_local_analysis.bat

VS Code Setup
-------------

Usage
+++++

Running the Tests
^^^^^^^^^^^^^^^^^

Press :kbd:`Ctrl + Shift + B` and run the task *axivion analysis*. This task
runs the Axivion analysis script ``tests/axivion/start_analysis.bat``.

Issues
^^^^^^

Open command palette by pressing :kbd:`Ctrl + Shift + P` and search for
*axivion*.

Local Builds
++++++++++++

If VS Code is installed the VS Code's ``settings.json`` is automatically
correctly configured for Axivion usage.

CI Builds
+++++++++

To get CI build results in VS Code add the following configuration to the
VS Code User settings:

.. code-block:: json

    "axivion.dashboards": [
        {
            "id": "user friendly Dashboard name",
            "url": "<path-to-dashboard>",
            "username": "<username>"
        }
    ]

Advanced Usage
--------------

Race Condition Analysis
+++++++++++++++++++++++

Go into the directory `racepdfs` and execute the wrapper batch file. This
should generate a directory with call graphs to variables that are accessed in
a potential unsafe manner. These can be used for closer investigation.

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

.. code-block:: bash

    gravis --script tests/axivion/gravis_export_architecture_svg.py

from a shell in the root of the project and the image will be
automatically updated.
