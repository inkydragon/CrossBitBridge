# C++ / ncalrpc example

Follow the root [AGENTS.md](../../AGENTS.md). See [build and test instructions](docs/build.md) and [adapter contracts / Win32 references](docs/architecture.md).

- Only the x86 host loads the legacy DLL. Keep process pointers out of the wire format.
- Preserve local authenticated RPC and the current-user interface ACL.
- Edit the IDL, not generated `*.gen.h` / `*.gen.c` files. Implement adapter and client semantics explicitly.
- Keep C++ resource owners outside the RPC SEH frame; preserve fatal-exception propagation.
