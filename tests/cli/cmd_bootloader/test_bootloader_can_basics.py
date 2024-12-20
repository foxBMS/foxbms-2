#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Testing file 'cli/cmd_bootloader/bootloader_can_basics.py'."""

import io
import sys
import unittest
from pathlib import Path

import can

# Redirect message or not
MSG_REDIRECT = True

sys.path.insert(0, str(Path(__file__).parent.parent.parent.parent))

# pylint: disable=protected-access
# pylint: disable=wrong-import-position
from cli.cmd_bootloader.bootloader_can import BootloaderCanBasics  # noqa: E402
from cli.cmd_bootloader.bootloader_can_messages import (  # noqa: E402
    AcknowledgeFlag,
    AcknowledgeMessage,
    BootFsmState,
    CanFsmState,
    RequestCode,
    StatusCode,
    YesNoAnswer,
)


class TestBootloaderCanBasics(unittest.TestCase):
    """Class to test the class BootloaderCanBasics."""

    def setUp(self):
        # Redirect the sys.stdout to the StringIO object
        if MSG_REDIRECT:
            sys.stdout = io.StringIO()

        # Initialize virtual CAN bus instance
        self.can_bus = can.interface.Bus(
            "test", interface="virtual", preserve_timestamps=True
        )
        self.can_bus_test = can.interface.Bus(
            "test", interface="virtual", preserve_timestamps=True
        )

        # Initialize BootloaderCanBasics instance with virtual CAN bus instance
        self.bl = BootloaderCanBasics(can_bus=self.can_bus_test)

    def tearDown(self):
        self.can_bus.shutdown()
        self.can_bus_test.shutdown()

        # Reset sys.stdout to its original value
        sys.stdout = sys.__stdout__

    def test_init(self):
        """Function to test the init function."""
        # Test if the system exception will be raised if dbc_file is not a file
        dbc_file = Path("./fake")
        with self.assertRaises(SystemExit) as cm:
            BootloaderCanBasics(can_bus=self.can_bus_test, dbc_file=dbc_file)
        self.assertEqual(cm.exception.code, f"File '{dbc_file}' does not exist.")

    def test_wait_can_message(self):
        """Function to test function test_wait_can_message()."""
        # Case 1: return not none
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToTransferProgram.value,
            "StatusCode": StatusCode.ReceivedAndProcessed.value,
            "YesNoAnswer": YesNoAnswer.No.value,
        }
        db_message = self.bl.db.get_message_by_name("f_AcknowledgeMessage")
        data = db_message.encode(msg)
        test_message = can.Message(arbitration_id=db_message.frame_id, data=data)
        self.can_bus.send(test_message)
        msg_waited = self.bl.wait_can_message(
            arbitration_id=db_message.frame_id,
            params={"AcknowledgeMessage": "ReceivedCmdToTransferProgram"},
        )
        self.assertEqual(
            msg_waited,
            {
                "AcknowledgeFlag": AcknowledgeFlag.Received.name,
                "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToTransferProgram.name,
                "StatusCode": StatusCode.ReceivedAndProcessed.name,
                "YesNoAnswer": YesNoAnswer.No.name,
            },
        )

        # Case 2: return none
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToTransferProgram.value,
            "StatusCode": StatusCode.ReceivedAndProcessed.value,
            "YesNoAnswer": YesNoAnswer.No.value,
        }
        db_message = self.bl.db.get_message_by_name("f_AcknowledgeMessage")
        data = db_message.encode(msg)
        test_message = can.Message(arbitration_id=db_message.frame_id, data=data)
        self.can_bus.send(test_message)
        msg_waited = self.bl.wait_can_message(
            arbitration_id=db_message.frame_id,
            params={"AcknowledgeMessage": "ReceivedProgramInfo"},
            timeout_total=0.1,
            timeout_bus_recv=0.1,
        )
        self.assertIsNone(msg_waited)

        # Case 3: not valid key in input param
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToTransferProgram.value,
            "StatusCode": StatusCode.ReceivedAndProcessed.value,
            "YesNoAnswer": YesNoAnswer.No.value,
        }
        db_message = self.bl.db.get_message_by_name("f_AcknowledgeMessage")
        data = db_message.encode(msg)
        test_message = can.Message(arbitration_id=db_message.frame_id, data=data)
        self.can_bus.send(test_message)
        msg_waited = self.bl.wait_can_message(
            arbitration_id=db_message.frame_id,
            params={
                "fake": "ReceivedProgramInfo",
                "AcknowledgeMessage": "ReceivedProgramInfo",
            },
            timeout_total=0.1,
            timeout_bus_recv=0.1,
        )
        self.assertIsNone(msg_waited)

    def test_send_request_to_bootloader(self):
        """Function to test function send_request_to_bootloader()."""
        self.bl.send_request_to_bootloader(RequestCode.CmdToRunProgram)
        message = self.can_bus.recv()
        msg = self.bl.db.decode_message(message.arbitration_id, message.data)
        self.assertEqual(msg, {"RequestCode8Bits": "CmdToRunProgram"})

    def test_send_data_to_bootloader(self):
        """Function to test function send_data_to_bootloader()."""
        self.bl.send_data_to_bootloader(0x1FFFFFFFFFFFFFFF)
        message = self.can_bus.recv()
        msg = self.bl.db.decode_message(message.arbitration_id, message.data)
        self.assertEqual(msg, {"Data": 0x1FFFFFFFFFFFFFFF})

    def test_send_crc_to_bootloader(self):
        """Function to test function send_crc_to_bootloader()."""
        self.bl.send_crc_to_bootloader(0x1FFFFFFFFFFFFFF0)
        message = self.can_bus.recv()
        msg = self.bl.db.decode_message(message.arbitration_id, message.data)
        self.assertEqual(msg, {"Crc": 0x1FFFFFFFFFFFFFF0})

    def test_send_transfer_program_info_to_bootloader(self):
        """Function to test function send_transfer_program_info_to_bootloader()."""
        self.bl.send_transfer_program_info_to_bootloader(16, 2)
        message = self.can_bus.recv()
        msg = self.bl.db.decode_message(message.arbitration_id, message.data)
        self.assertEqual(msg, {"ProgramLength": 16, "RequiredTransferLoops": 2})

    def test_send_loop_number_to_bootloader(self):
        """Function to test function send_loop_number_to_bootloader()."""
        self.bl.send_loop_number_to_bootloader(3)
        message = self.can_bus.recv()
        msg = self.bl.db.decode_message(message.arbitration_id, message.data)
        self.assertEqual(msg, {"LoopNumber": 3})

    def test_wait_bootloader_state_msg(self):
        """Function to test function wait_bootloader_state_msg()."""
        msg = {
            "BootFsmState": BootFsmState.BootFsmStateWait.value,
            "CanFsmState": CanFsmState.CanFsmStateNoCommunication.value,
        }
        db_message = self.bl.db.get_message_by_name("f_BootloaderFsmStates")
        data = db_message.encode(msg)
        test_message = can.Message(arbitration_id=db_message.frame_id, data=data)
        self.can_bus.send(test_message)

        msg_waited = self.bl.wait_bootloader_state_msg()
        self.assertEqual(
            msg_waited,
            {
                "BootFsmState": BootFsmState.BootFsmStateWait.name,
                "CanFsmState": CanFsmState.CanFsmStateNoCommunication.name,
            },
        )

    def test_wait_data_transfer_info_msg(self):
        """Function to test function wait_data_transfer_info_msg()."""
        msg = {"CurrentLoopNumber": 100}
        db_message = self.bl.db.get_message_by_name("f_DataTransferInfo")
        data = db_message.encode(msg)
        test_message = can.Message(arbitration_id=db_message.frame_id, data=data)
        self.can_bus.send(test_message)

        msg_waited = self.bl.wait_data_transfer_info_msg()
        self.assertEqual(msg_waited, msg)

    def test_wait_can_ack_msg(self):
        """Function to test function wait_can_ack_msg()."""
        # Case 1: received the ACK message with the StatusCode ReceivedAndProcessed
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToTransferProgram.value,
            "StatusCode": StatusCode.ReceivedAndProcessed.value,
            "YesNoAnswer": YesNoAnswer.No.value,
        }
        db_message = self.bl.db.get_message_by_name("f_AcknowledgeMessage")
        data = db_message.encode(msg)
        test_message = can.Message(arbitration_id=db_message.frame_id, data=data)
        self.can_bus.send(test_message)
        msg_waited = self.bl.wait_can_ack_msg(
            AcknowledgeMessage.ReceivedCmdToTransferProgram,
            timeout_total=0.1,
            timeout_ack=0.01,
            processed_level=StatusCode.ReceivedAndProcessed,
        )
        self.assertEqual(
            msg_waited,
            {
                "AcknowledgeFlag": AcknowledgeFlag.Received.name,
                "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToTransferProgram.name,
                "StatusCode": StatusCode.ReceivedAndProcessed.name,
                "YesNoAnswer": YesNoAnswer.No.name,
            },
        )

        # Case 2: received the ACK message with the StatusCode no ReceivedAndProcessed
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToTransferProgram.value,
            "StatusCode": StatusCode.ReceivedButNotProcessed.value,
            "YesNoAnswer": YesNoAnswer.No.value,
        }
        db_message = self.bl.db.get_message_by_name("f_AcknowledgeMessage")
        data = db_message.encode(msg)
        test_message = can.Message(arbitration_id=db_message.frame_id, data=data)
        self.can_bus.send(test_message)
        msg_waited = self.bl.wait_can_ack_msg(
            AcknowledgeMessage.ReceivedCmdToTransferProgram,
            timeout_total=0.1,
            timeout_ack=0.01,
            processed_level=StatusCode.ReceivedAndProcessed,
        )
        self.assertIsNone(msg_waited)


if __name__ == "__main__":
    unittest.main()
