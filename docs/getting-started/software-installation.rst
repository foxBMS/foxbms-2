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

Getting started with |foxbms| requires getting the sources and installing
all required software dependencies.
These are the required steps:

#. Install |git|
#. Get the |foxbms| sources
#. Install the software dependencies:

   #. Install |ti| (TI)'s |code-composer-studio| (required for compiling)
   #. Install |ti| (TI)'s |halcogen| (required for code generation)
   #. Install Python and an virtual environment (furnishes the Python
      environment used by |foxbms|)
   #. Install Ruby and the Ceedling package (required for unit testing)
   #. Install GCC (required for unit testing)
   #. Install VS Code (required for developing)
   #. Check the installation
   #. (Optional) Install VS Code

These steps are described below in detail.

.. _git_install:

*************
Install |git|
*************

Download the latest version of `Git`_ and run the installer.

*******************
Getting the Sources
*******************

The sources are available from a |git| repository at |github_foxbms| in
|foxbms_repository|.
The following example shows how to clone the |foxbms| sources from GitHub.

In the case that you have been supplied with a "library-project" by a partner,
please use this archive equivalent to the "cloned |foxbms| repository" in the
later steps of this manual.

.. note::

   The username in this installation manual is ``vulpes`` and needs to be
   replaced with the actual username.

.. warning::

   If your user name contains whitespace (e.g., ``vulpes vulpes``, you **MUST**
   clone the repository to another path, that does not contain whitespace,
   e.g., ``C:\foxbms-2``).


   .. tabs::

      .. group-tab:: Win32/PowerShell

         .. code-block:: powershell

            git clone https://github.com/foxBMS/foxbms-2
            cd foxbms-2

      .. group-tab:: Win32/cmd.exe

         .. code-block:: bat

            git clone https://github.com/foxBMS/foxbms-2
            cd foxbms-2

      .. group-tab:: Win32/Git bash

         .. code-block:: shell

            git clone https://github.com/foxBMS/foxbms-2
            cd foxbms-2

      .. group-tab:: Linux

         .. code-block:: shell

            git clone https://github.com/foxBMS/foxbms-2
            cd foxbms-2

.. note::

   ``git`` requires a correct proxy setup.
   This means that the environment variables ``http_proxy`` and ``https_proxy``
   must be set accordingly to your network configuration.

.. warning::

   Do not clone/download |foxbms| into a directory structure that includes
   whitespace.

   .. table::

        +------+--------------------------------------+
        | Bad  | ``C:\bad directory name\foxbms-2``   |
        +------+--------------------------------------+
        | Bad  | ``/opt/bad directory name/foxbms-2`` |
        +------+--------------------------------------+
        | Good | ``C:\Users\vulpes\foxbms-2``         |
        +------+--------------------------------------+
        | Good | ``C:\foxbms-2``                      |
        +------+--------------------------------------+

.. _software_prerequisites:

**********************
Software Prerequisites
**********************

.. admonition:: General Hint

   No software installation here should alter the ``PATH`` environment
   variable.
   When an installer asks during the setup to add something to ``PATH``, always
   remove that option, whether it is explicitly mentioned in
   that step or not.
   The only exception to that rule **MAY** be |git| and ``VS Code``.

.. _css_install:

Install |code-composer-studio|
==============================

Download `Code Composer Studio`_ (CCS) version |version_ccs| and run the
installer (chose ``Windows single file installer for CCS IDE``).
Do **not** change the default installation directory chosen by the installer
and let the installer proceed with the installation into the installation
directory (e.g., ``C:\ti\ccs1031`` for CCS ``10.3.1``).
Select the ``Hercules™ Safety MCUs`` option during the installation.

Installing Code Composer Studio may take a while.

Install |halcogen|
------------------

Download `HALCoGen`_ version |version_hcg| and run the installer.
Do **not** change the default installation directory chosen by the installer
and let the installer proceed with the installation into the installation
directory (i.e., ``C:\ti\...`` for |halcogen| ``04.07.01`` that means into
``C:\ti\Hercules\HALCoGen\v04.07.01``).

Installing |halcogen| may take a while.

.. _python_installation_and_configuration:

Install Python and Configuration
===================================

Install Python
--------------

#. Install `Python <https://www.python.org>`_.
   If you have already installed Python from https://www.python.org this step
   can be skipped.

   .. note::

      The minimum required Python version is 3.10.

#. Open a terminal and run `py --version`, this should print something like
   `Python 3.12.4` or similar to the terminal:

   .. tabs::

      .. group-tab:: Win32/PowerShell

         .. code-block:: powershell

            py --version
            Python 3.12.4

      .. group-tab:: Win32/cmd.exe

         .. code-block:: bat

            py --version
            Python 3.12.4

      .. group-tab:: Win32/Git bash

         .. code-block:: shell

            py --version
            Python 3.12.4

      .. group-tab:: Linux

         .. code-block:: shell

            python3
            Python 3.12.8

Virtual Environment Creation
----------------------------

#. Create a virtual environment **2025-01-pale-fox** by running:

   .. tabs::

      .. group-tab:: Win32/PowerShell

         .. code-block:: powershell

            py -m venv $env:USERPROFILE\foxbms-envs\2025-01-pale-fox

      .. group-tab:: Win32/cmd.exe

         .. code-block:: bat

            py -m venv %USERPROFILE%\foxbms-envs\2025-01-pale-fox

      .. group-tab:: Win32/Git bash

         .. code-block:: shell

            py -m venv $USERPROFILE/foxbms-envs/2025-01-pale-fox

      .. group-tab:: Linux

         .. code-block:: shell

            python3 -m venv $HOME/foxbms-envs/2025-01-pale-fox

#. Activate the virtual environment by running:

   .. tabs::

      .. group-tab:: Win32/PowerShell

         .. code-block:: powershell

            &"$env:USERPROFILE\foxbms-envs\2025-01-pale-fox\Scripts\activate.ps1"

      .. group-tab:: Win32/cmd.exe

         .. code-block:: bat

            %USERPROFILE%\foxbms-envs\2025-01-pale-fox\Scripts\activate.bat

      .. group-tab:: Win32/Git bash

         .. code-block:: shell

            source $USERPROFILE/foxbms-envs/2025-01-pale-fox/Scripts/activate

      .. group-tab:: Linux

         .. code-block:: shell

            source $HOME/foxbms-envs/2025-01-pale-fox/bin/activate

#. Install the required packages by running:

   .. tabs::

      .. group-tab:: Win32/PowerShell

         .. code-block:: powershell

            cd path\to\foxbms-2 # cd into the root of the repository
            python -m pip install -r requirements.txt --no-deps

      .. group-tab:: Win32/cmd.exe

         .. code-block:: bat

            @REM cd into the root of the repository
            cd path\to\foxbms-2
            python -m pip install -r requirements.txt --no-deps

      .. group-tab:: Win32/Git bash

         .. code-block:: shell

            cd path/to/foxbms-2 # cd into the root of the repository
            python -m pip install -r requirements.txt --no-deps

      .. group-tab:: Linux

         .. code-block:: shell

            cd path/to/foxbms-2 # cd into the root of the repository
            python -m pip install -r requirements.txt --no-deps


.. _ruby_install_and_gem_install:

Install Ruby and Add Required Gems
==================================

#. Download the installer version for
   `Ruby 3.1.3-x64 without Devkit <https://github.com/oneclick/rubyinstaller2/releases/download/RubyInstaller-3.1.3-1/rubyinstaller-3.1.3-1-x64.exe>`_.

#. Use ``C:\Ruby\Ruby3.1.3-x64`` as installation directory
#. Do **NOT** add Ruby to ``PATH``.
#. Install the required packages, i.e., Gems by running:

   .. tabs::

      .. group-tab:: Win32/PowerShell

         .. code-block:: powershell

            cd path\to\foxbms-2 # cd into the root of the repository
            cd tools\vendor\ceedling
            C:\Ruby\Ruby3.1.3-x64\bin\bundle.bat install # install the Ruby Gems

      .. group-tab:: Win32/cmd.exe

         .. code-block:: bat

            @REM cd into the root of the repository
            cd path\to\foxbms-2
            cd tools\vendor\ceedling
            @REM install the Ruby Gems
            C:\Ruby\Ruby3.1.3-x64\bin\bundle.bat install

      .. group-tab:: Win32/Git bash

         .. code-block:: shell

            cd path/to/foxbms-2 # cd into the root of the repository
            cd tools/vendor/ceedling
            /c/Ruby/Ruby3.1.3-x64/bin/bundle.bat install

      .. group-tab:: Linux

         .. code-block:: shell

            cd path/to/foxbms-2 # cd into the root of the repository
            cd tools/vendor/ceedling
            bundle install # install the Ruby Gems

Install GCC
===========

.. note::

   Installing MinGW64 requires 7-Zip to be installed.
   7-Zip can be download from https://7-zip.org.

#. Download MinGW-W64 version x86_64-posix-seh from
   `sourceforge.net <https://sourceforge.net/projects/mingw-w64/files/mingw-w64/>`_
   (**use exactly this**
   `7z-archive <https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/8.1.0/threads-posix/seh/x86_64-8.1.0-release-posix-seh-rt_v6-rev0.7z>`_).

#. Extract the archive.
#. Copy the extracted mingw64 directory to ``C:\mingw64\x86_64-8.1.0-release-posix-seh-rt_v6-rev0``.
#. Verify that GCC is available at ``C:\MinGW64\x86_64-8.1.0-release-posix-seh-rt_v6-rev0\mingw64\bin\gcc.exe``.

Install Doxygen
===============

#. Download Doxygen version 1.11.0 from
   `GitHub <https://github.com/doxygen/doxygen/releases/tag/Release_1_11_0>`_.
   (use this
   `zip-archive <https://github.com/doxygen/doxygen/releases/download/Release_1_11_0/doxygen-1.11.0.windows.x64.bin.zip>`__).

#. Extract the archive.
#. Copy the extracted archive to ``C:\Users\<username>\doxygen\1.11.0``.
#. Verify that ``doxygen.exe`` is available at ``C:\Users\<username>\doxygen\1.11.0\doxygen.exe``.

Install Graphviz
================

#. Download GraphViz version 11.0.0 from
   `https://graphviz.org/download <https://graphviz.org/download>`_.
   (use this
   `zip-archive <https://gitlab.com/api/v4/projects/4207231/packages/generic/graphviz-releases/11.0.0/windows_10_cmake_Release_Graphviz-11.0.0-win64.zip>`__).

#. Extract the archive.
#. Copy the extracted archive to ``C:\Users\<username>\graphviz\11.0.0``.
#. Verify that ``dot.exe`` is available at ``C:\Users\<username>\graphviz\11.0.0\bin\dot.exe``.

Check Installation
==================

Check that all required software is installed by running in ``cmd.exe`` or
``PowerShell``.

   .. tabs::

      .. group-tab:: Win32/PowerShell

         .. code-block:: powershell

            cd path\to\foxbms-2 # cd into the root of the repository
            .\fox.ps1 install --check

      .. group-tab:: Win32/cmd.exe

         .. code-block:: bat

            @REM cd into the root of the repository
            cd path\to\foxbms-2
            fox.bat install --check

      .. group-tab:: Win32/Git bash

         .. code-block:: shell

            cd path/to/foxbms-2 # cd into the root of the repository
            ./fox.sh install --check

      .. group-tab:: Linux

         .. code-block:: shell

            cd path/to/foxbms-2 # cd into the root of the repository
            ./fox.sh install --check

(Optional) Install VS Code
==========================

|foxbms| supports developing with `Visual Studio Code`_ (hereinafter |code|).

#. Download |code| from the project website at `Visual Studio Code`_.
#. Install code: |foxbms| recommends installing |code| with the ``User Installer``, which does not require elevated rights.
#. *Optional:* Let the installer add code to the ``PATH`` variable.

.. _environment_updates:

Environment Updates
===================

Sometimes it might be required to update the build environment.
It that is the case, it is then mentioned in the :ref:`CHANGELOG`.

To update the build environment the following steps must be done:

.. note::

   The placeholder ``<name-of-the-new-env>`` must be replaced
   with the actual name of the new build environment, which is
   then documented in the :ref:`CHANGELOG`.

1. Create a virtual environment **<name-of-the-new-env>** by running:

   .. tabs::

      .. group-tab:: Win32/PowerShell

         .. code-block:: powershell

            py -m venv $env:USERPROFILE\foxbms-envs\<name-of-the-new-env>

      .. group-tab:: Win32/cmd.exe

         .. code-block:: bat

            py -m venv %USERPROFILE%\foxbms-envs\<name-of-the-new-env>

      .. group-tab:: Win32/Git bash

         .. code-block:: shell

            py -m venv $USERPROFILE/foxbms-envs/<name-of-the-new-env>

      .. group-tab:: Linux

         .. code-block:: shell

            python3 -m venv $HOME/foxbms-envs/<name-of-the-new-env>

1. Activate the virtual environment by running:

   .. tabs::

      .. group-tab:: Win32/PowerShell

         .. code-block:: powershell

            &"$env:USERPROFILE\foxbms-envs\<name-of-the-new-env>\Scripts\activate.ps1"

      .. group-tab:: Win32/cmd.exe

         .. code-block:: bat

            %USERPROFILE%\foxbms-envs\<name-of-the-new-env>\Scripts\activate.bat

      .. group-tab:: Win32/Git bash

         .. code-block:: shell

            source $USERPROFILE/foxbms-envs/<name-of-the-new-env>/Scripts/activate

      .. group-tab:: Linux

         .. code-block:: shell

            source $HOME/foxbms-envs/<name-of-the-new-env>/bin/activate

1. Install the required packages by running:

   .. tabs::

      .. group-tab:: Win32/PowerShell

         .. code-block:: powershell

            cd path\to\foxbms-2 # cd into the root of the repository
            python -m pip install -r requirements.txt --no-deps

      .. group-tab:: Win32/cmd.exe

         .. code-block:: bat

            @REM cd into the root of the repository
            cd path\to\foxbms-2
            python -m pip install -r requirements.txt --no-deps

      .. group-tab:: Win32/Git bash

         .. code-block:: shell

            cd path/to/foxbms-2 # cd into the root of the repository
            python -m pip install -r requirements.txt --no-deps

      .. group-tab:: Linux

         .. code-block:: shell

            cd path/to/foxbms-2 # cd into the root of the repository
            python -m pip install -r requirements.txt --no-deps

************************
Debugger toolchain setup
************************

For the development of an embedded system both a way of downloading the
software into the target and debugging the running software in the target is
necessary.
Since the setup is highly dependent on the selected toolchain, this
manual does not give any details on the installation of such a debugger
toolchain.
The manuals of the tool vendors are exhaustive on the topic of
installation and setup.

More details on the selection and usage of debugger toolchains can be
found in :ref:`DEBUGGING_THE_APPLICATION`.
