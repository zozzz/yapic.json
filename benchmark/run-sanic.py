import shlex
import subprocess
import sys
import time
from os import path, system

from sanic import Sanic
from sanic import response

from zibo import json as zibo_json
import ujson

app = Sanic()

CONCURENCY = 10
REQUESTES = 1000


with open(path.join(path.dirname(__file__), "large-data.json"), "r") as f:
    LARGE_JSON_STRING = f.read()
    LARGE_JSON = zibo_json.loads(LARGE_JSON_STRING)


@app.route("/zibo-stream")
async def get_zibo_strem(request):
    async def streaming_fn(response):
        zibo_json.dump(LARGE_JSON, response)
    return response.stream(streaming_fn, content_type="application/json; charset=utf-8")


@app.route("/zibo")
async def get_zibo(request):
    return response.text(zibo_json.dumps(LARGE_JSON, ensure_ascii=False), content_type="application/json; charset=utf-8")


@app.route("/ujson-stream")
async def get_ujson_stream(request):
    async def streaming_fn(response):
        ujson.dump(LARGE_JSON, response)
    return response.stream(streaming_fn, content_type="application/json; charset=utf-8")


@app.route("/ujson")
async def get_ujson(request):
    return response.text(ujson.dumps(LARGE_JSON, ensure_ascii=False), content_type="application/json; charset=utf-8")


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "serve":
        app.run(host="0.0.0.0", port=8000, debug=False, log_config=None)
    else:
        print(["python", __file__, "serve"])
        proc = subprocess.Popen(["python", __file__, "serve"])
        time.sleep(5)

        def run_test(name):
            # system("siege -b -c %s -r %s http://localhost:8000/%s" % (CONCURENCY, REQUESTES, name))
            system("ab -c %s -n %s http://localhost:8000/%s" % (CONCURENCY, REQUESTES, name))

        run_test("zibo")
        run_test("zibo-stream")
        run_test("ujson")
        run_test("ujson-stream")

        proc.terminate()
