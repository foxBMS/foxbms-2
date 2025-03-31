#!/usr/bin/env python3
#
# Copyright (c) 2010 - 2025, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
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

"""Testing file 'cli/cmd_bootloader/bootloader_can.py'."""

import logging
import sys
import time
import unittest
from pathlib import Path
from unittest.mock import MagicMock, patch

import can

try:
    from cli.cmd_bootloader.bootloader_can import (
        BootloaderCanBasics,
        BootloaderInterfaceCan,
    )
    from cli.cmd_bootloader.bootloader_can_messages import (
        AcknowledgeFlag,
        AcknowledgeMessage,
        BootFsmState,
        CanFsmState,
        StatusCode,
        YesNoAnswer,
    )
except ModuleNotFoundError:
    sys.path.insert(0, str(Path(__file__).parents[3]))
    from cli.cmd_bootloader.bootloader_can import (
        BootloaderCanBasics,
        BootloaderInterfaceCan,
    )
    from cli.cmd_bootloader.bootloader_can_messages import (
        AcknowledgeFlag,
        AcknowledgeMessage,
        BootFsmState,
        CanFsmState,
        StatusCode,
        YesNoAnswer,
    )


# pylint: disable=unused-argument
@patch.object(logging, "info", return_value=None)
@patch.object(logging, "warning", return_value=None)
@patch.object(logging, "error", return_value=None)
@patch.object(time, "sleep", return_value=None)
class TestBootloaderInterfaceCan(unittest.TestCase):
    """Class to test the class BootloaderInterfaceCan."""

    def setUp(self):
        # Initialize virtual CAN bus instance
        self.can_bus = can.interface.Bus(
            "test", interface="virtual", preserve_timestamps=True
        )
        self.can_bus_test = can.interface.Bus(
            "test", interface="virtual", preserve_timestamps=True
        )

        # Initialize BootloaderCanBasics instance with virtual CAN bus instance
        self.bl = BootloaderInterfaceCan(can_bus=self.can_bus_test)

    def tearDown(self):
        self.can_bus.shutdown()
        self.can_bus_test.shutdown()

    def test_send_crc(self, *args):
        """Function to test the function send_crc()."""
        # Case 1: if the input is_crc_of_vector_table is True
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedCrcOfVectorTable.value,
            "StatusCode": StatusCode.ReceivedAndProcessed.value,
            "YesNoAnswer": YesNoAnswer.Yes.value,
        }
        self.send_test_ack_message(msg)
        ret_val_1_1, ret_val_2_1 = self.bl.send_crc(
            crc_8_bytes=0x2FFFFFFFFFFFFFFF, is_crc_of_vector_table=True
        )
        self.assertTrue(ret_val_1_1)
        self.assertTrue(ret_val_2_1)

        # Case 2: return true, true (answer is yes)
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.Received8BytesCrc.value,
            "StatusCode": StatusCode.ReceivedAndProcessed.value,
            "YesNoAnswer": YesNoAnswer.Yes.value,
        }
        self.send_test_ack_message(msg)
        ret_val_1_1, ret_val_2_1 = self.bl.send_crc(0x2FFFFFFFFFFFFFFF)
        self.assertTrue(ret_val_1_1)
        self.assertTrue(ret_val_2_1)

        # Case 3: return true, false (answer is no)
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.Received8BytesCrc.value,
            "StatusCode": StatusCode.ReceivedAndProcessed.value,
            "YesNoAnswer": YesNoAnswer.No.value,
        }
        self.send_test_ack_message(msg)
        ret_val_1_2, ret_val_2_2 = self.bl.send_crc(0x2FFFFFFFFFFFFFFF)
        self.assertTrue(ret_val_1_2)
        self.assertFalse(ret_val_2_2)

        # Case 4: if the ack message is None (Put this case at the end, because
        # in this case the return of wait_can_ack_msg is mocked.)
        self.bl.can.wait_can_ack_msg = MagicMock()
        self.bl.can.wait_can_ack_msg.return_value = None
        ret_val_1_1, ret_val_2_1 = self.bl.send_crc(crc_8_bytes=0x2FFFFFFFFFFFFFFF)
        self.assertFalse(ret_val_1_1)
        self.assertFalse(ret_val_2_1)

    def test_send_program_info(self, *args):
        """Function to test the function send_program_info()."""
        ## Case 1: return True
        # Send beforehand the valid ACK message
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedProgramInfo.value,
            "StatusCode": StatusCode.ReceivedAndProcessed.value,
            "YesNoAnswer": YesNoAnswer.Yes.value,
        }
        self.send_test_ack_message(msg)

        # Send beforehand the valid state message of bootloader
        msg = {
            "BootFsmState": BootFsmState.BootFsmStateWait.value,
            "CanFsmState": CanFsmState.CanFsmStateWaitForDataLoops.value,
        }
        self.send_test_status_message(msg)

        ret_val_1 = self.bl.send_program_info(16, 2)
        self.assertTrue(ret_val_1)

        ## Case 2: return false
        # Send beforehand the valid ACK message
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedProgramInfo.value,
            "StatusCode": StatusCode.ReceivedAndProcessed.value,
            "YesNoAnswer": YesNoAnswer.No.value,
        }
        self.send_test_ack_message(msg)

        # Send beforehand the invalid state message of bootloader
        msg = {
            "BootFsmState": BootFsmState.BootFsmStateWait.value,
            "CanFsmState": CanFsmState.CanFsmStateError.value,
        }
        self.send_test_status_message(msg)
        ret_val_2 = self.bl.send_program_info(16, 2)
        self.assertFalse(ret_val_2)

        # Case 4: if the returned can_fsm_state is not 'CanFsmStateWaitForDataLoops'
        # Send beforehand the valid ACK message
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedProgramInfo.value,
            "StatusCode": StatusCode.ReceivedAndProcessed.value,
            "YesNoAnswer": YesNoAnswer.Yes.value,
        }
        self.send_test_ack_message(msg)
        self.bl.get_bootloader_state = MagicMock()
        self.bl.get_bootloader_state.return_value = (None, None)
        ret_val_2 = self.bl.send_program_info(16, 2)
        self.assertFalse(ret_val_2)

        # Case 3: if the ack message is None
        self.bl.can.wait_can_ack_msg = MagicMock()
        self.bl.can.wait_can_ack_msg.return_value = None
        ret_val_2 = self.bl.send_program_info(16, 2)
        self.assertFalse(ret_val_2)

    def test_send_loop_number_to_bootloader(self, *args):
        """Function to test the function send_loop_number_to_bootloader()."""
        self.bl.can.send_loop_number_to_bootloader = MagicMock()
        self.bl.can.send_loop_number_to_bootloader.return_value = False
        self.assertFalse(self.bl.send_loop_number_to_bootloader(10))

    def test_send_data_to_bootloader(self, *args):
        """Function to test the function send_data_to_bootloader()."""
        self.bl.can.send_data_to_bootloader = MagicMock()
        self.bl.can.send_data_to_bootloader.return_value = False
        self.assertFalse(self.bl.send_data_to_bootloader(0x1FFFFFFFFFFFFFFF))

    def test_wait_can_ack_msg(self, *args):
        """Function to test the function wait_can_ack_msg()."""
        self.bl.can.wait_can_ack_msg = MagicMock()
        self.bl.can.wait_can_ack_msg.return_value = None
        self.assertIsNone(
            self.bl.wait_can_ack_msg(
                AcknowledgeMessage.ReceivedCmdToTransferProgram,
                timeout_total=0.1,
                timeout_ack=0.01,
                processed_level=StatusCode.ReceivedAndProcessed,
            )
        )

    def test_start_transfer(self, *args_):
        """Function to test the function start_transfer()."""
        ## Case 1: return True
        # Send beforehand the valid ACK message
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToTransferProgram.value,
            "StatusCode": StatusCode.ReceivedAndProcessed.value,
            "YesNoAnswer": YesNoAnswer.No.value,
        }
        self.send_test_ack_message(msg)
        # Send beforehand the valid state message of bootloader
        msg = {
            "BootFsmState": BootFsmState.BootFsmStateWait.value,
            "CanFsmState": CanFsmState.CanFsmStateWaitForInfo.value,
        }
        self.send_test_status_message(msg)

        ret_val_1 = self.bl.start_transfer()
        self.assertTrue(ret_val_1)

        ## Case 2: return false
        # Send beforehand the valid ACK message
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToTransferProgram.value,
            "StatusCode": StatusCode.ReceivedAndProcessed.value,
            "YesNoAnswer": YesNoAnswer.No.value,
        }
        self.send_test_ack_message(msg)

        # Send beforehand the invalid state message of bootloader
        msg = {
            "BootFsmState": BootFsmState.BootFsmStateWait.value,
            "CanFsmState": CanFsmState.CanFsmStateError.value,
        }
        self.send_test_status_message(msg)

        ret_val_2 = self.bl.start_transfer()
        self.assertFalse(ret_val_2)

        # if the ack message is None
        self.bl.can.wait_can_ack_msg = MagicMock()
        self.bl.can.wait_can_ack_msg.return_value = None
        ret_val_2 = self.bl.start_transfer()
        self.assertFalse(ret_val_2)

    def test_reset_bootloader(self, *args):
        """Function to test the function reset_bootloader()."""
        ## Case 1: return True
        # Send beforehand the valid ACK message
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToResetBootProcess.value,
            "StatusCode": StatusCode.ReceivedAndInProcessing.value,
            "YesNoAnswer": YesNoAnswer.No.value,
        }
        self.send_test_ack_message(msg)

        # Send beforehand the valid state message of bootloader
        msg = {
            "BootFsmState": BootFsmState.BootFsmStateWait.name,
            "CanFsmState": CanFsmState.CanFsmStateNoCommunication.name,
        }
        self.send_test_status_message(msg)

        # Send beforehand the current loop number
        msg = {"CurrentLoopNumber": 0}
        db_message = self.bl.can.db.get_message_by_name("f_DataTransferInfo")
        data = db_message.encode(msg)
        test_message = can.Message(arbitration_id=db_message.frame_id, data=data)
        self.can_bus.send(test_message)

        ret_val_1 = self.bl.reset_bootloader(time_to_wait=0.1)
        self.assertTrue(ret_val_1)

        ## Case 2: return false 1: invalid current loop number
        # Send beforehand the valid ACK message
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToResetBootProcess.value,
            "StatusCode": StatusCode.ReceivedAndInProcessing.value,
            "YesNoAnswer": YesNoAnswer.No.value,
        }
        self.send_test_ack_message(msg)

        # Send beforehand the invalid state message of bootloader
        msg = {
            "BootFsmState": BootFsmState.BootFsmStateWait.name,
            "CanFsmState": CanFsmState.CanFsmStateNoCommunication.name,
        }
        self.send_test_status_message(msg)

        # Send beforehand the current loop number
        msg = {"CurrentLoopNumber": 1000}
        db_message = self.bl.can.db.get_message_by_name("f_DataTransferInfo")
        data = db_message.encode(msg)
        test_message = can.Message(arbitration_id=db_message.frame_id, data=data)
        self.can_bus.send(test_message)

        ret_val_2 = self.bl.reset_bootloader(time_to_wait=0.1)
        self.assertFalse(ret_val_2)

        ## Case 3: return false 2: invalid CAN fsm state
        # Send beforehand the valid ACK message
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToResetBootProcess.value,
            "StatusCode": StatusCode.ReceivedAndInProcessing.value,
            "YesNoAnswer": YesNoAnswer.No.value,
        }
        self.send_test_ack_message(msg)

        # Send beforehand the invalid state message of bootloader
        msg = {
            "BootFsmState": BootFsmState.BootFsmStateLoad.name,
            "CanFsmState": CanFsmState.CanFsmStateError.name,
        }
        self.send_test_status_message(msg)

        # Send beforehand the current loop number
        msg = {"CurrentLoopNumber": 0}
        db_message = self.bl.can.db.get_message_by_name("f_DataTransferInfo")
        data = db_message.encode(msg)
        test_message = can.Message(arbitration_id=db_message.frame_id, data=data)
        self.can_bus.send(test_message)

        ret_val_3 = self.bl.reset_bootloader(time_to_wait=0.1)
        self.assertFalse(ret_val_3)

        ## Case 4: get_bootloader_state() fails at the first time
        # Send beforehand the valid ACK message
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToResetBootProcess.value,
            "StatusCode": StatusCode.ReceivedAndInProcessing.value,
            "YesNoAnswer": YesNoAnswer.No.value,
        }
        self.send_test_ack_message(msg)

        # Mock the get_bootloader_state to have different behavior
        self.bl.get_bootloader_state = MagicMock()
        self.bl.get_bootloader_state.side_effect = [
            (None, None),
            (None, None),
            (None, None),
            (None, None),
            (None, None),
        ]
        self.assertFalse(self.bl.reset_bootloader(time_to_wait=0.1))

        ## Case 5: if the ack message is None
        self.bl.can.wait_can_ack_msg = MagicMock()
        self.bl.can.wait_can_ack_msg.return_value = None
        self.assertFalse(self.bl.reset_bootloader(time_to_wait=0.1))

    def test_run_app_on_bootloader(self, *args):
        """Function to test the function run_app_on_bootloader()."""
        ## Case 1: return True
        # Send beforehand an ACK message to indicate the reception of the the message
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToRunProgram.value,
            "StatusCode": StatusCode.ReceivedAndInProcessing.value,
            "YesNoAnswer": YesNoAnswer.No.value,
        }
        self.send_test_ack_message(msg)
        # Send beforehand an ACK message to show there is an valid program available on board
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToRunProgram.value,
            "StatusCode": StatusCode.ReceivedAndProcessed.value,
            "YesNoAnswer": YesNoAnswer.Yes.value,
        }
        self.send_test_ack_message(msg)
        # Send beforehand an message that should send by foxBMS application
        message = can.Message(arbitration_id=0x220, data=bytes(8))
        self.can_bus.send(message)
        self.assertTrue(self.bl.run_app_on_bootloader())

        # Case 2: if the received ack message contains a no
        # Send beforehand an ACK message to indicate the reception of the the message
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToRunProgram.value,
            "StatusCode": StatusCode.ReceivedAndInProcessing.value,
            "YesNoAnswer": YesNoAnswer.No.value,
        }
        self.send_test_ack_message(msg)
        # Send beforehand an ACK message to show there is an valid program available on board
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToRunProgram.value,
            "StatusCode": StatusCode.ReceivedAndProcessed.value,
            "YesNoAnswer": YesNoAnswer.No.value,
        }
        self.send_test_ack_message(msg)
        # Send beforehand an message that should send by foxBMS application
        message = can.Message(arbitration_id=0x220, data=bytes(8))
        self.can_bus.send(message)
        self.assertFalse(self.bl.run_app_on_bootloader())

        # Case 3: if no foxBMS message has been received
        # Send beforehand an ACK message to indicate the reception of the the message
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToRunProgram.value,
            "StatusCode": StatusCode.ReceivedAndInProcessing.value,
            "YesNoAnswer": YesNoAnswer.No.value,
        }
        self.send_test_ack_message(msg)
        # Send beforehand an ACK message to show there is an valid program available on board
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToRunProgram.value,
            "StatusCode": StatusCode.ReceivedAndProcessed.value,
            "YesNoAnswer": YesNoAnswer.Yes.value,
        }
        self.send_test_ack_message(msg)
        self.bl.can.wait_can_message = MagicMock()
        self.bl.can.wait_can_message.return_value = None
        self.assertFalse(self.bl.run_app_on_bootloader())

        # Case 2: if wait_can_message return None
        self.bl.can.wait_can_message = MagicMock()
        self.bl.can.wait_can_message.return_value = None
        self.assertFalse(self.bl.run_app_on_bootloader())

        # Case 3: if the first ack message is None
        self.bl.can.wait_can_ack_msg = MagicMock()
        self.bl.can.wait_can_ack_msg.return_value = None
        self.assertFalse(self.bl.run_app_on_bootloader())

        # Case 4: if the second ack message is None
        self.bl.can.wait_can_ack_msg = MagicMock()
        msg = {
            "AcknowledgeFlag": AcknowledgeFlag.Received.value,
            "AcknowledgeMessage": AcknowledgeMessage.ReceivedCmdToRunProgram.value,
            "StatusCode": StatusCode.ReceivedAndInProcessing.value,
            "YesNoAnswer": YesNoAnswer.No.value,
        }
        self.bl.can.wait_can_ack_msg.side_effect = [msg, None]
        self.assertFalse(self.bl.run_app_on_bootloader())

    def test_get_foxbms_state(self, *args):
        """Function to test the function get_foxbms_state()."""
        test_message = can.Message(arbitration_id=0x220, data=[0] * 8)
        self.can_bus.send(test_message)
        fsm_state = self.bl.get_foxbms_state()
        self.assertEqual("UNINITIALIZED", fsm_state)

    def test_get_bootloader_state(self, *args):
        """Function to test the function get_bootloader_state()."""
        msg = {
            "BootFsmState": BootFsmState.BootFsmStateWait.value,
            "CanFsmState": CanFsmState.CanFsmStateNoCommunication.value,
        }
        self.send_test_status_message(msg)
        can_fsm_state, boot_fsm_state = self.bl.get_bootloader_state()
        self.assertEqual(can_fsm_state, CanFsmState.CanFsmStateNoCommunication.name)
        self.assertEqual(boot_fsm_state, BootFsmState.BootFsmStateWait.name)

        # Case 2: if the ack message is None
        self.bl.can.wait_bootloader_state_msg = MagicMock()
        self.bl.can.wait_bootloader_state_msg.return_value = None
        can_fsm_state, boot_fsm_state = self.bl.get_bootloader_state()
        self.assertIsNone(can_fsm_state)
        self.assertIsNone(boot_fsm_state)

    def test_get_current_num_of_loops(self, *args):
        """Function to test the function get_current_num_of_loops()."""
        msg = {"CurrentLoopNumber": 1000}
        db_message = self.bl.can.db.get_message_by_name("f_DataTransferInfo")
        data = db_message.encode(msg)
        test_message = can.Message(arbitration_id=db_message.frame_id, data=data)
        self.can_bus.send(test_message)
        current_num_of_loops = self.bl.get_current_num_of_loops()
        self.assertEqual(current_num_of_loops, 1000)

        # Case 2: if the ack message is None
        self.bl.can.wait_data_transfer_info_msg = MagicMock()
        self.bl.can.wait_data_transfer_info_msg.return_value = None
        self.assertIsNone(self.bl.get_current_num_of_loops())

    @patch.object(BootloaderCanBasics, "wait_bootloader_version_info_msg")
    @patch.object(BootloaderCanBasics, "send_request_to_bootloader")
    def test_get_bootloader_version_num(
        self,
        mock_send_request_to_bootloader,
        mock_wait_bootloader_version_info_msg,
        *args,
    ):
        """Function to test the function get_bootloader_version_num()."""

        # Case 1: no version number has been received
        mock_send_request_to_bootloader.return_value = None
        mock_wait_bootloader_version_info_msg.return_value = None

        major_version_number, minor_version_number, patch_version_number = (
            self.bl.get_bootloader_version_num()
        )
        self.assertIsNone(major_version_number)
        self.assertIsNone(minor_version_number)
        self.assertIsNone(patch_version_number)

        # Case 2: version number has been successfully received
        mock_send_request_to_bootloader.return_value = None
        mock_wait_bootloader_version_info_msg.return_value = {
            "MajorVersionNumber": 6,
            "MinorVersionNumber": 2,
            "PatchVersionNumber": 4,
        }

        major_version_number, minor_version_number, patch_version_number = (
            self.bl.get_bootloader_version_num()
        )
        self.assertEqual(6, major_version_number)
        self.assertEqual(2, minor_version_number)
        self.assertEqual(4, patch_version_number)

    def send_test_ack_message(self, msg, *args):
        """Function to send a debug ack message."""
        db_message = self.bl.can.db.get_message_by_name("f_AcknowledgeMessage")
        data = db_message.encode(msg)
        test_message = can.Message(arbitration_id=db_message.frame_id, data=data)
        self.can_bus.send(test_message)

    def send_test_status_message(self, msg, *args):
        """Function to send a debug status message."""
        db_message = self.bl.can.db.get_message_by_name("f_BootloaderFsmStates")
        data = db_message.encode(msg)
        test_message = can.Message(arbitration_id=db_message.frame_id, data=data)
        self.can_bus.send(test_message)


if __name__ == "__main__":
    unittest.main()
