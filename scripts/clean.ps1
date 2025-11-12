param(
  # 清理构建目录（匹配 build, build-* 等）
  [switch]$Build,
  # 清理发布产物目录 dist
  [switch]$Dist,
  # 一键全部清理（构建+发布）
  [switch]$All,
  # 仅预览将要删除的内容
  [switch]$DryRun
)

$ErrorActionPreference = 'Stop'

if (-not ($Build -or $Dist -or $All)) { $All = $true }

# 规范化到仓库根（脚本位于 <repo>/scripts/ 下）
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$targets = @()

if ($All -or $Build) {
  $targets += (Get-ChildItem -Path $root -Directory -Filter 'build*' -ErrorAction SilentlyContinue)
  $pkg = Join-Path $root '_packages'
  if (Test-Path $pkg) { $targets += (Get-Item $pkg) }
}
if ($All -or $Dist) {
  $d = Join-Path $root 'dist'
  if (Test-Path $d) { $targets += (Get-Item $d) }
}

if ($targets.Count -eq 0) {
  Write-Host '没有可清理的目录。'
  exit 0
}

Write-Host '[clean] 将处理以下目录：'
$targets | ForEach-Object { Write-Host ' - ' $_.FullName }

if ($DryRun) { Write-Host '[clean] DryRun 模式，仅展示不删除。'; exit 0 }

foreach($t in $targets){
  try {
    Remove-Item $t.FullName -Recurse -Force -ErrorAction Stop
    Write-Host "[clean] 已删除: $($t.FullName)"
  } catch {
    Write-Warning "删除失败: $($t.FullName) -> $($_.Exception.Message)"
  }
}

Write-Host '[clean] 完成'
