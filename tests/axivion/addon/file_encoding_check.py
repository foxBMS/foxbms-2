# Copyright (c) 2022, Axivion GmbH
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


import fnmatch
import typing
from enum import Enum, auto
from pathlib import Path

from bauhaus import analysis, ir
from bauhaus.analysis import config
from bauhaus.ir.common.output import unparse

from . import iisb_base


class EncodingEnum(Enum):
    ascii = auto()
    big5 = auto()
    big5hkscs = auto()
    cp037 = auto()
    cp273 = auto()
    cp424 = auto()
    cp437 = auto()
    cp500 = auto()
    cp720 = auto()
    cp737 = auto()
    cp775 = auto()
    cp850 = auto()
    cp852 = auto()
    cp855 = auto()
    cp856 = auto()
    cp857 = auto()
    cp858 = auto()
    cp860 = auto()
    cp861 = auto()
    cp862 = auto()
    cp863 = auto()
    cp864 = auto()
    cp865 = auto()
    cp866 = auto()
    cp869 = auto()
    cp874 = auto()
    cp875 = auto()
    cp932 = auto()
    cp949 = auto()
    cp950 = auto()
    cp1006 = auto()
    cp1026 = auto()
    cp1125 = auto()
    cp1140 = auto()
    cp1250 = auto()
    cp1251 = auto()
    cp1252 = auto()
    cp1253 = auto()
    cp1254 = auto()
    cp1255 = auto()
    cp1256 = auto()
    cp1257 = auto()
    cp1258 = auto()
    euc_jp = auto()
    euc_jis_2004 = auto()
    euc_jisx0213 = auto()
    euc_kr = auto()
    gb2312 = auto()
    gbk = auto()
    gb18030 = auto()
    hz = auto()
    iso2022_jp = auto()
    iso2022_jp_1 = auto()
    iso2022_jp_2 = auto()
    iso2022_jp_2004 = auto()
    iso2022_jp_3 = auto()
    iso2022_jp_ext = auto()
    iso2022_kr = auto()
    latin_1 = auto()
    iso8859_2 = auto()
    iso8859_3 = auto()
    iso8859_4 = auto()
    iso8859_5 = auto()
    iso8859_6 = auto()
    iso8859_7 = auto()
    iso8859_8 = auto()
    iso8859_9 = auto()
    iso8859_10 = auto()
    iso8859_11 = auto()
    iso8859_13 = auto()
    iso8859_14 = auto()
    iso8859_15 = auto()
    iso8859_16 = auto()
    johab = auto()
    koi8_r = auto()
    koi8_t = auto()
    koi8_u = auto()
    kz1048 = auto()
    mac_cyrillic = auto()
    mac_greek = auto()
    mac_iceland = auto()
    mac_latin2 = auto()
    mac_roman = auto()
    mac_turkish = auto()
    ptcp154 = auto()
    shift_jis = auto()
    shift_jis_2004 = auto()
    shift_jisx0213 = auto()
    utf_32 = auto()
    utf_32_be = auto()
    utf_32_le = auto()
    utf_16 = auto()
    utf_16_be = auto()
    utf_16_le = auto()
    utf_7 = auto()
    utf_8 = auto()
    utf_8_sig = auto()


@analysis.rule("IISB-FileEncodingCheck")
class IisbEncodingRule(iisb_base.IISBRule, analysis.AnalysisRule):
    title = """Force Source Files to be decodable with specific encoding."""

    _rule_description = """
        <p>All user include and primary files in the project shall be encoded
        with a defined encoding (see <code>general_encoding</code>). Exceptions
        can be defined in <code>alternative_encoding</code></p>
    """

    _message_descriptions = {
        "not_found": "File does not exist, encoding could not be verified",
        "could_not_open": "File could not be read, encoding could not be verified due to: {}",
        "wrong_encoding": "File does not have required encoding {}",
    }

    general_encoding: typing.Union[EncodingEnum, str] = EncodingEnum.utf_8
    """Define a default encoding for all source code files, not mentioned
    in <code>alternative_encoding</code>."""

    alternative_encoding: typing.Dict[
        config.FileGlobPattern, typing.Union[EncodingEnum, str]
    ] = dict()
    """Define an alternative encoding for certain files."""

    def execute(self, ir_graph: ir.Graph):
        ir_base_path = Path(ir_graph.Basepath)
        for file in ir_graph.nodes_of_type(
            ir.Physical, ("User_Include_File", "Primary_File")
        ):
            encoding = self.general_encoding
            for pat, enc_str in self.alternative_encoding.items():
                if fnmatch.fnmatch(name=file.Full_Name, pat=pat):
                    encoding = enc_str
            encoding_str = (
                encoding.name if isinstance(encoding, EncodingEnum) else encoding
            )
            if not encoding_str:
                continue

            path = Path(file.Full_Name)
            if not path.exists() and not path.is_absolute():
                # this is necessary if the cwd is not the project root directory
                # (e.g. when run by perform_tests)
                path = ir_base_path / path
            if not path.exists():
                self.add_message(
                    primary_sloc=file,
                    msg_key="not_found",
                    entity=unparse.entity(file),
                )
                continue

            try:
                with open(
                    path, encoding=encoding_str, errors="strict", mode="rt"
                ) as fd:
                    for _line in fd:
                        pass
            except OSError as e:
                self.add_message(
                    primary_sloc=file,
                    msg_key="could_not_open",
                    message_arguments=(str(e),),
                    entity=unparse.entity(file),
                )
            except ValueError as _:
                self.add_message(
                    primary_sloc=file,
                    msg_key="wrong_encoding",
                    message_arguments=(encoding_str,),
                    entity=unparse.entity(file),
                )
