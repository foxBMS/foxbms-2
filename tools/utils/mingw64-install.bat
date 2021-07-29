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

@TITLE Installing MinGW64

@SET MINGW64_BASE_INSTALL_DIRECTORY=C:\mingw64
@SET MINGW64_VERSION=x86_64-8.1.0-release-posix-seh-rt_v6-rev0
@SET MINGW64_INSTALL_DIRECTORY=%MINGW64_BASE_INSTALL_DIRECTORY%\%MINGW64_VERSION%
@SET MINGW64_DOWNLOAD_URL=https://sourceforge.net/projects/mingw-w64/files/Toolchains%%20targetting%%20Win64/Personal%%20Builds/mingw-builds/8.1.0/threads-posix/seh/%MINGW64_VERSION%.7z/download
@SET MINGW64_INSTALLER="%USERPROFILE%\Downloads\%MINGW64_VERSION%.7z"

@ECHO Downloading MinGW64
@curl.exe -fsSL %MINGW64_DOWNLOAD_URL% -o %MINGW64_INSTALLER%
@ECHO Done

@IF NOT EXIST %MINGW64_BASE_INSTALL_DIRECTORY% @(
    @ECHO Creating directory
    @MKDIR %MINGW64_BASE_INSTALL_DIRECTORY%
    @MKDIR %MINGW64_INSTALL_DIRECTORY%
    @ECHO Done
)

@IF NOT EXIST "C:\Program Files\7-Zip\7z.exe" @(
    @ECHO [31mAutomated instllation failed as 7-zip is missing.[0m
    @ECHO Download 7-Zip from https://7-zip.org/ and install it in the default directory
    @ECHO After that re-run this script
    @PAUSE
)


@ECHO Installing MinGW64 %MINGW64_VERSION%
@"C:\Program Files\7-Zip\7z.exe" x %MINGW64_INSTALLER% -o%MINGW64_INSTALL_DIRECTORY%
@ECHO Done
