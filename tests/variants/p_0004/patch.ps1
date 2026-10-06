#!/usr/bin/env pwsh
#
# Copyright (c) 2010 - 2026, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# We kindly request you to use one or more of the following phrases to refer to
# foxBMS in your hardware, software, documentation or advertising materials:
#
# - "This product uses parts of foxBMS®"
# - "This product includes parts of foxBMS®"
# - "This product is derived from foxBMS®"

#Requires -Version 7.4

# Make all error terminating errors
$ErrorActionPreference = "STOP"
$PSNativeCommandUseErrorActionPreference = $true

Push-Location "$PSScriptRoot"

$env:REPO_ROOT = (git rev-parse --show-toplevel) -join "`n"

Pop-Location

Push-Location "$env:REPO_ROOT"

.\fox.ps1 waf configure -v -c yes

# Create the bootstrap project
.\fox.ps1 waf bootstrap_library_project -v -c yes

# Create a directory for extracted files
$env:LIB_BUILD_DIR = Join-Path -Path "$env:REPO_ROOT" -ChildPath  "lib-build"

New-Item -Type Directory "$env:LIB_BUILD_DIR" -Force

$tmp = Join-Path -Path "$env:REPO_ROOT" -ChildPath  "library-project.tar.gz"

C:\Windows\System32\tar.exe -jxf $tmp --directory "$env:LIB_BUILD_DIR"

Pop-Location

# Build the library
Push-Location "$env:LIB_BUILD_DIR"
&"$env:LIB_BUILD_DIR\fox.ps1" waf --cwd "$env:LIB_BUILD_DIR" configure build -v -c yes
Pop-Location

Push-Location "$PSScriptRoot"
git apply "lib-build.patch"
Pop-Location
