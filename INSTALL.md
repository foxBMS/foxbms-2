# Installation Instructions for the Host Machine

This document serves as a short summary on how to setup the **host machine**
for development of foxBMS 2.

For more detailed installation instructions (that include more automation and
less user interaction during the installation) for the host machine see
[docs/getting-started/software-installation.rst](./docs/getting-started/software-installation.rst).

> **_NOTE 1:_**
>
> Read the documentation carefully and follow every point exactly as described.\
> Otherwise, the configuration workload in later points of the setup or the
> development will be significantly higher.
>
> **_NOTE 2:_**
>
> These are the installation instructions for the host machine.\
> These are **NOT** the instructions for installing (i.e., flashing) the created
> binary on the target.\
> Flashing the binary requires a hardware debugger/flashing tool.

## Installation steps

1. Install [git](https://git-scm.com).
1. Clone the repository into a path, that does **NOT** contain whitespace

   ```shell
   git clone https://github.com/foxBMS/foxbms-2
   ```

1. Install Code Composer Studio (CCS)
   [version 12.0.0](https://www.ti.com/tool/download/CCSTUDIO/12.0.0)
   (chose `Windows single file (offline) installer for Code Composer Studio IDE (all features, devices)`).\
   When running the installer:
   1. Do **NOT** change the default installation directory chosen by the
      installer
      (e.g., `C:\ti\ccs1200` for CCS 12.0.0)
      and let the installer proceed with the installation.
   1. Select the Hercules™ Safety MCUs option during the installation.
1. Install [HALCoGen](https://www.ti.com/tool/HALCOGEN) version 04.07.01.\
   When running the installer:
   1. Do **NOT** change the default installation directory chosen by the
      installer
      (e.g., `C:\ti\Hercules\HALCoGen\v04.07.01` for HALCoGen 04.07.01)
      and let the installer proceed with the installation.
1. Install [Python](https://www.python.org/).\
   If you have already installed Python from <https://www.python.org> this step
   can be skipped.
1. Open a terminal and run `py --version`, this should print something like
   `Python 3.12.4` or similar to the terminal:

   ```pwsh
   py --version
   Python 3.12.4
   ```

1. Create a virtual environment **2024-08-pale-fox** by running in `cmd.exe` or
   `PowerShell`.
   - `cmd.exe`:

      ```cmd
      py -m venv %USERPROFILE%\foxbms-envs\2024-08-pale-fox
      ```

   - `PowerShell`:

     ```pwsh
     py -m venv $env:USERPROFILE\foxbms-envs\2024-08-pale-fox
     ```

1. Activate the virtual environment by running in `cmd.exe` or ``PowerShell`.
   - `cmd.exe`:

      ```cmd
      %USERPROFILE%\foxbms-envs\2024-08-pale-fox\Scripts\activate.bat
      ```

   - `PowerShell`:

     ```pwsh
     &"$env:USERPROFILE\foxbms-envs\2024-08-pale-fox\Scripts\activate.ps1"
     ```

1. Install the required packages by running:

   ```pwsh
   cd path\to\foxbms-2 # cd into the root of the repository
   python -m pip install -r requirements.txt --no-deps
   ```

1. Install [Ruby](https://www.ruby-lang.org)
   1. Download the installer version for
   [Ruby 3.1.3-x64 without Devkit](https://github.com/oneclick/rubyinstaller2/releases/download/RubyInstaller-3.1.3-1/rubyinstaller-3.1.3-1-x64.exe).
   1. Use `C:\Ruby\Ruby3.1.3-x64` as installation directory
   1. Do **NOT** add Ruby to `PATH`.
   1. Install the required packages, i.e., Gems by running:

      ```pwsh
      cd path\to\foxbms-2 # cd into the root of the repository
      cd tools\vendor\ceedling
      C:\Ruby\Ruby3.1.3-x64\bin\bundle install # install the Ruby Gems
      ```

1. Install GCC

   > **_NOTE:_** Installing MinWG64 requires 7-Zip to be installed.
                 7-Zip can be download from https://7-zip.org.

   1. Download MinGW-W64 version x86_64-posix-seh from
      [sourceforge.net](https://sourceforge.net/projects/mingw-w64/files/mingw-w64/)
      (use this [7z-archive](
      https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/8.1.0/threads-posix/seh/x86_64-8.1.0-release-posix-seh-rt_v6-rev0.7z)).
   1. Extract the archive.
   1. Copy the extracted mingw64 directory to
      `C:\mingw64\x86_64-8.1.0-release-posix-seh-rt_v6-rev0`.
   1. Verify that `gcc.exe` is available at
      `C:\MinGW64\x86_64-8.1.0-release-posix-seh-rt_v6-rev0\mingw64\bin\gcc.exe`.

1. Install Doxygen

   1. Download Doxygen version 1.11.0 from
      [GitHub](https://github.com/doxygen/doxygen/releases/tag/Release_1_11_0)
      (use this [zip-archive](
      https://github.com/doxygen/doxygen/releases/download/Release_1_11_0/doxygen-1.11.0.windows.x64.bin.zip)).
   1. Extract the archive.
   1. Copy the extracted archive to `C:\Users\<username>\doxygen\1.11.0`.
   1. Verify that `doxygen.exe` is available at
      `C:\Users\<username>\doxygen\1.11.0\doxygen.exe`.

1. Install Graphviz

   1. Download GraphViz version 11.0.0 from
      [https://graphviz.org/download](https://graphviz.org/download)
      (use this [zip-archive](
      https://gitlab.com/api/v4/projects/4207231/packages/generic/graphviz-releases/11.0.0/windows_10_cmake_Release_Graphviz-11.0.0-win64.zip)).
   1. Extract the archive.
   1. Copy the extracted archive to `C:\Users\<username>\graphviz\11.0.0`.
   1. Verify that `dot.exe` is available at
      `C:\Users\<username>\graphviz\11.0.0\bin\dot.exe`.

1. Check that all required software is installed by running in `cmd.exe` or
   `PowerShell`.

   - `cmd.exe`:

      ```cmd
      fox.bat install --check
      ```

   - `PowerShell`:

     ```pwsh
     .\fox.ps1 install --check
     ```

1. (Optional): Install VS Code.

   1. Download VS Code from the project website at
      [Visual Studio Code](https://code.visualstudio.com).
   1. Install code: |foxbms| recommends installing |code| with the
      `User Installer`, which does not require elevated rights.
   1. *Optional:* Let the installer add code to the `PATH` variable.

All required software is now installed.


## Environment Updates

Sometimes it might be required to update the build environment.
It that is the case, it is then mentioned in the
[CHANGELOG.md](./CHANGELOG.md).

To update the build environment the following steps must be done:

   > **_NOTE:_** The placeholder ``<name-of-the-new-env>`` must be replaced
                 with the actual name of the new build environment, which is
                 then documented in the [CHANGELOG.md](./CHANGELOG.md).

1. Create a virtual environment ``<name-of-the-new-env>`` by running in `cmd.exe` or
   `PowerShell`.
   - `cmd.exe`:

      ```cmd
      py -m venv %USERPROFILE%\foxbms-envs\<name-of-the-new-env>
      ```

   - `PowerShell`:

     ```pwsh
     py -m venv $env:USERPROFILE\foxbms-envs\<name-of-the-new-env>
     ```
1. Activate the virtual environment by running in `cmd.exe` or ``PowerShell`.
   - `cmd.exe`:

      ```cmd
      %USERPROFILE%\foxbms-envs\<name-of-the-new-env>\Scripts\activate.bat
      ```

   - `PowerShell`:

     ```pwsh
     &"$env:USERPROFILE\foxbms-envs\<name-of-the-new-env>\Scripts\activate.ps1"
     ```

1. Install the required packages by running:

   ```pwsh
   cd path\to\foxbms-2 # cd into the root of the repository
   python -m pip install -r requirements.txt --no-deps
   ```
