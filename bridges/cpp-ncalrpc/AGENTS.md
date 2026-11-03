# C++ / ncalrpc example

Windows-only, IDL-first cross-bitness RPC template. Keep the project small: ordinary IDL and source files, no binding DSL or macros. C++ is the implemented path; .NET AOT is an architecture option, not a validated backend.

- Build: `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1`
- Test: `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/test.ps1 [-Julia julia]`
- Only x86 loads the vendor DLL. Keep process pointers out of the wire format.
- MIDL generates stubs; project-specific adapter/client semantics remain explicit code.
- Preserve local-only authenticated RPC and current-user interface ACLs.
- Do not upload proprietary DLLs or headers without the user's authorization.
- Do not push, publish artifacts externally, or create remote repositories unless requested.
- Keep build products in ignored build/ and artifacts/. Record actual native test evidence.

Follow the repository-root AGENTS.md for project-wide instructions. Cross-implementation selection guidance is in ../../docs/approaches.md. Paths in this file are relative to this example directory.
