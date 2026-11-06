# Blender 3.6 MCP — local setup (verified)

| Item | Value |
|---|---|
| Blender | `P:\Blender\blender.exe` — **3.6.23** |
| MCP server | `P:\Blender\blender-mcp` (sandraschi/blender-mcp, v0.11.1) |
| Python env | `P:\Blender\blender-mcp\.venv` |
| Transport | Streamable HTTP — `http://127.0.0.1:10849/mcp` |
| Tools | 71 |
| Skill | `blender-expert` (+ `autonomous-modeling`, `pipeline-automation`) from GitHub |
| Bridge addon | `P:\Blender\3.6\scripts\addons\blender_bridge_addon.py` (enabled; for live GUI) |

## Start / stop

```powershell
powershell -ExecutionPolicy Bypass -File tools\blender_mcp_start.ps1
powershell -ExecutionPolicy Bypass -File tools\blender_mcp_stop.ps1
```

The start script sets `BLENDER_EXECUTABLE=P:\Blender\blender.exe` and waits for
`http://127.0.0.1:10849/api/v1/health`.

## Client

```powershell
# list the 71 MCP tools
P:\Blender\blender-mcp\.venv\Scripts\python.exe tools\blender_mcp_client.py

# call a tool (key=value args)
P:\Blender\blender-mcp\.venv\Scripts\python.exe tools\blender_mcp_client.py blender_status operation=status

# run a Blender Python file headlessly through MCP
P:\Blender\blender-mcp\.venv\Scripts\python.exe tools\blender_run.py tools\mcp_test_render.py
```

## Verified

- `scene_get_hierarchy` returned the default Blender 3.6.23 scene.
- `blender_status` reported Blender available at `P:\Blender\blender.exe`.
- `mcp_test_render.py` rendered a Cycles image through MCP:
  `P:\Blender\blender-mcp\logs\mcp_connection_test.png`.

## State model

`script_execute` / `blender_run.py` start a fresh headless Blender per call.
For multi-step design, either:

1. open the design `.blend` at the start of each script and save it at the end, or
2. run a live GUI session (`blender_session start`) with the bridge addon
   (Properties → Scene → Blender MCP Bridge → Start Bridge) and use
   `blender_session run_script`.

## Notes

- The local repo is 38 commits behind `origin/main`; it is functional and tested.
  Update with `git -C P:\Blender\blender-mcp pull --ff-only` + reinstall the venv
  package if a newer server is required.
- The skill was downloaded from
  `https://raw.githubusercontent.com/sandraschi/blender-mcp/main/skills/blender-expert/SKILL.md`
  and installed in `.agents/skills/` and `~/.dsh/skills/`.

## Live GUI bridge (implemented)

- The addon lender_bridge_addon.py was patched from "blender": (4, 2, 0) to (3, 6, 0)
  so Blender 3.6 lists and persists it; it is enabled in userpref.blend.
- The running GUI starts the bridge on TCP 127.0.0.1:10850 (Properties > Scene > Blender MCP Bridge).
- Execute code in the live GUI:

`powershell
P:\Blender\blender-mcp\.venv\Scripts\python.exe tools\blender_live.py ping
P:\Blender\blender-mcp\.venv\Scripts\python.exe tools\blender_live.py path\to\script.py
`

- Design scene: Design/Blender/pirate_boat.blend (boat + standing pirates on the rim,
  camera/lights/material preview). Opened in the live GUI.