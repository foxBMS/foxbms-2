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

function _win32_get_vars() {
    # get conda environment name
    SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
    CONDA_ENV_FILE=$(readlink -f "${SCRIPTDIR}/../../../conf/env/conda_env_win32.yaml")
    LINE_WITH_NAME=`sed '1q;d' $CONDA_ENV_FILE`
    CONDA_DEVELOPMENT_ENVIRONMENT_NAME=`echo "$LINE_WITH_NAME" | cut -d":" -f 2 | xargs`

    # miniconda installation directories
    if [[ $USERPROFILE = *" "* ]]; then
        MINICONDA_DIRECTORIES=($SYSTEMDRIVE/miniconda3/ $ProgramData/miniconda3/)
    else
        MINICONDA_DIRECTORIES=($USERPROFILE/miniconda3/ $LOCALAPPDATA/Continuum/miniconda3/ $SYSTEMDRIVE/miniconda3/ $ProgramData/miniconda3/)
    fi

    # Activate script path (relative to the environment directory)
    # File extension not necessary as .bat are executable by default on Windows
    # (and the activate script is a batch script)
    ACTIVATE_SCRIPT=Scripts/activate

    # Loop over all known default installation directories of miniconda
    # If we find a matching activate script, we assume it is a conda valid
    # environment
    BASE_ENVS_FOUND=()
    for env in "${MINICONDA_DIRECTORIES[@]}"
    do
        if [ -f "$env$ACTIVATE_SCRIPT" ]; then
            BASE_ENVS_FOUND+=("$env")
        fi
    done

    if [ ${#BASE_ENVS_FOUND[@]} -eq 0 ]; then
        echo "Could not find conda environment in default directories [${MINICONDA_DIRECTORIES[@]}]"
        echo "Exiting..."
        exit 1
    fi

    # Loop over all found conda base environments and check if there is an
    # environment with the correct name that includes a python.exe
    ENVSDIR="envs/"
    DEVEL_ENV_FOUND=""
    for base_env in "${BASE_ENVS_FOUND[@]}"
    do
        if [ -f "$base_env$ENVSDIR$CONDA_DEVELOPMENT_ENVIRONMENT_NAME/python.exe" ]; then
            DEVEL_ENV_FOUND="$base_env"
            break
        fi
        if [ -f "${HOME}/.conda/${ENVSDIR}${CONDA_DEVELOPMENT_ENVIRONMENT_NAME}/python.exe" ]; then
            DEVEL_ENV_FOUND="$base_env"
            break
        fi
    done

    # maybe we found the enviroment name not, but maybe this is on purpose as
    # the user wants to run an conda environment update
    if [ "${1}" == "update" ]; then
        DEVEL_ENV_FOUND="$base_env"
    fi

    if [ -z "${DEVEL_ENV_FOUND}" ]; then
        echo "Could not find conda development environment directories [${BASE_ENVS_FOUND}]"
        echo "Run '$SCRIPTDIR/../conda-update-env.sh'"
        echo "Exiting..."
        exit 1
    fi

    CONDA_BASE_ENVIRONMENT_INCLUDING_DEVELOPMENT_ENVIRONMENT=$DEVEL_ENV_FOUND
    CONDA_BASE_ENVIRONMENT_ACTIVATE_SCRIPT=$DEVEL_ENV_FOUND$ACTIVATE_SCRIPT
    CONDA_DEVELOPMENT_ENVIRONMENT_NAME=$CONDA_DEVELOPMENT_ENVIRONMENT_NAME
    CONDA_DEVELOPMENT_ENVIRONMENT_CONFIGURATION_FILE=$CONDA_ENV_FILE
}

function _linux_get_vars() {
    # get conda environment name
    SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
    CONDA_ENV_FILE=$(readlink -f "${SCRIPTDIR}/../../../conf/env/conda_env_linux.yaml")
    LINE_WITH_NAME=`sed '1q;d' $CONDA_ENV_FILE`
    CONDA_DEVELOPMENT_ENVIRONMENT_NAME=`echo "$LINE_WITH_NAME" |tr -d '\r' | cut -d":" -f 2 | xargs`

    # miniconda installation directories
    MINICONDA_DIRECTORIES=(~/miniconda3/ /opt/miniconda3/)

    # Activate script path (relative to the environment directory)
    # File extension not necessary as .bat are executable by default on Windows
    # (and the activate script is a batch script)
    ACTIVATE_SCRIPT=bin/activate

    # Loop over all known default installation directories of miniconda
    # If we find a matching activate script, we assume it is a conda valid
    # environment
    BASE_ENVS_FOUND=()
    for env in "${MINICONDA_DIRECTORIES[@]}"
    do
        if [ -f "$env$ACTIVATE_SCRIPT" ]; then
            BASE_ENVS_FOUND+=("$env")
        fi
    done
    if [ ${#BASE_ENVS_FOUND[@]} -eq 0 ]; then
        echo "Could not find conda environment in default directories [${MINICONDA_DIRECTORIES[@]}]"
        echo "Exiting..."
        exit 1
    fi

    # Loop over all found conda base environments and check if there is an
    # environment with the correct name that includes a python.exe
    ENVSDIR="envs"
    DEVEL_ENV_FOUND=""
    for base_env in "${BASE_ENVS_FOUND[@]}"
    do
        if [ -f "${HOME}/miniconda3/${ENVSDIR}/${CONDA_DEVELOPMENT_ENVIRONMENT_NAME}/bin/python" ]; then
            DEVEL_ENV_FOUND="$base_env"
            break
        fi
        if [ -f "${HOME}/.conda/${ENVSDIR}/${CONDA_DEVELOPMENT_ENVIRONMENT_NAME}/bin/python" ]; then
            DEVEL_ENV_FOUND="$base_env"
            break
        fi
        if [ -f "${base_env}/${ENVSDIR}/${CONDA_DEVELOPMENT_ENVIRONMENT_NAME}/bin/python" ]; then
            DEVEL_ENV_FOUND="$base_env"
            break
        fi
    done

    # maybe we found the enviroment name not, but maybe this is on purpose as
    # the user wants to run an conda environment update
    if [ "${1}" == "update" ]; then
        DEVEL_ENV_FOUND="$base_env"
    fi

    if [ -z "${DEVEL_ENV_FOUND}" ]; then
        echo "Could not find conda development environment directories [${BASE_ENVS_FOUND}]"
        echo "Run '$SCRIPTDIR/../conda-update-env.sh'"
        echo "Exiting..."
        exit 1
    fi
    CONDA_BASE_ENVIRONMENT_INCLUDING_DEVELOPMENT_ENVIRONMENT=$DEVEL_ENV_FOUND
    CONDA_BASE_ENVIRONMENT_ACTIVATE_SCRIPT=$DEVEL_ENV_FOUND$ACTIVATE_SCRIPT
    CONDA_DEVELOPMENT_ENVIRONMENT_NAME=$CONDA_DEVELOPMENT_ENVIRONMENT_NAME
    CONDA_DEVELOPMENT_ENVIRONMENT_CONFIGURATION_FILE=$CONDA_ENV_FILE
}

set -e
# MacOS
if [ "$(uname)" == "Darwin" ]; then
    echo "MacOS is currently not supported."
    exit 1
# Linux
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    _linux_get_vars $1
    echo $CONDA_BASE_ENVIRONMENT_INCLUDING_DEVELOPMENT_ENVIRONMENT
    echo $CONDA_BASE_ENVIRONMENT_ACTIVATE_SCRIPT
    echo $CONDA_DEVELOPMENT_ENVIRONMENT_NAME
    echo $CONDA_DEVELOPMENT_ENVIRONMENT_CONFIGURATION_FILE
# Windows
elif [ "$(expr substr $(uname -s) 1 9)" == "CYGWIN_NT" ]; then
    echo "Cygwin is not supported."
    exit 1
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
    echo "32bit Windows is not supported."
    exit 1
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ] || [ "$(expr substr $(uname -s) 1 7)" == "MSYS_NT" ] ; then
    _win32_get_vars $1
    echo $CONDA_BASE_ENVIRONMENT_INCLUDING_DEVELOPMENT_ENVIRONMENT
    echo $CONDA_BASE_ENVIRONMENT_ACTIVATE_SCRIPT
    echo $CONDA_DEVELOPMENT_ENVIRONMENT_NAME
    echo $CONDA_DEVELOPMENT_ENVIRONMENT_CONFIGURATION_FILE
fi
