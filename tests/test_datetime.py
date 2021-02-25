import pytest
from datetime import timezone, date, datetime, time, tzinfo, timedelta
from yapic import json as yapic_json
import json as py_json


class TZInfo(tzinfo):
    def __init__(self, offset):
        self.offset = offset

    def utcoffset(self, date):
        return timedelta(seconds=self.offset)


class MyDate(date):
    pass


class MyDateTime(datetime):
    pass


class MyTime(time):
    pass


@pytest.mark.parametrize(
    "value,expected",
    [(date(2011, 4, 30), '"2011-04-30"'), (date(1987, 12, 1), '"1987-12-01"'), (time(12, 32), '"12:32"'),
     (time(12, 32, 23), '"12:32:23"'), (time(12, 32, 23, 345), '"12:32:23.345"'), (time(12, 32, 0, 1), '"12:32:00.1"'),
     (time(12, 32, 0, 12), '"12:32:00.12"'), (time(12, 32, 0, 123), '"12:32:00.123"'),
     (time(12, 32, 0, 1234), '"12:32:00.1234"'), (time(12, 32, 0, 12345), '"12:32:00.12345"'),
     (time(12, 32, 0, 123456), '"12:32:00.123456"'), (datetime(2017, 4, 1, 12, 23, 45), '"2017-04-01 12:23:45"'),
     (datetime(2017, 4, 1, 4, 1, 2), '"2017-04-01 04:01:02"'),
     (datetime(2017, 11, 12, 13, 14, 15), '"2017-11-12 13:14:15"'),
     (datetime(2017, 4, 1, 12, 23, 45, tzinfo=TZInfo(0)), '"2017-04-01T12:23:45Z"'),
     (datetime(2017, 4, 1, 12, 23, 45, tzinfo=TZInfo(3600)), '"2017-04-01T12:23:45+01:00"'),
     (datetime(2017, 4, 1, 12, 23, 45, tzinfo=TZInfo(-3600)), '"2017-04-01T12:23:45-01:00"'),
     (datetime(2017, 4, 1, 12, 23, 45, 1, tzinfo=TZInfo(-3600)), '"2017-04-01T12:23:45.1-01:00"'),
     (datetime(2017, 4, 1, 12, 23, 45, 12, tzinfo=TZInfo(-3600)), '"2017-04-01T12:23:45.12-01:00"'),
     (datetime(2017, 4, 1, 12, 23, 45, 123, tzinfo=TZInfo(-3600)), '"2017-04-01T12:23:45.123-01:00"'),
     (datetime(2017, 4, 1, 12, 23, 45, 1234, tzinfo=TZInfo(-3600)), '"2017-04-01T12:23:45.1234-01:00"'),
     (datetime(2017, 4, 1, 12, 23, 45, 12345, tzinfo=TZInfo(-3600)), '"2017-04-01T12:23:45.12345-01:00"'),
     (datetime(2017, 4, 1, 12, 23, 45, 123456, tzinfo=TZInfo(-3600)), '"2017-04-01T12:23:45.123456-01:00"'),
     (datetime(2017, 4, 1, 12, 23, 45, 23000, tzinfo=TZInfo(-3600)), '"2017-04-01T12:23:45.23-01:00"'),
     (datetime(2017, 4, 1, 12, 23, 45, 3, tzinfo=TZInfo(-3600)), '"2017-04-01T12:23:45.3-01:00"'),
     (datetime(2017, 4, 1, 12, 23, 45, 3, tzinfo=TZInfo(-5400)), '"2017-04-01T12:23:45.3-01:30"')])
def test_datetime_encode(value, expected, ensure_ascii):
    assert yapic_json.dumps(value, ensure_ascii=ensure_ascii) == expected
    assert yapic_json.dumpb(value, ensure_ascii=ensure_ascii) == expected.encode("utf-8")


def test_datetime_subclass_encode():
    assert yapic_json.dumps(MyDate(2020, 1, 1)) == '"2020-01-01"'
    assert yapic_json.dumps(MyDateTime(2020, 1, 1, 12, 34, 56)) == '"2020-01-01 12:34:56"'
    assert yapic_json.dumps(MyTime(12, 34, 56)) == '"12:34:56"'


class WrongTZ(tzinfo):
    pass


def test_datetime_encode_wtz(ensure_ascii):
    with pytest.raises(Exception):
        yapic_json.dumps(datetime(2017, 12, 1, 1, 1, 1, tzinfo=WrongTZ()))


class WrongTZ2(tzinfo):
    def utcoffset(self, dt):
        return 0


def test_datetime_encode_wtz2(ensure_ascii):
    with pytest.raises(Exception):
        yapic_json.dumps(datetime(2017, 12, 1, 1, 1, 1, tzinfo=WrongTZ2()))


def __tz(sec):
    return timezone(timedelta(0, sec))


@pytest.mark.parametrize("value,expected", [
    ('"12:34:25"', time(12, 34, 25)),
    ('"01:01:01"', time(1, 1, 1)),
    ('"01:01:01.123456"', time(1, 1, 1, 123456)),
    ('"01:01:01Z"', time(1, 1, 1, 0, timezone.utc)),
    ('"01:01:01.123456Z"', time(1, 1, 1, 123456, timezone.utc)),
    ('"01:01:01+1000"', time(1, 1, 1, 0, __tz(10 * 3600))),
    ('"01:01:01.123456+1000"', time(1, 1, 1, 123456, __tz(10 * 3600))),
    ('"01:01:01+10:00"', time(1, 1, 1, 0, __tz(10 * 3600))),
    ('"01:01:01+10:23"', time(1, 1, 1, 0, __tz(10 * 3600 + 23 * 60))),
    ('"01:01:01-1000"', time(1, 1, 1, 0, __tz(-10 * 3600))),
    ('"01:01:01.123456-1000"', time(1, 1, 1, 123456, __tz(-10 * 3600))),
    ('"01:01:01-10:00"', time(1, 1, 1, 0, __tz(-10 * 3600))),
    ('"01:01:01-10:23"', time(1, 1, 1, 0, __tz(-(10 * 3600 + 23 * 60)))),
    ('"1900-01-01"', date(1900, 1, 1)),
    ('"2017-05-13"', date(2017, 5, 13)),
    ('"2017-05-13 20:24:23"', datetime(2017, 5, 13, 20, 24, 23)),
    ('"2017-05-13T20:24:23"', datetime(2017, 5, 13, 20, 24, 23)),
    ('"2017-05-13t20:24:23"', datetime(2017, 5, 13, 20, 24, 23)),
    ('"2017-05-13 20:24:23Z"', datetime(2017, 5, 13, 20, 24, 23, tzinfo=timezone.utc)),
    ('"2017-05-13 20:24:23z"', datetime(2017, 5, 13, 20, 24, 23, tzinfo=timezone.utc)),
    ('"2017-05-13 20:24:23.123Z"', datetime(2017, 5, 13, 20, 24, 23, 123, tzinfo=timezone.utc)),
    ('"2017-05-13 20:24:23.123+0100"', datetime(2017, 5, 13, 20, 24, 23, 123, tzinfo=__tz(3600))),
    ('"2017-05-13 20:24:23.123+01:00"', datetime(2017, 5, 13, 20, 24, 23, 123, tzinfo=__tz(3600))),
    ('"2017-05-13 20:24:23.123-0100"', datetime(2017, 5, 13, 20, 24, 23, 123, tzinfo=__tz(-3600))),
    ('"2017-05-13 20:24:23.123-01:00"', datetime(2017, 5, 13, 20, 24, 23, 123, tzinfo=__tz(-3600))),
])
def test_datetime_decode(value, expected):
    assert yapic_json.loads(value, parse_date=True) == expected
    bytes_value = value.encode("utf-8")
    assert yapic_json.loads(bytes_value, parse_date=True) == expected


@pytest.mark.parametrize("value", [
    '"11:11:11 something"',
    '"11:11"',
    '"11:11:11+12"',
    '"1922-10-11S"',
])
def test_datetime_decode_as_string(value):
    assert yapic_json.loads(value, parse_date=True) == py_json.loads(value)

    bytes_value = value.encode("utf-8")
    assert yapic_json.loads(bytes_value, parse_date=True) == py_json.loads(value)


@pytest.mark.parametrize("value", [
    '"2017-02-29"',
    '"2017-02-10 25:34:12"',
    '"25:34:12+1000"',
])
def test_datetime_decode_invalid(value):
    # TODO: strict parsing https://github.com/zozzz/yapic.json/issues/10
    # with pytest.raises(ValueError):
    #     yapic_json.loads(value, parse_date=True)

    assert yapic_json.loads(value, parse_date=True) == py_json.loads(value)


@pytest.mark.parametrize("value,expected", [
    ('"2017-05-13 22:34:12.11111111111"', datetime(2017, 5, 13, 22, 34, 12, 111111)),
    ('"2017-05-13 22:34:12.999999999999-01:00"', datetime(2017, 5, 13, 22, 34, 12, 999999, tzinfo=__tz(-3600))),
    ('"22:34:12.11111111111"', time(22, 34, 12, 111111)),
    ('"22:34:12.11111111111+1000"', time(22, 34, 12, 111111, tzinfo=__tz(36000))),
])
def test_datetime_decode_largems(value, expected):
    assert yapic_json.loads(value, parse_date=True) == expected
