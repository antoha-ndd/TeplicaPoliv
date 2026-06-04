#!/usr/bin/env python3
"""Создаёт/обновляет Thing TeplicaPoliv и items в OpenHAB через REST API."""
import base64
import json
import sys
import urllib.error
import urllib.parse
import urllib.request

BASE = "http://192.168.0.51:8080"
AUTH = ("admin", "admin")
BRIDGE_UID = "mqtt:broker:9d15be98b2"
THING_UID = "mqtt:topic:9d15be98b2:teplicaesp"
MQTT_PREFIX = "TeplicaPoliv"


def auth_header():
    return "Basic " + base64.b64encode(f"{AUTH[0]}:{AUTH[1]}".encode()).decode()


def req(method, path, data=None):
    url = BASE + path
    r = urllib.request.Request(url, method=method)
    r.add_header("Content-Type", "application/json; charset=utf-8")
    r.add_header("Authorization", auth_header())
    body = json.dumps(data).encode("utf-8") if data is not None else None
    try:
        with urllib.request.urlopen(r, data=body, timeout=45) as resp:
            return resp.status, resp.read().decode("utf-8", errors="replace")
    except urllib.error.HTTPError as e:
        return e.code, e.read().decode("utf-8", errors="replace")


def ch_number(cid, label, rel_topic):
    return {
        "uid": f"{THING_UID}:{cid}",
        "id": cid,
        "channelTypeUID": "mqtt:number",
        "itemType": "Number",
        "kind": "STATE",
        "label": label,
        "defaultTags": [],
        "properties": {},
        "configuration": {"stateTopic": f"{MQTT_PREFIX}/{rel_topic}"},
    }


def ch_switch_rw(cid, label, state_rel, cmd_rel):
    return {
        "uid": f"{THING_UID}:{cid}",
        "id": cid,
        "channelTypeUID": "mqtt:switch",
        "itemType": "Switch",
        "kind": "STATE",
        "label": label,
        "defaultTags": [],
        "properties": {},
        "configuration": {
            "stateTopic": f"{MQTT_PREFIX}/{state_rel}",
            "commandTopic": f"{MQTT_PREFIX}/{cmd_rel}",
            "on": "1",
            "off": "0",
        },
    }


def ch_switch_ro(cid, label, rel_topic):
    return {
        "uid": f"{THING_UID}:{cid}",
        "id": cid,
        "channelTypeUID": "mqtt:switch",
        "itemType": "Switch",
        "kind": "STATE",
        "label": label,
        "defaultTags": [],
        "properties": {},
        "configuration": {
            "stateTopic": f"{MQTT_PREFIX}/{rel_topic}",
            "on": "1",
            "off": "0",
        },
    }


def main():
    code, body = req("GET", f"/rest/things/{THING_UID}")
    if code == 404:
        minimal = {
            "UID": THING_UID,
            "label": "Теплица ESP (TeplicaPoliv)",
            "thingTypeUID": "mqtt:topic",
            "bridgeUID": BRIDGE_UID,
            "configuration": {},
        }
        code, body = req("POST", "/rest/things", minimal)
        if code >= 400:
            print("POST thing:", code, body[:1500])
            return 1

    code, body = req("GET", f"/rest/things/{THING_UID}")
    if code != 200:
        print("GET thing:", code, body[:500])
        return 1
    thing = json.loads(body)

    thing["channels"] = [
        ch_number("t_temp1", "Температура 1", "state/temp1"),
        ch_number("t_temp2", "Температура 2", "state/temp2"),
        ch_number("t_minwater", "Порог MinWater", "state/minwater"),
        ch_switch_ro("t_limiter", "Бочка полна", "state/limiter"),
        ch_switch_rw("t_motor1", "Мотор 1 открыт", "state/motor1", "motor1"),
        ch_switch_rw("t_motor2", "Мотор 2 открыт", "state/motor2", "motor2"),
        ch_switch_rw("t_motor3", "Мотор 3 открыт", "state/motor3", "motor3"),
        ch_switch_rw("t_motor4", "Мотор 4 открыт", "state/motor4", "motor4"),
        ch_switch_rw("t_pump", "Насос", "state/pump", "pump"),
    ]
    thing.pop("statusInfo", None)

    code, body = req("PUT", f"/rest/things/{THING_UID}", thing)
    print(f"Thing PUT: HTTP {code}")
    if code >= 400:
        print(body[:2000])
        return 1

    grp = {"type": "Group", "name": "gTeplica", "label": "Теплица"}
    c3, _ = req("PUT", "/rest/items/gTeplica", grp)
    print(f"Group gTeplica: HTTP {c3}")

    items = [
        ("Teplica_Temp1", "Температура 1", "Number:Temperature", f"{THING_UID}:t_temp1"),
        ("Teplica_Temp2", "Температура 2", "Number:Temperature", f"{THING_UID}:t_temp2"),
        ("Teplica_MinWater", "Порог MinWater", "Number", f"{THING_UID}:t_minwater"),
        ("Teplica_Limiter", "Бочка полна", "Switch", f"{THING_UID}:t_limiter"),
        ("Teplica_Motor1", "Мотор 1", "Switch", f"{THING_UID}:t_motor1"),
        ("Teplica_Motor2", "Мотор 2", "Switch", f"{THING_UID}:t_motor2"),
        ("Teplica_Motor3", "Мотор 3", "Switch", f"{THING_UID}:t_motor3"),
        ("Teplica_Motor4", "Мотор 4", "Switch", f"{THING_UID}:t_motor4"),
        ("Teplica_Pump", "Насос", "Switch", f"{THING_UID}:t_pump"),
    ]

    for name, label, typ, channel_uid in items:
        item = {
            "type": typ,
            "name": name,
            "label": label,
            "groupNames": ["gTeplica"],
        }
        c2, b2 = req("PUT", f"/rest/items/{name}", item)
        if c2 not in (200, 201):
            print(f"Item {name}: HTTP {c2} {b2[:400]}")
            continue
        ch_enc = urllib.parse.quote(channel_uid, safe="")
        c4, b4 = req("PUT", f"/rest/links/{name}/{ch_enc}", {})
        if c4 not in (200, 201):
            print(f"  link {name}: HTTP {c4} {b4[:300]}")
        else:
            print(f"Item+link {name}: OK")

    print("OK: Thing Teplica ESP, group gTeplica, links created.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
