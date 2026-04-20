#!/usr/bin/env python3
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

"""Defines the interfaces for the communication and its child processes"""

import time
from abc import ABC, abstractmethod
from multiprocessing import Process
from typing import Any

from ..helpers.click_helpers import recho
from ..helpers.logger import get_listener, logger
from .parameter import ComControl


class ComInterface:
    """Base class for communication interfaces.
    Manages process control, logging, and process lifecycle for communication backends.
    """

    def __init__(self, name: str) -> None:
        """Initializes the communication interface.

        Args:
            name (str): Name of the communication interface instance.
        """
        self.name = name
        self.control = ComControl(log_level=logger.level)
        self._processes: dict[str, ProcessInterface] = {}
        # Listener is used to log messages received from other processes via
        # the control.logger queue. Without Listener and logging.QueueHandler,
        # two logging messages would be printed simultaneously.
        self.log_listener = get_listener(self.control.logger)

    def start(self) -> None:
        """Starts all managed processes and waits until they are ready.
        Also starts the logging listener.

        Raises:
            ChildProcessError: If a process terminates during initialization.
        """
        self.log_listener.start()
        if self._processes:
            for x in self._processes.values():
                # If more than one process is needed for communication, the
                # ready event must be reset, otherwise not all processes are started
                if x.control.ready.is_set():
                    x.control.ready.clear()
                logger.debug("Start: %s", x)
                x.start()
                while not x.control.ready.is_set():
                    if x.control.shutdown.is_set():
                        error_text = (
                            f"Process {x.name} terminated during initialization."
                        )
                        raise ChildProcessError(error_text)
                    time.sleep(0.1)
        else:
            recho("No process available")

    def shutdown(self, block: bool = False, timeout: int | None = None) -> None:
        """Signals all processes to shut down and optionally waits until they have finished.

        Args:
            block (bool): If True, blocks until all processes have terminated
                or timeout is reached.
            timeout (int | None): Maximum time to wait in seconds if blocking.
                None means indefinite.
        """
        self.control.shutdown.set()
        start_time = time.time()
        if block:
            while True:
                if not self.is_alive():
                    break
                if timeout and (time.time() - start_time) > timeout:
                    break

    def is_alive(self, process_name: str | None = None) -> bool:
        """Checks whether the managed process or all processes are still alive.

        Args:
            process_name (str | None): Name of the process to check. If None, checks all.

        Returns:
            bool: True if the specified process (or all) is alive, False otherwise.
        """
        if not self._processes:
            return False
        if process_name is None:
            return all(x.is_alive() for x in self._processes.values())
        return self._processes[process_name].is_alive()


class ProcessInterface(Process, ABC):
    """Abstract base class for communication processes.
    Provides shutdown signaling and enforces implementation of the run method.
    """

    name: str
    control: ComControl
    parameter: Any

    def shutdown(self) -> None:
        """Signals the process to shut down by setting the shutdown event."""
        self.control.shutdown.set()

    @abstractmethod
    def run(self) -> None:
        """Main process logic. Must be implemented by subclasses."""
