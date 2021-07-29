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

function _win32_find_ccs() {
    TI_DIR="/${SYSTEMDRIVE:0:1}/ti"
    CCS_DIRS=`ls --directory $TI_DIR/ccs*`
    CCS_VERSION=0
    for i in $CCS_DIRS
    do
        NUMBER=$(echo "$i" | tr -dc '0-9')
        if [ "$NUMBER" -gt "$CCS_VERSION" ]; then
            CCS_VERSION=$NUMBER
        fi
    done
    COMPILER_VERSION=`ls --directory $TI_DIR/ccs$CCS_VERSION/ccs/tools/compiler/ti-cgt-arm_* | xargs -n 1 basename`
    CCS_COMPILER_BIN=$TI_DIR/ccs$CCS_VERSION/ccs/tools/compiler/$COMPILER_VERSION/bin
    CCS_COMPILER_LIB=$TI_DIR/ccs$CCS_VERSION/ccs/tools/compiler/$COMPILER_VERSION/lib
    CCS_UTILS_CYGWIN=$TI_DIR/ccs$CCS_VERSION/ccs/utils/bin
    CCS_UTILS_BIN=$TI_DIR/ccs$CCS_VERSION/ccs/utils/cygwin
    CCS_UTILS_TIOBJ2BIN=$TI_DIR/ccs$CCS_VERSION/ccs/utils/tiobj2bin
}

function _linux_find_ccs() {
    TI_DIR="/opt/ti"
    CCS_DIRS=`ls --directory $TI_DIR/ccs*`
    CCS_VERSION=0
    for i in $CCS_DIRS
    do
        NUMBER=$(echo "$i" | tr -dc '0-9')
        if [ "$NUMBER" -gt "$CCS_VERSION" ]; then
            CCS_VERSION=$NUMBER
        fi
    done
    COMPILER_VERSION=`ls --directory $TI_DIR/ccs$CCS_VERSION/ccs/tools/compiler/ti-cgt-arm_* | xargs -n 1 basename`
    CCS_COMPILER_BIN=$TI_DIR/ccs$CCS_VERSION/ccs/tools/compiler/$COMPILER_VERSION/bin
    CCS_COMPILER_LIB=$TI_DIR/ccs$CCS_VERSION/ccs/tools/compiler/$COMPILER_VERSION/lib
    CCS_UTILS_BIN=$TI_DIR/ccs$CCS_VERSION/ccs/utils/cygwin
    CCS_UTILS_TIOBJ2BIN=$TI_DIR/ccs$CCS_VERSION/ccs/utils/tiobj2bin
}

set -e
# MacOS
if [ "$(uname)" == "Darwin" ]; then
    echo "MacOS is currently not supported."
    exit 1
# Linux
elif [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    _linux_find_ccs
    echo $CCS_COMPILER_BIN
    echo $CCS_COMPILER_LIB
    echo $CCS_UTILS_BIN
    echo $CCS_UTILS_TIOBJ2BIN
# Windows
elif [ "$(expr substr $(uname -s) 1 9)" == "CYGWIN_NT" ]; then
    echo "Cygwin is not supported."
    exit 1
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
    echo "32bit Windows is not supported."
    exit 1
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ] || [ "$(expr substr $(uname -s) 1 7)" == "MSYS_NT" ] ; then
    _win32_find_ccs
    echo $CCS_COMPILER_BIN
    echo $CCS_COMPILER_LIB
    echo $CCS_UTILS_BIN
    echo $CCS_UTILS_CYGWIN
    echo $CCS_UTILS_TIOBJ2BIN
fi
