import pytest
from yapic import json as yapic_json
import json as py_json

CASES = [
    (0, "0"),
    (1, "1"),
    (-1, "-1"),
    (9223372036854775807, "9223372036854775807"),
    (-9223372036854775808, "-9223372036854775808"),
] + [(i, py_json.dumps(i)) for i in range(-100000, 100000, 123)]


@pytest.mark.parametrize("value,expected", CASES)
def test_long_encode(value, ensure_ascii, expected):
    assert yapic_json.dumps(value, ensure_ascii=ensure_ascii) == expected
    assert yapic_json.dumpb(value, ensure_ascii=ensure_ascii) == expected.encode("utf-8")


def test_long_encode_overflow(ensure_ascii):
    with pytest.raises(yapic_json.JsonEncodeError) as ex:
        yapic_json.dumps(9223372036854775808, ensure_ascii=ensure_ascii)
    ex.match("Python int too large to convert to C long.")


@pytest.mark.parametrize("expected,value", CASES)
def test_long_decode(value, expected):
    assert yapic_json.loads(value) == expected


def test_long_decode_invalid1():
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads('1b')
    ex.match("Found junk data after valid JSON data at position: 1.")


def test_long_decode_invalid2():
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads('01')
    ex.match("Found junk data after valid JSON data at position: 1.")


@pytest.mark.parametrize("value", [
    "11111111111111111111111111111111111111111111111111111111111111111",
    "9223372036854775808",
    "9223372036854775809",
    "9223372036854775810",
    "-9223372036854775809",
    "-9223372036854775810",
])
def test_long_decode_big(value):
    assert yapic_json.loads(value) == float(value)
