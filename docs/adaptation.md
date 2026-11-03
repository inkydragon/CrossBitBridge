# Investigating and adapting a legacy library

The following questions and small experiments can help establish a bridge design. Their order and scope depend on the user's task and available evidence. [Approaches](approaches.md) links to candidate paths; each example's README documents its actual implementation.

## Gather evidence about the native interface

| Information to establish | Possible sources or checks |
|---|---|
| Architecture, exports, and ABI | Vendor headers, compiler settings, binary inspection, and working native examples. Export names alone do not establish complete signatures. |
| Lengths, encoding, and ownership | Vendor documentation, annotations, allocation/free pairs, and existing wrapper behavior. Record gaps explicitly. |
| Persistent state and execution requirements | Initialization/shutdown code, retained pointers, callbacks, thread-affinity requirements, and thread-local error handling. A simple signature does not establish stateless behavior. |
| Existing behavior | A working x86 Julia/C#/C++ wrapper, native tests, or a small direct call to the DLL. Note which behavior comes from the wrapper rather than the DLL. |
| Deployment constraints | Permitted runtimes, target machines, toolchain availability, DLL dependencies, access requirements, and restrictions on uploading proprietary inputs. |

For Windows process/architecture constraints, see [Microsoft's interoperability documentation](https://learn.microsoft.com/en-us/windows/win32/winprog64/process-interoperability).

## Describe the process boundary

For each operation, identify inputs, outputs, error channels, resource lifetime, and the process responsible for each allocation or object. A pointer's numeric value is not by itself a reference to usable memory in another process. Copied values, copied buffers, remote identifiers, and explicit resource operations are possible representations; the API semantics determine which are adequate.

For MIDL interfaces, Microsoft's [interface guide](https://learn.microsoft.com/en-us/windows/win32/rpc/developing-the-interface) explains the contract and generated stubs. [size_is](https://learn.microsoft.com/en-us/windows/win32/midl/size-is) describes allocation size, while [length_is](https://learn.microsoft.com/en-us/windows/win32/midl/length-is) describes the number of transmitted elements. These declarations still need correct length and ownership information from the underlying API.

For interfaces with callbacks or state across calls, examine reentrancy, thread identity, cleanup, and what happens after host failure. Neither a transport nor COM automatically supplies the vendor library's ownership semantics.

## Use a bounded experiment

A few representative operations can expose different requirements: a scalar result, a variable-length output, an error return, or an operation that retains state. Choose cases relevant to the library rather than treating a fixed number as the final interface scope.

The repository's [C++/ncalrpc example](../bridges/cpp-ncalrpc/README.md) provides concrete IDL, adapter code, build commands, and a Julia client. [MSL-LoadLib's example](https://mslnz.github.io/msl-loadlib/latest/usage/ipc/) shows a different separation between native wrappers and remote methods. Use their validation boundaries when deciding which parts can be reused.

Possible checks include comparing bridged outputs and errors with a direct x86 baseline, verifying actual process architectures, exercising a rejection case, and observing host shutdown. Distinguish failures to load the DLL, transport failures, and native-library errors. A successful scalar call alone does not establish support for the library's other interfaces.

## Record the decision and its limits

A concise record can include the requirements, sources consulted, alternatives examined, working commands, results, and unresolved questions. Explain which observations support the chosen design. Use [performance guidance](performance.md) if measured overhead affects the choice.

If the vendor DLL is unavailable, separate sample validation from real-library compatibility. Building without proprietary binaries and testing on a local or self-hosted machine is one possible workflow. The user's access and publication constraints determine where those steps may run.
