import pytest
import math
import decimal
import json as py_json
import platform
from yapic import json as yapic_json

CASES = [
    (decimal.Decimal("1.1"), "1.1"),
    (decimal.Decimal("-1.1"), "-1.1"),
    (decimal.Decimal("3.14159265359"), "3.14159265359"),
    (decimal.Decimal("1e100"), "1E+100"),
    (decimal.Decimal("1e-100"), "1E-100"),
    (decimal.Decimal("nan"), "NaN"),
    (decimal.Decimal("infinity"), "Infinity"),
    (decimal.Decimal("-infinity"), "-Infinity"),
]


@pytest.mark.parametrize("value,expected", CASES)
def test_float_encode(value, expected, ensure_ascii):
    assert yapic_json.dumps(value, ensure_ascii=ensure_ascii) == expected
