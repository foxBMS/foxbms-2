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
SCRIPTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
if [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]; then
    WAFSCRIPT="$SCRIPTDIR/../../../tools/utils/bash/run-python.sh $SCRIPTDIR/../../../tools/waf"
elif [ "$(expr substr $(uname -s) 1 9)" == "CYGWIN_NT" ]; then
    echo "Cygwin is not supported."
    exit 1
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW32_NT" ]; then
    echo "32bit Windows is not supported."
    exit 1
elif [ "$(expr substr $(uname -s) 1 10)" == "MINGW64_NT" ] || [ "$(expr substr $(uname -s) 1 7)" == "MSYS_NT" ] ; then
    WAFSCRIPT="$SCRIPTDIR/../../../tools/utils/cmd/run-python.bat $SCRIPTDIR/../../../tools/waf"
fi

echo ""
echo "running preproc test"
echo ""
cd $SCRIPTDIR/preproc/
$WAFSCRIPT distclean
$WAFSCRIPT configure build

echo ""
echo "running install test"
echo ""
cd $SCRIPTDIR/install/
$WAFSCRIPT distclean
# this test does not work on our platform and we don't need the feature
# $WAFSCRIPT configure build

echo ""
echo "running general test"
echo ""
cd $SCRIPTDIR/general/
$WAFSCRIPT distclean
$WAFSCRIPT configure build

echo ""
echo "running init test"
echo ""
cd $SCRIPTDIR/init/
$WAFSCRIPT distclean
$WAFSCRIPT configure build

echo ""
echo "running install_group test"
echo ""
export WAF_TEST_GROUP=waftest
cd $SCRIPTDIR/install_group
$WAFSCRIPT distclean
$WAFSCRIPT configure build
