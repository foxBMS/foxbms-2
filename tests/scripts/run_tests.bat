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

@SET BASE_DIR=%~dp0\..\..
@PUSHD %BASE_DIR%\tests\scripts
@ECHO =========================================================================
@ECHO Unit testing waf-tools
@CALL %BASE_DIR%\tools\utils\cmd\run-python-coverage.bat run --source=%BASE_DIR%\tools\waf-tools -m unittest discover --start-directory waf-tools --top-level-directory .
@IF %ERRORLEVEL% NEQ 0 (
    @ECHO [31mUnit testing waf tools failed[0m
    @EXIT /b %ERRORLEVEL%
) ELSE (
    @ECHO [32mUnit testing waf tools successful[0m
)
@ECHO =========================================================================
@ECHO Unit testing GUI
@CALL %BASE_DIR%\tools\utils\cmd\run-python-coverage.bat run --append --source=%BASE_DIR%\tools\gui -m unittest discover --start-directory gui --top-level-directory .
@IF %ERRORLEVEL% NEQ 0 (
    @ECHO [31mUnit testing the GUI failed[0m
    @POPD
    @EXIT /b %ERRORLEVEL%
) ELSE (
    @ECHO [32mUnit testing GUI successful[0m
)
@ECHO =========================================================================
@ECHO Creating report
@CALL %BASE_DIR%\tools\utils\cmd\run-python-coverage.bat report
@IF %ERRORLEVEL% NEQ 0 (
    @ECHO [31mCould not create coverage report[0m
    @POPD
    @EXIT /b %ERRORLEVEL%
) ELSE (
    @ECHO [32mCoverage Report created[0m
)
@CALL %BASE_DIR%\tools\utils\cmd\run-python-coverage.bat html --directory=%BASE_DIR%\build\unit_test_scripts\
@IF %ERRORLEVEL% NEQ 0 (
    @ECHO [31mCould not create html coverage report[0m
    @POPD
    @EXIT /b %ERRORLEVEL%
) ELSE (
    @ECHO [32mHTML Coverage Report created[0m
)
@CALL %BASE_DIR%\tools\utils\cmd\run-python-coverage.bat xml -o %BASE_DIR%\build\unit_test_scripts\CoberturaCoverageScripts.xml
@IF %ERRORLEVEL% NEQ 0 (
    @ECHO [31mCould not create xml coverage report[0m
    @POPD
    @EXIT /b %ERRORLEVEL%
) ELSE (
    @ECHO [32mXML Coverage Report created[0m
)
@ECHO =========================================================================
@ECHO Done
@POPD
