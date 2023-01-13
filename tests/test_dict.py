import json as py_json
import re
from collections.abc import ItemsView
from decimal import Decimal
from uuid import UUID

import pytest
from yapic import json as yapic_json


class Int:
    def __init__(self, value):
        self.value = value

    def __json__(self):
        return self.value


def default(o):
    return o.value


# yapf: disable
large_dict = {
    "first_name": "Vetési",
    "last_name": "Zoltán",
    "email": "vetesi.zoltan@gmail.com",
    "yapic": {
        "json": {
            "dumps": ["obj", {"encode_ascii": True}, {"default": None}],
            "dump": ["obj", "file", {"encode_ascii": True}, {"default": None}]
        },
        "ntimes_faster": 2,
        "another": 3242153,
        "__json__": Int(10),
        "__json__2": Int(20)
    },
    "unicode": "половину",
    "2": ["Под", "aÁд"]
}


@pytest.mark.parametrize("value,expected", [
    ({}, "{}"),
    ({"Hello": 42}, '{"Hello":42}'),
    ({"1": 1}, '{"1":1}'),
    ({"Под водом проводе скоро половину свог живота": "половину"}, py_json.dumps),
    ({True: True}, '{"true":true}'),
    ({False: False}, '{"false":false}'),
    ({1: 1}, '{"1":1}'),
    ({100: 100}, '{"100":100}'),
    ({3.14: 3.14}, '{"3.14":3.14}'),
    ({UUID("f4aa36f7-254b-472d-9dc6-e030f244054a"): 1}, '{"f4aa36f7-254b-472d-9dc6-e030f244054a":1}'),
    ({Decimal("4.2"): "OK"}, '{"4.2":"OK"}'),
    # TODO:
    # ({datetime(2000, 1, 1, 12, 34, 56): "OK"}, '{"2000-01-01 12:34:56":"OK"}'),
    # ({date(2000, 1, 1): "OK"}, '{"2000-01-01":"OK"}'),
    # ({time(12, 34, 56): "OK"}, '{"12:34:56":"OK"}'),
    (large_dict, py_json.dumps),
])
def test_dict_encode(value, expected, ensure_ascii):
    if expected is py_json.dumps:
        expected = py_json.dumps(value, ensure_ascii=ensure_ascii, separators=(",", ":"), default=default)
    assert yapic_json.dumps(value, ensure_ascii=ensure_ascii) == expected
    assert yapic_json.dumps(value.items(), ensure_ascii=ensure_ascii) == expected
    assert yapic_json.dumps(ItemsView(value), ensure_ascii=ensure_ascii) == expected

    assert yapic_json.dumpb(value, ensure_ascii=ensure_ascii) == expected.encode("utf-8")
    assert yapic_json.dumpb(value.items(), ensure_ascii=ensure_ascii) == expected.encode("utf-8")
    assert yapic_json.dumpb(ItemsView(value), ensure_ascii=ensure_ascii) == expected.encode("utf-8")
# yapf: enable


def test_dict_same_keys_encode(ensure_ascii):
    data = [{"key": 1}, {"key": 2}, {"key": 3}, {"key": 4}, {"key": 5}]

    assert yapic_json.dumps(data, ensure_ascii=ensure_ascii) == py_json.dumps(
        data, ensure_ascii=ensure_ascii, separators=(",", ":")
    )


def test_dict_recursive(ensure_ascii):
    d = dict(recursive=True)
    d["self"] = d

    with pytest.raises(yapic_json.JsonEncodeError) as ex:
        yapic_json.dumps(d, ensure_ascii=ensure_ascii)

    ex.match("Maximum recursion level reached, while encoding dict entry .*? at 'self' key.")


@pytest.mark.parametrize(
    "value",
    [
        "{}",
        '{"Hello World": "Hello World"}',
        '{"1":1}',
        '{"1.1": 1.1}',
        '{"true": true}',
        '{"false": false}',
        '{"null": null}',
        '{"array": [1]}',
        '{"true"  :  true}',
        '{   "true"  :  true}',
        '{   "true"  :true}',
        '{   "true":true}',
        '{   "true":true  }',
        '{   "true":true , "x":"y" }',
        '{   "true":true,"x":"y" }',
        '{"true":true    ,"x":"y"}',
        '{"true":true,     "x":"y"}',
        '{"Под водом проводе скоро половину свог живота":true}',
        py_json.dumps(large_dict, separators=(" , ", " : "), ensure_ascii=True, default=default),
        py_json.dumps(large_dict, separators=(",", ":"), ensure_ascii=False, default=default),
        py_json.dumps(large_dict, indent=4, separators=(", ", ": "), ensure_ascii=True, default=default),
        py_json.dumps(large_dict, indent=4, separators=("   , ", ":"), ensure_ascii=False, default=default),
    ],
)
def test_dict_decode(value):
    assert yapic_json.loads(value) == py_json.loads(value)
    bytes_value = value.encode("utf-8")
    assert yapic_json.loads(bytes_value) == py_json.loads(value)


def test_dict_decode_object_hook():
    def hook(o):
        o["value"] *= 2
        return o

    assert yapic_json.loads('{"value":2}', object_hook=hook) == dict(value=4)


def test_dict_decode_invalid1(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type('{"A":2} f'))
    ex.match(re.escape("Found junk data after valid JSON data: line 1 column 9 (char 8)"))


def test_dict_decode_invalid2(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type("{"))
    ex.match(re.escape("Unexpected end of data: line 1 column 2 (char 1)"))


def test_dict_decode_invalid3(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type('{"'))
    ex.match(re.escape("Unexpected end of data: line 1 column 3 (char 2)"))


def test_dict_decode_invalid4(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type('{"D"'))
    ex.match(re.escape("Unexpected end of data: line 1 column 5 (char 4)"))


def test_dict_decode_invalid5(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type('{"D"2'))
    ex.match(
        re.escape("Unexpected character found when decoding 'dict', expected one of ':': line 1 column 5 (char 4)")
    )


def test_dict_decode_invalid6(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type('{"D" :'))
    ex.match(re.escape("Unexpected end of data: line 1 column 7 (char 6)"))


def test_dict_decode_invalid7(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type('{"D" :2'))
    ex.match(re.escape("Unexpected end of data: line 1 column 8 (char 7)"))


def test_dict_decode_invalid8(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type('{"D" :2b'))
    ex.match(
        re.escape("Unexpected character found when decoding 'dict', expected one of ',', '}': line 1 column 8 (char 7)")
    )


def test_dict_decode_invalid9(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type("{:}"))
    ex.match(
        re.escape("Unexpected character found when decoding 'dict', expected one of '\"': line 1 column 2 (char 1)")
    )


def test_dict_decode_invalid10(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type('{"id":0,}'))
    ex.match(
        re.escape("Unexpected character found when decoding 'dict', expected one of '\"': line 1 column 9 (char 8)")
    )


def test_dict_decode_invalid11(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type('{"x": true,'))
    ex.match(re.escape("Unexpected end of data: line 1 column 12 (char 11)"))


def test_dict_decode_invalid12(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type("{1:1}"))
    ex.match(
        re.escape("Unexpected character found when decoding 'dict', expected one of '\"': line 1 column 2 (char 1)")
    )


def test_dict_decode_invalid13(decoder_input_type):
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads(decoder_input_type('{"foo":"bar","baz'))
    ex.match(re.escape("Unexpected end of data: line 1 column 18 (char 17)"))
