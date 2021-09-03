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

# exit immediately on failure
set -e
# NAMES AND PATHS
NAME_DOC_DEPLOY_JOB='doc_deployment'
DIR_DOC_DEPLOY='doc'
DOC_ARTIFACT='build/docs/.'
NAME_COV_DEPLOY_JOB='cov_report_deployment'
DIR_COV_DEPLOY='cov'
COV_ARTIFACT='build/unit_test/artifacts/gcov/.'
NAME_COV_SCRIPT_DEPLOY_JOB='cov_script_report_deployment'
DIR_COV_SCRIPT_DEPLOY='cov_script'
COV_SCRIPT_ARTIFACT='build/unit_test_scripts/.'
REMOTE="$REMOTE_USER@$REMOTE_HOSTNAME"
echo "Value of variable REMOTE_HOSTNAME: $REMOTE_HOSTNAME"
echo "Value of variable REMOTE_USER: $REMOTE_USER"

echo 'This is the deployment-script for local reports and documentation.'
echo 'I will try to detect the current CI-job:'
# check for CI and if we are in the right job
if [ "$CI_JOB_NAME" != "$NAME_DOC_DEPLOY_JOB" ] && [ "$CI_JOB_NAME" != "$NAME_COV_DEPLOY_JOB" ] && [ "$CI_JOB_NAME" != "$NAME_COV_SCRIPT_DEPLOY_JOB" ]; then
    echo 'Expected environment variables not matched.'
    echo "Value of CI_JOB_NAME is $CI_JOB_NAME."
    echo 'Make sure you are running this script in CI in the right job.'
    exit 1
fi
echo "Detected CI-job: $CI_JOB_NAME."

# do not run on scheduled jobs
if [ "$CI_PIPELINE_SOURCE" == 'schedule' ]; then
    echo 'Detected that I am on a scheduled job; Gracefully aborting.'
    exit 0
fi

# list ssh bins
echo 'These are the bins that I can find:'
echo "ssh:          $(which ssh)"
echo "ssh-agent:    $(which ssh-agent)"
echo "ssh-add:      $(which ssh-add)"
echo "ssh-keygen:   $(which ssh-keygen)"
echo "scp:          $(which scp)"

# make sure the ssh-agent is running
echo 'Making sure the ssh-agent is running:'
eval $(ssh-agent -s)

# give ssh-agent the private key
# If setting this up, paste the ssh private key (that you have generated for this purpose!)
# into the (create if necessary) variable INTERNAL_WEBSERVER_SSH_PRIVATE_KEY in GitLab.
echo 'Passing the ssh-key to the agent.'
echo 'If this fails, make sure the variable INTERNAL_WEBSERVER_SSH_PRIVATE_KEY is configured in Gitlab CI.'
echo "$INTERNAL_WEBSERVER_SSH_PRIVATE_KEY" | tr -d '\r' | ssh-add -

# create ssh directory if not existing
echo 'Making sure ~/.ssh exists locally.'
mkdir -p ~/.ssh
echo 'Touching known_hosts'
touch ~/.ssh/known_hosts

# add the hostkey of the remote to the known hosts
# in order not to have to change the script every time the host-key changes,
# create a variable INTERNAL_WEBSERVER_SSH_HOST_KEY in Gitlab and add the output
# of ssh-keyscan. The remote hostname should match the one in this script.
echo 'Removing old ssh host key.'
ssh-keygen -R $REMOTE_HOSTNAME
echo 'Loading ssh host key.'
echo 'If this fails, make sure the variable INTERNAL_WEBSERVER_SSH_HOST_KEY is configured in Gitlab CI.'
echo "$INTERNAL_WEBSERVER_SSH_HOST_KEY" >> ~/.ssh/known_hosts

# decide the name of basedir and the artifact
if [ "$CI_JOB_NAME" == "$NAME_DOC_DEPLOY_JOB" ]; then
    BASE_DIR=$DIR_DOC_DEPLOY
    ARTIFACT=$DOC_ARTIFACT
elif [ "$CI_JOB_NAME" == "$NAME_COV_DEPLOY_JOB" ]; then
    BASE_DIR=$DIR_COV_DEPLOY
    ARTIFACT=$COV_ARTIFACT
elif [ "$CI_JOB_NAME" == "$NAME_COV_SCRIPT_DEPLOY_JOB" ]; then
    BASE_DIR=$DIR_COV_SCRIPT_DEPLOY
    ARTIFACT=$COV_SCRIPT_ARTIFACT
fi

REMOTE_BASE_DIR="$REMOTE_WEB_DIR/$BASE_DIR"
REMOTE_TARGET_DIR="$REMOTE_BASE_DIR/$CI_COMMIT_REF_SLUG/"
echo "I will store the files in $REMOTE_TARGET_DIR."

# mkdir -p base directory (based on JOB_NAME doc/ or cov/)
echo "Creating directory $REMOTE_BASE_DIR on remote."
ssh $REMOTE "mkdir -p $REMOTE_BASE_DIR"

# rm -rf $REMOTE_TARGET_DIR
echo "Removing directory $REMOTE_TARGET_DIR (in order to make sure that it is empty)."
ssh $REMOTE "rm -rf $REMOTE_TARGET_DIR"

# mkdir -p $REMOTE_TARGET_DIR
echo "Creating directory $REMOTE_TARGET_DIR on remote."
ssh $REMOTE "mkdir -p $REMOTE_TARGET_DIR"

# tar and copy local artifacts to $REMOTE_TARGET_DIR
echo 'This is the local artifact:'
ls -al $ARTIFACT
echo "Copying artifact from $ARTIFACT to $REMOTE_TARGET_DIR on remote."
echo "pushing into $ARTIFACT"
echo "pushd $ARTIFACT"
pushd $ARTIFACT
echo "tar czf - . | ssh $REMOTE \"cd $REMOTE_TARGET_DIR && tar xzf -\""
tar czf - . | ssh $REMOTE "cd $REMOTE_TARGET_DIR && tar xzf -"
echo "popd"
popd

if [ "$CI_COMMIT_BRANCH" == "$CI_DEFAULT_BRANCH" ]; then
    echo "Detected a build on default branch $CI_DEFAULT_BRANCH."
    echo "Creating additional dir with commit SHA."

    REMOTE_COMMIT_DIR="$REMOTE_BASE_DIR/$CI_COMMIT_SHA/"
    # copy contents to dir with commit ID, if we are on master
    echo "Commit SHA is $CI_COMMIT_SHA."
    echo "Removing $REMOTE_COMMIT_DIR."
    ssh $REMOTE "rm -rf $REMOTE_COMMIT_DIR"
    echo "Creating $REMOTE_COMMIT_DIR."
    ssh $REMOTE "mkdir -p $REMOTE_COMMIT_DIR"
    echo "Copying files from $REMOTE_TARGET_DIR. to $REMOTE_COMMIT_DIR"
    ssh $REMOTE "cp -r $REMOTE_TARGET_DIR. $REMOTE_COMMIT_DIR"

fi

# housekeeping: kill the current agent
echo 'Cleaning up and killing current ssh-agent:'
eval $(ssh-agent -k)

echo 'Live long and prosper. 🖖'
