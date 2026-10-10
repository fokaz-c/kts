# AGENTS.md

This file is both:

1. A development plan for this Game Boy emulator project.
2. Guidance for AI agents or contributors working in this repository.

The project owner has final decision authority. Do not make architectural, dependency, naming, or scope decisions without asking when the choice is ambiguous.

## Project Goal

Build a Game Boy emulator in modern C++ with a learning-focused design.

Initial hardware target:

- Start with original Game Boy / DMG behavior.
- Design with eventual Game Boy Color support in mind, but do not implement GBC-specific behavior until explicitly approved.

Accuracy goal:

- Prioritize readability, learning value, and clear structure.
- Aim to pass common CPU/emulator test ROMs over time.
- Cycle accuracy may be considered later, but is not the initial priority unless explicitly requested.

## Confirmed Tooling

- Version control: `git`
- Build system: `CMake`
- Build generator: `Ninja`
- Language standard: `C++23`
- Compiler/toolchain: `gcc`
- Debugger: `gdb`

## Dependency Policy

- Keep dependencies as light as possible.
- Use `CMake FetchContent` only for required dependencies.
- Avoid adding dependencies without asking first.
- Prefer keeping the emulator core independent of frontend, test, and tooling dependencies.

## Planned Repository Layout

Use this initial layout unless the owner approves a change:

```text
CMakeLists.txt
src/
include/
tests/
docs/
resources/
```

Directory intent:

- `src/`: implementation files.
- `include/`: public or shared project headers.
- `tests/`: automated tests and test harnesses.
- `docs/`: Game Boy technical documentation, notes, datasheets, references, and project documentation.
- `resources/`: non-source resources such as test ROMs, fixtures, sample data, or locally provided assets.

## Core / Frontend Separation

The emulator core should be separated from frontend concerns.

The core should contain emulator logic such as:

- CPU
- instruction decoding/execution
- memory bus
- cartridge handling
- timers
- interrupts
- PPU
- APU
- joypad/input state

Frontend concerns should be isolated, such as:

- window creation
- rendering output
- audio playback
- keyboard/controller input
- debugger UI
- CLI behavior

Do not choose a frontend/windowing/audio library without asking first.

Pending frontend decision examples:

- SDL2
- SDL3
- GLFW plus an audio library
- headless-only first
- another option chosen by the owner

## Initial Milestones

### Milestone 1: Project skeleton

Pending approval before implementation.

Approved initial skeleton pieces:

- `src/`
- `include/`
- `tests/`
- `docs/`
- `resources/`
- `.gitignore`

Planned build targets, pending architecture discussion before generating `CMakeLists.txt`:

- emulator core library
- thin executable target

Approved warning policy for future CMake setup:

- `-Wall`
- `-Wextra`
- `-Wpedantic`
- `-Wconversion`
- `-Wsign-conversion`

The approved initial architecture is recorded in `docs/architecture-decision-log.md`. The initial owned components are CPU, Bus, and Timer; the CPU uses compile-time bus polymorphism and instruction-level stepping in M-cycles. Approval of architecture does not itself authorize generating `CMakeLists.txt` or source/header files; wait for the owner to ask for implementation.

### Milestone 2: CPU-focused development

Primary first emulator milestone:

- Implement enough CPU functionality to run known CPU test ROMs.
- Focus on correctness and clear implementation.
- Add tests around CPU behavior as the implementation develops.

### Milestone 3: Test ROM support

Target known Game Boy emulator test ROMs, such as CPU validation ROMs, when the owner provides or approves them.

Rules:

- Do not commit copyrighted commercial ROMs.
- Test ROMs may be stored under `resources/test-roms/` if approved.
- If test ROMs are third-party open-source/public test assets, preserve their license and attribution.
- If ROMs are local-only, keep them out of git as appropriate.

### Milestone 4: ROM loading

After CPU test ROM work begins or passes sufficient tests, add cartridge/ROM loading support.

Do not prioritize general ROM loading before the CPU test-ROM milestone unless the owner changes the plan.

## Testing Plan

Testing should be present from the beginning.

Approved test framework:

- GoogleTest

Use `CMake FetchContent` for GoogleTest unless the owner later approves a different dependency setup.

Likely test categories:

- CPU instruction behavior
- flag behavior
- memory bus behavior
- interrupt behavior
- timer behavior
- cartridge header parsing
- test ROM execution harnesses

## Documentation Policy

Use both `docs/` and `resources/`.

Suggested split:

- `docs/`: specifications, CPU reference material, implementation notes, architecture notes, debugging notes.
- `resources/`: test ROMs, fixtures, generated/reference data, local non-source assets.

Do not assume external documentation or ROM assets are redistributable. Ask before committing any third-party files.

## Git / VCS Guidelines

- Use git for version control.
- Do not create commits unless explicitly requested.
- Do not create branches unless explicitly requested.
- Do not overwrite or remove user changes without explicit approval.
- Keep changes small and focused.

## C++ Style Guidelines

These are initial guidelines and may be refined later.

- Use C++23.
- Prefer clear, readable code over clever abstractions.
- Keep emulator core logic deterministic and testable.
- Prefer strong types where they improve clarity.
- Avoid global mutable state unless explicitly justified.
- Keep platform-specific code out of the emulator core.
- Avoid adding comments that merely restate the code.
- Add comments for non-obvious hardware behavior, timing constraints, or emulator tradeoffs.

## Agent Rules

When working in this repository:

1. Ask before making ambiguous decisions.
2. The project owner has final decision authority.
3. Do not add dependencies without approval.
4. Use GoogleTest for automated tests unless the owner approves a change.
5. Do not choose a frontend/windowing/audio library without approval.
6. Do not commit ROMs unless their licensing and intended location are approved.
7. Keep the core separate from frontend/platform code.
8. Prefer minimal, focused changes.
9. Validate changes with the most relevant available build or tests.
10. Report clearly what was changed and what validation was run.

## Pending Decisions

The following decisions still require owner approval:

- Frontend/windowing/input/audio library, if any.
- Exact CPU test ROM suite to target first.

- Formatting/linting tools, if any.
- Whether to use `docs/` for copied datasheets or only links/notes.
