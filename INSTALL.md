# Software Installation

<!-- This file must manually be kept in sync with
     ./docs/getting-started/software-installation.rst
-->

This document describes how to setup the **host machine** for development of
foxBMS 2.

Use [docs/getting-started/software-installation.rst](./docs/getting-started/software-installation.rst):

- if you need more detailed installation instructions.
- for installation instructions on Linux.
- if you want to use bash instead of PowerShell.

> **_NOTE:_**
>
> Read the documentation carefully and follow every point exactly as
> described.\
> Otherwise, the configuration workload in later points of the setup or the
> development will be significantly higher.

> **_NOTE:_**
>
> Some programs are installed into a specific foxBMS 2 prefix.
> This is mostly then done, when the installers do not support side-by-side or
> versioned installations by themself.\
> The installation prefixes are as follows
>
> | Operating system   | Installation prefix   |
> |--------------------|-----------------------|
> | Windows            | ``C:\\foxbms``        |

> **_WARNING:_**
>
> - All commands shall be run in PowerShell 7 or later.
> - All commands shall be run in the root of the repository.

> **_WARNING:_**
>
> These are the installation instructions for the host machine.\
> These are **NOT** the instructions for installing (i.e., flashing) the
> created binary on the target.\
> Flashing the binary requires a hardware debugger/flashing tool.

Getting started with foxBMS 2 requires getting the sources and installing
all required software dependencies.
These are the required steps:

1. Install git
1. Get the foxBMS sources
1. Install TI Code Composer Studio (required for compiling)
1. Install TI HALCoGen (required for code generation)
1. Install Python (required for tools)
1. Install virtual Python environment (required for tools)
1. Install Ruby (required for unit testing)
1. Install Ruby Gems (required for unit testing)
1. Install mingw-w64 (required for unit testing)
1. Install Doxygen (required for building the documentation)
1. Install Graphviz (required for building the documentation)
1. Install drawio-desktop (required for building the documentation)
1. Install VS Code (required developing)
1. Check the installation

> **_NOTE:_**
>
> No software installation here should change the ``PATH`` environment
> variable.\
> When an installer asks during the setup to add something to ``PATH``, always
> remove that option, whether it is explicitly mentioned in
> that step or not.\
> The only exception to that rule **MAY** be git and ``VS Code``.

The installation steps are described below in detail.

## Install git

1. Download the latest version of [git](https://git-scm.com) and run the
   installer.
1. _Optional:_ Let the installer add git to the ``PATH`` variable.

## Get the foxBMS 2 Sources

The sources are available from a git repository at <https://github.com/foxBMS/>
in <https://github.com/foxBMS/foxbms-2>.
The following example shows how to clone the foxBMS 2 sources from GitHub.

In the case that you have been supplied with a "library-project" by a partner,
please use this archive equivalent to the cloned foxBMS 2 repository" in the
later steps of this manual.

> **_NOTE:_**
>
> ``git`` requires a correct proxy setup.
> This means that the environment variables ``http_proxy`` and ``https_proxy``
> must be set accordingly to your network configuration.

> **_WARNING:_**
>
> Do not clone/download foxBMS 2 into a directory structure that includes
> whitespace.
>
> | Good                         | Bad                                  |
> |------------------------------|--------------------------------------|
> | ``C:\Users\vulpes\foxbms-2`` | ``C:\Users\my user name\foxbms-2``   |
> | ``/opt/devel/foxbms-2``      | ``/opt/bad directory name/foxbms-2`` |

Clone the repository by running

```pwsh
git clone https://github.com/foxBMS/foxbms-2
```

## Install Code Composer Studio

Download
[Code Composer Studio](https://www.ti.com/tool/download/CCSTUDIO/12.8.1) (CCS)
version 12.8.1 and run the installer (chose the
`single file (offline) installer for Code Composer Studio IDE (all features, devices)`
depending on your host platform).

When running the installer:

1. Do **NOT** change the default installation directory chosen by the
   installer and let the installer proceed with the installation into the
   installation directory (e.g., ``C:\ti\ccs1281`` for CCS ``12.8.1``).
1. Select the `Hercules™ Safety MCUs` option during the installation.

Installing TI Code Composer Studio may take a while.

## Install HALCoGen

Download [HALCoGen](https://www.ti.com/tool/HALCOGEN) version 04.07.01
and run the installer.
When running the installer:
Do **not** change the default installation directory chosen by the
installer and let the installer proceed with the installation into the
installation directory (i.e., ``C:\ti\...`` for TI HALCoGen ``04.07.01``
that means into ``C:\ti\Hercules\HALCoGen\v04.07.01``).

Installing TI HALCoGen may take a while.

## Install Python

1. Install [Python 3.12](https://www.python.org/).\
   If you have already installed Python 3.12 from <https://www.python.org> this
   step can be skipped.

   > **_NOTE:_**
   >
   > The required Python version is exactly 3.12.x

1. Open a terminal and run `py -3.12 --version`, this should print
   something like `Python 3.12.8` or similar:

   ```pwsh
   py -3.12 --version
   Python 3.12.8
   ```

## Install Virtual Python Environment

1. Create a virtual environment **2025-06-pale-fox** by running:

   ```pwsh
   py -3.12 -m venv C:\foxbms\envs\2025-06-pale-fox
   ```

1. Activate the virtual environment by running:

   ```pwsh
   C:\foxbms\envs\2025-06-pale-fox\Scripts\activate.ps1
   ```

1. Install the required packages by running:

   > **_NOTE:_**
   >
   > ``pip`` requires a correct proxy setup.

   ```pwsh
   python -m pip install -r requirements.txt
   ```

1. Deactivate the virtual environment by running:

   ```pwsh
   deactivate
   ```

The virtual environment includes the pre-commit framework (<https://pre-commit.com/>).
The pre-commit framework installs its dependencies defined in
``.pre-commit-config.yaml`` by itself when it is run and dependencies are
missing.

## Install Ruby

> **_NOTE:_**
>
> Installing MinGW64 requires 7-Zip to be installed.
> 7-Zip can be download from <https://7-zip.org>.

1. Download the zip-archive
[Ruby 3.4.2-1 (x64)](https://github.com/oneclick/rubyinstaller2/releases/download/RubyInstaller-3.4.2-1/rubyinstaller-3.4.2-1-x64.7z).
1. Extract the archive to ``C:\foxbms\Ruby\Ruby34-x64``.
1. Verify that ``ruby.exe`` is available at
   ``C:\foxbms\Ruby\Ruby34-x64\bin\ruby.exe``.

## Install Ruby gems

Install the required Ruby packages, i.e., Ruby gems by running:

```pwsh
C:\foxbms\Ruby\Ruby34-x64\bin\gem.cmd install --install-dir C:\foxbms\Ceedling\1.0.1 ceedling
```

## Install mingw-w64

> **_NOTE:_**
>
> Installing mingw-w64 requires 7-Zip to be installed.
> 7-Zip can be download from <https://7-zip.org>.

1. Download mingw-w64 version x86_64-posix-seh from
  [sourceforge.net](https://sourceforge.net/projects/mingw-w64/files/mingw-w64/)
  (**use this**
  [7z-archive](https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/8.1.0/threads-posix/seh/x86_64-8.1.0-release-posix-seh-rt_v6-rev0.7z)).
1. Extract the archive.
1. Copy the extracted `mingw64` directory to
  `C:\foxbms\mingw-w64\x86_64-8.1.0-release-posix-seh-rt_v6-rev0`.
1. Verify that `gcc.exe` is available at
  `C:\foxbms\mingw-w64\x86_64-8.1.0-release-posix-seh-rt_v6-rev0\bin\gcc.exe`.

## Install Doxygen

1. Download Doxygen version 1.14.0 from
   [GitHub](https://github.com/doxygen/doxygen/releases/tag/Release_1_14_0)
   (use this
   [zip-archive](https://github.com/doxygen/doxygen/releases/download/Release_1_14_0/doxygen-1.14.0.windows.x64.bin.zip)).
1. Extract the archive.
1. Copy the extracted archive to `C:\foxbms\Doxygen\1.14.0`.
1. Verify that `doxygen.exe` is available at
  `C:\foxbms\Doxygen\1.14.0\doxygen.exe`.

## Install Graphviz

1. Download Graphviz version 13.0.0 from
  [https://graphviz.org/download](https://graphviz.org/download)
  (use this
  [zip-archive](https://gitlab.com/api/v4/projects/4207231/packages/generic/graphviz-releases/13.0.0/windows_10_cmake_Release_Graphviz-13.0.0-win64.zip)).
1. Extract the archive.
1. Copy the extracted archive to `C:\foxbms\Graphviz\13.0.0`.
1. Verify that `dot.exe` is available at
   `C:\foxbms\Graphviz\13.0.0\bin\dot.exe`.

## Install drawio-desktop

1. Download drawio-desktop version 27.0.9 from
   [https://github.com/jgraph/drawio-desktop/releases](https://github.com/jgraph/drawio-desktop/releases)
   (use this
   [zip-archive](https://github.com/jgraph/drawio-desktop/releases/download/v27.0.9/draw.io-27.0.9-windows.zip)).
1. Extract the archive.
1. Copy the extracted archive to `C:\foxbms\draw.io\27.0.9`.
1. Verify that `draw.io.exe` is available at
    `C:\foxbms\draw.io\27.0.9\draw.io.exe`.

## Install VS Code

foxBMS 2 supports development using Visual Studio Code.

1. Download VS Code from the project website at <https://code.visualstudio.com>.
1. Run the installer.
1. _Optional:_ Let the installer add VS Code to the `PATH` variable.

## Installation Check

Check that all required software is installed by running:

```pwsh
.\fox.ps1 install --check
```

After following these steps, the check shall succeed and all required software
has been successfully installed.

## Environment Updates

Sometimes it might be required to update the build environment.
It that is the case, it is then mentioned in the
[CHANGELOG.md](./CHANGELOG.md).

To update the build environment the following steps must be done:

> **_NOTE:_**
>
> The placeholder ``<name-of-the-new-env>`` must be replaced
> with the actual name of the new build environment, which is
> then documented in the
> [CHANGELOG.md](./CHANGELOG.md).

1. Create a virtual environment **\<name-of-the-new-env\>** by running:

   ```pwsh
   py -3.12-m venv C:\foxbms\envs\<name-of-the-new-env>
   ```

1. Activate the virtual environment by running:

   ```pwsh
   C:\foxbms\envs\<name-of-the-new-env>\Scripts\activate.ps1
   ```

1. Install the required packages by running:

   ```pwsh
   python -m pip install -r requirements.txt
   ```
