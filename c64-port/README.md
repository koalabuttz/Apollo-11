# Apollo 11 — native Commodore 64 ports

The target is complete native C64 implementations of both Luminary 099 and
Comanche 055. Verified Executive and arithmetic paths are implemented; the
complete mission applications remain under development. See
[the full scope and coverage inventory](docs/PORT_SCOPE.md).
The C64 binary contains no AGC instruction decoder or AGC emulator. Virtual AGC
is used only on the host as a verification oracle.

**Implemented:** NOVAC/FINDVAC admission and allocation; ENDJOB1/EJSCAN;
basic CHANG1/CHANJOB context exchange and completion through basic dispatch;
explicit historical-2CADR mappings to native C continuations; platform interrupt
exclusion and bounded IRQ admission requests; basic JOBSLEEP/JOBWAKE and
PRIOCHNG. Native demonstration jobs run, sleep, wake, change priority, resume
and complete. WAITLIST now retains nine task slots, differential timer state
and native task completion; a CIA Timer B adapter publishes clock events.
Bounded alarm recording, STARTSB1 cleanup and protected Type C job recovery
are implemented. P66 RODCOMP's rate update, PIPA/time snapshots and vector-input
preparation, VXSC KPIP1 scaling and PDDL THISTPIP vector pushdown/timestamp
load, DSU PIPTIME elapsed-time subtraction, STORE 30D and fixed-divisor DDV 4SEC(28) are verified
(docs/GUIDANCE.md, docs/PREPARATION.md, docs/SCALING.md,
docs/PUSHDOWN.md, docs/ELAPSED.md, docs/STORE.md, docs/DIVIDE.md). PDVL GDT/2 now pushes the quotient and loads the gravity vector (docs/GRAVITY.md). **Deferred:** complete Apollo
remaining guidance jobs, complete interpretive operation coverage and
remaining restart routes/glue (docs/RESTART.md).

26 original admission/completion scenarios and 53 dispatch cases (109 checkpoints,
including eight sequential two-job executions) agree word-for-word with original
assembly. The native C64 diagnostic runs the same jobs and checks all 135
original-derived state fingerprints in VICE. A separate real raster-IRQ diagnostic
checks interrupted C, register preservation, bounded delivery and overflow.
A third diagnostic checks 81 lifecycle cases / 115 checkpoints, five native
sequences and an IRQ-admitted waker. These are bounded behavioral checks,
not a complete flight Executive or real-time equivalence claim. WAITLIST adds
196 cases / 306 checkpoints, ten native task sequences and a fourth C64
diagnostic with six real CIA Timer B IRQs. The PAL adapter samples two
centiseconds per event; foreground delivery and masking limits are documented.

## Repository

Upstream: https://github.com/chrislgarry/Apollo-11

Pinned base: `911e5c0283c629c50cb97666f34065e8c07d71a5`.
Development branch: `port/c64-native-c`. All changes are under `c64-port/`.
The full upstream history and historical `.agc` files are retained unchanged.
Fork: https://github.com/koalabuttz/Apollo-11
The development branch is published on that fork; master retains the pinned base.
See `docs/HANDOFF.md` and `docs/GRAVITY.md` for the current milestone.

## Build and run

From `c64-port/`, with GCC/Clang, Make, Python 3, and LLVM-MOS installed:

```sh
make test
ASAN_OPTIONS=detect_leaks=0 make sanitize
make c64 MOS_CC=/path/to/llvm-mos/bin/mos-c64-clang
make c64-irq MOS_CC=/path/to/llvm-mos/bin/mos-c64-clang
make c64-lifecycle MOS_CC=/path/to/llvm-mos/bin/mos-c64-clang
make c64-waitlist MOS_CC=/path/to/llvm-mos/bin/mos-c64-clang
make c64-restart MOS_CC=/path/to/llvm-mos/bin/mos-c64-clang
make c64-guidance MOS_CC=/path/to/llvm-mos/bin/mos-c64-clang
make c64-prepare MOS_CC=/path/to/llvm-mos/bin/mos-c64-clang
make c64-scale MOS_CC=/path/to/llvm-mos/bin/mos-c64-clang
make c64-push MOS_CC=/path/to/llvm-mos/bin/mos-c64-clang
make c64-elapsed MOS_CC=/path/to/llvm-mos/bin/mos-c64-clang
make c64-store MOS_CC=/path/to/llvm-mos/bin/mos-c64-clang
make c64-divide MOS_CC=/path/to/mos-c64-clang
```

The deliverables are `build/executive.prg`, `build/irq.prg`,
`build/lifecycle.prg`, `build/waitlist.prg`, `build/restart.prg`, `build/guidance.prg`,
`build/prepare.prg`, `build/scale.prg`, `build/push.prg`, `build/elapsed.prg`, `build/store.prg` and `build/divide.prg`
(ordinary C64 PRGs with BASIC SYS stubs).
Load a PRG on a C64 and RUN, or use VICE. The console diagnostic prints a test summary,
not an interactive DSKY. Target compiler: LLVM-MOS SDK **v23.3.0**.
Download the pinned release from the URL in `tools/versions.json`, verify its
SHA256, and extract with `tar --no-same-owner -xJf llvm-mos-linux.tar.xz`.
Toolchains and C64 ROMs are not bundled in this repository.

### Execute the original assembly oracle

```sh
python3 tools/prepare_reference.py
make differential REFERENCE="$PWD/deps/fixture"
```

This builds pinned yaYUL/yaAGC and assembles a full reference fixture using
Virtual AGC's corrected supporting transcription, with the requested upstream
scheduler, interrupt, alarm, restart and selected guidance source files copied
**byte-for-byte**. The selected repository's full MAIN
cannot currently assemble unmodified; details are in `docs/VERIFICATION.md`.
No native port implementation is linked into the Executive reference runner.

The test checks full saved state and selected basic dispatch addresses, including
actual BBANK/SUPERBNK transfers, rather than just the small manual assertions. Arithmetic
is also compared against Virtual AGC's numerical helper API. MP/DAS and the
P66 rate update add 12,856 actual-instruction checkpoints; the sixth C64 PRG
checks 568 complete six-word states and three rate histories. PIPA/time and vector
preparation add 3,396 checkpoints, including every six-TS skip combination;
the seventh PRG checks 324 complete 39-word states and composed histories.
VXSC KPIP1 adds 3364 complete 41-word checkpoints; the eighth PRG checks 292
states, signed rounding boundaries and full-prefix histories.
PDDL THISTPIP adds 3496 complete 86-word checkpoints and a ninth PRG with 200
complete final-state checks, all integrated skip inputs and retained vector histories.
DSU PIPTIME adds 4664 complete 89-word checkpoints and a tenth PRG with 172
complete states, both signs of all overflow paths and three retained histories.
STORE 30D adds 5044 complete 89-word checkpoints and an eleventh PRG with 176
full final states, retained histories and source/destination signed-zero corners.
DDV adds 8098 complete 93-word checkpoints and a twelfth PRG with 156 states.
Its host oracle uses the pinned engine's hardware DV model; the fast-path signed-zero
discrepancy and two-model audit are documented in docs/DIVIDE.md. Earlier oracles
remain unchanged.
Regenerate frozen
C64 expectations only after a successful differential test:

```sh
python3 tools/freeze_vectors.py
python3 tools/freeze_lifecycle.py
python3 tools/freeze_waitlist.py
python3 tools/freeze_restart.py
python3 tools/freeze_guidance.py
python3 tools/freeze_prepare.py
python3 tools/freeze_scale.py
python3 tools/freeze_push.py
python3 tools/freeze_elapsed.py
python3 tools/freeze_store.py
python3 tools/freeze_divide.py
make test c64 c64-irq c64-lifecycle c64-waitlist c64-restart c64-guidance c64-prepare c64-scale c64-push c64-elapsed c64-store c64-divide
```

### VICE verification

```sh
python3 tools/vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/VICE/data/C64 --data-dir /path/to/VICE/data
python3 tools/irq_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/VICE/data/C64 --data-dir /path/to/VICE/data
python3 tools/lifecycle_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/VICE/data/C64 --data-dir /path/to/VICE/data
python3 tools/waitlist_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/VICE/data/C64 --data-dir /path/to/VICE/data
python3 tools/restart_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/VICE/data/C64 --data-dir /path/to/VICE/data
python3 tools/guidance_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/VICE/data/C64 --data-dir /path/to/VICE/data
python3 tools/prepare_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/VICE/data/C64 --data-dir /path/to/VICE/data
python3 tools/scale_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/VICE/data/C64 --data-dir /path/to/VICE/data
python3 tools/divide_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
python3 tools/store_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
python3 tools/elapsed_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
python3 tools/push_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/VICE/data/C64 --data-dir /path/to/VICE/data
```

Provide standard ROM files `kernal-901227-03.bin`, `basic-901226-01.bin`, and
`chargen-901225-01.bin`. The helper uses the console monitor, loads the PRG after
BASIC boot, runs native code, and checks a RAM completion mailbox. A cycle limit
or missing completion record fails the test. The helpers record timing and stack
sentinel observations; the IRQ, lifecycle and WAITLIST helpers write structured timing
reports and reject unpreserved compiler ZP allocations. No display server is required.

The fifth diagnostic checks 190 alarm/restart cases / 226 checkpoints and six
recovered native job completions. Original alarm records and protected phases
are retained; this is a bounded recovery adaptation.
The sixth diagnostic checks 568 complete numerical/rate states and three P66
rate histories, with six measured call intervals and 12 bytes of rate state.
The seventh diagnostic checks 324 complete preparation states, all six-TS skip
combinations and three composed histories, with 78 bytes of modeled state.
The eighth checks 292 complete scaling states with 82 bytes of total state.
Its largest measured sample takes 26.122 ms; see docs/SCALING.md for timing limits.
The ninth checks 200 final pushdown/timestamp states with 172 bytes of total state.
PDDL-only samples reach 1.067 ms; the sampled full prefix takes 21.101 ms.
See docs/PUSHDOWN.md for scope, scratch and ownership.
The tenth checks 172 final elapsed-time states with 178 bytes of total state.
DSU-only samples reach 1.402 ms; the full prefix takes 21.748 ms.
See docs/ELAPSED.md for overflow correction and the boundary before STORE 30D.
The eleventh checks 176 complete stored-time states with the same 178-byte context.
STORE-only samples reach 0.322 ms; full prefix 22.168 ms. See docs/STORE.md.
The twelfth checks 156 complete division states with 186 bytes of context.
DDV samples reach 6.519 ms; full prefix 28.358 ms. See docs/DIVIDE.md.

## Engineering record

- `docs/DISPATCH.md`: native callback API, context swaps, sequential reference boundaries.
- `docs/IRQ.md`: exclusion, queue ownership, fault boundaries and C64 IRQ ABI.
- `docs/LIFECYCLE.md`: sleep/wake, priority scans, callback actions and oracle boundaries.
- `docs/RESTART.md`: alarm history, cleanup, protected job re-entry and exclusions.
- `docs/GUIDANCE.md`: P66 rate boundary, raw scales, multiword math and oracle domains.
- `docs/DIVIDE.md`: fixed-divisor DDV, exact scratch/register effects and hardware-DV oracle audit.
- `docs/STORE.md`: STORE 30D, relative destination, old-word register effects and overlap semantics.
- `docs/ELAPSED.md`: DSU PIPTIME, signed-zero/carry behavior, overflow realignment and retained state.
- `docs/PUSHDOWN.md`: PDDL THISTPIP, owned VAC payload, pointer bounds and exchange debris.
- `docs/SCALING.md`: VXSC KPIP1, partial products, signed rounding, scratch and interpreter-entry adaptations.
- `docs/PREPARATION.md`: PIPA/time snapshots, vector scratch, TS skips and the pre-interpreter boundary.
- `docs/WAITLIST.md`: task list/counter semantics, native callbacks, CIA ownership and limits.
- `docs/ARCHITECTURE.md`: original structures, calling conventions, dependencies.
- `docs/SOURCE_MAP.md`: original labels mapped to C and reference stop boundaries.
- `docs/FIDELITY.md`: classifications, adaptations, and unsupported behavior.
- `docs/VERIFICATION.md`: executed commands, results, reference provenance.
- `docs/PERFORMANCE.md`: measured native size, RAM, stack, and timing limits.
- `docs/ROADMAP.md`: next bounded milestone.
- `evidence/`: retained snapshots, logs, and native binary.
- `tools/versions.json`: exact source/tool/ROM identities and hashes.

The historical source remains public domain with its original attribution.
New native port code is MIT licensed. The host-only Virtual AGC adapter is
GPL-2.0-or-later; it is never linked into the C64 PRG. See `LICENSE`.

PDVL build/test: `make test-gravity c64-gravity`; original comparison:
`python3 tools/gravity_differential.py deps/fixture`. The C64 diagnostic compares
156 complete 99-word states using lossless compact expectations.

## Verified native VAD/VSU

Reusable vector arithmetic now matches 13,600 original-assembly checkpoints.
128 complete states pass host, sanitizer and C64 diagnostics. See `docs/VECTOR.md`.

## Verified SPVAC and CHANG2

Staged VAC admission and interpretive context exchange match 768 complete
original-assembly checkpoints. Host/sanitizer tests and all 768 C64 state
fingerprints pass. Native mixed-context execution is a separate integration
stage. See `docs/EXECUTIVE_SERVICES.md`.

## Translate native single and double phase-table changes

All 49,152 complete original-assembly states match; 128 host/sanitizer/C64 states pass. See `docs/PHASE.md`.

## Translate arbitrary-factor vector scaling and unrounded DP products

All 8,192 complete assembly checkpoints and 128 host/sanitizer/C64 states pass. See `docs/PRODUCT.md`.

## Translate scalar and vector shifts with original rounding and scratch effects

All 26,880 complete assembly states and 168 host/sanitizer/C64 states pass. See `docs/SHIFT.md`.

## Translate general double-precision DDV and backward division

All 163,840 original-assembly comparisons and 573 host/sanitizer/C64 states pass using the hardware DV model. See `docs/GENERAL_DIVIDE.md`.

## Translate complete P66 updated-velocity calculation through STOVL and position load

All 4,648 complete assembly checkpoints pass; all 156 C64 states pass across two RAM-safe images. See `docs/VELOCITY.md`.

## Translate scalar square root and normalized root with source error boundary

All 132,096 assembly comparisons and 637 host/sanitizer/C64 states pass; negative inputs stop at the declared pre-1302 boundary. See `docs/NORM.md`.

## Translate abort flags and selected restart-group cleanup paths

All 49,152 complete assembly comparisons and 128 host/sanitizer/C64 states pass. See `docs/RECOVERY_CLEANUP.md`.

## Translate unrounded triple-precision dot product and overflow accumulation

All 10,272 complete assembly comparisons and 96 host/sanitizer/C64 states pass. See `docs/DOT.md`.

## Translate scalar sums and backward vector subtraction with exact overflow effects

All 27,200 complete assembly comparisons and 256 host/sanitizer/C64 states pass. See `docs/SUM.md`.

## Execute native basic and interpretive continuations through original context exchanges

Native C callbacks now enter and exit interpretation, yield at completed operation pairs, restore interpreter payloads, and resume through the original Executive lifecycle. All 140 full original-assembly checkpoints agree; host and sanitizer tests and the PAL C64 diagnostic pass, including masked repeats and an actual raster IRQ admission. This milestone supplies scheduling plumbing; remaining guidance bodies still need translation and binding. See `docs/INTERPRETER_NATIVE.md`.

## Translate ENEMA through restart validation and source-defined exit boundaries

The native restart entry composes the verified cleanup and phase validation routines through the original MMDSPLAY, PTBAD, and fresh-start request boundaries. All 4,096 full 457-word original-assembly comparisons agree, and 128 complete C64 target states pass with host and sanitizer verification. Subsequent display and restart execution remain to be completed. See `docs/ENEMA.md`.

## Translate Comanche Executive admission and release with seven native core sets

A separate Comanche 055 Executive retains its seven core sets, five VACs, and source BAILOUT boundaries. All 1,536 source-derived assembly comparisons agree; host and sanitizer tests and the C64 run pass, including 40 complete-state samples. The generated fixture documents five indentation repairs and matches the pinned corrected full rope; historical repository files remain unchanged. CM dispatch and lifecycle are next. See `docs/COMANCHE_EXECUTIVE.md`.

## Translate absolute value, vector length and half-unit normalization with signed-zero fidelity

ABS, ABVAL and UNIT now preserve raw length, half-unit vector, overflow, and scratch effects. All 50,736 complete 104-word assembly comparisons agree, including sixteen signed-zero regressions found by P66 composition. Host and sanitizer tests and all 118 full-state C64 checks pass. Full application binding and remaining flight routines are still in progress. See `docs/UNIT.md`.

## Compose native P66 radial velocity and predicted altitude from verified arithmetic

The P66 radial group now computes half-unit radius, radial velocity and predicted altitude with original VAC, signed-zero and scratch effects. All 4,952 complete 134-word original-assembly checkpoints agree; full-prefix host and sanitizer checks pass. Six C64 images verify 203 complete stage outputs from exact original pre-UNIT states. The full-prefix C64 diagnostic exceeds RAM, so whole-program overlay integration remains required. See `docs/RADIAL.md`.

## Translate P66 desired radial acceleration and gravity through the rotation boundary

The native P66 acceleration group preserves original VAC, scalar division, vector magnitude, gravity scaling and implicit stack pop effects through DANZIG before CDU rotation. All 16,384 full-stage assembly states and the 7,428-checkpoint composed host prefix agree. Strict host, sanitizer and 96 full-state C64 stage checks pass; whole-program overlay execution remains in progress. See `docs/ACCELERATION.md`.

## Execute bounded native disk overlays with canonical erasable storage on the C64

A resident Executive and loader execute separately linked native C payloads from a D64 image, swap and reload them after ordinary C returns, reset transient BSS, and preserve the 2,048-word canonical source image and platform mappings. Host bounds and error checks, sanitizers and the stock PAL C64 disk-loading proof pass. The demonstrated payloads test the ABI; full mission module allocation, media handling and guidance integration remain unfinished. See `docs/OVERLAYS.md`.

## Fix source-exact WAITLIST positive-zero control flow and final-slot insertion

WAITLIST now skips interval additions on early CCS positive zero and follows the final positive-zero insertion route instead of raising a false full-queue alarm. The 256-case full-word regression detects 688 previous field errors and passes after the fix; the original 196-case suite remains unchanged and passes. Host, sanitizer and all 256 C64 states agree with original assembly. See `docs/WAITLIST_ZERO.md`.
