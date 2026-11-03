[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")][string]$Configuration = "Release",
    [string]$Julia = ""
)
$ErrorActionPreference = "Stop"
$root = Split-Path $PSScriptRoot -Parent
$hostExe = Join-Path $root "build/Win32/$Configuration/ncal_host.exe"
$dll = Join-Path $root "build/Win32/$Configuration/sample_legacy.dll"
$probe = Join-Path $root "build/x64/$Configuration/ncal_probe.exe"
$logs = Join-Path $root "artifacts/logs"
New-Item -ItemType Directory -Force $logs | Out-Null
# Negative witness: an x86 host must reject an x64 DLL.
$wrongDll = Join-Path $root "build/x64/$Configuration/ncal_client.dll"
$negative = Start-Process -FilePath $hostExe `
    -ArgumentList @("NcalRpc-negative-$([Guid]::NewGuid().ToString('N'))", ('"' + $wrongDll + '"')) `
    -Wait -PassThru -RedirectStandardError "$logs/wrong-architecture.txt"
if ($negative.ExitCode -ne 3) { throw "Expected DLL initialization failure, got $($negative.ExitCode)" }
"PASS: x86 host rejects x64 DLL; exit=3" | Add-Content "$logs/wrong-architecture.txt"
$endpoint = "NcalRpc-$([Guid]::NewGuid().ToString('N'))"
$server = $null
try {
    # Quote the DLL path explicitly for Start-Process on Windows PowerShell 5.1.
    $server = Start-Process -FilePath $hostExe -ArgumentList @($endpoint, ('"' + $dll + '"')) `
        -PassThru -RedirectStandardOutput "$logs/host.stdout.txt" -RedirectStandardError "$logs/host.stderr.txt"
    & $probe $endpoint 2>&1 | Tee-Object -FilePath "$logs/probe.txt"
    $probeExit = $LASTEXITCODE
    if ($probeExit -ne 0) { throw "Probe failed: $probeExit" }
    if ($server.HasExited) { throw "Host exited unexpectedly: $($server.ExitCode)" }
    if ($Julia) {
        & $Julia --startup-file=no (Join-Path $root "examples/julia/smoke.jl") `
            (Join-Path $root "build/x64/$Configuration/ncal_client.dll") $endpoint 2>&1 |
            Tee-Object -FilePath "$logs/julia.txt"
        if ($LASTEXITCODE -ne 0) { throw "Julia smoke test failed" }
    }
    "PASS endpoint=$endpoint" | Set-Content "$logs/result.txt"
} finally {
    if ($server -and -not $server.HasExited) { Stop-Process -Id $server.Id -Force }
    Get-Content "$logs/host.stdout.txt", "$logs/host.stderr.txt" -ErrorAction SilentlyContinue
}
