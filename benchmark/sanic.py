from sanic import Sanic
from sanic import response

from zibo import json as zibo_json

app = Sanic()


@app.route("/zibo")
async def index(request):
    async def streaming_fn(response):
        zibo_json.dump("Almafa", response)
        await True
    return response.stream(streaming_fn, content_type="application/json")


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8000)
