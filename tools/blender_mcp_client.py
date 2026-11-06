#!/usr/bin/env python3
"""Minimal MCP client for the local Blender MCP HTTP server (FastMCP).

Usage:
  python tools/blender_mcp_client.py                 # list tools
  python tools/blender_mcp_client.py <tool> '<json>' # call a tool
"""
import asyncio
import json
import sys

from fastmcp import Client

URL = "http://127.0.0.1:10849/mcp"


async def main():
    async with Client(URL) as c:
        if len(sys.argv) == 1:
            tools = await c.list_tools()
            print(f"{len(tools)} tools")
            for t in tools:
                desc = (t.description or "").replace("\n", " ")[:90]
                print(f"{t.name:40s} {desc}")
            return
        name = sys.argv[1]
        args = {}
        if len(sys.argv) > 2:
            raw = sys.argv[2]
            if raw.strip().startswith("{"):
                args = json.loads(raw)
            else:
                for item in sys.argv[2:]:
                    k, _, v = item.partition("=")
                    try:
                        args[k] = json.loads(v)
                    except Exception:
                        args[k] = v
        res = await c.call_tool(name, args)
        print(json.dumps(res.data if getattr(res, "data", None) is not None else str(res), indent=1, ensure_ascii=False, default=str))


if __name__ == "__main__":
    asyncio.run(main())
