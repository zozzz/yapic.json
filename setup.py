#!/usr/bin/env python3

import sys
import os
from glob import glob
from os import path
from distutils.core import setup, Extension

DEVELOP = os.environ.get("DEBUG") == "1"
__version__ = "1.0.0"


sources = glob("libs/double-conversion/double-conversion/*.cc")
sources.append("src/json.cpp")

extension = Extension(
    name="zibo.json",
    library_dirs=[path.join(sys.prefix, "libs")],
    language="c++",
    sources=sources,
    # include_dirs=[
    #     "src"
    # ],
    define_macros=[
        ("ZIBO_JSON_VERSION_MAJOR", __version__.split(".")[0]),
        ("ZIBO_JSON_VERSION_MINOR", __version__.split(".")[1]),
        ("ZIBO_JSON_VERSION_PATCH", __version__.split(".")[2]),
        ("ZIBO_JSON_USE_SSE", 0),
        ("NDEBUG", 1)
    ],
    undef_macros=(["NDEBUG"] if DEVELOP else None),
    extra_compile_args=["-msse4.2", "-std=c++11"],
    # extra_compile_args=["-std=c++11"],
)

setup(
    name="zibo.json",
    ext_modules=[extension],
    version=__version__,
    description="Fastest JSON encode / decode library.",
    author="Vetési Zoltán",
    author_email="vetesi.zoltan@gmail.com",
    url="http://github.com"
)
