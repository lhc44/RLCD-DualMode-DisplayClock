[CmdletBinding()]
param([switch]$SkipReconfigure)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root
if (-not $env:IDF_PATH) {
  $bundledIdf = 'C:\esp\v6.0.2\esp-idf'
  if (-not (Test-Path -LiteralPath $bundledIdf)) { throw 'ESP-IDF v6.0.2 was not found. Install it, then rerun this script.' }
  $env:IDF_PATH = $bundledIdf
}
if (-not $env:IDF_TOOLS_PATH -and (Test-Path -LiteralPath 'C:\Espressif\tools')) {
  $env:IDF_TOOLS_PATH = 'C:\Espressif\tools'
}
$pythonExe = $null
if ($env:IDF_TOOLS_PATH) {
  $pythonExe = Get-ChildItem -LiteralPath (Join-Path $env:IDF_TOOLS_PATH 'python') -Directory -ErrorAction SilentlyContinue |
    Sort-Object Name -Descending |
    ForEach-Object { Join-Path $_.FullName 'venv/Scripts/python.exe' } |
    Where-Object { Test-Path -LiteralPath $_ } |
    Select-Object -First 1
}
if (-not $pythonExe) { $pythonExe = (Get-Command python -ErrorAction Stop).Source }
if (-not $env:IDF_PYTHON_ENV_PATH) {
  $env:IDF_PYTHON_ENV_PATH = Split-Path -Parent (Split-Path -Parent $pythonExe)
}
if (-not $env:ESP_IDF_VERSION) { $env:ESP_IDF_VERSION = '6.0.2' }
$toolBins = @()
if ($env:IDF_TOOLS_PATH) {
  $xtensaBin = Get-ChildItem -LiteralPath (Join-Path $env:IDF_TOOLS_PATH 'xtensa-esp-elf') -Directory -ErrorAction SilentlyContinue |
    Sort-Object Name -Descending |
    ForEach-Object { Join-Path $_.FullName 'xtensa-esp-elf/bin' } |
    Where-Object { Test-Path -LiteralPath (Join-Path $_ 'xtensa-esp32s3-elf-gcc.exe') } |
    Select-Object -First 1
  $ninjaBin = Get-ChildItem -LiteralPath (Join-Path $env:IDF_TOOLS_PATH 'ninja') -Directory -ErrorAction SilentlyContinue |
    Sort-Object Name -Descending |
    ForEach-Object { Join-Path $_.FullName 'ninja.exe' } |
    Where-Object { Test-Path -LiteralPath $_ } |
    ForEach-Object { Split-Path -Parent $_ } |
    Select-Object -First 1
  $toolBins = @($xtensaBin, $ninjaBin) | Where-Object { $_ }
}
if ($toolBins.Count) { $env:PATH = (($toolBins -join ';') + ';' + $env:PATH) }
# ESP-IDF invokes ccache when it is installed.  Keep its database and temporary
# files under this project instead of a potentially protected user profile path.
$env:CCACHE_DIR = Join-Path $root '.build-cache'
$env:CCACHE_TEMPDIR = Join-Path $root '.build-cache-tmp'
New-Item -ItemType Directory -Force -Path $env:CCACHE_DIR, $env:CCACHE_TEMPDIR | Out-Null
function Invoke-Idf([string[]]$Arguments) {
  # Calling the Python entry point works in both ESP-IDF PowerShell and a
  # standard Windows PowerShell session; invoking idf.py directly is blocked
  # by some Windows file-association policies.
  & $pythonExe (Join-Path $env:IDF_PATH 'tools/idf.py') @Arguments
  if ($LASTEXITCODE -ne 0) { throw ("idf.py " + ($Arguments -join ' ') + " failed: " + $LASTEXITCODE) }
}
if (-not $SkipReconfigure) { Invoke-Idf @('reconfigure') }
$utf8NoBom = [System.Text.UTF8Encoding]::new($false)
$paths = @(
  (Join-Path $root 'managed_components/espressif__esp-dsp/modules/kalman/ekf/common/ekf.cpp'),
  (Join-Path $root 'managed_components/espressif__esp-dsp/modules/kalman/ekf_imu13states/ekf_imu13states.cpp')
)
foreach ($path in $paths) {
  if (-not (Test-Path -LiteralPath $path)) { throw ("Managed ESP-DSP source not found: " + $path) }
  $text = [IO.File]::ReadAllText($path)
  if ($text -notmatch '(?m)^#include <cmath>$') {
    $first = [regex]::Match($text, '(?m)^#include [^\r\n]+$')
    if (-not $first.Success) { throw ("No include insertion point: " + $path) }
    $text = $text.Insert($first.Index + $first.Length, [Environment]::NewLine + '#include <cmath>')
    [IO.File]::WriteAllText($path, $text, $utf8NoBom)
  }
}
Invoke-Idf @('build')
