# SPDX-License-Identifier: GPL-2.0-or-later
# Fork changes (2026-11-09): prepare a local Windows runtime beside the built EXE.
<#
.SYNOPSIS
Copy Qt plugins/DLLs, MinGW runtimes and bundled Python 3.9 beside CANgaroo.
.EXAMPLE
.\scripts\windows\build-qt.ps1
.\scripts\windows\copy-runtime.ps1
.EXAMPLE
.\scripts\windows\copy-runtime.ps1 -ExePath '.\build\local-run\cangaroo.exe'
#>
param(
    [string]$QtDir = 'E:\Qt\6.12.0\mingw_64',
    [string]$MinGWDir = 'E:\Qt\Tools\mingw1310_64',
    [string]$ExePath = ''
)
$ErrorActionPreference = 'Stop'
$repoDir = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
if ([string]::IsNullOrWhiteSpace($ExePath)) {
    $ExePath = Join-Path $repoDir 'build\qt-mingw-release\bin\cangaroo.exe'
}
$deployQt = Join-Path $QtDir 'bin\windeployqt.exe'
$pythonBin = Join-Path $MinGWDir 'opt\bin'
$pythonLib = Join-Path $MinGWDir 'opt\lib\python3.9'
$compilerDlls = @('libgcc_s_seh-1.dll', 'libstdc++-6.dll', 'libwinpthread-1.dll')
$requiredFiles = @($ExePath, $deployQt, "$pythonBin\libpython3.9.dll", "$pythonLib\os.py")
$requiredFiles += $compilerDlls | ForEach-Object { Join-Path "$MinGWDir\bin" $_ }
foreach ($required in $requiredFiles) {
    if (-not (Test-Path -LiteralPath $required -PathType Leaf)) {
        throw "Missing runtime input: $required. Build the EXE and check Qt/MinGW paths first."
    }
}
$resolvedExe = (Resolve-Path -LiteralPath $ExePath).Path
$binDir = Split-Path -Parent $resolvedExe
$previousPath = $env:PATH
try {
    $env:PATH = "$QtDir\bin;$MinGWDir\bin;$env:SystemRoot\System32;$env:SystemRoot"
    & $deployQt --release --compiler-runtime --no-translations --dir $binDir $resolvedExe
    if ($LASTEXITCODE -ne 0) { throw "windeployqt failed: $LASTEXITCODE" }

    # Optional Python extension dependencies are copied from the same Python kit.
    # Keep the compiler's runtime DLLs authoritative rather than overwriting them
    # with older copies in opt/bin.
    foreach ($dll in Get-ChildItem -LiteralPath $pythonBin -Filter '*.dll' -File) {
        if ($dll.Name -notin $compilerDlls) {
            Copy-Item -LiteralPath $dll.FullName -Destination $binDir -Force
        }
    }
    foreach ($name in $compilerDlls) {
        Copy-Item -LiteralPath (Join-Path "$MinGWDir\bin" $name) -Destination $binDir -Force
    }
    $targetPythonLib = Join-Path $binDir 'lib\python3.9'
    New-Item -ItemType Directory -Force $targetPythonLib | Out-Null
    # Merge only; do not purge destination files or change installed toolchains.
    & "$env:SystemRoot\System32\robocopy.exe" $pythonLib $targetPythonLib /E /R:1 /W:1 /XD __pycache__ /NFL /NDL /NJH /NJS /NP
    if ($LASTEXITCODE -ge 8) { throw "Python standard library copy failed: $LASTEXITCODE" }

    foreach ($relative in @('Qt6Core.dll', 'Qt6Widgets.dll', 'platforms\qwindows.dll',
                            'libpython3.9.dll', 'lib\python3.9\encodings\__init__.py')) {
        if (-not (Test-Path -LiteralPath (Join-Path $binDir $relative) -PathType Leaf)) {
            throw "Runtime output missing: $relative"
        }
    }
    Write-Host "Local runtime ready: $resolvedExe"
    Write-Host 'Keep the entire output directory together. Vendor adapter drivers remain separate.'
    $global:LASTEXITCODE = 0
} finally {
    $env:PATH = $previousPath
}
