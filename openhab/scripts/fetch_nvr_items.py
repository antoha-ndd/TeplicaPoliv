#!/usr/bin/env python3
import base64
import json
import urllib.request

BASE = "http://192.168.0.51:8080"
cred = base64.b64encode(b"admin:admin").decode()


def get(path):
    req = urllib.request.Request(BASE + path)
    req.add_header("Authorization", "Basic " + cred)
    with urllib.request.urlopen(req, timeout=15) as r:
        return json.load(r)


items = get("/rest/items")
nvr = sorted([i for i in items if i["name"].startswith("NVR_")], key=lambda x: x["name"])
for i in nvr:
    s = i.get("state", "NULL")
    if len(s) > 150:
        s = s[:150] + "..."
    print(f"{i['name']:28} {i['type']:8} {s}")
