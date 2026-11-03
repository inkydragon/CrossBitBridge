# Bridging approaches

Use these sources and examples to investigate a bridge for a particular library. They do not rank languages or transports. Record the application's requirements and compare candidate implementations against them; see [adaptation](adaptation.md) and [measurement](performance.md).

## Establish the constraints

Questions that can change the available choices include:

- Which process architecture can load the library, and which compiler/runtime ABI does its interface require?
- What wrappers, headers, working examples, or COM type information already exist?
- Which values can be copied, and which operations depend on persistent resources, callbacks, or a particular thread?
- Which client languages, deployment environments, build tools, and access controls must be supported?
- What call frequency, payload sizes, latency targets, and maintenance constraints matter to the user?

On Windows, a 64-bit process cannot directly load a 32-bit DLL for execution. Microsoft documents cross-bitness RPC and out-of-process COM as supported communication paths. This establishes the process boundary, not the best wrapper or transport for an application. [Process interoperability](https://learn.microsoft.com/en-us/windows/win32/winprog64/process-interoperability).

## Paths and evidence

| Path | Available evidence | Questions for the target application |
|---|---|---|
| C++ + MIDL + ncalrpc | The repository's [example](../bridges/cpp-ncalrpc/README.md) and [validation record](../bridges/cpp-ncalrpc/docs/validation.md) demonstrate x64 Julia calling an x86 C API DLL. Microsoft describes [MIDL stub generation](https://learn.microsoft.com/en-us/windows/win32/rpc/developing-the-interface). | Does the actual DLL satisfy the example's ABI, lifetime, and thread assumptions? What adapter code and deployment dependencies remain? |
| Out-of-process COM | Microsoft's [cross-bitness documentation](https://learn.microsoft.com/en-us/windows/win32/winprog64/process-interoperability) describes communication with a 32-bit COM server and wrapping a non-COM DLL. | Are marshalable COM interfaces already available? If not, who defines the wrapper, ownership rules, threading behavior, and deployment? |
| .NET + named pipes | [NamedPipeServerStream](https://learn.microsoft.com/en-us/dotnet/api/system.io.pipes.namedpipeserverstream) provides synchronous/asynchronous stream access. It does not define the application's RPC messages. | What framing, serialization, dispatch, and client implementation are needed? Can the chosen dependencies be published and run as win-x86? |
| .NET + gRPC over IPC | Microsoft provides [named pipe/UDS configuration examples](https://learn.microsoft.com/en-us/aspnet/core/grpc/interprocess?view=aspnetcore-10.0) and documents [gRPC Native AOT support](https://learn.microsoft.com/en-us/aspnet/core/fundamentals/native-aot). | Does the complete SDK, package, transport, and target-architecture combination work? Which clients can connect using that transport? |
| HTTP service | ASP.NET Core's [Native AOT guide](https://learn.microsoft.com/en-us/aspnet/core/fundamentals/native-aot) includes a Minimal API template and its compatibility limits. | How are operations and binary payloads represented? What measured overhead, endpoint lifecycle, and access controls fit the application? |
| Python + MSL-LoadLib | The upstream [client-server guide](https://mslnz.github.io/msl-loadlib/latest/usage/ipc/) demonstrates a 64-bit Python client accessing a 32-bit library. | Can the application use the Python runtime and its wrappers? What packaging, data types, and call costs need checking? |

Only the C++/ncalrpc path has an implementation and validation record in this repository. The other rows identify upstream documented capabilities or examples, not completed CrossBitBridge integrations. Individual feature support does not establish that a combined win-x86 deployment works.

Microsoft recommends ncalrpc for local calls within Windows RPC in its [protocol-sequence guidance](https://learn.microsoft.com/en-us/windows/win32/rpc/choosing-a-protocol-sequence). That recommendation is not a benchmark of every named-pipe, HTTP, or language-specific implementation.

## MSL-LoadLib case study

The [upstream example](https://mslnz.github.io/msl-loadlib/latest/usage/ipc/) separates native calls in a `Server32` subclass from requests made by `Client64.request32`. Creating the client starts the server. The guide describes exchanging Python objects through a file using `pickle` and includes runnable examples for several library types.

To evaluate this pattern, trace one operation from the public client method through argument conversion, native invocation, result conversion, and server shutdown. Then identify which pieces an application can reuse. CrossBitBridge has not run MSL-LoadLib or compared its performance with ncalrpc.

## Investigating C++ and .NET AOT integration

For a proposed implementation, identify which tool owns each boundary rather than estimating maintenance from the language name alone:

| Boundary | Source or concrete evidence | What to inspect |
|---|---|---|
| RPC contract to stubs | [MIDL generates C client/server stubs](https://learn.microsoft.com/en-us/windows/win32/rpc/developing-the-interface); the C++ example compiles them directly. | Which generated files and hand-written adapters does another host language need? |
| Managed code to a C API | [LibraryImport source generation](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/pinvoke-source-generation). | Which signatures, marshaling rules, and calling conventions are supported by the selected toolchain? |
| AOT deployment | [.NET Native AOT targets, prerequisites, and restrictions](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/). Runtime code generation such as Reflection.Emit is unsupported. | Which signatures are known at build time? Which dependencies and target runtime identifiers actually publish and run? |
| Native caller to an AOT library | [Native AOT exports](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/interop) use UnmanagedCallersOnly and have type restrictions. | Can a fixed C ABI represent the required operations, errors, and ownership rules? |

One design to test is a native MIDL/RPC shell calling an AOT library through C exports, with the AOT library invoking the vendor DLL. This combines documented building blocks; there is no end-to-end example of that combination here. Other designs may avoid that boundary by using a different transport. Count the generated code, handwritten code, build dependencies, and deployment artifacts of the actual designs before drawing maintenance conclusions.

## Additional language entry points

- [Rust Win32 RPC bindings](https://microsoft.github.io/windows-docs-rs/doc/windows/Win32/System/Rpc/index.html): inspect the available APIs and the remaining stub/adapter work. Bindings alone are not an end-to-end cross-bitness example.
- [Zig C interoperability](https://ziglang.org/learn/overview/): investigate compiling or linking the required native components with the selected target and SDK. No Zig bridge has been validated here.
- [Go net/rpc](https://pkg.go.dev/net/rpc): inspect its method requirements and default gob encoding when considering a client in another language. The package documents that it is frozen. That status alone does not determine suitability for a particular application.
