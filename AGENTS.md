# AGENTS.md

This file is both:

1. A living project map and development plan for this Game Boy emulator.
2. Guidance for AI agents and contributors working in this repository.

The project owner has final decision authority. Do not make architectural, dependency, naming, or scope decisions without asking when the choice is ambiguous.

## Project Goal

Build a Game Boy emulator in modern C++ with a learning-focused design.

- Initial hardware target: original Game Boy / DMG.
- Keep eventual Game Boy Color support in mind, but do not implement GBC-specific behavior until explicitly approved.
- Prioritize readability, learning value, testability, and clear structure.
- Aim to pass common CPU/emulator test ROMs over time. Cycle accuracy is not the initial priority unless explicitly requested.

## Current Project Index

The repository currently contains the following project areas (this is a working-tree inventory, not a claim that every listed file is tracked by git):

- `CMakeLists.txt`: CMake project setup; defines the `gameboy_core` INTERFACE target and, when `BUILD_TESTING` is enabled, the `gameboy_core_tests` executable. GoogleTest is obtained through `FetchContent` if a suitable installed package is not found.
- `CMakePresets.json`: `ninja-debug` configure, build, and test presets; uses `build/` as the binary directory.
- `include/gameboy/types.hpp`: fixed-width integer aliases (`u8`, `u16`, `i8`, `i16`).
- `include/gameboy/core/cycle_count.hpp`: `gameboy::CycleCount`, represented in M-cycles.
- `include/gameboy/cpu/registers.hpp` and `src/gameboy/cpu/registers.cpp`: CPU register file, register-pair access, and flag access.
- `tests/unit/test_cycle_count.cpp` and `tests/unit/test_registers.cpp`: current unit-test sources.
- `docs/architecture-decision-log.md`: approved architectural decisions and remaining questions.
- `docs/high-level-design.md`: component responsibilities, execution coordination, and timing questions.
- `README`: brief project introduction and Pan Docs link. Its build section is currently a placeholder and should be reconciled with the configured CMake presets when updating user-facing build instructions.
- `.gitignore`, `.clang-format`, `.clangd`, `.editorconfig`, and `.zed/`: repository/editor configuration. Formatting and linting policy has not been approved; do not treat a present config as authorization to introduce tooling or impose new policy.

The implementation is at an early foundation stage. Do not imply that a CPU, Bus, Timer, emulator executable, frontend, ROM loader, or full memory map already exists unless code is added and verified.

## Confirmed Tooling and Build

- Version control: `git`
- Build system: CMake (minimum version 3.24)
- Generator: Ninja
- Language: C++23; C++ extensions are disabled
- CMake testing: CTest
- Test framework: GoogleTest 1.14.0 through `FetchContent`, with installed-package lookup supported
- Compiler/toolchain and debugger: gcc and gdb are the confirmed development environment

Typical configured workflow, using the existing preset:

```sh
cmake --preset ninja-debug
cmake --build --preset ninja-debug
ctest --preset ninja-debug
```

The build requires GoogleTest to be available locally or fetchable during CMake configuration. Do not claim tests pass unless the commands have actually been run successfully.

The CMake test target currently receives `-Wall`, `-Wextra`, `-Wpedantic`, `-Wconversion`, and `-Wsign-conversion` on GNU/Clang, and `/W4` plus `/permissive-` on MSVC. Do not silently broaden project-wide policy beyond the current configuration.

## Dependency Policy

- Keep dependencies as light as possible.
- Use `CMake FetchContent` only for required dependencies.
- GoogleTest is the approved automated test framework.
- Ask before adding dependencies or changing dependency setup.
- Keep the emulator core independent of frontend, test, and tooling dependencies.

## Planned Repository Layout

Current and intended areas:

```text
CMakeLists.txt
CMakePresets.json
include/
src/
tests/
docs/
resources/    # planned; create/use for approved assets as needed
```

Directory intent:

- `src/`: implementation files.
- `include/`: public or shared project headers.
- `tests/`: automated tests and test harnesses.
- `docs/`: technical notes, project documentation, references/links, and architecture decisions.
- `resources/`: non-source assets such as approved test ROMs and fixtures; do not add third-party assets without checking licensing and attribution.

Keep changes focused and consistent with the existing layout. Do not create additional project areas or reorganize files without a clear need and owner approval where the choice is ambiguous.

## Core / Frontend Separation

The emulator core should remain separate from frontend concerns.

Core responsibilities may include CPU, instruction decoding/execution, memory Bus, cartridge handling, timers, interrupts, PPU, APU, and joypad state. Frontend concerns include windowing, presentation, audio playback, physical input, debugger UI, and CLI behavior.

Do not choose a frontend/windowing/audio library without asking first. Frontend options (including headless-only development) remain open.

## Approved Architecture

The approved initial architecture is recorded in `docs/architecture-decision-log.md`; the high-level design is in `docs/high-level-design.md`. Read those documents before implementing architecture-sensitive behavior, and preserve the distinction between approved decisions and open questions.

Approved direction:

- An `Emulator` composition root owns the CPU, concrete Bus, and initial Timer, using direct member objects where practical.
- The CPU uses a small compile-time Bus contract; do not introduce runtime-polymorphic Bus interfaces without approval.
- The Bus incrementally implements the Game Boy memory map. IE (`0xFFFF`) and IF (`0xFF0F`) are accessed through ordinary mapped Bus operations; interrupt request generation is distinct from CPU interrupt servicing.
- CPU execution is instruction-stepped initially: a complete instruction reports elapsed time as `CycleCount` in M-cycles. True machine-cycle stepping is deferred.
- The Emulator coordinates elapsed-time advancement of implemented peripherals; CPU-specific behavior remains the CPU's responsibility.

Open hardware/API details remain unresolved in the design documents, including exact DMG timing semantics and parts of interrupt, HALT/STOP, and peripheral request behavior. Do not turn recommendations or unresolved questions in those documents into approved decisions. Check reliable hardware documentation and tests before implementing behavior affected by those questions.

## Development Plan

The repository skeleton, initial CMake setup, and foundational register/CycleCount code now exist. Continue with the approved CPU-focused milestone; do not recreate or treat the skeleton as pending.

### CPU-focused development

- Implement CPU functionality incrementally, prioritizing correctness and readability.
- Add focused GoogleTest coverage as behavior is implemented.
- Consult the architecture documents for execution, Bus, interrupt, and timing boundaries.

### Test ROM support

Target known emulator/CPU test ROMs only when the owner provides or approves the suite.

- Do not commit copyrighted commercial ROMs.
- Store ROMs under `resources/test-roms/` only if approved.
- Preserve license and attribution for redistributable third-party test assets.
- Keep local-only ROMs out of git as appropriate.

### ROM loading

General cartridge/ROM loading is not the current priority ahead of CPU-focused tests unless the owner changes the plan. Implement only when appropriate to the project milestone or explicitly requested.

## Documentation Policy

Maintain architecture and implementation notes in `docs/`. Prefer links and original notes over copied reference materials unless the owner approves copying and redistribution terms are clear. Do not assume external documentation or ROM assets are redistributable.

When project structure, build commands, implementation status, or approved decisions change, update this index and relevant documentation so they do not contradict the codebase.

## Git / VCS Guidelines

- Use git for version control.
- Do not create commits or branches unless explicitly requested.
- Do not overwrite or remove user changes without explicit approval.
- Check the working tree before editing and keep changes small and focused.

## C++ Style Guidelines

- Use C++23.
- Prefer clear, readable code over clever abstractions.
- Keep emulator-core logic deterministic and testable.
- Prefer strong types where they improve clarity.
- Avoid global mutable state unless explicitly justified.
- Keep platform-specific code out of the emulator core.
- Avoid comments that merely restate code; document non-obvious hardware behavior, timing constraints, and tradeoffs.
- Follow existing source formatting. Do not introduce a formatter/linter or treat its configuration as an approved project-wide policy without owner approval.

## Agent Rules

When working in this repository:

1. Ask before making ambiguous architectural, dependency, naming, or scope decisions.
2. Respect the project owner's final decision authority.
3. Do not add dependencies without approval.
4. Use GoogleTest for automated tests unless the owner approves a change.
5. Do not choose a frontend/windowing/audio library without approval.
6. Do not add ROMs unless licensing and intended location are approved.
7. Keep core logic separate from frontend/platform code.
8. Make minimal, focused changes and preserve user work.
9. Validate changes with the most relevant available build or tests.
10. Report clearly what changed and what validation was run; never claim unrun checks passed.

## Pending Decisions

- Frontend/windowing/input/audio library, if any.
- Exact CPU test ROM suite to target first.
- Formatting/linting tools and whether existing editor configs should be formal project policy.
- Whether `docs/` should contain copied datasheets or only links and original notes.
