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

@TITLE Installing Ruby

@ECHO Downloading Ruby
@SET RUBYINSTALLER="%USERPROFILE%\Downloads\rubyinstaller-devkit-2.7.2-1-x64.exe"
@SET RUBY_INSTALL_DIRECTORY=C:\Ruby\Ruby272-x64
@curl.exe -fsSL https://github.com/oneclick/rubyinstaller2/releases/download/RubyInstaller-2.7.2-1/rubyinstaller-devkit-2.7.2-1-x64.exe -o "%USERPROFILE%\Downloads\rubyinstaller-devkit-2.7.2-1-x64.exe"
@ECHO Done
@ECHO Installing Ruby 2.7.2-x64
@%RUBYINSTALLER% /verysilent /dir="%RUBY_INSTALL_DIRECTORY%" /tasks="noassocfiles,nomodpath,noridkinstall"
@ECHO Done
@ECHO Installing dependencies
@PUSHD %~dp0..\vendor\ceedling
@ECHO %__CD__%
@%RUBY_INSTALL_DIRECTORY%\bin\bundle install %1
@POPD
@ECHO Done
