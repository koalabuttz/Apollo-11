# Complete native C64 port scope

The goal is the **entire Luminary099 lunar-module program and entire Comanche055
command-module program**, translated to native C for the Commodore 64. A selected
flight routine or a collection of verified primitives does not complete this
scope. Original historical source files remain byte-for-byte unchanged.

The machine-readable inventory is [port-coverage.json](port-coverage.json). It
records every original file's SHA256, byte count and source line count against
repository revision `911e5c0283c629c50cb97666f34065e8c07d71a5`. There are 175 AGC
files plus the two source-tree READMEs. Counts include comments and declarations;
they measure the source corpus, not translated executable instructions.

| Program | AGC files | Source lines | Untranslated | Partial | Complete |
|---|---:|---:|---:|---:|---:|
| Luminary099 | 90 | 64,838 | 76 | 14 | 0 |
| Comanche055 | 85 | 65,348 | 85 | 0 | 0 |

`partial` means a declared bounded path or data definition has a native
implementation and verification artifacts. It does not imply that the rest of
its file, callers or device interactions are translated. For example, the
SERVICER entry covers only the separately checked `4SEC(28)` constant. The
RESTART_TABLES file remains untranslated: including original assembly in a host
fixture does not translate it. Comanche files remain untranslated even when they
look similar to a Luminary implementation; each needs its own source mapping and
verification before coverage is claimed.

The C64 target must execute native functions and native continuations. An AGC
instruction decoder, an AGC emulator running on the C64, or shipping the original
rope as executable bytecode would not meet this goal. The pinned host assembly
engine remains an independent numerical/control-flow oracle used during
verification, outside the C64 application.

Both integrated programs need documented C64 adapters for their original input
roles, including elapsed time, inertial/optical/radar measurements, discrete
signals, operator commands and communication. Their guidance, navigation,
attitude, engine/jet, display and communication outputs must come from native
translated calculations and flow through tested adapters. Fixtures, constant
outputs and callback placeholders do not implement the corresponding flight
paths. Software acceptance on a C64 does not by itself establish qualification
for physical spacecraft hardware.

Speed and real time are not acceptance criteria. A stock C64 runs the flight
programs about 10-20 times slower than the AGC (each AGC centisecond of a run
takes 10-20 centiseconds on the C64). Acceptance compares what the C64 computes
and displays against the host and yaAGC on the same inputs, on the AGC's own
clock (the C64 charges each step's AGC time, it does not follow the wall
clock), so a slow run that matches is accepted and a fast run that differs is
not. Speed work continues, but no acceptance check waits on it.

A complete memory plan is still required. The AGC programs exceed a naive C64
resident-memory budget when translated without planning. Disk overlays may be
necessary, with persistent contexts and explicit rules for reloads, interrupts,
scheduled work and restart. The inventory records this architecture as
unresolved rather than choosing an untested overlay design. An accepted plan
must account for code, constants, data, hardware/software/compiler stacks,
reserved I/O memory and every resident or loaded program phase.
The flight build's plan is the thin resident plus overlays loaded from a
2 MB REU store (docs/FLIGHT-STORE.md); each program's
`docs/acceptance/<Program>-memory_and_overlay_validation.md`, written by
`tools/memory_acceptance.py`, records its layout, build-time checks and
runtime REU bound checks.

Run the inventory and immutable-source audit from the repository root:

```sh
python3 c64-port/tools/port_coverage.py
python3 c64-port/tools/port_coverage.py --list-remaining
```

The completion gate is:

```sh
python3 c64-port/tools/port_coverage.py --require-complete
```

It intentionally exits with a failure today. All 175 AGC files still require
complete native coverage, and integrated LM and CM C64 applications have not
passed acceptance. Routine development checks use the audit without
`--require-complete`; this distinction does not narrow the completion goal.

A file can become `complete` only with a reviewed `complete_native_coverage`
certificate tied to the immutable source revision. Set
`reviewed_all_source_paths` to true after reviewing the entire file, then provide
SHA256-bound evidence in each required category:

- `source_to_native_map`: every original path and data definition mapped to
  native code or a justified equivalent native representation.
- `independent_assembly_comparison`: independent comparisons covering the mapped
  paths, their inputs, signed zeros, stores, overflow and control effects.
- `host_and_sanitizer_results`: native implementation and meaningful boundary
  and integration checks.
- `c64_execution_results`: actual native C64 execution checks for these paths.

Each evidence record contains a path relative to `c64-port` and its `sha256`.
The tool checks that evidence exists within the port directory and matches its
recorded hash. These checks support a review; metadata and file hashes alone do
not prove semantic correctness. A reviewed coverage certificate must not be
created merely to make the gate green.

In addition, both `integrated_applications` entries must be complete, have
`reviewed_entire_program: true`, and supply SHA256-bound evidence for a bootable
C64 application, integrated program tests, device-adapter tests, native-output
tests, memory/overlay validation and recovery/restart tests. Complete-file
certificates cannot substitute for this integrated application acceptance.

The inventory is an honest completion checklist. Continue adding independently
verified milestones and commits, update their declared coverage, and retain
untranslated and partial classifications until the corresponding scope is met.
