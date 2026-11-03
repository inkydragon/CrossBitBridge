# Bridging approaches

Design discussion and candidate paths for adapting legacy libraries. This is not a requirement to implement every backend. See the [example index](../README.md) for current status and each example's own validation record for evidence.

## Selection

- Windows only, C/C++ SDK, frequent small calls, and latency concerns: start with `examples/cpp-ncalrpc/`.
- Existing C# wrappers, with a preference for C# throughout and simple deployment: consider Native AOT + NamedPipeServerStream/NamedPipeClientStream + a small binary request/response protocol. Publish and test win-x86, not just win-x64.
- Standard contracts, multiple client languages, or streaming: consider .proto + gRPC. .NET officially supports named pipe/UDS IPC, and ASP.NET Core supports gRPC Native AOT. Verify the complete combination of target versions and Windows x86.
- Existing marshalable COM interfaces: evaluate out-of-process COM first. Ordinary C/C++ objects still need wrappers; ownership, threads, and callbacks remain the adapter's responsibility. Complex object lifetimes alone do not require COM.
- Ease of debugging and business-level wrappers matter more than small call overhead: an HTTP service is an option.

## C++ and .NET Native AOT

The following comparison concerns retaining Windows RPC ncalrpc. It is architectural analysis, not a measured comparison. The running [C++ example](../examples/cpp-ncalrpc/README.md) does not establish that another path is slower or unsuitable.

| Dimension | Native C++ | .NET Native AOT |
|---|---|---|
| IDL-driven build | Compile and link MIDL stubs directly | MIDL does not emit C# directly; retain a native bridge or add a generator |
| C DLL binding | Vendor headers and function pointers | LibraryImport generates marshaling for build-time-known signatures |
| Dynamic signatures | libffi could be added; not implemented in the C++ example | No Reflection.Emit under AOT; arbitrary signatures still need an engine such as libffi |
| ncalrpc access | Link Rpcrt4 and use SDK declarations | P/Invoke accesses management APIs but does not automatically provide complete NDR stubs |
| Server dispatch | Native Impl_* functions enter the adapter | C++ stubs can call an AOT DLL through a fixed C ABI, adding an ABI/lifetime boundary |
| Memory | Explicit management and review | Managed code can simplify logic; native buffers and RPC allocations still have separate lifetimes |
| Exceptions | Catch RPC SEH inside the client C ABI | Define a native exception boundary; RPC SEH is not automatically an ordinary C# exception |
| Toolchain | SDK/MIDL + MSVC + CMake | Usually retain RPC tools and add .NET SDK/AOT tools |
| Distribution | EXE/DLL with static CRT in the C++ example | Includes required AOT runtime; measure actual size |
| Maintenance | One main ABI/build stack | Useful when C# simplifies business logic, but native RPC and managed interop remain separate layers |

The official Native AOT target table currently lists Windows x86, so lack of x86 support is not grounds for rejecting it. Verify the selected .NET version, NuGet dependencies, and win-x86 publication separately. A .NET SDK is installed locally, but no .NET AOT build has been published for this repository.

## Using .NET IPC instead of ncalrpc

Native AOT + direct named pipes or official gRPC IPC can avoid a C++ RPC layer. Direct named pipes require a message protocol; gRPC provides a standard contract and generated clients. Neither has been benchmarked against ncalrpc here. Verify actual win-x86 publication and calls for the selected SDK and dependencies.

When retaining ncalrpc with substantial existing C# logic, a C++ RPC shell can call a .NET AOT native DLL through fixed, blittable C ABI exports using `UnmanagedCallersOnly`. The AOT DLL can call the vendor library through LibraryImport. This reuses native MIDL stubs instead of hand-writing NDR, but adds an interop boundary.

## Other compiled languages

Rust can access Win32 RPC through windows/windows-sys and reuse native MIDL stubs; API bindings do not automatically generate a complete high-level server from IDL. Zig can reuse native stubs through the C ABI, subject to toolchain verification. Go can build an x86 host and call DLLs, and provides net/rpc, but its default protocol is awkward for a Julia client and standard net/rpc is frozen. Changing languages generally does not remove interface-definition and marshaling work.

## References

- [ASP.NET Core gRPC IPC](https://learn.microsoft.com/en-us/aspnet/core/grpc/interprocess): named pipes/UDS and HTTP/2.
- [ASP.NET Core Native AOT](https://learn.microsoft.com/en-us/aspnet/core/fundamentals/native-aot): AOT support for gRPC.
- [NamedPipeServerStream](https://learn.microsoft.com/en-us/dotnet/api/system.io.pipes.namedpipeserverstream): synchronous/asynchronous streams; does not define an RPC contract automatically.
- [Rust Win32 RPC API](https://microsoft.github.io/windows-docs-rs/doc/windows/Win32/System/Rpc/index.html).
- [Go net/rpc](https://pkg.go.dev/net/rpc): protocol and frozen status.
- [Zig overview](https://ziglang.org/learn/overview/): C interoperability.
- [.NET Native AOT](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/): target support and restrictions.
- [P/Invoke source generation](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/pinvoke-source-generation): LibraryImport marshaling.
- [Native AOT interoperability](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/interop): C exports and type restrictions.

For measurement guidance, see [performance.md](performance.md). For adaptation workflow, see [adaptation.md](adaptation.md).
