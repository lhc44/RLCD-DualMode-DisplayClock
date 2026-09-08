param(
    [string]$Output = (Join-Path $PSScriptRoot 'package')
)
$ErrorActionPreference = 'Stop'
$project = Join-Path $PSScriptRoot 'source\RLCDControl.csproj'
$outDir = Join-Path $Output 'net8-x64'
New-Item -ItemType Directory -Force -Path $outDir | Out-Null
# Compact framework-dependent WinForms executable; requires .NET 8 Windows Desktop Runtime.
dotnet publish $project -c Release -r win-x64 --self-contained false -p:PublishSingleFile=true -p:DebugType=None -o $outDir
if ($LASTEXITCODE -ne 0) { throw "dotnet publish failed: $LASTEXITCODE" }
$sourceExe = Join-Path $outDir 'RLCDControl.exe'
$targetExe = Join-Path $outDir 'RLCD-Control-Net8-x64.exe'
if (Test-Path -LiteralPath $targetExe) { Remove-Item -LiteralPath $targetExe -Force }
Rename-Item -LiteralPath $sourceExe -NewName 'RLCD-Control-Net8-x64.exe' -Force
Write-Host "Created: $targetExe"
