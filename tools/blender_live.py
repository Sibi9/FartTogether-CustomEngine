#!/usr/bin/env python3
"""Execute Python in the LIVE Blender GUI via the blender_bridge_addon TCP socket.

Usage:
  python tools/blender_live.py ping
  python tools/blender_live.py path/to/script.py
  python tools/blender_live.py "import bpy; print(bpy.context.scene.name)"
"""
import json
import socket
import struct
import sys

HOST, PORT = "127.0.0.1", 10850


def call(msg):
    s = socket.create_connection((HOST, PORT), timeout=15)
    payload = json.dumps(msg).encode("utf-8")
    s.sendall(struct.pack("!I", len(payload)) + payload)
    hdr = s.recv(4)
    n = struct.unpack("!I", hdr)[0]
    data = b""
    while len(data) < n:
        data += s.recv(4096)
    s.close()
    return json.loads(data)


if __name__ == "__main__":
    arg = sys.argv[1]
    if arg == "ping":
        print(json.dumps(call({"type": "ping", "params": {}}), indent=1))
    else:
        code = open(arg, encoding="utf-8").read() if arg.endswith(".py") else arg
        res = call({"type": "execute_code", "params": {"code": code}})
        print(json.dumps(res, indent=1, ensure_ascii=False)[:6000])
