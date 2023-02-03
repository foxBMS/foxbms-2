@REM Copyright (c) 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
@REM All rights reserved.
@REM
@REM SPDX-License-Identifier: BSD-3-Clause
@REM
@REM Redistribution and use in source and binary forms, with or without
@REM modification, are permitted provided that the following conditions are met:
@REM
@REM 1. Redistributions of source code must retain the above copyright notice, this
@REM    list of conditions and the following disclaimer.
@REM
@REM 2. Redistributions in binary form must reproduce the above copyright notice,
@REM    this list of conditions and the following disclaimer in the documentation
@REM    and/or other materials provided with the distribution.
@REM
@REM 3. Neither the name of the copyright holder nor the names of its
@REM    contributors may be used to endorse or promote products derived from
@REM    this software without specific prior written permission.
@REM
@REM THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
@REM AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
@REM IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
@REM DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
@REM FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
@REM DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
@REM SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
@REM CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
@REM OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
@REM OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
@REM
@REM We kindly request you to use one or more of the following phrases to refer to
@REM foxBMS in your hardware, software, documentation or advertising materials:
@REM
@REM - "This product uses parts of foxBMS&reg;"
@REM - "This product includes parts of foxBMS&reg;"
@REM - "This product is derived from foxBMS&reg;"

@SETLOCAL EnableExtensions EnableDelayedExpansion

@NET FILE 1>NUL 2>NUL
@IF %ERRORLEVEL% NEQ 0 (
    @ECHO [31mInstalling LLVM with this script requires an elevated terminal.[0m
    @ECHO Open cmd.exe again with administrative privileges ^("Run as administrator"^) and run the script again.
    @ECHO Exiting...
    @PAUSE
    @EXIT /b 1
)

@TITLE Installing LLVM

@SET LLVM_BASE_INSTALL_DIRECTORY=C:\Program Files\LLVM
@SET LLVM_VERSION=13.0.0
@SET LLVM_TEMP_DIR=%TEMP%\LLVM
@SET LLVM_INSTALL_DIRECTORY=%LLVM_BASE_INSTALL_DIRECTORY%\%LLVM_VERSION%
@SET LLVM_DOWNLOAD_URL=https://github.com/llvm/llvm-project/releases/download/llvmorg-%LLVM_VERSION%/LLVM-%LLVM_VERSION%-win64.exe
@SET LLVM_INSTALLER="%USERPROFILE%\Downloads\llvm-%LLVM_VERSION%-win64.exe"

@IF NOT EXIST "%LLVM_TEMP_DIR%" (
    @MKDIR "%LLVM_TEMP_DIR%"
)

@IF EXIST "%LLVM_BASE_INSTALL_DIRECTORY%" (
    @XCOPY "%LLVM_BASE_INSTALL_DIRECTORY%" "%LLVM_TEMP_DIR%" /s /y
    @IF %ERRORLEVEL% NEQ 0 (
        @EXIT /b %ERRORLEVEL%
    )
)

@IF EXIST "%LLVM_BASE_INSTALL_DIRECTORY%\Uninstall.exe" (
    @"%LLVM_BASE_INSTALL_DIRECTORY%\Uninstall.exe" /S
)

@REM Maybe there is already a versioned installation
@FOR /F "tokens=* USEBACKQ" %%F IN (`dir "%LLVM_BASE_INSTALL_DIRECTORY%" /b /o-n`) do @(
  @set "LATEST_LLVM_VERSION=%%F"
  @goto :done
)
:done
@IF EXIST "%LLVM_BASE_INSTALL_DIRECTORY%\%LATEST_LLVM_VERSION%\Uninstall.exe" (
    @"%LLVM_BASE_INSTALL_DIRECTORY%\%LATEST_LLVM_VERSION%\Uninstall.exe" /S
)

@REM Cleanup what ever might be left over during uninstallation
@RMDIR /s /q "%LLVM_BASE_INSTALL_DIRECTORY%"

@ECHO Downloading LLVM
@curl.exe -fsSL %LLVM_DOWNLOAD_URL% -o "%LLVM_INSTALLER%"
@ECHO Done
@ECHO Installing LLVM %LLVM_VERSION%
@%LLVM_INSTALLER% /S /tasks="noassocfiles,nomodpath" /D=%LLVM_INSTALL_DIRECTORY%
@ECHO Done

@ECHO Restoring old LLVM versions
@XCOPY "%LLVM_TEMP_DIR%" "%LLVM_BASE_INSTALL_DIRECTORY%" /s /e /y
@IF %ERRORLEVEL% EQU 0 (
    @RMDIR /s /q "%LLVM_TEMP_DIR%"
)
@ECHO Done
