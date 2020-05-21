import json as py_json
from yapic import json


def test_d():
    # print(len(json.dumpb(["almafa" * 30000, "almafa" * 30000])))
    assert json.dumpb("Á", ensure_ascii=False) == py_json.dumps("Á", ensure_ascii=False).encode("utf-8")
    assert json.dumpb("\xFF", ensure_ascii=False) == py_json.dumps("\xFF", ensure_ascii=False).encode("utf-8")
    assert json.dumpb("\uFFFF", ensure_ascii=False) == py_json.dumps("\uFFFF", ensure_ascii=False).encode("utf-8")
