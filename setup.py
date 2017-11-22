#!/usr/bin/env python3

import sys
import os
from glob import glob
from os import path
from pathlib import Path
from setuptools import setup, Extension
from setuptools.command.test import test as TestCommand
from setuptools import Command

VERSION = "1.0.0"

define_macros = {
    "YAPIC_JSON_VERSION_MAJOR": VERSION.split(".")[0],
    "YAPIC_JSON_VERSION_MINOR": VERSION.split(".")[1],
    "YAPIC_JSON_VERSION_PATCH": VERSION.split(".")[2],
}
undef_macros = []
extra_compile_args = []

if sys.platform == "win32":
    define_macros["UNICODE"] = 1

    DEVELOP = sys.executable.endswith("python_d.exe")
    if DEVELOP:
        define_macros["_DEBUG"] = 1
        undef_macros.append("NDEBUG")
        extra_compile_args.append("/MTd")
        # extra_compile_args.append("/Zi")
else:
    DEVELOP = False


sources = glob("libs/double-conversion/double-conversion/*.cc")
sources.append("src/json.cpp")
depends = glob("src/*.h")

extension = Extension(
    name="yapic.json",
    language="c++",
    sources=sources,
    depends=depends,
    include_dirs=[
        "libs/double-conversion",
        "libs/yapic.core/src/yapic/core/include"
    ],
    define_macros=list(define_macros.items()),
    undef_macros=list(undef_macros),
    extra_compile_args=extra_compile_args
)


def cmd_prerun(cmd, requirements):
    for r in requirements(cmd.distribution):
        installed = cmd.distribution.fetch_build_eggs(r if r else None)
        print(installed)

        for dp in map(lambda x: x.location, installed):
            if dp not in sys.path:
                sys.path.insert(0, dp)

    cmd.run_command('build_ext')

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
        self.pytest_args = "-x -s"
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


setup(
    name="yapic.json",
    packages=["yapic.json"],
    package_dir={"yapic.json": "src"},
    ext_modules=[extension],
    version=VERSION,
    description="Fastest JSON encode / decode library.",
    author="Vetési Zoltán",
    author_email="vetesi.zoltan@gmail.com",
    url="http://github.com",
    tests_require=["pytest"],
    extras_require={
        "benchmark": [
            "simplejson",
            "ujson",
            "python-rapidjson",
            "termcolor",
            "metamagic.json"
        ]
    },
    python_requires=">=3.5",
    cmdclass={
        "test": PyTest,
        "benchmark": Benchmark
    }
)
