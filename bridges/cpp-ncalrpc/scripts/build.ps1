[CmdletBinding()]
param(
    [string]$Idl = "spec/sample.idl",
    [string]$Interface = "NcalRpc_v1_0",
    [string]$Adapter = "examples/sample/adapter.cpp",
    [string]$Client = "examples/sample/client.cpp",
    [string]$Probe = "examples/sample/probe.cpp",
    [ValidateSet("Debug", "Release")][string]$Configuration = "Release",
    [switch]$GenerateOnly
)
$ErrorActionPreference = "Stop"
$root = Split-Path $PSScriptRoot -Parent
function Resolve-Input([string]$Path) {
    if (-not [IO.Path]::IsPathRooted($Path)) { $Path = Join-Path $root $Path }
    return (Resolve-Path -LiteralPath $Path).Path
}
$idlPath = Resolve-Input $Idl
$adapterPath = Resolve-Input $Adapter
$clientPath = Resolve-Input $Client
$probePath = Resolve-Input $Probe
foreach ($arch in @("Win32", "x64")) {
    $build = Join-Path $root "build/$arch"
    & cmake -S $root -B $build -G "Visual Studio 17 2022" -A $arch `
        "-DNCAL_IDL=$idlPath" "-DNCAL_INTERFACE=$Interface" `
        "-DNCAL_ADAPTER=$adapterPath" "-DNCAL_CLIENT=$clientPath" `
        "-DNCAL_PROBE=$probePath"
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed ($arch)" }
    if ($GenerateOnly) {
        & cmake --build $build --config $Configuration --target rpc_codegen
    } else {
        & cmake --build $build --config $Configuration --parallel
    }
    if ($LASTEXITCODE -ne 0) { throw "Build failed ($arch)" }
}
