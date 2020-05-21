
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


UNPRINTABLE_ESCAPES = frozenset(list(range(1, 32)) + [0x7F])


class Generator:
    ASCII = set(range(1, 128))
    EXTENDED_ASCII = set(range(128, 256))

    ESC_DQUOTE = set([ord('"')])
    ESC_BSLASH = set([ord('\\')])
    ESC_CR = set([ord('\r')])
    ESC_LF = set([ord('\n')])
    ESC_TAB = set([ord('\t')])
    ESC_BS = set([ord('\b')])
    ESC_FF = set([ord('\f')])

    ESCAPED = ESC_DQUOTE | ESC_BSLASH | ESC_CR | ESC_LF | ESC_TAB | ESC_BS | ESC_FF

    STATES = (
        ("ASCII", ASCII - UNPRINTABLE_ESCAPES - ESCAPED),
        # ("EXTENDED_ASCII", EXTENDED_ASCII),
        ("COMMON_ESCAPES", ESCAPED),
        ("UNPRINTABLE_ESCAPES", UNPRINTABLE_ESCAPES - ESCAPED),
        ("NULL", set([0])),
    )

    def expand_states(self):
        for i in range(0, 128):
            for state_idx, state in enumerate(self.STATES):
                if i in state[1]:
                    yield state_idx

    def state_macro(self):
        for state_idx, state in enumerate(self.STATES):
            yield state[0], state_idx

    def generate(self, f):
        for macro_name, macro_value in self.state_macro():
            f.write("#define YAPIC_ENCODE_STATE_%s %s\n\n" % (macro_name, macro_value))

        f.write("static const unsigned char str_encode_table[128] = {\n\t")

        for i, state in enumerate(self.expand_states()):
            f.write("{0:<2}".format(state))

            if i != 127:
                f.write(",")

            if (i + 1) % 16 == 0:
                f.write("\n")
                if i != 127:
                    f.write("\t")

        f.write("};\n\n")


if __name__ == "__main__":
    from os import path
    with open(path.join("..", "src", "str_encode_table.h"), "w") as f:
        Generator().generate(f)
