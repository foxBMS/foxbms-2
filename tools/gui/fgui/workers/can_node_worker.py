#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2010 - 2023, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Implements a process, that uses a CAN adapter to send and receive messages.
Received messages are put in a queue to be consumed by e.g., a GUI. The
messages to be send can be put into the queue by any other thread/process."""

import multiprocessing
import time
from pathlib import Path
from queue import Empty
from typing import Union

from can import SizedRotatingLogger
from can.interface import Bus
from can.message import Message

from ..misc.can.can_helpers import adapter_initialize, adapter_uninitialize


class DummyLogger:
    """Dummy logger class"""

    def __call__(self, *args, **kwargs):
        pass

    def __init__(self, *args, **kwargs):
        pass

    def stop(self):
        """required dummy function"""
        pass  # pylint: disable=unnecessary-pass


class CanAdapterProcess(  # pylint: disable=too-many-instance-attributes
    multiprocessing.Process
):
    """Process to interact with a CAN adapter."""

    def __init__(
        self, adapter: str, baud_rate: int, logging_dir: Union[Path, None] = None
    ) -> None:
        multiprocessing.Process.__init__(self)

        self.init_adapter = adapter
        self.baud_rate = baud_rate

        self.receive_queue = multiprocessing.Queue(maxsize=0)
        self.send_queue = multiprocessing.Queue(maxsize=0)

        self.locked = multiprocessing.Lock()  # only one app my use this process

        self.g_idle = multiprocessing.Event()  # waiting for an app to use the process
        self.g_canceled = multiprocessing.Event()  # on program exit
        self.app_paused = multiprocessing.Event()  # an app pauses using the process
        # an app does no longer use the process
        self.app_canceled = multiprocessing.Event()

        self.g_idle.set()  # we start up and happily idle
        self.g_canceled.clear()  # and we dont throw the CAN process away on startup
        self.app_paused.set()  # an app starts in paused mode
        self.app_canceled.clear()  # an app does not throw the CAN process away on startup
        self.logging_dir = logging_dir

    def run(self) -> None:
        """Implements the functionality of the process, that is run, once the
        process has started."""
        while True:  # 'whole-program-loop' -> only exit on program exit
            # first make sure we can always safely exit if someone cancels
            if self.g_canceled.is_set():
                self._empty_process_queues()
                break

            # idle on startup or if no app uses the process anymore
            if self.g_idle.is_set():
                self._empty_process_queues()
                time.sleep(1)
                continue

            # we are not in idle mode, so we must run in an app context - nice!
            # initialize the adapter and then start receiving/transmitting messages
            adapter = adapter_initialize(self.init_adapter, self.baud_rate)
            if self.logging_dir:
                logger = SizedRotatingLogger(
                    base_filename=self.logging_dir / "foxbms.asc",
                    max_bytes=50000,
                )
            else:
                logger = DummyLogger()
            while True:  # 'app' loop -> exit on single app exit
                # check if we are paused, then we just read the CAN bus and forget
                # the messages
                if self.app_canceled.is_set():  # pylint: disable=no-else-break
                    self._empty_process_queues()
                    adapter_uninitialize(adapter)
                    logger.stop()
                    break
                elif self.app_paused.is_set():
                    self._empty_process_queues()
                    self._msg_receive_and_forget(adapter)
                    continue
                else:
                    # we are not paused, so lets receive and transmit messages
                    self._msg_receive(adapter, logger)
                    self._msg_send(adapter, logger)

    def _msg_receive_and_forget(  # pylint: disable=no-self-use
        self, adapter: Bus
    ) -> None:
        """Receive a message, to empty the CAN adapter buffer.
        The message data is not further used."""
        adapter.recv(0.05)

    def _msg_receive(self, adapter: Bus, logger: SizedRotatingLogger) -> None:
        """Receive a message and log it."""
        message: Message = adapter.recv(0.05)
        if message:
            self.receive_queue.put(message)
            logger(message)

    def _msg_send(self, adapter: Bus, logger: SizedRotatingLogger) -> None:
        """Send messages as long as there are messages to be sent."""
        while not self.send_queue.empty():
            message: Message = self.send_queue.get()
            adapter.send(message)
            message.is_rx = False  # to have it correctly in the logger
            logger(message)

    def g_cancel(self) -> None:
        """Global canceling (i.e., not just 'pausing') the process.
        All queues are cleared."""
        self.receive_queue.close()
        self.send_queue.close()
        self.receive_queue.join_thread()
        self.send_queue.join_thread()
        self.g_canceled.set()

    def _empty_process_queues(self):
        """empty all queues associated with the process"""
        for i in [self.receive_queue, self.send_queue]:
            while not i.empty():
                try:
                    i.get(timeout=0.001)
                except (ValueError, Empty):
                    pass
