# Local validation record

Date: 2026-11-03. Tests ran on Windows x64, invoking Windows PowerShell from WSL; these were not Linux model tests.

## Project rename

The repository was subsequently renamed from `T:\NcalRpc` to `T:\CrossBitBridge`. Commands and output below retain their historical paths. The original layout evidence remains in `artifacts/previous-layout/`; the later pre-rename build and logs are now in `artifacts/before-project-rename/`. Current build commands are in the example README. Post-rename validation is appended below.

## Directory migration

The initial commands below ran in the old root layout. Products remain in `artifacts/previous-layout/build/`, and old logs in `artifacts/previous-layout/logs/`. Post-migration validation is appended below.

## Environment

- Windows 10.0.26300; the system command reported full version 10.0.26300.9711.
- Visual Studio 2022, MSVC 19.44.35229.0, Windows SDK 10.0.26100.0.
- CMake 4.4.4, Windows PowerShell 5.1.
- Optional Julia client: Julia 1.15.0-DEV.341 / x86_64.

## Commands

```powershell
./scripts/build.ps1 -Idl 'T:\NcalRpc\artifacts\spec with spaces\alternate.idl' -GenerateOnly
./scripts/build.ps1
./scripts/test.ps1 -Julia T:\julia-libm-review-20261103-003936\win64\bin\julia.exe
```

The first IDL was a copy of sample.idl to test external paths and spaces. The default root IDL was then restored. Builds exited 0 and generated win32/x64 stubs. The copy and complete build transcript are retained under the ignored artifacts directory.

## Results

- The x86 host and example DLL have PE machine 0x014c.
- The x64 client DLL and probe have PE machine 0x8664.
- Ping returned 32 from the RPC peer; static assertions and PE inspection confirmed an x64 caller.
- 19 + 23 returned 42. INT32_MAX + 1 returned transport status 0, application status 1, and an initialized result of 0.
- Array copying and scaling handled positive, negative, zero, and fractional values correctly.
- Empty arrays and 1,048,576-element arrays succeeded.
- Oversized lengths and null input with a nonzero count were rejected by the client. Server NDR range checks were not separately tested with malicious handcrafted RPC messages.
- A nonexistent endpoint returned a transport error; no exception escaped the client C ABI.
- Loading an x64 DLL into the x86 host failed with Win32 error 193 and process exit code 3. This negative case is included in test.ps1.
- Real x64 Julia called the client DLL through existing ccall support; scalar and array cases passed.

Successful output:

```text
PASS: x64 client -> ncalrpc -> x86 DLL; scalar, overflow, arrays, empty, bounds, missing endpoint
PASS: Julia x64 -> client DLL -> ncalrpc -> x86 C++ DLL
Julia 1.15.0-DEV.341 / x86_64
READY endpoint=NcalRpc-<session-id> bits=32
```

Pre-migration logs are now in `artifacts/previous-layout/logs/`: `build.txt`, `probe.txt`, `julia.txt`, `host.stdout.txt`, `host.stderr.txt`, and `wrong-architecture.txt`.

## Not verified

- GitHub Actions has not run; only its workflow file is provided.
- No .NET Native AOT implementation was built or measured.
- No compatibility claim is made for real proprietary DLLs, C++ ABI, callbacks, or fixed-thread requirements.
- No performance benchmark was run; no minimum-latency claim is made.
- ACL rejection has not been tested with a second Windows user. Successful calls used the current user's authenticated connection.
- The C++ path does not compile arbitrary Julia wrapper functions into standalone native hosts.

## Post-migration validation (2026-11-03)

The example root is now `T:\NcalRpc\examples\cpp-ncalrpc`. Old CMake caches were not reused; both Win32 and x64 were generated and built from scratch in the new directory.

From the repository root:

```powershell
./examples/cpp-ncalrpc/scripts/build.ps1
./examples/cpp-ncalrpc/scripts/test.ps1 -Julia T:\julia-libm-review-20261103-003936\win64\bin\julia.exe
```

Both commands exited 0. The native probe and Julia smoke test passed, covering rejection of an incorrectly sized DLL architecture, scalars, overflow, empty and million-element arrays, input bounds, and a nonexistent endpoint. New test logs are in the example's `artifacts/logs/`.

The root CI working directory/artifact paths, Markdown relative links, and generated-file ignore rules were also checked. GitHub Actions was not run.

## Post-rename validation (2026-11-03)

The repository now resides at `T:\CrossBitBridge`, with Git metadata retained. README commands and the CMake project name were updated. The example's `NcalRpc` IDL interface and session endpoint prefix remain example-local names.

From the new repository root, both commands exited 0:

```powershell
./examples/cpp-ncalrpc/scripts/build.ps1
./examples/cpp-ncalrpc/scripts/test.ps1 -Julia T:\julia-libm-review-20261103-003936\win64\bin\julia.exe
```

Both architectures were rebuilt from fresh CMake directories. The native probe, wrong-architecture rejection, and real Julia client tests passed. Current logs are in the example's `artifacts/logs/`. Markdown relative links were checked; GitHub Actions was not run.

## Verification before the example commit (2026-11-03)

Executed on Windows x64 with PowerShell 7.6.9, matching the workflow's PowerShell Core shell. CMake 4.4.4, MSVC 19.44.35229.0, and Windows SDK 10.0.26100.0 were used.

From the example directory, clean both target builds, generate from an alternate IDL path containing spaces, restore the default IDL, rebuild, and test:

```powershell
cmake --build build/Win32 --config Release --target clean
cmake --build build/x64 --config Release --target clean
New-Item -ItemType Directory -Force 'artifacts/spec with spaces'
Copy-Item spec/sample.idl 'artifacts/spec with spaces/alternate.idl'
./scripts/build.ps1 -Idl 'artifacts/spec with spaces/alternate.idl' -GenerateOnly
./scripts/build.ps1
./scripts/test.ps1 -Julia T:\julia-libm-review-20261103-003936\win64\bin\julia.exe
```

All project build and test steps passed. The native probe and real Julia client passed, including the x86 server bitness check, scalar results, integer overflow, copied arrays, empty arrays, the maximum supported array size, invalid inputs, and an unavailable endpoint. The wrong-architecture DLL test returned host exit 3 / Win32 error 193. PE inspection reconfirmed x86 for the DLL/host and x64 for the client/probe.

An additional `versioninfo(verbose=true)` command failed after the passing project tests because this machine's Julia package cache targeted incompatible CPU features. The diagnostic was rerun successfully with `--compiled-modules=no --pkgimages=no`; it reported Julia 1.15.0-DEV.341, commit 985bc50ab5, x86_64 Windows. It also emitted StyledStrings import warnings. This workaround applied only to diagnostics; the Julia RPC smoke test passed with its normal options. No Julia environment or project source was changed to address the cache issue.

Local evidence is retained in ignored `artifacts/precommit/transcript.txt` (including the initial diagnostic failure), `artifacts/precommit/julia-versioninfo.txt`, `artifacts/precommit/pe-architecture.txt`, and `artifacts/logs/`. Markdown links and workflow YAML were checked. GitHub Actions itself has not run; ACL rejection with a second user and performance remain unverified.
