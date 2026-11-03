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

## Stage 1: bridges directory (2026-11-03)

Renamed the top-level `examples/` to `bridges/`. Historical commands above are unchanged. Old ignored products and logs were archived at repository-local `migration-archive-stage1/` (locally excluded from Git); no old CMake cache was reused.

From `T:\CrossBitBridge\bridges\cpp-ncalrpc`, Windows PowerShell ran:

```powershell
./scripts/build.ps1 -Idl 'T:\CrossBitBridge\bridges\cpp-ncalrpc\artifacts\spec with spaces\alternate.idl' -GenerateOnly
./scripts/build.ps1
./scripts/test.ps1 -Julia T:\julia-libm-review-20261103-003936\win64\bin\julia.exe
```

All commands completed successfully (PowerShell process exit 0). Fresh Win32/x64 generation, default-IDL rebuild, native scalar/overflow/empty/maximum-array/input-bound/missing-endpoint probes, x86 rejection of the x64 DLL (exit 3), and real x64 Julia smoke tests passed. Transcript: `artifacts/stage1.txt`; test logs: `artifacts/logs/`. Markdown links, current top-level path references, ignored build/log directories, and `git diff --check` passed. CI is checked after pushing this commit; earlier CI statements above describe historical validation only.

## Stage 2: independent legacy sample (2026-11-03)

Stage 1 commit `fadb51548eabd399c076e198aa856ab36fa8dd8a` passed [Windows CI](https://github.com/inkydragon/CrossBitBridge/actions/runs/41613414304) before this stage began. Its fresh build and logs were archived at locally ignored `migration-archive-stage2/`, including the stage 1 transcript. Stage 2 used fresh CMake directories again.

From `T:\CrossBitBridge`, Windows PowerShell executed:

```powershell
./legacy/cpp_legacy/build.ps1
cd bridges/cpp-ncalrpc
Copy-Item spec/sample.idl 'artifacts/spec with spaces/alternate.idl'
./scripts/build.ps1 -Idl 'T:\CrossBitBridge\bridges\cpp-ncalrpc\artifacts\spec with spaces\alternate.idl' -GenerateOnly
./scripts/build.ps1
./scripts/test.ps1 -LegacyDll ../../legacy/cpp_legacy/build/Win32/Release/sample_legacy.dll -Julia T:\julia-libm-review-20261103-003936\win64\bin\julia.exe
```

All builds and tests passed; the PowerShell process exited 0. Transcript: `artifacts/stage2.txt`. The independently built Win32 Release DLL was used explicitly. A recursive check found no `sample_legacy.dll` in the bridge's fresh build directory. The three moved legacy source files are byte-identical to stage 1, retaining names, exports, calling convention, and behavior.

The native probe passed scalar, integer overflow, copied arrays, empty arrays, maximum array size, invalid input, and missing endpoint checks. The x86 host rejected the x64 client DLL with exit 3. Real Windows Julia 1.15.0-DEV.341 / x86_64 passed its scalar and array smoke test. Logs remain under `artifacts/logs/`.

Additional invocation from the repository root passed using a DLL copied to an external path with spaces:

```powershell
./bridges/cpp-ncalrpc/scripts/test.ps1 -LegacyDll 'bridges/cpp-ncalrpc/artifacts/external dll with spaces/sample_legacy.dll'
```

That process exited 0 (`artifacts/external-dll.txt`). This reran the native and wrong-architecture checks using a relative path resolved from outside the bridge directory. Missing `-LegacyDll` under noninteractive PowerShell and a nonexistent DLL both exited 1; diagnostics are in `artifacts/missing-parameter.txt` and `artifacts/missing-dll.txt`. Markdown links, stale current path/option references, workflow paths, build/log ignore rules, and `git diff --check` passed. Stage 2 CI is checked after pushing this commit. The unverified security, compatibility, and performance boundaries above still apply.

## C++20 and resource ownership refactor (2026-11-03)

Reviewed manual cleanup in the binding, user security descriptor, and DLL adapter. Token and SID resources could leak if allocation threw between acquisition and manual release; these now have scoped owners. DLL exports are resolved before publishing adapter state, and the first failed export lookup retains its error code. Windows absolute-path classification replaces manual indexing. The public C ABI and IDL remain unchanged; empty endpoints now return `ERROR_INVALID_PARAMETER` explicitly.

CMake explicitly selects C++20, a stable mode supported by VS 2022, rather than relying on MSVC's default C++14 mode or a preview mode. Generated Win32/x64 projects were checked for `stdcpp20`. See Microsoft's [language mode documentation](https://learn.microsoft.com/en-us/cpp/build/reference/std-specify-language-standard-version).

Executed locally on Windows through Windows PowerShell, with VS 2022 / MSVC and SDK 10.0.26100.0:

```powershell
./scripts/build.ps1 -Configuration Debug
./scripts/test.ps1 -Configuration Debug -LegacyDll ../../legacy/cpp_legacy/build/Win32/Release/sample_legacy.dll
./scripts/build.ps1
./scripts/test.ps1 -LegacyDll ../../legacy/cpp_legacy/build/Win32/Release/sample_legacy.dll -Julia T:\julia-libm-review-20261103-003936\win64\bin\julia.exe
```

Both configurations built and passed native cross-bitness tests, including the wrong-architecture rejection. Debug also verifies that the SEH helper compiles without relying on optimization. Release passed the real x64 Julia smoke test. The probe now checks null output arguments, an empty endpoint, initialized scalar/status outputs after unavailable-endpoint errors, and successful calls after 16 rounds of failed ping/add/scale calls. These tests exercise cleanup paths; no resource-leak measurement or allocation-failure injection was performed.

Additional Release host checks rejected relative, drive-relative (`C:relative.dll`), and root-relative (`\relative.dll`) paths with exit 3. Loading the x86 host executable as a same-architecture module without vendor exports returned error 127 / exit 3. An empty argument was omitted by Windows PowerShell and reached the usage check (exit 2), so it does not establish empty-string adapter behavior.

Local transcripts and the additional-check script are retained in ignored `artifacts/refactor/debug.txt`, `release.txt`, and `validate.ps1`. `git diff --check` passed. GitHub Actions was not run for this refactor. Cross-user ACL rejection, allocation failures, malicious NDR messages, vendor crashes, and performance remain unverified.

## Review fixes without WinRT (2026-11-03)

Kept standard C++ RAII and Windows SDK APIs; no WinRT or new dependency was added. The null-invalid token owner is named `unique_token`, and `unique_local<T>` retains the allocated element type. Security setup failures report the failed operation and Win32 error. `Impl_Scale` once again clears valid output before rejecting null input.

`rpc_try` now uses `RpcExceptionFilter`. The tracked native probe verifies both a recoverable `RpcRaiseException(RPC_S_SERVER_UNAVAILABLE)` and a synthetic `RaiseException(EXCEPTION_ACCESS_VIOLATION)` caught by an outer SEH handler. This tests exception filtering without performing an invalid memory access. Debug and Release Win32/x64 builds and native integration tests passed; Release also passed the real x64 Julia smoke test using the commands in the preceding section. Transcripts: ignored `artifacts/refactor/review-fixes-debug.txt` and `review-fixes-release.txt`.

A local direct adapter regression test confirmed null input with a two-element valid output returns application status 2 and changes `[123,456]` to `[0,0]`; null output also returns 2. An isolated test replaced only `OpenProcessToken` with a failure returning `ERROR_ACCESS_DENIED` and confirmed a null descriptor plus `Security setup: OpenProcessToken failed: 5`. Test sources are in ignored `artifacts/review/`; this fault injection is not a production hook. It does not verify all security failure branches.

The first diagnostic invocation was interrupted by Windows PowerShell treating the expected native stderr message as a terminating `NativeCommandError`. The test was rerun using `Start-Process -RedirectStandardError` and passed; transcript: `artifacts/refactor/review-fixes-security.txt`. The project builds, native/Julia tests, and direct output test had already passed before that shell-level interruption. `git diff --check` passed. Remote CI was not run.

## Generated-file naming and notices (2026-11-03)

MIDL now writes `bridge.gen.h`, `bridge_c.gen.c`, and `bridge_s.gen.c`. The build runs `scripts/mark-generated.cmake` after MIDL and tracks that script as a generation dependency. Every output begins with:

```c
/* Generated by MIDL. DO NOT EDIT. Edit the IDL and rerun scripts/build.ps1. */
```

Existing generated directories were moved into ignored `artifacts/idl-naming/{Win32,x64}-before/` before validation. Starting with no generated directory in either build, generated both architectures from `artifacts/spec with spaces/alternate.idl` using `./scripts/build.ps1 -Idl 'artifacts/spec with spaces/alternate.idl' -GenerateOnly`, then restored the default IDL with `./scripts/build.ps1`. Release builds, native cross-bitness tests, wrong-architecture rejection, and the real x64 Julia smoke test all passed. Transcript: `artifacts/idl-naming/build-test.txt`.

For each architecture, checked that the generated directory contains exactly the three `.gen` files and that each begins with the notice. Running the marking script again preserved each file's SHA-256 hash. A subsequent incremental build passed without rerunning MIDL or recompiling. Check script and transcript: ignored `artifacts/idl-naming/check.ps1` and `check.txt`. Current source and build references use the generated names; `git diff --check` passed. GitHub Actions was not run.

## Typed DLL path (2026-11-03)

Changed the internal adapter initialization parameter from a borrowed `const wchar_t*` to `const std::filesystem::path&`. The host constructs the path once; the adapter validates it and passes `path.c_str()` to `LoadLibraryExW`. Custom C++ adapters need the updated signature in `rpc/adapter.h`. The exported client C ABI is unchanged. Its endpoint remains a borrowed null-terminated UTF-16 string; no string allocation or exception path was added to the client. Borrowing and lifetime requirements are documented in the client header and binding interface.

Release Win32/x64 builds, native cross-bitness integration tests, wrong-architecture rejection, and the real x64 Julia smoke test passed. Transcript: ignored `artifacts/refactor/path-type.txt`. Additional host checks rejected relative, drive-relative, and root-relative paths with exit 3; native integration tests passed using the legacy DLL copied to an absolute path containing spaces. Script and transcript: ignored `artifacts/refactor/check-path.ps1` and `path-inputs.txt`. `git diff --check` passed. Debug and remote CI were not rerun for this change.
