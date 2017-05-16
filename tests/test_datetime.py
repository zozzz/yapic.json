import pytest
import datetime
from datetime import timezone
from zibo import json as zibo_json
import json as py_json


class TZInfo(datetime.tzinfo):
    def __init__(self, offset):
        self.offset = offset

    def dst(self, date):
        return datetime.timedelta(seconds=0)

    def utcoffset(self, date):
        return datetime.timedelta(seconds=self.offset)


@pytest.mark.parametrize("value,expected", [
    (datetime.date(2011, 4, 30), '"2011-04-30"'),
    (datetime.date(1987, 12, 1), '"1987-12-01"'),
    (datetime.time(12, 32), '"12:32"'),
    (datetime.time(12, 32, 23), '"12:32:23"'),
    (datetime.time(12, 32, 23, 345), '"12:32:23.345"'),
    (datetime.time(12, 32, 0, 345), '"12:32:00.345"'),

    (datetime.datetime(2017, 4, 1, 12, 23, 45), '"2017-04-01 12:23:45"'),
    (datetime.datetime(2017, 4, 1, 4, 1, 2), '"2017-04-01 04:01:02"'),
    (datetime.datetime(2017, 11, 12, 13, 14, 15), '"2017-11-12 13:14:15"'),
    (datetime.datetime(2017, 4, 1, 12, 23, 45, tzinfo=TZInfo(0)), '"2017-04-01T12:23:45Z"'),
    (datetime.datetime(2017, 4, 1, 12, 23, 45, tzinfo=TZInfo(3600)), '"2017-04-01T12:23:45+01:00"'),
    (datetime.datetime(2017, 4, 1, 12, 23, 45, tzinfo=TZInfo(-3600)), '"2017-04-01T12:23:45-01:00"'),
    (datetime.datetime(2017, 4, 1, 12, 23, 45, 123, tzinfo=TZInfo(-3600)), '"2017-04-01T12:23:45.123-01:00"'),
    (datetime.datetime(2017, 4, 1, 12, 23, 45, 23, tzinfo=TZInfo(-3600)), '"2017-04-01T12:23:45.023-01:00"'),
    (datetime.datetime(2017, 4, 1, 12, 23, 45, 3, tzinfo=TZInfo(-3600)), '"2017-04-01T12:23:45.003-01:00"'),
    (datetime.datetime(2017, 4, 1, 12, 23, 45, 3, tzinfo=TZInfo(-5400)), '"2017-04-01T12:23:45.003-01:30"'),
])
def test_datetime_encode(value, expected, ensure_ascii):
    assert zibo_json.dumps(value, ensure_ascii=ensure_ascii) == expected


class WrongTZ(datetime.tzinfo):
    pass


def test_datetime_encode_wtz(ensure_ascii):
    with pytest.raises(Exception) as ex:
        zibo_json.dumps(datetime.datetime(2017, 12, 1, 1, 1, 1, tzinfo=WrongTZ()))


class WrongTZ2(datetime.tzinfo):
    def utcoffset(self, dt):
        return 0


def test_datetime_encode_wtz2(ensure_ascii):
    with pytest.raises(Exception) as ex:
        zibo_json.dumps(datetime.datetime(2017, 12, 1, 1, 1, 1, tzinfo=WrongTZ2()))


def __tz(sec):
    return timezone(datetime.timedelta(0, sec))


@pytest.mark.parametrize("value,expected", [
    ('"12:34:25"', datetime.time(12, 34, 25)),
    ('"01:01:01"', datetime.time(1, 1, 1)),
    ('"01:01:01.123456"', datetime.time(1, 1, 1, 123456)),
    ('"01:01:01Z"', datetime.time(1, 1, 1, 0, timezone.utc)),
    ('"01:01:01.123456Z"', datetime.time(1, 1, 1, 123456, timezone.utc)),
    ('"01:01:01+1000"', datetime.time(1, 1, 1, 0, __tz(10 * 3600))),
    ('"01:01:01.123456+1000"', datetime.time(1, 1, 1, 123456, __tz(10 * 3600))),
    ('"01:01:01+10:00"', datetime.time(1, 1, 1, 0, __tz(10 * 3600))),
    ('"01:01:01+10:23"', datetime.time(1, 1, 1, 0, __tz(10 * 3600 + 23 * 60))),
    ('"01:01:01-1000"', datetime.time(1, 1, 1, 0, __tz(-10 * 3600))),
    ('"01:01:01.123456-1000"', datetime.time(1, 1, 1, 123456, __tz(-10 * 3600))),
    ('"01:01:01-10:00"', datetime.time(1, 1, 1, 0, __tz(-10 * 3600))),
    ('"01:01:01-10:23"', datetime.time(1, 1, 1, 0, __tz(-(10 * 3600 + 23 * 60)))),

    ('"1900-01-01"', datetime.date(1900, 1, 1)),
    ('"2017-05-13"', datetime.date(2017, 5, 13)),

    ('"2017-05-13 20:24:23"', datetime.datetime(2017, 5, 13, 20, 24, 23)),
    ('"2017-05-13T20:24:23"', datetime.datetime(2017, 5, 13, 20, 24, 23)),
    ('"2017-05-13t20:24:23"', datetime.datetime(2017, 5, 13, 20, 24, 23)),
    ('"2017-05-13 20:24:23Z"', datetime.datetime(2017, 5, 13, 20, 24, 23, tzinfo=timezone.utc)),
    ('"2017-05-13 20:24:23z"', datetime.datetime(2017, 5, 13, 20, 24, 23, tzinfo=timezone.utc)),
    ('"2017-05-13 20:24:23.123Z"', datetime.datetime(2017, 5, 13, 20, 24, 23, 123, tzinfo=timezone.utc)),
    ('"2017-05-13 20:24:23.123+0100"', datetime.datetime(2017, 5, 13, 20, 24, 23, 123, tzinfo=__tz(3600))),
    ('"2017-05-13 20:24:23.123+01:00"', datetime.datetime(2017, 5, 13, 20, 24, 23, 123, tzinfo=__tz(3600))),
    ('"2017-05-13 20:24:23.123-0100"', datetime.datetime(2017, 5, 13, 20, 24, 23, 123, tzinfo=__tz(-3600))),
    ('"2017-05-13 20:24:23.123-01:00"', datetime.datetime(2017, 5, 13, 20, 24, 23, 123, tzinfo=__tz(-3600))),

])
def test_datetime_decode(value, expected):
    assert zibo_json.loads(value, parse_date=True) == expected


@pytest.mark.parametrize("value", [
    '"11:11:11 something"',
    '"11:11"',
    '"11:11:11+12"',
    '"1922-10-11S"',
])
def test_datetime_decode_as_string(value):
    assert zibo_json.loads(value, parse_date=True) == py_json.loads(value)


@pytest.mark.parametrize("value", [
    '"2017-02-29"',
    '"2017-02-10 25:34:12"',
    '"25:34:12+1000"',
    '"22:34:12.11111111111"',
    '"22:34:12.11111111111+1000"'
])
def test_datetime_decode_invalid(value):
    with pytest.raises(ValueError):
        zibo_json.loads(value, parse_date=True)
