#!/usr/bin/env python3
"""Execute a Blender Python file through the local Blender MCP server.

Usage: python tools/blender_run.py path/to/script.py
"""
import asyncio
import json
import sys

from fastmcp import Client

URL = "http://127.0.0.1:10849/mcp"


async def main():
    path = sys.argv[1]
    with open(path, encoding="utf-8") as f:
        code = f.read()
    async with Client(URL) as c:
        res = await c.call_tool("script_execute", {"code": code})
        data = res.data if getattr(res, "data", None) is not None else str(res)
        print(json.dumps(data, indent=1, ensure_ascii=False, default=str))


if __name__ == "__main__":
    asyncio.run(main())

