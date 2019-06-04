import pytest
import json as py_json
from uuid import UUID
from collections.abc import ItemsView
from yapic import json as yapic_json


class Int:
    def __init__(self, value):
        self.value = value

    def __json__(self):
        return self.value


def default(o):
    return o.value


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
    (large_dict, py_json.dumps)
])
def test_dict_encode(value, expected, ensure_ascii):
    if expected is py_json.dumps:
        expected = py_json.dumps(value, ensure_ascii=ensure_ascii, separators=(",", ":"), default=default)
    assert yapic_json.dumps(value, ensure_ascii=ensure_ascii) == expected
    assert yapic_json.dumps(value.items(), ensure_ascii=ensure_ascii) == expected
    assert yapic_json.dumps(ItemsView(value), ensure_ascii=ensure_ascii) == expected


def test_dict_recursive(ensure_ascii):
    d = dict(
        recursive=True
    )
    d["self"] = d

    with pytest.raises(yapic_json.JsonEncodeError) as ex:
        yapic_json.dumps(d, ensure_ascii=ensure_ascii)

    ex.match("Maximum recursion level reached, while encoding dict entry .*? at 'self' key.")


@pytest.mark.parametrize("value", [
    '{}',
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
])
def test_dict_decode(value):
    assert yapic_json.loads(value) == py_json.loads(value)


def test_dict_decode_object_hook():
    def hook(o):
        o["value"] *= 2
        return o

    assert yapic_json.loads('{"value":2}', object_hook=hook) == dict(value=4)


def test_dict_decode_invalid1():
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads('{"A":2} f')
    ex.match("Found junk data after valid JSON data at position: 8.")


def test_dict_decode_invalid2():
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads('{')
    ex.match("Unexpected end of data at position: 1.")


def test_dict_decode_invalid3():
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads('{"')
    ex.match("Unexpected end of data at position: 2.")


def test_dict_decode_invalid4():
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads('{"D"')
    ex.match("Unexpected end of data at position: 4.")


def test_dict_decode_invalid5():
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads('{"D"2')
    ex.match("Unexpected character found when decoding 'dict', expected one of ':' at position: 4.")


def test_dict_decode_invalid6():
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads('{"D" :')
    ex.match("Unexpected end of data at position: 6.")


def test_dict_decode_invalid7():
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads('{"D" :2')
    ex.match("Unexpected end of data at position: 7.")


def test_dict_decode_invalid8():
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads('{"D" :2b')
    ex.match("Unexpected character found when decoding 'dict', expected one of ',', '}' at position: 7.")


def test_dict_decode_invalid9():
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads('{:}')
    ex.match("Unexpected character found when decoding 'dict', expected one of \" at position: 1.")
