# Start the local Blender MCP HTTP backend (sandraschi/blender-mcp) for Blender 3.6.
$ErrorActionPreference = 'Stop'
$Repo = 'P:\Blender\blender-mcp'
$Exe  = 'P:\Blender\blender.exe'
$env:BLENDER_EXECUTABLE = $Exe
$env:PYTHONUNBUFFERED = '1'
$port = 10849
$existing = Get-NetTCPConnection -LocalPort $port -State Listen -ErrorAction SilentlyContinue
if ($existing) { Write-Host "blender-mcp already listening on $port (PID $($existing.OwningProcess))"; return }
Start-Process -FilePath "$Repo\.venv\Scripts\python.exe" `
  -ArgumentList '-m','uvicorn','blender_mcp.server:asgi_app','--host','127.0.0.1','--port',"$port" `
  -WorkingDirectory $Repo -WindowStyle Hidden
for ($i=0; $i -lt 40; $i++) {
  Start-Sleep -Milliseconds 500
  try { $r = Invoke-WebRequest "http://127.0.0.1:$port/api/v1/health" -UseBasicParsing -TimeoutSec 2; if ($r.StatusCode -eq 200) { Write-Host "blender-mcp healthy: $($r.Content)"; return } } catch {}
}
throw "blender-mcp did not become healthy on port $port"
