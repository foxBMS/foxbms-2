.. include:: ./../macros.txt
.. include:: ./../units.txt

.. This file must manually be kept in sync with
.. ./INSTALL.md
.. except for the Linux part, that is only documented in here

.. _SOFTWARE_INSTALLATION:

#####################
Software Installation
#####################

This document describes how to setup the **host machine** for development of
|foxbms|.

.. note::

   Read the documentation carefully and follow every point exactly as
   described.
   Otherwise, the configuration workload in later points of the setup or the
   development will be significantly higher.

.. note::

   Some programs are installed into a specific |foxbms| prefix.
   This is mostly then done, when the installers do not support side-by-side or
   versioned installations by themself.

.. warning::

   - All commands shall be run in PowerShell 7 or later.
   - All commands shall be run in the root of the repository.

.. warning::
   These are the installation instructions for the host machine.
   These are **NOT** the instructions for installing (i.e., flashing) the
   created binary on the target.
   Flashing the binary requires a hardware debugger/flashing tool.

Getting started with |foxbms| requires getting the sources and installing
all required software dependencies.
These are the required steps:

#. Install |git|
#. Get the |foxbms| sources
#. Install |ti-code-composer-studio| (required for compiling)
#. Install |ti-halcogen| (required for code generation)
#. Install |python| (required for tools)
#. Install |virtual-python-environment| (required for tools)
#. Install |ruby| (required for unit testing)
#. Install |ruby-gems| (required for unit testing)
#. Install |mingw-w64| (required for unit testing)
#. Install |doxygen| (required for building the documentation)
#. Install |graphviz| (required for building the documentation)
#. Install |drawio-desktop| (required for building the documentation)
#. Install |vs-code| (required developing)
#. Check the installation

.. note::

   No software installation here should change the ``PATH`` environment
   variable.
   When an installer asks during the setup to add something to ``PATH``, always
   remove that option, whether it is explicitly mentioned in
   that step or not.
   The only exception to that rule **MAY** be |git| and ``VS Code``.

The installation steps are described below in detail.

.. _git_install:

*************
Install |git|
*************

#. Download the latest version of |git| at |url_git| and run the
   installer.
#. *Optional:* Let the installer add |git| to the ``PATH`` variable.


************************
Get the |foxbms| Sources
************************

The sources are available from a |git| repository at |github_foxbms|
in |foxbms_repository|.
The following example shows how to clone the |foxbms| sources from GitHub.

In the case that you have been supplied with a "library-project" by a partner,
please use this archive equivalent to the cloned |foxbms| repository" in the
later steps of this manual.

.. note::

   ``git`` requires a correct proxy setup.
   This means that the environment variables ``http_proxy`` and ``https_proxy``
   must be set accordingly to your network configuration.

.. warning::

   Do not clone/download |foxbms| into a directory structure that includes
   whitespace.

   .. table::

      +------------------------------+--------------------------------------+
      | Good                         | Bad                                  |
      +------------------------------+--------------------------------------+
      | ``C:\Users\vulpes\foxbms-2`` | ``C:\Users\my user name\foxbms-2``   |
      +------------------------------+--------------------------------------+
      | ``/opt/devel/foxbms-2``      | ``/opt/bad directory name/foxbms-2`` |
      +------------------------------+--------------------------------------+

Clone the repository by running

.. code-block:: shell

   git clone https://github.com/foxBMS/foxbms-2

.. _css_install:

*********************************
Install |ti-code-composer-studio|
*********************************

Download `Code Composer Studio`_ (CCS)
version |version_ti-code-composer-studio| and run the installer (chose the
``single file (offline) installer for Code Composer Studio IDE (all features, devices)``
depending on your host platform).

.. tabs::

   .. group-tab:: Windows

      When running the installer:

      #. Do **NOT** change the default installation directory chosen by the
         installer and let the installer proceed with the installation into the
         installation directory (e.g., ``C:\ti\ccs1281`` for CCS ``12.8.1``).
      #. Select the ``Hercules™ Safety MCUs`` option during the installation.

   .. group-tab:: Linux

      When running the installer:

      #. Do **NOT** change the default installation directory chosen by the
         installer and let the installer proceed with the installation into the
         installation directory (e.g., ``/opt/ti/ccs1281`` for CCS ``12.8.1``).
      #. Select the ``Hercules™ Safety MCUs`` option during the installation.

Installing |ti-code-composer-studio| may take a while.

*********************
Install |ti-halcogen|
*********************

.. tabs::

   .. group-tab:: Windows

      Download |ti-halcogen| version |version_ti-halcogen| from
      |url_ti-halcogen_releases|
      and run the installer.
      When running the installer:
      Do **not** change the default installation directory chosen by the
      installer and let the installer proceed with the installation into the
      installation directory (i.e., ``C:\ti\...`` for |ti-halcogen| ``04.07.01``
      that means into ``C:\ti\Hercules\HALCoGen\v04.07.01``).

      Installing |ti-halcogen| may take a while.

   .. group-tab:: Linux

      |ti-halcogen| is not available on Linux.

****************
Install |python|
****************

.. tabs::

   .. group-tab:: Windows

         #. Install `Python 3.12 <https://www.python.org>`_.
            If you have already installed Python 3.12 from https://www.python.org this
            step can be skipped.

            .. note::

               The required Python version is exactly 3.12.x.

         #. Open a terminal and run ``py -3.12 --version``, this should print
            something like ``Python 3.12.8`` or similar:

            .. tabs::

               .. code-block:: powershell

                  py -3.12 --version
                  Python 3.12.8

   .. group-tab:: Linux

      #. Install ``Python 3.12``.
         If you have already installed Python 3.12 this step can be skipped.

         .. note::

            The required Python version is exactly 3.12.x.

      #. Open a terminal and run ``py --version``, this should print
         something like ``Python 3.12.8`` or similar:

         .. code-block:: shell

            python3.12 --version
            Python 3.12.8

************************************
Install |virtual-python-environment|
************************************

#. Create a virtual environment **2025-03-pale-fox** by running:

   .. tabs::

      .. group-tab:: Windows

         .. tabs::

            .. group-tab:: PowerShell

               .. code-block:: powershell

                  py -3.12 -m venv C:\foxbms\envs\2025-03-pale-fox

            .. group-tab:: Git bash

               .. code-block:: shell

                  py -3.12 -m venv /C/foxbms/envs/2025-03-pale-fox

      .. group-tab:: Linux

         .. code-block:: shell

            python3.12 -m venv /opt/foxbms/envs/2025-03-pale-fox

#. Activate the virtual environment by running:

   .. tabs::

      .. group-tab:: Windows

         .. tabs::

            .. group-tab:: PowerShell

               .. code-block:: powershell

                  C:\foxbms\envs\2025-03-pale-fox\Scripts\activate.ps1

            .. group-tab:: Git bash

               .. code-block:: shell

                  source /C/foxbms/envs/2025-03-pale-fox/Scripts/activate

      .. group-tab:: Linux

         .. code-block:: shell

            source /opt/foxbms/envs/2025-03-pale-fox/bin/activate

#. Install the required packages by running:

   .. note::

      ``pip`` requires a correct proxy setup.

   .. code-block:: shell

      python -m pip install -r requirements.txt

The virtual environment includes the pre-commit framework (|url_pre-commit|).
The pre-commit framework installs its dependencies defined in
``.pre-commit-config.yaml`` by itself when it is run and dependencies are
missing.

.. _ruby_install:

**************
Install |ruby|
**************

.. note::

   Installing MinGW64 requires 7-Zip to be installed.
   7-Zip can be download from https://7-zip.org.

.. tabs::

   .. group-tab:: Windows

      #. Download the zip-archive
         `Ruby 3.4.2-1 (x64)  <https://github.com/oneclick/rubyinstaller2/releases/download/RubyInstaller-3.4.2-1/rubyinstaller-3.4.2-1-x64.7z>`_.
      #. Extract the archive to ``C:\foxbms\Ruby\Ruby34-x64``.

   .. group-tab:: Linux

      Use your distributions package manager to install |ruby|.

.. _ruby_gem_install:

*******************
Install |ruby-gems|
*******************

Install the required Ruby packages, i.e., |ruby-gems| by running:

.. tabs::

   .. group-tab:: Windows

      .. tabs::

         .. group-tab:: PowerShell

            .. code-block:: powershell

               C:\foxbms\Ruby\Ruby34-x64\bin\gem.cmd install --install-dir C:\foxbms\Ceedling\1.0.1 ceedling

         .. group-tab:: Git bash

            .. code-block:: shell

               /C/foxbms/Ruby/Ruby34-x64/bin/gem.cmd install --install-dir /C/foxbms/Ceedling/1.0.1 ceedling

   .. group-tab:: Linux

      .. code-block:: shell

         # adapt the path to the gem binary accordingly to the Ruby
         # installation from the previous step
         gem install --install-dir /opt/foxbms/Ceedling/1.0.1 ceedling

*******************
Install |mingw-w64|
*******************

.. tabs::

   .. group-tab:: Windows

      .. note::

         Installing |mingw-w64| requires 7-Zip to be installed.
         7-Zip can be download from https://7-zip.org.

      #. Download |mingw-w64| version x86_64-posix-seh from
         `sourceforge.net <https://sourceforge.net/projects/mingw-w64/files/mingw-w64/>`_
         (**use this**
         `7z-archive <https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/8.1.0/threads-posix/seh/x86_64-8.1.0-release-posix-seh-rt_v6-rev0.7z>`_).

      #. Extract the archive.
      #. Copy the extracted ``mingw-w64`` directory to
         ``C:\foxbms\mingw-w64\x86_64-8.1.0-release-posix-seh-rt_v6-rev0``.
      #. Verify that ``gcc.exe`` is available at
         ``C:\foxbms\mingw-w64\x86_64-8.1.0-release-posix-seh-rt_v6-rev0\bin\gcc.exe``.

   .. group-tab:: Linux

      GCC is already available.

*****************
Install |doxygen|
*****************

.. tabs::

   .. group-tab:: Windows

      #. Download |doxygen| version 1.13.2 from
         `GitHub <https://github.com/doxygen/doxygen/releases/tag/Release_1_13_2>`_.
         (use this
         `zip-archive <https://github.com/doxygen/doxygen/releases/download/Release_1_13_2/doxygen-1.13.2.windows.x64.bin.zip>`__).

      #. Extract the archive.
      #. Copy the extracted archive to ``C:\foxbms\Doxygen\1.13.2``.
      #. Verify that ``doxygen.exe`` is available at
         ``C:\foxbms\Doxygen\1.13.2\doxygen.exe``.

   .. group-tab:: Linux

      Use your distributions package manager to install |doxygen|.

******************
Install |graphviz|
******************

.. tabs::

   .. group-tab:: Windows

      #. Download Graphviz version 12.2.1 from
         `https://graphviz.org/download <https://graphviz.org/download>`_.
         (use this
         `zip-archive <https://gitlab.com/api/v4/projects/4207231/packages/generic/graphviz-releases/12.2.1/windows_10_cmake_Release_Graphviz-12.2.1-win64.zip>`__).

      #. Extract the archive.
      #. Copy the extracted archive to ``C:\foxbms\Graphviz\12.2.1``.
      #. Verify that ``dot.exe`` is available at
         ``C:\foxbms\Graphviz\12.2.1\bin\dot.exe``.

   .. group-tab:: Linux

      Use your distributions package manager to install Graphviz.

************************
Install |drawio-desktop|
************************

.. tabs::

   .. group-tab:: Windows

      #. Download |drawio-desktop| version 26.0.9 from
         `https://github.com/jgraph/drawio-desktop/releases <https://github.com/jgraph/drawio-desktop/releases>`_
         (use this
         `installer <https://github.com/jgraph/drawio-desktop/releases/download/v26.0.9/draw.io-26.0.9-windows-installer.exe>`_).

      #. Run the installer and use the default installation directory.
      #. Verify that ``draw.io.exe`` is available at
         ``C:\Program Files\draw.io\draw.io.exe``.

   .. group-tab:: Linux

      Use your distributions package manager to install |drawio-desktop|.

*****************
Install |vs-code|
*****************

|foxbms| supports development using `Visual Studio Code`_.

#. Download |vs-code| from the project website at `Visual Studio Code`_.
#. Run the installer.
#. *Optional:* Let the installer add |vs-code| to the ``PATH`` variable.

******************
Installation Check
******************

Check that all required software is installed by running:

.. tabs::

   .. group-tab:: Windows

      .. tabs::

         .. group-tab:: PowerShell

            .. code-block:: powershell

               .\fox.ps1 install --check

         .. group-tab:: Git bash

            .. code-block:: shell

               ./fox.sh install --check

   .. group-tab:: Linux

      .. code-block:: shell

         ./fox.sh install --check

After following these steps, the check shall succeed and all required software
has been successfully installed.

.. _environment_updates:

*******************
Environment Updates
*******************

Sometimes it might be required to update the build environment.
It that is the case, it is then mentioned in the
:ref:`CHANGELOG`.

To update the build environment the following steps must be done:

.. note::

   The placeholder ``<name-of-the-new-env>`` must be replaced
   with the actual name of the new build environment, which is
   then documented in the
   :ref:`CHANGELOG`.

#. Create a virtual environment **<name-of-the-new-env>** by running:

   .. tabs::

      .. group-tab:: Windows

         .. tabs::

            .. group-tab:: PowerShell

               .. code-block:: powershell

                  py -3.12 -m venv C:\foxbms\envs\<name-of-the-new-env>

            .. group-tab:: Git bash

               .. code-block:: shell

                  py -3.12 -m venv /C/foxbms/envs/<name-of-the-new-env>

      .. group-tab:: Linux

         .. code-block:: shell

            python3.12 -m venv /opt/foxbms/envs/<name-of-the-new-env>

#. Activate the virtual environment by running:

   .. tabs::

      .. group-tab:: Windows

         .. tabs::

            .. group-tab:: PowerShell

               .. code-block:: powershell

                  C:\foxbms\envs\<name-of-the-new-env>\Scripts\activate.ps1

            .. group-tab:: Git bash

               .. code-block:: shell

                  source /C/foxbms/envs/<name-of-the-new-env>/Scripts/activate

      .. group-tab:: Linux

         .. code-block:: shell

            source /opt/foxbms/envs/<name-of-the-new-env>/bin/activate

#. Install the required packages by running:

   .. note::

      ``pip`` requires a correct proxy setup.

   .. code-block:: shell

      python -m pip install -r requirements.txt

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
