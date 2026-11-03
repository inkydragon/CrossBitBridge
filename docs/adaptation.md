# Adapting a legacy library

This is a suggested workflow for a dedicated bridge project. Select an approach using [approaches.md](approaches.md), then use the chosen example's README for concrete files, commands, and implementation limits.

1. Obtain the target architecture, calling convention, headers, examples, dependent DLLs, and permitted inputs. Export names alone do not establish full signatures.
2. Start with 1–3 representative operations to validate the chain: scalars, input/output arrays, and error returns. Then complete the interface scope requested by the user. Existing x86 Julia/C#/C++ wrappers can serve as behavioral baselines.
3. Establish length units, encoding, error codes, and ownership. Record unknowns rather than guessing how memory is freed.
4. Check whether the API retains pointers or resources across calls, invokes callbacks, or requires a fixed thread or thread-local state. Stateless copying is a useful starting assumption, not a property inferred from a simple function signature. Use dedicated adaptation when the assumption does not hold.
5. Keep native pointers and language objects inside their owning process. Define explicit values, buffers, or resource semantics at the process boundary.
6. Copy the relevant example and edit ordinary contracts and source files. Use its standard generators and build tools; the repository does not require a new binding DSL.
7. Run a real cross-bitness test. Compare important outputs with a local x86 baseline, include an error/rejection case, and distinguish transport errors from library errors.
8. If the vendor DLL is unavailable, report what passed using the sample and what remains unverified. Cloud builds may compile wrappers without proprietary binaries; real-library tests can run locally or on a self-hosted runner.

A useful deliverable is a small dedicated project containing its contract, host, client, build instructions, and replayable evidence. This workflow does not imply permission to upload proprietary inputs or publish artifacts.

The implemented reference is [C++ / ncalrpc](../examples/cpp-ncalrpc/README.md). Its README documents IDL path arguments, adapter files, and build commands. Its [architecture](../examples/cpp-ncalrpc/docs/architecture.md) documents native dispatch, security, and lifetime boundaries.
