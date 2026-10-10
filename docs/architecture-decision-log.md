# Game Boy Emulator — Architectural Decision Log

Entries marked **Approved** record directions explicitly approved by the project owner. Proposed entries are not approved; their open questions remain undecided unless stated otherwise.

## Architecture overview

An `Emulator` object exclusively owns the CPU, concrete memory `Bus`, and Timer, using direct member objects where practical. The CPU accesses memory through a small bus contract expressed with C++ templates / compile-time polymorphism. Initially, the CPU executes complete instructions and reports elapsed time as a dedicated `CycleCount` measured in M-cycles. Under approved ADL-006, the Emulator coordinates CPU processing and peripheral advancement while the CPU retains CPU-specific behavior. Exact DMG timing semantics remain subject to hardware verification. The concrete bus incrementally implements the Game Boy memory map.

The project targets original Game Boy / DMG behavior first, prioritizes learning and readability, and defers true CPU machine-cycle stepping until needed. These are established project constraints, distinct from the software architecture choices recorded below.

## Decision summary

| ID | Title | Status |
|---|---|---|
| ADL-001 | Component Ownership | Approved |
| ADL-002 | CPU Execution Model | Approved |
| ADL-003 | CPU-to-Bus Dependency | Approved |
| ADL-004 | Bus Responsibilities | Approved |
| ADL-005 | Interrupt Register Access | Approved |
| ADL-006 | Execution Coordination and CPU Responsibility | Approved; hardware timing details open |

## ADL-001: Component Ownership

**Status:** Approved

### Context

The emulator needs a clear place to construct and coordinate the CPU, memory bus, and hardware components. Their ownership and access relationships affect lifetime management, testability, and future integration.

### Options considered

- **CPU owns the bus:** Simple ownership relationship, but couples the CPU's lifetime and construction to a particular bus.
- **Emulator owns CPU and bus:** Centralizes component ownership and allows the emulator to coordinate execution and peripherals.
- **Ownership varies by context:** Can fit tests or specialized setups, but may leave production lifetime rules less explicit.

### Proposed decision

An `Emulator` object exclusively owns the CPU, concrete `Bus`, and initial `Timer`, using direct member objects where practical. The CPU accesses the bus through the compile-time bus contract.

### Rationale

A single composition boundary makes component lifetimes explicit and gives the emulator a natural place to coordinate execution. Direct member objects provide straightforward exclusive ownership where construction constraints permit. This fits the plan to keep emulator-core behavior separate from frontend concerns.

### Consequences

- **Easier:** Centralized construction and lifetime management; coordinating CPU execution with the Bus and Timer.
- **Limitations:** Direct-member construction order and reference validity must be handled correctly.
- **Possible future changes:** Additional components may require more coordination or explicit initialization sequencing.

### Professional systems-development relevance

Explicit ownership matters in C++ because object lifetime and reference validity are central to safe design. In embedded systems, a top-level system or board object commonly composes hardware-facing components. This organizational model does not prescribe a particular hardware abstraction or memory-management technique.

### Open questions and assumptions

- Ownership is exclusive by `Emulator`; use direct member objects where practical.
- The initial owned components are CPU, Bus, and Timer.
- Construction ordering and exact component APIs remain implementation details.

## ADL-002: CPU Execution Model

**Status:** Approved

### Context

The CPU needs an execution interface. The design balances a readable initial implementation against timing precision for more detailed hardware behavior.

### Options considered

- **Whole-instruction stepping:** One call executes a complete instruction and reports elapsed time. Simpler to implement and test, but cannot represent intermediate CPU states during an instruction.
- **CPU machine-cycle-by-machine-cycle stepping:** Exposes finer-grained CPU progress and supports closer CPU/peripheral interleaving, but requires a more complex execution model.
- **Both from the start:** Offers multiple control levels immediately, at the cost of additional implementation and API complexity.

### Proposed decision

Initially, `CPU::step()` executes one complete instruction and returns a dedicated `CycleCount` measured in M-cycles. The Emulator coordinates peripheral advancement using that elapsed time. HALT and STOP are modeled from the start as distinct CPU states. True CPU machine-cycle-by-machine-cycle execution is deferred until needed.

### Rationale

Instruction-level stepping matches the learning-focused goal and the decision not to prioritize cycle accuracy initially. Returning an explicit timing type lets the system account for peripheral progress without requiring the CPU to expose intermediate instruction states. `CycleCount` represents the elapsed M-cycles for the instruction actually executed, so variable instruction timing is supported. Timing-unit conversions should be centralized rather than repeated as assumptions throughout CPU and peripheral code; this preserves a possible future timing-unit migration without changing the approved initial M-cycle contract.

### Consequences

- **Easier:** CPU execution and tests can be organized around complete instructions; peripheral advancement can use instruction timing. The explicit `CycleCount` can represent differing elapsed M-cycles for differing instruction paths.
- **Limitations:** Peripherals cannot be interleaved with CPU activity at every point within an instruction. This is not cycle-by-cycle CPU execution.
- **Possible future changes:** Moving to CPU machine-cycle stepping may require changing the CPU execution interface and system coordination. If finer time units are needed before that, a centralized conversion boundary could allow a future T-cycle-based elapsed-time contract while CPU execution remains instruction-level. Changing units alone would not provide sub-instruction synchronization or cycle accuracy.

### Professional systems-development relevance

This is an accuracy-versus-complexity trade-off common in emulators and simulations. It also makes timing units and API contracts important: callers must interpret returned values consistently.

### Open questions and assumptions

- `CycleCount` is the dedicated timing type and M-cycles are the initial API unit; normal `CPU::step()` executes one complete instruction and reports its elapsed M-cycles. The returned count reflects the executed instruction/path, including variable timing.
- On the original Game Boy in normal-speed operation, one M-cycle equals four clock cycles (T-cycles). Any conversion is centralized; M-cycle-to-T-cycle arithmetic must not be scattered across the codebase.
- A future T-cycle elapsed-time representation may be considered without changing CPU instruction granularity or component ownership, but would change the timing contract and peripheral advancement APIs. T-cycle accounting alone does not make the emulator cycle-accurate.
- The precise elapsed-time behavior and wake-up rules for HALT and STOP remain to be specified consistently with DMG hardware behavior.

## ADL-006: Execution Coordination and CPU Responsibility

**Status:** Approved for software responsibility boundary and instruction-level coordination; DMG hardware timing semantics remain open

### Context

The Emulator owns CPU, concrete Bus, and Timer (ADL-001, Approved). The CPU executes complete instructions and reports elapsed M-cycles initially (ADL-002, Approved); it uses a compile-time Bus contract (ADL-003, Approved). IE/IF are memory-mapped and CPU interrupt servicing is separate from peripheral request generation (ADL-005, Approved).

The owner has approved Emulator-coordinated execution with CPU-owned semantics, operation-boundary interrupt checks, Timer ownership, Bus routing to device-owned registers, and explicit peripheral ordering. These are software architecture choices. They do not approve exact DMG timing, HALT/STOP behavior, HALT-bug behavior, interrupt-entry timing, or Timer clock behavior.

### Options considered

- **Execute an instruction, then advance peripherals without explicitly considering CPU state:** Simple for ordinary execution, but fails to make instruction, interrupt-service, HALT, and STOP processing explicit. It can obscure CPU-specific behavior or push state decisions into the Emulator.
- **Put CPU execution and interrupt semantics in the Emulator:** Makes scheduling visible, but requires the Emulator to inspect CPU internals and implement instruction-vs-interrupt choice, IME/priority effects, and HALT/STOP rules. This splits CPU semantics and conflicts with the approved responsibility boundary.
- **Emulator coordination with CPU-owned semantics (approved):** Emulator coordinates execution and peripheral advancement. In normal instruction execution, `CPU::step()` executes one complete instruction and reports its elapsed M-cycles. CPU also owns interrupt service and HALT/STOP processing, but their relationship to the instruction-step API and their result/time reporting remain unresolved. Peripherals generate requests through the hardware model, and CPU applies CPU interrupt semantics at approved operation boundaries.

### Decision

`Emulator::step()` coordinates CPU processing and advancement of implemented peripherals. CPU owns CPU state, instruction execution, interrupt servicing, and distinct HALT and STOP states/behavior. CPU processing checks interrupt conditions at CPU-operation boundaries; this is an approved scheduling boundary, not a statement that DMG hardware checks only at that point. In normal instruction execution, `CPU::step()` executes one complete instruction and returns its elapsed `CycleCount` in M-cycles. The non-instruction API and result/time reporting for interrupt service and HALT/STOP remain unresolved; this decision does not require them to use the same API or result type. The Emulator advances implemented peripherals under the approved coordination boundary using elapsed time made available by the eventual CPU processing contract. Peripherals own device state; Bus routes mapped device-register reads/writes to the owning device while IE (`0xFFFF`) and IF (`0xFF0F`) remain available through ordinary Bus operations. Peripheral ordering is explicit and based on hardware dependencies; no generic scheduler is introduced. True CPU machine-cycle-by-machine-cycle execution remains deferred.

A request raised during peripheral advancement is considered at a subsequent CPU operation under the instruction-level coordination model. This is the approved software scheduling approximation, not an assertion about exact hardware sampling timing. Exact interrupt timing and hardware state transitions remain subject to verification.

### Rationale

The Emulator is the system-level coordinator without becoming a second CPU implementation. CPU-specific decisions remain local to the CPU. Compared with treating every call as a normal instruction, the contract makes CPU-state processing explicit; compared with moving semantics into the Emulator, it avoids splitting CPU behavior. Instruction-level scheduling is readable and testable, but cannot observe peripheral events at every point within an instruction. That fidelity limit is explicit and does not authorize cycle-accuracy claims.

### Consequences

- **Easier:** One coordinator advances time; CPU behavior stays encapsulated; device state has a clear owner; mapped CPU accesses go through Bus; ordering is explicit when real dependencies exist.
- **Limitations:** Requests generated during a bulk peripheral update are not acted upon mid-instruction. Boundary scheduling approximates finer hardware timing. M-cycle reporting alone may not distinguish waiting from elapsed-time progress.
- **Possible future changes:** Finer CPU/peripheral synchronization may be considered later, retaining this ownership boundary but revisiting the progress API. No generic scheduler or machine-cycle CPU stepping is part of the approved initial architecture.

### Unresolved API and hardware questions

These questions do not revoke the approved software boundary:

1. **Non-instruction CPU contract:** How do interrupt service and HALT/STOP processing relate to the instruction-level `CPU::step()` API? Normal instruction execution remains one complete instruction with an M-cycle `CycleCount`; do not represent other CPU-owned behavior as ordinary instruction execution by assumption.
2. **Zero cycles and result shape:** Can any supported CPU operation legitimately report zero elapsed time? `CycleCount` alone is clear for normal instruction execution, but may not be sufficient for non-instruction processing. Decide only the minimum distinction required by verified behavior. Zero does not inherently mean waiting, and no generic result framework is proposed.
3. **Interrupt timing:** What are exact DMG interrupt-check, acknowledgement, service-entry, and peripheral-event timing rules, including EI delay and HALT interaction? Operation-boundary checks are an approved software policy; exact hardware behavior needs authoritative references and tests.
4. **HALT:** What are exact DMG wake conditions and HALT-bug trigger/fetch consequences? HALT is a distinct CPU state, but its hardware semantics remain subject to verification.
5. **STOP:** What are exact DMG wake sources and clock/power effects, including which Timer/peripheral clocks progress? STOP is distinct from HALT; its behavior remains subject to verification.
6. **Timer request path and clock refinement:** Timer owns its state and Emulator advances it; what minimal hardware-model path sets the timer request in IF, and what exact Timer clock/edge behavior applies in each CPU state? Preserve ordinary Bus access to IF. Keep the Timer's internal clock/edge calculation encapsulated so it can be refined against verified DMG behavior without changing ownership or the Emulator coordination boundary.
7. **Peripheral ordering:** Ordering is an explicit policy, not a generic scheduler. Concrete ordering/dependencies must be established from the actual peripherals and hardware behavior as they are added.

See `docs/high-level-design.md` for diagrams, the unresolved non-instruction CPU operation/result relationship, and implementation-readiness guidance. Approval of ADL-006 covers the software architecture choices stated in the Decision section only; it does not approve unresolved CPU API details or hardware semantics.

## ADL-003: CPU-to-Bus Dependency

**Status:** Approved

### Context

The CPU needs memory access, but its dependency on the bus affects coupling, test setup, and extensibility. The approved compile-time Bus contract is also the existing seam for fake-bus CPU tests.

### Options considered

- **Depend on the concrete `Bus`:** Straightforward and avoids an interface, but ties CPU code directly to the production bus and can make isolated CPU tests harder to configure.
- **Depend on a small abstract bus interface:** Allows a concrete production `Bus` and test doubles to provide the CPU's required operations. Adds an abstraction and may involve virtual dispatch if implemented with virtual functions.
- **C++ templates / compile-time polymorphism:** Avoids runtime dispatch, but can increase compile-time coupling and may make implementation diagnostics or binary organization more complex.

### Proposed decision

The CPU uses a small bus contract expressed through C++ templates / compile-time polymorphism. The concrete production `Bus` and a fake test bus both satisfy that contract. The initial contract exposes 8-bit and 16-bit reads and writes.

### Rationale

The compile-time contract supports isolated CPU tests with a fake Bus without requiring the production memory map to be complete, while avoiding runtime dispatch on bus access. The fake Bus can provide controlled memory for instruction, flag, and memory-effect tests; IE/IF state can support interrupt tests, and `CycleCount` results can be asserted for timing. CPU logic depends on the small operations it needs rather than a concrete production bus. Keeping the contract small limits unnecessary abstraction.

### Consequences

- **Easier:** Unit testing CPU behavior with a fake bus; developing CPU code before the production bus is complete.
- **Limitations:** Template-based dependencies couple CPU instantiations to bus types at compile time and can increase compile-time complexity or code generation.
- **Possible future changes:** The contract can evolve as CPU needs become clearer; the selected approach prioritizes minimizing runtime dispatch from the start.

### Professional systems-development relevance

Dependency inversion and test doubles are common techniques for isolating components in C++ systems. Templates provide compile-time polymorphism and can avoid virtual dispatch, which suits the stated performance preference for bus access. The trade-off is compile-time coupling and possible code-size or compilation costs, which should be monitored without premature optimization.

### Open questions and assumptions

- The initial contract exposes 8-bit and 16-bit reads and writes.
- Exact signatures, constraints, and type names remain implementation details.
- The requirement is to minimize runtime dispatch from the start; no numeric performance target is specified.
- The existing fake Bus is the CPU test double for controlled reads/writes and memory state; no additional testing abstraction is required by this decision.

## ADL-004: Bus Responsibilities

**Status:** Approved

### Context

The production bus must provide CPU memory access while the hardware map is developed incrementally. A flat test-only memory model does not define production bus responsibilities.

### Options considered

- **Flat 64 KiB production memory:** Simple initially, but does not represent the Game Boy's mapped regions and registers.
- **Complete memory map up front:** Comprehensive, but requires implementing hardware regions before the CPU-focused milestone needs them.
- **Incremental Game Boy memory map:** Add mapping and routing for agreed regions as components become available, while keeping the Game Boy address space in mind.

### Proposed decision

The concrete `Bus` incrementally implements the Game Boy memory map and routes accesses to cartridge ROM, VRAM, cartridge RAM, WRAM, OAM, I/O, HRAM, and IE as required by the first CPU milestone. Unmapped or not-yet-implemented addresses receive explicit temporary behavior documented per region. DMG access restrictions are implemented as relevant hardware is added. The fake bus shares the CPU's compile-time bus contract but has independent test storage.

### Rationale

Incremental mapping fits the CPU-first milestone and avoids treating production memory as a flat array when the target hardware uses mapped regions and registers. It keeps the intended production model grounded in Game Boy behavior without requiring all hardware components at the outset.

### Consequences

- **Easier:** Implementing only the mappings needed at each development stage; integrating additional hardware as introduced.
- **Limitations:** Until relevant mappings exist, behavior for those address ranges is incomplete. Temporary behavior is explicit and documented per region, not assumed to match final hardware behavior.
- **Possible future changes:** New components may require changes to routing and bus organization. Exact mappings must follow hardware behavior rather than convenience assumptions.

### Professional systems-development relevance

Memory mapping is fundamental in embedded systems and computer architecture: an address may refer to RAM, ROM, or a hardware register depending on the system's map. Incremental implementation is a project sequencing choice, not a claim that the hardware map itself is optional.

### Open questions and assumptions

- The first CPU milestone includes cartridge ROM, VRAM, cartridge RAM, WRAM, OAM, I/O, HRAM, and IE.
- Temporary behavior for unimplemented addresses is documented per region; the exact behavior remains to be specified during implementation.
- DMG access restrictions are applied as relevant hardware is added.
- The fake bus shares the compile-time bus contract and uses independent test storage.

## ADL-005: Interrupt Register Access

**Status:** Approved

### Context

The CPU must observe enabled interrupt requests and service interrupts according to Game Boy CPU state. The architecture needs to clarify whether interrupt registers use normal memory operations or special bus methods, and distinguish request generation from CPU servicing.

### Options considered

- **Ordinary bus operations:** CPU reads and writes the memory-mapped registers through the normal address path. This follows the mapped-register model but requires the bus to implement those addresses correctly.
- **Dedicated interrupt methods:** Gives interrupt handling a specialized API, but separates those register accesses from ordinary address-based memory operations.
- **Ordinary access plus helpers:** Retains address-based register access while adding convenience methods; this creates a larger interface and may duplicate access paths.

### Proposed decision

The `Bus` maintains dedicated register state for **Interrupt Enable (IE)** at `0xFFFF` and **Interrupt Flag (IF)** at `0xFF0F`, with correct read/write semantics. The CPU accesses them through ordinary bus operations and checks enabled pending requests using `IE & IF`, subject to its interrupt master enable (IME) state. Initial interrupt handling includes priority, acknowledgement, IME, EI's delayed effect, stack push, and vector jump. Interrupt request generation remains separate from CPU interrupt servicing.

### Rationale

IE and IF are memory-mapped registers on the original Game Boy, so the CPU's ordinary bus operations preserve the same access path used for other mapped locations, while the Bus keeps their dedicated state and implements their read/write semantics. Separating request generation from CPU servicing distinguishes hardware setting a request from the CPU's decision to respond. Explicitly modeling delayed EI behavior and service effects avoids treating IME changes as an instantaneous generic operation.

### Consequences

- **Easier:** A consistent address-based access path; CPU interrupt service can be tested against explicit IE/IF state and the fake bus.
- **Limitations:** Correctness depends on register read/write semantics and CPU service sequencing matching DMG behavior. `IE & IF` identifies enabled pending requests but does not by itself define the complete service sequence.
- **Possible future changes:** Timing refinements may be needed as CPU execution and peripheral behavior become more accurate.

### Professional systems-development relevance

This separates device-side event generation from processor-side handling, a common distinction in operating systems and embedded systems. It also keeps hardware register semantics visible in the memory map rather than hiding them behind a separate software-only channel.

### Open questions and assumptions

- The initial implementation includes priority selection, acknowledgement, IME state, delayed EI effect, stack push, and vector jump.
- IME is CPU state; EI's delayed effect is modeled explicitly.
- Exact edge cases and timing details must follow DMG hardware behavior and remain implementation-level details.
