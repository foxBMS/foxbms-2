@REM @copyright &copy; 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der
@REM  angewandten Forschung e.V. All rights reserved.
@REM
@REM BSD 3-Clause License
@REM Redistribution and use in source and binary forms, with or without
@REM modification, are permitted provided that the following conditions are
@REM met:
@REM 1.  Redistributions of source code must retain the above copyright notice,
@REM     this list of conditions and the following disclaimer.
@REM 2.  Redistributions in binary form must reproduce the above copyright
@REM     notice, this list of conditions and the following disclaimer in the
@REM     documentation and/or other materials provided with the distribution.
@REM 3.  Neither the name of the copyright holder nor the names of its
@REM     contributors may be used to endorse or promote products derived from
@REM     this software without specific prior written permission.
@REM
@REM THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
@REM IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
@REM THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
@REM PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
@REM CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
@REM EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
@REM PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
@REM PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
@REM LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
@REM NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
@REM SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
@REM
@REM We kindly request you to use one or more of the following phrases to refer
@REM to foxBMS in your hardware, software, documentation or advertising
@REM materials:
@REM
@REM &Prime;This product uses parts of foxBMS&reg;&Prime;
@REM
@REM &Prime;This product includes parts of foxBMS&reg;&Prime;
@REM
@REM &Prime;This product is derived from foxBMS&reg;&Prime;

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
    @ECHO Could not find conda environment in default directories [%MINICONDA_DIRECTORIES%]
    @ECHO Exiting...
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
@IF "%1"=="update" (
    @FOR %%i in (%BASE_ENVS_FOUND%) do @(
        @SET DEVEL_ENV_FOUND=%%i
        @GOTO done
    )
)
:done

@IF "%DEVEL_ENV_FOUND%"=="" (
    @ECHO Could not find conda development environment directories [%BASE_ENVS_FOUND%]
    @ECHO Run %~dp0..\conda-update-env.bat
    @ECHO Exiting...
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
