import json as py_json
from yapic import json


def test_d():
    assert json.loads('"D\\u00C1"'.encode("utf-8")) == "DÁ"

