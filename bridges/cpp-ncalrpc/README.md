# C++ / ncalrpc MWE

A demo/MWE for users and AI agents to copy and adapt. See the root [AGENTS.md](../../AGENTS.md) for project instructions and [adaptation guide](../../docs/adaptation.md) for workflow guidance. This project is not intended to grow into a general framework.

This Windows-only DLL bridge uses ordinary IDL as its communication contract. MIDL generates stubs, a C++ adapter calls the x86 DLL, an x64 client exposes a stable C ABI, and Julia calls it through existing `ccall` support.

The C++ path has run successfully on local Windows. There is no new DSL, binding macro, general signature inference, or Julia AOT compiler. IDL generates the RPC stubs; business adaptation and the client C ABI remain ordinary source files maintained by the user or AI. See the root [approach comparison](../../docs/approaches.md) for .NET Native AOT maintenance tradeoffs and the local [architecture](docs/architecture.md) for implementation details.

```text
Julia x64 / ncal_probe.exe
       ↓ C ABI
ncal_client.dll (x64, MIDL client stub)
       ↓ ncalrpc / Windows RPC / NDR
ncal_host.exe (x86, MIDL server stub)
       ↓ example adapter / LoadLibraryExW + GetProcAddress
sample_legacy.dll (x86, C++ implementation / exported C API)
```

## Local build and validation

The bridge uses C++20, a stable language mode supported by the CI's VS 2022 toolchain (selected explicitly; MSVC defaults to C++14). Install the Visual Studio 2022 / Build Tools C++ desktop tools, Windows SDK including MIDL, and CMake 3.24+. The full Visual Studio IDE is not required. The script uses the VS 2022 generator to build Win32 and x64; a developer command prompt is not required. Windows 10/11 x64 is recommended.

```powershell
cd T:\CrossBitBridge\bridges\cpp-ncalrpc
../../legacy/cpp_legacy/build.ps1
./scripts/build.ps1
./scripts/test.ps1 -LegacyDll ../../legacy/cpp_legacy/build/Win32/Release/sample_legacy.dll
# Optional: test the real Julia client; standard libraries only.
./scripts/test.ps1 -LegacyDll ../../legacy/cpp_legacy/build/Win32/Release/sample_legacy.dll -Julia julia
```

The [legacy sample](../../legacy/cpp_legacy/README.md) builds separately. This bridge includes its header through an explicit CMake include path, without linking or copying its DLL. `-LegacyDll` is required: the test checks that the file exists and resolves it to an absolute path before starting the x86 host. Relative DLL paths are resolved from the caller's working directory.

The test starts a host on a dedicated session endpoint, makes real RPC calls, and terminates that host in `finally`. It does not affect other host processes. It validates the call boundary; the example has no session resources requiring orderly shutdown.

- `../../legacy/cpp_legacy/build/Win32/Release/sample_legacy.dll`: independently built example x86 DLL.
- `build/Win32/Release/ncal_host.exe`: x86 RPC host.
- `build/x64/Release/ncal_client.dll`: x64 C ABI client.
- `build/x64/Release/ncal_probe.exe`: standalone integration test.
- `build/{Win32,x64}/generated/`: MIDL headers and stubs generated for each target.
- `artifacts/logs/`: host, native client, and optional Julia test logs.

## Supply your own IDL

Generate RPC stubs only:

```powershell
./scripts/build.ps1 -Idl C:\private\device.idl -GenerateOnly
```

To build your interface, also supply ordinary adapter sources. Changing interface names or methods in IDL alone does not implement their behavior:

```powershell
./scripts/build.ps1 `
  -Idl C:\private\device.idl `
  -Interface DeviceRpc_v1_0 `
  -Adapter C:\private\adapter.cpp `
  -Client C:\private\client.cpp `
  -Probe C:\private\probe.cpp
```

`-Interface` combines the IDL interface name and version: `DeviceRpc` with `version(1.0)` becomes `DeviceRpc_v1_0`. MIDL adds `Impl_` to server symbols and `Wire_` to client symbols. Build commands quote paths separately and support spaces. This template tracks one root IDL; after changing an imported IDL, touch the root IDL or regenerate. Import dependencies are not scanned automatically.

Adapter requirements:

1. Use explicit `handle_t` binding parameters and matching interface UUID/version on both sides. Do not use `[object]` COM IDL.
2. Include generated `bridge.h` in `adapter.cpp` and implement `Impl_*`, `adapter_initialize`, and `adapter_finalize`; see `rpc/adapter.h`.
3. Expose the caller-facing C ABI in `client.cpp` and convert recoverable RPC exceptions to transport status codes. Use `RpcExceptionFilter` so fatal process errors are not treated as ordinary RPC failures.
4. Test the generated interface in `probe.cpp`. Add project-specific include/link settings directly to CMake; no additional schema is required.
5. Use fixed-width values, explicitly sized arrays, and synchronous calls. Process pointers, STL objects, and Julia objects must not enter the wire protocol.

The example separates RPC transport status from DLL application status. `Ping` returns the server bitness; `Add` demonstrates overflow errors; `Scale` demonstrates copied arrays up to 1,048,576 elements, including empty arrays. Both IDL and client code bound array sizes. Outputs are initialized before calling the DLL; adapters must still handle the vendor's actual return semantics correctly.

The host loads vendor DLLs by absolute path at runtime, so CI does not need proprietary binaries or import libraries. `vendor/` is ignored. Store private headers and adapter sources according to your own confidentiality requirements.

## CI

The root [`.github/workflows/windows.yml`](../../.github/workflows/windows.yml) builds, tests, and uploads artifacts on a Windows 2022 runner. It installs x64 Julia to verify the example client. The directory-rename commit passed GitHub Actions; see the validation record for evidence. See [validation.md](docs/validation.md) for local Windows evidence.

Copy the example into your own private repository; no Julia package registration is required. Cloud builds can compile wrappers only, with proprietary DLL compatibility tests running locally or on a self-hosted runner.

## Current limits

- No .NET AOT backend, Lua, libffi, AI skill, or semantic generation of business adapters from arbitrary IDL is implemented.
- RPC may dispatch concurrent calls. The example adapter serializes DLL entry with a mutex but does not guarantee a fixed OS thread. Libraries with thread affinity need a dedicated executor.
- RPC accepts local, authenticated connections only. The interface DACL permits the host's current user, including other processes owned by that user. Cross-account rejection has not been tested.
- A DLL crash terminates the host. There is no automatic restart or retry. A timeout does not mean a DLL operation was canceled.
- The client creates a binding per call for simplicity. Connection caching and performance claims are outside this example.
- End users need Windows, prebuilt artifacts, and the target DLL, not SDK/MIDL/MSVC. This build uses the static CRT; vendor DLL dependencies remain vendor-specific.
