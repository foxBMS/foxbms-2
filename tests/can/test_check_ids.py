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

"""Testing file 'tests/can/check_ids.py'."""
# ruff: noqa:ANN002,ANN202
# pylint: disable=too-many-lines

import argparse
import logging  # noqa: TID251
import sys
import unittest
from pathlib import Path
from unittest.mock import MagicMock, mock_open, patch

from cantools.database.can.message import Message
from git.exc import InvalidGitRepositoryError

try:
    from tests.can import check_ids
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[2]))
    from tests.can import check_ids


class TestGetGitRepoRoot(unittest.TestCase):
    """Tests if repo root is found"""

    @patch("tests.can.check_ids.Repo")
    def test_valid_git_repo(self, mock_repo: MagicMock) -> None:
        """Should return repo root"""
        mock_ret = MagicMock()
        mock_ret.git.rev_parse.return_value = "test/path"
        mock_repo.return_value = mock_ret
        result = check_ids.get_git_root(Path("anything.path"))
        self.assertEqual(result, "test/path")

    @patch("tests.can.check_ids.Repo")
    def test_invalid_git_repo(self, mock_repo: MagicMock) -> None:
        """Should return repo root manually"""
        mock_repo.side_effect = InvalidGitRepositoryError
        result = check_ids.get_git_root(Path("test/tests/can/check_ids.py"))
        self.assertEqual(Path(result).resolve(), Path("test").resolve())


class TestConstructMsg(unittest.TestCase):
    """Tests for constructing message defines"""

    def test_no_f_not_cyclic_rx(self) -> None:
        """Tests message without leading f and cycle time"""
        msg = Message(
            name="TestMessage",
            comment="(in:can_cbs_rx_test_message.c:CANRX_TestMessage, fv:rx, type:Cli Test)",
            frame_id=15,
            length=123,
            signals="",
        )
        result = check_ids.construct_msg_define(msg)
        self.assertEqual(
            result,
            check_ids.ExpectedCanMessageDefines(
                dbc_name="TestMessage",
                dbc_id="0xF",
                dbc_direction="rx",
                dbc_cyclic=False,
                exp_id_macro=("CANRX_TEST_MESSAGE_ID", ""),
                exp_id_type_macro=("CANRX_TEST_MESSAGE_ID_TYPE", ""),
                exp_period_macro=("CANRX_TEST_MESSAGE_PERIOD_ms", ""),
                exp_phase_macro=("Rx - ND", ""),
                exp_endianness_macro=("CANRX_TEST_MESSAGE_ENDIANNESS", ""),
                exp_dlc_macro=("CANRX_TEST_MESSAGE_DLC", ""),
                exp_full_msg_macro=("CANRX_TEST_MESSAGE_MESSAGE", ""),
            ),
        )

    def test_f_cyclic_tx(self) -> None:
        """Tests message with leading f and cycle time"""
        msg = Message(
            name="f_TestMessage",
            cycle_time=10,
            comment="(in:can_cbs_tx_test_message.c:CANTX_TestMessage, fv:tx, type:Cli Test)",
            frame_id=14,
            length=123,
            signals="",
        )
        result = check_ids.construct_msg_define(msg)
        self.assertEqual(
            result,
            check_ids.ExpectedCanMessageDefines(
                dbc_name="f_TestMessage",
                dbc_id="0xE",
                dbc_direction="tx",
                dbc_cyclic=True,
                exp_id_macro=("CANTX_TEST_MESSAGE_ID", ""),
                exp_id_type_macro=("CANTX_TEST_MESSAGE_ID_TYPE", ""),
                exp_period_macro=("CANTX_TEST_MESSAGE_PERIOD_ms", ""),
                exp_phase_macro=("CANTX_TEST_MESSAGE_PHASE_ms", ""),
                exp_endianness_macro=("CANTX_TEST_MESSAGE_ENDIANNESS", ""),
                exp_dlc_macro=("CANTX_TEST_MESSAGE_DLC", ""),
                exp_full_msg_macro=("CANTX_TEST_MESSAGE_MESSAGE", ""),
            ),
        )

    def test_f_not_cyclic_tx(self) -> None:
        """Tests message with leading f, without cycle time"""
        msg = Message(
            name="f_TestMessage",
            comment="(in:can_cbs_tx_test_message.c:CANTX_TestMessage, fv:tx, type:Cli Test)",
            frame_id=13,
            length=123,
            signals="",
        )
        result = check_ids.construct_msg_define(msg)
        self.assertEqual(
            result,
            check_ids.ExpectedCanMessageDefines(
                dbc_name="f_TestMessage",
                dbc_id="0xD",
                dbc_direction="tx",
                dbc_cyclic=False,
                exp_id_macro=("CANTX_TEST_MESSAGE_ID", ""),
                exp_id_type_macro=("CANTX_TEST_MESSAGE_ID_TYPE", ""),
                exp_period_macro=("Tx - async - ND", ""),
                exp_phase_macro=("Tx - async - ND", ""),
                exp_endianness_macro=("CANTX_TEST_MESSAGE_ENDIANNESS", ""),
                exp_dlc_macro=("CANTX_TEST_MESSAGE_DLC", ""),
                exp_full_msg_macro=("Tx - async - ND", ""),
            ),
        )

    def test_malformed_direction(self) -> None:
        """Tests message with leading f, without cycle time"""
        msg = Message(
            name="f_TestMessage",
            comment="(in:can_cbs_tx_test_message.c:CANTX_TestMessage, fv:wx, type:Cli Test)",
            frame_id=13,
            length=123,
            signals="",
        )
        with self.assertRaises(SystemExit):
            check_ids.construct_msg_define(msg)


class TestGetDefinesFromFile(unittest.TestCase):
    """Tests for getting defines from a file"""

    def test_empty_file(self) -> None:
        """Empty file should return a empty list"""
        fake_content = ""
        with patch("builtins.open", mock_open(read_data=fake_content)):
            result = check_ids.get_defines_from_file(Path("path.h"), "CANRX")
        self.assertEqual(result, [])

    def test_non_cyclic(self) -> None:
        """Check if cyclic is set correctly"""
        fake_content = (
            "#define CANRX_BMS_STATE_REQUEST_ID (0x210u)\n"
            "#define CANRX_BMS_STATE_REQUEST_ID_TYPE (CAN_STANDARD_IDENTIFIER_11_BIT)\n"
        )
        with patch("builtins.open", mock_open(read_data=fake_content)):
            result = check_ids.get_defines_from_file(Path("path.h"), "CANRX")
        self.assertEqual(
            result,
            [
                check_ids.FoundCanMessageDefine(
                    define_name="CANRX_BMS_STATE_REQUEST_ID",
                    msg_id="0x210",
                    where="path.h:1",
                    cyclic=False,
                ),
                check_ids.FoundCanMessageDefine(
                    define_name="CANRX_BMS_STATE_REQUEST_ID_TYPE",
                    msg_id="CAN_STANDARD_IDENTIFIER_11_BIT",
                    where="path.h:2",
                    cyclic=False,
                ),
            ],
        )

    def test_cyclic(self) -> None:
        """Cyclic is set explicitly"""
        fake_content = (
            "#define CANRX_BMS_STATE_REQUEST_ID (0x210u)\n"
            "#define CANRX_BMS_STATE_REQUEST_ID_TYPE (CAN_STANDARD_IDENTIFIER_11_BIT)\n"
        )
        with patch("builtins.open", mock_open(read_data=fake_content)):
            result = check_ids.get_defines_from_file(Path("path.h"), "CANRX", True)
        self.assertEqual(
            result,
            [
                check_ids.FoundCanMessageDefine(
                    define_name="CANRX_BMS_STATE_REQUEST_ID",
                    msg_id="0x210",
                    where="path.h:1",
                    cyclic=True,
                ),
                check_ids.FoundCanMessageDefine(
                    define_name="CANRX_BMS_STATE_REQUEST_ID_TYPE",
                    msg_id="CAN_STANDARD_IDENTIFIER_11_BIT",
                    where="path.h:2",
                    cyclic=True,
                ),
            ],
        )

    def test_with_non_defines(self) -> None:
        """Test with noise between the define lines"""
        fake_content = (
            "some\nlines\n"
            "#define CANRX_BMS_STATE_REQUEST_ID (0x210u)\n"
            "in\nbetween\n"
            "#define CANRX_BMS_STATE_REQUEST_ID_TYPE (CAN_STANDARD_IDENTIFIER_11_BIT)\n"
        )
        with patch("builtins.open", mock_open(read_data=fake_content)):
            result = check_ids.get_defines_from_file(Path("path.h"), "CANRX")
        self.assertEqual(
            result,
            [
                check_ids.FoundCanMessageDefine(
                    define_name="CANRX_BMS_STATE_REQUEST_ID",
                    msg_id="0x210",
                    where="path.h:3",
                    cyclic=False,
                ),
                check_ids.FoundCanMessageDefine(
                    define_name="CANRX_BMS_STATE_REQUEST_ID_TYPE",
                    msg_id="CAN_STANDARD_IDENTIFIER_11_BIT",
                    where="path.h:6",
                    cyclic=False,
                ),
            ],
        )

    def test_with_other_defines(self) -> None:
        """Test with noise between the define lines"""
        fake_content = (
            "#define NOTHING_IMPORTANT 100\n"
            "#define CANRX_BMS_STATE_REQUEST_ID (0x210u)\n"
            "#define IRRELEVANT 0\n"
            "#define CANRX_BMS_STATE_REQUEST_ID_TYPE (CAN_STANDARD_IDENTIFIER_11_BIT)\n"
        )
        with patch("builtins.open", mock_open(read_data=fake_content)):
            result = check_ids.get_defines_from_file(Path("path.h"), "CANRX")
        self.assertEqual(
            result,
            [
                check_ids.FoundCanMessageDefine(
                    define_name="CANRX_BMS_STATE_REQUEST_ID",
                    msg_id="0x210",
                    where="path.h:2",
                    cyclic=False,
                ),
                check_ids.FoundCanMessageDefine(
                    define_name="CANRX_BMS_STATE_REQUEST_ID_TYPE",
                    msg_id="CAN_STANDARD_IDENTIFIER_11_BIT",
                    where="path.h:4",
                    cyclic=False,
                ),
            ],
        )


class TestLogs(unittest.TestCase):
    """tests logs"""

    @patch("logging.info")
    def test_log_found_msgs(self, mock_log: MagicMock) -> None:
        """Test log found message prints right text to right logging level"""
        check_ids.log_found_msgs(
            "RX",
            [
                check_ids.FoundCanMessageDefine(
                    define_name="CANRX_BMS_STATE_REQUEST_ID",
                    msg_id="0x210",
                    where="path.h:3",
                    cyclic=False,
                )
            ],
        )
        mock_log.assert_called_once_with(
            "Implemented %s defines are:\n%s\n",
            "RX",
            "path.h:3: CANRX_BMS_STATE_REQUEST_ID : 0x210 [False]\n",
        )

    @patch("logging.debug")
    def test_log_found(self, mock_log: MagicMock) -> None:
        """Tests log found prints right text with correct logging level"""
        check_ids.log_found("test_define_name", "path.h:3")
        mock_log.assert_called_once_with(
            "-> Found expected: %s @ %s", "test_define_name", "path.h:3"
        )

    @patch("logging.error")
    def test_log_not_found(self, mock_log: MagicMock) -> None:
        """Tests log not found prints right text with correct logging level"""
        expected = check_ids.ExpectedCanMessageDefines(
            dbc_name="f_TestMessage",
            dbc_id="0xD",
            dbc_direction="tx",
            dbc_cyclic=False,
            exp_id_macro=("CANTX_TEST_MESSAGE_ID", ""),
            exp_id_type_macro=("CANTX_TEST_MESSAGE_ID_TYPE", ""),
            exp_period_macro=("Tx - async - ND", ""),
            exp_phase_macro=("Tx - async - ND", ""),
            exp_endianness_macro=("CANTX_TEST_MESSAGE_ENDIANNESS", ""),
            exp_dlc_macro=("CANTX_TEST_MESSAGE_DLC", ""),
            exp_full_msg_macro=("Tx - async - ND", ""),
        )
        check_ids.log_not_found(expected, 13, "path.h:3")
        mock_log.assert_called_once_with(
            "Did not find expected macro implementation '%s' for '%s' (%s) for in '%s'.",
            13,
            "f_TestMessage",
            "0xD",
            "path.h:3",
        )


class TestCheckExpectedMessageFormat(unittest.TestCase):
    """tests expected message format checker"""

    def test_correct_messages(self) -> None:
        """Correct messages should return no errors"""
        messages = [
            Message(
                name="f_FirstMessage",
                length=32,
                signals="",
                frame_id=0x100,
                comment="Message contains no information"
                "(in:can_cbs_tx_f_first_message.c:CANTX_FirstMessage,"
                " fv:tx, type:testing)",
            ),
            Message(
                name="SecondMessage",
                length=64,
                signals="",
                frame_id=0x50,
                comment="(in:can_cbs_tx_second_message.c:CANTX_SecondMessage,"
                " fv:tx, type:other testing)",
            ),
        ]
        errors = check_ids.check_sorted_message_format(messages)
        self.assertEqual(errors, 0)

    def test_without_comment(self) -> None:
        """No comment should return error"""
        messages = [
            Message(
                name="f_FirstMessage",
                length=32,
                signals="",
                frame_id=0x100,
                comment="Message contains no information"
                "(in:can_cbs_tx_f_first_message.c:CANTX_FirstMessage,"
                " fv:tx, type:testing)",
            ),
            Message(name="SecondMessage", length=64, signals="", frame_id=0x50),
        ]
        original_log_level = logging.getLogger().getEffectiveLevel()
        logging.disable(logging.CRITICAL)
        errors = check_ids.check_sorted_message_format(messages)
        logging.disable(original_log_level)
        self.assertEqual(errors, 1)

    def test_malformed_comment(self) -> None:
        """Malformed comment should return error"""
        messages = [
            Message(
                name="f_FirstMessage",
                length=32,
                signals="",
                frame_id=0x100,
                comment="Message contains no information(fv:tx, type:testing)",
            ),
            Message(
                name="SecondMessage",
                length=64,
                signals="",
                frame_id=0x50,
                comment="(in:can_cbs_tx_second_message.c:CANTX_SecondMessage,"
                " fv:tx, type:other testing)",
            ),
        ]
        original_log_level = logging.getLogger().getEffectiveLevel()
        logging.disable(logging.CRITICAL)
        errors = check_ids.check_sorted_message_format(messages)
        logging.disable(original_log_level)
        self.assertEqual(errors, 1)

    def test_multiple_errors(self) -> None:
        """Multiple errors should be summed up"""
        messages = [
            Message(name="f_FirstMessage", length=32, signals="", frame_id=0x100),
            Message(
                name="SecondMessage",
                length=64,
                signals="",
                frame_id=0x50,
                comment="(fv:tx, type:other testing)",
            ),
        ]
        original_log_level = logging.getLogger().getEffectiveLevel()
        logging.disable(logging.CRITICAL)
        errors = check_ids.check_sorted_message_format(messages)
        logging.disable(original_log_level)
        self.assertEqual(errors, 2)


class TestFindImplementedDefines(unittest.TestCase):
    """tests that implemented defines are found"""

    def test_basic(self) -> None:
        """Tests basic behaviour, should pass"""

        def get_defines(file_to_check: Path, _selector: str, _cyclic: bool = False):
            match file_to_check.parts:
                case ("cyclic_tx.path",):
                    return [
                        check_ids.FoundCanMessageDefine(
                            define_name="CANTX_BMS_TEST_ID",
                            msg_id="0x210",
                            where="cyclic_tx.path:3",
                            cyclic=True,
                        ),
                        check_ids.FoundCanMessageDefine(
                            define_name="CANTX_BMS_TEST_ID_TYPE",
                            msg_id="CAN_STANDARD_IDENTIFIER",
                            where="cyclic_tx.path:6",
                            cyclic=True,
                        ),
                    ]
                case ("async_tx.path",):
                    return [
                        check_ids.FoundCanMessageDefine(
                            define_name="CANTX_BMS_ASYNC_TEST_ID",
                            msg_id="0x420",
                            where="async_tx.path:3",
                            cyclic=False,
                        ),
                        check_ids.FoundCanMessageDefine(
                            define_name="CANTX_BMS_ASYNC_TEST_ID_TYPE",
                            msg_id="CAN_STANDARD_IDENTIFIER",
                            where="async_tx.path:6",
                            cyclic=False,
                        ),
                    ]
                case ("rx.path",):
                    return [
                        check_ids.FoundCanMessageDefine(
                            define_name="CANRX_BMS_TEST_ID",
                            msg_id="0x630",
                            where="rx.path:3",
                            cyclic=False,
                        ),
                        check_ids.FoundCanMessageDefine(
                            define_name="CANRX_BMS_TEST_ID_TYPE",
                            msg_id="CAN_STANDARD_IDENTIFIER",
                            where="rx.path:6",
                            cyclic=False,
                        ),
                    ]
                case _:
                    sys.exit("something went really wrong")

        with patch(
            "tests.can.check_ids.get_defines_from_file", side_effect=get_defines
        ):
            (all_tx_defines, all_rx_defines, all_defines) = (
                check_ids.find_implemented_defines(
                    Path("cyclic_tx.path"), Path("async_tx.path"), Path("rx.path")
                )
            )
            self.assertEqual(
                all_tx_defines,
                [
                    check_ids.FoundCanMessageDefine(
                        define_name="CANTX_BMS_TEST_ID",
                        msg_id="0x210",
                        where="cyclic_tx.path:3",
                        cyclic=True,
                    ),
                    check_ids.FoundCanMessageDefine(
                        define_name="CANTX_BMS_TEST_ID_TYPE",
                        msg_id="CAN_STANDARD_IDENTIFIER",
                        where="cyclic_tx.path:6",
                        cyclic=True,
                    ),
                    check_ids.FoundCanMessageDefine(
                        define_name="CANTX_BMS_ASYNC_TEST_ID",
                        msg_id="0x420",
                        where="async_tx.path:3",
                        cyclic=False,
                    ),
                    check_ids.FoundCanMessageDefine(
                        define_name="CANTX_BMS_ASYNC_TEST_ID_TYPE",
                        msg_id="CAN_STANDARD_IDENTIFIER",
                        where="async_tx.path:6",
                        cyclic=False,
                    ),
                ],
            )
            self.assertEqual(
                all_rx_defines,
                [
                    check_ids.FoundCanMessageDefine(
                        define_name="CANRX_BMS_TEST_ID",
                        msg_id="0x630",
                        where="rx.path:3",
                        cyclic=False,
                    ),
                    check_ids.FoundCanMessageDefine(
                        define_name="CANRX_BMS_TEST_ID_TYPE",
                        msg_id="CAN_STANDARD_IDENTIFIER",
                        where="rx.path:6",
                        cyclic=False,
                    ),
                ],
            )
            self.assertEqual(
                all_defines,
                [
                    check_ids.FoundCanMessageDefine(
                        define_name="CANTX_BMS_TEST_ID",
                        msg_id="0x210",
                        where="cyclic_tx.path:3",
                        cyclic=True,
                    ),
                    check_ids.FoundCanMessageDefine(
                        define_name="CANTX_BMS_ASYNC_TEST_ID",
                        msg_id="0x420",
                        where="async_tx.path:3",
                        cyclic=False,
                    ),
                    check_ids.FoundCanMessageDefine(
                        define_name="CANRX_BMS_TEST_ID",
                        msg_id="0x630",
                        where="rx.path:3",
                        cyclic=False,
                    ),
                ],
            )

    def test_empty_lists(self) -> None:
        """Empty input lists should return empty lists"""

        def get_defines(file_to_check: Path, _selector: str, _cyclic: bool = False):
            match file_to_check.parts:
                case ("cyclic_tx.path",):
                    return []
                case ("async_tx.path",):
                    return []
                case ("rx.path",):
                    return []
                case _:
                    sys.exit("something went really wrong")

        with patch(
            "tests.can.check_ids.get_defines_from_file", side_effect=get_defines
        ):
            (all_tx_defines, all_rx_defines, all_defines) = (
                check_ids.find_implemented_defines(
                    Path("cyclic_tx.path"), Path("async_tx.path"), Path("rx.path")
                )
            )
            self.assertEqual(all_tx_defines, [])
            self.assertEqual(all_rx_defines, [])
            self.assertEqual(all_defines, [])


class TestCheckMacroId(unittest.TestCase):
    """test correct macro id checks"""

    def test_correct_id(self) -> None:
        """Matching ids should pass"""
        all_implemented_defines = [
            check_ids.FoundCanMessageDefine(
                define_name="CANTX_BMS_TEST_ID",
                msg_id="0x210",
                where="cyclic_tx.path:3",
                cyclic=True,
            ),
        ]
        values = {
            "exp_id_macro": ["CANTX_BMS_TEST_ID"],
            "dbc_id": "0x210",
        }
        check_ids.check_macro_id("CANTX_BMS_TEST_ID", values, all_implemented_defines)

    def test_wrong_dbc_id(self) -> None:
        """Different ids should fail"""
        all_implemented_defines = [
            check_ids.FoundCanMessageDefine(
                define_name="CANTX_BMS_FAIL_TEST_ID",
                msg_id="0x310",
                where="async_tx.path:2",
                cyclic=False,
            ),
        ]
        values = {
            "exp_id_macro": ["CANTX_BMS_FAIL_TEST_ID"],
            "dbc_id": "0x110",
        }
        with self.assertRaises(SystemExit):
            check_ids.check_macro_id(
                "CANTX_BMS_FAIL_TEST_ID", values, all_implemented_defines
            )


class TestCompareImplementedExpected(unittest.TestCase):
    """tests combinations of expected and found defines"""

    def test_matching_rx(self) -> None:
        """Matching rx defines should pass"""
        expected_defines = [
            check_ids.ExpectedCanMessageDefines(
                dbc_name="TestMessage",
                dbc_id="0xF",
                dbc_direction="rx",
                dbc_cyclic=False,
                exp_id_macro=("CANRX_TEST_MESSAGE_ID", ""),
                exp_id_type_macro=("CANRX_TEST_MESSAGE_ID_TYPE", ""),
                exp_period_macro=("CANRX_TEST_MESSAGE_PERIOD_ms", ""),
                exp_phase_macro=("Rx - ND", ""),
                exp_endianness_macro=("CANRX_TEST_MESSAGE_ENDIANNESS", ""),
                exp_dlc_macro=("CANRX_TEST_MESSAGE_DLC", ""),
                exp_full_msg_macro=("CANRX_TEST_MESSAGE_MESSAGE", ""),
            )
        ]
        all_rx_defines = [
            check_ids.FoundCanMessageDefine(
                define_name="CANRX_TEST_MESSAGE_ID",
                msg_id="",
                where="",
                cyclic=False,
            ),
            check_ids.FoundCanMessageDefine(
                define_name="CANRX_TEST_MESSAGE_ID_TYPE",
                msg_id="",
                where="",
                cyclic=False,
            ),
            check_ids.FoundCanMessageDefine(
                define_name="CANRX_TEST_MESSAGE_PERIOD_ms",
                msg_id="",
                where="",
                cyclic=False,
            ),
            check_ids.FoundCanMessageDefine(
                define_name="CANRX_TEST_MESSAGE_ENDIANNESS",
                msg_id="",
                where="",
                cyclic=False,
            ),
            check_ids.FoundCanMessageDefine(
                define_name="CANRX_TEST_MESSAGE_DLC",
                msg_id="",
                where="",
                cyclic=False,
            ),
            check_ids.FoundCanMessageDefine(
                define_name="CANRX_TEST_MESSAGE_MESSAGE",
                msg_id="",
                where="",
                cyclic=False,
            ),
        ]
        all_tx_defines = []
        original_log_level = logging.getLogger().getEffectiveLevel()
        logging.disable(logging.CRITICAL)
        errors = check_ids.check_implemented_against_expected_macro(
            all_rx_defines,
            all_tx_defines,
            expected_defines,
            "rx_file.path",
            "tx_cyclic.path",
            "tx_async.path",
        )
        logging.disable(original_log_level)
        self.assertEqual(errors, 0)

    def test_matching_tx_cyclic(self) -> None:
        """Matching defines should pass"""
        expected_defines = [
            check_ids.ExpectedCanMessageDefines(
                dbc_name="TestMessage",
                dbc_id="0xF",
                dbc_direction="tx",
                dbc_cyclic=True,
                exp_id_macro=("CANTX_TEST_MESSAGE_ID", ""),
                exp_id_type_macro=("CANTX_TEST_MESSAGE_ID_TYPE", ""),
                exp_period_macro=("CANTX_TEST_MESSAGE_PERIOD_ms", ""),
                exp_phase_macro=("Tx - ND", ""),
                exp_endianness_macro=("CANTX_TEST_MESSAGE_ENDIANNESS", ""),
                exp_dlc_macro=("CANTX_TEST_MESSAGE_DLC", ""),
                exp_full_msg_macro=("CANTX_TEST_MESSAGE_MESSAGE", ""),
            )
        ]
        all_rx_defines = []
        all_tx_defines = [
            check_ids.FoundCanMessageDefine(
                define_name="CANTX_TEST_MESSAGE_ID",
                msg_id="",
                where="",
                cyclic=True,
            ),
            check_ids.FoundCanMessageDefine(
                define_name="CANTX_TEST_MESSAGE_ID_TYPE",
                msg_id="",
                where="",
                cyclic=True,
            ),
            check_ids.FoundCanMessageDefine(
                define_name="CANTX_TEST_MESSAGE_PERIOD_ms",
                msg_id="",
                where="",
                cyclic=True,
            ),
            check_ids.FoundCanMessageDefine(
                define_name="CANTX_TEST_MESSAGE_ENDIANNESS",
                msg_id="",
                where="",
                cyclic=True,
            ),
            check_ids.FoundCanMessageDefine(
                define_name="CANTX_TEST_MESSAGE_DLC",
                msg_id="",
                where="",
                cyclic=True,
            ),
            check_ids.FoundCanMessageDefine(
                define_name="CANTX_TEST_MESSAGE_MESSAGE",
                msg_id="",
                where="",
                cyclic=True,
            ),
        ]
        original_log_level = logging.getLogger().getEffectiveLevel()
        logging.disable(logging.CRITICAL)
        errors = check_ids.check_implemented_against_expected_macro(
            all_rx_defines,
            all_tx_defines,
            expected_defines,
            "rx_file.path",
            "tx_cyclic.path",
            "tx_async.path",
        )
        logging.disable(original_log_level)
        self.assertEqual(errors, 0)

    def test_matching_tx_async(self) -> None:
        """Matching defines should pass"""
        expected_defines = [
            check_ids.ExpectedCanMessageDefines(
                dbc_name="TestMessage",
                dbc_id="0xF",
                dbc_direction="tx",
                dbc_cyclic=False,
                exp_id_macro=("CANTX_TEST_MESSAGE_ID", ""),
                exp_id_type_macro=("CANTX_TEST_MESSAGE_ID_TYPE", ""),
                exp_period_macro=("CANTX_TEST_MESSAGE_PERIOD_ms", ""),
                exp_phase_macro=("Tx - ND", ""),
                exp_endianness_macro=("CANTX_TEST_MESSAGE_ENDIANNESS", ""),
                exp_dlc_macro=("CANTX_TEST_MESSAGE_DLC", ""),
                exp_full_msg_macro=("CANTX_TEST_MESSAGE_MESSAGE", ""),
            )
        ]
        all_rx_defines = []
        all_tx_defines = [
            check_ids.FoundCanMessageDefine(
                define_name="CANTX_TEST_MESSAGE_ID",
                msg_id="",
                where="",
                cyclic=False,
            ),
            check_ids.FoundCanMessageDefine(
                define_name="CANTX_TEST_MESSAGE_ID_TYPE",
                msg_id="",
                where="",
                cyclic=False,
            ),
            check_ids.FoundCanMessageDefine(
                define_name="CANTX_TEST_MESSAGE_PERIOD_ms",
                msg_id="",
                where="",
                cyclic=False,
            ),
            check_ids.FoundCanMessageDefine(
                define_name="CANTX_TEST_MESSAGE_ENDIANNESS",
                msg_id="",
                where="",
                cyclic=False,
            ),
            check_ids.FoundCanMessageDefine(
                define_name="CANTX_TEST_MESSAGE_DLC",
                msg_id="",
                where="",
                cyclic=False,
            ),
            check_ids.FoundCanMessageDefine(
                define_name="CANTX_TEST_MESSAGE_MESSAGE",
                msg_id="",
                where="",
                cyclic=False,
            ),
        ]
        original_log_level = logging.getLogger().getEffectiveLevel()
        logging.disable(logging.CRITICAL)
        errors = check_ids.check_implemented_against_expected_macro(
            all_rx_defines,
            all_tx_defines,
            expected_defines,
            "rx_file.path",
            "tx_cyclic.path",
            "tx_async.path",
        )
        logging.disable(original_log_level)
        self.assertEqual(errors, 0)

    def test_error_count_rx(self) -> None:
        """Should return the number of missing defines found"""
        expected_defines = [
            check_ids.ExpectedCanMessageDefines(
                dbc_name="TestMessage",
                dbc_id="0xF",
                dbc_direction="rx",
                dbc_cyclic=False,
                exp_id_macro=("CANRX_TEST_MESSAGE_ID", ""),
                exp_id_type_macro=("CANRX_TEST_MESSAGE_ID_TYPE", ""),
                exp_period_macro=("CANRX_TEST_MESSAGE_PERIOD_ms", ""),
                exp_phase_macro=("Rx - ND", ""),
                exp_endianness_macro=("CANRX_TEST_MESSAGE_ENDIANNESS", ""),
                exp_dlc_macro=("CANRX_TEST_MESSAGE_DLC", ""),
                exp_full_msg_macro=("CANRX_TEST_MESSAGE_MESSAGE", ""),
            )
        ]
        all_rx_defines = []
        all_tx_defines = []
        original_log_level = logging.getLogger().getEffectiveLevel()
        logging.disable(logging.CRITICAL)
        errors = check_ids.check_implemented_against_expected_macro(
            all_rx_defines,
            all_tx_defines,
            expected_defines,
            "rx_file.path",
            "tx_cyclic.path",
            "tx_async.path",
        )
        logging.disable(original_log_level)
        self.assertEqual(errors, 6)

    def test_error_count_tx(self) -> None:
        """Should return the number of missing defines found"""
        expected_defines = [
            check_ids.ExpectedCanMessageDefines(
                dbc_name="f_TestMessage",
                dbc_id="0xE",
                dbc_direction="tx",
                dbc_cyclic=True,
                exp_id_macro=("CANTX_TEST_MESSAGE_ID", ""),
                exp_id_type_macro=("CANTX_TEST_MESSAGE_ID_TYPE", ""),
                exp_period_macro=("CANTX_TEST_MESSAGE_PERIOD_ms", ""),
                exp_phase_macro=("CANTX_TEST_MESSAGE_PHASE_ms", ""),
                exp_endianness_macro=("CANTX_TEST_MESSAGE_ENDIANNESS", ""),
                exp_dlc_macro=("CANTX_TEST_MESSAGE_DLC", ""),
                exp_full_msg_macro=("CANTX_TEST_MESSAGE_MESSAGE", ""),
            )
        ]
        all_rx_defines = []
        all_tx_defines = []
        original_log_level = logging.getLogger().getEffectiveLevel()
        logging.disable(logging.CRITICAL)
        errors = check_ids.check_implemented_against_expected_macro(
            all_rx_defines,
            all_tx_defines,
            expected_defines,
            "rx_file.path",
            "tx_cyclic.path",
            "tx_async.path",
        )
        logging.disable(original_log_level)
        self.assertEqual(errors, 7)

    def test_malformed_direction(self) -> None:
        """Passing a wrong direction should result in a sys exit"""
        expected_defines = [
            check_ids.ExpectedCanMessageDefines(
                dbc_name="TestMessage",
                dbc_id="0xF",
                dbc_direction="malformed",
                dbc_cyclic=False,
                exp_id_macro=("CANRX_TEST_MESSAGE_ID", ""),
                exp_id_type_macro=("CANRX_TEST_MESSAGE_ID_TYPE", ""),
                exp_period_macro=("CANRX_TEST_MESSAGE_PERIOD_ms", ""),
                exp_phase_macro=("Rx - ND", ""),
                exp_endianness_macro=("CANRX_TEST_MESSAGE_ENDIANNESS", ""),
                exp_dlc_macro=("CANRX_TEST_MESSAGE_DLC", ""),
                exp_full_msg_macro=("CANRX_TEST_MESSAGE_MESSAGE", ""),
            )
        ]
        all_rx_defines = []
        all_tx_defines = []
        original_log_level = logging.getLogger().getEffectiveLevel()
        logging.disable(logging.CRITICAL)
        with self.assertRaises(SystemExit):
            check_ids.check_implemented_against_expected_macro(
                all_rx_defines,
                all_tx_defines,
                expected_defines,
                "rx_file.path",
                "tx_cyclic.path",
                "tx_async.path",
            )
        logging.disable(original_log_level)


class TestDefineOrder(unittest.TestCase):
    """Test if wrong ordering of defines is correctly recognized"""

    @patch("logging.error")
    def test_correct_lines(self, mock_log: MagicMock) -> None:
        """Should pass with every define in the correct line"""
        expected_defines = [
            check_ids.ExpectedCanMessageDefines(
                dbc_name="TestMessage",
                dbc_id="0xF",
                dbc_direction="rx",
                dbc_cyclic=False,
                exp_id_macro=("CANRX_TEST_MESSAGE_ID", "test.path:3"),
                exp_id_type_macro=("CANRX_TEST_MESSAGE_ID_TYPE", "test.path:4"),
                exp_period_macro=("CANRX_TEST_MESSAGE_PERIOD_ms", "test.path:5"),
                exp_phase_macro=("Rx - ND", ""),
                exp_endianness_macro=("CANRX_TEST_MESSAGE_ENDIANNESS", "test.path:6"),
                exp_dlc_macro=("CANRX_TEST_MESSAGE_DLC", "test.path:7"),
                exp_full_msg_macro=("CANRX_TEST_MESSAGE_MESSAGE", "test.path:8"),
            )
        ]
        check_ids.check_define_order(expected_defines)
        mock_log.assert_not_called()

    @patch("logging.error")
    def test_wrong_lines(self, mock_log: MagicMock) -> None:
        """Wrong define line numbers should be printed"""
        expected_defines = [
            check_ids.ExpectedCanMessageDefines(
                dbc_name="TestMessage",
                dbc_id="0xF",
                dbc_direction="rx",
                dbc_cyclic=False,
                exp_id_macro=("CANRX_TEST_MESSAGE_ID", "test.path:3"),
                exp_id_type_macro=("CANRX_TEST_MESSAGE_ID_TYPE", "test.path:4"),
                exp_period_macro=("CANRX_TEST_MESSAGE_PERIOD_ms", "test.path:6"),
                exp_phase_macro=("Rx - ND", ""),
                exp_endianness_macro=("CANRX_TEST_MESSAGE_ENDIANNESS", "test.path:6"),
                exp_dlc_macro=("CANRX_TEST_MESSAGE_DLC", "test.path:7"),
                exp_full_msg_macro=("CANRX_TEST_MESSAGE_MESSAGE", "test.path:8"),
            )
        ]
        check_ids.check_define_order(expected_defines)
        mock_log.assert_called_once()


class TestExpectedMessageExistence(unittest.TestCase):
    """Test for the existence of a message"""

    def test_rx_with_message(self) -> None:
        """The check with provided message should pass"""
        expected_defines = [
            check_ids.ExpectedCanMessageDefines(
                dbc_name="TestMessage",
                dbc_id="0xF",
                dbc_direction="rx",
                dbc_cyclic=False,
                exp_id_macro=("CANRX_TEST_MESSAGE_ID", "test.path:3"),
                exp_id_type_macro=("CANRX_TEST_MESSAGE_ID_TYPE", "test.path:4"),
                exp_period_macro=("CANRX_TEST_MESSAGE_PERIOD_ms", "test.path:6"),
                exp_phase_macro=("Rx - ND", ""),
                exp_endianness_macro=("CANRX_TEST_MESSAGE_ENDIANNESS", "test.path:6"),
                exp_dlc_macro=("CANRX_TEST_MESSAGE_DLC", "test.path:7"),
                exp_full_msg_macro=("CANRX_TEST_MESSAGE_MESSAGE", "test.path:8"),
            )
        ]
        rx_txt = (
            "#define CANRX_TEST_MESSAGE_MESSAGE \\\n"
            "    { \\\n"
            "    .id = CANRX_TEST_MESSAGE_ID, \\\n"
            "    .idType = CANRX_TEST_MESSAGE_ID_TYPE, \\\n"
            "    .dlc = CANRX_TEST_MESSAGE_DLC, \\\n"
            "    .endianness = CANRX_TEST_MESSAGE_ENDIANNESS, \\\n"
            "    }, \\\n"
            "    { \\\n"
            "    .period = CANRX_TEST_MESSAGE_PERIOD_ms \\\n"
            "    }\n"
        )
        tx_txt = ""
        original_log_level = logging.getLogger().getEffectiveLevel()
        logging.disable(logging.CRITICAL)
        errors = check_ids.check_expected_define_existence(
            expected_defines, rx_txt, tx_txt
        )
        logging.disable(original_log_level)
        self.assertEqual(errors, 0)

    def test_rx_without_message(self) -> None:
        """Check without provided message should fail"""
        expected_defines = [
            check_ids.ExpectedCanMessageDefines(
                dbc_name="TestMessage",
                dbc_id="0xF",
                dbc_direction="rx",
                dbc_cyclic=False,
                exp_id_macro=("CANRX_TEST_MESSAGE_ID", "test.path:3"),
                exp_id_type_macro=("CANRX_TEST_MESSAGE_ID_TYPE", "test.path:4"),
                exp_period_macro=("CANRX_TEST_MESSAGE_PERIOD_ms", "test.path:6"),
                exp_phase_macro=("Rx - ND", ""),
                exp_endianness_macro=("CANRX_TEST_MESSAGE_ENDIANNESS", "test.path:6"),
                exp_dlc_macro=("CANRX_TEST_MESSAGE_DLC", "test.path:7"),
                exp_full_msg_macro=("CANRX_TEST_MESSAGE_MESSAGE", "test.path:8"),
            )
        ]
        rx_txt = "#define CANRX_TEST_MESSAGE_ID (0xFu)\n"
        tx_txt = ""
        original_log_level = logging.getLogger().getEffectiveLevel()
        logging.disable(logging.CRITICAL)
        errors = check_ids.check_expected_define_existence(
            expected_defines, rx_txt, tx_txt
        )
        logging.disable(original_log_level)
        self.assertEqual(errors, 1)

    def test_tx_cyclic_with_message(self) -> None:
        """The check with provided message should pass"""
        expected_defines = [
            check_ids.ExpectedCanMessageDefines(
                dbc_name="TestMessage",
                dbc_id="0xF",
                dbc_direction="tx",
                dbc_cyclic=True,
                exp_id_macro=("CANTX_TEST_MESSAGE_ID", "test.path:3"),
                exp_id_type_macro=("CANTX_TEST_MESSAGE_ID_TYPE", "test.path:4"),
                exp_period_macro=("CANTX_TEST_MESSAGE_PERIOD_ms", "test.path:6"),
                exp_phase_macro=("CANTX_TEST_MESSAGE_PHASE_ms", "test.path:7"),
                exp_endianness_macro=("CANTX_TEST_MESSAGE_ENDIANNESS", "test.path:8"),
                exp_dlc_macro=("CANTX_TEST_MESSAGE_DLC", "test.path:9"),
                exp_full_msg_macro=("CANTX_TEST_MESSAGE_MESSAGE", "test.path:10"),
            )
        ]
        tx_txt = (
            "#define CANTX_TEST_MESSAGE_MESSAGE \\\n"
            "    { \\\n"
            "    .id = CANTX_TEST_MESSAGE_ID, \\\n"
            "    .idType = CANTX_TEST_MESSAGE_ID_TYPE, \\\n"
            "    .dlc = CANTX_TEST_MESSAGE_DLC, \\\n"
            "    .endianness = CANTX_TEST_MESSAGE_ENDIANNESS, \\\n"
            "    }, \\\n"
            "    { \\\n"
            "    .period = CANTX_TEST_MESSAGE_PERIOD_ms, \\\n"
            "    .phase = CANTX_TEST_MESSAGE_PHASE_ms \\\n"
            "    }\n"
        )
        rx_txt = ""
        original_log_level = logging.getLogger().getEffectiveLevel()
        logging.disable(logging.CRITICAL)
        errors = check_ids.check_expected_define_existence(
            expected_defines, rx_txt, tx_txt
        )
        logging.disable(original_log_level)
        self.assertEqual(errors, 0)

    def test_tx_cyclic_without_message(self) -> None:
        """Check without provided message should fail"""
        expected_defines = [
            check_ids.ExpectedCanMessageDefines(
                dbc_name="TestMessage",
                dbc_id="0xF",
                dbc_direction="tx",
                dbc_cyclic=True,
                exp_id_macro=("CANTX_TEST_MESSAGE_ID", "test.path:3"),
                exp_id_type_macro=("CANTX_TEST_MESSAGE_ID_TYPE", "test.path:4"),
                exp_period_macro=("CANTX_TEST_MESSAGE_PERIOD_ms", "test.path:6"),
                exp_phase_macro=("CANTX_TEST_MESSAGE_PHASE_ms", "test.path:7"),
                exp_endianness_macro=("CANTX_TEST_MESSAGE_ENDIANNESS", "test.path:8"),
                exp_dlc_macro=("CANTX_TEST_MESSAGE_DLC", "test.path:9"),
                exp_full_msg_macro=("CANTX_TEST_MESSAGE_MESSAGE", "test.path:10"),
            )
        ]
        rx_txt = "#define CANTX_TEST_MESSAGE_MESSAGE {}"
        tx_txt = ""
        original_log_level = logging.getLogger().getEffectiveLevel()
        logging.disable(logging.CRITICAL)
        errors = check_ids.check_expected_define_existence(
            expected_defines, rx_txt, tx_txt
        )
        logging.disable(original_log_level)
        self.assertEqual(errors, 1)

    def test_tx_async_without_message(self) -> None:
        """Should pass since tx_async does not expect a message"""
        expected_defines = [
            check_ids.ExpectedCanMessageDefines(
                dbc_name="TestMessage",
                dbc_id="0xE",
                dbc_direction="tx",
                dbc_cyclic=False,
                exp_id_macro=("CANTX_TEST_MESSAGE_ID", "test.path:3"),
                exp_id_type_macro=("CANTX_TEST_MESSAGE_ID_TYPE", "test.path:4"),
                exp_period_macro=("CANTX_TEST_MESSAGE_PERIOD_ms", "test.path:6"),
                exp_phase_macro=("Tx - ND", ""),
                exp_endianness_macro=("CANTX_TEST_MESSAGE_ENDIANNESS", "test.path:6"),
                exp_dlc_macro=("CANTX_TEST_MESSAGE_DLC", "test.path:7"),
                exp_full_msg_macro=("CANTX_TEST_MESSAGE_MESSAGE", "test.path:8"),
            )
        ]
        rx_txt = ""
        tx_txt = ""
        original_log_level = logging.getLogger().getEffectiveLevel()
        logging.disable(logging.CRITICAL)
        errors = check_ids.check_expected_define_existence(
            expected_defines, rx_txt, tx_txt
        )
        logging.disable(original_log_level)
        self.assertEqual(errors, 0)

    @patch("logging.error")
    def test_malformed_direction(self, mock_log: MagicMock) -> None:
        """Malformed direction should be logged"""
        expected_defines = [
            check_ids.ExpectedCanMessageDefines(
                dbc_name="TestMessage",
                dbc_id="0xF",
                dbc_direction="wx",
                dbc_cyclic=True,
                exp_id_macro=("CANTX_TEST_MESSAGE_ID", "test.path:3"),
                exp_id_type_macro=("CANTX_TEST_MESSAGE_ID_TYPE", "test.path:4"),
                exp_period_macro=("CANTX_TEST_MESSAGE_PERIOD_ms", "test.path:6"),
                exp_phase_macro=("CANTX_TEST_MESSAGE_PHASE_ms", "test.path:7"),
                exp_endianness_macro=("CANTX_TEST_MESSAGE_ENDIANNESS", "test.path:8"),
                exp_dlc_macro=("CANTX_TEST_MESSAGE_DLC", "test.path:9"),
                exp_full_msg_macro=("CANTX_TEST_MESSAGE_MESSAGE", "test.path:10"),
            )
        ]
        rx_txt = ""
        tx_txt = ""
        check_ids.check_expected_define_existence(expected_defines, rx_txt, tx_txt)
        mock_log.assert_called_once()


class TestArgumentParser(unittest.TestCase):
    """Tests the argument parser"""

    def test_no_args_provided(self) -> None:
        """Default values should be used"""
        result = check_ids.parse_args([])
        self.assertEqual(result.verbosity, 0)
        self.assertEqual(result.input_file, check_ids.BDC_DIR_REL / "foxbms.dbc")
        self.assertEqual(
            result.tx_cyclic_message_definition_file, check_ids.TX_CYCLIC_MESSAGES
        )
        self.assertEqual(
            result.tx_async_message_definition_file, check_ids.TX_ASYNC_MESSAGES
        )
        self.assertEqual(result.rx_message_definition_file, check_ids.RX_MESSAGES)


class TestMain(unittest.TestCase):
    """Tests main to check if everything works together as expected"""

    @patch("logging.error")
    @patch("pathlib.Path.read_text")
    @patch("builtins.open")
    @patch("pathlib.Path.is_file")
    @patch("cantools.database.load_file")
    @patch("tests.can.check_ids.parse_args")
    def test_correct_inputs(  # noqa: PLR0913
        self,
        args_mock: MagicMock,
        can_load_file_mock: MagicMock,
        is_file_mock: MagicMock,
        open_mock: MagicMock,
        read_text_mock: MagicMock,
        mock_logger: MagicMock,
    ) -> None:
        """Test with correct inputs should pass"""
        configs = [
            argparse.Namespace(
                verbosity=0,
                input_file="input.path",
                rx_message_definition_file="rx.path",
                tx_async_message_definition_file="tx_async.path",
                tx_cyclic_message_definition_file="tx_cyclic.path",
            ),
            argparse.Namespace(
                verbosity=1,
                input_file="input.path",
                rx_message_definition_file="rx.path",
                tx_async_message_definition_file="tx_async.path",
                tx_cyclic_message_definition_file="tx_cyclic.path",
            ),
            argparse.Namespace(
                verbosity=2,
                input_file="input.path",
                rx_message_definition_file="rx.path",
                tx_async_message_definition_file="tx_async.path",
                tx_cyclic_message_definition_file="tx_cyclic.path",
            ),
        ]
        tx_cyclic_file_content = (
            "#define CANTX_TX_CYCLIC_TEST_MESSAGE_ID (0xE)\n"
            "#define CANTX_TX_CYCLIC_TEST_MESSAGE_ID_TYPE "
            "(CAN_STANDARD_IDENTIFIER_11_BIT)\n"
            "#define CANTX_TX_CYCLIC_TEST_MESSAGE_PERIOD_ms (10u)\n"
            "#define CANTX_TX_CYCLIC_TEST_MESSAGE_PHASE_ms (0u)\n"
            "#define CANTX_TX_CYCLIC_TEST_MESSAGE_ENDIANNESS "
            "(CAN_BIG_ENDIAN)\n"
            "#define CANTX_TX_CYCLIC_TEST_MESSAGE_DLC (CAN_DEFAULT_DLC)\n"
            "#define CANTX_TX_CYCLIC_TEST_MESSAGE_MESSAGE \\\n"
            "    { \\\n"
            "    .id = CANTX_TX_CYCLIC_TEST_MESSAGE_ID, \\\n"
            "    .idType = CANTX_TX_CYCLIC_TEST_MESSAGE_ID_TYPE, \\\n"
            "    .dlc = CANTX_TX_CYCLIC_TEST_MESSAGE_DLC, \\\n"
            "    .endianness = CANTX_TX_CYCLIC_TEST_MESSAGE_ENDIANNESS, \\\n"
            "    }, \\\n"
            "    { \\\n"
            "    .period = CANTX_TX_CYCLIC_TEST_MESSAGE_PERIOD_ms, \\\n"
            "    .phase = CANTX_TX_CYCLIC_TEST_MESSAGE_PHASE_ms \\\n"
            "    }\n"
        )

        tx_async_file_content = (
            "#define CANTX_TX_ASYNC_TEST_MESSAGE_ID (0xD)\n"
            "#define CANTX_TX_ASYNC_TEST_MESSAGE_ID_TYPE "
            "(CAN_STANDARD_IDENTIFIER_11_BIT)\n"
            "#define CANTX_TX_ASYNC_TEST_MESSAGE_ENDIANNESS "
            "(CANTX_BIG_ENDIAN)\n"
            "#define CANTX_TX_ASYNC_TEST_MESSAGE_DLC (CAN_DEFAULT_DLC)\n"
        )

        rx_file_content = (
            "#define CANRX_RX_TEST_MESSAGE_ID (0xF)\n"
            "#define CANRX_RX_TEST_MESSAGE_ID_TYPE "
            "(CAN_STANDARD_IDENTIFIER_11_BIT)\n"
            "#define CANRX_RX_TEST_MESSAGE_PERIOD_ms (CANRX_NOT_PERIODIC)\n"
            "#define CANRX_RX_TEST_MESSAGE_ENDIANNESS (CAN_BIG_ENDIAN)\n"
            "#define CANRX_RX_TEST_MESSAGE_DLC (CAN_DEFAULT_DLC)\n"
            "\n"
            "#define CANRX_RX_TEST_MESSAGE_MESSAGE \\\n"
            "    { \\\n"
            "    .id = CANRX_RX_TEST_MESSAGE_ID, \\\n"
            "    .idType = CANRX_RX_TEST_MESSAGE_ID_TYPE, \\\n"
            "    .dlc = CANRX_RX_TEST_MESSAGE_DLC, \\\n"
            "    .endianness = CANRX_RX_TEST_MESSAGE_ENDIANNESS, \\\n"
            "    }, \\\n"
            "    { \\\n"
            "    .period = CANRX_RX_TEST_MESSAGE_PERIOD_ms \\\n"
            "    }\n"
        )

        for config in configs:
            args_mock.return_value = config
            can_load_file_mock.return_value.messages = [
                Message(
                    name="RxTestMessage",
                    comment="(in:can_cbs_rx_test_message.c:CANRX_TestMessage,"
                    " fv:rx, type:Cli Test)",
                    frame_id=15,
                    length=123,
                    signals="",
                ),
                Message(
                    name="TxCyclicTestMessage",
                    comment="(in:can_cbs_tx_cyclic_test_message.c:"
                    "CANTX_CyclicTestMessage, fv:tx, type:Cli Test)",
                    cycle_time=10,
                    frame_id=14,
                    length=123,
                    signals="",
                ),
                Message(
                    name="TxAsyncTestMessage",
                    comment="(in:can_cbs_tx_async_test_message.c:"
                    "CANTX_AsyncTestMessage, fv:tx, type:Cli Test)",
                    frame_id=13,
                    length=123,
                    signals="",
                ),
            ]
            is_file_mock.return_value = True

            def fake_open(file: Path | str, *_args, **_kwargs):
                if file == "expected-defines.json.log":
                    return mock_open()()
                if file == Path("rx.path"):
                    return mock_open(read_data=rx_file_content)()
                if file == Path("tx_cyclic.path"):
                    return mock_open(read_data=tx_cyclic_file_content)()
                if file == Path("tx_async.path"):
                    return mock_open(read_data=tx_async_file_content)()
                if file == "found-defines.json.log":
                    return mock_open()()
                sys.exit(f"File {file} is not implemented in test")

            open_mock.side_effect = fake_open

            def fake_read_text(self: Path, *_args, **_kwargs):
                if self == Path("tx_cyclic.path"):
                    return tx_cyclic_file_content
                if self == Path("rx.path"):
                    return rx_file_content
                sys.exit(f"File {self} is not implemented in fake_read_text")

            read_text_mock.side_effect = fake_read_text

            errors = check_ids.main([])

            mock_logger.assert_not_called()
            self.assertEqual(errors, 0)

    @patch("cantools.database.load_file")
    @patch("tests.can.check_ids.parse_args")
    def test_wrong_messages(
        self,
        args_mock: MagicMock,
        can_load_file_mock: MagicMock,
    ) -> None:
        """Test with wrong message should exit"""
        args_mock.return_value = argparse.Namespace(
            verbosity=0,
            input_file="input.path",
            rx_message_definition_file="rx.path",
            tx_async_message_definition_file="tx_async.path",
            tx_cyclic_message_definition_file="tx_cyclic.path",
        )
        can_load_file_mock.return_value.messages = [
            Message(
                name="RxTestMessage",
                comment="",
                frame_id=15,
                length=123,
                signals="",
            )
        ]

        original_log_level = logging.getLogger().getEffectiveLevel()
        logging.disable(logging.CRITICAL)
        with self.assertRaises(SystemExit):
            check_ids.main([])
        logging.disable(original_log_level)

    @patch("cantools.database.load_file")
    @patch("tests.can.check_ids.parse_args")
    def test_non_files(
        self,
        args_mock: MagicMock,
        can_load_file_mock: MagicMock,
    ) -> None:
        """Test with non files as input should exit"""
        args_mock.return_value = argparse.Namespace(
            verbosity=0,
            input_file="input.path",
            rx_message_definition_file="rx.path",
            tx_async_message_definition_file="tx_async.path",
            tx_cyclic_message_definition_file="tx_cyclic.path",
        )
        can_load_file_mock.return_value.messages = [
            Message(
                name="RxTestMessage",
                comment="(in:can_cbs_tx_cyclic_test_message.c:"
                "CANTX_CyclicTestMessage, fv:tx, type:Cli Test)",
                frame_id=15,
                length=123,
                signals="",
            )
        ]

        with self.assertRaises(SystemExit):
            check_ids.main([])

    @patch("builtins.open")
    @patch("pathlib.Path.is_file")
    @patch("cantools.database.load_file")
    @patch("tests.can.check_ids.parse_args")
    def test_expected_and_found_not_matching(
        self,
        args_mock: MagicMock,
        can_load_file_mock: MagicMock,
        is_file_mock: MagicMock,
        open_mock: MagicMock,
    ) -> None:
        """Test with wrong inputs should fail"""
        args_mock.return_value = argparse.Namespace(
            verbosity=0,
            input_file="input.path",
            rx_message_definition_file="rx.path",
            tx_async_message_definition_file="tx_async.path",
            tx_cyclic_message_definition_file="tx_cyclic.path",
        )
        can_load_file_mock.return_value.messages = [
            Message(
                name="RxTestMessage",
                comment="(in:can_cbs_rx_test_message.c:CANRX_TestMessage,"
                " fv:rx, type:Cli Test)",
                frame_id=15,
                length=123,
                signals="",
            ),
            Message(
                name="TxCyclicTestMessage",
                comment="(in:can_cbs_tx_cyclic_test_message.c:"
                "CANTX_CyclicTestMessage, fv:tx, type:Cli Test)",
                cycle_time=10,
                frame_id=14,
                length=123,
                signals="",
            ),
            Message(
                name="TxAsyncTestMessage",
                comment="(in:can_cbs_tx_async_test_message.c:"
                "CANTX_AsyncTestMessage, fv:tx, type:Cli Test)",
                frame_id=13,
                length=123,
                signals="",
            ),
        ]
        is_file_mock.return_value = True

        tx_cyclic_file_content = (
            "#define CANTX_TX_CYCLIC_TEST_MESSAGE_ID (0xE)\n"
            "#define CANTX_TX_CYCLIC_TEST_MESSAGE_ID_TYPE "
            "(CAN_STANDARD_IDENTIFIER_11_BIT)\n"
            "#define CANTX_TX_CYCLIC_TEST_MESSAGE_PERIOD_ms (10u)\n"
            "#define CANTX_TX_CYCLIC_TEST_MESSAGE_ENDIANNESS "
            "(CAN_BIG_ENDIAN)\n"
            "#define CANTX_TX_CYCLIC_TEST_MESSAGE_DLC (CAN_DEFAULT_DLC)\n"
            "#define CANTX_TX_CYCLIC_TEST_MESSAGE_MESSAGE \\\n"
            "    { \\\n"
            "    .id = CANTX_TX_CYCLIC_TEST_MESSAGE_ID, \\\n"
            "    .idType = CANTX_TX_CYCLIC_TEST_MESSAGE_ID_TYPE, \\\n"
            "    .dlc = CANTX_TX_CYCLIC_TEST_MESSAGE_DLC, \\\n"
            "    .endianness = CANTX_TX_CYCLIC_TEST_MESSAGE_ENDIANNESS, \\\n"
            "    }, \\\n"
            "    { \\\n"
            "    .period = CANTX_TX_CYCLIC_TEST_MESSAGE_PERIOD_ms, \\\n"
            "    .phase = CANTX_TX_CYCLIC_TEST_MESSAGE_PHASE_ms \\\n"
            "    }\n"
        )

        tx_async_file_content = (
            "#define CANTX_TX_ASYNC_TEST_MESSAGE_ID (0xD)\n"
            "#define CANTX_TX_ASYNC_TEST_MESSAGE_ID_TYPE "
            "(CAN_STANDARD_IDENTIFIER_11_BIT)\n"
            "#define CANTX_TX_ASYNC_TEST_MESSAGE_ENDIANNESS "
            "(CANTX_BIG_ENDIAN)\n"
            "#define CANTX_TX_ASYNC_TEST_MESSAGE_DLC (CAN_DEFAULT_DLC)\n"
        )

        rx_file_content = (
            "#define CANRX_RX_TEST_MESSAGE_ID (0xF)\n"
            "#define CANRX_RX_TEST_MESSAGE_ID_TYPE "
            "(CAN_STANDARD_IDENTIFIER_11_BIT)\n"
            "#define CANRX_RX_TEST_MESSAGE_PERIOD_ms (CANRX_NOT_PERIODIC)\n"
            "#define CANRX_RX_TEST_MESSAGE_ENDIANNESS (CAN_BIG_ENDIAN)\n"
            "#define CANRX_RX_TEST_MESSAGE_DLC (CAN_DEFAULT_DLC)\n"
            "\n"
            "#define CANRX_RX_TEST_MESSAGE_MESSAGE \\\n"
            "    { \\\n"
            "    .id = CANRX_RX_TEST_MESSAGE_ID, \\\n"
            "    .idType = CANRX_RX_TEST_MESSAGE_ID_TYPE, \\\n"
            "    .dlc = CANRX_RX_TEST_MESSAGE_DLC, \\\n"
            "    .endianness = CANRX_RX_TEST_MESSAGE_ENDIANNESS, \\\n"
            "    }, \\\n"
            "    { \\\n"
            "    .period = CANRX_RX_TEST_MESSAGE_PERIOD_ms \\\n"
            "    }\n"
        )

        def fake_open(file: Path | str, *_args, **_kwargs):
            if file == "expected-defines.json.log":
                return mock_open()()
            if file == Path("rx.path"):
                return mock_open(read_data=rx_file_content)()
            if file == Path("tx_cyclic.path"):
                return mock_open(read_data=tx_cyclic_file_content)()
            if file == Path("tx_async.path"):
                return mock_open(read_data=tx_async_file_content)()
            if file == "found-defines.json.log":
                return mock_open()()
            sys.exit(f"File {file} is not implemented in test")

        open_mock.side_effect = fake_open

        original_log_level = logging.getLogger().getEffectiveLevel()
        logging.disable(logging.CRITICAL)
        with self.assertRaises(SystemExit):
            check_ids.main([])
        logging.disable(original_log_level)


if __name__ == "__main__":
    unittest.main()
