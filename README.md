# CrossBitBridge

Runnable examples and AI guidance for calling legacy libraries across process bitness.
Copy and adapt an example to your library.

- [Project instructions](AGENTS.md)
- [Approach comparison](docs/approaches.md) · [Adaptation workflow](docs/adaptation.md) · [Performance guidance](docs/performance.md)


## Examples

COM, .NET, and HTTP examples are not implemented yet.

### C++ / ncalrpc

[C++ / ncalrpc example](examples/cpp-ncalrpc/README.md):
x64 Julia → x64 client → x86 host → C API DLL. Verified locally on Windows.

```powershell
cd examples/cpp-ncalrpc
./scripts/build.ps1
./scripts/test.ps1
```

The example README covers prerequisites, custom IDL, and Julia testing.

## License

[MIT](LICENSE) © 2026 Chengyu HAN.
