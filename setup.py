#!/usr/bin/env python3

import sys
import os
from glob import glob
from os import path
from setuptools import setup, Extension

DEVELOP = os.environ.get("DEBUG") == "1"
VERSION = "1.0.0"

sources = glob("libs/double-conversion/double-conversion/*.cc")
sources.append("src/json.cpp")

extension = Extension(
    name="yapic.json",
    library_dirs=[path.join(sys.prefix, "libs")],
    language="c++",
    sources=sources,
    include_dirs=[
        "libs/double-conversion",
        "libs/yapic.core/src/yapic/core/include"
    ],
    define_macros=[
        ("YAPIC_JSON_VERSION_MAJOR", VERSION.split(".")[0]),
        ("YAPIC_JSON_VERSION_MINOR", VERSION.split(".")[1]),
        ("YAPIC_JSON_VERSION_PATCH", VERSION.split(".")[2]),
        ("YAPIC_JSON_USE_SSE", 0),
        ("NDEBUG", 1)
    ],
    undef_macros=(["NDEBUG"] if DEVELOP else None),
    # extra_compile_args=["-msse4.2", "-std=c++11"],
    extra_compile_args=[] if os.name == "nt" else ["-std=c++11"],
    # extra_compile_args=["-std=c++11"],
)

setup(
    name="yapic.json",
    ext_modules=[extension],
    version=VERSION,
    description="Fastest JSON encode / decode library.",
    author="Vetési Zoltán",
    author_email="vetesi.zoltan@gmail.com",
    url="http://github.com"
)
