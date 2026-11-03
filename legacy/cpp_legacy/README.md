# C++ legacy sample

An independently buildable x86 C API DLL used by the [C++ / ncalrpc bridge](../../bridges/cpp-ncalrpc/README.md). It contains no RPC code.

On Windows with Visual Studio 2022 C++ tools and CMake 3.24+, from this directory:

```powershell
./build.ps1
```

The default is Win32 Release, producing `build/Win32/Release/sample_legacy.dll`. Use `-Configuration Debug` for Debug. Other architectures are rejected. Build products remain in this directory's ignored `build/`.

The unchanged `legacy.h` and `legacy.def` define the exported C API: `legacy_add` and `legacy_scale`, both `__cdecl`. Status values are 0 (success), 1 (integer overflow), and 2 (invalid buffer). Addition checks signed 32-bit overflow; scaling accepts empty arrays.

Build the bridge separately, then pass this DLL's path to its `scripts/test.ps1 -LegacyDll` parameter. The bridge includes this sample's header but neither links nor copies the DLL. Each legacy sample occupies one directory directly under `legacy/`. See the bridge's [validation record](../../bridges/cpp-ncalrpc/docs/validation.md) for executed integration checks and limits.
