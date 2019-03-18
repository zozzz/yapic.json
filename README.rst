yapic.json
===========

.. image:: https://img.shields.io/appveyor/ci/zozzz/yapic-json/release.svg?label=windows&style=flat-square
      :alt: AppVeyor
      :target: https://ci.appveyor.com/project/zozzz/yapic-json

.. image:: https://img.shields.io/circleci/project/github/zozzz/yapic.json/release.svg?label=linux&style=flat-square
      :alt: CircleCI
      :target: https://circleci.com/gh/zozzz/yapic.json

.. image:: https://img.shields.io/travis/com/zozzz/yapic.json/release.svg?label=sdist&style=flat-square
      :alt: Travis
      :target: https://travis-ci.com/zozzz/yapic.json

.. image:: https://img.shields.io/pypi/dm/yapic.json.svg?style=flat-square
      :alt: PyPI - Downloads
      :target: https://pypi.org/project/yapic.json/


``yapic.json`` is an extreamly fast json encoder / decoder package for python.
Encoding and decoding output fully compatible with ``python.json`` package.

Features
--------

*  Extreamly fast *(see benchmark results in '/test/benchmark' directory)*
*  Fully compatible output with Python json package
*  Builtin object serialization method ``__json__`` *(see below)*
*  Strict `JSON (RFC 4627) <http://www.ietf.org/rfc/rfc4627.txt?number=4627>`_ expected: ``Infinity``, ``NaN`` (*JavaScript* compatible infinity and not a number symbols)
*  UTF-8 encoding & decoding support
*  Accurate float encoding & decoding
*  ``date`` / ``datetime`` / ``time`` encondig & decoding support


Requirements
^^^^^^^^^^^^

- Only works with Python 3.5 or greater
- c++ 11 comaptible compiler. *(only if u want to build from source)*

  Wheels provided for windows x86/x64 and linux x86/x64


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

-  `dumps <https://github.com/zozzz/yapic.json/blob/master/src/_json.pyi#L20>`_ (**obj:** ``Any``, ``*``, **default:** ``Callable[[Any], JSONT]=None``, **tojson:** ``str="__json__"``, **ensure_ascii:** ``bool=True``, **encode_datetime:** ``bool=True``)

   **default example:**

   .. code-block:: python

      >>> from yapic import json
      >>> def default_func(o):
      ...     if isinstance(o, complex):
      ...         return {"__complex__":True, "real":1, "imag":2}
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
      ...         return {"x":self.x, "y":self.y}
      ...
      >>> json.dumps(Point(10, 20))
      '{"x":10,"y":20}'


Exceptions
----------

- ``yapic.json.JsonError``: base exception class
- ``yapic.json.JsonEncodeError``: exception class for encoding errors
- ``yapic.json.JsonDecodeError``: exception class for decoding errors


Release Process
---------------

- change ``VERSION`` in ``setup.py``
- ``git add setup.py``
- ``git commit -m "chore(bump): VERSION"``
- ``git checkout release``
- ``git merge master``
- ``git tag -a VERSION -m "chore(bump): VERSION"``
- ``git push && git push --tags``
- ``git checkout master``
- ``git merge release``
