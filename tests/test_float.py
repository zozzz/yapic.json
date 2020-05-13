import pytest
import math
import decimal
import json as py_json
import platform
from yapic import json as yapic_json

IS32BIT = platform.architecture()[0] == "32bit"

CASES = [
    (1.1, "1.1"),
    (-1.1, "-1.1"),
    (3.14159265359, "3.14159265359"),
    (1e100, "1e+100"),
    (1e-100, "1e-100"),
    (float("nan"), "NaN"),
    (float("infinity"), "Infinity"),
    (float("-infinity"), "-Infinity"),
]

# rounding error on 32bit machine
if not IS32BIT:
    CASES.append((89255.0 / 1e22, "8.9255e-18"))


@pytest.mark.parametrize("value,expected", CASES)
def test_float_encode(value, expected, ensure_ascii):
    assert yapic_json.dumps(value, ensure_ascii=ensure_ascii) == expected


@pytest.mark.parametrize("expected,value", CASES)
def test_float_decode(value, expected, decoder_input_type):
    if math.isnan(expected):
        assert math.isnan(yapic_json.loads(value))
    else:
        value = decoder_input_type(value)
        assert yapic_json.loads(value) == py_json.loads(value)


@pytest.mark.parametrize("value", [
    "12345.34e23",
    "12345.34e-2300",
    "12345e+2300",
    "12345e-2300",
    "1.0001",
    "-0.0001",
    "1.0001e2",
    "31415.926535897932",
    "[31415.926535897932,314159.26535897932]",
    "0.0001e2",
    "0.1",
    "0.0000",
    "[0,0.0]",
    "1.00e2",
    "0e-2",
    "0E-7",
])
def test_float_decode2(value, decoder_input_type):
    value = decoder_input_type(value)
    assert yapic_json.loads(value) == py_json.loads(value)


@pytest.mark.parametrize("expected,value", CASES)
def test_float_parse_hook(value, expected, decoder_input_type):
    if not math.isnan(expected):
        value = decoder_input_type(value)
        assert yapic_json.loads(value, parse_float=decimal.Decimal) == py_json.loads(value, parse_float=decimal.Decimal)
