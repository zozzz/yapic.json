import pytest

# coverage https://emptysqua.re/blog/code-coverage-python-c-extensions/


@pytest.fixture(params=[True, False], ids=["Ascii out", "Unicode out"])
def ensure_ascii(request):
    return request.param


@pytest.fixture(
    params=[
        lambda v: v,
        lambda x: x.encode("utf-8"),
        lambda x: bytearray(x.encode("utf-8")),
    ],
    ids=["str", "bytes", "bytearray"],
)
def decoder_input_type(request):
    return request.param


@pytest.fixture(
    params=[
        lambda v: v,
        lambda v: '["ű",%s]' % v,
        lambda v: '["𐌌",%s]' % v,
        lambda x: x.encode("utf-8"),
        lambda x: bytearray(x.encode("utf-8")),
    ],
    ids=["Ascii", "2 byte unicode", "4 byte unicode", "bytes", "bytearray"],
)
def number_input_type(request):
    return request.param
