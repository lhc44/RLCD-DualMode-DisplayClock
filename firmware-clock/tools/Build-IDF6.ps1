[CmdletBinding()]
param([switch]$SkipReconfigure)
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root
if (-not $env:IDF_PATH) { throw '请先在 ESP-IDF PowerShell 中执行本脚本。' }
function Invoke-Idf([string[]]$Arguments) {
  & idf.py @Arguments
  if ($LASTEXITCODE -ne 0) { throw "idf.py $($Arguments -join ' ') failed: $LASTEXITCODE" }
}
if (-not $SkipReconfigure) { Invoke-Idf @('reconfigure') }
# ESP-DSP 1.7 uses std:: math functions without including <cmath>; IDF 6/Picolibc
# no longer supplies those names transitively. Keep the managed dependency patch idempotent.
$utf8NoBom = [System.Text.UTF8Encoding]::new($false)
$paths = @(
  (Join-Path $root 'managed_components/espressif__esp-dsp/modules/kalman/ekf/common/ekf.cpp'),
  (Join-Path $root 'managed_components/espressif__esp-dsp/modules/kalman/ekf_imu13states/ekf_imu13states.cpp')
)
foreach ($path in $paths) {
  if (-not (Test-Path -LiteralPath $path)) { throw "未找到已解析的 ESP-DSP 源文件: $path" }
  $text = [IO.File]::ReadAllText($path)
  if ($text -notmatch '(?m)^#include <cmath>$') {
    $first = [regex]::Match($text, '(?m)^#include [^\r\n]+$')
    if (-not $first.Success) { throw "未找到 include 插入点: $path" }
    $text = $text.Insert($first.Index + $first.Length, [Environment]::NewLine + '#include <cmath>')
    [IO.File]::WriteAllText($path, $text, $utf8NoBom)
  }
}
Invoke-Idf @('build')
