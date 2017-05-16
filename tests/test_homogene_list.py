import pytest
from zibo import json


def test_homogene_list_basics():
    assert isinstance(json.HomogeneList, type)

    class InvalidImpl(json.HomogeneList):
        pass

    assert issubclass(InvalidImpl, json.HomogeneList)
    assert isinstance(InvalidImpl(), json.HomogeneList)

    with pytest.raises(NotImplementedError) as excinfo:
        for x in InvalidImpl():
            pass

    excinfo.match(".*?InvalidImpl.__iter__$")
