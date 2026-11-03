# CrossBitBridge

Runnable examples and AI guidance for calling legacy libraries across process bitness.
Copy and adapt an example to your library.

- [Project instructions](AGENTS.md)
- [Approach comparison](docs/approaches.md) · [Adaptation workflow](docs/adaptation.md) · [Performance guidance](docs/performance.md)


## Examples

COM, .NET, and HTTP examples are not implemented yet.

### C++ / ncalrpc

[C++ / ncalrpc example](bridges/cpp-ncalrpc/README.md):
x64 Julia → x64 client → x86 host → C API DLL. Verified locally on Windows.

```powershell
cd bridges/cpp-ncalrpc
../../legacy/cpp_legacy/build.ps1
./scripts/build.ps1
./scripts/test.ps1 -LegacyDll ../../legacy/cpp_legacy/build/Win32/Release/sample_legacy.dll
```

[C++ legacy sample](legacy/cpp_legacy/README.md) builds the x86 DLL independently. `legacy/<case>/` holds library samples; `bridges/<language>-<transport>/` holds RPC hosts, adapters, clients, and tests.

See the [build and test guide](bridges/cpp-ncalrpc/docs/build.md) for prerequisites, custom IDL, and Julia testing.

## License

[MIT](LICENSE) © 2026 Chengyu HAN.
