[CmdletBinding()]
param([Parameter(Mandatory = $true)][string]$Port)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
& (Join-Path $PSScriptRoot 'Build-IDF6.ps1')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Set-Location $root
& idf.py -p $Port flash
exit $LASTEXITCODE