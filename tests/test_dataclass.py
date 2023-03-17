import re
import sys
from dataclasses import _FIELDS, MISSING, Field, asdict, dataclass, field
from typing import Any, List

import pytest
from yapic import json as yapic_json


class MyField(Field):
    pass


def myfield(default_factory):
    if sys.version_info[:3] >= (3, 10, 0):
        return MyField(
            default_factory=default_factory,
            default=MISSING,
            init=True,
            repr=True,
            hash=None,
            compare=True,
            metadata=None,
            kw_only=False,
        )
    else:
        return MyField(
            default_factory=default_factory,
            default=MISSING,
            init=True,
            repr=True,
            hash=None,
            compare=True,
            metadata=None,
        )


@dataclass
class Empty:
    pass


@dataclass
class Point:
    x: int
    y: int


@dataclass
class Points:
    values: List[Point]


@dataclass
class Circle:
    origo: Point


@pytest.mark.parametrize(
    "inst",
    [
        Empty(),
        Point(x=1, y=2),
        Points(values=[Point(x=3, y=4), Point(x=5, y=6)]),
        Circle(origo=Point(x=10, y=20)),
    ],
)
def test_encode_dataclass(inst):
    assert yapic_json.dumpb(inst) == yapic_json.dumpb(asdict(inst))
    assert yapic_json.dumps(inst) == yapic_json.dumps(asdict(inst))


def test_custom_field():
    @dataclass
    class CustomField:
        name: str = myfield(lambda: "Hello World")

    cf = CustomField()
    assert yapic_json.dumpb(cf) == yapic_json.dumpb(asdict(cf))


def test_recursive_1():
    @dataclass
    class R:
        r: "R" = None

    r = R()
    r.r = r

    with pytest.raises(yapic_json.JsonEncodeError) as ex:
        yapic_json.dumpb(r)
    ex.match(
        re.escape(
            "Maximum recursion level reached, while encoding dataclass "
            "test_recursive_1.<locals>.R(r=...) entry at 'r' key."
        )
    )


def test_recursive_2():
    @dataclass
    class R:
        r: List[Any] = None

    r = R()
    r.r = [r]

    with pytest.raises(yapic_json.JsonEncodeError) as ex:
        yapic_json.dumpb(r)
    ex.match(
        re.escape(
            "Maximum recursion level reached, while encoding list entry "
            "test_recursive_2.<locals>.R(r=[...]) at 0 index."
        )
    )


def test_err_wrong_dataclass():
    class DC:
        pass

    setattr(DC, _FIELDS, tuple())
    with pytest.raises(yapic_json.JsonEncodeError) as ex:
        yapic_json.dumpb(DC())
    ex.match(re.escape("Dataclass atrribute '__dataclass_fields__' has wrong type: <class 'tuple'>, expected dict."))


def test_wrong_field():
    @dataclass
    class WrongField:
        name: str = "Hello World"

    inst = WrongField()
    assert yapic_json.dumpb(inst) == b'{"name":"Hello World"}'

    getattr(WrongField, _FIELDS)["name"] = "This is a wrong value"

    assert yapic_json.dumpb(inst) == b"{}"


def test_missing_attribute():
    @dataclass
    class MissingAttribute:
        name: str = "Hello World"

    inst = MissingAttribute()
    assert yapic_json.dumpb(inst) == yapic_json.dumpb(asdict(inst))

    getattr(MissingAttribute, _FIELDS)["extra"] = field(default="Extra Field")

    with pytest.raises(AttributeError) as ex:
        assert yapic_json.dumpb(inst) == yapic_json.dumpb(asdict(inst))
    ex.match(re.escape("'MissingAttribute' object has no attribute 'extra'"))
