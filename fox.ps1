#!/usr/bin/env pwsh
#
# Copyright (c) 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

# Make all errors terminating errors
$ErrorActionPreference = "STOP"

# Unsupported operating systems
if ($IsLinux) {
    Write-Error "Use the 'fox.sh' instead."
    exit 1
}
elseif ($IsMacOS) {
    Write-Error "MacOS is currently not supported."
    exit 1
}

# Error handling when Python environment is not found
function InstallHelper($env_dir) {
    $PYTHON = "py"
    try {
        get-command "$PYTHON" | out-null
    }
    catch {
        # No python available at all
        Write-Host "Could not find $PYTHON."  -ForegroundColor Red
        Write-Host "Install Python from python.org and rerun the command." -ForegroundColor Red
        Pop-Location
        exit 1
    }
    $FALLBACK_SCRIPT = Join-Path "$PSScriptRoot" "cli" "fallback" "fallback.py"
    &$PYTHON "$FALLBACK_SCRIPT" "$env_dir"
    Pop-Location
    exit 1
}

# Push into the repository root
Push-Location "$PSScriptRoot"

$env:PREFIX = "C:\foxbms"

# Name of the Python environment
$env:ENV_NAME = "2025-03-pale-fox"

$env:FOXBMS_PYTHON_ENV_DIRECTORY = Join-Path "$env:PREFIX" "envs" "$env:ENV_NAME"

$env:FOXBMS_PYTHON_ACTIVATION_SCRIPT_REL_PATH = "Scripts\activate.ps1"

$env:FOXBMS_PYTHON_ACTIVATION_SCRIPT = Join-Path "$env:FOXBMS_PYTHON_ENV_DIRECTORY" "$env:FOXBMS_PYTHON_ACTIVATION_SCRIPT_REL_PATH"
# check if the activation script exists
if (-not (Test-Path -Path "$env:FOXBMS_PYTHON_ACTIVATION_SCRIPT")) {
    Write-Host "Could not find expected Python venv '$env:ENV_NAME' (expected directory '$env:FOXBMS_PYTHON_ENV_DIRECTORY' to exist)."  -ForegroundColor Red
    InstallHelper $env:FOXBMS_PYTHON_ENV_DIRECTORY # exists 1 in any case
}

# Activate Python environment
&"$env:FOXBMS_PYTHON_ACTIVATION_SCRIPT"

# Ensure that the Python executable is available
try {
    get-command "python" | out-null
}
catch {
    deactivate
    Pop-Location
    Write-Host "Could not find python executable."
    exit 1
}

# Environment is active and we have found a python executable,
# therefore we can run fox.py

# Special case if on Windows and the GUI shall open
if ($args.Contains("gui")) {
    pythonw "$PSScriptRoot\fox.py" "gui"
    if ($LastExitCode -ne 0) {
        deactivate
        Pop-Location
        exit 1
    }
    exit 0 # if GUI is requested, early exit
}

# Run fox.py, after running the command, deactivate the environment and exit
# with the fox.py exit code
python "$PSScriptRoot\fox.py" $args
if ($LastExitCode -ne 0) {
    deactivate
    Pop-Location
    exit 1
}
deactivate
Pop-Location
