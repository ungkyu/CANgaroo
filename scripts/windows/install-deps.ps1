param([string]$PythonExe = 'python')
$ErrorActionPreference = 'Stop'
$repoDir = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$depsDir = Join-Path $repoDir '.deps\python'
$requirements = Join-Path $PSScriptRoot 'requirements.txt'
New-Item -ItemType Directory -Force $depsDir | Out-Null
# pybind11 is header-only; the pip interpreter need not match the linked runtime.
& $PythonExe -m pip install --upgrade --target $depsDir -r $requirements
if ($LASTEXITCODE -ne 0) { throw "Dependency installation failed: $LASTEXITCODE" }
