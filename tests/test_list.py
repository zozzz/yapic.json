import pytest
import json as py_json
from yapic import json as yapic_json

CASES = [
    [],
    [[], [], []],
    [[[[[[[[[[[[[[[[1]]]]]]]]]]]]]]]],
    [1],
    [-1],
    [42],
    [1.1, 3.1345343645634645, 20e123],
    ["Hello World"],
    ["𐌀𐌂𐌃𐌄𐌅𐌆𐌇𐌈𐌉𐌋𐌌𐌍𐌐𐌑𐌓𐌔𐌕𐌖𐌘𐌙𐌚"],
    ["половинуÁ𐌐𐌑𐌓"],
    [["H", [1], [4.4, 5.5], [[["W"]]]] for i in range(10)],
]


@pytest.mark.parametrize("value", CASES)
def test_list_encode(value, ensure_ascii):
    expected = py_json.dumps(value, separators=(",", ":"), ensure_ascii=ensure_ascii)
    assert yapic_json.dumps(value, ensure_ascii=ensure_ascii) == expected


def test_list_encode_recursive(ensure_ascii):
    l = [1, 2]
    l.append(l)

    with pytest.raises(yapic_json.JsonEncodeError) as ex:
        yapic_json.dumps(l, ensure_ascii=ensure_ascii)

    ex.match("Maximum recursion level reached, while encoding list entry .*? at 2 index.")


@pytest.mark.parametrize("expected", CASES)
def test_list_decode(expected, ensure_ascii):
    value = py_json.dumps(expected, separators=(",", ":"), ensure_ascii=ensure_ascii)
    assert yapic_json.loads(value) == expected


def test_list_decode_invalid1():
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads('[')
    ex.match("Unexpected end of data at position: 1.")


def test_list_decode_invalid2():
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads('[2')
    ex.match("Unexpected end of data at position: 2.")


def test_list_decode_invalid3():
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads('[2,')
    ex.match("Unexpected end of data at position: 3.")


def test_list_decode_invalid4():
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads('[2,]')
    ex.match("Unexpected charcter: ']' at position: 3.")


def test_list_decode_invalid5():
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads('[2b')
    ex.match("Unexpected character found when decoding 'list', expected one of ',', ']' at position: 2.")


def test_list_decode_invalid6():
    with pytest.raises(yapic_json.JsonDecodeError) as ex:
        yapic_json.loads('[,]')
    ex.match("Unexpected charcter: ',' at position: 1.")
