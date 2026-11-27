$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot
python -m pip install -r backend_sql/requirements.txt -q
Start-Process -FilePath "python" -ArgumentList "-m","uvicorn","app:app","--app-dir","backend_sql","--host","127.0.0.1","--port","8000" -WindowStyle Hidden
Start-Sleep -Seconds 2
Start-Process "http://127.0.0.1:8000"
Write-Host "Server running at http://127.0.0.1:8000"
