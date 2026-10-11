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

## Execute Comanche basic and interpretive jobs through source-exact lifecycle transitions

The separate seven-core CM runtime now dispatches native C callbacks, exchanges basic and interpreted contexts, sleeps and wakes jobs, and changes priorities through original Comanche semantics. All 876 complete source-derived checkpoints and twenty real native sequences agree; host and sanitizer checks and four stock C64 diagnostics pass. IRQ coordination remains caller-owned pending the dedicated adapter. See `docs/COMANCHE_DISPATCH.md`.

## Translate P66 operator counts, task admission and phase-timer entry paths

DESCBITS, RODTASK and P66VERTA now follow original operator arithmetic, priority-22 admission, phase termination, delayed task scheduling and immediate RODCOMP entry with exact shared scratch. All 73,728 original-entry checkpoints agree; host, sanitizer and 128 full-state C64 checks pass. STRTP66 bias initialization, display admission and later continuations are separate ongoing milestones. See `docs/P66_STARTUP.md`.

## Port exact native trigonometric services

All 65,536 complete source states match CDULOGIC, SIN, COS and CDUTRIGS; all-state sanitizers and 128 frozen C64 cases pass. The native fixed-point coefficients and overflow behavior retain the original instruction semantics. See `docs/TRIG.md`.

## Run live P66 continuations from native disk overlays

The canonical 2,048-word image drives actual compiled arithmetic callbacks through the mixed Executive. All 5,887 source checkpoints pass; the C64 disk proof passes 72 full-state fingerprints including two real P66 jobs, both IRQ states and SUPERBNK forms. The current driver ends before UNIT and retains its core/VAC for further translation. See `docs/P66_NATIVE_DRIVER.md`.

## Port original native axis rotations

All 32,768 complete AX*SR*T source states match; strict and all-state sanitizer checks pass. C64 verification compares 128 full frozen states with source scratch aliases and both rotation directions preserved. See `docs/ROTATION.md`.

## Port live CDU projection and acceleration feedback

Live native trig and rotation feed DOT14 and AFC with original source scheduling boundaries. All 39,432 direct and full-prefix checkpoints match; entire-corpus sanitizers and all 144 frozen C64 checkpoints pass. See `docs/PROJECTION.md`.

## Implement exact native hardware divide recurrence

The independently authored fixed-point recurrence matches 327,680 isolated original hardware-DV instruction states, including raw register overflow and signed zero. Full-state sanitizers and all 255 frozen C64 register-pair checks pass. See `docs/HARDWARE-DIVIDE.md`.

## Port source STOPRATE and FASTCHNG initialization services

All 1,024 complete source checkpoints match including return-register debris, physical banks and IRQ state. Strict and sanitizer checks and 128 complete frozen C64 states pass. See `docs/P66_INITIALIZATION.md`.

## Port original force feedback and throttle clamps

All 16,384 complete force-feedback states match unchanged assembly with all three clamp paths covered. Full-state sanitizers and 64 complete frozen C64 cases pass. See `docs/THROTTLE_FEEDBACK.md`.

## Port complete bounded native THROTTLE service

THROTTLE+3 through THDUMP matches all 16,384 complete 147-word source states, including hardware-DV weighting, force saturation, critical branches, FASTCHNG and pulse commands. Strict and all-state sanitizer checks and 96 complete C64 states pass; the enclosing return bridge is a separate continuation. See `docs/THROTTLE-SERVICE.md`.

## Port canonical whole-image P66 bias initialization

All 1,024 original STRTP66A entry cases match the complete 2,048-word erasable image plus physical SUPERBNK/IRQ. Strict and sanitizer ownership checks and 64 complete whole-image C64 states pass. See `docs/P66_BIAS.md`.

## Port canonical vertical-guidance startup through RODCOMP

All 1,024 complete original source images match through STOPRATE, flag updates, phase termination, timer admission and POSTJUMP. Strict and sanitizer checks and 64 whole-image C64 cases pass, including source resource bailouts. See `docs/P66_VERTICAL_START.md`.

## Port Comanche timer queue and native task execution

All 306 complete original WAITLIST checkpoints match, including zero-sign branches, source task transfers and 01203/01204 resource alarms. Strict, sanitizer and C64 checks cover all checkpoints, ten actual native callback sequences and ownership guards. See `docs/COMANCHE_WAITLIST.md`.

## Connect Comanche Executive to real native C64 interrupts

Exact source admission and callback states verify the CM runtime adaptation. C64 checks pass 512 admission fingerprints, seven real raster interrupts and 15 full callback states while preserving native registers, compiler storage, caller IRQ state and KERNAL coexistence. See `docs/COMANCHE_IRQ.md`.

## Port real P66 major-mode display job admission

NEWMODEX matches 1,024 complete original erasable images, admitting the actual DSPMMJB basic job. Strict, sanitizer and C64 checks pass 64 full images; deferred display formatting and consumption remain separate continuations. See `docs/P66_MAJOR_MODE.md`.

## Compose complete native STARTP66 through RODCOMP

The complete source landing-start sequence matches 1,024 whole-memory entry states, with actual display-job and timer admission and both resource bailouts. Strict and sanitizer checks and 32 complete C64 images pass. Deferred display execution and the remaining landing cycle are separately tracked. See `docs/P66_LAUNCH.md`.

## Port complete native horizontal velocity arithmetic

All 65,536 original horizontal-velocity checkpoints match, preserving UNIT overflow continuations and physical bank effects. Strict and all-state sanitizer checks and all 256 complete frozen C64 states pass. See `docs/HORIZONTAL.md`.

## Port actual P66 major-mode display formatting

DSPMMJB matches all 1,024 original whole-memory states across every decimal mode and relay dirty/clean branch. Strict and sanitizer checks and 64 complete C64 images pass through the actual ENDOFJOB entry; relay drain remains a separate service. See `docs/P66_MODE_DISPLAY.md`.

## Port original canonical throttle call and return bridges

The actual source EXIT/BANKCALL and THDUMP/SWRETURN/INTPRET paths match all 8,192 complete erasable images. Strict and all-state sanitizer comparisons and 64 whole-image C64 checks pass, with unsupported-boundary no-mutation guards. See `docs/THROTTLE_BRIDGE.md`.

## Port Comanche phase records and restart foundation

CM PHASCHNG phase handling, STARTSB2/STARTSUB cleanup, GOPROG3 and Type C restart admissions match all 50,432 complete source checkpoints. Strict, sanitizer and stock C64 checks pass; actual restart-selected execution is a separate continuation. See `docs/COMANCHE_PHASE.md` and `docs/COMANCHE_RESTART.md`.

## Execute Comanche restart-selected native callbacks

All 320 complete original restart/dispatch states match across 64 source sequences and 192 actual native callbacks. Strict, sanitizer and stock C64 checks preserve caller frames and channel11 readback. The documented test callback bodies establish runtime composition; arbitrary mission bodies still require translation. See `docs/COMANCHE_RESTART_NATIVE.md`.

## Bind Comanche runtime to the canonical erasable image

The sole 2,048-word image supplies CM Executive, WAITLIST, phase and restart views. Poison/regather tests preserve all 320 source checkpoints and execute 192 native callbacks; strict, sanitizer and stock C64 checks pass without a second persistent guidance image. See `docs/COMANCHE_NATIVE_IMAGE.md`.

## Port Comanche ENEMA error and fresh-start paths

All 4,096 complete original ENEMA states match across 439 owned words, including navigation/error combinations, counter startup and conditional TVCEXEC scheduling. Strict and sanitizer checks and stock C64 fingerprints plus full-state samples pass; downstream selected mission tasks remain separate translation work. See `docs/COMANCHE_ENEMA.md`.

## Split source-faithful guidance into returning native phases

All 97,558 complete UNIT/radial/projection source states and full sanitizer corpora match. All 480 C64 phase entry/exit cases pass with zero extra zero page. Internal C ABI returns retain ownership; scheduling occurs only at original source boundaries. See `docs/GUIDANCE_PHASED.md`.

## Port original DSKY relay drain and channel10 output

All 1,024 whole-memory DSPOUTSB source states and actual channel10 output events match. Strict and sanitizer checks and 64 complete C64 images pass; the native screen adapter and live application wiring remain separate integrations. See `docs/DSKY_RELAY.md`.

## Render actual native DSKY relay output on the C64

The source mode66 display job and channel10 relay drain now drive real C64 screen/color RAM. Host, sanitizer and target checks verify numeric/sign latches, lights, blanking and the128cs flash cycle; live application keyboard/device integration remains pending. See `docs/DSKY_DISPLAY.md`.

## Port original Comanche startup and cleanup entries

SLAP1/DOFSTART and MR.KLEAN/P00KLEAN/V37KLEAN match all640 complete source checkpoints, preserving2047 image words and15 external I/O fields with native Z boundary qualification. Strict and sanitizer checks and640 C64 fingerprints plus five full-state samples pass. See `docs/COMANCHE_STARTUP.md`.

## Split acceleration at source boundaries and preserve return control

All65,536 complete acceleration phase states and65,536 independent source Q observations match. Full-state sanitizers and128numerical/65control frozen C64 cases pass. Separate native phase APIs support genuine source scheduling boundaries. See `docs/ACCELERATION_PHASED.md`.

## Port genuine P66 velocity scheduling boundaries

Three native velocity groups stop at source LOC03366,03370 and03373 with EDOP zero. All13936 complete118-word source checkpoints match SHA977279d21f48ea76643d805554338b2d4fb8fb9b8da9f72a80952c9a2d6c912d; strict/sanitizer checks and96 actualC64 entry/exit states pass. LOC03371 retains pendingSTADR and cannot hand off. C64 images17922/17035/16086bytes use no additional zero page, at most20 hardware and47 software stack bytes. The owning canonical driver supplies scheduling/control state and producer exclusion. See `docs/P66-VELOCITY-PHASES.md`.

## Port P66 feedback through six source-aligned overlays

All98304 complete124-word original feedback checkpoints match SHA2681746eff8644b4b4ff08531cc7cc9d4d1eddd11d8cc943ce30d98a7c76bd76. Strict192 source entry/exit states and196602 invalidMODE checks, full-source sanitizers andsix actualC64 images pass. Source LOC03437 still has pendingPDDL inEDOP00052 and retains job ownership; canonical scheduling applies only at genuine completed source boundaries. Equivalent raw-sign predicates preserve the complete earlier monolithic source corpus. See `docs/P66-PHASED-FEEDBACK.md`.

## Port P66 horizontal-exit display control

All8192 complete2048-word source images plusSUPER/IRQ match SHA b77721d0f877bf7d71452a34837d05cc72914aadc78fbc8dc22759d0a251571b. Native EXIT/BANKCALL/DISPEXIT+3 follows suppression to actualENDOFJOB or enabled output to originalREGODSPR, preserving NV/BUF2 and source registers. Strict64 states/14 invalid states, full-source sanitizers andactualC64 64 states pass; PRG4178bytes, zero additional zero page, hardware14 andsoftware12 stack bytes. See `docs/P66-DISPLAY-CONTROL.md`.

## Port actual P66 REGODSPR display-job admission

All8192 complete2048-word source images plusSUPER/IRQ match SHA d1d8bd7dea68056cc34531153360729db463ee886ee7999014e2b61a8ee62fba. Native REGODSPR/NOVAC/SAVELOCR/BANKJUMP copies NV/MPAC2/CADR/flags into originalMAKEPLAY job storage and reaches actualENDOFJOB;1024 no-core cases reach originalBAILOUT1. Strict64 states/18 invalid states, full-source sanitizers andactualC64 64 states pass; PRG11904bytes, zero additional zero page, hardware18/software678 stack bytes. Display formatting follows in separate native source services. See `docs/P66-REGODSPR.md`.

## Preserve separate P66 STORE20 and acceleration-add handoffs

The separate STORE20 and DAD groups stop at genuine source LOC03421 and03422. All 81,920 complete 112-word original checkpoints match SHA d0b4c030a35450fb4ea75dbc906600ad163b041b004060473d4430fab002788d; strict 64 source states, 65,534 invalid MODE cases and full-source sanitizers pass. Two actual C64 images pass 32 original states each, with zero additional zero page and at most 14 hardware / 371 software stack bytes. The earlier combined sum API and retained numerical evidence remain unchanged. See `docs/P66-ACCELERATION-STORE.md`.

## Port original P66 noun-verb entry dispatch

Native NVMONOPT reaches UPDATVB-1 or the actual busy caller return. All 8,192 complete 2,048-word source images plus SUPER/IRQ match SHA 188f356b2c434205fc424a1eca0b54097d83576087b8209ea8f253276155c8fe. Strict 64 source states / 13 invalid entries, full-source sanitizers and 64 actual C64 source states pass. The 4,367-byte PRG uses no additional zero page and 14 hardware / 12 software stack bytes. Formatting and output continue in separately verified services. See `docs/P66-NV-ENTRY.md`.

## Port source P66 verb and noun update services

Original UPDATVB and UPDATNN for V06N60 match all 8,192 complete 2,048-word source images plus SUPER/IRQ, SHA 29c140f45d7a4d5ff1c2d3f2b18759bdc6e584c1370d196e9b6f071af3b4ece9. Strict 64 source states and 20 invalid entries, full-source sanitizers and 64 actual C64 source states pass. The 11,091-byte PRG uses no additional zero page and 33 hardware / 47 software stack bytes. This milestone preserves source continuation boundaries for later data formatting. See `docs/P66-NV-WORDS.md`.

## Port original DSKY relay character and blanking services

Native DSPIN,11DSPIN and2BLANK match all 4,096 complete 2,048-word original-source images plus SUPER/IRQ, SHA 65097a2bedb65f7a5ebf81f42b101107b2e51ced4ae5aed81fd510e05bef0180. The entry domain includes all eight retained erasable banks, ordinary caller returns and source character/sign/blank counts; original clean-positive-zero CCS holes remain explicitly excluded. Strict and sanitizer checks and 64 actual C64 source images pass. The 9,859-byte PRG uses zero additional zero page and 10 hardware / 9 software stack bytes. Earlier EB2-only proof artifacts are retained. See `docs/DSKY-PINBALL.md`.

## Port source five-digit signed DSKY decimal formatting

Original DSPDECWD and DSPDECNR match all 4,096 complete 2,048-word source images plus SUPER/IRQ across all eight retained erasable banks, SHA c10d492637f9b2fe2cc0fd4dd5a6513c7921ce92e442bdb09da09fea0476ccc2. Native formatting preserves rounded and unrounded sign handling and all five decimal characters through the actual caller return. Strict checks, the entire sanitizer snapshot stream and 64 actual C64 source images pass. The 16,051-byte PRG uses zero additional zero page and 13 hardware / 33 software stack bytes. Earlier EB2-only proof artifacts remain immutable. See `docs/DSKY-DECIMAL.md`.

## Port source DSKY double-precision output scaling

Original DP1,DP2 andDP3OUTSF match all 4,096 complete 2,048-word source images plus SUPER/IRQ, SHA 5f1679fb963ea1e13e4286e247b0f8bbb7ed4f1197b22c6af9489cc897ea472b. Native fresh READLO, mixed-sign TP agreement, DMP and rescaling reach actual DSPDCEND bank41. The documented domain retains all eight initial erasable banks and normal/mixed noun addressing. Strict and sanitizer checks and 64 actual C64 source images pass; PRG14,508 bytes, zero additional zero page, hardware13/software122 stack bytes. See `docs/DSKY-SCALE.md`.

## Port original DSKY two-digit noun-verb formatter

Native DSPDECVN matches all 4,096 complete 2,048-word source images plus SUPER/IRQ, SHA 284719c9194b0c106ff4c0e79ed0000adbb28450d928f9297c606afb51fdc924. Original SHORTMP consumes retained MPAC+2, formats values0..99 into adjacent source display characters and returns through the actual caller, with all eight retained erasable banks covered. Strict and sanitizer checks and 64 actual C64 source images pass. The 13,289-byte PRG uses zero additional zero page and 13 hardware / 40 software stack bytes. See `docs/DSKY-TWO-DIGIT.md`.

## Qualify shared native arithmetic against original Comanche assembly

Original Comanche SQRT,DDV andUNIT match 132,096 / 163,840 / 50,736 complete source states. Their unchanged native kernels and existing frozen vectors pass host/sanitizer checks and 1,328 complete C64 vectors. CM dispatch addresses, signed-zero hardware-DV oracle qualification and indentation-only original-source assembly repairs are audited separately; whole normalized-original CM rope matches the independently pinned assembly SHA2ba31de9291cd10fb351a64d261bae8514a1cb75b4651bfa6a135dfa821a2d79. This proves bounded arithmetic reuse; surrounding CM program coverage remains pending. See `docs/COMANCHE_NUMERICS.md`.

## Port Comanche-specific AX*SR*T rotation scratch semantics

Original Comanche TERM1TMP aliases BUF2 rather than Luminary MPAC+3/+4. A distinct native CM rotation preserves the original scratch and retained MPAC words. All 32,768 complete 131-word source states match, with strict host/sanitizer and 128 complete actual C64 states passing. The 45,659-byte diagnostic remains below ROM, uses no additional zero page and 23 hardware / 570 software stack bytes. Original CM and LM kernels retain separate verified semantics. See `docs/COMANCHE_ROTATION.md`.

## Port original Comanche RECTIFY and MINIRECT conic reset

Twelve explicit native groups reproduce original RECTIFY/MINIRECT position and velocity deviation shifts, conic stores, clears and RVQ return. All 24,064 complete 2,048-word source images match SHA 7b93fc96fd7e845b9fec26c64cbfd48e96ba80dc472e1990cd21590381faee23, with all-source sanitizer equality and 384 complete actual C64 phase states passing. Source DANZIG boundaries permit Executive handoffs; final RVQ returns to NEWOPS without introducing a job check. All five VACs, eight initial erasable banks, fourteen return fixed banks and Earth/Moon/raw corners are covered. Maximum target image ends0557C, with hardware20/software170 stack bytes and zero additional zero page. See `docs/COMANCHE-RECTIFY.md`.

## Run the live P66 Executive through the acceleration command

The live native Executive now runs RODCOMP through completed AFC/DELVROD at LOC03433 using 23 source-aligned callbacks and 27 disk-loaded arithmetic modules. All 192 C64 source fingerprints pass, including real higher-P66 preemption, both SUPER forms and enabled/masked native interrupts. The resident ends at $6fb9; overlays fit16KB with zero BSS/noinit, the canonical image remains authoritative and measured software stack is736/2048bytes. Independent original-source, sanitizer and first-legal pending NEWJOB comparisons pass at all22 handoffs. Actual resident PRG and D81 media are retained. Feedback, throttle, horizontal and final display lifecycle remain subsequent work. See `docs/P66-LIVE-GUIDANCE.md`.
