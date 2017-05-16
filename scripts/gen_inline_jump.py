
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


class InlineJumpTable:
    # ASCII = set(range(1, 128))

    ESC_DQUOTE = set([ord('"')])
    ESC_BSLASH = set([ord('\\')])
    ESC_CR = set([ord('\r')])
    ESC_LF = set([ord('\n')])
    ESC_TAB = set([ord('\t')])
    ESC_BS = set([ord('\b')])
    ESC_FF = set([ord('\f')])
    ESC_DEL = set([0x7F])

    # NORMAL_ESCAPE = ESC_DQUOTE | ESC_BSLASH | ESC_CR | ESC_LF | ESC_TAB | ESC_BS | ESC_FF

    CASES = (
        ({0}, "EncoderJT_ON_NULL();"),
        (UNPRINTABLE_ESCAPES, "EncoderJT_ON_UNPRINTABLE();"),
        (ESC_DQUOTE, "EncoderJT_AppendChar('\\\\'); EncoderJT_AppendChar('\"');"),
        (ESC_BSLASH, "EncoderJT_AppendChar('\\\\'); EncoderJT_AppendChar('\\\\');"),
        (ESC_CR, "EncoderJT_AppendChar('\\\\'); EncoderJT_AppendChar('r');"),
        (ESC_LF, "EncoderJT_AppendChar('\\\\'); EncoderJT_AppendChar('n');"),
        (ESC_TAB, "EncoderJT_AppendChar('\\\\'); EncoderJT_AppendChar('t');"),
        (ESC_BS, "EncoderJT_AppendChar('\\\\'); EncoderJT_AppendChar('b');"),
        (ESC_FF, "EncoderJT_AppendChar('\\\\'); EncoderJT_AppendChar('f');"),
        (ESC_DEL, "EncoderJT_ON_DEL();"),

        (set(range(0, 128)), "EncoderJT_ON_ASCII();"),
    )

    def generate(self, f):
        prev_case = 0
        for i in range(0, 128):
            current = 0
            for ck, cv in enumerate(self.CASES):
                if i in cv[0]:
                    current = ck
                    break

            if current != prev_case:
                f.write("\t%s\nbreak;\n" % self.CASES[prev_case][1])
                prev_case = current

            f.write("case 0x%.2X:\n" % i)

        f.write("\t%s\nbreak;\n" % self.CASES[prev_case][1])


if __name__ == "__main__":
    from os import path
    with open(path.join("..", "src", "str_encode_jump.h"), "w") as f:
        InlineJumpTable().generate(f)
