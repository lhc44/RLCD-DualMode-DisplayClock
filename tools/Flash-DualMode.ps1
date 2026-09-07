[CmdletBinding()]
param([Parameter(Mandatory = $true)][string]$Port)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$release = Join-Path $root 'release'
$python = 'C:\Espressif\tools\python\v6.0.2\venv\Scripts\python.exe'
if (-not (Test-Path -LiteralPath $python)) { throw 'ESP-IDF Python v6.0.2 was not found.' }
$files = @('bootloader.bin','partition-table.bin','ota_data_initial.bin','RLCD-Clock-OTA0.bin','RLCD-USB-Display-OTA1.bin','srmodels.bin')
foreach ($file in $files) { if (-not (Test-Path -LiteralPath (Join-Path $release $file))) { throw ("Missing release artifact: " + $file) } }
& $python -m esptool --chip esp32s3 --port $Port --baud 460800 --before default-reset --after hard-reset write-flash --flash-mode dio --flash-size 16MB --flash-freq 80m `
  0x0 (Join-Path $release 'bootloader.bin') `
  0x8000 (Join-Path $release 'partition-table.bin') `
  0xf000 (Join-Path $release 'ota_data_initial.bin') `
  0x20000 (Join-Path $release 'RLCD-Clock-OTA0.bin') `
  0x6e0000 (Join-Path $release 'RLCD-USB-Display-OTA1.bin') `
  0xfa0000 (Join-Path $release 'srmodels.bin')
if ($LASTEXITCODE -ne 0) { throw ("esptool failed: " + $LASTEXITCODE) }
