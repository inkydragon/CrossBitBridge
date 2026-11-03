# Comparing bridge performance

Do not claim equivalent performance based only on the language, AOT, binary encoding, or local transport. Named pipes are a transport; gRPC includes marshaling and dispatch; ncalrpc is the local transport path of Windows RPC. Compare the complete call chain.

Benchmark only when performance matters, using identical contracts: empty/scalar calls, 4 KiB and 1 MiB arrays, and real DLL operations. Record warmed-up p50/p99 latency, throughput, and allocations. Separate cold startup, connection establishment, and connection reuse. The client in `examples/cpp-ncalrpc/` currently creates a binding for each call; its total call cost is not the inherent minimum cost of ncalrpc. Match access controls, buffer-copy semantics, and serial/parallel settings.

These are measurement suggestions when performance is part of the task, not mandatory benchmark infrastructure for every example. The current C++ example has no recorded latency or throughput benchmark; see its [validation limits](../examples/cpp-ncalrpc/docs/validation.md).
