import pytest
from enum import Enum, IntEnum, IntFlag, auto
from yapic import json as yapic_json


class _AnyEnum(Enum):
    v1 = {"x": 1, "y": 2}


class _IntEnum(IntEnum):
    v1 = 42


class _IntEnum2(IntEnum):
    v1 = auto()
    v2 = auto()
    v3 = auto()


class _FlagEnum(IntFlag):
    v1 = 8


class _TupleEnum(tuple, Enum):
    v1 = (4, 2)


class _StrEnum(str, Enum):
    v1 = "Hello World"


CASES = [
    (_AnyEnum.v1, '{"x":1,"y":2}'),
    (_IntEnum.v1, '42'),
    (_IntEnum2.v3, '3'),
    (_FlagEnum.v1, '8'),
    (_TupleEnum.v1, '[4,2]'),
    (_StrEnum.v1, '"Hello World"'),
    (
        {
            _StrEnum.v1: "ok"
        },
        '{"Hello World":"ok"}',
    ),
    (
        {
            _IntEnum.v1: "x-42"
        },
        '{"42":"x-42"}',
    ),
]


@pytest.mark.parametrize("value,expected", CASES)
def test_encode_enum(value, expected, ensure_ascii):
    assert yapic_json.dumps(value, ensure_ascii=ensure_ascii) == expected


def test_invalid_dict_key():
    invalid = {_AnyEnum.v1: "invalid"}

    with pytest.raises(yapic_json.JsonEncodeError) as ex:
        assert yapic_json.dumps(invalid)
    ex.match("invalid dict key")
