@REM Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

@PUSHD %~dp0

@SET "ENV_NAME=2024-08-pale-fox"

@SET "FOXBMS_PYTHON_ENV_DIRECTORY_USER=%USERPROFILE%\foxbms-envs\%ENV_NAME%"
@SET "FOXBMS_PYTHON_ENV_DIRECTORY_ROOT=C:\foxbms-envs\%ENV_NAME%"

@SET "FOXBMS_PYTHON_ENV_DIRECTORY=%FOXBMS_PYTHON_ENV_DIRECTORY_USER%"

@REM Prefer the user installation

@IF NOT EXIST %FOXBMS_PYTHON_ENV_DIRECTORY_USER% (
    @SET "FOXBMS_PYTHON_ENV_DIRECTORY=%FOXBMS_PYTHON_ENV_DIRECTORY_ROOT%"
)

@IF NOT EXIST %FOXBMS_PYTHON_ENV_DIRECTORY% (
    @ECHO '%FOXBMS_PYTHON_ENV_DIRECTORY_USER%' and
    @ECHO '%FOXBMS_PYTHON_ENV_DIRECTORY_ROOT%' do not exist.
    @ECHO One of both must be available. See Installation instructions in
    @ECHO '%~dp0\INSTALL.md'
    @EXIT /b 1
)

@CALL %FOXBMS_PYTHON_ENV_DIRECTORY%\Scripts\activate.bat

@REM If the activation script failed, exit with error
@IF %ERRORLEVEL% NEQ 0 (
    @ECHO The activation script of the environment is missing.
    @EXIT /b %ERRORLEVEL%
)

@REM Check if Python executable exists
@SET PYTHON_EXE=python
@WHERE %PYTHON_EXE% 1>NUL 2>NUL
@IF %ERRORLEVEL% NEQ 0 (
    @EXIT /b %ERRORLEVEL%
)


@REM @REM The environment is setup, so let's run the application
@%PYTHON_EXE% "%~dp0\fox.py" %*

@REM if fox.py failed, exit with this error
@IF %ERRORLEVEL% NEQ 0 (
    @EXIT /b %ERRORLEVEL%
)
@POPD
