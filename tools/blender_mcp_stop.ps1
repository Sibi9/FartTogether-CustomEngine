# Stop the local Blender MCP HTTP backend on port 10849.
$c = Get-NetTCPConnection -LocalPort 10849 -State Listen -ErrorAction SilentlyContinue
if ($c) { Stop-Process -Id $c.OwningProcess -Force; Write-Host "stopped PID $($c.OwningProcess)" } else { Write-Host "nothing listening on 10849" }
