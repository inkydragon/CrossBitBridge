# C++ / ncalrpc architecture

## Current implementation

The implemented path uses C++ + Windows SDK MIDL + Windows RPC `ncalrpc`. Its communication interface is IDL-first: MIDL generates native C stubs, and DLL calls, SEH, RPC runtime access, and Win32 loading share one native toolchain. Users and AI agents edit ordinary IDL, C++, and tests without a new DSL.

See [bridging approaches](../../../docs/approaches.md) for cross-implementation comparisons, including .NET Native AOT. This document covers the running C++ example.

## Responsibilities

| Layer | Generated | Supplied by a person or AI |
|---|---|---|
| RPC contract | MIDL headers and client/server stubs from IDL | Parameter semantics, lengths, directions, errors, UUID/version |
| x86 DLL adapter | AI can draft it | Target ABI, symbols, ownership, bounds checks, error conversion |
| x64 client | MIDL Wire_* functions | Stable C ABI, SEH handling, language-friendly wrappers |
| Build | CMake + PowerShell + CI | Project-specific include/link settings and real DLL tests |

`rpc/server.cpp` and `rpc/binding.cpp` implement the host and connection infrastructure. `examples/sample/adapter.cpp`, `client.cpp`, and `probe.cpp` are project-specific adapters; they are not inferred automatically from arbitrary IDL. Generated files are named `bridge.gen.h`, `bridge_c.gen.c`, and `bridge_s.gen.c`. The build prepends a `DO NOT EDIT` notice after each MIDL invocation; change the configured IDL and regenerate instead. The stubs retain their `.c` extension because MIDL emits C. MIDL handles marshaling, not verification that a DLL obeys declared buffer lengths.

The independent [legacy sample](../../../legacy/cpp_legacy/README.md) owns `legacy.cpp`, `legacy.h`, and `legacy.def`, with its own Win32 build. This bridge owns IDL, host, adapter, client, probe, and Julia examples. The adapter includes the legacy header through an explicit include directory; no legacy DLL is built, linked, or copied here. Tests receive the separately built DLL using required `-LegacyDll` and pass its absolute path to the host.

## Security and lifetime

The client starts an x86 EXE under the intended user with a session-specific endpoint. It binds directly to the known endpoint without Endpoint Mapper, service/COM registration, or registry changes.

Only ncalrpc is exposed. Clients use WINNT authentication and packet privacy. The server requires local-only, secure-only calls and supplies a DACL for its user's SID through RpcServerRegisterIf3. Endpoint names are not authentication. Other processes belonging to the same user are allowed; this is a same-user template, not a process sandbox.

The host initializes the DLL before listening. There is no remote Shutdown method; the test owns the process and terminates it in `finally`. This validates the main path under the assumption of no resources retained across calls. Production adapters involving device handles, background threads, or persistent memory need orderly shutdown and resource handling.

Resource ownership is scoped: `rpc/resources.h` uses `unique_ptr` deleters for token handles (`unique_token`), typed LocalAlloc memory (`unique_local<T>`), and DLL modules; `rpc_binding` owns each client binding. Raw pointers remain at Win32, MIDL, and C ABI boundaries as borrowed buffers or API output parameters. `std::span` carries the adapter's output length during initialization.

Client calls share `ncal_call`. Its outer frame owns the binding, while `rpc_try` catches RPC SEH in a separate frame with no destructible local objects. This avoids MSVC C2712 and releases the binding after either success or a caught recoverable RPC exception. `RpcExceptionFilter` lets fatal exceptions, such as access violations, continue to an outer handler or terminate the process. Callables in this frame must not own resources that require unwinding across SEH. The host's adapter guard finalizes the DLL after listening returns and calls drain, including early initialization failures after the adapter has started.

## Adaptation limits

The example uses synchronous calls with copied values and buffers. Its mutex serializes DLL entry but does not guarantee a fixed OS thread. Retained pointers, device handles, callbacks, thread affinity, and C++ object lifetimes need dedicated adapters. See the [README](../README.md) for source files and commands and [validation](validation.md) for tested boundaries.

## Primary sources

- [Windows process interoperability](https://learn.microsoft.com/en-us/windows/win32/winprog64/process-interoperability): cross-bitness RPC/COM and DLL-loading restrictions.
- [Choosing an RPC protocol sequence](https://learn.microsoft.com/en-us/windows/win32/rpc/choosing-a-protocol-sequence): ncalrpc for local calls.
- [MIDL /env](https://learn.microsoft.com/en-us/windows/win32/midl/-env): target-specific stub generation.
- [MIDL /prefix](https://learn.microsoft.com/en-us/windows/win32/midl/-prefix): client/server prefixes.
- [RpcServerRegisterIf3](https://learn.microsoft.com/en-us/windows/win32/api/rpcdce/nf-rpcdce-rpcserverregisterif3): interface registration and security descriptors.
