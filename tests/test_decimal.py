import pytest
import math
import decimal
import json as py_json
import platform
from yapic import json as yapic_json

CASES = [
    (decimal.Decimal("0.1"), "0.1"),
    (decimal.Decimal("1.1"), "1.1"),
    (decimal.Decimal("-1.1"), "-1.1"),
    (decimal.Decimal("3.14159265359"), "3.14159265359"),
    (decimal.Decimal("1e100"), "1E+100"),
    (decimal.Decimal("1e-100"), "1E-100"),
    (decimal.Decimal("0e-8"), "0E-8"),
    (decimal.Decimal("0E+3"), "0E+3"),
    (decimal.Decimal("nan"), "NaN"),
    (decimal.Decimal("infinity"), "Infinity"),
    (decimal.Decimal("-infinity"), "-Infinity"),
]


@pytest.mark.parametrize("value,expected", CASES)
def test_decimal_encode(value, expected, ensure_ascii):
    assert yapic_json.dumps(value, ensure_ascii=ensure_ascii) == expected


@pytest.mark.parametrize("expected,value", CASES)
def test_decimal_decode(value, expected):
    if value not in ("NaN", "Infinity", "-Infinity"):
        assert yapic_json.loads(value, parse_float=decimal.Decimal) == expected
        assert yapic_json.loads(value, parse_float=decimal.Decimal) == py_json.loads(value, parse_float=decimal.Decimal)


def test_decimal_decode_nan():
    assert math.isnan(yapic_json.loads("NaN", parse_float=decimal.Decimal)) \
        == math.isnan(py_json.loads("NaN", parse_float=decimal.Decimal))


def test_decimal_decode_infinity():
    assert math.isinf(yapic_json.loads("Infinity", parse_float=decimal.Decimal)) \
        == math.isinf(py_json.loads("Infinity", parse_float=decimal.Decimal))

    assert math.isinf(yapic_json.loads("-Infinity", parse_float=decimal.Decimal)) \
        == math.isinf(py_json.loads("-Infinity", parse_float=decimal.Decimal))
