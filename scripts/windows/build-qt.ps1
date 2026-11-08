# SPDX-License-Identifier: GPL-2.0-or-later
# Fork changes (2026-11-08): Windows build or CAN/Tx Generator test integration.
# See docs/windows-development_eng.md for the change record.
param(
    [string]$QtDir = 'E:\Qt\6.12.0\mingw_64',
    [string]$MinGWDir = 'E:\Qt\Tools\mingw1310_64',
    [int]$Jobs = 8
)
$ErrorActionPreference = 'Stop'
$repoDir = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$depsDir = Join-Path $repoDir '.deps\python'
$buildDir = Join-Path $repoDir 'build\qt-mingw-release\src'
$qmake = Join-Path $QtDir 'bin\qmake.exe'
$make = Join-Path $MinGWDir 'bin\mingw32-make.exe'
$pythonInclude = Join-Path $MinGWDir 'opt\include\python3.9'
$pythonLibrary = Join-Path $MinGWDir 'opt\lib\libpython3.9.dll.a'
$pybindInclude = Join-Path $depsDir 'pybind11\include'
foreach ($required in @($qmake, $make, "$pythonInclude\Python.h", $pythonLibrary, "$pybindInclude\pybind11\embed.h")) {
    if (-not (Test-Path -LiteralPath $required)) {
        throw "Missing dependency: $required. Install Qt/MinGW and run scripts/windows/install-deps.ps1."
    }
}
if ($Jobs -lt 1) { throw 'Jobs must be at least 1.' }
$previousPath = $env:PATH
$previousPythonPath = $env:PYTHONPATH
New-Item -ItemType Directory -Force $buildDir | Out-Null
Push-Location $buildDir
try {
    # Exclude Cygwin utilities from the build process.
    $env:PATH = "$QtDir\bin;$MinGWDir\bin;$MinGWDir\opt\bin;$env:SystemRoot\System32;$env:SystemRoot"
    $env:PYTHONPATH = $depsDir
    $qmakeArgs = @(
        (Join-Path $repoDir 'src\src.pro'), 'CONFIG+=release', 'CONFIG+=c++20', 'CONFIG-=debug_and_release',
        '-after', 'CONFIG-=link_pkgconfig', 'PKGCONFIG=',
        "INCLUDEPATH+=$($pythonInclude.Replace('\', '/'))",
        "INCLUDEPATH+=$($pybindInclude.Replace('\', '/'))",
        "LIBS+=$($pythonLibrary.Replace('\', '/'))", 'DEFINES+=STRSAFE_NO_DEPRECATE'
    )
    & $qmake @qmakeArgs
    if ($LASTEXITCODE -ne 0) { throw "qmake failed: $LASTEXITCODE" }
    & $make "-j$Jobs"
    if ($LASTEXITCODE -ne 0) { throw "Build failed: $LASTEXITCODE" }
} finally {
    Pop-Location
    $env:PATH = $previousPath
    $env:PYTHONPATH = $previousPythonPath
}
