@REM Copyright (c) 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

@TITLE foxBMS Development Console

@REM Set the code page to 850
@REM https://docs.microsoft.com/en-us/windows-server/administration/windows-commands/chcp
@REM Get the current code page
@FOR /F "tokens=* USEBACKQ" %%F IN (`chcp.com 2^>^&1`) DO @(
    @SET CODEPAGE_OUT=%%F
)
@REM split outoput by whitespace and use the last argument as this contains the
@REM activate code page
@FOR %%a in (%CODEPAGE_OUT%) do @(
    @SET CODEPAGE=%%a
)
@REM Remove the trailing dot if it exists
@IF "%CODEPAGE:~-1%"=="." @(
    @SET CODEPAGE=%CODEPAGE:~0,-1%
) ELSE (
    @SET CODEPAGE=%CODEPAGE%
)

@REM If the code page is not multilingual, set it to 850
@IF NOT "%CODEPAGE%"=="850" (
    @chcp.com 850 2>&1 > nul
)
@REM now we have code page that will work

@CALL %~dp0\tools\utils\cmd\find_git.bat

@IF DEFINED GIT_DIR (
    @set "PATH=%GIT_DIR%;%PATH%"
)

@FOR /F "usebackq tokens=*" %%A in ("%~dp0\conf\env\paths_win32.txt") do @(
    @IF EXIST %%A (
        @CALL SET "NewPath=%%NewPath%%;%%A"
    )
)

@IF DEFINED NewPath (
    @SET "PATH=%NewPath:~1%;%PATH%"
)

@FOR %%X in (armcl.exe) DO @(
    @SET ARMCL_AVAILABLE=%%~$PATH:X
)

@IF NOT DEFINED ARMCL_AVAILABLE @(
    @ECHO [33mCould not find pinned compiler. Try to use any available in 'C:\ti\'.[0m
    @SET CCS_COMPILER_BIN=
    @CALL %~dp0\tools\utils\cmd\find_ccs.bat
    @FOR %%x in (!CCS_COMPILER_BIN! !CCS_COMPILER_LIB! !CCS_UTILS_BIN! !CCS_UTILS_CYGWIN! !CCS_UTILS_TIOBJ2BIN!) do @(
        @CALL SET "CCS_PATHS=%%CCS_PATHS%%%%x;"
    )
    @SET PATH=!CCS_PATHS:~0,-1!;!PATH!
)

@SET CONDA_BASE_ENVIRONMENT_ACTIVATE_SCRIPT=""
@CALL %~dp0\tools\utils\cmd\find_base_conda.bat

@IF %CONDA_BASE_ENVIRONMENT_ACTIVATE_SCRIPT%=="" (
    pause
    @EXIT /b 1
)

@CALL %CONDA_BASE_ENVIRONMENT_ACTIVATE_SCRIPT% %CONDA_DEVELOPMENT_ENVIRONMENT_NAME%

@PUSHD %~dp0
@SET PYEXE=python
@WHERE %PYEXE% 1>NUL 2>NUL
@IF %ERRORLEVEL% neq 0 SET PYEXE=py
@%PYEXE% -x "%~dp0\tools\waf" %*
@IF %ERRORLEVEL% NEQ 0 (
    @chcp.com %CODEPAGE% 2>&1 > nul
    @EXIT /b %ERRORLEVEL%
) ELSE (
    @chcp.com %CODEPAGE% 2>&1 > nul
    @conda deactivate
)
@POPD
