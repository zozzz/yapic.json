import time
import sys
import math
import re
from decimal import Decimal
from termcolor import colored

import json as py_json
import simplejson
import ujson
import rapidjson
import metamagic.json
from yapic import json as yapic_json

BENCHMARKS = []


class BenchmarkMeta(type):
    def __init__(self, name, bases, dict):
        if name != "Benchmark":
            BENCHMARKS.append(self)


class Benchmark(metaclass=BenchmarkMeta):
    ROUNDS = 100
    ITERATIONS = 1000
    ENSURE_ASCII = True

    ENCODER = (
        ("yapic", yapic_json.dumps),
        ("python", py_json.dumps),
        # ("simple", simplejson.dumps),
        ("ujson", ujson.dumps),
        ("rapidjson", rapidjson.dumps),
        ("metamagic", metamagic.json.dumps),
    )

    DECODER = (
        ("yapic", yapic_json.loads),
        ("python", py_json.loads),
        # ("simple", simplejson.loads),
        ("ujson", ujson.loads),
        ("rapidjson", rapidjson.loads),
        ("metamagic", metamagic.json.loads),
    )

    def get_encode_data(self):
        pass

    def get_decode_data(self):
        return py_json.dumps(self.get_encode_data(), separators=(',', ':'), ensure_ascii=self.ENSURE_ASCII)

    time = time.perf_counter

    @classmethod
    def run_all(cls, only=None):
        modes = []
        skip_comparsion = False
        if only:
            if "ENCODE" in only:
                modes.append("ENCODE")
                only.remove("ENCODE")
            if "DECODE" in only:
                modes.append("DECODE")
                only.remove("DECODE")
            if "NOCMP" in only:
                skip_comparsion = True
        if not modes:
            modes = ["ENCODE", "DECODE"]

        cls.__measure(only, modes, skip_comparsion)

    def make_fn(cls, callback, args, kwargs):
        def runner(storage):
            storage["rounds"] = cls.ROUNDS
            storage["iterations"] = cls.ITERATIONS

            try:
                # warmup
                callback(*args, **kwargs)

                for r in range(cls.ROUNDS):
                    start = cls.time()
                    for i in range(cls.ITERATIONS):
                        callback(*args, **kwargs)
                    result = Decimal(cls.time() - start)

                    # start = cls.time()
                    # for i in range(cls.ITERATIONS):
                    #     pass
                    # overhead = Decimal(cls.time() - start)
                    # result -= overhead

                    storage["times"].append((result / Decimal(cls.ITERATIONS)) * (Decimal(1000) * Decimal(1000)))
            except Exception as e:
                storage["error"] = e
        return runner

    @classmethod
    def analyze_data(cls, data):
        times = data["times"]
        min_ = min(times)
        max_ = max(times)
        sum_ = sum(times)
        mean = sum_ / Decimal(data["rounds"])
        std_dev = []

        for t in times:
            std_dev.append(pow(abs(t - mean), 2))

        res = dict(
            min=min_,
            max=max_,
            mean=mean,
            call_sec=(Decimal(data["rounds"]) * Decimal(data["iterations"])) / sum_,
            std_dev=Decimal(math.sqrt(sum(std_dev) / Decimal(data["rounds"])))
        )
        res["score"] = res["mean"]
        return res

    @classmethod
    def __measure(cls, only, modes, skip_comparsion):
        libname_max = 0

        if "ENCODE" in modes and cls.ENCODER:
            for e in cls.ENCODER:
                libname_max = max(libname_max, len(e[0]))

        if "DECODE" in modes and cls.DECODER:
            for e in cls.DECODER:
                libname_max = max(libname_max, len(e[0]))

        table = Table(100, libname_max)

        for benchmark in BENCHMARKS:
            if only:
                matched = False
                for pattern in only:
                    if re.match(pattern, benchmark.__name__):
                        matched = True
                        break

                if not matched:
                    # table.write_skipped(benchmark.__doc__.strip())
                    continue

            b = benchmark()
            benchmark.encode_data = {}
            benchmark.decode_data = {}

            table.write_header(b.__doc__.strip(), benchmark.ROUNDS, benchmark.ITERATIONS)

            for t in modes:
                t = getattr(b, t + "R")
                if not t:
                    continue

                if t is b.ENCODER:
                    data = benchmark.encode_data
                    args = [b.get_encode_data()]
                    kwargs = {"ensure_ascii": benchmark.ENSURE_ASCII}
                    # kwargs = {}
                    table.write_group("ENCODE")
                else:
                    data = benchmark.decode_data
                    args = [b.get_decode_data()]
                    kwargs = {}
                    table.write_group("DECODE")

                for (lib_name, lib_fn) in t:
                    if skip_comparsion and lib_name != "yapic":
                        continue

                    if lib_name in ("simple", "python") and t is b.ENCODER:
                        kwargs["default"] = b.default
                    elif "default" in kwargs:
                        del kwargs["default"]
                    elif lib_name == "metamagic" and t is b.ENCODER:
                        del kwargs["ensure_ascii"]

                    local_data = data[lib_name] = {"times": []}
                    table.write_row(lib_name, True, None)
                    b.make_fn(lib_fn, args, kwargs)(local_data)
                    if "error" in local_data:
                        table.write_row(lib_name, False, local_data["error"])
                    else:
                        row_data = cls.analyze_data(local_data)
                        table.write_row(lib_name, False, row_data)

                table.update_rows()

    def default(self, o):
        raise RuntimeError("Unsupported")


class Table:
    def __init__(self, width, libname_max):
        self.width = width
        self.lineno = 0
        width = width - 4 - (libname_max + 5)
        self.columns = [
            ["Title", "title", libname_max + 5, None, None],
            ["Min", "min", round(width / 5), lambda a, b: a < b, lambda a, b: a > b],
            ["Max", "max", round(width / 5), lambda a, b: a < b, lambda a, b: a > b],
            ["Mean", "mean", round(width / 5), lambda a, b: a < b, lambda a, b: a > b],
            ["StdDev", "std_dev", round(width / 5), lambda a, b: a < b, lambda a, b: a > b],
            ["Call/sec", "call_sec", 0, lambda a, b: a > b, lambda a, b: a < b]
        ]

        used = 0
        for i, x in enumerate(self.columns):
            if i > 0:
                used += x[2]
        self.columns[len(self.columns) - 1][2] = width - used

        self.row_data = []
        self.row_start_at = 0

    def write_skipped(self, title):
        self._write(colored("SKIPPED: %s" % title, "yellow") + "\n")

    def write_header(self, title, rounds, iters):
        data = colored("=" * self.width, "cyan") + "\n"

        tdata = colored("| %s" % title, "cyan")
        iters = " (rounds: %s, iterations: %s)" % (rounds, iters)
        tdata += colored(iters, "blue")
        data += tdata + (" " * (self.width - len(title) - len(iters) - 3)) + colored("|", "cyan") + "\n"
        data += colored("-" * self.width, "cyan") + "\n"
        for i, c in enumerate(self.columns):
            if i == 0:
                data += colored("|", "cyan")
                data += colored((" {:<%s}" % (c[2] + 1)).format(c[0]), "white")
            else:
                data += colored(("{:>%s} " % (c[2] - 2)).format(c[0]), "white")
            data += colored("|", "cyan")
        data += "\n"

        data += colored("=" * self.width, "cyan") + "\n"
        self._write(data)

    def write_group(self, title):
        data = colored("  %s ->\n" % title, "magenta")
        self._write(data)
        self.row_data = []
        self.row_start_at = self.lineno + 1

    def write_row(self, title, in_progress, data=None, color=None):
        if in_progress:
            self._write(colored(("    {:<%s}meassuring..." % self.columns[0][2]).format(title), "red"))
        elif isinstance(data, Exception):
            self.row_data.append(dict(title=title, data=data))
            err_max_length = 0 - 4 - self.columns[0][2] - 7
            for x in self.columns:
                err_max_length += x[2]
            row_template = "\r    {0:<%s}error: {1:<%s}\n" % (self.columns[0][2], err_max_length)
            self._write(colored(row_template.format(title, str(data)[0:err_max_length]), "white", "on_red"))
        else:
            self.row_data.append(dict(title=title, data=data))
            row = "\r"
            for (ctitle, field, width, best_cmp, worst_cmp) in self.columns:
                if field == "title":
                    value = title
                    field_value = ("    {:<%s}" % width).format(value)
                else:
                    value = data[field]
                    # field_value = ("{0:>%s.5f}  ").format(value)
                    field_value = ("{0:>%s.5f}  " % (width - 2)).format(value)

                if color and color.get(field):
                    row += colored(field_value, color.get(field))
                else:
                    row += field_value
            self._write(row + "\n")

    def update_rows(self):
        sys.stdout.write("\033[%sA" % len(self.row_data))
        row_data = []
        error_rows = []

        for row in self.row_data:
            if isinstance(row["data"], Exception):
                error_rows.append(row)
            else:
                row_data.append(row)

        row_data.sort(key=lambda item: item["data"]["score"])
        best = {}
        worst = {}

        for i, data in enumerate(row_data):
            for (title, field, width, best_cmp, worst_cmp) in self.columns:
                if field == "title":
                    continue

                if field not in best or best_cmp(data["data"][field], best[field]["value"]):
                    best[field] = dict(value=data["data"][field], index=i)

                if field not in worst or worst_cmp(data["data"][field], worst[field]["value"]):
                    worst[field] = dict(value=data["data"][field], index=i)

        for i, data in enumerate(row_data + error_rows):
            color = {}
            for k, v in worst.items():
                if v["index"] == i:
                    color[k] = "red"

            for k, v in best.items():
                if v["index"] == i:
                    color[k] = "green"

            sys.stdout.write("\033[G\033[K")
            self.write_row(data["title"], False, data["data"], color)

    def _write(self, data):
        sys.stdout.write(data)
        sys.stdout.flush()
        self.lineno += len(data.splitlines())
