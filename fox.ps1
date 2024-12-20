#!/usr/bin/env pwsh
#
# Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

# Make all error terminating errors
$ErrorActionPreference = "STOP"

Push-Location "$PSScriptRoot"

$env:ENV_NAME = "2025-01-pale-fox"
$env:ENV_DIR = "foxbms-envs"

if ($IsLinux) {
    $env:FOXBMS_PYTHON_ENV_DIRECTORY_USER = "$env:HOME/$env:ENV_DIR/$env:ENV_NAME"
    $env:FOXBMS_PYTHON_ENV_DIRECTORY_ROOT = "/opt/$env:ENV_DIR/$env:ENV_NAME"
    $env:FOXBMS_PYTHON_ACTIVATION_SCRIPT_REL_PATH = "bin/activate"
}
elseif ($IsMacOS) {
    Write-Error "MacOS is currently not supported."
    exit 1
}
elseif ($IsWindows) {
    $env:FOXBMS_PYTHON_ENV_DIRECTORY_USER = "$env:USERPROFILE\$env:ENV_DIR\$env:ENV_NAME"
    $env:FOXBMS_PYTHON_ENV_DIRECTORY_ROOT = "C:\$env:ENV_DIR\$env:ENV_NAME"
    $env:FOXBMS_PYTHON_ACTIVATION_SCRIPT_REL_PATH = "Scripts\activate.ps1"
}
else {
    # assume Windows anyway
    $env:FOXBMS_PYTHON_ENV_DIRECTORY_USER = "$env:USERPROFILE\$env:ENV_DIR\$env:ENV_NAME"
    $env:FOXBMS_PYTHON_ENV_DIRECTORY_ROOT = "C:\$env:ENV_DIR\$env:ENV_NAME"
    $env:FOXBMS_PYTHON_ACTIVATION_SCRIPT_REL_PATH = "Scripts\activate.ps1"
}

$env:FOXBMS_PYTHON_ENV_DIRECTORY = "$env:FOXBMS_PYTHON_ENV_DIRECTORY_USER"

# if the user env directory does not exist, use root
if (-not (Test-Path -Path "$env:FOXBMS_PYTHON_ENV_DIRECTORY_USER")) {
    $env:FOXBMS_PYTHON_ENV_DIRECTORY = "$env:FOXBMS_PYTHON_ENV_DIRECTORY_ROOT"
}

$env:FOXBMS_PYTHON_ACTIVATION_SCRIPT = Join-Path "$env:FOXBMS_PYTHON_ENV_DIRECTORY" "$env:FOXBMS_PYTHON_ACTIVATION_SCRIPT_REL_PATH"
# check if the activation script exists
if (-not (Test-Path -Path "$env:FOXBMS_PYTHON_ACTIVATION_SCRIPT")) {
    Write-Error "Could not find expected Python venv '$env:ENV_NAME'. Exiting..."
    exit 1
}

&"$env:FOXBMS_PYTHON_ACTIVATION_SCRIPT"

try {
    get-command "python" | out-null
}
catch {
    deactivate
    Pop-Location
    exit 1
}

if ($args.Contains("gui")) {
    pythonw "$PSScriptRoot\fox.py" "gui"
    if ($LastExitCode -ne 0) {
        deactivate
        Pop-Location
        exit 1
    }
}
else {
    python "$PSScriptRoot\fox.py" $args
    if ($LastExitCode -ne 0) {
        deactivate
        Pop-Location
        exit 1
    }
}
deactivate
Pop-Location
