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

"""Implements the a multiprocessing based file writer and reader"""

import signal
import sys
from pathlib import Path
from queue import Empty
from time import sleep

from can import Message
from can.io import SizedRotatingLogger

from ..helpers.logger import add_queue_handler, logger
from .com_interface import ComInterface, ProcessInterface
from .parameter import ComControl, FileParameter


class File(ComInterface):
    """Handles file-based communication by reading from and writing to input/output files.
    Depending on the provided parameters, it initializes processes for file reading and writing.
    """

    def __init__(self, name: str, parameter: FileParameter) -> None:
        """Initializes the File communication interface.

        Args:
            name (str): The name of the interface.
            parameter (FileParameter): Configuration parameters for file operations.
        """
        super().__init__(name=name)
        if parameter.input_file:
            logger.debug("Add process reading an input file.")
            self._processes[FileReader.__name__] = FileReader(
                name, self.control, parameter
            )
        if parameter.output_file:
            logger.debug("Add process writing to an output file.")
            self._processes[FileWriter.__name__] = FileWriter(
                name, self.control, parameter
            )

    def read(self) -> str | None:
        """Reads data from the output queue if available.

        Returns:
            str | None: The next line of data if available, otherwise None.

        Raises:
            RuntimeError: If the related reading process is not running.
        """
        # If the related process is not alive, this method would block
        # otherwise.
        if self.control.output.empty():
            if not self.is_alive(FileReader.__name__):
                debug_text = "Input queue can not be read, because related process is not running."
                logger.debug(debug_text)
            return None
        return self.control.output.get(block=False)

    def write(self, data: str) -> None:
        """Writes string data to the input queue for output processing.

        Args:
            data (str): The data to write.

        Raises:
            RuntimeError: If the related writing process is not running.
        """
        if not self.is_alive(FileWriter.__name__):
            error_text = "Output queue can not be written, because related process is not running."
            raise RuntimeError(error_text)
        self.control.input.put(data)

    def write_can_message(self, msg: Message) -> None:
        """Writes a CAN message to the input queue for output processing.

        Args:
            msg (Message): The CAN message to write.

        Raises:
            RuntimeError: If the related writing process is not running.
        """
        if not self.is_alive(FileWriter.__name__):
            error_text = "Output queue can not be written, because related process is not running."
            raise RuntimeError(error_text)
        self.control.input.put(msg)


class FileReader(ProcessInterface):
    """Process that reads lines from an input file and places them on the output queue."""

    def __init__(
        self, name: str, control: ComControl, parameter: FileParameter
    ) -> None:
        """Initializes the FileReader process.

        Args:
            name (str): The process name.
            control (ComControl): Communication control object.
            parameter (FileParameter): File parameter configuration.
        """
        super().__init__(name=name, daemon=True)
        self.name = name
        self.control = control
        self.parameter = parameter

    def run(self) -> None:
        """Starts the file reading process and reads from the input file."""
        signal.signal(signal.SIGINT, signal.SIG_IGN)
        add_queue_handler(self.control.logger)
        logger.setLevel(self.control.log_level)
        logger.debug("Start reading from an input file.")
        self._read_from_file()

    def _read_from_file(self) -> None:
        """Reads lines from the input file and puts them into the output queue.

        Raises:
            FileNotFoundError: If the input file does not exist.
        """
        try:
            if self.parameter.input_file is None:
                raise FileNotFoundError
            with open(
                file=self.parameter.input_file, encoding=self.parameter.encoding
            ) as f:
                self.control.ready.set()
                while not self.control.shutdown.is_set():
                    line = f.readline()
                    if not line:
                        break
                    self.control.output.put(line)
                logger.debug("Finished reading input file.")
        except FileNotFoundError:
            logger.error("File '%s' does not exit.", self.parameter.input_file)
            self.shutdown()


class FileWriter(ProcessInterface):
    """Process that writes data to an output file or logs CAN messages."""

    def __init__(
        self, name: str, control: ComControl, parameter: FileParameter
    ) -> None:
        """Initializes the FileWriter process and CAN logger if required.

        Args:
            name (str): The process name.
            control (ComControl): Communication control object.
            parameter (FileParameter): File parameter configuration.
        """
        if not isinstance(parameter.output_file, Path):
            logger.error(
                "FileWriter can not be created, because output file "
                "must be a valid path."
            )
            sys.exit(1)
        super().__init__(name=name, daemon=True)
        self.name = name
        self.control = control
        self.parameter = parameter
        self._can_logger = None
        if parameter.can_logger.log_can_files:
            try:
                # pylint: disable-next=line-too-long
                output_directory: Path = self.parameter.output_file.parent  # type: ignore[union-attr]
                output_directory.mkdir(parents=True, exist_ok=True)
                self._can_logger = SizedRotatingLogger(
                    base_filename=self.parameter.output_file,  # type: ignore[arg-type]
                    max_bytes=self.parameter.can_logger.max_bytes,
                    encoding=self.parameter.encoding,
                    rollover_count=self.parameter.can_logger.rollover_count,
                )
            except ValueError:
                logger.error("Could not create logger object.")
                sys.exit(1)

    def run(self) -> None:
        """Starts the file writing process, either writing CAN logs or standard output."""
        signal.signal(signal.SIGINT, signal.SIG_IGN)
        add_queue_handler(self.control.logger)
        logger.setLevel(self.control.log_level)
        if self._can_logger:
            logger.debug("Start writing to a CAN log file.")
            self._log_can_message()
        else:
            logger.debug("Start writing to an output file.")
            self._write_to_file()

    def _write_to_file(self) -> None:
        """Writes string messages from the input queue to the output file.

        Raises:
            TypeError: If the received message is not a string.
            FileNotFoundError: If the output file does not exist.
        """
        msg = None
        try:
            with open(
                file=self.parameter.output_file,
                mode="w",
                encoding=self.parameter.encoding,
            ) as f:
                self.control.ready.set()
                while True:
                    try:
                        msg = self.control.input.get(block=False)
                        f.write(msg + "\n")
                        f.flush()
                    except Empty:
                        if self.control.shutdown.is_set():
                            break
                        sleep(0.1)
        except TypeError:
            logger.error("Type of received message '%s' is not string.", msg)
        except FileNotFoundError:
            logger.error("File %s not found.", self.parameter.output_file)
        self.shutdown()

    def _log_can_message(self) -> None:
        """Logs CAN messages from the input queue to a log file, adjusting timestamps.

        Raises:
            TypeError: If the CAN logger is not initialized.
        """
        if self._can_logger is None:
            error_text = "CAN logger has not yet been initialized"
            raise TypeError(error_text)
        self.control.ready.set()
        first_timestamp: float = 0
        # get the first message
        while True:
            # store timestamp of first message to have the correct start time
            try:
                msg: Message = self.control.input.get()
                first_timestamp = msg.timestamp
                msg.timestamp = 0
                self._can_logger(msg)
                break
            except Empty:
                sleep(0.1)
                if self.control.shutdown.is_set():
                    return
        while True:
            try:
                msg = self.control.input.get()
                msg.timestamp = msg.timestamp - first_timestamp
                self._can_logger(msg)
            except Empty:
                sleep(0.1)
                if self.control.shutdown.is_set():
                    return
