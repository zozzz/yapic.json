from collections import namedtuple
from gen_str_state import UNPRINTABLE_ESCAPES, NORMAL_ESCAPE

Value = namedtuple("AsciiValue", ("type", "length", "chars"))


class StorageType:
    pass


class Pointer(StorageType):
    def get_type(self):
        return "char*"

    def get_value_max_length(self, max_chars):
        return 2 + max_chars * 4 + max_chars - 1 + 4

    # def get_value(self, value: Value):
    #     if value.length == 0:
    #         return "NULL"
    #     chars = ["0x%0.2X" % i for i in value.chars]
    #     for i in range(len(chars), 6):
    #         chars.append("0x00")
    #     return "{%s,%s,%s}" % (",".join(chars), value.type, value.length)

    def get_value(self, value: Value):
        if value.length == 0:
            return "NULL"
        chars = ["\\x%0.2X" % i for i in value.chars]
        for i in range(len(chars), 6):
            chars.append("\\x00")
        return "\"%s\\x%0.2X\\x%0.2X\"" % ("".join(chars), value.type, value.length)

    def get_macros(self, prefix):
        yield f"{prefix}_TYPE(value)", "((value)[6])"
        yield f"{prefix}_LENGTH(value)", "((value)[7])"
        yield f"{prefix}_CHR(value, idx)", "((value)[idx])"
        yield f"{prefix}_IS_NOT_NULL(value)", "((value) != NULL)"


class Int(StorageType):
    def get_type(self):
        return "uint64_t"

    def get_value_max_length(self, max_chars):
        return 16

    def get_value(self, value: Value):
        if value.length == 0:
            return "0x%0.16X" % 0
        else:
            v = value.type << 56
            v |= value.length << 48

            for i, ch in enumerate(value.chars):
                v |= ch << (i * 8)

            return "0x%0.16X" % v

    def get_macros(self, prefix):
        yield f"{prefix}_TYPE(value)", "((value) >> 56)"
        yield f"{prefix}_LENGTH(value)", "(((value) >> 48) & 0xFF)"
        yield f"{prefix}_CHR(value, idx)", "(((value) >> (idx * 8)) & 0xFF)"
        yield f"{prefix}_IS_NOT_NULL(value)", "((value) != 0)"


def HEX(x):
    return list("0123456789ABCDEF")[x]


class AsciiCacheTable:

    def __init__(self, name, type, width):
        self.name = name
        self.type = type
        self.width = width

    def write(self, file):
        for macro_name, macro_value in self.type.get_macros(self.name):
            file.write("#define %s %s\n\n" % (macro_name, macro_value))

        file.write("static const %s %s[] = {\n\t" % (self.type.get_type(), self.name))
        max_length = self.type.get_value_max_length(6) + 2
        variants = list(self.variants())
        last = len(variants) - 1

        for i, v in enumerate(variants):
            repr_ = self.type.get_value(v)
            file.write("{:<{width}}".format(repr_ + ("" if i == last else ","), width=max_length))
            if (i + 1) % self.width == 0 and i != last:
                file.write("\n\t")

        file.write("\n};\n")

    def variants(self):
        yield Value(0, 0, [0])
        for ch in range(1, 256):
            if ch in UNPRINTABLE_ESCAPES or ch > 127:
                chars = [ord("\\"), ord("u"), ord("0"), ord("0")]
                chars.append(ord(HEX((ch & 0xF0) >> 4)))
                chars.append(ord(HEX(ch & 0x0F)))
            elif ch in NORMAL_ESCAPE:
                chars = [ord("\\")]
                if ch == ord("\""):
                    chars.append(ord("\""))
                elif ch == ord("\\"):
                    chars.append(ord("\\"))
                elif ch == ord("\r"):
                    chars.append(ord("r"))
                elif ch == ord("\n"):
                    chars.append(ord("n"))
                elif ch == ord("\t"):
                    chars.append(ord("t"))
                elif ch == ord("\b"):
                    chars.append(ord("b"))
                elif ch == ord("\f"):
                    chars.append(ord("f"))
            else:
                chars = [ch]
            yield Value(1, len(chars), chars)


if __name__ == "__main__":
    import uuid
    import re
    from os import path

    with open(path.join("..", "src", "str_encode_ascii.h"), "w") as f:
        include_guard = re.sub("-", "_", str(uuid.uuid4()))
        f.write("/*** !!! AUTO GENERATED DO NOT EDIT !!! ***/\n\n")
        f.write("#ifndef A%s\n" % include_guard)
        f.write("#define A%s\n\n" % include_guard)

        f.write("\n")
        AsciiCacheTable("AsciiCache_Int", Int(), 4).write(f)
        f.write("\n")
        f.write("\n")
        AsciiCacheTable("AsciiCache_Ptr", Pointer(), 4).write(f)

        f.write("#endif /* A%s */\n" % include_guard)
