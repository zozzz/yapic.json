import json as py_json
import re

import pytest
from yapic import json as yapic_json

# TODO: test surrogate pairs


@pytest.fixture(
    params=[
        range(0, 128),
        range(128, 256),
        range(256, 2048),
        range(2048, 65536),
        range(65536, 1114111),
    ],
    ids=[
        "Ascii chars",
        "Extended Ascii chars",
        "UTF8 2 byte chars",
        "UTF8 3 byte chars",
        "UTF8 4 byte chars",
    ],
)
def unicode_chars(request):
    return request.param


def test_encode_chars(unicode_chars, ensure_ascii):
    for chc in unicode_chars:
        ch = chr(chc)
        assert yapic_json.dumps(ch, ensure_ascii=ensure_ascii) == py_json.dumps(ch, ensure_ascii=ensure_ascii), chc

        if not (chc >= 0xD800 and chc <= 0xDFFF):
            assert yapic_json.dumpb(ch, ensure_ascii=ensure_ascii) == py_json.dumps(
                ch, ensure_ascii=ensure_ascii
            ).encode("utf-8"), chc


def test_decode_chars(unicode_chars, ensure_ascii):
    for chc in unicode_chars:
        ch = chr(chc)
        json_ch = py_json.dumps(ch, ensure_ascii=ensure_ascii)
        if ensure_ascii and chc >= 0xD800 and chc <= 0xDFFF:  # invalid unicode escape
            with pytest.raises(yapic_json.JsonDecodeError):
                yapic_json.loads(json_ch)

            with pytest.raises(yapic_json.JsonDecodeError):
                yapic_json.loads(json_ch.encode("utf-8"))
        else:
            assert yapic_json.loads(json_ch) == py_json.loads(json_ch), json_ch

            if chc >= 0xD800 and chc <= 0xDFFF:
                bytes_ch = json_ch.encode("utf-8", errors="surrogatepass")
                with pytest.raises(yapic_json.JsonDecodeError) as ex:
                    yapic_json.loads(bytes_ch)
                ex.match(re.escape("Invalid UTF-8 character: line 1 column 2 (char 1)"))
            else:
                bytes_ch = json_ch.encode("utf-8")
                assert yapic_json.loads(bytes_ch) == py_json.loads(json_ch), bytes_ch


@pytest.mark.parametrize(
    "value",
    [
        "A" * (64 * 100),
        "Árvíztűrő tükörfúrógép",
        "половину",
        "половинуÁ𐌐𐌑𐌓 \r\nFsdf áésfak,sd opőfiunü39q35r78égp-vbfynkjsa.géélfhgpqa97gi3ztö" * 10,
        '\r\n\t\b\f\\"',
    ],
    ids=[
        "Long ASCII (64000 char)",
        "Árvíztűrő tükörfúrógép",
        "Short UTF-8",
        "Long UTF-8",
        "Escaped chars",
    ],
)
def test_encode_string(value, ensure_ascii):
    zv = yapic_json.dumps(value, ensure_ascii=ensure_ascii)
    pv = py_json.dumps(value, ensure_ascii=ensure_ascii)
    assert len(zv) == len(pv)
    assert zv == pv

    yapic_bytes = yapic_json.dumpb(value, ensure_ascii=ensure_ascii)
    python_bytes = py_json.dumps(value, ensure_ascii=ensure_ascii).encode("utf-8")
    assert len(yapic_bytes) == len(python_bytes)
    assert yapic_bytes == python_bytes


@pytest.mark.parametrize(
    "value",
    [
        "",
        "Hello World",
        "ASCII-ŲŢƑ8",
        '\r\n\t\b\f\\"',
        "Árvíztűrő tükörfúrógép",
        "𐌀𐌂𐌃𐌄𐌅𐌆𐌇𐌈𐌉𐌋𐌌𐌍𐌐𐌑𐌓𐌔𐌕𐌖𐌘𐌙𐌚" * 400,
        "половинуÁ𐌐𐌑𐌓 \r\nFsdf áésfak,sd opőfiunü39q35r78égp-vbfynkjsa.géélfhgpqa97gi3ztö" * 10,
        "Език за програмиране е изкуствен език, предназначен за изразяване на изчисления, които могат да се извършат от машина, по-специално от компютър. Езиците за програмиране могат да се използват за създаване на програми, които контролират поведението на машина, да  реализират алгоритми точно или във вид на човешка комуникация."
        * 200,
    ],
    ids=[
        "Empty string",
        "Hello World",
        "ASCII-UTF8",
        "Escaped chars",
        "Árvíztűrő tükörfúrógép",
        "Long UTF-8 v1",
        "Long UTF-8 v2",
        "Long UTF-8 v3",
    ],
)
def test_decode_string(value, ensure_ascii):
    expected = value
    value = py_json.dumps(value, ensure_ascii=True)
    assert yapic_json.loads(value) == expected

    bytes_value = value.encode("utf-8")
    assert yapic_json.loads(bytes_value) == py_json.loads(value)

    bytearray_value = bytearray(value, "utf-8")
    assert yapic_json.loads(bytearray_value) == yapic_json.loads(value)


def test_decode_invalid_input():
    with pytest.raises(TypeError) as ex:
        yapic_json.loads({})
    ex.match("argument 1 must be str or bytes")


def test_decode_unterminated(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type('"Hello'))
    ex.match(re.escape("Unexpected end of data: line 1 column 7 (char 6)"))


def test_decode_unterminated2(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type('"\\'))
    ex.match(re.escape("Unexpected end of data: line 1 column 3 (char 2)"))


def test_decode_unterminated3(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type('"\\u'))
    ex.match(re.escape("Unexpected end of data: line 1 column 4 (char 3)"))


def test_decode_unterminated4(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type('"\\u0'))
    ex.match(re.escape("Unexpected end of data: line 1 column 5 (char 4)"))


def test_decode_unterminated5(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type('"\\u00'))
    ex.match(re.escape("Unexpected end of data: line 1 column 6 (char 5)"))


def test_decode_unterminated6(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type('"\\u000'))
    ex.match(re.escape("Unexpected end of data: line 1 column 7 (char 6)"))


def test_decode_unterminated7(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type('"\\u0000'))
    ex.match(re.escape("Unexpected end of data: line 1 column 8 (char 7)"))


def test_decode_unterminated8(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type('"\\u00000'))
    ex.match(re.escape("Unexpected end of data: line 1 column 9 (char 8)"))


def test_decode_invalid_escape(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type('"\\g'))
    ex.match(re.escape("Invalid escaped character while decoding 'string': line 1 column 3 (char 2)"))


@pytest.mark.parametrize(
    "value",
    [
        b"\x81",
        b"\xC1",
        b"\xE1",
        b"\xE1\x81",
        b"\xF1",
        b"\xF1\x81",
        b"\xF1\x81\x82",
    ],
    ids=[
        "Invalid: start with continuation",
        "Invalid: incomplete seq (1 / 2)",
        "Invalid: incomplete seq (1 / 3)",
        "Invalid: incomplete seq (2 / 3)",
        "Invalid: incomplete seq (1 / 4)",
        "Invalid: incomplete seq (2 / 4)",
        "Invalid: incomplete seq (3 / 4)",
    ],
)
def test_decode_invalid_utf8(value):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(b'"' + value + b'"')
    ex.match("Invalid UTF-8 character")
