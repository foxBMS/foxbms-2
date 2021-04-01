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

@SETLOCAL EnableDelayedExpansion
@SETLOCAL EnableExtensions
@SETLOCAL

@TITLE Installing LLVM

@SET LLVM_BASE_INSTALL_DIRECTORY=C:\Program Files\LLVM
@SET LLVM_VERSION=11.0.1
@SET LLVM_INSTALL_DIRECTORY=%LLVM_BASE_INSTALL_DIRECTORY%\%LLVM_VERSION%
@SET LLVM_DOWNLOAD_URL=https://github.com/llvm/llvm-project/releases/download/llvmorg-11.0.1/LLVM-11.0.1-win64.exe
@SET LLVM_INSTALLER="%USERPROFILE%\Downloads\llvm-%LLVM_VERSION%-win64.exe"

@ECHO Downloading LLVM
@curl.exe -fsSL %LLVM_DOWNLOAD_URL% -o %LLVM_INSTALLER%
@ECHO Done
@ECHO Installing LLVM %LLVM_VERSION%
@%LLVM_INSTALLER% /S /tasks="noassocfiles,nomodpath" /D=%LLVM_INSTALL_DIRECTORY%
@ECHO Done
