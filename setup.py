#!/usr/bin/env python3

import sys
from glob import glob
from pathlib import Path
from setuptools import setup, Extension
from setuptools.command.test import test as TestCommand
from setuptools import Command

VERSION = "1.7.3"

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

extension = Extension(name="yapic.json._json",
                      language="c++",
                      sources=sources,
                      depends=depends,
                      include_dirs=["libs/double-conversion", "libs/yapic.core/src/yapic/core/include"],
                      define_macros=list(define_macros.items()),
                      undef_macros=list(undef_macros),
                      extra_compile_args=extra_compile_args)


def cmd_prerun(cmd, requirements):
    for r in requirements(cmd.distribution):
        installed = cmd.distribution.fetch_build_eggs(r if r else [])

        for dp in map(lambda x: x.location, installed):
            if dp not in sys.path:
                sys.path.insert(0, dp)

    cmd.run_command('build')

    ext = cmd.get_finalized_command("build_ext")
    ep = str(Path(ext.build_lib).absolute())

    if ep not in sys.path:
        sys.path.insert(0, ep)

    for e in ext.extensions:
        if e._needs_stub:
            ext.write_stub(ep, e, False)


class PyTest(TestCommand):
    user_options = [
        ("pytest-args=", "a", "Arguments to pass to pytest"),
        ("file=", "f", "File to run"),
    ]

    def initialize_options(self):
        super().initialize_options()
        self.pytest_args = "-x -s -vv"
        self.file = None

    def finalize_options(self):
        super().finalize_options()
        if self.file:
            self.pytest_args += " " + self.file.replace("\\", "/")

    def run(self):
        def requirements(dist):
            yield dist.install_requires
            yield dist.tests_require

        cmd_prerun(self, requirements)
        self.run_tests()

    def run_tests(self):
        import shlex
        import pytest
        errno = pytest.main(shlex.split(self.pytest_args))
        sys.exit(errno)


class Benchmark(Command):
    user_options = [
        ("file=", "f", "File to run"),
    ]

    def initialize_options(self):
        self.file = None

    def finalize_options(self):
        pass

    def run(self):
        def requirements(dist):
            yield dist.extras_require["benchmark"]

        cmd_prerun(self, requirements)
        sys.path.insert(0, "./benchmark")
        from run import Benchmark
        Benchmark.run_all(self.file)


setup(name="yapic.json",
      version=VERSION,
      url="https://github.com/zozzz/yapic.json/",
      author="Zoltán Vetési",
      author_email="vetesi.zoltan@gmail.com",
      long_description=(Path(__file__).parent / "README.rst").read_text(encoding="utf-8"),
      license="BSD",
      packages=["yapic.json"],
      package_dir={"yapic.json": "src"},
      package_data={"yapic.json": ["_json.pyi"]},
      ext_modules=[extension],
      description="Fastest JSON encode / decode library.",
      tests_require=["pytest"],
      extras_require={"benchmark": ["simplejson", "ujson", "python-rapidjson", "termcolor", "metamagic.json"]},
      python_requires=">=3.5",
      cmdclass={
          "test": PyTest,
          "benchmark": Benchmark
      },
      classifiers=[
          "Development Status :: 5 - Production/Stable",
          "License :: OSI Approved :: BSD License",
          "Operating System :: Microsoft :: Windows",
          "Operating System :: Unix",
          "Operating System :: POSIX :: Linux",
          "Operating System :: MacOS",
          "Programming Language :: C++",
          "Programming Language :: Python :: 3.6",
          "Programming Language :: Python :: 3.7",
          "Programming Language :: Python :: 3.8",
          "Programming Language :: Python :: 3.9",
          "Programming Language :: Python :: 3 :: Only",
          "Programming Language :: Python :: Implementation :: CPython",
          "Topic :: Software Development :: Libraries :: Python Modules",
          "Topic :: Utilities",
          "Typing :: Typed",
      ])
