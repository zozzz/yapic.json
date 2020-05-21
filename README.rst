yapic.json
===========

.. image:: https://img.shields.io/appveyor/ci/zozzz/yapic-json/release.svg?label=windows&style=flat-square
      :alt: AppVeyor
      :target: https://ci.appveyor.com/project/zozzz/yapic-json

.. image:: https://img.shields.io/circleci/project/github/zozzz/yapic.json/release.svg?label=linux&style=flat-square
      :alt: CircleCI
      :target: https://circleci.com/gh/zozzz/yapic.json

.. image:: https://img.shields.io/travis/com/zozzz/yapic.json/release.svg?label=osx&style=flat-square
      :alt: Travis
      :target: https://travis-ci.com/zozzz/yapic.json

.. image:: https://img.shields.io/pypi/dm/yapic.json.svg?style=flat-square
      :alt: PyPI - Downloads
      :target: https://pypi.org/project/yapic.json/


``yapic.json`` is an extreamly fast json encoder / decoder package for python.
Encoding and decoding output fully compatible with ``python.json`` package.

Features
--------

*  Extreamly fast *(see benchmark results in '/benchmark' directory)*
*  Fully compatible output with Python json package
*  Builtin object serialization method ``__json__`` *(see below)*
*  Strict `JSON (RFC 4627) <http://www.ietf.org/rfc/rfc4627.txt?number=4627>`_ expected: ``Infinity``, ``NaN`` (*JavaScript* compatible infinity and not a number symbols)
*  UTF-8 encoding & decoding support
*  Accurate float encoding & decoding
*  ``date`` / ``datetime`` / ``time`` encondig & decoding support *(can encode subclasses)*
*  ``uuid.UUID`` encoding support
*  `ItemsView <https://docs.python.org/3/library/collections.abc.html#collections.abc.ItemsView>`_ encoding support

   .. code-block:: python

      from collections.abc import ItemsView

      class MyDictGenerator(ItemsView):
         def __iter__(self):
            yield ("some_key", "some_value")
            yield ("meaning_of_life", 42)

      json.dumps(MyDictGenerator()) == '{"some_key":"some_value","meaning_of_life":42}'


Requirements
------------

- Only works with Python 3.5 or greater
- c++ 11 comaptible compiler. *(only if u want to build from source)*

  Wheels provided for *windows x86/x64* and *linux x86/x64* and *osx x64*


Usage
-----

Very similar that ``python.json``, let's see some example

Json data to python

.. code-block:: python

   from yapic import json

   >>> json.loads('"Hello World"')
   "Hello World"

Python object to json data

.. code-block:: python

   from yapic import json

   >>> json.dumps("Hello World")
   '"Hello World"'

   class Point:
      def __json__(self):
         return {"x":1, "y":2}

   >>> json.dumps(Point())
   '{"x":1,"y":2}'

Functions
---------

-  `loads <https://github.com/zozzz/yapic.json/blob/master/src/_json.pyi#L11>`_ (**s:** ``bytes``, ``str``, ``*``, **object_hook:** ``Callable[[dict], Any]]=None``, **parse_float:** ``Callable[[str], Any]]=None``, **parse_date:** ``bool=True``)

   **object_hook example:**

   .. code-block:: python

      >>> from yapic import json
      >>> def hook(dict_):
      ...     if "__complex__" in dict_:
      ...         return complex(dict_["real"], dict_["imag"])
      ...
      >>> json.loads('{"__complex__":true, "real":1, "imag":2}',
      >>>     object_hook=hook)
      (1+2j)

   **parse_float example:**

   .. code-block:: python

      >>> from yapic import json
      >>> from decimal import Decimal
      >>> json.loads("1.2", parse_float=Decimal)
      Decimal('1.2')

-  `dumps <https://github.com/zozzz/yapic.json/blob/master/src/_json.pyi#L20>`_ (**obj:** ``Any``, ``*``, **default:** ``Callable[[Any], JSONT]=None``, **tojson:** ``str="__json__"``, **ensure_ascii:** ``bool=True``, **encode_datetime:** ``bool=True``) -> ``str``

   **default example:**

   .. code-block:: python

      >>> from yapic import json
      >>> def default_func(o):
      ...     if isinstance(o, complex):
      ...         return {"__complex__": True, "real": 1, "imag": 2}
      ...
      >>> json.dumps(1 + 2j, default=default_func)
      '{"__complex__":true,"real":1,"imag":2}'

   **tojson example:**

   .. code-block:: python

      >>> from yapic import json
      >>> class Point(object):
      ...     def __init__(self, x, y):
      ...         self.x = x
      ...         self.y = y
      ...     def __json__(self):
      ...         return {"x": self.x, "y": self.y}
      ...
      >>> json.dumps(Point(10, 20))
      '{"x":10,"y":20}'

-  `dumpb <https://github.com/zozzz/yapic.json/blob/master/src/_json.pyi#L50>`_ (**obj:** ``Any``, ``*``, **default:** ``Callable[[Any], JSONT]=None``, **tojson:** ``str="__json__"``, **ensure_ascii:** ``bool=True``, **encode_datetime:** ``bool=True``) -> ``bytes``

   Same as ``dumps``, but this function is return ``bytes`` insted of ``str``



Exceptions
----------

- ``yapic.json.JsonError``: base exception class
- ``yapic.json.JsonEncodeError``: exception class for encoding errors
- ``yapic.json.JsonDecodeError``: exception class for decoding errors


Json to Python translations
---------------------------

.. csv-table::
   :header: Json, Python

   """string""", "str"
   "42", "int"
   "4.2, 4e2", "float (unless you specify parse_float)"
   "Infinity", "float(""inf"")"
   "NaN", "float(""NaN"")"
   "true", "True"
   "false", "False"
   "null", "None"
   "2000-01-01 12:34:56", "datetime without timezone"
   "2000-01-01 12:34:56Z", "datetime with utc timezone"
   "2000-01-01 12:34:56+0300", "datetime with custom timezone"
   "2000-01-01", "date"
   "10:12:34", "time without timezone"
   "10:12:34+0300", "time with custom timezone"
   "{...}", "dict (unless you specify object_hook)"
   "[...]", "list"


Python to Json translations
---------------------------

.. csv-table::
   :header: Python, Json

   "str", """..."""
   "int(42)", "42"
   "float(4.2), Decimal(4.2)", "4.2"
   "float(""inf""), Decimal(""inf"")", "Infinity"
   "float(""nan""), Decimal(""nan"")", "NaN"
   "True", "true"
   "False", "false"
   "None", "null"
   "datetime", """2000-01-01 12:34:56"", ""2000-01-01T12:34:56+0300"""
   "date", """2000-01-01"""
   "time", """12:34:56"", ""12:34:56+0300"""
   "UUID", """aba04c17-6ea3-48c1-8dcd-74f0a9b79bee"""
   "dict, ItemsView", "{...}"
   "list, tuple, set, iterable", "[...]"
