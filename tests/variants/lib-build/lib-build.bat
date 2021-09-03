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

@REM This scripts works only when foxBMS 2 is developed inside a git repository


@FOR /F "tokens=* USEBACKQ" %%F IN (`git rev-parse --show-toplevel`) DO @(
    @SET REPO_ROOT=%%F
)

@REM go the repository root
@cd %REPO_ROOT%

@call waf.bat bootstrap_library_project -v -c yes
@IF %ERRORLEVEL% NEQ 0 (
    @EXIT /b %ERRORLEVEL%
)

@mkdir libbuild

@tar -jxf library-project.tar.bz2 --directory libbuild
@IF %ERRORLEVEL% NEQ 0 (
    @EXIT /b %ERRORLEVEL%
)

@REM Build the library
@cd libbuild
@CALL waf.bat configure build -v -c yes
@IF %ERRORLEVEL% NEQ 0 (
    @EXIT /b %ERRORLEVEL%
)
@cd ..

@REM Copy file that includes the library header and uses a function from it
@copy /y tests\variants\lib-build\lib-build_cc-options.yaml  conf\cc\cc-options.yaml
@IF %ERRORLEVEL% NEQ 0 (
    @EXIT /b %ERRORLEVEL%
)
@copy /y tests\variants\lib-build\lib-build_main.c           src\app\main\main.c
@IF %ERRORLEVEL% NEQ 0 (
    @EXIT /b %ERRORLEVEL%
)

@REM Fix doxygen comment
@Powershell.exe -Command "((Get-Content -path .\src\app\main\main.c -Raw) -replace '@file    lib-build_main.c','@file    main.c') | Set-Content -NoNewline -Path .\src\app\main\main.c"
@IF %ERRORLEVEL% NEQ 0 (
    @EXIT /b %ERRORLEVEL%
)

@REM Fix library and header path
@Powershell.exe -Command "((Get-Content -path .\conf\cc\cc-options.yaml -Raw) -replace 'REPO_ROOT', $env:REPO_ROOT) | Set-Content -NoNewline -Path .\conf\cc\cc-options.yaml"
@IF %ERRORLEVEL% NEQ 0 (
    @EXIT /b %ERRORLEVEL%
)

@REM Build the library
@CALL waf.bat configure -v -c yes
@IF %ERRORLEVEL% NEQ 0 (
    @EXIT /b %ERRORLEVEL%
)

@CALL waf.bat build_bin -v -c yes
@IF %ERRORLEVEL% NEQ 0 (
    @EXIT /b %ERRORLEVEL%
)
