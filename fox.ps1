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

<#
.SYNOPSIS
Runs the foxBMS entry-point script with the configured Python environment.
#>

# Make all errors terminating errors
$ErrorActionPreference = "STOP"

if ($PSVersionTable.PSVersion.Major -lt 7) {
    Write-Host "This script requires PowerShell 7 or later." -ForegroundColor Red
    exit 1
}

# Unsupported operating systems
if ($IsMacOS) {
    Write-Host "MacOS is currently not supported." -ForegroundColor Red
    exit 1
}
elseif ($IsLinux) {
    Write-Host "Use the 'fox.sh' script instead." -ForegroundColor Red
    exit 1
}

$PythonEnvironmentInstallationProcess = @"
<Window xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
        Title="Installing"
        SizeToContent="WidthAndHeight"
        WindowStartupLocation="CenterScreen"
        ResizeMode="NoResize"
        WindowStyle="ToolWindow">
  <StackPanel Margin="30">
    <TextBlock TextAlignment="Center" FontSize="14">
      Installation in progress...
    </TextBlock>
    <ProgressBar Minimum="0" Maximum="100" IsIndeterminate="True" Height="20" Margin="0,20,0,0"/>
  </StackPanel>
</Window>
"@

if ($args.Contains("gui")) {
    Add-Type -AssemblyName PresentationFramework
}

# Error handling when Python environment is not found
function InstallHelper($env_dir, $cli_args) {
    $PYTHON = "py"
    try {
        get-command "$PYTHON" | out-null
    }
    catch {
        # No python available at all
        if ($cli_args.Contains("gui")) {
            # throw a graphical error message in case the script was started in
            # GUI mode ('fox.ps1 gui' or 'gui.lnk'.)
            [System.Windows.MessageBox]::Show("Could not find '$PYTHON' executable.`nInstall Python3 from python.org and rerun the command.", "Dependency Error", "OK", "Error") | Out-Null
        } else {
            Write-Host "Could not find '$PYTHON' executable." -ForegroundColor Red
            Write-Host "Install Python3 from python.org and rerun the command." -ForegroundColor Red
            Pop-Location
        }
        # exit as we miss the most basic dependency
        exit 1
    }

    # we have at least some 'py' executable.
    $PYTHON_SETUP_SCRIPT = Join-Path "$PSScriptRoot" "cli" "helpers" "python_setup.py"
    if ($cli_args.Contains("gui")) {
        # in GUI mode we need to ensure that 'py -3.12' succeeds in the
        # python_setup.py script, so we need to run this test here also and exit
        # in case Python3.12 is not available.
        py -3.12 --version *> $null
        if ($LASTEXITCODE -ne 0) {
            [System.Windows.MessageBox]::Show("The 'py' executable exists, but Python3.12 is required. Go to https://www.python.org, download Python3.12 and rerun the command.", "Error", "OK", "Error") | Out-Null
            exit 1
        }

        # we have Python3.12, so let's decide whether the user wants to
        # automatically install the Python environment
        $result = [System.Windows.MessageBox]::Show("The Python environment is missing.`nDo you want to install the Python environment?", "Confirm Installation", "YesNo", "Question")
        if ($result -eq "Yes") {

            $reader = [System.Xml.XmlReader]::Create([System.IO.StringReader]$PythonEnvironmentInstallationProcess)
            $window = [Windows.Markup.XamlReader]::Load($reader)

            # Start the Python environment installation script as a background process
            $psi = New-Object System.Diagnostics.ProcessStartInfo
            $psi.FileName = $PYTHON
            # user provides confirmation through dialog
            $psi.Arguments = "$PYTHON_SETUP_SCRIPT $env_dir --confirm"
            $psi.UseShellExecute = $false
            $psi.RedirectStandardOutput = $true
            $psi.RedirectStandardError = $true

            $process = New-Object System.Diagnostics.Process
            $process.StartInfo = $psi
            $process.Start() | Out-Null

            $timer = New-Object System.Windows.Threading.DispatcherTimer
            $timer.Interval = [TimeSpan]::FromMilliseconds(500)
            $timer.Add_Tick({
                if ($process.HasExited) {
                    $timer.Stop()
                    $window.Close()
                }
            })

            $timer.Start()

            # ShowDialog blocks until closed
            $window.ShowDialog() | Out-Null

            # After the installation finished, inform the user about the result
            # In case of an installation error, remove the invalid environment
            if ($process.ExitCode -eq 0) {
                [System.Windows.MessageBox]::Show("Environment installation completed successfully.", "Success", "OK", "Information")
            } else {
                [System.Windows.MessageBox]::Show("Environment installation failed.", "Error", "OK", "Error")
                # Remove directory that contains the failed installation
                Remove-Item -Path "$env_dir" -Recurse -Force
            }
        } else {
            [System.Windows.MessageBox]::Show("Installation aborted.", "Error", "OK", "Error")
        }
    } else {
        # in non GUI mode, everything is handled in the Python script
        &$PYTHON "$PYTHON_SETUP_SCRIPT" "$env_dir"
        Pop-Location
    }
    exit 1
}

# Push into the repository root
Push-Location "$PSScriptRoot"

# foxBMS-prefix for installed tools
$env:PREFIX = "C:\foxbms"

# Name of the Python environment
$env:ENV_NAME = "2025-11-pale-fox"

$env:FOXBMS_PYTHON_ENV_DIRECTORY = Join-Path "$env:PREFIX" "envs" "$env:ENV_NAME"

# Activation script path
$env:FOXBMS_PYTHON_ACTIVATION_SCRIPT_REL_PATH = "Scripts\activate.ps1"

$env:FOXBMS_PYTHON_ACTIVATION_SCRIPT = Join-Path "$env:FOXBMS_PYTHON_ENV_DIRECTORY" "$env:FOXBMS_PYTHON_ACTIVATION_SCRIPT_REL_PATH"
# check if the activation script exists
if (-not (Test-Path -Path "$env:FOXBMS_PYTHON_ACTIVATION_SCRIPT")) {
    Write-Host "Could not find expected Python venv '$env:ENV_NAME' (expected directory '$env:FOXBMS_PYTHON_ENV_DIRECTORY' to exist)." -ForegroundColor Red
    InstallHelper $env:FOXBMS_PYTHON_ENV_DIRECTORY $args # exists 1 in any case
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

# Special case: if on Windows and the GUI shall open
if ($args.Contains("gui")) {
    # by default use 'pythonw.exe' so that we can early exit after GUI start
    # in case we need to debug the gui and provide the debug option,
    # we need stdout and stderr, so we start the GUI 'blocking' using
    # 'python.exe'
    $USE_PYTHON = "pythonw"

    # arguments that require 'python.exe' to be used
    if ($args.Contains("-h") -or $args.Contains("--help") -or $args.Contains("--debug-gui")) {
        $USE_PYTHON = "python"
    }

    # desired python executable is now defined
    &$USE_PYTHON "$PSScriptRoot\fox.py" $args

    if ($LastExitCode -ne 0) {
        deactivate
        Pop-Location
        exit 1
    }

    deactivate
    Pop-Location
    # if GUI is requested, early exit
    exit 0
}

# Run fox.py, after running the command, deactivate the environment and exit
# with the fox.py exit code
# Special case: the 'bms' commands need to read from stdin and if stdin is
# empty (as it should be as the invocation is just 'fox.py bms' and some
# arguments), we would pipe an empty to string to stdin and this is no valid
# input in Python's cmd.Cmd implementation. Therefore we just do not pipe
# $input to the script. Note: this is not required in fox.sh as bash handles
# stdin piping different.
if (!($args.Contains("bms") -or $args.Contains("install"))) {
    Write-Output "$input" | python "$PSScriptRoot\fox.py" $args
} else {
    python "$PSScriptRoot\fox.py" $args
}

if ($LastExitCode -ne 0) {
    deactivate
    Pop-Location
    exit 1
}
deactivate
Pop-Location
