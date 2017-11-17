import pytest

import json as py_json
import ujson
import simplejson
from yapic import json as yapic_json


@pytest.fixture(
    params=[
        py_json.dumps,
        ujson.dumps,
        simplejson.dumps,
        yapic_json.dumps
    ],
    ids=[
        "YAPIC",
        "PYTHON",
        "UJSON",
        "SIMPLEJSON"
    ]
)
def dumps(request):
    return request.param
