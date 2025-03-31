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

"""Filters CAN messages"""


class CANFilter:
    """Implements the CAN message filter functionalities

    :param _ids: CAN ids which should be filtered
    :param _id_pos: Position of the CAN ids in the log file
    :param _sampling: Number of specific CAN messages that
        should be stored
    """

    def __init__(
        self, ids: list[str], id_pos: int, sampling: dict[str, int] | None = None
    ) -> None:
        self._ids = self.extend_ids(ids)
        self._id_pos = id_pos
        self._sampling = sampling
        if sampling:
            self._occurrence = {x: 0 for x in sampling}

    def filter_msg(self, msg: str) -> str | None:
        """Filters the incoming CAN message with
        respect to the _ids, _id_pos and _sampling.
        This function handles only CAN messages
        with whitespace separated parts.

        :param msg: One CAN messages as string
        :return: One CAN message that fulfills the
            filter conditions
        """
        try:
            msg_parts = [x for x in msg.lstrip().split() if x]
            msg_id = msg_parts[self._id_pos]
            if msg_id not in self._ids:
                return None
            if self._sampling and msg_id in self._sampling:
                self._occurrence[msg_id] += 1
                if self._occurrence[msg_id] % self._sampling[msg_id] == 0:
                    self._occurrence[msg_id] = 0
                    return msg
                return None
            return msg
        except IndexError:
            return None

    @staticmethod
    def extend_ids(ids: list[str]) -> list[str]:
        """Extends the list with CAN ids with
        respect to the used abbreviations as 201-20F.

        This function only accepts only ids in
        hexadecimal.

        :param ids: List with CAN ids
        :return: Extended list with CAN ids

        Note:
        """
        copy_ids = ids.copy()
        for can_id in ids:
            if "-" in can_id:
                id_split = can_id.split("-")
                start = int(id_split[0], 16)
                end = int(id_split[1], 16)
                copy_ids.remove(can_id)
                for i in range(start, end + 1):
                    # hex(i)[2:] removes 0x
                    new_id = hex(i)[2:].upper()
                    if new_id not in copy_ids:
                        copy_ids.append(new_id)
        return copy_ids
