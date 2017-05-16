import pytest

import json as py_json
import ujson
import simplejson
from zibo import json as zibo_json


@pytest.fixture(
    params=[
        py_json.dumps,
        ujson.dumps,
        simplejson.dumps,
        zibo_json.dumps
    ],
    ids=[
        "ZIBO",
        "PYTHON",
        "UJSON",
        "SIMPLEJSON"
    ]
)
def dumps(request):
    return request.param
