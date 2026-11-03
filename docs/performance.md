# Investigating bridge performance

A comparison can answer a particular workload question, such as whether call overhead meets a latency budget or whether array transfers sustain the required rate. Define that question and the acceptable bounds before choosing measurements. These notes do not prescribe a transport or require a benchmark for every adaptation.

## Identify the measured path

Language choice, AOT, binary encoding, and local transport describe different parts of a call. Draw the complete path and identify connection setup, serialization, copying, dispatch, native execution, and result handling. A microbenchmark of one part does not establish end-to-end performance.

Sources for interpreting those layers include Microsoft's [Windows RPC protocol guidance](https://learn.microsoft.com/en-us/windows/win32/rpc/choosing-a-protocol-sequence), [.NET gRPC IPC configurations](https://learn.microsoft.com/en-us/aspnet/core/grpc/interprocess?view=aspnetcore-10.0), and [MSL-LoadLib's documented exchange mechanism](https://mslnz.github.io/msl-loadlib/latest/usage/ipc/). These sources describe mechanisms and configurations; a matched comparison for the target DLL would require workload-specific measurements.

## Choose comparable experiments

| Question | Possible experiment | Context to retain |
|---|---|---|
| Small-call overhead | Empty/scalar operations, then the real DLL operation | Warm-up, connection reuse, DLL execution time |
| Transfer cost | Payload sizes from the actual application; 4 KiB and 1 MiB are possible sample points | Encoding, copy count, allocation, input/output direction |
| Tail behavior | Repeated calls under the expected concurrency | p50/p99, sample count, load, timeouts, errors |
| Startup and deployment | Start the packaged host and make its first successful call | Process startup, DLL loading, authentication, package size and dependencies |
| Value of batching | Equivalent work performed as separate calls and as one batch | Changed semantics, buffering, throughput, latency |

Record the hardware, OS, toolchain, runtime/package versions, commands, and input data. Matching output contracts, access controls, buffer ownership, and concurrency makes it possible to attribute differences to the compared designs. If those differ intentionally, report the difference as part of the design rather than isolating it as transport cost.

## Available repository evidence

The [C++ validation record](../examples/cpp-ncalrpc/docs/validation.md) covers functional tests, not latency or throughput. Its [client wrappers](../examples/cpp-ncalrpc/examples/sample/client.cpp) create a binding for every call, with authentication configured in [binding.cpp](../examples/cpp-ncalrpc/rpc/binding.cpp). Measurements of that client include those choices; they describe the wrapper's connection policy as well as the transport.

Record whether results satisfy the user's stated target, what alternatives were actually measured, and which conclusions remain open. A result for one payload, machine, or connection policy does not establish a general ranking of languages or transports.
