#!/usr/bin/env bash
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
set -e

# Unsupported operating systems
if [ "$(uname)" == "Darwin" ]; then
    echo "MacOS is currently not supported."
    exit 1
elif [ "$(printf "%s" "$(uname -s)" | cut -c 1-9)" == "CYGWIN_NT" ]; then
    echo "Cygwin is not supported."
    exit 1
elif [ "$(printf "%s" "$(uname -s)" | cut -c 1-10)" == "MINGW32_NT" ]; then
    echo "32bit Windows is not supported."
    exit 1
fi

# Define prefix Linux
if [ "$(printf "%s" "$(uname -s)" | cut -c 1-5)" == "Linux" ]; then
    IsLinux="1"
# Define prefix Windows
elif [ "$(printf "%s" "$(uname -s)" | cut -c 1-10)" == "MINGW64_NT" ] || [ "$(printf "%s" "$(uname -s)" | cut -c 1-7)" == "MSYS_NT" ] ; then
    IsWindows="1"

fi

# Error handling when Python environment is not found
function InstallHelper() {
    env_dir="$1"
    if [ "$IsWindows" == "1" ]; then
        PYTHON="py"
    else
        PYTHON="python3"
    fi
    if ! command -v "$PYTHON" &> /dev/null
        then
    # No python available at all
        echo "Could not find $PYTHON."
        if [ "$IsWindows" == "1" ]; then
            echo "Install Python from python.org and rerun the command."
        else
            echo "Use your distributions package manager to install Python3."
        fi
        popd > /dev/null
        exit 1
    fi
    FALLBACK_SCRIPT="$SCRIPTDIR/cli/fallback/fallback.py"
    $PYTHON "$FALLBACK_SCRIPT" "$env_dir"
    popd > /dev/null
    exit 1
}

# Push into the repository root
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
pushd "$SCRIPTDIR" > /dev/null

# Define prefix Linux
if [ "$IsLinux" == "1" ]; then
    PREFIX="/opt/foxbms"
# Define prefix Windows
elif [ "$IsWindows" = "1" ] ; then
    PREFIX="/C/foxbms"
fi

# Name of the Python environment
ENV_NAME="2025-03-pale-fox"

FOXBMS_PYTHON_ENV_DIRECTORY="${PREFIX}/envs/${ENV_NAME}"

# Activation script path
if [ "$IsLinux" == "1" ]; then
    FOXBMS_PYTHON_ACTIVATION_SCRIPT_REL_PATH="bin/activate"
elif [ "$IsWindows" = "1" ] ; then
    FOXBMS_PYTHON_ACTIVATION_SCRIPT_REL_PATH="Scripts/activate"
fi

FOXBMS_PYTHON_ACTIVATION_SCRIPT="${FOXBMS_PYTHON_ENV_DIRECTORY}/${FOXBMS_PYTHON_ACTIVATION_SCRIPT_REL_PATH}"
# check if the activation script exists
if [ ! -f "${FOXBMS_PYTHON_ACTIVATION_SCRIPT}" ]; then
    echo "Could not find expected Python venv '${ENV_NAME}' (expected directory '${FOXBMS_PYTHON_ENV_DIRECTORY}' to exist)."
    InstallHelper "${FOXBMS_PYTHON_ENV_DIRECTORY}" # exists 1 in any case
fi
# Activate Python environment
# shellcheck source=/dev/null
source "$FOXBMS_PYTHON_ACTIVATION_SCRIPT"

# Ensure that the Python executable is available
if ! command -v python &> /dev/null
    then
    deactivate
    popd > /dev/null
    echo "Could not find python executable."
    exit 1
fi

# Environment is active and we have found a python executable,
# therefore we can run fox.py

# Run fox.py, after running the command, deactivate the environment and exit
# with the fox.py exit code
python "${SCRIPTDIR}/fox.py" "$@" || rc="$?"
deactivate
popd > /dev/null
exit $((rc))
