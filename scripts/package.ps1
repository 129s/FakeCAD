param(
    [ValidateSet('Release','Debug')]
    [string[]]$Config = @('Release','Debug'),

    [string]$BuildDir = 'build',

    # 可选：指定生成器（首次配置时），例如 "Visual Studio 17 2022" 或 "Ninja"
    [string]$Generator,

    # 可选：Qt bin 目录（若未在 PATH），例如 C:\Qt\6.6.2\msvc2022_64\bin
    [string]$QtBin,

    # 可选：传给 CMake 的 CMAKE_PREFIX_PATH；若未指定且提供了 QtBin，将自动推导为其父目录
    [string]$CMakePrefixPath,

    # 可选：安装/打包输出根目录
    [string]$DistDir = 'dist',

    # 打包时是否也构建测试（默认否，能显著提速并避免不必要的依赖）
    [switch]$WithTests,

    # 打包模式：dynamic（默认）/ single（静态单EXE）/ both（同时）
    [ValidateSet('dynamic','single','both')]
    [string]$Mode = 'dynamic',

    # 单EXE相关参数
    [string]$SingleBuildDir = 'build-single',
    [string]$SingleCMakePrefixPath,
    [string]$SingleGenerator,
    [string]$UPXPath,
    [string]$UPXArgs = '--best',
    [switch]$SizeOptimize
)

$ErrorActionPreference = 'Stop'

# 规范化到仓库根（脚本位于 <repo>/scripts/ 下）
$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
if (-not [System.IO.Path]::IsPathRooted($BuildDir)) { $BuildDir = Join-Path $RepoRoot $BuildDir }
if (-not [System.IO.Path]::IsPathRooted($DistDir))  { $DistDir  = Join-Path $RepoRoot $DistDir  }
if (-not [System.IO.Path]::IsPathRooted($SingleBuildDir)) { $SingleBuildDir = Join-Path $RepoRoot $SingleBuildDir }
if ($SingleCMakePrefixPath -and -not [System.IO.Path]::IsPathRooted($SingleCMakePrefixPath)) {
    $SingleCMakePrefixPath = (Resolve-Path $SingleCMakePrefixPath).Path
}
$script:SingleGeneratorEffective = $SingleGenerator

function Get-ProjectVersion {
    param([string]$Root = '.')
    $cmakeLists = Join-Path $Root 'CMakeLists.txt'
    if (!(Test-Path $cmakeLists)) { throw "找不到 $cmakeLists" }
    $content = Get-Content $cmakeLists -Raw
    $m = [regex]::Match($content, 'project\s*\([^\)]*VERSION\s*([0-9]+\.[0-9]+\.[0-9]+)')
    if ($m.Success) { return $m.Groups[1].Value }
    return '0.0.0'
}

function Ensure-Configured {
    param([string]$Src = '.', [string]$Bin = $BuildDir)
    if (!(Test-Path (Join-Path $Bin 'CMakeCache.txt'))) {
        Write-Host "[configure] 生成 CMakeCache..."
        $args = @('-S', $RepoRoot, '-B', $Bin)
        if ($Generator) { $args += @('-G', $Generator) }
        if (-not $CMakePrefixPath -and $QtBin) {
            $script:CMakePrefixPath = (Split-Path $QtBin -Parent)
        }
        if ($CMakePrefixPath) { $args += @('-D', "CMAKE_PREFIX_PATH=$CMakePrefixPath") }
        if (-not $WithTests) { $args += @('-D','FAKECAD_BUILD_TESTS=OFF') }
        cmake @args | Write-Host
    }
}

function Find-WinDeployQt {
    $candidates = @('windeployqt.exe','windeployqt6.exe','windeployqt-qt6.exe')
    if ($QtBin) {
        foreach($name in $candidates){
            $p = Join-Path $QtBin $name
            if (Test-Path $p) { return $p }
        }
    }
    foreach($name in $candidates){
        $cmd = Get-Command $name -ErrorAction SilentlyContinue
        if ($cmd) { return $cmd.Source }
    }
    # 尝试从 CMakeCache 中的 CMAKE_PREFIX_PATH 猜测
    $cache = Join-Path $BuildDir 'CMakeCache.txt'
    if (Test-Path $cache) {
        $line = Select-String -Path $cache -Pattern '^CMAKE_PREFIX_PATH:.*=(.*)$' -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($line) {
            $prefix = $line.Matches[0].Groups[1].Value.Split(';')[0]
            foreach($name in $candidates){
                $probe = Join-Path $prefix (Join-Path 'bin' $name)
                if (Test-Path $probe) { return $probe }
            }
        }
    }
    throw '未找到 windeployqt（windeployqt.exe/windeployqt6.exe），请将其加入 PATH 或通过 -QtBin 指定 Qt 的 bin 目录。'
}

function Build-And-Package {
    param(
        [string]$Cfg,
        [string]$Version,
        [string]$WinDeploy
    )

    Write-Host "[build] $Cfg"
    cmake --build $BuildDir --config $Cfg -j | Write-Host

    $prefix = Join-Path $DistDir $Cfg
    if (Test-Path $prefix) { Remove-Item $prefix -Recurse -Force }
    New-Item -ItemType Directory -Force -Path $prefix | Out-Null

    Write-Host "[install] $Cfg -> $prefix"
    cmake --install $BuildDir --config $Cfg --prefix $prefix | Write-Host

    $exe = Join-Path $prefix 'fakecad.exe'
    if (!(Test-Path $exe)) { throw "未找到已安装的可执行文件: $exe" }

    Write-Host "[deploy] windeployqt -> $prefix"
    # 让 windeployqt 自动判断 Debug/Release（兼容 MSYS2 套件仅提供 Release Qt 的情况）
    & $WinDeploy --dir "$prefix" --compiler-runtime "$exe" | Write-Host

    # 写入 qt.conf，确保插件/翻译路径相对当前目录
    $qtConf = Join-Path $prefix 'qt.conf'
    if (!(Test-Path $qtConf)) {
        Set-Content -Path $qtConf -Encoding ASCII -Value "[Paths]`nPlugins=.`nTranslations=translations`n"
    }

    # 额外：若使用 MinGW 工具链，拷贝常见运行时 DLL（在 Qt 的 mingw64\bin 常见）
    if ($QtBin) {
        $mingwDlls = @('libstdc++-6.dll','libgcc_s_seh-1.dll','libwinpthread-1.dll')
        foreach($dll in $mingwDlls){
            $src = Join-Path $QtBin $dll
            if (Test-Path $src) { Copy-Item $src -Destination $prefix -Force }
        }

        # 递归补齐 Qt 第三方依赖（icu/pcre2/zlib/zstd/double-conversion等）
        $objdump = Join-Path $QtBin 'objdump.exe'
        if (Test-Path $objdump) {
            function Get-NeededDllNames([string]$file){
                & $objdump -p $file 2>$null | Select-String 'DLL Name' | ForEach-Object { ($_ -split ':')[1].Trim() } | Where-Object { $_ }
            }
            $systemDllPrefixes = @('KERNEL32','api-ms-','ext-ms-','USER32','GDI32','ADVAPI32','SHELL32','ole32','WS2_32','VERSION','WINMM','USERENV','MPR','NETAPI32','AUTHZ','msvcrt')
            $isSystem = { param($n) ($systemDllPrefixes | ForEach-Object { $n.ToUpper().StartsWith($_) }) -contains $true }

            $queue = New-Object System.Collections.Queue
            $seed = @($exe,'Qt6Core.dll','Qt6Gui.dll','Qt6Widgets.dll','Qt6Network.dll','Qt6Svg.dll') | ForEach-Object { Join-Path $prefix $_ } | Where-Object { Test-Path $_ }
            foreach($s in $seed){ $queue.Enqueue($s) }
            $visited = @{}

            while($queue.Count -gt 0){
                $cur = $queue.Dequeue()
                if ($visited.ContainsKey($cur)) { continue }
                $visited[$cur] = $true
                foreach($name in Get-NeededDllNames $cur){
                    if (& $isSystem $name) { continue }
                    if ($name -match '^Qt6') { continue }
                    $dest = Join-Path $prefix $name
                    if (!(Test-Path $dest)){
                        $src = Join-Path $QtBin $name
                        if (Test-Path $src) {
                            Copy-Item $src $dest -Force
                            $queue.Enqueue($dest)
                        }
                    }
                }
            }
        }

        # 确保 platform 插件存在（有些环境下 windeployqt --debug/--release 会遗漏）
        $platDir = Join-Path $prefix 'platforms'
        if (!(Test-Path (Join-Path $platDir 'qwindows.dll')) -and !(Test-Path (Join-Path $platDir 'qwindowsd.dll'))){
            $qtPrefix = Split-Path $QtBin -Parent
            $qtPlugins = Join-Path $qtPrefix 'lib/qt6/plugins/platforms'
            $fallback = Join-Path $qtPlugins 'qwindows.dll'
            if (Test-Path $fallback) {
                New-Item -ItemType Directory -Force -Path $platDir | Out-Null
                Copy-Item $fallback -Destination $platDir -Force
            }
        }
    }

    $zipName = "FakeCAD-$Version-win64-$Cfg.zip"
    $zipPath = Join-Path $DistDir $zipName
    if (Test-Path $zipPath) { Remove-Item $zipPath -Force }
    Write-Host "[zip] $zipPath"
    Compress-Archive -Path (Join-Path $prefix '*') -DestinationPath $zipPath -Force
}

function Ensure-SingleConfigured {
    param([string]$SinglePrefix)
    if (!(Test-Path (Join-Path $SingleBuildDir 'CMakeCache.txt'))) {
        Write-Host "[single-configure] 生成 CMakeCache..."
        $args = @('-S', $RepoRoot, '-B', $SingleBuildDir, '-D', 'FAKECAD_SINGLE_EXE=ON')
        if ($script:SingleGeneratorEffective) { $args += @('-G', $script:SingleGeneratorEffective) }
        if ($SinglePrefix) { $args += @('-D', "CMAKE_PREFIX_PATH=$SinglePrefix") }
        if ($SizeOptimize) { $args += @('-D','FAKECAD_SIZE_OPTIMIZE=ON') }
        if (-not $WithTests) { $args += @('-D','FAKECAD_BUILD_TESTS=OFF') }
        cmake @args | Write-Host
    }
}

function Build-SingleExe {
    param([string]$Cfg, [string]$Version)

    Write-Host "[single-build] $Cfg"
    cmake --build $SingleBuildDir --config $Cfg -j | Write-Host

    $prefix = Join-Path $SingleBuildDir ("install-$Cfg")
    if (Test-Path $prefix) { Remove-Item $prefix -Recurse -Force }
    New-Item -ItemType Directory -Force -Path $prefix | Out-Null

    Write-Host "[single-install] $Cfg -> $prefix"
    cmake --install $SingleBuildDir --config $Cfg --prefix $prefix | Write-Host

    $exe = Join-Path $prefix 'fakecad.exe'
    if (!(Test-Path $exe)) { throw "未找到可执行文件: $exe" }

    if (!(Test-Path $DistDir)) { New-Item -ItemType Directory -Force -Path $DistDir | Out-Null }
    $outExe = Join-Path $DistDir ("FakeCAD-$Version-win64-Single-$Cfg.exe")
    Copy-Item $exe $outExe -Force

    if ($UPXPath) { Try-UPX -ExePath $outExe }
}

function Try-UPX {
    param([string]$ExePath)
    if (-not $UPXPath) { return }
    if (!(Test-Path $UPXPath)) { throw "未找到 UPX: $UPXPath" }
    Write-Host "[upx] 压缩 $ExePath"
    & $UPXPath $UPXArgs $ExePath | Write-Host
}

function Resolve-SingleGenerator {
    param([string]$SinglePrefix)
    if ($script:SingleGeneratorEffective) { return }
    if ($Generator -and $Generator -match 'Visual Studio') {
        $script:SingleGeneratorEffective = $Generator
        return
    }
    if ($SinglePrefix) {
        $libDir = Join-Path $SinglePrefix 'lib'
        if (Test-Path (Join-Path $libDir 'Qt6Core.lib')) {
            $script:SingleGeneratorEffective = 'Visual Studio 17 2022'
            return
        }
        if (Test-Path (Join-Path $libDir 'libQt6Core.a')) {
            $script:SingleGeneratorEffective = 'Ninja'
            return
        }
    }
    $script:SingleGeneratorEffective = 'Visual Studio 17 2022'
}

# 主流程
$version = Get-ProjectVersion -Root $RepoRoot
if (!(Test-Path $DistDir)) { New-Item -ItemType Directory -Force -Path $DistDir | Out-Null }

$doDynamic = $Mode -ne 'single'
$doSingle  = $Mode -ne 'dynamic'

if ($doDynamic) {
    Ensure-Configured -Src $RepoRoot -Bin $BuildDir
    $windeploy = Find-WinDeployQt
    foreach ($cfg in $Config) {
        Build-And-Package -Cfg $cfg -Version $version -WinDeploy $windeploy
    }
}

if ($doSingle) {
    if (-not $UPXPath) {
        $autoUpx = Get-Command upx -ErrorAction SilentlyContinue
        if ($autoUpx) { $UPXPath = $autoUpx.Source }
    }

    $singlePrefix = if ($SingleCMakePrefixPath) { $SingleCMakePrefixPath } else { $CMakePrefixPath }
    if (-not $singlePrefix) { throw '单EXE模式需要指定静态 Qt 前缀（-SingleCMakePrefixPath 或 -CMakePrefixPath）。' }

    Resolve-SingleGenerator -SinglePrefix $singlePrefix
    Ensure-SingleConfigured -SinglePrefix $singlePrefix

    foreach ($cfg in $Config) {
        Build-SingleExe -Cfg $cfg -Version $version
    }
}

Write-Host "完成：打包输出位于 $DistDir"
