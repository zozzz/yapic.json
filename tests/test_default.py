import pytest
from zibo import json as zibo_json


def test_default_encode(ensure_ascii):
    class O:
        pass

    def default(o):
        if isinstance(o, O):
            return dict(key="value")

    assert zibo_json.dumps(O(), ensure_ascii=ensure_ascii, default=default) == '{"key":"value"}'


def test_default_encode_exception(ensure_ascii):
    class Ex(Exception):
        pass

    class O:
        def do_something(self):
            raise Ex()

    def default(o):
        if isinstance(o, O):
            o.do_something()

    with pytest.raises(Ex):
        zibo_json.dumps(O(), ensure_ascii=ensure_ascii, default=default)

    def default2(o):
        if isinstance(o, O):
            raise Ex()

    with pytest.raises(Ex):
        zibo_json.dumps(O(), ensure_ascii=ensure_ascii, default=default2)


def test_default_encode_dict_key(ensure_ascii):
    class DeferredString:
        pass

    def default(o):
        if isinstance(o, DeferredString):
            return "loaded-later"

    x = {}
    x[DeferredString()] = 42

    assert zibo_json.dumps(x, ensure_ascii=ensure_ascii, default=default) == '{"loaded-later":42}'


def test_default_invalid_dict_key(ensure_ascii):
    class DeferredString:
        pass

    def default(o):
        if isinstance(o, DeferredString):
            return dict()
        raise ValueError("Invalid...")

    x = {}
    x[DeferredString()] = 42

    with pytest.raises(ValueError) as ex:
        zibo_json.dumps(x, ensure_ascii=ensure_ascii, default=default)

    ex.match("Invalid...")


def test_default_encode_recursive(ensure_ascii):
    class JsonValue:
        pass

    def default(o):
        return JsonValue()

    with pytest.raises(zibo_json.JsonEncodeError) as ex:
        zibo_json.dumps(JsonValue(), ensure_ascii=ensure_ascii, default=default)

    ex.match("Maximum recursion level reached, while encoding .*? with default function.")
