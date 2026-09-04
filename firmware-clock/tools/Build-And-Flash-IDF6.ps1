[CmdletBinding()]
param([Parameter(Mandatory = $true)][string]$Port)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
& (Join-Path $PSScriptRoot 'Build-IDF6.ps1')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Set-Location $root
$pythonExe = Get-ChildItem -LiteralPath (Join-Path $env:IDF_TOOLS_PATH 'python') -Directory -ErrorAction Stop |
  Sort-Object Name -Descending |
  ForEach-Object { Join-Path $_.FullName 'venv/Scripts/python.exe' } |
  Where-Object { Test-Path -LiteralPath $_ } |
  Select-Object -First 1
if (-not $pythonExe) { throw 'ESP-IDF Python environment was not found.' }
& $pythonExe (Join-Path $env:IDF_PATH 'tools/idf.py') '-p' $Port 'flash'
exit $LASTEXITCODE
