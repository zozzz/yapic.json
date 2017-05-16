
import re
from os import path

LICENSE = u"""
// Copyright (c) 2013, Zoltán Vetési
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Zoltán Vetési nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL ZOLTÁN VETÉSI BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""


class TableGen:
    @classmethod
    def generate(cls):
        include_guard = re.sub(r"\.", "_", cls.__filename__.upper())
        with open(path.join("..", "src", cls.__filename__), "w") as f:
            f.write(LICENSE.strip() + "\n\n")
            f.write("/*** !!! AUTO GENERATED DO NOT EDIT !!! ***/\n\n")
            f.write("#ifndef %s_\n" % include_guard)
            f.write("#define %s_\n\n" % include_guard)

            f.write("/** STATES **/\n\n")

            cls.write_states(f)

            f.write("\n")

            f.write("/** TABLE **/\n\n")

            cls.write_table(f)

            f.write("\n")

            f.write("/** UTF-8 utilities **/\n\n")

            f.write("#define UTF8_OCT2_MASK 0x1F\n")
            f.write("#define UTF8_OCT3_MASK 0x0F\n")
            f.write("#define UTF8_OCT4_MASK 0x07\n")
            f.write("#define UTF8_OCT_PART_MASK 0x3F\n\n")

            f.write("\n")

            f.write("/** Switch-Case **/\n\n")

            cls.write_cases(f)

            f.write("\n")

            f.write("#endif /* %s_ */\n" % include_guard)

    @classmethod
    def write_states(cls, f):
        items = []
        max_length = 0

        for key, idx, bytes in cls.itergroups():
            max_length = max(max_length, len(key))
            items.append((key, idx, bytes))

        rf = "#define {:<" + str(max_length + 2) + "}{:>2}\n"
        for key, idx, bytes in sorted(items, key=lambda i: i[1]):
            f.write(rf.format(key, idx))

    @classmethod
    def write_table(cls, f):
        bytes = range(0, 0xFF + 1)

        f.write("static unsigned char str_state_table[] = {\n\t")

        writed = 0
        bf = "{:>2}"
        for writed, byte in enumerate(bytes):
            if byte % 16 == 0 and byte != 0:
                f.write("\n\t")

            st_index = cls.get_index(byte)

            f.write(bf.format(st_index))
            if writed != 0xFF:
                f.write(", ")

        f.write("\n};\n")

    @classmethod
    def write_cases(cls, f):
        cases_per_line = 6

        for key, idx, bytes in cls.itergroups():
            f.write("#define %s_CASE \\\n\t" % key)
            for writed, byte in enumerate(sorted(bytes)):
                if writed != 0 and writed % cases_per_line == 0:
                    f.write(" \\\n\t")

                f.write(("case 0x%.02X" % byte) + (": " if writed + 1 != len(bytes) else ""))

            f.write("\n\n")

    @classmethod
    def write_hex_table(cls, f):
        bytes = range(0, 0xFF + 1)

        f.write("static const unsigned short hex_char_by_value[] = {\n\t")

        writed = 0
        bf = "{:>2}"
        for writed, byte in enumerate(bytes):
            if byte % 16 == 0 and byte != 0:
                f.write("\n\t")

            st_index = cls.get_index(byte)

            f.write(bf.format(st_index))
            if writed != 0xFF:
                f.write(", ")

        f.write("\n};\n")

    @classmethod
    def itergroups(cls):
        for key in cls.__dict__:
            if key.startswith("STR_"):
                if getattr(cls, key) is not None:
                    idx, bytes = getattr(cls, key)
                    yield key, idx, bytes

    @classmethod
    def get_index(cls, byte):
        indexes = []
        for key in cls.__dict__:
            if key.startswith("STR_"):
                if getattr(cls, key) is not None:
                    idx, bytes = getattr(cls, key)
                    if byte in bytes:
                        indexes.append(idx)

        if len(indexes) > 1 or len(indexes) == 0:
            raise ValueError("Something is wrong with this byte: %.02x" % byte)
        return indexes[0]


UNPRINTABLE_ESCAPES = frozenset([
    0x01,  # Start of Heading
    0x02,  # Start of Text
    0x03,  # End of Text
    0x04,  # End of Transmission
    0x05,  # Enquiry
    0x06,  # Acknowledge
    0x07,  # Bell
    0x0B,  # Vertical Tab
    0x0E,  # Shift Out
    0x0F,  # Shift In
    0x10,  # Data Link Escape
    0x11,  # Device Control 1
    0x12,  # Device Control 2
    0x13,  # Device Control 3
    0x14,  # Device Control 4
    0x15,  # Negative Acknowledge
    0x16,  # Synchronous idle
    0x17,  # End of Trans. block
    0x18,  # Cancel
    0x19,  # End of Medium
    0x1A,  # Substitutie
    0x1B,  # Escape
    0x1C,  # File Separator
    0x1D,  # Group Separator
    0x1E,  # Record Separator
    0x1F,  # Unit Separator
    # 0x7F  # DEL
])

NORMAL_ESCAPE = frozenset([
    0x00,
    ord('"'),
    ord('\\'),
    ord('\r'),
    ord('\n'),
    ord('\t'),
    ord('\b'),
    ord('\f'),
    0x7F
])


class EncoderGen(TableGen):
    __filename__ = "str_encode_table.h"

    STR_ASCII = (1, set(range(1, 0x80)) - UNPRINTABLE_ESCAPES - NORMAL_ESCAPE)

    STR_UTF8_START_OCT_2 = (2, set(range(0xC0, 0xE0)))
    STR_UTF8_START_OCT_3 = (3, set(range(0xE0, 0xF0)))
    STR_UTF8_START_OCT_4 = (4, set(range(0xF0, 0xF5)))
    STR_UTF8_PART = (15, set(range(0x80, 0xC0)))

    STR_ESC_DQUOTE = (5, set([ord('"')]))
    STR_ESC_BSLASH = (6, set([ord('\\')]))
    STR_ESC_CR = (7, set([ord('\r')]))
    STR_ESC_LF = (8, set([ord('\n')]))
    STR_ESC_TAB = (9, set([ord('\t')]))
    STR_ESC_BS = (10, set([ord('\b')]))
    STR_ESC_FF = (11, set([ord('\f')]))
    STR_ESC_DEL = (0x7F, set([0x7F]))

    STR_UNPRINTABLE_ESC = (12, UNPRINTABLE_ESCAPES)

    STR_NULL = (13, set([0]))

    STR_UTF8_INVALID = (14, set(range(0xF5, 0xFF + 1)))


SKIP_BYTES_WHEN_DECODE = frozenset([
    ord("\""),
    ord("\0"),
    ord("\\")
])


class DecoderGen(TableGen):
    __filename__ = "str_decode_table.h"

    STR_ASCII = (1, set(range(1, 0x80)) - SKIP_BYTES_WHEN_DECODE)

    STR_UTF8_START_OCT_2 = (2, set(range(0xC0, 0xE0)))
    STR_UTF8_START_OCT_3 = (3, set(range(0xE0, 0xF0)))
    STR_UTF8_START_OCT_4 = (4, set(range(0xF0, 0xF5)))
    STR_UTF8_PART = (9, set(range(0x80, 0xC0)))

    STR_QUOTE = (5, set([ord("\"")]))
    STR_ESC_START = (6, set([ord("\\")]))

    STR_NULL = (7, set([0]))

    STR_UTF8_INVALID = (8, set(range(0xF5, 0xFF + 1)))


"""
char_range = set(range(0, 0xFF+1))

for k in EncoderGen.__dict__:
    if k.startswith("STR_"):
        idx, chars = getattr(EncoderGen, k)
        char_range -= chars
"""

EncoderGen.generate()
DecoderGen.generate()
