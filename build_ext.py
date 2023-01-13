import sys
from glob import glob

from setuptools import Extension

VERSION = "1.8.9"

define_macros = {
    "YAPIC_JSON_VERSION_MAJOR": VERSION.split(".")[0],
    "YAPIC_JSON_VERSION_MINOR": VERSION.split(".")[1],
    "YAPIC_JSON_VERSION_PATCH": VERSION.split(".")[2],
}
undef_macros = []
extra_compile_args = []

if sys.platform == "win32":
    define_macros["UNICODE"] = "1"

    DEVELOP = sys.executable.endswith("python_d.exe")
    if DEVELOP:
        define_macros["_DEBUG"] = "1"
        undef_macros.append("NDEBUG")
        extra_compile_args.append("/MTd")
        # extra_compile_args.append("/Zi")
    # extra_compile_args.append("/FAs")
else:
    extra_compile_args.append("-std=c++11")
    extra_compile_args.append("-Wno-unknown-pragmas")
    extra_compile_args.append("-Wno-write-strings")

    DEVELOP = sys.executable.endswith("-dbg")
    if DEVELOP:
        define_macros["_DEBUG"] = 1
        undef_macros.append("NDEBUG")
        extra_compile_args.append("-g3")
    else:
        extra_compile_args.append("-O3")

sources = glob("libs/double-conversion/double-conversion/*.cc")
sources.append("src/json.cpp")
depends = glob("src/*.h")

extension = Extension(
    name="yapic.json._json",
    language="c++",
    sources=sources,
    depends=depends,
    include_dirs=["libs/double-conversion", "libs/yapic.core/src/yapic/core/include"],
    define_macros=list(define_macros.items()),
    undef_macros=list(undef_macros),
    extra_compile_args=extra_compile_args,
)


def build(setup_kwargs):
    setup_kwargs.update(
        packages=["yapic.json"],
        package_dir={"yapic.json": "src"},
        package_data={"yapic.json": ["_json.pyi"]},
        ext_modules=[extension],
    )
