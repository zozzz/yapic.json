import pytest
from yapic import json as yapic_json


def test_json_method_encode(ensure_ascii):
    class O:
        def __json__(self):
            return dict(key="value")

    assert yapic_json.dumps(O(), ensure_ascii=ensure_ascii) == '{"key":"value"}'


def test_json_method_encode_exception(ensure_ascii):
    class Ex(Exception):
        pass

    class O:
        def __json__(self):
            raise Ex()

    with pytest.raises(Ex):
        yapic_json.dumps(O(), ensure_ascii=ensure_ascii)


def test_json_method_encode_cname(ensure_ascii):
    class O:
        def tojson(self):
            return dict(key="value")

    assert yapic_json.dumps(O(), ensure_ascii=ensure_ascii, tojson="tojson") == '{"key":"value"}'


def test_json_method_encode_dict_key(ensure_ascii):
    class DeferredString:
        def __json__(self):
            return "loaded-later"

    x = {}
    x[DeferredString()] = 42

    assert yapic_json.dumps(x, ensure_ascii=ensure_ascii) == '{"loaded-later":42}'


def test_json_method_encode_invalid_dict_key(ensure_ascii):
    class DeferredString:
        def __json__(self):
            return dict()

    x = {}
    x[DeferredString()] = 42

    with pytest.raises(yapic_json.JsonEncodeError) as ex:
        yapic_json.dumps(x, ensure_ascii=ensure_ascii)

    ex.match("This {} is an invalid dict key, please provide")


def test_json_method_encode_recursive(ensure_ascii):
    class Recursive:
        def __json__(self):
            return self

    with pytest.raises(yapic_json.JsonEncodeError) as ex:
        yapic_json.dumps(Recursive(), ensure_ascii=ensure_ascii)

    ex.match("Maximum recursion level reached, while encoding .*? with '__json__' method.")
