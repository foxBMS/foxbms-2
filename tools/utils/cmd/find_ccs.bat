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

@SETLOCAL EnableDelayedExpansion EnableExtensions

@SET CCS_VERSIONS=
@IF EXIST %SystemDrive%\ti @(
    @FOR /D %%G in (%SystemDrive%\ti\ccs*) do @(
        @SET CCS_INSTALL_PATH=%%G
        @SET /a CCS_VERSION=!CCS_INSTALL_PATH:~9!
        @SET CCS_VERSIONS=!CCS_VERSIONS! !CCS_VERSION!
    )
)

@SET CCS_VERSIONS=!CCS_VERSIONS:~1!

@SET i=-1
@FOR %%f in (!CCS_VERSIONS!) do @(
    @SET /a i=!i!+1
    @SET names[!i!]=%%f
)
@SET lastindex=!i!

@SET /a USE_CCS_VERSION=0
@FOR /L %%f in (0,1,!lastindex!) do @(
    @SET /a nr=!names[%%f]!
    @IF !nr! GTR !USE_CCS_VERSION! @(
        @SET USE_CCS_VERSION=!names[%%f]!
    )
)

:CONTINUE
@for /f "delims=" %%a in ('dir /b C:\ti\ccs!USE_CCS_VERSION!\ccs\tools\compiler\ti-cgt-arm_*') do @(
    @SET "COMPILER_VERSION=%%a"
)

@ENDLOCAL & (
    @SET CCS_COMPILER_BIN=%SystemDrive%\ti\ccs%USE_CCS_VERSION%\ccs\tools\compiler\%COMPILER_VERSION%\bin
    @SET CCS_COMPILER_LIB=%SystemDrive%\ti\ccs%USE_CCS_VERSION%\ccs\tools\compiler\%COMPILER_VERSION%\lib
    @SET CCS_UTILS_BIN=%SystemDrive%\ti\ccs%USE_CCS_VERSION%\ccs\utils\bin
    @SET CCS_UTILS_CYGWIN=%SystemDrive%\ti\ccs%USE_CCS_VERSION%\ccs\utils\cygwin
    @SET CCS_UTILS_TIOBJ2BIN=%SystemDrive%\ti\ccs%USE_CCS_VERSION%\ccs\utils\tiobj2bin
)
