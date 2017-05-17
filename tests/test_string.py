import pytest

import json as py_json
from zibo import json as zibo_json

# TODO: test surrogate pairs


@pytest.fixture(
    params=[
        range(0, 128),
        range(128, 256),
        range(256, 2048),
        range(2048, 65536),
        range(65536, 1114111)
    ],
    ids=[
        "Ascii chars",
        "Extended Ascii chars",
        "UTF8 2 byte chars",
        "UTF8 3 byte chars",
        "UTF8 4 byte chars"
    ]
)
def unicode_chars(request):
    return request.param


def test_encode_chars(unicode_chars, ensure_ascii):
    for chc in unicode_chars:
        ch = chr(chc)
        assert zibo_json.dumps(ch, ensure_ascii=ensure_ascii) == py_json.dumps(ch, ensure_ascii=ensure_ascii), chc


def test_decode_chars(unicode_chars, ensure_ascii):
    for chc in unicode_chars:
        ch = chr(chc)
        json_ch = py_json.dumps(ch, ensure_ascii=ensure_ascii)
        if ensure_ascii and chc >= 0xD800 and chc <= 0xDFFF:  # invalid unicode escape
            with pytest.raises(zibo_json.JsonDecodeError):
                zibo_json.loads(json_ch)
        else:
            assert zibo_json.loads(json_ch) == py_json.loads(json_ch), json_ch


@pytest.mark.parametrize("value", [
    ("A" * (64 * 100),),
    ("Árvíztűrő tükörfúrógép",),
    ("половину",),
    ("половинуÁ𐌐𐌑𐌓 \r\nFsdf áésfak,sd opőfiunü39q35r78égp-vbfynkjsa.géélfhgpqa97gi3ztö" * 10,),
    ("\r\n\t\b\f\\\"",)
])
def test_encode_string(value, ensure_ascii):
    zv = zibo_json.dumps(value, ensure_ascii=ensure_ascii)
    pv = py_json.dumps(value, ensure_ascii=ensure_ascii)
    assert len(zv) == len(pv)
    assert zv == pv


@pytest.mark.parametrize("value", [
    "Hello World",
    "\r\n\t\b\f\\\"",
    "Árvíztűrő tükörfúrógép",
    "𐌀𐌂𐌃𐌄𐌅𐌆𐌇𐌈𐌉𐌋𐌌𐌍𐌐𐌑𐌓𐌔𐌕𐌖𐌘𐌙𐌚" * 400,
    "половинуÁ𐌐𐌑𐌓 \r\nFsdf áésfak,sd opőfiunü39q35r78égp-vbfynkjsa.géélfhgpqa97gi3ztö" * 10
])
def test_decode_string(value, ensure_ascii):
    expected = value
    value = py_json.dumps(value, ensure_ascii=True)
    assert zibo_json.loads(value) == expected


def test_decode_unterminated():
    with pytest.raises(zibo_json.JsonDecodeError) as ex:
        zibo_json.loads('"Hello')
    ex.match("Unexpected end of data at position: 6.")


def test_decode_unterminated2():
    with pytest.raises(zibo_json.JsonDecodeError) as ex:
        zibo_json.loads('"\\')
    ex.match("Unexpected end of data at position: 2.")


def test_decode_unterminated3():
    with pytest.raises(zibo_json.JsonDecodeError) as ex:
        zibo_json.loads('"\\u')
    ex.match("Unexpected end of data at position: 3.")


def test_decode_unterminated4():
    with pytest.raises(zibo_json.JsonDecodeError) as ex:
        zibo_json.loads('"\\u0')
    ex.match("Unexpected end of data at position: 4.")


def test_decode_unterminated5():
    with pytest.raises(zibo_json.JsonDecodeError) as ex:
        zibo_json.loads('"\\u00')
    ex.match("Unexpected end of data at position: 5.")


def test_decode_unterminated6():
    with pytest.raises(zibo_json.JsonDecodeError) as ex:
        zibo_json.loads('"\\u000')
    ex.match("Unexpected end of data at position: 6.")


def test_decode_unterminated7():
    with pytest.raises(zibo_json.JsonDecodeError) as ex:
        zibo_json.loads('"\\u0000')
    ex.match("Unexpected end of data at position: 7.")


def test_decode_unterminated8():
    with pytest.raises(zibo_json.JsonDecodeError) as ex:
        zibo_json.loads('"\\u00000')
    ex.match("Unexpected end of data at position: 8.")


def test_decode_invalid_escape():
    with pytest.raises(zibo_json.JsonDecodeError) as ex:
        zibo_json.loads('"\\g')
    ex.match("Invalid escaped character while decoding 'string' at position: 2.")
