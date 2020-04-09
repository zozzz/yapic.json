# -*- coding: utf-8 -*-

from os import path
from io import StringIO
from datetime import timezone, timedelta
from collections.abc import ItemsView
import math
import decimal
import codecs

import json as py_json
import simplejson
import ujson
import rapidjson
from yapic import json as yapic_json

from benchmark import Benchmark
from datetime import datetime, tzinfo, timedelta

# TODO: pass default method as argument only if it is required


class LatencyList(Benchmark):
    """ Latency: list """
    def get_encode_data(self):
        return list()


class LatencySet(Benchmark):
    """ Latency: set """

    DECODER = None

    def get_encode_data(self):
        return set()


class LatencyDict(Benchmark):
    """ Latency: dict """
    def get_encode_data(self):
        return dict()


class LatencyAsciiToAscii(Benchmark):
    """ Latency: ascii -> ascii """

    ENSURE_ASCII = True

    def get_encode_data(self):
        return "A"


class LatencyAsciiToUnicode(Benchmark):
    """ Latency: ascii -> unicode """

    ENSURE_ASCII = False

    def get_encode_data(self):
        return "A"


class LatencyUnicodeToUnicode(Benchmark):
    """ Latency: unicode -> unicode """

    ENSURE_ASCII = False

    def get_encode_data(self):
        return "सां"


class LatencyUnicodeToAscii(Benchmark):
    """ Latency: unicode -> ascii """

    ENSURE_ASCII = True

    def get_encode_data(self):
        return "सां"


class LatencyTrue(Benchmark):
    """ Latency: True """
    def get_encode_data(self):
        return True


class LatencyFalse(Benchmark):
    """ Latency: False """
    def get_encode_data(self):
        return False


class LatencyNone(Benchmark):
    """ Latency: None """
    def get_encode_data(self):
        return None


class LatencyLongMin(Benchmark):
    """ Latency: Long min """
    def get_encode_data(self):
        return 0


class LatencyLongMax(Benchmark):
    """ Latency: Long max """
    def get_encode_data(self):
        return 9223372036854775807


class LatencyFloat(Benchmark):
    """ Latency: Float to ascii """
    def get_encode_data(self):
        return 1.1


class LatencyFloatBig(Benchmark):
    """ Latency: Big float to ascii """
    def get_encode_data(self):
        return math.pi


class LatencyFloat2(Benchmark):
    """ Latency: Float to unicode """

    ENSURE_ASCII = False

    def get_encode_data(self):
        return 1.1


class LatencyDefaultFn(Benchmark):
    """ Latency: Default function """

    ENSURE_ASCII = False
    DECODER = False

    def __init__(self):
        encoders = list(self.ENCODER)
        self.ENCODER = []
        for lib, dumps in encoders:
            self.ENCODER.append((lib, self.__wrap(dumps)))

    class X:
        pass

    def get_encode_data(self):
        return self.X()

    def default(self, o):
        return "A"

    def __wrap(self, dumps):
        def wrapped(*args, **kwargs):
            kwargs["default"] = self.default
            dumps(*args, **kwargs)

        return wrapped


class LatencyDateTime(Benchmark):
    """ Latency: Datetime """

    DECODER = (("yapic", yapic_json.loads), ("rapidjson", rapidjson.loads))

    def get_encode_data(self):
        return datetime(2017, 4, 3, 21, 40, 12)

    def get_decode_data(self):
        return '"2017-04-03T21:40:12"'

    def default(self, o):
        return o.strftime("%Y-%m-%d %H:%M:%S")


class StringC1000AsciiToAscii(Benchmark):
    """ String: 1000 ASCII char """
    def get_encode_data(self):
        return "ABCDE" * 200


class StringC200ExtendedAsciiToAscii(Benchmark):
    """  String:1000 Extended ASCII char -> ASCII """
    def get_encode_data(self):
        return "ÁáÉéÍ" * 200


class StringC1000AsciiToUnicode(Benchmark):
    """ String: 1000 ASCII char -> UNICODE """

    ENSURE_ASCII = False

    def get_encode_data(self):
        return "ABCDE" * 200


class StringC200ExtendedAsciiToUnicode(Benchmark):
    """ String: 200 Extended ASCII char -> UNICODE """

    ENSURE_ASCII = False

    def get_encode_data(self):
        return "ÁŐ" * 100


class String2BUnicodeTextToAscii(Benchmark):
    """ String: 2B Unicode text -> Ascii """

    ITERATIONS = 100

    def get_encode_data(self):
        return "Език за програмиране е изкуствен език, предназначен за изразяване на изчисления, които могат да се извършат от машина, по-специално от компютър. Езиците за програмиране могат да се използват за създаване на програми, които контролират поведението на машина, да  реализират алгоритми точно или във вид на човешка комуникация." * 200


class String2BUnicodeTextToUnicode(String2BUnicodeTextToAscii):
    """ String: 2 byte Unicode text -> Unicode """

    ENSURE_ASCII = False


class String4BUnicodeTextToAscii(Benchmark):
    """ String: 4B Unicode text -> Ascii """

    ITERATIONS = 100

    def get_encode_data(self):
        return "𐌀𐌂𐌃𐌄𐌅𐌆𐌇𐌈𐌉𐌋𐌌𐌍𐌐𐌑𐌓𐌔𐌕𐌖𐌘𐌙𐌚" * 400


class String4BUnicodeTextToUnicode(String4BUnicodeTextToAscii):
    """ String: 4B Unicode text -> Unicode """

    ITERATIONS = 100
    ENSURE_ASCII = False


class StringMixedUnicodeTextToAscii(Benchmark):
    """ String: Mixed Unicode text -> Ascii """

    ITERATIONS = 100

    def get_encode_data(self):
        return ("𐌀𐌂𐌃 𐌄𐌅𐌆𐌇𐌈\n𐌉𐌋𐌌𐌍𐌐\"𐌑𐌓𐌔𐌕𐌖𐌘𐌙𐌚" + "ABCD EFGHIJ\t\t\nKLMNOP\nQRSTUV W XY Z" + "Език за програмиране е изк" +
                "Áí óéÉ\náÍÓ") * 200


class BytesC1000AsciiToAscii(Benchmark):
    """ Bytes: 1000 ASCII char """
    ENCODER = None
    def get_decode_data(self):
        return b'"' + ("ABCDE" * 200).encode("utf-8") + b'"'


class BytesC200ExtendedAsciiToAscii(Benchmark):
    """ Bytes: 1000 Extended ASCII char """
    ENCODER = None
    def get_decode_data(self):
        return b'"' + ("ÁáÉéÍ" * 200).encode("utf-8") + b'"'


class BytesC1000AsciiToUnicode(Benchmark):
    """ Bytes: 1000 ASCII char """
    ENCODER = None
    def get_decode_data(self):
        return b'"' + ("ABCDE" * 200).encode("utf-8") + b'"'


class BytesC200ExtendedAsciiToUnicode(Benchmark):
    """ Bytes: 200 Extended ASCII char """
    ENCODER = None
    def get_decode_data(self):
        return b'"' + ("ÁŐ" * 100).encode("utf-8") + b'"'


class Bytes2BUnicodeTextToAscii(Benchmark):
    """ Bytes: 2B Unicode text """
    ENCODER = None
    ITERATIONS = 100
    def get_decode_data(self):
        return b'"' + ("Език за програмиране е изкуствен език, предназначен за изразяване на изчисления, които могат да се извършат от машина, по-специално от компютър. Езиците за програмиране могат да се използват за създаване на програми, които контролират поведението на машина, да  реализират алгоритми точно или във вид на човешка комуникация." * 200).encode("utf-8") + b'"'


class Bytes4BUnicodeTextToAscii(Benchmark):
    """ Bytes: 4B Unicode text """
    ENCODER = None
    ITERATIONS = 100
    def get_decode_data(self):
        return b'"' + ("𐌀𐌂𐌃𐌄𐌅𐌆𐌇𐌈𐌉𐌋𐌌𐌍𐌐𐌑𐌓𐌔𐌕𐌖𐌘𐌙𐌚" * 400).encode("utf-8") + b'"'


class BytesMixedUnicodeTextToAscii(Benchmark):
    """ Bytes: Mixed Unicode text """
    ENCODER = None
    ITERATIONS = 100
    def get_decode_data(self):
        bytes = ((r"𐌀𐌂𐌃 𐌄𐌅𐌆𐌇𐌈\n𐌉𐌋𐌌𐌍𐌐\"𐌑𐌓𐌔𐌕𐌖𐌘𐌙𐌚ABCD EFGHIJ\t\t\nKLMNOP\nQRSTUV W XY ZЕзик за програмиране е изкÁí óéÉ\náÍÓ") * 200).encode("utf-8")
        return b'"' + bytes + b'"'


class ListOfInts(Benchmark):
    """ List of int values """

    ITERATIONS = 100

    def get_encode_data(self):
        return list(range(10000, 10200))


class ListOfFalse(Benchmark):
    """ List of false values """

    ITERATIONS = 100

    def get_encode_data(self):
        return [False for i in range(10000, 10300)]


class ListOfFloats(Benchmark):
    """ List of float values """

    ITERATIONS = 100

    def get_encode_data(self):
        return [i * math.pi for i in range(100000, 100300)]


class ListOfFloatsUnicode2B(Benchmark):
    """ List of float values (Unicode 2 byte) """

    ENCODER = None
    ITERATIONS = 100
    ENSURE_ASCII = False

    def get_encode_data(self):
        return [i * math.pi for i in range(100000, 100300)] + ["ő"]


class ListOfFloatsUnicode4B(Benchmark):
    """ List of float values (Unicode 4 byte) """

    ENCODER = None
    ITERATIONS = 100
    ENSURE_ASCII = False

    def get_encode_data(self):
        return [i * math.pi for i in range(100000, 100300)] + ["𐌌"]


class ListOfFloatsAsDecimal(Benchmark):
    """ List of float values to Decimal """

    ENCODER = None
    ITERATIONS = 100

    DECODER = (("yapic", lambda v: yapic_json.loads(v, parse_float=decimal.Decimal)),
               ("python", lambda v: py_json.loads(v, parse_float=decimal.Decimal)),
               ("rapidjson", lambda v: rapidjson.loads(v, use_decimal=True)))

    def get_encode_data(self):
        return [i * math.pi for i in range(100000, 100300)]


class ListOfFloatsNaN(Benchmark):
    """ List of NaN values """

    ITERATIONS = 100

    def get_encode_data(self):
        return [float("nan") for i in range(100000, 100300)]


class ListOfFloatsInfinity(Benchmark):
    """ List of Infinity values """

    ITERATIONS = 100

    def get_encode_data(self):
        return [float("infinity") for i in range(100000, 100300)]


class ListOfStringsAscii(Benchmark):
    """ List of ascii strings -> ascii"""

    ITERATIONS = 100

    def get_encode_data(self):
        res = []
        for x in range(256):
            res.append("ABCDE" * 40)
        return res


class ListOfStringsAsciiToUnicode(Benchmark):
    """ List of ascii strings -> unicode"""

    ITERATIONS = 100
    ENSURE_ASCII = False

    def get_encode_data(self):
        res = []
        for x in range(256):
            res.append("ABCDE" * 40)
        return res


class ListOfStrings2BUnicodeToAscii(Benchmark):
    """ List of 2 byte unicode strings -> ascii"""

    ITERATIONS = 100

    def get_encode_data(self):
        res = []
        for x in range(256):
            res.append("Език за програмиране е изкуствен език, предназначен" * 2)
        return res


class ListOfStrings2BUnicodeToUnicode(ListOfStrings2BUnicodeToAscii):
    """ List of 2 byte unicode strings -> unicode"""

    ENSURE_ASCII = False


class ListOfStrings4BUnicodeToUnicode(Benchmark):
    """ List of 4 byte unicode strings -> unicode"""

    ITERATIONS = 100
    ENSURE_ASCII = False

    def get_encode_data(self):
        res = []
        for x in range(256):
            res.append("𐌀𐌂𐌃𐌄𐌅𐌆𐌇𐌈𐌉𐌋𐌌𐌍𐌐𐌑𐌓𐌔𐌕𐌖𐌘𐌙𐌚" * 10)
        return res


class ListOfStringsMixed(Benchmark):
    """ List of mixed strings -> unicode"""

    ITERATIONS = 100
    ENSURE_ASCII = False

    def get_encode_data(self):
        res = []
        for x in range(256):
            res.append(
                "𐌀𐌂𐌃 𐌄𐌅𐌆𐌇𐌈\n𐌉𐌋𐌌𐌍𐌐\"𐌑𐌓𐌔𐌕𐌖𐌘𐌙𐌚ABCD EFGHIJ\t\t\nKLMNOP\nQRSTUV W XY ZЕзик за програмиране е изкÁí óéÉ\náÍÓ")
        return res


class ListOfStringsUnicodeEscape(Benchmark):
    """ List of unicode escapes """
    ENCODER = None
    ITERATIONS = 100
    ENSURE_ASCII = False

    def get_encode_data(self):
        res = []
        for x in range(256):
            res.append(
                py_json.dumps(
                    "𐌀𐌂𐌃 𐌄𐌅𐌆𐌇𐌈\n𐌉𐌋𐌌𐌍𐌐\"𐌑𐌓𐌔𐌕𐌖𐌘𐌙𐌚ABCD EFGHIJ\t\t\nKLMNOP\nQRSTUV W XY ZЕзик за програмиране е изкÁí óéÉ\náÍÓ",
                    ensure_ascii=True))
        return res


class ListOfBytesAscii(Benchmark):
    """ List of ascii bytes"""

    ENCODER = None
    ITERATIONS = 100

    def get_decode_data(self):
        res = []
        for x in range(256):
            res.append(b'"' + ("ABCDE" * 40).encode("utf-8") + b'"')
        return b"[" + b",".join(res) + b"]"


class ListOfBytes2BUnicodeToAscii(Benchmark):
    """ List of 2 byte unicode bytes"""

    ENCODER = None
    ITERATIONS = 100

    def get_decode_data(self):
        res = []
        for x in range(256):
            res.append(b'"' + ("Език за програмиране е изкуствен език, предназначен" * 2).encode("utf-8") + b'"')
        return b"[" + b",".join(res) + b"]"


class ListOfBytes4BUnicodeToUnicode(Benchmark):
    """ List of 4 byte unicode bytes"""

    ENCODER = None
    ITERATIONS = 100

    def get_decode_data(self):
        res = []
        for x in range(256):
            res.append(b'"' + ("𐌀𐌂𐌃𐌄𐌅𐌆𐌇𐌈𐌉𐌋𐌌𐌍𐌐𐌑𐌓𐌔𐌕𐌖𐌘𐌙𐌚" * 10).encode("utf-8") + b'"')
        return b"[" + b",".join(res) + b"]"


class ListOfBytesMixed(Benchmark):
    """ List of mixed bytes"""

    ENCODER = None
    ITERATIONS = 100

    def get_decode_data(self):
        res = []
        for x in range(256):
            data = r"𐌀𐌂𐌃 𐌄𐌅𐌆𐌇𐌈\n𐌉𐌋𐌌𐌍𐌐\"𐌑𐌓𐌔𐌕𐌖𐌘𐌙𐌚ABCD EFGHIJ\t\t\nKLMNOP\nQRSTUV W XY ZЕзик за програмиране е изкÁí óéÉ\náÍÓ".encode(
                "utf-8")
            res.append(b'"' + data + b'"')
        return b"[" + b",".join(res) + b"]"


class TupleOfInts(Benchmark):
    """ Tuple of int values """

    ITERATIONS = 100

    def get_encode_data(self):
        return tuple(range(10000, 10200))


class SetOfInts(Benchmark):
    """ Set of int values """

    ITERATIONS = 100
    DECODER = None

    def get_encode_data(self):
        return set(range(10000, 10200))

    def default(self, o):
        return list(o)


class MinMaxInt(Benchmark):
    """ Min & Max Int """
    def get_encode_data(self):
        return (-9223372036854775808, 9223372036854775807)


class ListOfDateTimeWithTZInfo(Benchmark):
    """ List of datetime with tzinfo """

    DECODER = LatencyDateTime.DECODER

    def get_encode_data(self):
        return [datetime(2017, 4, 3, 21, 40, 12, tzinfo=timezone(timedelta(seconds=7200))) for i in range(100)]

    def get_decode_data(self):
        return py_json.dumps(['"2017-04-03T21:40:12+02:00"' for i in range(100)], separators=(",", ":"))


class LargeDataToAscii(Benchmark):
    """ Large data -> Ascii """

    ITERATIONS = 100

    def get_encode_data(self):
        with codecs.open(path.join(path.dirname(__file__), "large-data.json"), "r", "utf-8") as f:
            return py_json.load(f)


class LargeDataBytes(Benchmark):
    """ Large data bytes """

    ENCODER = None
    ITERATIONS = 10

    def get_decode_data(self):
        with open(path.join(path.dirname(__file__), "large-data.json"), "rb") as f:
            return f.read()


class LargeDataToUnicode(LargeDataToAscii):
    """ Large data -> Unicode """

    ENSURE_ASCII = False


class LargeDataFormattedToAscii(LargeDataToAscii):
    """ Large formatted data -> Ascii """

    ENSURE_ASCII = True
    ENCODER = None

    def get_decode_data(self):
        return py_json.dumps(self.get_encode_data(), separators=(", ", ": "), indent=4)


class ToFile:
    ENCODER = (("yapic", yapic_json.dump), ("python", py_json.dump), ("simple", simplejson.dump), ("ujson", ujson.dump))

    DECODER = None

    def __init__(self):
        self.string_io = StringIO()
        encoders = list(self.ENCODER)
        self.ENCODER = []
        for lib, dump in encoders:
            self.ENCODER.append((lib, self.__wrap(dump)))

    def __wrap(self, dump):
        def wrapped(obj, **kwargs):
            dump(obj, self.string_io, **kwargs)
            self.string_io.truncate(0)
            self.string_io.seek(0)

        return wrapped


class LatencyFileAscii(ToFile, LatencyAsciiToAscii):
    """ LatencyFile: ascii -> ascii """


class LatencyFileUnicode(ToFile, LatencyAsciiToUnicode):
    """ LatencyFile: ascii -> unicode """


class LargeDataToFileAscii(ToFile, LargeDataToAscii):
    """ Large data -> file (Ascii) """


class LargeDataToFileUnicode(ToFile, LargeDataToUnicode):
    """ Large data -> file (Unicode) """


class MypyDataToAscii(Benchmark):
    """ Mypy data -> Ascii """

    ENSURE_ASCII = True
    ITERATIONS = 10

    def get_encode_data(self):
        with codecs.open(path.join(path.dirname(__file__), "builtins.data.json"), "r", "utf-8") as f:
            return py_json.load(f)


class ListViewBase(Benchmark):
    """ ListView dict """

    ENCODER = (("yapic", yapic_json.dumps), )
    DECODER = None
    ITERATIONS = 100

    def _get_dict(self):
        with codecs.open(path.join(path.dirname(__file__), "builtins.data.json"), "r", "utf-8") as f:
            return py_json.load(f)

    def get_encode_data(self):
        return self._get_dict()


class ListViewDictItems(ListViewBase):
    """ ListView dict.items() """
    class dict_items:
        def __init__(self, data):
            self.data = data

        def __json__(self):
            return self.data.items()

    def get_encode_data(self):
        return self.dict_items(self._get_dict())


class ListViewIterator(ListViewBase):
    """ ListView iterator """
    class lv_factory:
        def __init__(self, data):
            self.data = data

        def __json__(self):
            return ItemsView(self.data)

    def get_encode_data(self):
        return self.lv_factory(self._get_dict())


class ListViewDictCopy(ListViewBase):
    """ ListView similar with dict creation from iterable """
    class dict_factory:
        def __init__(self, data):
            self.data = data

        def __json__(self):
            return dict(self.data.items())

    def get_encode_data(self):
        return self.dict_factory(self._get_dict())


if __name__ == "__main__":
    import sys
    Benchmark.run_all(sys.argv[1:])
