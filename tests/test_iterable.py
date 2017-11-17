import pytest
from yapic import json as yapic_json
from yapic.json import JsonEncodeError


@pytest.mark.only
def test_iterable_encode_basic(ensure_ascii):
    assert yapic_json.dumps(iter([1, 2, 3, 4, 5]), ensure_ascii=ensure_ascii) == "[1,2,3,4,5]"


def test_iterable_encode_obj(ensure_ascii):
    class Iterable:
        def __iter__(self):
            self.i = 0
            return self

        def __next__(self):
            self.i += 1
            if self.i < 6:
                return self.i
            raise StopIteration()

    assert yapic_json.dumps(Iterable(), ensure_ascii=ensure_ascii) == "[1,2,3,4,5]"


def test_iterable_encode_generator(ensure_ascii):
    def generator():
        yield 1
        yield 2
        yield 3
        yield 4
        yield 5

    assert yapic_json.dumps(generator(), ensure_ascii=ensure_ascii) == "[1,2,3,4,5]"


def test_iterable_encode_exception():
    def generator():
        yield 1
        raise RuntimeError("some error")

    with pytest.raises(RuntimeError) as excinfo:
        yapic_json.dumps(generator())

    excinfo.match("some error")


def test_iterable_encode_recursive():
    def recursive():
        yield 1
        yield recursive()

    with pytest.raises(JsonEncodeError) as excinfo:
        yapic_json.dumps(recursive())

    excinfo.match("Maximum recursion level reached, while encoding iterable entry .*? at 1 index.")


def test_iterable_encode_recursive2():
    def recursive():
        yield recursive()

    with pytest.raises(JsonEncodeError) as excinfo:
        yapic_json.dumps(recursive())

    excinfo.match("Maximum recursion level reached, while encoding iterable entry .*? at 0 index.")
