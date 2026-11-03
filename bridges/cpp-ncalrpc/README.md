# C++ / ncalrpc example

Call an x86 C API DLL from x64 Julia through a local Windows RPC bridge:

```text
Julia / native probe (x64) → client DLL (x64) → ncalrpc → host (x86) → legacy DLL (x86)
```

- [Environment, build, and tests](docs/build.md)
- [Architecture and Win32 API references](docs/architecture.md)
- [Adapting a legacy library](../../docs/adaptation.md)

## Source files

| Path | Purpose |
|---|---|
| `spec/sample.idl` | RPC interface: ping, integer addition, and array scaling |
| `rpc/` | Host, binding, exception handling, and resource ownership |
| `examples/sample/` | DLL adapter, client C ABI, and native test |
| `examples/julia/smoke.jl` | Julia wrappers and smoke test |
| [Legacy sample](../../legacy/cpp_legacy/README.md) | Independently built x86 DLL |

MIDL generates `bridge.gen.h`, `bridge_c.gen.c`, and `bridge_s.gen.c`. Edit the IDL and regenerate; do not edit generated files. Adapter and client behavior are implemented by hand.

## Limits

Calls are synchronous and copy data across processes. The adapter serializes DLL calls but does not guarantee a fixed thread. Callbacks, retained pointers, and persistent resources require additional adapter code and shutdown handling.

RPC is local and authenticated, with an interface ACL for the host's user; cross-user rejection has not been tested. A DLL crash terminates the host. Automatic restart, retry, and cancellation are not implemented.
