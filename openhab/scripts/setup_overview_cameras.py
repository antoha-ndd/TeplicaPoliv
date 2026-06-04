#!/usr/bin/env python3
"""Link NVR ipcamera items and configure Overview page widgets."""
from __future__ import annotations

import base64
import json
import urllib.error
import urllib.request

BASE = "http://192.168.0.51:8080"
AUTH = base64.b64encode(b"admin:admin").decode()
OH_HOST = "192.168.0.51"
OH_PORT = 8080
CAM_COUNT = 10

# Labels from things (approximate)
CAM_LABELS = {
    1: "Канал 1 (NVR .80)",
    2: "Канал 2 (NVR .80)",
    3: "Камера .93",
    4: "Камера .94",
    5: "Канал 5",
    6: "Канал 6 (NVR .80)",
    7: "Канал 7 (NVR .80)",
    8: "Канал 8 (NVR .80)",
    9: "Камера .99",
    10: "Канал 10 (NVR .80)",
}


def request(method: str, path: str, data=None):
    url = BASE + path
    body = None
    headers = {"Authorization": "Basic " + AUTH, "Content-Type": "application/json"}
    if data is not None:
        body = json.dumps(data).encode("utf-8")
    req = urllib.request.Request(url, data=body, headers=headers, method=method)
    try:
        with urllib.request.urlopen(req, timeout=30) as r:
            if r.length and r.length > 0:
                return json.loads(r.read().decode())
            return r.read().decode() or None
    except urllib.error.HTTPError as e:
        err = e.read().decode("utf-8", "replace")
        raise RuntimeError(f"{method} {path} -> {e.code}: {err}") from e


def link_item(item: str, channel_uid: str):
    request("PUT", f"/rest/links/{item}/{channel_uid}", {})


def mjpeg_url(n: int) -> str:
    return f"http://{OH_HOST}:{OH_PORT}/ipcamera/nvr{n}/ipcamera.mjpeg"


def snapshot_url(n: int) -> str:
    return f"http://{OH_HOST}:{OH_PORT}/ipcamera/nvr{n}/ipcamera.jpg"


def make_camera_col(n: int) -> dict:
    label = CAM_LABELS.get(n, f"Канал {n}")
    return {
        "component": "oh-grid-col",
        "config": {"width": "50%", "sm": "100%", "md": "50%", "lg": "33%"},
        "slots": {
            "default": [
                {
                    "component": "oh-image-card",
                    "config": {
                        "title": label,
                        "url": mjpeg_url(n),
                        "refreshInterval": 0,
                        "lazy": True,
                        "style": {
                            "width": "100%",
                            "maxHeight": "280px",
                            "objectFit": "contain",
                        },
                    },
                }
            ],
        },
    }


def build_overview_page() -> dict:
    cols = [make_camera_col(n) for n in range(1, CAM_COUNT + 1)]
    return {
        "uid": "overview",
        "component": "oh-layout-page",
        "config": {"label": "Overview"},
        "slots": {
            "masonry": [
                {
                    "component": "oh-block",
                    "config": {"title": "Видеонаблюдение NVR", "icon": "camera"},
                    "slots": {
                        "default": [
                            {
                                "component": "oh-grid-row",
                                "config": {},
                                "slots": {"default": cols},
                            }
                        ]
                    },
                }
            ],
            "default": [],
        },
    }


def main():
    print("Linking items to ipcamera channels...")
    for n in range(1, CAM_COUNT + 1):
        thing = f"ipcamera:generic:nvr{n}"
        links = [
            (f"NVR_Cam{n}_MjpegUrl", f"{thing}:mjpegUrl"),
            (f"NVR_Cam{n}_ImageUrl", f"{thing}:imageUrl"),
        ]
        if n == 1:
            links.append((f"NVR_Cam{n}_Image", f"{thing}:image"))
        for item, ch in links:
            try:
                link_item(item, ch)
                print(f"  linked {item}")
            except RuntimeError as e:
                print(f"  skip {item}: {e}")

    print("\nUpdating Overview page...")
    page = build_overview_page()
    request("PUT", "/rest/ui/components/ui:page/overview", page)
    print("Done. Open http://192.168.0.51:8080/overview")
    print("\nMJPEG URLs:")
    for n in range(1, CAM_COUNT + 1):
        print(f"  {n}: {mjpeg_url(n)}")


if __name__ == "__main__":
    main()
