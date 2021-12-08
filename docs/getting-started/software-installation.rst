.. include:: ./../macros.txt
.. include:: ./../units.txt

.. _SOFTWARE_INSTALLATION:

#####################
Software Installation
#####################

.. warning::

   Read the documentation carefully and follow every point exactly as described.
   Otherwise, the configuration workload in later points of the setup or the
   development will be significantly higher.

.. note::

   For some installation tasks, batch scripts are provided to automate the
   installation process as much as possible. However, if you do not want to use
   these scripts, the manual installation process is also described.

Getting started with |foxbms| requires getting the sources and installing
all required software dependencies. These are the required steps:

#. getting the |foxbms| sources,
#. installing the software dependencies:

   #. installing |git|
   #. installing |texas-instruments| (TI)'s |code-composer-studio|
      (required for compiling)
   #. installing |texas-instruments| (TI)'s |halcogen| (required for code
      generation)
   #. installing Continuum's Miniconda (furnishes the Python environment used
      by |foxbms|),
   #. installing LLVM (required for code formatting)
   #. installing Ruby and the Ceedling package (required for unit testing)
   #. installing GCC (required for unit testing)
   #. installing Cppcheck (required for static program analysis)
   #. installing VS Code (required for developing)
#. setting up a development environment

These steps are described below in detail.

*******************
Getting the Sources
*******************

The sources are available from a |git| repository at |github_foxbms| in
|foxbms_repository|.
The following example shows how to clone (see :numref:`clone-foxbms-2`) or
download (see :numref:`download-foxbms-2`) the |foxbms| sources from GitHub .

In the case that you have been supplied with a "library-project" by a partner,
please use this archive equivalent to the "cloned |foxbms| repository" in the
later steps of this manual.

.. warning::

   If your user name contains whitespace (e.g., ``vulpes vulpes``, you **MUST**
   clone or download the repository to another path, that does not contain
   whitespace, e.g., ``C:\foxbms-2``.

.. code-block:: console
   :caption: Cloning the |foxbms| repository
   :name: clone-foxbms-2

   C:\Users\vulpes\Documents>git clone https://github.com/foxBMS/foxbms-2
   C:\Users\vulpes\Documents>cd foxbms-2

.. code-block:: console
   :caption: Downloading a release
   :name: download-foxbms-2

   C:\Users\vulpes\Documents>curl -Ss -L -o foxbms-2-v1.2.1.zip https://github.com/foxBMS/foxbms-2/archive/v1.2.1.zip
   C:\Users\vulpes\Documents>tar -x -f foxbms-2-v1.2.1.zip
   C:\Users\vulpes\Documents>ren foxbms-2-1.2.1 foxbms-2
   C:\Users\vulpes\Documents>cd foxbms-2

.. note::

   Both ``git`` and ``curl`` require a correct proxy setup. This means
   that the environment variables ``http_proxy`` and ``https_proxy`` must
   be set accordingly to your network configuration.

.. warning::

   Do not clone/download |foxbms| into a directory structure that includes
   whitespace.

   .. table::

        +------+----------------------------------------------+
        | Bad  | ``C:\bad directory name\Documents\foxbms-2`` |
        +------+----------------------------------------------+
        | Good | ``C:\Users\vulpes\Documents\foxbms-2``       |
        +------+----------------------------------------------+
        | Good | ``C:\foxbms-2``                              |
        +------+----------------------------------------------+

.. _software_prerequisites:

**********************
Software Prerequisites
**********************

.. admonition:: General Hint

   No software installation here should alter the ``PATH`` environment
   variable. When an installer asks during the setup to add something to
   ``PATH``, always remove that option, whether it is explicitly mentioned in
   that step or not. The only exception to that rule **MAY** be |git| and
   ``VS Code``.

.. _git_install:

Installing |git|
================

Download the latest version of `Git`_ and run the installer.

.. _css_install:

Installing |code-composer-studio|
=================================

Download `Code Composer Studio`_ (CCS) version |version_ccs| and run the
installer (chose ``Windows single file installer for CCS IDE``).
Do **not** change the default installation directory chosen by the installer
and let the installer proceed with the installation into the installation
directory (i.e. ``C:\ti\...`` for CCS ``10.3.1``). Select the
``Hercules™ Safety MCUs`` option during the installation.

Installing Code Composer Studio may take a while.

Installing |halcogen|
---------------------

Download `HALCoGen`_ version |version_hcg| and run the installer.
Do **not** change the default installation directory chosen by the installer
and let the installer proceed with the installation into the installation
directory (i.e. ``C:\ti\...`` for |halcogen| ``04.07.01`` that means into
``C:\ti\Hercules\HALCoGen\v04.07.01``).

Installing |halcogen| may take a while.

.. _miniconda_install_and_configuration:

Installing Miniconda and Configuration
======================================


.. note::

   If you already have installed miniconda3 in one of the default miniconda3
   installation directories (``%USERPROFILE%\miniconda3``,
   ``%LOCALAPPDATA%\Continuum\miniconda3``) or ``C:\miniconda3`` you can skip
   this installation of miniconda3 and go on to ``conda configuration``

#. Installation of miniconda3

   +--------------------------------------------------------------------------------+-----------------------------------------+
   | Automated Installation                                                         | Manual Installation                     |
   +================================================================================+=========================================+
   | For automated installation of miniconda3 according to |foxbms| requirements    | The manual installation                 |
   | just run:                                                                      | of miniconda3 is                        |
   |                                                                                | described in                            |
   | .. code-block:: console                                                        | :ref:`MANUAL_MINICONDA_INSTALLATION`.   |
   |   :caption: Running the automated ``miniconda3-install.bat`` installer script  |                                         |
   |      for miniconda3                                                            |                                         |
   |                                                                                |                                         |
   |   C:\Users\vulpes\Documents\foxbms-2>tools\utils\miniconda3-install.bat        |                                         |
   |                                                                                |                                         |
   +--------------------------------------------------------------------------------+-----------------------------------------+

#. conda configuration

   +--------------------------------------------------------------------------------+-----------------------------------------+
   | Automated Configuration                                                        | Manual Configuration                    |
   +================================================================================+=========================================+
   | For automated configuration of conda according to |foxbms| requirements just   | The manual configuration                |
   | run:                                                                           | of conda is                             |
   |                                                                                | described in                            |
   | .. code-block:: console                                                        | :ref:`MANUAL_CONDA_CONFIGURATION`.      |
   |   :caption: Running the automated ``conda-init.bat`` configuration script      |                                         |
   |                                                                                |                                         |
   |   C:\Users\vulpes\Documents\foxbms-2>tools\utils\conda-init.bat                |                                         |
   |                                                                                |                                         |
   | .. note::                                                                      |                                         |
   |                                                                                |                                         |
   |    If the script returns                                                       |                                         |
   |    *Maybe you need to add "foxbms" to your channels list.*                     |                                         |
   |    please follow the instructions at :ref:`condarc_configuration`.             |                                         |
   |                                                                                |                                         |
   +--------------------------------------------------------------------------------+-----------------------------------------+

Installing LLVM
===============

+--------------------------------------------------------------------------------+-----------------------------------------+
| Automated Installation                                                         | Manual Installation                     |
+================================================================================+=========================================+
| For automated installation of LLVM according to |foxbms| requirements          | Download `LLVM`_ (version |version_llvm||
| just run (the installer will prompt for privileged installation):              | or greater) from `github.com/LLVM`_ and |
|                                                                                | run the installer. Let it install it    |
| .. code-block:: console                                                        | into |path_llvm|. Do not add LLVM to    |
|   :caption: Running the automated ``llvm-install.bat`` installer script        | ``PATH``.                               |
|      for LLVM                                                                  |                                         |
|                                                                                |                                         |
|   C:\Users\vulpes\Documents\foxbms-2>tools\utils\llvm-install.bat              |                                         |
|                                                                                |                                         |
+--------------------------------------------------------------------------------+-----------------------------------------+

Installing Ruby and Required Gems
=================================

+--------------------------------------------------------------------------------+-----------------------------------------+
| Automated Installation                                                         | Manual Installation                     |
+================================================================================+=========================================+
| For automated installation of Ruby according to |foxbms| requirements just     | The manual installation                 |
| run:                                                                           | of Ruby is                              |
|                                                                                | described in                            |
| .. code-block:: console                                                        | :ref:`MANUAL_RUBY_INSTALLATION`.        |
|   :caption: Running the automated ``ruby-install.bat`` installer script for    |                                         |
|      Ruby                                                                      |                                         |
|                                                                                |                                         |
|   C:\Users\vulpes\Documents\foxbms-2>tools\utils\ruby-install.bat              |                                         |
|                                                                                |                                         |
| Add ``--http-proxy=http://user:password@server:port`` (adapted to your         |                                         |
| settings) to the installer script if you are behind a proxy.                   |                                         |
+--------------------------------------------------------------------------------+-----------------------------------------+

Installing GCC
==============

.. note::

   Installing MinWG64 requires 7-Zip to be installed. 7-Zip can be download
   from https://7-zip.org/ and should be installed in the default directory.

+--------------------------------------------------------------------------------+-----------------------------------------+
| Automated Installation                                                         | Manual Installation                     |
+================================================================================+=========================================+
| For automated installation of MinGW64 according to |foxbms| requirements       | The manual installation                 |
| just run:                                                                      | of MinGW64 is                           |
|                                                                                | described in                            |
| .. code-block:: console                                                        | :ref:`MANUAL_MINGW64_INSTALLATION`.     |
|   :caption: Running the automated ``mingw64-install.bat`` installer script     |                                         |
|      for MinGW64                                                               |                                         |
|                                                                                |                                         |
|   C:\Users\vulpes\Documents\foxbms-2>tools\utils\mingw64-install.bat           |                                         |
|                                                                                |                                         |
+--------------------------------------------------------------------------------+-----------------------------------------+

Installing Cppcheck
===================

.. note::

   The Cppcheck installer does not support installing different versions of
   Cppcheck. If there is already some Cppcheck version installed into
   ``C:\Program Files\Cppcheck\*`` this installation must be copied to some
   temporary folder, and then can be copied back into
   ``C:\Program Files\Cppcheck\other-cppcheck-version`` after the installation
   has finished.

#. Download Cppcheck
   `version 2.2 <https://github.com/danmar/cppcheck/releases/download/2.2/cppcheck-2.2-x64-Setup.msi>`_
   from the project's GitHub release page.
#. Install Cppcheck into ``C:\Program Files\Cppcheck\Cppcheck-2.2-x64`` by
   running the installer.

Installing VS Code and Extensions
=================================

|foxbms| supports developing with `Visual Studio Code`_ (hereinafter |code|).

#. Download |code| from the project website at `Visual Studio Code`_.
#. Install code: |foxbms| recommends installing |code| with the
   ``User Installer``, which does not require elevated rights.
#. *Optional:* Let the installer add code to the PATH variable.
#. Install extensions: To get the best development experience the following
   extensions are recommended. |code| will list these among the recommended
   extensions after you have run ``waf configure``.

   .. table:: VS Code extensions
        :name: vs-code-extensions
        :widths: grid

        +------------------------+-------------------------------------+
        | Name                   | Marketplace Link                    |
        +========================+=====================================+
        | C/C++                  | `C/C++ extension`_                  |
        +------------------------+-------------------------------------+
        | Python                 | `Python extension`_                 |
        +------------------------+-------------------------------------+
        | Assembler              | `ASM extension`_                    |
        +------------------------+-------------------------------------+
        | Linker script          | `Linker script extension`_          |
        +------------------------+-------------------------------------+
        | Code Spell Checker     | `Code Spell Checker extension`_     |
        +------------------------+-------------------------------------+
        | YAML                   | `YAML extension`_                   |
        +------------------------+-------------------------------------+
        | TOML                   | `TOML extension`_                   |
        +------------------------+-------------------------------------+
        | Test Explorer          | `Test Explorer extension`_          |
        +------------------------+-------------------------------------+
        | Test Explorer Ceedling | `Test Explorer Ceedling extension`_ |
        +------------------------+-------------------------------------+

All required software is now installed.

.. _conda_env_update:

*****************************
Development Environment Setup
*****************************

Open a terminal and navigate into the cloned |foxbms| repository and install
the development environment. The development environment is installed with the
following command:

.. code-block:: console
   :caption: Updating the miniconda environment
   :name: updating-the-miniconda-environment

   C:\Users\vulpes\Documents\foxbms-2>tools\utils\conda-update-env.bat

This scripts creates a development environment inside the miniconda
installation.

.. note::

  An update of the |foxbms| development environment needs to be done
   - the first time a |foxbms| project is setup on a computer.
   - each time a new development environment is released. If that is the case
     it is mentioned in the :ref:`CHANGELOG`.

**OPTIONAL**: Verify the miniconda environment by running the following script:

.. code-block:: console
   :caption: Verifying the miniconda environment
   :name: verifying-the-miniconda-environment

   C:\Users\vulpes\Documents\foxbms-2>tools\utils\cmd\run-python-script.bat tests\env\packages_test.py -f tests\env\conda_env_win32.json

**If all the indicated steps have been followed, at this point all the
required dependencies have been installed/updated.**

************************
Debugger toolchain setup
************************

For the development of an embedded system both a way of downloading the
software into the target and debugging the running software in the target is
necessary. Since the setup is highly dependent on the selected toolchain, this
manual does not give any details on the installation of such a debugger
toolchain. The manuals of the tool vendors are exhaustive on the topic of
installation and setup.

More details on the selection and usage of debugger toolchains can be
found in :ref:`DEBUGGING_THE_APPLICATION`.

*********************************
Optional: CAN-Driver installation
*********************************

One way of communicating with the BMS is through the CAN protocol.
The toolchain of |foxbms| is targeted on the usage of CAN interfaces of the
PCAN series by PEAK System.
In order to use these probes, the required driver has to be downloaded from the
`website of the vendor <https://www.peak-system.com/>`_.

When installing, please make sure that the option for "PCAN-Basic API" is
selected, as this is required for the |foxbms| GUI application.
