[CmdletBinding()]
param([switch]$SkipReconfigure)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root
if (-not $env:IDF_PATH) { throw 'Run this script from an ESP-IDF PowerShell session.' }
function Invoke-Idf([string[]]$Arguments) {
  & idf.py @Arguments
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