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

@SETLOCAL enabledelayedexpansion

@REM set working directory to script directory
@PUSHD %~dp0

@REM get conda environment name
@CALL :NORMALIZEPATH "%~dp0..\..\..\conf\env\conda_env_win32.yaml"
@SET CONDA_ENV_FILE=%NORMALIZED_PATH%
@FOR /f "usebackq tokens=*" %%A IN ("%CONDA_ENV_FILE%") DO @(
    @FOR /F "tokens=1" %%B IN ("%%A") DO @(
        @IF "%%B" equ "name:" (
            @FOR /f "tokens=2" %%G IN ("%%A") DO @SET CONDA_DEVELOPMENT_ENVIRONMENT_NAME=%%G
        )
    )
)

@REM miniconda installation directories
@IF not "%USERPROFILE%"=="%USERPROFILE: =%" @(
    @SET MINICONDA_DIRECTORIES=%SystemDrive%\miniconda3\ %ProgramData%\miniconda3\
) ELSE @(
    @SET MINICONDA_DIRECTORIES=%USERPROFILE%\miniconda3\ %LOCALAPPDATA%\Continuum\miniconda3\ %SystemDrive%\miniconda3\ %ProgramData%\miniconda3\
)

@REM Activate script path (relative to the environment directory)
@REM File extension not necessary as .bat are executable by default on Windows
@REM (and the activate script is a batch script)
@SET ACTIVATE_SCRIPT=Scripts\activate.bat

@REM Loop over all known default installation directories of miniconda
@REM If we find a matching activate script, we assume it is a conda valid
@REM environment
@SET BASE_ENVS_FOUND=
@FOR %%x IN (%MINICONDA_DIRECTORIES%) DO @(
    @IF EXIST %%x%ACTIVATE_SCRIPT% (
        @SET BASE_ENVS_FOUND=!BASE_ENVS_FOUND! %%x
    )
)

@IF "%BASE_ENVS_FOUND%"=="" (
    @ECHO [31mCould not find conda environment in default directories [%MINICONDA_DIRECTORIES%][0m
    @ECHO [31mExiting...[0m
    @EXIT /b 1
)

@REM Loop over all found conda base environments and check if there is an
@REM environment with the correct name that includes a python.exe
@SET DEVEL_ENV_FOUND=
@FOR %%x IN (%BASE_ENVS_FOUND%) DO @(
    @IF EXIST %%xenvs\%CONDA_DEVELOPMENT_ENVIRONMENT_NAME%\python.exe (
        @SET DEVEL_ENV_FOUND=%%x
    )
)

@REM maybe we found the enviroment name not, but maybe this is on purpose as
@REM the user wants to run an conda environment update
@IF "%1"=="update" (
    @FOR %%i in (%BASE_ENVS_FOUND%) do @(
        @SET DEVEL_ENV_FOUND=%%i
        @GOTO done
    )
)
:done

@IF "%DEVEL_ENV_FOUND%"=="" (
    @ECHO [31mCould not find conda development environment directories [%BASE_ENVS_FOUND%][0m
    @ECHO [31mRun %~dp0..\conda-update-env.bat[0m
    @ECHO [31mExiting...[0m
    @EXIT /b 1
)

@ENDLOCAL & (
    @SET "CONDA_BASE_ENVIRONMENT_INCLUDING_DEVELOPMENT_ENVIRONMENT=%DEVEL_ENV_FOUND%"
    @SET "CONDA_BASE_ENVIRONMENT_ACTIVATE_SCRIPT=%DEVEL_ENV_FOUND%%ACTIVATE_SCRIPT%"
    @SET "CONDA_DEVELOPMENT_ENVIRONMENT_NAME=%CONDA_DEVELOPMENT_ENVIRONMENT_NAME%"
    @SET "CONDA_DEVELOPMENT_ENVIRONMENT_CONFIGURATION_FILE=%CONDA_ENV_FILE%"
)

@POPD

:NORMALIZEPATH
    @SET NORMALIZED_PATH=%~dpfn1
    @EXIT /B
