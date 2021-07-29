#!/usr/bin/env bash
#
# Copyright (c) 2010 - 2021, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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
# - "This product uses parts of foxBMS&reg;"
# - "This product includes parts of foxBMS&reg;"
# - "This product is derived from foxBMS&reg;"

set -e
if [ $# -eq 0 ]; then
    echo "No script file to run specified."
    exit 1
fi

if [ ! -f $1 ]; then
    echo "Script '$1' does not exist."
    exit 1
fi

# MacOS
if [ "$(uname)" == "Darwin" ]; then
    echo "MacOS is currently not supported."
    exit 1
# Linux
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
    while read -r line; do
        PATHS_TO_ADD=`echo "${PATHS_TO_ADD}:$line"| sed --expression='s/^M/:/g'`
    done < ${SCRIPTDIR}/../../../conf/env/paths_linux.txt
    PATHS_TO_ADD=`echo $PATHS_TO_ADD | awk '{gsub("C:", "/c", $0); print}'`
    PATHS_TO_ADD=$(echo "${PATHS_TO_ADD#?}" | tr '\\' '/')
    export PATH=$PATHS_TO_ADD:$PATH
    # call find_base_conda and make sure that we do not exit by printing
    # the exit code to CONDA_VARS (otherwise we would exit with set -e, here
    # we will not as echo returns exit code 0)
    CONDA_VARS=$($SCRIPTDIR/../../../tools/utils/bash/find_base_conda.sh || echo $?)
    if [ "${CONDA_VARS: -1}" == "1" ]; then
        # strip the exit code that we have printed to CONDA_VARS and
        # print the rest
        echo "${CONDA_VARS: : -1}"
        exit 1
    fi
    CONDA_VARS_ARRAY=($CONDA_VARS)
    CONDA_BASE_ENVIRONMENT_INCLUDING_DEVELOPMENT_ENVIRONMENT=${CONDA_VARS_ARRAY[0]}
    CONDA_BASE_ENVIRONMENT_ACTIVATE_SCRIPT=${CONDA_VARS_ARRAY[1]}
    CONDA_DEVELOPMENT_ENVIRONMENT_NAME=${CONDA_VARS_ARRAY[2]}
    CONDA_DEVELOPMENT_ENVIRONMENT_CONFIGURATION_FILE=${CONDA_VARS_ARRAY[3]}
    source $CONDA_BASE_ENVIRONMENT_ACTIVATE_SCRIPT base
    conda activate ${CONDA_DEVELOPMENT_ENVIRONMENT_NAME}
    set + # deactivate in any case the environment, therefore ignore errors
    python "$1" "${@:2}"
    set -
    conda deactivate
# Windows
elif [ "$(expr substr $(uname -s) 1 9)" == "CYGWIN_NT" ]; then
    echo "Cygwin is not supported."
    exit 1
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
    echo "32bit Windows is not supported."
    exit 1
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ] || [ "$(expr substr $(uname -s) 1 7)" == "MSYS_NT" ] ; then
    CODEPAGE=`chcp.com | sed 's/[^0-9]*//g'`
    if [ "$CODEPAGE" != "850" ]; then
        chcp.com 850 >/dev/null 2>&1
    fi
    SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
    while read -r line; do
        PATHS_TO_ADD=`echo "${PATHS_TO_ADD}:$line"| sed --expression='s/^M/:/g'`
    done < ${SCRIPTDIR}/../../../conf/env/paths_win32.txt
    PATHS_TO_ADD=`echo $PATHS_TO_ADD | awk '{gsub("C:", "/c", $0); print}'`
    PATHS_TO_ADD=$(echo "${PATHS_TO_ADD#?}" | tr '\\' '/')
    export PATH=$PATHS_TO_ADD:$PATH
    CONDA_VARS=$($SCRIPTDIR/find_base_conda.sh)
    CONDA_VARS_ARRAY=($CONDA_VARS)
    CONDA_BASE_ENVIRONMENT_INCLUDING_DEVELOPMENT_ENVIRONMENT=${CONDA_VARS_ARRAY[0]}
    CONDA_BASE_ENVIRONMENT_ACTIVATE_SCRIPT=${CONDA_VARS_ARRAY[1]}
    CONDA_DEVELOPMENT_ENVIRONMENT_NAME=${CONDA_VARS_ARRAY[2]}
    CONDA_DEVELOPMENT_ENVIRONMENT_CONFIGURATION_FILE=${CONDA_VARS_ARRAY[3]}
    source $CONDA_BASE_ENVIRONMENT_ACTIVATE_SCRIPT base
    conda activate ${CONDA_DEVELOPMENT_ENVIRONMENT_NAME}
    python.exe "$1" "${@:2}"
    conda deactivate
    chcp.com $CODEPAGE >/dev/null 2>&1
fi
