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
source code. In this project one of the tools that are used is the
Axivion Bauhaus Suite.

Setup
-----

- Install OpenJDK and make it available in PATH.
- Install Python3.8 from https://www.python.org/downloads/windows/. Use the
  user installer and make sure to install the Python launcher. Leave all other
  options as they are selected.
- Install Axivion Bauhaus Suite from the zip-file in a versioned directory
  e.g., ``C:\Bauhaus\x.y.z`` and add ``C:\Bauhaus\x.y.z\bin`` to the user PATH
  environment variable.
- Copy the license file into ``%USERPROFILE%\.bauhaus``.
- Copy the certificate of the dashboard server to
  ``%USERPROFILE%\.bauhaus\auto.cert``.
- **Optional:** If there is an Axivion Dashboard server running: Set the user
  environment variable ``AXIVION_PASSWORD=xyz`` to the user token obtained from
  the dashboard.

To verify that the install can successfully build run the following commands in
a terminal inside the project root (please make sure that the terminal is
freshly spawned so that it inherits the new environment variables):

.. code-block:: console

   C:\Users\vulpes\Documents\foxbms-2>waf build_axivion clean_axivion

Local Builds And Results
------------------------

- Run the analysis step

  .. code-block:: console

     C:\Users\vulpes\Documents\foxbms-2>tests\axivion\start_local_analysis.bat

- Run the following command in to start a local dashserver and **do not** close
  the terminal afterwards:

  .. code-block:: console

    dashserver start --local --noauth --install_file=%USERPROFILE%\.bauhaus\localbuild\projects\foxbms-2.db

- Follow the instructions printed on the terminal to see the results.

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
++++++++++++++

Race Condition Analysis
^^^^^^^^^^^^^^^^^^^^^^^

Go into the directory `racepdfs` and execute the wrapper batch file. This
should generate a directory with call graphs to variables that are accessed in
a potential unsafe manner. These can be used for closer investigation.
