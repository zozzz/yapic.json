import pytest
import math
from zibo import json as zibo_json


@pytest.mark.parametrize("value,expected", [
    (True, "true"),
    (False, "false"),
    (None, "null"),
    (float("nan"), "NaN"),
    (float("infinity"), "Infinity"),
    (float("-infinity"), "-Infinity"),
])
def test_const_encode(value, expected, ensure_ascii):
    assert zibo_json.dumps(value, ensure_ascii=ensure_ascii) == expected


@pytest.mark.parametrize("value,expected", [
    ("true", True),
    ("false", False),
    ("null", None),
    ("Infinity", float("infinity")),
    ("-Infinity", float("-infinity")),
])
def test_const_decode(value, expected):
    assert zibo_json.loads(value) == expected


def test_nan_decode():
    assert math.isnan(zibo_json.loads("NaN"))
