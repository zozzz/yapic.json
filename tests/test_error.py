import json as py_json

from yapic import json


def test_decode_error(decoder_input_type):
    assert json.JsonDecodeError is json.JSONDecodeError

    for exc_type in (py_json.JSONDecodeError, json.JsonDecodeError, json.JSONDecodeError, json.JsonError, ValueError):
        try:
            json.loads(decoder_input_type("{"))
        except exc_type:
            pass
        except Exception:
            assert False, f"Not matching the given exception: {exc_type}"


def test_encode_error():
    class Unserializable:
        pass

    for exc_type in (json.JsonEncodeError, json.JsonError, ValueError):
        try:
            json.dumps(Unserializable())
        except exc_type:
            pass
        except Exception:
            assert False, f"Not matching the given exception: {exc_type}"
