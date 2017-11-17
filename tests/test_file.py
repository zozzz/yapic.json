import pytest
import json as py_json
from io import StringIO
from yapic import json as yapic_json


@pytest.mark.parametrize(
    "value,expected",
    [
        ({"Hello": "World"}, '{"Hello":"World"}'),
        ({"Hello": "Árvíztűrő tükörfúrógép"}, py_json.dumps),
        ("Под водом проводе скоро половину свог живота", py_json.dumps),
        ("Под водом проводе скоро половину свог живота" * 100000, py_json.dumps),
    ],
    ids=[
        "Hello World",
        "Árvíztűrő tükörfúrógép",
        "Short UTF-8",
        "Long UTF-8"
    ])
def test_encode_to_file(value, expected, ensure_ascii):
    with StringIO() as s:
        yapic_json.dump(value, s, ensure_ascii=ensure_ascii)
        if expected is py_json.dumps:
            assert s.getvalue() == py_json.dumps(value, ensure_ascii=ensure_ascii, separators=(",", ":"))
        else:
            assert s.getvalue() == expected
