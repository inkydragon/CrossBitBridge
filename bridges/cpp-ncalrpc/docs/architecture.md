# Architecture

The x86 host loads the legacy DLL by absolute path. The x64 client exposes a C ABI to Julia. MIDL stubs marshal calls over `ncalrpc`; the test runner supplies the same endpoint name to host and client. Endpoint names identify the listener; authentication and the interface ACL control access.

## Adapter contract

- Use explicit `handle_t` bindings, matching UUID/version, fixed-width values, and sized arrays. Process pointers and C++ objects stay out of the wire format.
- Implement `Impl_*` from `bridge.gen.h` and the lifecycle hooks in [`rpc/adapter.h`](../rpc/adapter.h). Initialization borrows a `const std::filesystem::path&`. If listening returns normally, the host drains RPC calls before finalization; the test runner forcibly terminates the host without running adapter cleanup.
- Client functions return transport status separately from the DLL's application status. Endpoint strings are borrowed, null-terminated UTF-16. Caller buffers remain valid until return.
- `ncal_call` owns the binding outside the SEH frame. Keep destructible resources outside `rpc_try`; `RpcExceptionFilter` converts recoverable exceptions to status codes and lets fatal exceptions propagate.

The sample bounds arrays to 1,048,576 elements and initializes outputs before calling the DLL. Each client call creates a binding. Library-specific ABI, ownership, and threading requirements belong in the adapter.

## Windows API references

- [ncalrpc](https://learn.microsoft.com/en-us/windows/win32/midl/ncalrpc): local communication and endpoint naming.
- [LoadLibraryExW](https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibraryexw): DLL loading and dependency search flags.
- [RpcServerRegisterIf3](https://learn.microsoft.com/en-us/windows/win32/api/rpcdce/nf-rpcdce-rpcserverregisterif3): interface registration and access control.
- [RpcExceptionFilter](https://learn.microsoft.com/en-us/windows/win32/api/rpcdce/nf-rpcdce-rpcexceptionfilter): RPC exception filtering.
