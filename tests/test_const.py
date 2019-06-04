import pytest
import math
from uuid import UUID
from yapic import json as yapic_json


@pytest.mark.parametrize("value,expected", [
    (True, "true"),
    (False, "false"),
    (None, "null"),
    (float("nan"), "NaN"),
    (float("infinity"), "Infinity"),
    (float("-infinity"), "-Infinity"),
])
def test_const_encode(value, expected, ensure_ascii):
    assert yapic_json.dumps(value, ensure_ascii=ensure_ascii) == expected


@pytest.mark.parametrize("value,expected", [
    ("true", True),
    ("false", False),
    ("null", None),
    ("Infinity", float("infinity")),
    ("-Infinity", float("-infinity")),
])
def test_const_decode(value, expected):
    assert yapic_json.loads(value) == expected


def test_nan_decode():
    assert math.isnan(yapic_json.loads("NaN"))


def test_uuid_encode():
    id = UUID("af113bd3-cd8e-40b4-8bda-e61752c3b2bf")
    assert yapic_json.dumps(id) == "af113bd3-cd8e-40b4-8bda-e61752c3b2bf"
