# AGENTS.md — CrossBitBridge

## Project goal

CrossBitBridge provides small, runnable examples and AI guidance for calling libraries across process bitness. Help users generate and verify dedicated bridges for their legacy libraries. Deliver demos/MWEs that are easy to understand, copy, and build independently.

## Working principles

- Complete the user's current request and keep outputs small and reviewable. Do not grow examples into a general RPC framework.
- Use ordinary source files, standard interface descriptions, and existing toolchains. Do not introduce a unified DSL, binding macros, or multi-backend abstractions on your own initiative.
- Maintain each implementation independently. Extract shared code only when actual duplication and clear benefits justify it.
- Distinguish implemented paths, executed validation, and exploratory options. Report actual evidence, open questions, and validation limits; do not generalize a passing example into real-library compatibility or performance claims.
- Future skills should reference runnable examples and documentation to guide AI in generating dedicated projects. Do not add skills or implementation paths unless requested.

## Language and synchronization

1. Use English by default throughout the rest of the project, including Markdown files without a language suffix, code comments, script messages, and CI configuration.
2. Keep only the root `AGENTS.zh-cn.md` as a Chinese document and use it as the content source for the English `AGENTS.md`. Maintain all other documents in English only; do not create other `*.zh-cn.md` files. Preserve constraints, open questions, and verification limits faithfully in `AGENTS.md`; do not expand it independently.

## Content ownership

- Root `AGENTS.md`: project goals, working principles, and repository conventions.
- Root [README.md](README.md): project entry point and example index.
- Root `docs/`: technical discussions, comparisons, and references shared across implementations. See [approaches](docs/approaches.md), [adaptation workflow](docs/adaptation.md), and [performance comparisons](docs/performance.md).
- `bridges/<language>-<transport>/`: implementation source, local guidance, build steps, and validation records. Consult the example's own documentation for specific implementation requirements.
- `legacy/<case>/`: independently buildable legacy library samples, one directory per case without a language category layer. Bridge tests receive an explicit DLL path.
- Root `.github/workflows/`: CI entry points for examples.

## Operating conventions

- Read an example's local guidance before modifying it. Run builds or validation relevant to the change and record results accurately.
- Preserve each implementation's access controls and resource boundaries. Keep build products in the corresponding example's ignored directories.
- Do not upload vendor DLLs, private headers, or logs without user authorization. Do not push, publish artifacts, or create remote repositories unless requested.
