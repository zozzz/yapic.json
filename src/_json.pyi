from typing import Union, Optional, Callable, Any
from datetime import date, datetime, time

JSONT = Union[str, bytes, dict, list, tuple, int, float, date, datetime, time]

# class Writeable:
#     def write(data: str) -> Any:
#         pass


def loads(s: Union[bytes, str],
          *,
          object_hook: Optional[Callable[[dict], Any]] = None,
          parse_float: Optional[Callable[[str], Any]] = None,
          parse_date: bool = True) -> JSONT:
    """ Convert JSON string to Python object


    :param s: data which is need to convert to Python object
    :param object_hook: if specified, will be called with the result of
        any object literal decoded. The return value of *object_hook*
        will be used instead of the ``dict``.
    :param parse_float: if specified, will be called with
        the string of every JSON float to be decoded
    :param parse_date: if ``True`` parse ``date`` / ``datetime`` / ``time``
        according to ``ISO 8601``format
    """


def dumps(obj: Any,
          *,
          default: Optional[Callable[[Any], JSONT]],
          tojson: str = "__json__",
          ensure_ascii: bool = True,
          encode_datetime: bool = True) -> str:
    """ Convert Python object to JSON string.


    :param obj: python object which is need to convert to JSON
    :param default: default function
        for not supported python type serialization
    :param tojson: method name which called on objects to get
        serializable Python object
    :param ensure_ascii: always return ASCII compatible string.
    :param encode_datetime: if ``True`` encode ``date`` / ``datetime`` / ``time``
        objects to string according to ISO 8601 format
    """


def dumpb(obj: Any,
          *,
          default: Optional[Callable[[Any], JSONT]],
          tojson: str = "__json__",
          ensure_ascii: bool = True,
          encode_datetime: bool = True) -> bytes:
    """ Convert Python object to JSON bytes.


    :param obj: python object which is need to convert to JSON
    :param default: default function
        for not supported python type serialization
    :param tojson: method name which called on objects to get
        serializable Python object
    :param ensure_ascii: always return ASCII compatible string.
    :param encode_datetime: if ``True`` encode ``date`` / ``datetime`` / ``time``
        objects to string according to ISO 8601 format
    """
