#!/usr/bin/env python3
import base64
import json
import urllib.request

BASE = "http://192.168.0.51:8080"
cred = base64.b64encode(b"admin:admin").decode()


def get(path):
    req = urllib.request.Request(BASE + path)
    req.add_header("Authorization", "Basic " + cred)
    with urllib.request.urlopen(req, timeout=20) as r:
        return json.load(r)


things = get("/rest/things")
cam_things = [t for t in things if t.get("thingTypeUID") == "ipcamera:generic"]
print(f"ipcamera things: {len(cam_things)}\n")
for t in sorted(cam_things, key=lambda x: x["UID"]):
    cfg = t.get("configuration", {})
    print(f"UID: {t['UID']}")
    print(f"  label: {t.get('label')}")
    print(f"  status: {t.get('statusInfo', {})}")
    print(f"  ip: {cfg.get('ipAddress')} port: {cfg.get('port')}")
    print(f"  ffmpegInput: {cfg.get('ffmpegInput', '')[:100]}")
    linked = {}
    for ch in t.get("channels", []):
        for li in ch.get("linkedItems", []):
            linked[ch["id"]] = li
    if linked:
        print(f"  linked: {linked}")
    else:
        print("  linked: (none)")
    print()
