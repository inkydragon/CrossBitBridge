[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")][string]$Configuration = "Release"
)
$ErrorActionPreference = "Stop"
$build = Join-Path $PSScriptRoot "build/Win32"
& cmake -S $PSScriptRoot -B $build -G "Visual Studio 17 2022" -A Win32
if ($LASTEXITCODE -ne 0) { throw "Legacy CMake configure failed" }
& cmake --build $build --config $Configuration --parallel
if ($LASTEXITCODE -ne 0) { throw "Legacy build failed" }
