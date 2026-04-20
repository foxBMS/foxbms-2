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

"""Testing file 'cli/com/file_com.py'."""

import signal
import sys
import unittest
from dataclasses import fields
from pathlib import Path
from queue import Empty
from time import sleep
from unittest.mock import MagicMock, Mock, mock_open, patch

from can import Message

try:
    from cli.com.file_com import File, FileReader, FileWriter
    from cli.com.parameter import ComControl, FileParameter
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.com.file_com import File, FileReader, FileWriter
    from cli.com.parameter import ComControl, FileParameter


class TestFileInterface(unittest.TestCase):
    """Unit tests for the File communication interface.
    Tests initialization, reading, and writing functionality.
    """

    @patch("cli.com.file_com.logger")
    def test_file_init_with_input_output(self, _: Mock) -> None:
        """Test File instantiation with both input and output file.
        Ensures FileReader and FileWriter processes are present.
        """
        param = FileParameter(input_file="in.txt", output_file="out.txt")
        file_obj = File("test", param)
        # pylint: disable=W0212
        self.assertIn("FileReader", file_obj._processes)
        self.assertIn("FileWriter", file_obj._processes)

    def test_file_read_returns_none_if_queue_empty_and_reader_alive(self) -> None:
        """Test that read() returns None if the output queue is empty
        and the FileReader process is still alive.
        """
        param = FileParameter(input_file="in.txt")
        file_obj = File("test", param)
        file_obj.is_alive = MagicMock(return_value=True)
        self.assertIsNone(file_obj.read())

    def test_file_read_none_if_reader_dead(self) -> None:
        """Test that read() returning None if the FileReader process is dead."""
        param = FileParameter(input_file="in.txt")
        file_obj = File("test", param)
        file_obj.is_alive = MagicMock(return_value=False)
        self.assertIsNone(file_obj.read())

    def test_file_read_returns_value(self) -> None:
        """Test that read() returns a value from the output queue if available."""
        param = FileParameter(input_file="in.txt")
        file_obj = File("test", param)
        file_obj.is_alive = MagicMock(return_value=True)
        file_obj.control.output.put("testline")
        # The while loop ensures that empty() will return the correct state
        # at the moment the test starts
        while file_obj.control.output.empty():
            sleep(0.1)
        self.assertEqual(file_obj.read(), "testline")

    def test_file_write_raises_if_writer_dead(self) -> None:
        """Test that write() raises RuntimeError if the FileWriter process is dead."""
        param = FileParameter(output_file="out.txt")
        file_obj = File("test", param)
        file_obj.is_alive = MagicMock(return_value=False)
        with self.assertRaises(RuntimeError):
            file_obj.write("data")

    def test_file_write_puts_data(self) -> None:
        """Test that write() puts data into the input queue if the writer is alive."""
        param = FileParameter(output_file="out.txt")
        file_obj = File("test", param)
        file_obj.is_alive = MagicMock(return_value=True)
        file_obj.write("data")
        self.assertEqual(file_obj.control.input.get(), "data")

    def test_file_write_can_message(self) -> None:
        """Test that write_can_message() puts a CAN message into the input queue.
        The data in the message should be preserved.
        """
        param = FileParameter(output_file="out.txt")
        file_obj = File("test", param)
        file_obj.is_alive = MagicMock(return_value=True)
        msg = Message(data=b"test")
        file_obj.write_can_message(msg)
        returned_msg = file_obj.control.input.get()
        # Only data can be compared, as object IDs change in the queue
        self.assertEqual(returned_msg.data, msg.data)

    def test_file_write_can_message_raises_if_writer_dead(self) -> None:
        """Test that write_can_message() raises RuntimeError if the FileWriter
        process is dead.
        """
        param = FileParameter(output_file="out.txt")
        file_obj = File("test", param)
        file_obj.is_alive = MagicMock(return_value=False)
        msg = Message(data=b"test")
        with self.assertRaises(RuntimeError):
            file_obj.write_can_message(msg)


class TestFileReaderStart(unittest.TestCase):
    """Test the start call of MQTT."""

    def test_start_call(self) -> None:
        """Verify that calling the start method will create the process without an
        exception.
        """
        param = FileParameter(input_file="test.txt")
        control = ComControl()
        reader = FileReader("test", control, param)
        reader.start()
        reader.terminate()
        # Wait until process is dead then call close()
        while reader.is_alive():
            sleep(0.1)
        reader.close()


class TestFileReaderInit(unittest.TestCase):
    """TestCase for the __init__ method of FileReader."""

    def test_init_assigns_attributes(self):
        """Test that __init__ correctly assigns attributes."""
        mock_control = MagicMock()
        mock_parameter = MagicMock()
        reader = FileReader("test_name", mock_control, mock_parameter)
        self.assertEqual(reader.name, "test_name")
        self.assertIs(reader.control, mock_control)
        self.assertIs(reader.parameter, mock_parameter)


class TestFileReaderRun(unittest.TestCase):
    """TestCase for the run method of FileReader."""

    def setUp(self):
        """Set up FileReader with mocks."""
        self.mock_control = MagicMock()
        self.mock_control.logger = MagicMock()
        self.mock_control.log_level = 20
        self.mock_parameter = MagicMock()
        self.reader = FileReader("reader", self.mock_control, self.mock_parameter)

    @patch("cli.com.file_com.signal.signal")
    @patch("cli.com.file_com.add_queue_handler")
    @patch("cli.com.file_com.logger.setLevel")
    def test_run_calls_helpers_and_read(
        self, mock_set_level: Mock, mock_add_handler: Mock, mock_signal: Mock
    ) -> None:
        """Test that run sets signal, logging, and calls _read_from_file."""
        with patch.object(self.reader, "_read_from_file") as mock_read:
            self.reader.run()
            mock_signal.assert_called_once_with(signal.SIGINT, signal.SIG_IGN)
            mock_add_handler.assert_called_once_with(self.mock_control.logger)
            mock_set_level.assert_called_once_with(self.mock_control.log_level)
            mock_read.assert_called_once()


class TestFileReaderReadFromFile(unittest.TestCase):
    """TestCase for the _read_from_file method of FileReader."""

    def setUp(self):
        """Set up FileReader with mocks."""
        self.mock_control = MagicMock()
        self.mock_control.logger = MagicMock()
        self.mock_control.ready = MagicMock()
        self.mock_control.shutdown = MagicMock()
        self.mock_control.output = MagicMock()
        self.mock_parameter = MagicMock()
        self.reader = FileReader("reader", self.mock_control, self.mock_parameter)

    def test_read_from_file_success(self):
        """Test that _read_from_file reads lines and puts them into the output queue."""
        self.mock_parameter.input_file = "dummy.txt"
        self.mock_parameter.encoding = "utf-8"
        self.mock_control.shutdown.is_set.side_effect = [False, False, True]
        mock_file_content = "line1\nline2\n"
        with (
            patch("builtins.open", mock_open(read_data=mock_file_content)),
            patch("cli.com.file_com.logger.debug") as mock_debug,
        ):
            # pylint: disable=W0212
            self.reader._read_from_file()
            self.mock_control.ready.set.assert_called_once()
            self.mock_control.output.put.assert_any_call("line1\n")
            self.mock_control.output.put.assert_any_call("line2\n")
            mock_debug.assert_called_with("Finished reading input file.")

    def test_read_from_file_to_end(self):
        """Test that _read_from_file reads all lines in a file."""
        self.mock_parameter.input_file = "dummy.txt"
        self.mock_parameter.encoding = "utf-8"
        self.mock_control.shutdown.is_set.side_effect = [False, False, False]
        mock_file_content = "line1\nline2\n"
        with (
            patch("builtins.open", mock_open(read_data=mock_file_content)),
            patch("cli.com.file_com.logger.debug") as mock_debug,
        ):
            # pylint: disable=W0212
            self.reader._read_from_file()
            self.mock_control.ready.set.assert_called_once()
            mock_debug.assert_called_with("Finished reading input file.")

    def test_read_from_file_file_not_found(self):
        """Test that _read_from_file handles FileNotFoundError and calls shutdown."""
        self.mock_parameter.input_file = None
        with (
            patch("cli.com.file_com.logger.error") as mock_error,
            patch.object(self.reader, "shutdown") as mock_shutdown,
        ):
            # pylint: disable=W0212
            self.reader._read_from_file()
            mock_error.assert_called_once()
            mock_shutdown.assert_called_once()


class TestFileWriterStart(unittest.TestCase):
    """Test the start call of MQTT."""

    def test_start_call(self) -> None:
        """Verify that calling the start method will create the process without an
        exception.
        """
        param = FileParameter(output_file="test.txt")
        control = ComControl()
        writer = FileWriter("test", control, param)
        writer.start()
        writer.terminate()
        # Wait until process is dead then call close()
        while writer.is_alive():
            sleep(0.1)
        writer.close()


@patch("cli.com.file_com.logger")
class TestFileWriterInit(unittest.TestCase):
    """Test cases for FileWriter initialization (__init__)."""

    def setUp(self) -> None:
        """Prepare mock objects for initialization tests."""
        # spec parameter needs to be defined via fields in case a dataclass
        # should be mocked
        self.mock_control = MagicMock(spec=[field.name for field in fields(ComControl)])
        self.mock_output_file = Path("/tmp/test.log")  # noqa
        self.mock_can_logger = MagicMock()
        # spec parameter needs to be defined via fields in case a dataclass
        # should be mocked
        self.mock_parameter = MagicMock(
            spec=[field.name for field in fields(FileParameter)]
        )
        self.mock_parameter.output_file = self.mock_output_file
        self.mock_parameter.encoding = "utf-8"
        self.mock_parameter.can_logger.log_can_files = True
        self.mock_parameter.can_logger.max_bytes = 1024
        self.mock_parameter.can_logger.rollover_count = 2

    @patch("cli.com.file_com.Path.mkdir")
    @patch("cli.com.file_com.SizedRotatingLogger")
    def test_init_with_valid_path_and_logger(
        self,
        mock_sized_logger: Mock,
        _: Mock,
        mock_logger: Mock,
    ) -> None:
        """Test initialization with valid path and logger creation."""
        mock_sized_logger.return_value = self.mock_can_logger
        fw = FileWriter("TestProcess", self.mock_control, self.mock_parameter)
        # pylint: disable=W0212
        self.assertIs(fw._can_logger, self.mock_can_logger)
        mock_logger.error.assert_not_called()

    def test_init_with_invalid_path_type_exits(self, mock_logger: Mock) -> None:
        """Test initialization exits if output_file is not Path."""
        self.mock_parameter.output_file = "not_a_path"
        with self.assertRaises(SystemExit):
            FileWriter("TestProcess", self.mock_control, self.mock_parameter)
        mock_logger.error.assert_called_once()

    @patch("cli.com.file_com.SizedRotatingLogger", side_effect=ValueError)
    def test_init_logger_creation_failure_exits(
        self, _: Mock, mock_logger: Mock
    ) -> None:
        """Test initialization exits if logger creation fails."""
        with self.assertRaises(SystemExit):
            with patch("cli.com.file_com.Path.mkdir"):
                FileWriter("TestProcess", self.mock_control, self.mock_parameter)
        mock_logger.error.assert_called_once_with("Could not create logger object.")

    @patch("cli.com.file_com.SizedRotatingLogger")
    def test_init_with_log_can_files_false(
        self, mock_sized_logger: Mock, *_: list[Mock]
    ) -> None:
        """Test initialization does not create logger if log_can_files is False."""
        self.mock_parameter.can_logger.log_can_files = False
        fw = FileWriter("TestProcess", self.mock_control, self.mock_parameter)
        # pylint: disable=W0212
        self.assertIsNone(fw._can_logger)
        mock_sized_logger.assert_not_called()


@patch("cli.com.file_com.add_queue_handler")
@patch("cli.com.file_com.logger")
@patch("cli.com.file_com.signal")
class TestFileWriterRun(unittest.TestCase):
    """Test cases for FileWriter.run()."""

    def setUp(self) -> None:
        """Prepare FileWriter instance and mocks for run method."""
        # spec parameter needs to be defined via fields in case a dataclass
        # should be mocked
        self.mock_control = MagicMock(spec=[field.name for field in fields(ComControl)])
        self.mock_parameter = MagicMock(
            spec=[field.name for field in fields(FileParameter)]
        )
        self.mock_parameter.output_file = Path("/tmp/test.log")  # noqa
        self.mock_parameter.encoding = "utf-8"
        self.mock_parameter.can_logger.log_can_files = True

    @patch("cli.com.file_com.Path.mkdir")
    @patch("cli.com.file_com.SizedRotatingLogger")
    def test_run_calls_log_can_message_if_logger_exists(self, *_: list[Mock]) -> None:
        """Test run calls _log_can_message if logger is present."""
        fw = FileWriter("TestProcess", self.mock_control, self.mock_parameter)
        # pylint: disable=W0212
        fw._can_logger = MagicMock()
        fw._log_can_message = MagicMock()
        fw._write_to_file = MagicMock()
        fw.run()
        fw._log_can_message.assert_called_once()
        fw._write_to_file.assert_not_called()

    def test_run_calls_write_to_file_if_logger_missing(self, *_: list[Mock]) -> None:
        """Test run calls _write_to_file if logger is None."""
        self.mock_parameter.can_logger.log_can_files = False
        fw = FileWriter("TestProcess", self.mock_control, self.mock_parameter)
        # pylint: disable=W0212
        fw._can_logger = None
        fw._log_can_message = MagicMock()
        fw._write_to_file = MagicMock()
        fw.run()
        fw._write_to_file.assert_called_once()
        fw._log_can_message.assert_not_called()


@patch("cli.com.file_com.logger")
class TestFileWriterWriteToFile(unittest.TestCase):
    """Test cases for FileWriter._write_to_file()."""

    def setUp(self) -> None:
        """Prepare mocks for _write_to_file method."""
        # spec parameter needs to be defined via fields in case a dataclass
        # should be mocked
        self.mock_control = MagicMock(spec=[field.name for field in fields(ComControl)])
        self.mock_control.input.get = MagicMock(
            side_effect=["msg1", Empty(), "msg2", Empty(), Empty()]
        )
        self.mock_control.shutdown.is_set = MagicMock(side_effect=[False, False, True])
        self.mock_parameter = MagicMock(
            spec=[field.name for field in fields(FileParameter)]
        )
        self.mock_parameter.output_file = Path("/tmp/test_write.log")  # noqa
        self.mock_parameter.encoding = "utf-8"
        self.mock_parameter.can_logger.log_can_files = False

        self.fw = FileWriter("TestProcess", self.mock_control, self.mock_parameter)
        self.fw.shutdown = MagicMock()

    @patch("cli.com.file_com.open", new_callable=mock_open)
    def test_write_to_file_success(self, _: Mock, mock_logger: Mock) -> None:
        """Test writing messages to file works and calls shutdown."""
        # pylint: disable=W0212
        self.fw._write_to_file()
        self.fw.shutdown.assert_called_once()
        mock_logger.error.assert_not_called()

    @patch("cli.com.file_com.open", side_effect=mock_open)
    def test_write_to_file_type_error(self, mock_file: Mock, mock_logger: Mock) -> None:
        """Test TypeError while writing to file is logged."""
        # pylint: disable=W0212
        mock_file.write = Mock(side_effect=TypeError)
        self.fw._write_to_file()
        mock_logger.error.assert_called_once()
        self.fw.shutdown.assert_called_once()

    @patch("cli.com.file_com.open", side_effect=FileNotFoundError)
    def test_write_to_file_file_not_found_error(
        self, _: Mock, mock_logger: Mock
    ) -> None:
        """Test FileNotFoundError is logged."""
        # pylint: disable=W0212
        self.fw._write_to_file()
        mock_logger.error.assert_called_once_with(
            "File %s not found.", self.mock_parameter.output_file
        )
        self.fw.shutdown.assert_called_once()


class TestFileWriterLogCanMessage(unittest.TestCase):
    """Test cases for FileWriter._log_can_message()."""

    def setUp(self) -> None:
        """Prepare mocks for _log_can_message method."""
        # spec parameter needs to be defined via fields in case a dataclass
        # should be mocked
        self.mock_control = MagicMock(spec=[field.name for field in fields(ComControl)])
        self.mock_control.input.get = MagicMock()
        self.mock_control.shutdown.is_set = MagicMock(return_value=False)
        self.mock_control.ready.set = MagicMock()
        self.mock_parameter = MagicMock(
            spec=[field.name for field in fields(FileParameter)]
        )
        self.mock_parameter.output_file = Path("/tmp/test_can.log")  # noqa
        self.mock_parameter.encoding = "utf-8"
        self.mock_parameter.can_logger.log_can_files = True

        with patch("cli.com.file_com.Path.mkdir"):
            with patch("cli.com.file_com.SizedRotatingLogger"):
                self.fw = FileWriter(
                    "TestProcess", self.mock_control, self.mock_parameter
                )
        # pylint: disable=W0212
        self.fw._can_logger = MagicMock()
        self.fw.shutdown = MagicMock()

    def test_log_can_message_type_error(self) -> None:
        """Test TypeError is raised if _can_logger is None."""
        # pylint: disable=W0212
        self.fw._can_logger = None
        with self.assertRaises(TypeError):
            self.fw._log_can_message()

    @patch("cli.com.file_com.sleep", return_value=None)
    def test_log_can_message_success(self, _: Mock) -> None:
        """Test CAN messages are logged and timestamp adjusted."""
        msg1 = MagicMock(spec=Message)
        msg1.timestamp = 100.0
        msg2 = MagicMock(spec=Message)
        msg2.timestamp = 150.0
        self.mock_control.input.get = MagicMock(
            side_effect=[msg1, msg2, Empty(), Empty()]
        )
        self.mock_control.shutdown.is_set = MagicMock(side_effect=[False, True])
        # pylint: disable=W0212
        self.fw._log_can_message()
        # msg1 timestamp should be set to 0
        self.assertEqual(msg1.timestamp, 0)
        # msg2 timestamp should be adjusted
        self.assertEqual(msg2.timestamp, 50.0)
        # _can_logger should be called twice
        self.assertEqual(self.fw._can_logger.call_count, 2)

    @patch("cli.com.file_com.sleep", return_value=None)
    def test_log_can_message_stop_first_msg(self, _: Mock) -> None:
        """Test CAN messages stops after the first message"""
        self.mock_control.input.get = MagicMock(side_effect=[Empty(), Empty()])
        self.mock_control.shutdown.is_set = MagicMock(side_effect=[False, True])
        self.fw._log_can_message()  # pylint: disable=protected-access
        self.assertEqual(self.fw._can_logger.call_count, 0)  # pylint: disable=protected-access


if __name__ == "__main__":
    unittest.main()
