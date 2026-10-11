# Native Apollo port handoff

## Repository

Fork: https://github.com/koalabuttz/Apollo-11
Development branch: **port/c64-native-c**. Upstream and master remain based on
`911e5c0283c629c50cb97666f34065e8c07d71a5`. Historical files are unchanged; all
port changes live under `c64-port/`. Full upstream history is preserved.
The first archived milestone ended at `a9edd173fdd269262f886c19ae983276bf4e7e6d`.
Use `git rev-parse port/c64-native-c` to identify this milestone's current tip.

```sh
git clone --branch port/c64-native-c https://github.com/koalabuttz/Apollo-11.git
cd Apollo-11
git remote add upstream https://github.com/chrislgarry/Apollo-11.git
cd c64-port
make test
python3 tools/prepare_reference.py
make differential REFERENCE="$PWD/deps/fixture"
make c64 MOS_CC=/path/to/mos-c64-clang
make c64-irq MOS_CC=/path/to/mos-c64-clang
make c64-lifecycle MOS_CC=/path/to/mos-c64-clang
make c64-waitlist MOS_CC=/path/to/mos-c64-clang
make c64-restart MOS_CC=/path/to/mos-c64-clang
make c64-guidance MOS_CC=/path/to/mos-c64-clang
make c64-prepare MOS_CC=/path/to/mos-c64-clang
make c64-scale MOS_CC=/path/to/mos-c64-clang
make c64-push MOS_CC=/path/to/mos-c64-clang
make c64-elapsed MOS_CC=/path/to/mos-c64-clang
make c64-store MOS_CC=/path/to/mos-c64-clang
make c64-divide MOS_CC=/path/to/mos-c64-clang
python3 tools/vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
python3 tools/irq_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
python3 tools/lifecycle_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
python3 tools/waitlist_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
python3 tools/restart_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
python3 tools/guidance_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
python3 tools/prepare_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
python3 tools/scale_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
python3 tools/divide_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
python3 tools/store_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
python3 tools/elapsed_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
python3 tools/push_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
```

## Completed work

The fourteenth milestone translates PDVL GDT/2 after the quotient. All 99
modeled words match 5218 original checkpoints over 3865 cases. All legal DP
push positions, complete-prefix positions, skip inputs and retained histories
are covered. Host/sanitizer suites pass; the thirteenth C64 PRG checks 156
complete states with zero failures. Lossless expected-state storage keeps the
40690-byte PRG below $c000. Context is 198 bytes. PDVL samples reach 0.747 ms;
full prefix 28.586 ms. GRAVITY.md records contracts, original bank digestion,
compression and evidence. Earlier 218 evidence hashes remain unchanged.


The thirteenth milestone translates fixed-divisor DDV 4SEC(28), stopping before
pending PDVL GDT/2. It preserves TPAGREE signed-zero/mixed-sign stores, both
source overflow exits, exact quotient stages and register/scratch debris. All
93 modeled words/results match 8098 checkpoints over 6735 cases, including every
ordinary quotient and 1022 alternate mixed-sign boundary representations. The
DDV oracle selects the pinned engine's hardware DV model via a retained one-line
host-only patch: its fast path mishandles sign-extended negative zero. A two-model
audit records 798 differing quotient/L rows; the checkout and earlier oracles
are untouched. All twelve host/sanitizer suites and the twelfth C64 PRG's 156
complete-state checks pass. Context is 186 bytes; measured DDV reaches 6423
cycles (6.519 ms), full prefix 27940 (28.358 ms). Earlier 195 evidence identities,
twelve overlays, rope and historical sources remain unchanged. DIVIDE.md records
the numerical contract, model discrepancy and oracle provenance explicitly.

The twelfth milestone translates STORE 30D, stopping before DDV/PDVL. It writes
the elapsed DP pair to VAC-relative words 30/31, clears A through CCS MODE and
retains the old word 31 in L. All 89 modeled words and prior DSU observations
match 5044 original-assembly checkpoints over 3681 cases, including all 190
VAC/push positions, 35 overlapping saves, 256 source/destination corners and
three retained histories. All eleven host/sanitizer suites pass. The eleventh
C64 PRG checks 176 complete states, restoring I, native scope rejection and
reserved memory layout, with zero failures. Total state remains 178 bytes;
STORE samples reach 317 PAL cycles (0.322 ms), full prefix 21841 (22.168 ms).
Earlier 176 evidence hashes, twelve source overlays, rope and historical files
remain unchanged. STORE.md defines the state/bank/control boundaries.

The eleventh milestone translates DSU PIPTIME and its overflow realignment,
stopping before STORE 30D. All 89 modeled words and observed TS 7 paths match
4664 original-assembly checkpoints over 3491 cases. Both overflow signs,
resolved initial overflow retaining OVFIND, signed-zero/carry corners and three
four-step histories are covered. All ten host/sanitizer suites pass. The tenth
C64 PRG checks 172 full final states, restoring I and stack/ZP checks, with zero
failures. Total state is 178 bytes; DSU samples reach 1381 PAL cycles (1.402 ms),
and the sampled full prefix takes 21427 (21.748 ms). Earlier 157 evidence hashes
remain intact. The twelfth byte-identical source overlay supplies LIMITS, with
unchanged rope and historical sources. ELAPSED.md records the ownership,
overflow semantics, physical bank assertions and next STORE 30D boundary.

The tenth milestone translates PDDL THISTPIP after scaling. It saves six vector
words in an owned 43-word VAC payload, advances PUSHLOC, loads the timestamp
and preserves original MPAC/A/L exchange debris with MODE=DP. All 86 modeled
words match 3496 original-assembly checkpoints over 2323 cases, including all
five VACs at all 38 legal offsets and three four-push retained histories. All
nine host/sanitizer suites pass. The ninth C64 PRG checks 200 complete final
states with zero failures, restoring I and verifying native capacity rejection.
Total state is 172 bytes; sampled PDDL alone reaches 1051 cycles (1.067 ms),
and the sampled full prefix through PDDL takes 20790 (21.101 ms). Earlier 138
evidence files and historical sources remain unchanged. PUSHDOWN.md documents
FIXLOC/PUSHLOC ownership, the native scope check and stop before DSU PIPTIME.

The ninth milestone translates VXSC KPIP1 into native partial-product arithmetic
and signed AGC rounding, stopping at DANZIG before PDDL. All 41 modeled words
match 3364 actual-assembly checkpoints over 3355 cases, including exact-half
rounding, high-word propagation, full RODCOMP/interpreter entries, and three
four-step histories. All eight host/sanitizer suites and earlier comparisons
pass. The eighth C64 PRG checks 292 complete states with zero failures, restoring
I and restricted ZP/stack checks. It uses 82 bytes of state; sampled scaling
reaches 25737 PAL cycles (26.122 ms), and the sampled full prefix plus scaling
takes 19838 (20.135 ms). These do not establish a live guidance deadline.
SCALING.md documents the numeric contract and explicit interpreter-state adaptation.

The eighth milestone extends RODCOMP through PIPA/time snapshot exchanges and
vector-input preparation, stopping before TC INTPRET at bank 32,3353. All 39
modeled words and six TS-skip observations agree with 3396 original-assembly
checkpoints over 1171 cases, including every skip combination in both sign
families. The final delta's overflow can skip CAF ZERO; its +/-1 is faithfully
stored into low MPAC/TEM words. MODE aliases MPAC +7; MPAC +2 and DELVROD low
words remain intact. Three four-step full-prefix histories compare both staged
and direct composed calls with original final states. All seven host/sanitizer
suites and prior comparisons pass. The seventh C64 PRG checks 324 complete
states and histories, restoring I and ZP/stack canaries. State is 78 bytes and
the complete sampled call takes 6692 PAL cycles (6.792 ms). PREPARATION.md gives
the boundaries, stable-input/scratch ownership and remaining interpreter work.

The seventh milestone translates RODCOMP's initial P66 rate adjustment and its
required MP/DAS arithmetic. Count clearing, raw fixed-point scaling, both signed
zeros, low-word carry, independent word signs and final A/L overflow agree with
12,856 actual-instruction checkpoints across 12,835 cases. Three eight-step
histories retain rate state between switch events. All six host/sanitizer suites
pass; earlier snapshots and the rope are unchanged. The sixth C64 PRG checks
568 complete six-word expectations and the histories in PAL VICE, with zero
failures, restoring I and stack/ZP checks. State is 12 bytes; the larger sampled
update is 2108 PAL cycles (2.140 ms). GUIDANCE.md defines the bank 32,3275..3302
boundary and raw-scale/caller-exclusion contract. Interpreter continuation,
physical switch input and complete P66 scheduling/recovery remain untranslated.

The sixth milestone adds bounded ALARM1/BAILOUT1/POODOO1 alarm recording,
STARTSB1 cleanup with explicit device inputs, phase-pair validation and Type C
variable-job re-entry. 190 cases / 226 checkpoints match independent original
assembly across all 448 modeled words. The fifth C64 PRG passes every fingerprint
and completes six reconstructed native jobs after explicitly reinitializing a
faulted runtime. Alarm history and unrelated payloads are retained. RESTART.md
defines stage boundaries and excludes full ENEMA glue and other restart forms.


The fifth milestone adds WAITLIST insertion, T3RUPT2 list/timer reload and
TASKOVER, preserving nine tasks, both zeros and equal-deadline ordering. Native
tasks run under restoring exclusion and may reschedule tasks/admit or wake jobs.
A separate CIA1 Timer B gate publishes ticks only. The PAL diagnostic uses 20 ms
periods/two-centisecond samples; source/mask/poll limits are explicit in WAITLIST.md.
All 196 cases / 306 checkpoints match unchanged original assembly, including ten
actual native task sequences. Six real CIA IRQs pass register/decimal, interrupted
arithmetic, KERNAL clock and task/job integration checks. The task state is 58
bytes and its clock-publication buffer is three bytes; no heap is used.

The fourth milestone added basic JOBSLEEP/JOBWAKE and PRIOCHNG, plus native
sleep/reprioritize callback actions and guarded foreground wake. Dormant jobs
retain core/VAC ownership. Original CADR matching, bank reconstruction, priority
low bits, CCS scan bias and scratch debris match 115 independent checkpoints.
Five native sequences run on host and PAL C64; a real raster IRQ admits a waker
while a VAC-owning job sleeps. LIFECYCLE.md records exact semantics and boundaries.

The third milestone added restoring C64 exclusion guards, a four-entry IRQ
admission-request buffer and foreground delivery safe points. Actual raster IRQs
preserve LLVM-MOS registers and interrupted C frames, while dispatch remains
cooperative. Queue overflow and resource-fault behavior are documented in IRQ.md.

The original bounded admission and release/selection paths remain verified.
The second milestone implements basic CHANG1/CHANJOB context exchange and
completion through basic dispatch. Historical two-word addresses map to native
C continuations, with the original ascending packed-priority scans retained.
All MPAC/MODE, LOC/BANKSET debris, PUSHLOC/PRIORITY, FIXLOC and OVFIND transitions
are compared with unchanged original assembly. Interpreter dispatch is rejected
before release or exchange; it has not been approximated with native callbacks.

Two native demonstration jobs actually execute, yield, resume and complete.
Eight sequences cover higher/equal/lower priorities, VAC/FAKEPRET low-bit ties,
minimum/maximum priority and retained stale state. They check restored payloads
inside the native callbacks. The 53 dispatch cases yield 109 full-state/target
checkpoints. Original target Z/BBANK/SUPERBNK transfers are checked independently.

Validation passes:

- Host numerical tests and the original 26 admission/completion scenarios.
- Host dispatch/fault tests and all eight native two-job executions.
- 26 admission/completion plus 109 dispatch checkpoints, word-for-word against
  original assembly, and the separate Virtual AGC numerical helper comparisons.
- All twelve host suites under AddressSanitizer/UndefinedBehaviorSanitizer (leak scanning disabled).
- PAL C64 VICE: zero failures, all 135 original-derived fingerprints, eight
  actual native two-job sequences and three repeated timed-path checks.
- Host IRQ publication/FIFO tests over 600 starting-ticket cases, guards, idle
  delivery, overflow, exact resource faults and callback-local fail-stop.
- Separate PAL C64 diagnostic: nine actual raster IRQs, register/decimal canary,
  isolated ISR locals, interrupted native C arithmetic, eight original-derived
  checkpoints, overflow, masked delivery, KERNAL timer advancement and removal.
- 74 IRQ guarded intervals plus IRQ and hash-free latency observations.
- Lifecycle host/fault/restoring-guard tests, 81 cases / 115 word-for-word assembly
  checkpoints and five actual native callback sequences; ASan/UBSan pass.
- Third PAL C64 diagnostic: all 115 lifecycle fingerprints, five sequences,
  real IRQ-admitted waker, retained VAC/MPAC/OVFIND checks, 76 guarded intervals,
  four integration intervals and compiler-ZP/stack checks.
- WAITLIST host tests, exhaustive positive TIME3 overflow positions, source-buffer
  saturation, ambiguous-phase rejection, guarded/native capacity and mapping faults.
- 306 complete task-state/target comparisons, ten actual native task sequences;
  fourth PAL C64 diagnostic checks all fingerprints, six CIA IRQs, three observed
  20 ms periods, 156 primitive timings and retained stack/mailbox evidence.

Alarm/restart host tests and all 226 checkpoints, five ASan/UBSan suites and
the fifth PAL diagnostic with six recovered jobs pass (RESTART.md).
Numerical/P66 host checks and all 12,856 independent checkpoints pass; the sixth
PAL diagnostic verifies 568 complete states and three rate histories (GUIDANCE.md).
PIPA/time and vector preparation adds 3396 complete independent checkpoints;
the seventh PAL diagnostic checks 324 states and three composed histories
(PREPARATION.md). All earlier evidence and historical sources are unchanged.

Retained executables, snapshots, logs, target mailbox/stack dumps, maps and hashes
are in `evidence/`; reproducible output is in `build/`. DISPATCH.md documents the
callback ABI, startup scratch convention, reference boundaries and fail-stop
behavior. PERFORMANCE.md records the current image/RAM/stack and timing samples.
IRQ.md defines queue ownership, safe points, fail-stop behavior and the C64 ABI.
The Executive state remains 648 bytes; the target request buffer is 32 bytes.

## Limits and next milestone

This cooperative basic-job translation includes native demonstration routines
and P66 preparation through PDVL GDT/2. Complete flight OS and guidance workloads
remain future work. Callbacks return to a trampoline; persistent values belong
in MPAC/VAC, not suspended C stacks.
An unmapped address latches a runtime fault. No rollback is claimed; bounded
explicit recovery is documented in RESTART.md. Idle dispatch uses a bounded
A=2/L=0 convention, not Apollo self-check.

The IRQ handler publishes through `exec_irq_post`; it must never mutate Executive
state or invoke native dispatch. Foreground wrappers restore IRQ state and admit
at declared safe points. The C64 IRQ/task builds require `-fno-static-stack -mlto-zp=0`;
the legacy console build is unbound and uses the usual compiler defaults.
Source rate, callbacks and masked intervals still need application bounds;
finite raster testing does not establish an unconditional delivery deadline.

Next guidance stage: VSU VBIAS, implicit quotient-fed VXSC, shifts and velocity
accumulation through STOVL 24D/R. Parallel work adds reusable vector arithmetic,
Executive services, phase maintenance and general scalar arithmetic.
Interpreter dispatch, flight deadlines and physical hardware verification remain open. See ROADMAP.md.

Acceptance for basic dispatch, IRQ delivery, lifecycle, bounded WAITLIST,
bounded Type C job recovery, pre-interpreter P66 preparation, VXSC KPIP1, PDDL THISTPIP, DSU PIPTIME, STORE 30D and fixed-divisor DDV is met using the hardware-DV oracle
(see DIVIDE.md for the pinned fast-path discrepancy).
No master/upstream merge is needed.
No second-agent/human review was performed. Native C is MIT/public-domain
translation; the separate GPL-2.0-or-later original-assembly adapters are host
verification tools and are never linked into the C64 executable.

## Verified native VAD/VSU

Reusable vector arithmetic now matches 13,600 original-assembly checkpoints.
128 complete states pass host, sanitizer and C64 diagnostics. See [VECTOR.md](VECTOR.md).

## Verified SPVAC and CHANG2

Staged VAC admission and interpretive context exchange match 768 complete
original-assembly checkpoints. Host/sanitizer tests and all 768 C64 state
fingerprints pass. Native mixed-context execution is a separate integration
stage. See [EXECUTIVE_SERVICES.md](EXECUTIVE_SERVICES.md).

## Translate native single and double phase-table changes

All 49,152 complete original-assembly states match; 128 host/sanitizer/C64 states pass. See [PHASE.md](PHASE.md).

## Translate arbitrary-factor vector scaling and unrounded DP products

All 8,192 complete assembly checkpoints and 128 host/sanitizer/C64 states pass. See [PRODUCT.md](PRODUCT.md).

## Translate scalar and vector shifts with original rounding and scratch effects

All 26,880 complete assembly states and 168 host/sanitizer/C64 states pass. See [SHIFT.md](SHIFT.md).

## Translate general double-precision DDV and backward division

All 163,840 original-assembly comparisons and 573 host/sanitizer/C64 states pass using the hardware DV model. See [GENERAL_DIVIDE.md](GENERAL_DIVIDE.md).

## Translate complete P66 updated-velocity calculation through STOVL and position load

All 4,648 complete assembly checkpoints pass; all 156 C64 states pass across two RAM-safe images. See [VELOCITY.md](VELOCITY.md).

## Translate scalar square root and normalized root with source error boundary

All 132,096 assembly comparisons and 637 host/sanitizer/C64 states pass; negative inputs stop at the declared pre-1302 boundary. See [NORM.md](NORM.md).

## Translate abort flags and selected restart-group cleanup paths

All 49,152 complete assembly comparisons and 128 host/sanitizer/C64 states pass. See [RECOVERY_CLEANUP.md](RECOVERY_CLEANUP.md).

## Translate unrounded triple-precision dot product and overflow accumulation

All 10,272 complete assembly comparisons and 96 host/sanitizer/C64 states pass. See [DOT.md](DOT.md).

## Translate scalar sums and backward vector subtraction with exact overflow effects

All 27,200 complete assembly comparisons and 256 host/sanitizer/C64 states pass. See [SUM.md](SUM.md).

## Full project endpoint

The user explicitly requested complete Luminary 099 and Comanche 055 native
C64 ports. PORT_SCOPE.md and port-coverage.json track both complete source
trees; bounded milestones do not certify either complete application. Run
`python3 tools/port_coverage.py` to audit progress and historical source identity.

## Execute native basic and interpretive continuations through original context exchanges

Native C callbacks now enter and exit interpretation, yield at completed operation pairs, restore interpreter payloads, and resume through the original Executive lifecycle. All 140 full original-assembly checkpoints agree; host and sanitizer tests and the PAL C64 diagnostic pass, including masked repeats and an actual raster IRQ admission. This milestone supplies scheduling plumbing; remaining guidance bodies still need translation and binding. See [INTERPRETER_NATIVE.md](INTERPRETER_NATIVE.md).

## Translate ENEMA through restart validation and source-defined exit boundaries

The native restart entry composes the verified cleanup and phase validation routines through the original MMDSPLAY, PTBAD, and fresh-start request boundaries. All 4,096 full 457-word original-assembly comparisons agree, and 128 complete C64 target states pass with host and sanitizer verification. Subsequent display and restart execution remain to be completed. See [ENEMA.md](ENEMA.md).

## Translate Comanche Executive admission and release with seven native core sets

A separate Comanche 055 Executive retains its seven core sets, five VACs, and source BAILOUT boundaries. All 1,536 source-derived assembly comparisons agree; host and sanitizer tests and the C64 run pass, including 40 complete-state samples. The generated fixture documents five indentation repairs and matches the pinned corrected full rope; historical repository files remain unchanged. CM dispatch and lifecycle are next. See [COMANCHE_EXECUTIVE.md](COMANCHE_EXECUTIVE.md).

## Translate absolute value, vector length and half-unit normalization with signed-zero fidelity

ABS, ABVAL and UNIT now preserve raw length, half-unit vector, overflow, and scratch effects. All 50,736 complete 104-word assembly comparisons agree, including sixteen signed-zero regressions found by P66 composition. Host and sanitizer tests and all 118 full-state C64 checks pass. Full application binding and remaining flight routines are still in progress. See [UNIT.md](UNIT.md).

## Compose native P66 radial velocity and predicted altitude from verified arithmetic

The P66 radial group now computes half-unit radius, radial velocity and predicted altitude with original VAC, signed-zero and scratch effects. All 4,952 complete 134-word original-assembly checkpoints agree; full-prefix host and sanitizer checks pass. Six C64 images verify 203 complete stage outputs from exact original pre-UNIT states. The full-prefix C64 diagnostic exceeds RAM, so whole-program overlay integration remains required. See [RADIAL.md](RADIAL.md).

## Translate P66 desired radial acceleration and gravity through the rotation boundary

The native P66 acceleration group preserves original VAC, scalar division, vector magnitude, gravity scaling and implicit stack pop effects through DANZIG before CDU rotation. All 16,384 full-stage assembly states and the 7,428-checkpoint composed host prefix agree. Strict host, sanitizer and 96 full-state C64 stage checks pass; whole-program overlay execution remains in progress. See [ACCELERATION.md](ACCELERATION.md).

## Execute bounded native disk overlays with canonical erasable storage on the C64

A resident Executive and loader execute separately linked native C payloads from a D64 image, swap and reload them after ordinary C returns, reset transient BSS, and preserve the 2,048-word canonical source image and platform mappings. Host bounds and error checks, sanitizers and the stock PAL C64 disk-loading proof pass. The demonstrated payloads test the ABI; full mission module allocation, media handling and guidance integration remain unfinished. See [OVERLAYS.md](OVERLAYS.md).

## Fix source-exact WAITLIST positive-zero control flow and final-slot insertion

WAITLIST now skips interval additions on early CCS positive zero and follows the final positive-zero insertion route instead of raising a false full-queue alarm. The 256-case full-word regression detects 688 previous field errors and passes after the fix; the original 196-case suite remains unchanged and passes. Host, sanitizer and all 256 C64 states agree with original assembly. See [WAITLIST_ZERO.md](WAITLIST_ZERO.md).

## Execute Comanche basic and interpretive jobs through source-exact lifecycle transitions

The separate seven-core CM runtime now dispatches native C callbacks, exchanges basic and interpreted contexts, sleeps and wakes jobs, and changes priorities through original Comanche semantics. All 876 complete source-derived checkpoints and twenty real native sequences agree; host and sanitizer checks and four stock C64 diagnostics pass. IRQ coordination remains caller-owned pending the dedicated adapter. See [COMANCHE_DISPATCH.md](COMANCHE_DISPATCH.md).

## Translate P66 operator counts, task admission and phase-timer entry paths

DESCBITS, RODTASK and P66VERTA now follow original operator arithmetic, priority-22 admission, phase termination, delayed task scheduling and immediate RODCOMP entry with exact shared scratch. All 73,728 original-entry checkpoints agree; host, sanitizer and 128 full-state C64 checks pass. STRTP66 bias initialization, display admission and later continuations are separate ongoing milestones. See [P66_STARTUP.md](P66_STARTUP.md).

## Port exact native trigonometric services

All 65,536 complete source states match CDULOGIC, SIN, COS and CDUTRIGS; all-state sanitizers and 128 frozen C64 cases pass. The native fixed-point coefficients and overflow behavior retain the original instruction semantics. See [TRIG.md](TRIG.md).

## Run live P66 continuations from native disk overlays

The canonical 2,048-word image drives actual compiled arithmetic callbacks through the mixed Executive. All 5,887 source checkpoints pass; the C64 disk proof passes 72 full-state fingerprints including two real P66 jobs, both IRQ states and SUPERBNK forms. The current driver ends before UNIT and retains its core/VAC for further translation. See [P66_NATIVE_DRIVER.md](P66_NATIVE_DRIVER.md).

## Port original native axis rotations

All 32,768 complete AX*SR*T source states match; strict and all-state sanitizer checks pass. C64 verification compares 128 full frozen states with source scratch aliases and both rotation directions preserved. See [ROTATION.md](ROTATION.md).

## Port live CDU projection and acceleration feedback

Live native trig and rotation feed DOT14 and AFC with original source scheduling boundaries. All 39,432 direct and full-prefix checkpoints match; entire-corpus sanitizers and all 144 frozen C64 checkpoints pass. See [PROJECTION.md](PROJECTION.md).

## Implement exact native hardware divide recurrence

The independently authored fixed-point recurrence matches 327,680 isolated original hardware-DV instruction states, including raw register overflow and signed zero. Full-state sanitizers and all 255 frozen C64 register-pair checks pass. See [HARDWARE-DIVIDE.md](HARDWARE-DIVIDE.md).

## Port source STOPRATE and FASTCHNG initialization services

All 1,024 complete source checkpoints match including return-register debris, physical banks and IRQ state. Strict and sanitizer checks and 128 complete frozen C64 states pass. See [P66_INITIALIZATION.md](P66_INITIALIZATION.md).

## Port original force feedback and throttle clamps

All 16,384 complete force-feedback states match unchanged assembly with all three clamp paths covered. Full-state sanitizers and 64 complete frozen C64 cases pass. See [THROTTLE_FEEDBACK.md](THROTTLE_FEEDBACK.md).

## Port complete bounded native THROTTLE service

THROTTLE+3 through THDUMP matches all 16,384 complete 147-word source states, including hardware-DV weighting, force saturation, critical branches, FASTCHNG and pulse commands. Strict and all-state sanitizer checks and 96 complete C64 states pass; the enclosing return bridge is a separate continuation. See [THROTTLE-SERVICE.md](THROTTLE-SERVICE.md).

## Port canonical whole-image P66 bias initialization

All 1,024 original STRTP66A entry cases match the complete 2,048-word erasable image plus physical SUPERBNK/IRQ. Strict and sanitizer ownership checks and 64 complete whole-image C64 states pass. See [P66_BIAS.md](P66_BIAS.md).

## Port canonical vertical-guidance startup through RODCOMP

All 1,024 complete original source images match through STOPRATE, flag updates, phase termination, timer admission and POSTJUMP. Strict and sanitizer checks and 64 whole-image C64 cases pass, including source resource bailouts. See [P66_VERTICAL_START.md](P66_VERTICAL_START.md).

## Port Comanche timer queue and native task execution

All 306 complete original WAITLIST checkpoints match, including zero-sign branches, source task transfers and 01203/01204 resource alarms. Strict, sanitizer and C64 checks cover all checkpoints, ten actual native callback sequences and ownership guards. See [COMANCHE_WAITLIST.md](COMANCHE_WAITLIST.md).

## Connect Comanche Executive to real native C64 interrupts

Exact source admission and callback states verify the CM runtime adaptation. C64 checks pass 512 admission fingerprints, seven real raster interrupts and 15 full callback states while preserving native registers, compiler storage, caller IRQ state and KERNAL coexistence. See [COMANCHE_IRQ.md](COMANCHE_IRQ.md).

## Port real P66 major-mode display job admission

NEWMODEX matches 1,024 complete original erasable images, admitting the actual DSPMMJB basic job. Strict, sanitizer and C64 checks pass 64 full images; deferred display formatting and consumption remain separate continuations. See [P66_MAJOR_MODE.md](P66_MAJOR_MODE.md).

## Compose complete native STARTP66 through RODCOMP

The complete source landing-start sequence matches 1,024 whole-memory entry states, with actual display-job and timer admission and both resource bailouts. Strict and sanitizer checks and 32 complete C64 images pass. Deferred display execution and the remaining landing cycle are separately tracked. See [P66_LAUNCH.md](P66_LAUNCH.md).

## Port complete native horizontal velocity arithmetic

All 65,536 original horizontal-velocity checkpoints match, preserving UNIT overflow continuations and physical bank effects. Strict and all-state sanitizer checks and all 256 complete frozen C64 states pass. See [HORIZONTAL.md](HORIZONTAL.md).

## Port actual P66 major-mode display formatting

DSPMMJB matches all 1,024 original whole-memory states across every decimal mode and relay dirty/clean branch. Strict and sanitizer checks and 64 complete C64 images pass through the actual ENDOFJOB entry; relay drain remains a separate service. See [P66_MODE_DISPLAY.md](P66_MODE_DISPLAY.md).

## Port original canonical throttle call and return bridges

The actual source EXIT/BANKCALL and THDUMP/SWRETURN/INTPRET paths match all 8,192 complete erasable images. Strict and all-state sanitizer comparisons and 64 whole-image C64 checks pass, with unsupported-boundary no-mutation guards. See [THROTTLE_BRIDGE.md](THROTTLE_BRIDGE.md).

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

All 1,024 whole-memory DSPOUTSB source states and actual channel10 output events match. Strict and sanitizer checks and 64 complete C64 images pass; the native screen adapter and live application wiring remain separate integrations. See [DSKY_RELAY.md](DSKY_RELAY.md).

## Render actual native DSKY relay output on the C64

The source mode66 display job and channel10 relay drain now drive real C64 screen/color RAM. Host, sanitizer and target checks verify numeric/sign latches, lights, blanking and the128cs flash cycle; live application keyboard/device integration remains pending. See [DSKY_DISPLAY.md](DSKY_DISPLAY.md).

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

## Port Comanche DIFEQ Nystrom numerical phases

Original DIFEQ+0/+1 with DIFEQCOM through STORE ALPHAV, and DIFEQ+2 through STORE ZV, run as explicit native phases. All 19,840 complete 2,048-word original states from 1,792 calls match SHA 9b8b544ee7eac53ad9ac77fa6f27e4d980a23b0e3dcf22d7843e9ff4607a3ecf, including raw index overflow corners. Full-source sanitizer equality and 960 original phase entry/exit C64 states pass across 30 images. Genuine DANZIG returns preserve scheduling semantics; STCALL-to-NEWOPS internal transitions retain ownership. Actual PRGs, maps, mailboxes and stack dumps are retained; maximum image ends05F36, hardware20/software177 stack bytes, zero additional zero page. Integrator control surrounding these stores remains subsequent work. See `docs/COMANCHE-DIFEQ.md`.

## Port Comanche DIFEQ0 initialization and source JSWITCH routing

Native orbital initialization and JSWITCH BON/GOTO routing reproduce all 5,120 complete 2,048-word original images plus IRQ from 1,024 calls, SHA 5a0688f511a9c6097a5aeafe9b8ac628c6ff9598345b029c40e0471e0d1cb37c. Strict and full-source sanitizer checks plus four actual C64 images containing 128 original phase entry/exit states pass. Phases0..2 reach genuine DANZIG; phase3 restores IRQ and returns to NEWOPS at original DOW/ACCOMP without inserting a job check. Maximum target image ends0409B, hardware20/software44 stack bytes, zero additional zero page. Actual PRGs and all original-source proof artifacts are retained. See `docs/COMANCHE-ORBITAL-INITIAL.md`.

## Correct DSKY positive-zero blanking and NOUT increment overflow

Original 2BLANK skips INCR NOUT for positive zero, and original INCR overflow-corrects register addition at037777. Generic Pinball and private P66 mode display now preserve both source behaviors. New independent full-image corpora cover4,096 Pinball and1,024 mode cases, with complete sanitizer streams and128 actualC64 source states passing. Prior published source corpus identities rerun unchanged; earlier constructors did not establish these edges. Source arithmetic and historical files remain unchanged outside the native corrections. Current dependent basic diagnostic also passes its original source images, task admission and real screen output after the fixes. See `docs/DSKY-ZERO-CORRECTIONS.md`.

## Preserve full-width P66 callback alarms

Native callback status now retains full16-bit alarm and fault values instead of the target short enum. Dedicated strict/sanitizer and10 actualC64 enabled/masked-interrupt transport checks preserve01302,01202,8003,ffff and native failure4, commit the qualified source boundary and stop without callback retry. Existing numerical, preemption and pending-request source hashes remain unchanged; this proves status transport rather than additional alarm arithmetic. See `docs/P66-STATUS.md`.

## Port source V06N60 display-data preparation

Original NVSUB through ENTPAS0, table reload, MIXNOUN and DECDSP reaches source DSPDCPUT. All 4,096 complete 2,048-word source images plus SUPER/IRQ match SHA a6f0742ccbc774a0c5eb3e66b62289b6832efcbdfd1c3f6424aeffe53bfd0089. Strict64 source states/10 invalid entries, full-source sanitizers and64 actualC64 states pass. Original noun60 physical data pairs and source continuation state remain authoritative; targetPRG12274bytes, hardware29/software43 stack bytes and zero additional zero page. See `docs/P66-NV-ROWS.md`.

## Port original V06N60 scaling and three-row display consumer

Native scale selection, DSPDCEND BANKCALL/decimal/SWRETURN, row-loop and NVSUBEND/SUPDXCHZ preserve the actual source caller. All4,096 scale-selection states and12,288 complete2050-word three-row checkpoints match original assembly; full-row SHA 2ecaa7528c4f5ae8281952543f146c7390eb9a281d610fbdef49fbc25f819a58. Strict64 states/30 invalid entries, full-source sanitizers andfour actualC64 sixteen-state images pass. PRGs8843/11922/12296/9067bytes use no additional zero page and at most29 hardware/153 software stack bytes. MAKEPLAY/GOPLAY admission and surrounding scheduler control are separate services. See `docs/P66-NV-RENDER.md`.

## Port original GOPLAY display completion and flag services

The original GOPLAY consumer now carries V06N60 through all three display rows and the original return, flash, copy, flag and ENDJOB paths. Source comparisons retain 8192 direct and 18432 full-consumer states, sanitizer streams and the actual C64 PRG, mailbox and stack measurements. See `docs/P66-GOPLAY.md`.

## Port original LM BASIC PRIOCHNG service

The original PRIOCHNG service preserves all source registers, banks and scheduling effects across 8192 complete original states, host and sanitizer checks, and 64 C64 cases. This is a scheduling service milestone; complete LM application acceptance remains pending. See `docs/LM-BASIC-PRIORITY.md`.

## Share the resident Executive lifecycle code

The shared Executive native step is explicitly outlined, reducing the resident to $6e6b while preserving its algorithms. The new PRG and D81 pass 192 original-source C64 checkpoint comparisons with measured software stack 736 bytes; previous artifacts remain retained. See `docs/P66-RESIDENT-OUTLINING.md`.

## Port source P66 entry and completed-pair runtime bridges

Native source-entry bridges preserve the original NEWOPS, EXIT, THDUMP and basic REGODSPR paths without adding polling points. Full original-state comparisons cover 16384 cases with sanitizer and 64 C64 cases. See `docs/P66-RUNTIME-BRIDGES.md`.

## Original immutable fixed data

Both programs now retain 2871 independently assembled immutable words with exact C64 readback. Nine reviewed data-only original files have complete source coverage certificates; mixed noun-reader and Comanche restart files remain partial. Original CM ALTVAR01357 and R53FLBIT05010 layout distinctions are explicit. See `docs/FIXED_DATA.md`.

## Port genuine P66 feedback handoffs and horizontal internal phases

All 492557 original-source numerical and control checkpoints match the native fine phases and sanitizer streams. Thirty-seven actual C64 PRGs pass 1184 frozen entry and exit states. Every completed feedback pair remains an explicit scheduling boundary; pending UNIT and length operations retain ownership until their genuine source return. See `docs/P66-FINE-PHASES.md`.

## Compose real P66 basic jobs, task admission and display output

The native basic binding executes DSPMMJB and ENDJOB, admits RODTASK at its actual 100-centisecond WAITLIST interval, invokes the mapped RODCOMP job and routes DSPOUT to the C64 screen. The RODTASK source comparison covers 2048 complete states, sanitizer results and 64 C64 states; the final resident diagnostic passes eight complete source images with the corrected zero, alarm-width and shared-Executive dependencies. Full descent and interrupt composition remain separate milestones. See `docs/P66-BASIC-RUNTIME.md`.

## Port original Luminary VERB and NOUN keyboard entries

The real VERB and NOUN source entries preserve 4096 complete erasable and interrupt states, including signed-zero blanking, with matching sanitizer streams and 64 actual C64 cases. See `docs/DSKY-KEYBOARD-SERVICES.md`.

## Port source CHARIN verb and noun admission effects

The original CHARIN path preserves display locks, key-release channel events and real VERB or NOUN execution. All 4096 complete source states and sanitizer streams match; the native C64 diagnostic passes 64 cases. See `docs/DSKY-KEYBOARD-SERVICES.md`.

## Port source KEYCOM admission of real CHARIN jobs

KEYCOM admits the actual historical CHARIN priority and address pair into all eight Executive contexts, retaining source bailout and RESUME entry boundaries. Original-source and sanitizer comparisons cover 4096 complete states; 64 C64 cases pass. See `docs/DSKY-KEYBOARD-SERVICES.md`.

## Port original KEYRUPT register saves and time sampling

The real KEYRUPT1 path preserves raw overflow, interrupted Q and banks, double-word time sampling and channel15 key input before actual KEYCOM admission. All 4096 complete original states, sanitizer streams and 64 C64 cases agree; full interrupt entry and resume composition remain separately qualified. See `docs/DSKY-KEYBOARD-SERVICES.md`.

## Port original Comanche TIG-5 task and phase scheduling

All 1536 complete original-source states agree on the host and under sanitizers; the actual C64 PRG passes 192 representative whole-image fingerprints and a full retained state. Native TIG-5 selects the original TIG-0 delay, phase change, S40.13 priority and TASKOVER route. Resident mission-task composition remains a separate milestone. See `docs/COMANCHE_TIG5.md`.

## Port actual Comanche SETJTAG task and native callback

The original SETJTAG body saves elapsed time, sets JETAG, changes phase and returns through TASKOVER. All 1024 complete source states and real native callback sequences pass host, sanitizer and C64 checks; the initial harness failure and corrected proof remain retained. See `docs/COMANCHE_ENTRY_TASKS.md`.

## Preserve pending NEWJOB during real REGODSPR admission

Actual REGODSPR NOVAC admission preserves or replaces the original pending priority request across 8192 complete source states, matching sanitizers and 64 C64 cases. No synthetic polling point or job-completion placeholder is introduced. See `docs/P66-RUNTIME-REGODSPR.md`.

## Port actual throttle after-poll continuation fetch

The original NEWOPS and LOC03507/03510 fetch routes resume THROTTLE+3 with exact CYR and control debris. All 8192 complete source states and sanitizer snapshots match; 64 native C64 cases pass. See `docs/P66-RUNTIME-FETCH.md`.

## Port canonical LM JOBWAKE source effects

The original JOBWAKE service preserves raw L, bank and scheduling effects in 8192 complete source states, including first wake, no match and both signed zeros. Strict host, sanitizer and 64 C64 comparisons pass. See `docs/LM-BASIC-WAKE.md`.

## Port real MAKEPLAY priority and wake prelude

Native MAKEPLAY follows the actual PRIOCHNG, JOBWAKE, GOSLEEPS, GOPLAY and duplicate ENDOFJOB routes with source XCHTOEND replacement. Original comparisons cover 12288 complete states, matching sanitizer streams and 64 C64 cases; the sleep and resumed lifecycle remains separately qualified. See `docs/P66-MAKEPLAY.md`.

## Port canonical Comanche restart timing services

Complete original-image FINDTIME and ITSLGCL2 source effects; independent source, sanitizer and 192 C64 state checks pass. See `docs/COMANCHE_RESTART_TIMING.md`.

## Port canonical Comanche long-call bank transfers

Original LONGCALL, LONGCYCL, LONGRTRN and GETCADR transitions pass complete-image source and sanitizer comparisons plus C64 checks. See `docs/COMANCHE_LONGCALL.md`.

## Port actual Comanche restart-table selection

Original RESTARTS and phase-part selection use independently checked restart data; complete-image source, sanitizer and C64 checks pass. See `docs/COMANCHE_RESTART_SELECT.md`.

## Port real Comanche clock-task source control

Original timer cleanup, clock update and display admission transitions pass complete-image source, sanitizer and C64 checks. See `docs/COMANCHE_CLOCKTASK.md`.

## Port full-image Comanche WAITLIST return effects

Canonical WAITLIST wrapper preserves source registers, banks, scratch and dynamic task destinations; independent source, sanitizer and C64 checks pass. See `docs/COMANCHE_WAITLIST_IMAGE.md`.

## Port actual Comanche T3RUPT task transfer

Original T3RUPT2 and TASKOVER complete-image transitions pass source, sanitizer and C64 checks up to their documented hardware-return boundary. See `docs/COMANCHE_TASK_TRANSFER.md`.

## Port canonical Comanche job-admission return effects

Original NOVAC and FINDVAC complete-image effects, including the actual saved-Q return, pass source, sanitizer and C64 checks. See `docs/COMANCHE_ADMISSION_IMAGE.md`.

## Port real Comanche SVCT3 housekeeping

Actual SVCT3 drift, IMU admission and delay recheck paths pass independent complete-image source, sanitizer and C64 checks. See `docs/COMANCHE_SVCT3.md`.

## Port canonical Comanche fixed and variable delays

Actual FIXDELAY and VARDELAY return effects pass whole-image original-source, sanitizer and C64 checks within their documented interrupt context. See `docs/COMANCHE_DELAY_IMAGE.md`.

## Port original Comanche QUICTRIG and scalar trigonometry

All 164096 original-source full-image scalar/vector trig states match native and sanitized outputs; 128 C64 state checks pass. See `docs/COMANCHE-QUICTRIG.md`.

## Complete Comanche single-precision source return paths

Every original SINGLE_PRECISION_SUBROUTINES path is mapped through real source TCQ returns; all fixed return addresses and banks pass independent source, sanitizer and C64 checks. See `docs/COMANCHE-SINGLE-PRECISION.md`.

## Port original Luminary JOBSLEEP state transitions

All 8192 whole original-image JOBSLEEP transitions match native and sanitizer outputs; real C64 checks preserve scheduler ownership and source return effects. See `docs/LM-BASIC-SLEEP.md`.

## Port actual MAKEPLAY sleep and wake control

All 16384 complete original states qualify the true GOSLEEPS, wake and busy paths; native sanitizer and C64 checks pass. See `docs/P66-MAKEPLAY-SLEEP.md`.

## Port Luminary Pinball busy-state source control

All 8192 complete original-image busy transitions match native and sanitized outputs; C64 checks pass with exact scratch and register effects. See `docs/LM-PINBALL-BUSY.md`.

## Port actual Comanche INITDSP and priority transitions

All 12288 original complete-image display restart and priority states match native and sanitized outputs; genuine C64 checks pass. See `docs/COMANCHE-DISPLAY-RESTART.md`.

## Port Comanche Pinball relay formatting and blanking

Actual DSPIN, 11DSPIN and 2BLANK source effects match all 8192 original states and sanitizer outputs; real C64 checks pass. See `docs/COMANCHE-PINBALL.md`.

## Port original Comanche numeric display formatting

All 12288 original complete-image numeric formatting states match native and sanitized outputs; C64 execution checks pass. See `docs/COMANCHE-FORMATTERS.md`.

## Complete original LM and CM noun-table readers

All three reader algorithms and six tables in each original source file are mapped; 58368 full original images match native and sanitizer outputs, with 1216 actual C64 state checks. See `docs/NOUN_READERS.md`.

## Complete original LM and CM memory declarations and layouts

All five declaration source files map original aliases, allocation cursors and emitted metadata into the sole canonical image; independent original-rope checks, sanitizer and C64 execution pass. See `docs/LAYOUT.md`.

## Complete Luminary single-precision sine and cosine source paths

Every raw sixteen-bit input and real source return path is independently qualified across 262144 whole original states; sanitizer and 128 C64 state checks pass. See `docs/LUMINARY-SINGLE-PRECISION.md`.

## Reviewed whole-file coverage

Both original scalar sine/cosine files, both noun-table reader files and five memory-declaration files now have reviewed complete-source certificates. The inventory totals18 complete AGC source files; bounded runtime and mission routines remain partial, and both complete C64 applications remain unverified.

## Port canonical Luminary ENDOFJOB register and resume effects

All 16384 whole original-image ENDOFJOB states match native and sanitized outputs through actual basic, interpreted and idle resume entries; 64 real C64 state checks pass. See `docs/LM-BASIC-FINISH.md`.

## Complete original Comanche SMALLMP and DPADD routines

All original MYSUBS words, signed-zero and overflow paths, actual returns and four scalar aliases are mapped; 294912 whole original states match native and sanitizer outputs, and 128 C64 state checks pass. See `docs/COMANCHE-MYSUBS.md`.

## Port actual Comanche timer-interrupt lead-in

All 1024 complete original timer-interrupt states preserve registers, banks and external ISR/pending metadata; source, sanitizer and C64 checks pass. See `docs/COMANCHE_TASK_INTERRUPT.md`.

## Execute continuous original Comanche task and service paths

Actual continuous SETJTAG, CLOKTASK, SVCT3, WAITLIST, admission and delay paths match 1856 whole source states across384 cases; sanitizer and real C64 checks pass. See `docs/COMANCHE_SERVICE_LOOP.md`.

## Run real Comanche task services from native C64 disk overlays

The resident and four actual compiled mission/service bodies fit their memory slots; 12 real C64 paths verify58 source checkpoints, three complete source images and both caller interrupt masks. Rebuilding reproduces resident, modules and D64 byte-for-byte. See `docs/COMANCHE_SERVICE_OVERLAY.md`.

## Port original Comanche relay scan and physical display output

All4096 complete original DSPOUTSB and channel10 states match native and sanitized outputs; real C64 checks include actual screen and color RAM. See `docs/COMANCHE-RELAY.md`.

## Port Comanche numerical display-scale wrappers

All4096 complete original scale-wrapper states match native and sanitized outputs through the real decimal continuation; C64 execution checks pass. See `docs/COMANCHE-SCALE.md`.

## Complete original Luminary three-axis torque integration

All original torque routine paths, data and declarations are mapped through the actual external BACKP continuation; 36864 complete source and sanitizer states plus2048 real C64 state checks pass. See `docs/LM_TORQUE.md`.

## Torque and documentation coverage

The inventory now records20 complete native algorithm/data/declaration files and two comment-only CM documents preserved byte-for-byte. Documentation cannot qualify executable source: the acceptance gate independently checks every original line and requires zero translated algorithms and emitted words. Both full applications remain unverified;153 original AGC files still need complete native coverage.

## Adopt source-selected native continuations without repeating scheduling

Verified scheduler adoption preserves original source state and queued interrupts. Host and sanitizer checks cover 384 full states and 11 rejection cases; stock-ROM C64 proofs cover 256 full states. The shared compiled guidance continuation supports qualified source LOC+1 routing. This certifies these scheduler seams, not the complete Executive or either application. See `docs/NATIVE-SOURCE-DISPATCH.md`.

## Execute canonical Comanche ENDOFJOB selection and returns

Comanche ENDOFJOB matches 16,384 original full-state transitions through basic, interpreter, and idle returns. Strict host and sanitizer checks pass; C64 verifies 128 fingerprints and two complete state samples. This is bounded Executive coverage. See `docs/COMANCHE_BASIC_FINISH.md`.

## Translate numeric CHARIN entries through original NUM arithmetic

Native octal and decimal entry matches 8,192 complete original source states across verb, noun and three register fields, signs and digit counts. Strict host, sanitizer and 64-case C64 proofs pass. This certifies the numeric entry body; hardware IRQ composition is a separate milestone. See `docs/DSKY-NUMERIC-ENTRY.md`.

## Translate original Comanche clock-job call and display routing

The native clock-job bridge matches 24,576 original full states and 2,048 continuous QUICTRIG compositions. Strict and sanitizer comparisons pass; C64 validates 64 frozen complete-state cases. Downstream display and job bodies are separate source domains. See `docs/COMANCHE-CLOCKJOB.md`.

## Route Comanche display requests through canonical MAKEPLAY entry

REGODSP, CLOCPLAY, GOFLASH and SAVELOCS preserve all original state through the actual MAKEPLAY entry. Evidence covers 24,576 direct states, 2,544 composed clock-job checkpoints, sanitizers and 64 C64 cases. The downstream consumer remains a separate milestone. See `docs/COMANCHE-DISPLAY-CONTROL.md`.

## Run hardware keyboard IRQs through actual source job lifecycle

Three qualified keyboard groups preserve fixed-vector KEYRUPT and RESUME, source Executive selection, CHARIN and job completion. Source, sanitizer and stock-ROM C64 proofs are retained with exact dependency identities; the compiled family fits C64 RAM and stack limits. Prior failed target sizing remains explicitly recorded. See `docs/DSKY-KEYBOARD-RUNTIME.md`.

## Complete source display jobs through actual Luminary ENDOFJOB

Display formatting, three rows and job completion match 11,776 original full states, 5,120 lifecycle checkpoints and 12,288 final states. Exact source streams, strict and sanitizer evidence, and 22 linked native overlays are retained. Combined disk execution is separately qualified; this does not certify either complete application. See `docs/P66-DISPLAY-JOB.md`.

## Share resident scheduler services with returned overlay callbacks

Resident scheduler adapters reuse the qualified core algorithm and preserve exact source-selected state. Strict and sanitizer checks cover display, lifecycle and final source corpora plus six seam guards. Four C64 scheduler overlays are linked and hashed; combined disk execution is a separate proof. See `docs/NATIVE-SCHEDULER-OVERLAYS.md`.

## Run numeric entry through hardware IRQ and canonical job completion

The NUM-only runtime mapping preserves hardware keyboard delivery, source IRQ return, Executive selection, numeric entry and completion across 3,072 full source checkpoints. Strict and sanitizer streams match; C64 validates eight complete pipelines covering every EB and SUPER context within the fixed RAM limit. Broader numeric body target coverage is retained separately. See `docs/DSKY-NUMERIC-RUNTIME.md`.

## Execute original Comanche DANZIG foreground selection

Canonical DANZIG preserves all 4,096 original complete states through actual basic/interpreter selection, bank and editing-register behavior. Strict and sanitizer checks pass; C64 verifies 128 fingerprints and two complete state samples. Unsupported malformed NEWJOB input is explicitly rejected. See `docs/COMANCHE_DANZIG.md`.

## Translate canonical Comanche CDU trigonometry and source phases

CDUTRIGS and CD*TR*GS match 655,360 original complete states across all CDU inputs, modes, return contexts and EB settings. Strict and full sanitizer streams agree; ten C64 PRGs validate 320 complete phase states. This retains original declaration identities and leaves the remaining powered-flight interfaces unclaimed. See `docs/COMANCHE-POWERED-TRIG.md`.

## Qualify original Luminary and Comanche gimbal entry stages

Both native gimbal entry stages match 2,560 complete original source states and sanitizer results; stock-ROM C64 validates 320 complete states and all 40 original constants. Independent review passed. Both files remain partial pending actual external helpers and downstream control paths. See `docs/GIMBAL_STAGES.md`.

## Translate actual Comanche numeric keyboard entry through NUM

Canonical Comanche CHARIN and numeric arithmetic match 8,192 original complete states with the mission-specific register and return layout. Strict and full sanitizer comparisons agree; C64 validates 64 complete-state cases. Hardware keyboard admission and interrupt composition are separate pending domains. See `docs/COMANCHE-NUMERIC-ENTRY.md`.

## Run Comanche mission services through hardware RESUME and DANZIG on disk

Returned native overlay callbacks execute real automatic T3 IRQ, qualified task services, hardware RESUME and canonical foreground selection. Host and full sanitizer evidence covers 384 original cases and 2,624 complete checkpoints; stock-ROM C64 D64 replay matches 82 checkpoints and a complete final image across 12 paths. Selected CLOCKJOB and NBDONLY flight bodies remain explicit continuation boundaries. See `docs/COMANCHE_MISSION_LOOP.md`.

## Current integration and documentation coverage

Complete portable host integration passed in an isolated committed checkout. Coverage records now include the qualified scheduler, keyboard, display, powered-flight and gimbal stages, retaining their partial scopes. Four remaining comment-only files are preserved byte-for-byte with fresh original-layout assembly equality. Whole-file accounting is now 20 native algorithm/data files plus six documentation files; both complete applications remain uncertified. See `docs/REMAINING-DOCUMENTATION.md` and `evidence/integration-current-host-proof.json`.

## Run original manual P66 descent through display and job completion on C64

The coherent Luminary manual P66 path matches 1,204 complete raw 2,052-word source checkpoints on host and under sanitizers. A real resident plus 96-module D81 reproduces 148 checkpoints through enabled V06N60 rows and actual ENDOFJOB idle, with zero additional zero-page storage and measured 772-byte software stack. Separately identified pending-job evidence covers 404 injections and 808 modeled-state checkpoints. Other flight programs and arbitrary full-CPU multi-job preemption remain outside this milestone. See `docs/P66-COMPLETE-DESCENT.md`.

## Translate canonical Comanche axis selection through actual caller return

Basic AX source control and reused rotation arithmetic match 131,072 original complete states and full sanitizer output. Two C64 PRGs verify 64 complete phase states. Source register, bank and return effects are retained; remaining powered-flight interfaces are separate domains. See `docs/COMANCHE-POWERED-AXIS.md`.

## Compose Comanche powered-flight interfaces through actual source RVQ

Eight CDU trigonometry and six transformation interfaces match 868,352 original complete state images, including real internal calls and source RVQ control. Full sanitizer streams agree; eight C64 PRGs verify 256 composed returns. SMCDURES and NBSM remain pending before whole-file certification. See `docs/COMANCHE-POWERED-INTERFACE.md`.

## Translate Comanche MAKEPLAY priority and canonical display selection

MAKEPLAY priority, clock and normal display routes match 24,576 original complete states and full sanitizer output. Strict frozen tests include 34 unchanged invalid contexts; C64 validates 64 complete states. The actual wake/sleep consumers are qualified separately. See `docs/COMANCHE-MAKEPLAY.md`.

## Execute canonical Comanche JOBWAKE with exact scheduler image

Source JOBWAKE matches 8,192 complete original states including register, bank, scan and caller effects. Full sanitizer output agrees; strict tests cover 19 unchanged invalid contexts and C64 validates 64 complete states. No full Executive coverage claim is made. See `docs/COMANCHE-BASIC-WAKE.md`.

## Deliver Comanche keyboard IRQs through genuine source RESUME and NOVAC

Native Comanche fixed-vector KEYRUPT and KEYCOM match 12,288 original complete states through genuine hardware RESUME and source CHARIN admission. Full sanitizer output agrees; C64 validates 64 complete states with the mission-specific ISR and seven-core scheduler image. UPRUPT and full keyboard composition remain separate source domains. See `docs/COMANCHE-KEYBOARD-IRQ.md`.

## Translate original Comanche verb and noun CHARIN paths

Actual Comanche verb/noun CHARIN follows source editing and return effects through ENDOFJOB across all EB contexts. All 4,096 original complete states match strict and full sanitizer output; C64 verifies 64 complete states. Broader keyboard lifecycle is qualified separately. See `docs/COMANCHE-CHARIN-NV.md`.

## Translate original Comanche TWIDDLE and remaining waitlist domains

Extended waitlist control preserves 2,048 original complete states, including TWIDDLE, invalid-delay alarm routing, both full-list mutation paths and the previously excluded positive-delay range. Strict and sanitizer checks pass; C64 validates 128 fingerprints and two complete samples. Alarm bodies remain actual continuation boundaries. See `docs/COMANCHE_WAITLIST_EXTENDED.md`.

## Broad current sanitizer audit

The isolated committed snapshot `69801cb` passes all portable sanitizer recipes, including full decoded reference-stream comparisons. Retained compressed evidence avoids duplicate reference expansion; the audit compares every byte. See `evidence/integration-current-sanitize-proof.json` and retained logs. Later additive milestones retain their own source and sanitizer qualification. This does not certify either complete application.

## Comanche mission scope correction

The retained NVWORD=0 CLOKTASK cohort updates TBASE and reschedules; it does not admit or execute CLOCKJOB. All 384 retained original source outcomes confirm this. Corrected metadata preserves prior byte copies and all source/native/C64 fidelity identities. Actual covered foreground outcomes are NBDONLY and unchanged interpreted NEWOPS. Separate clock-job body qualifications remain valid. See `evidence/cm-mission-scope-correction.json`.

## Translate actual DELCOMP helpers for both mission attitude domains

Both original DELCOMP routines match 10,240 complete canonical states and full sanitizer output through actual source RVQ, including bank and editing-word effects. Stock-ROM C64 validates 320 complete states. Containing attitude and angle files remain partial; whole gimbal compositions are being qualified separately. See `docs/DELCOMP.md`.

## Translate actual Comanche inter-bank source control and returns

Native Comanche inter-bank call and return entries preserve 7,168 original whole-image states. Strict and sanitizer checks pass; C64 validates 112 fingerprints and two full-state samples. Actual external targets remain typed continuations, with unsupported entries rejected. See `docs/COMANCHE_INTERBANK.md`.

## Resume canonical Comanche idle and selected basic jobs

Source idle dispatch matches 4,096 original complete states with actual register, bank and interpreter effects. Strict and sanitizer checks pass; C64 validates 128 fingerprints and two complete samples. This is additive Executive coverage; whole-file completion is separately audited. See `docs/COMANCHE_IDLE_DISPATCH.md`.

## Translate canonical Comanche display sleep control and wake continuation

CM GOSLEEPS and UPENT2 preserve all 6,144 original complete states, including mission-specific ITEMP1 and L effects. Strict tests cover 39 unchanged invalid boundaries; full sanitizer output and 64-state C64 proof pass. Canonical scheduler JOBSLEEP and rendering bodies remain separate qualifications. See `docs/COMANCHE-MAKEPLAY-SLEEP.md`.

## Execute canonical Comanche JOBSLEEP and real successor dispatch

Canonical Comanche JOBSLEEP matches 16,384 complete original states through basic, interpreted and idle successors while retaining the dormant VAC. Strict tests cover 23 invalid unchanged contexts and explicit source transfers; full sanitizer output and 64-state C64 proof pass. This extends bounded Executive coverage. See `docs/COMANCHE-BASIC-SLEEP.md`.

## Run actual Comanche CLOCKJOB admission and source bodies on C64 disk

Nonzero-NV source timer cohorts actually admit and execute CLOCKJOB through QUICTRIG, SWRETURN and display control to MAKEPLAY. Separate zero-NV CLOCKJOB entries finish and restore the suspended interpreter. All 448 source sequences and 3,392 complete checkpoints match host and full sanitizers; real C64 D64 replay matches 106 checkpoints across fourteen paths plus the complete final image. Downstream MAKEPLAY/rendering and other flight programs remain pending. See `docs/COMANCHE_CLOCK_MISSION.md`.

## Adopt real P66 launch continuations without adding an Executive poll

The separate direct-launch wrapper preserves the zero-pending STARTP66 domain and existing-pending vertical subentry, adopting actual RODCOMP without an extra source poll. All 2,048 complete original states and full sanitizer output agree; strict and C64 checks cover 24 callback/pending/IRQ cases. Production disk startup composition is a separate milestone. See `docs/P66-DIRECT-LAUNCH.md`.

## Run physical Comanche keyboard input through genuine job lifecycle

Actual Comanche fixed-vector keyboard IRQ, hardware RESUME, idle admission, CHARIN and canonical END preserve all 4,096 complete checkpoints across 1,024 source lifecycles. Full sanitizer output agrees; stock-ROM C64 physical GETIN replay verifies eight complete lifecycles and 32 states. Additional key bodies remain separate qualifications. See `docs/COMANCHE-KEYBOARD-RUNTIME.md`.

## Complete native Luminary and Comanche gimbal source files

Both complete original gimbal files now have reviewed native mappings, 10,240 exact complete source/native/sanitized states and 80 C64 states per application with all 40 constants checked. NEWANGL continuation bodies and live scheduling remain application integration work. See `docs/GIMBAL_COMPLETE.md`.

## Compose actual Comanche display sleep wake and successor lifecycle

The actual GOSLEEPS, JOBWAKE, XCHTOEND/XCHSLEEP and JOBSLEEP sequence now matches 2,048 complete source/native/sanitized checkpoints across 512 continuous paths. Stock C64 execution verifies 64 complete checkpoints across 16 paths; whole application acceptance remains pending. See `docs/COMANCHE-DISPLAY-SLEEP-LIFECYCLE.md`.

## Close original Comanche powered flight numerical and fixed return groups

The remaining SMCDURES, legacy NBSM, original QUICTRIG return and all-bank RVQ groups now have source-exact native C proofs. Seven cumulative packets compare 2,187,264 full source/native/sanitized states and 1,600 C64 states across 50 diagnostic PRGs. All 187 emitted source words, including five mixed-file constants, are mapped; whole-file inventory certification awaits independent review. See `docs/COMANCHE-POWERED-FLIGHT.md`.

## Translate canonical Luminary CHANG2 to genuine basic and interpreter returns

The source CHANG2 helper matches 16,384 complete 2052-word source/native/sanitized transitions and 64 complete disk-backed C64 checkpoints. Transactional invalid-entry guards are verified. Scheduler action adoption and live multijob integration remain separate work. See `docs/LM-INTERPRETER-CHANGE.md`.

## Preserve genuine higher priority and pending IRQ through P66 direct launch

The direct P66 launch preserves the original higher-priority job and pending IRQ rather than inserting an extra Executive poll. A complete original-source case is unchanged across strict host, sanitizer and actual C64 execution. See `docs/P66-DIRECT-PENDING.md`.

## Complete original Comanche powered-flight subroutine source

Independent review reconstructed every one of 187 original emitted words, checked all seven qualification dependencies and exhaustively validated the five-word constant accessor. The whole source file is now certified within its documented numerical and return contracts. The inventory has 29 complete files (23 native/data/declaration and six comment-only files), with 146 source files and both integrated applications still requiring complete acceptance. See `docs/COMANCHE-POWERED-FLIGHT.md`.

## Translate actual Luminary and Comanche DSKY plus and minus key entry

Both original CHARIN paths now execute SIGNTEST, +ON/-ON, DSPIN/FIXCLPAS and genuine ENDOJOB. 8,192 complete source/native/sanitized states and 64 complete C64 checkpoints pass; numeric display input remains source-owned. See `docs/DSKY-SIGN-ENTRY.md`.

## Fit genuine Luminary P66 startup seams into native C64 disk overlays

STARTP66 setup, bias and vertical stages now load through fixed source-call seams without an added Executive poll. Strict and full sanitizer runs match 2,816 complete original states. A stock C64 disk proof passes 88 complete checkpoints across 32 cases; all eight application service payloads fit, with three startup stages executed in this packet. See `docs/P66-STARTUP-OVERLAYS.md`.

## Translate generic Luminary and Comanche MXV and VXM source phases

Both original generic matrix/vector paths now match 65,536 complete source/native/sanitized states with raw overflow A and original scratch retained. Stock C64 execution passes 128 complete checkpoints and 66 transactional invalid-entry guards. Operand loading, DANZIG dispatch and full Interpreter/application coverage remain separate work. See `docs/NATIVE-MATRIX.md`.

## Translate actual Luminary and Comanche DSKY Clear key editing

Both original Clear paths now preserve negative-zero DIM semantics and perform CLPASS component backtracking and actual UPDATVB redisplay. 16,384 full source/native/sanitized states and 64 complete C64 checkpoints pass. Superseded diagnostic-build failure evidence is retained beside the successful target build. See `docs/DSKY-CLEAR-ENTRY.md`.

## Qualify exact passive original fixed word storage for both applications

Both original fixed-word datasets now have data-only native readers: 36 populated banks and four zero banks per mission. Strict and sanitizer audits exhaust all 65,536 reader arguments per link group. Stock C64 reads match all 81,920 individual words per mission across both IRQ states while preserving the canonical image and ROM mapping. The LM allocation-only disk retains every existing file; executable registry allocation and source caller controls remain separate work. See `docs/PASSIVE_FIXED_STORAGE.md`.

## Run original Comanche fixed memory display and checksum call prefixes

Actual DSPFMEM and COMADRS lead-ins now read the entire 40,960-address original fixed-word domain through native SUPDACAL and return to genuine callers. 81,920 source sequences and 163,840 complete source/native/sanitized checkpoints pass; stock C64 verifies 64 sequences and 128 checkpoints with full-state samples. Passive data is separately exact-word qualified. Later octal display/checksum bodies and whole application acceptance remain pending. See `docs/COMANCHE_FIXED_CALLERS.md`.

## Translate actual Luminary and Comanche Reset and Error key behavior

Both original RSET/ERROR paths now preserve their differing failure registers and restart-light effects, including the negative-zero CADRSTOR key-release corner. 8,192 complete source/native/sanitized states and 64 complete C64 checkpoints pass. See `docs/DSKY-RESET-ENTRY.md`.

## Adopt source-selected canonical interpreter exchanges in the native scheduler

Native scheduler adoption now preserves the exact source-selected CHANG2 context without repeating exchange or admitting pending IRQs. All 16,384 complete source transitions and 64 real three-overlay C64 cases are qualified; historical scheduler and complete P66 regressions pass. Remaining application bodies and live scheduling paths retain separate completion gates. See `docs/NATIVE-CANONICAL-EXCHANGE.md`.

## Translate remaining original Comanche extended interbank control groups

Fourteen original interbank groups are qualified across 901,120 source-executed complete states and 16,384 explicitly rejected invalid typed BANKJUMP inputs, with full sanitizer equality and 56 actual C64 contexts. Ordinary caller return contracts and whole-file completion remain separate work. See `docs/COMANCHE_INTERBANK_EXTENDED.md`.

## Translate original Comanche dynamic display numerical caller and callee route

Original DYNDISP, P40CNV85 and S41.1 now compose CDU, matrix and rotation helpers through 27 native stages. All 13,824 complete uninterrupted host source states match strict and full sanitizer output; 68 frozen caller transitions pass on C64. Noun85 display formatting and live Executive scheduling remain separate integration work. See `docs/COMANCHE-DYNDISP-NUMERICS.md`.

## Translate original Luminary and Comanche invalid keyboard alarms

Actual CHARALRM and FALTON behavior, octal8/9 rejection and blocked negative-count editing match 8,192 complete original states under strict and full sanitizer comparison. All64 frozen complete states pass on C64. Wider Pinball continuation composition remains ongoing. See `docs/DSKY-INVALID-KEY.md`.

## Translate all original Luminary powered flight numerical and return groups

All 267 original emitted source words and six constants now map to seven reviewed native groups. Complete source/native/full sanitizer proofs cover 1,982,464 states; 38 actual C64 diagnostics qualify 1,216 complete states. Independent whole-file review includes exact composed FLESHPOT return contracts. The whole-file certificate and application runtime acceptance remain separate steps. See `docs/LUMINARY-POWERED-FLIGHT.md`.

## Complete original Luminary powered-flight subroutine source

Independent review reconstructed every one of 267 original emitted words, verified all seven complete source/sanitizer qualification dependencies and 38 actual C64 diagnostics, and exhausted the six-word constant accessor. The whole source file is certified within its published numerical and return contracts, including composed normalized FLESHPOT return inputs. The inventory has 30 complete files (24 native/data/declaration and six comment-only files), with 145 source files and both integrated applications still requiring complete acceptance. See `docs/LUMINARY-POWERED-FLIGHT.md`.

## Translate real Luminary and Comanche Key Release source continuations

Actual KEYREL, RELDSP and display job wake paths match8,192 complete original source states under strict and full sanitizer comparison. Both CADRSTOR signed zeros, source literal-DAS fallthrough and true END/PINBRNCH/BLNKSUB1 continuations are retained. All64 complete target states pass; later continuation bodies remain active work. See `docs/DSKY-KEY-RELEASE.md`.

## Compose actual Comanche normal noun85 display and Executive completion

Actual normal V06N85 now composes source display selection, priority and job wake, NVSUB, fresh three-row scaling and decimal formatting, and original Executive completion. All 13,312 uninterrupted complete host source states match full sanitizer output; 60 frozen caller states pass on C64. Flashing, busy, monitor and relay-drain paths remain separate work. See `docs/COMANCHE-NORMAL-N85.md`.

## Translate actual Comanche canonical CHANG1 and CHANG2 context exchanges

All outgoing and selected basic/interpreter combinations now preserve the original CHANG1 and CHANG2 registers, canonical image and selected return tuple. All12,288 complete original states match strict and full sanitizer output, and96 actual fixed-slot C64 states pass. Selected application bodies and whole Executive completion remain separate work. See `docs/COMANCHE_CONTEXT_CHANGE.md`.

## Translate original Luminary and Comanche inverse trigonometric source stages

Native ARCSIN and ARCCOS sign, clamp, root, polynomial and epilogue stages match316,640 complete original states under strict and full sanitizer comparison. Eight actual C64 diagnostics pass256 complete states and every original coefficient. Root independently checked168 original emitted audit words and131,072 constant-access arguments. Original alarm handler, post-alarm continuation and whole INTERPRETER completion remain separate work. See `docs/INVERSE-TRIG.md`.

## Compose uninterrupted original Comanche dynamic display through actual job completion

All64 original allocated-VAC DYNDISP paths now compose numerical transformation, normal V06N85 display and source END/idle processing. All3,392 complete checkpoints match strict and full sanitizer output, including actual VAC/core release. Constituent C64 proofs remain independent; resident scheduler execution and relay drain are separate acceptance work. See `docs/COMANCHE-DYNAMIC-DISPLAY.md`.

## Translate original Comanche job and VAC admission including resource exhaustion

Actual NOVAC, FINDVAC, SPVAC and SPVACIN success and resource-exhaustion effects match12,288 complete source states under strict and full sanitizer comparison. Ninety-six fixed-slot C64 contexts pass, including reserved VAC retention on no-core bailout. The actual alarm body and admitted application bodies remain separate work. See `docs/COMANCHE_ADMISSION_COMPLETE.md`.

## Translate complete original Comanche RCS matrix update job source

All33 emitted words of the RCS matrix update file now map to reviewed native gates, trigonometric calls, angular offset, products, stores, actual END entry and constant. All131,072 complete original states match strict and full sanitizer output; 64 actual C64 states pass. Independent review confirms source behavior; whole containing-file certification follows separately. See `docs/COMANCHE-RCS-MATRIX.md`.

## Complete original Comanche RCS matrix-update source

Independent arithmetic/control review, exact original33-word mapping, complete source/sanitizer comparisons and actual C64 execution certify the containing RCS matrix-update file within its canonical source entry and external END contracts. The inventory has31 complete files (25 native/data/declaration and six comment-only files), with144 source files and both integrated applications still requiring complete acceptance. See `docs/COMANCHE-RCS-MATRIX.md`.

## Translate original Luminary and Comanche display blanking and banked return

Actual BLNKSUB1 and SUPDXCHZ+1 return phases match8,192 complete original states under strict and full sanitizer comparison, including both zero forms, opaque BUF inputs, all erase banks and supported saved bank tuples. All64 frozen complete C64 states pass. Normal BLANKSUB entry and PINBRNCH/ENTER continuations remain active work. See `docs/DSKY-BLANK-RETURN.md`.

## Translate original Comanche priority changes through mixed source context switches

Actual PRIOCHNG now reaches genuine current and selected basic or interpreter source returns with complete register and canonical state preservation. All14,336 complete source states match strict and full sanitizer output, and112 disk-backed C64 contexts pass. Source CCSHOLE/+0 faults and selected application body execution remain separate work. See `docs/COMANCHE_PRIORITY_COMPLETE.md`.

## Translate complete original Luminary and Comanche system test lead ins

Both containing source files are translated with every original entry and bank-return branch,35,328 complete original/source/full-sanitizer states and352 actual C64 checkpoints. Independent review corrected and separately tested the inherited IRQ state at INTPRET+2. External caller, display and recovery bodies remain separate. See `docs/SYSTEM_TEST_LEAD_INS.md`.

## Complete original system test lead-in sources

Independent original-source review, complete source/sanitizer comparisons and actual C64 execution certify both SYSTEM_TEST_STANDARD_LEAD_INS files within their original caller and external continuation contracts. The inventory has33 complete files (27 native/data/declaration and six comment-only files), with142 source files and both integrated applications still requiring complete acceptance. See `docs/SYSTEM_TEST_LEAD_INS.md`.

## Translate original Luminary and Comanche display branch selection

Original PINBRNCH and UPFLAG paths through ERASER, MARKPLAY, KEEPPRIO and PLAYJUM1 match16,384 complete original and full-sanitizer states, with64 actual C64 checkpoints. Each retains the source-selected GOPLAY continuation and original register and IRQ ownership. See `docs/DSKY-PINBRANCH.md`.

## Qualify exhaustive inverse trig high word inputs for both missions

A separate supplementary corpus independently inspects all32,768 high-word values per function in both missions, closing the2,045-value omission in the original corpus. All327,668 complete original/native/full-sanitizer states match; eight actual C64 diagnostics pass256 states and all36 constants. Original frozen algorithms and proof identities are unchanged. See `docs/INVERSE-TRIG-SWEEP.md`.

## Translate original Luminary and Comanche ordinary display blanking

Actual BLANKSUB caller setup, source selected buffer and unchanged BLNKSUB1 bank return match8,192 complete source/full-sanitizer states and64 actual C64 checkpoints. The native return preserves actual busy Q and available Q+1 selection without an extra Executive poll. See `docs/DSKY-BLANKSUB.md`.

## Translate original Luminary and Comanche ENTER pass selection

Actual CHARIN34 and ENTER through pass selection, source FLASHOFF and physical REQRET return match8,192 complete original/full-sanitizer states and64 actual C64 checkpoints. Signed-zero request/count contexts and the actual MMCHANG request-count exception are preserved; subsequent noun and verb bodies remain separate. See `docs/DSKY-ENTER-PASS.md`.

## Translate original Comanche minutes and seconds display scaling

Actual M/SOUT scaling, separation, signed two-character formatting and middle blanking match512 continuous paths and3,072 complete source/full-sanitizer states; the reusable decimal pair matches1,024 additional states. Actual C64 checks112 complete frozen states plus63 unchanged guards. Surrounding noun40 caller composition remains separate. See `docs/COMANCHE-MINUTES-SECONDS.md`.

## Run genuine Luminary P66 startup guidance and display jobs through native C64 application routing

Actual STARTP66, guidance, deferred mode and display jobs, canonical END and idle pass16,586 complete original/native/full-sanitizer host states. A104-module native C64 disk application passes156 complete source fingerprints with resident end$6810 and2,032 bytes remaining below canonical RAM. Clean host verification streams retained original archives. Keyboard bodies, recurrence and full Luminary flight application acceptance remain separate. See `docs/P66-APPLICATION-STARTUP.md`.

## Translate complete original Luminary SPS backup RCS source

All82 byte-original emitted words including jet-sense, deadband, outer-rate, coast and inhibition paths plus four constants are reviewed and mapped. All196,608 complete original states match strict and full sanitizer execution;96 actual C64 states and every constant pass. No complete docked autopilot or flight application acceptance is claimed. See `docs/LUMINARY-SPS-RCS.md`.

## Complete original Luminary SPS backup RCS source

Independent arithmetic/control review, exact original82-word mapping, complete source/sanitizer comparisons and actual C64 execution certify the containing SPS backup RCS file within its canonical source entry and external END contracts. The inventory has34 complete files (28 native/data/declaration and six comment-only files), with141 source files and both integrated applications still requiring complete acceptance. See `docs/LUMINARY-SPS-RCS.md`.

## Translate original Luminary and Comanche ENTER noun admission

Actual ENTPAS0 noun selection composes the unchanged fixed-data noun reader and original SETNCADR stores through source-selected VERBFAN, MIXNOUN, GODSPALM, REQADD and INTMCTBS boundaries. All8,192 complete original/full-sanitizer states and64 actual C64 checkpoints match, including all nouns0..99, signed zero and retained bank contexts. Subsequent verb and mixed-noun bodies remain separate. See `docs/DSKY-ENTER-ZERO.md`.

## Translate original Luminary idle task dispatch and saved frame resume bridges

Literal idle-to-basic, T3-vector-to-task and TASKOVER architectural RESUME transfers match24,576 complete original/native/full-sanitizer states and128 actual C64 states across two stock-ROM disk proofs. The dispatch packet retains its tested IRQ0 contract; production hardware IRQ1 dispatch and generic selected task bodies remain separate. See `docs/LM-RECURRENCE-BRIDGES.md`.

## Compose original Comanche normal noun40 and noun85 display source paths

The generalized noun-table caller and source row pipeline match18,688 uninterrupted complete original/native/full-sanitizer states across512 noun40 and128 noun85 paths. All64 frozen caller states and108 unchanged guards pass, with actual C64 caller execution. Whole-chain host composition, downstream clock mission integration and display device acceptance remain separately identified. See `docs/COMANCHE-NORMAL-N40.md`.

## Translate original Luminary and Comanche mixed noun selection

Actual MIXNOUN source scaling selection and noun addressing match8,192 complete original/native/full-sanitizer states and64 actual C64 states. The mission-specific routine13 difference is preserved: Luminary360-CDU uses a single word while Comanche DPFRACOT uses a double-word minor. Subsequent verb and request continuations remain separate. See `docs/DSKY-MIXED-NOUN.md`.

## Translate original Luminary and Comanche scalar triple and vector sign operations

Source SIGN and COMP preserve mission-specific entry and DANZIG handoffs, original complement/control effects and every retained image word. All262,144 complete original/native/full-sanitizer states match; four actual C64 diagnostics pass256 states. Independent factory audit exhausts admitted MODE and ordinary operand/high words. Caller operand binding and whole INTERPRETER completion remain separate. See `docs/INTERPRETED-SIGN.md`.

## Run actual Comanche clock display normal sleep and busy paths on native C64 disk

Actual timer admission, CLOCKJOB trigonometry, MAKEPLAY, mixed source priority and normal GOPLAY, sleep or matching-TEMPFLSH busy END match128 complete host/source sequences and2,560 full-sanitizer checkpoints. A stock C64 disk passes6 continuous sequences and122 full boundaries plus6 complete terminal images. The earlier timed-out8context trial is retained explicitly. Noun40 conversion and complete application acceptance remain separate. See `docs/COMANCHE_CLOCK_DISPLAY_MISSION.md`.

## Translate original Luminary and Comanche verb dispatch selection

Actual VERBFAN handles all40 first-list verbs through mission-specific passive CADR data and source BANKJUMP, while extended40..99 reaches the real RELDSP boundary. All8,192 complete original/native/full-sanitizer states and64 actual C64 states match. Selected verb bodies remain explicit external source continuations. See `docs/DSKY-VERB-DISPATCH.md`.

## Compose genuine Comanche CLOCKJOB through noun40 display and actual Executive return

Nine unchanged original clock mission contexts now run continuously through normal noun40 formatting and canonical END restoring the actual suspended interpreter. All387 complete2060-field original/native/full-sanitizer checkpoints match. Nine genuine END transitions and36 unchanged guards pass on C64. Whole continuous chain is host qualified; combined resident loading and display device acceptance remain separate. See `docs/COMANCHE-CLOCK-NORMAL-DISPLAY.md`.

## Translate original Luminary and Comanche double precision square source operations

Original DSQ and direct DSQSUB preserve source arithmetic, caller-Q and mission-specific DANZIG handoffs across17 emitted words per interpreter. All262,144 complete original/native/full-sanitizer states match; four actual C64 diagnostics pass256 complete states. Independent review finds no discrepancy. Whole INTERPRETER and caller operand dispatch remain separate. See `docs/INTERPRETED-SQUARE.md`.

## Translate original Luminary and Comanche internal display release

Internal RELDSP executes actual JOBWAKE and the ordinary RELRET caller return across arbitrary DSPLOCK and signed core lists. All8,192 complete original/native/full-sanitizer states and64 actual C64 states match, retaining branch-sensitive unused Executive fields and source producer exclusion. Extended verb fanout and selected body execution remain separate. See `docs/DSKY-INTERNAL-RELEASE.md`.

## Translate original Luminary and Comanche extended verb fanout

Actual POSTJUMP and GOEXTVB execute all60 source-selected TC/TCF fanout branches with exact CPU, Q and bank effects. All8,192 complete original/native/full-sanitizer states and64 actual C64 states match. Mission-specific V96 and V49 return-Q differences are preserved; verb69 stops at its genuine restart entry. See `docs/DSKY-EXTENDED-DISPATCH.md`.

## Translate complete original Luminary interbank communication source

All14 source entries and76 emitted words, source control aliases and allfour negative-overflow BANKJUMP successors are mapped and independently reviewed. All921,088 complete original/native/full-sanitizer states and448 actual C64 states match. The invalid register-Q fault frontier is retained before any external instruction fetch, alongside ordinary compiled-caller/saved-return bounds and quiesced producers. See `docs/LUMINARY-INTERBANK.md`.

## Complete original Luminary interbank source

All14 entries and76 emitted words, including allfour negative-overflow MAKECADR successors, are qualified by independent review,921088 complete source/native/sanitizer states and448 actual C64 states. The certificate uses the historical baseline and retains the compiled-caller domain and external return/fault boundaries.35 files are complete;140 files and both full applications remain open. See `docs/LUMINARY-INTERBANK.md`.

## Translate original display alarm and relay source paths

Actual source alarm display, relay controls and monitor termination match8192 complete original/native/full-sanitizer states and64 actual C64 states. Keyboard, scheduler and abort continuations retain their actual source handoffs. See `docs/DSKY-SOURCE-ALARM.md`.

## Translate original Comanche mixed display initialization and priority sequences

Actual INITDSP prefixes and mixed source priority continuations match15360 complete original/native/full-sanitizer checkpoints across7168 sequences and120 actual C64 checkpoints across56 sequences. Selected job bodies remain explicit later continuations. See `docs/COMANCHE_INIT_DISPLAY_MIXED.md`.

## Translate complete original Luminary RCS failure monitor source

All69 original source words and18 fixed constants are mapped and binary-checked. Channel32 changes, both valve-mask directions and actual NOVAC admission match131072 complete original/native/full-sanitizer states and192 actual C64 states. External RESUME, resource-alarm processing and admitted ACCJOB bodies retain explicit source boundaries. See `docs/LUMINARY-RCS-MONITOR.md`.

## Complete original Luminary RCS failure monitor source

Independent arithmetic/control review, exact original69-word mapping, complete source/sanitizer comparisons and actual C64 execution certify the containing RCS failure monitor file within its canonical source entry, hardware channel32 and external RESUME/BAILOUT contracts. The inventory has36 complete files (30 native/data/declaration and six comment-only files), with139 source files and both integrated applications still requiring complete acceptance. See `docs/LUMINARY-RCS-MONITOR.md`.

## Translate original shared verb noun display admission source

Both original NVSUB/NVMONOPT and NVSUB1/2 selection bodies match8192 complete original/native/full-sanitizer states,80 unchanged invalid-input guards and64 actual C64 states. Source-selected callee and blocked caller boundaries remain explicit; formatter and selected callee bodies have separate qualifications. See `docs/DSKY-NV-ADMISSION.md`.

## Translate original shared load response recall and job wake paths

Actual VBPROC/VBTERM/VBRESEQ/PROCKEY and direct RECALTST match8192 complete original/native/full-sanitizer states and64 actual C64 states, including source-signed-zero load status and not-found LOCCTR minus one fixed-data exchanges. Both internal RELDSP calls and real JOBWAKE preserve source effects and actual END handoffs. See `docs/DSKY-LOAD-CONTROL.md`.

## Canonical Luminary admission source entries

Literal NOVAC/FINDVAC/SPVAC/SPVACIN and their real caller/BAILOUT handoffs match16384 complete2050-field original/native/full-sanitizer states,22 unchanged guard domains and64 actual C64 disk-overlay states. Admitted bodies and recovery remain separate. Retained-original checks require no emulator or stale build outputs. See `docs/LM-CANONICAL-ADMISSION.md`.

## Translate original Comanche alarm recording and overflow recovery handoffs

Actual ALARM/VARALARM/CURTAINS/BORTENT match16384 complete original/native/full-sanitizer states and64 actual C64 states. All1803 genuine MULTFAIL negative-overflow paths retain their inhibited-IRQ POODOO frontier; ordinary returns and later abort recovery bodies remain separately qualified. See `docs/COMANCHE_ALARM_RECORD.md`.

## Translate original shared saved display caller return source

Both original NVSUBEND entries and SUPDXCHZ saved fixed-caller return match8192 complete original/native/full-sanitizer states,22 unchanged invalid-input guards and64 actual C64 states. Raw registers, all saved bank/EB/SUPER combinations and actual source caller entries are retained; caller body execution remains separate. See `docs/DSKY-NV-RETURN.md`.

## Translate original shared blocked display sleep admission source

Both original ENDIDLE/NVSUBUSY/PRENVBSY/NVSBWAIT/NVSUBSY1 bodies match10240 complete original/native/full-sanitizer states,120 unchanged invalid-input guards and80 actual C64 states. Actual JOBSLEEP/POODOO/NVSBCOM entries remain explicit, including the original CADRSTOR negative-zero behavior; selected service execution remains separate. See `docs/DSKY-NV-BUSY.md`.

## Translate original Comanche abort snapshot and GENTRAN copy source

Actual VAC5STOR and containing GENTRAN copy effects match8192 complete original/native/full-sanitizer states and64 actual C64 states, preserving canonical scratch aliases and the source-selected SWRETURN entry. Abort producers and recovery bodies are separate source-qualified continuations. See `docs/COMANCHE_VAC5_STORE.md`.

## Translate original shared load response interrupt shims

Actual JAMPROC/JAMTERM fixed entries, channel7 write event, REQRET controls and unchanged load/wake/END composition match8192 complete original/native/full-sanitizer states and64 actual C64 states. Producer exclusion and real source IRQ/channel ownership remain explicit. See `docs/DSKY-JAM-CONTROL.md`.

## Integrate actual native P66 timer dispatch and second descent iteration

The111-module native C64 disk reproduces STARTP66, guidance, display and idle,100 actual timer pulses, original T3 RODTASK admission/TASKOVER resume and a second P66/display/idle iteration. All310 complete actual C64 source checkpoints and13926 clean strict/full-sanitizer source states match. Hardware stack77B, software785B, resident6f59 and171B control metadata fit the documented independent allocations. One-shot source task behavior and remaining application/device/recovery domains are explicit. See `docs/P66-APPLICATION-RECURRENCE.md`.

## Translate complete original Comanche interbank communication source

All14 source entries and76 emitted words, source control aliases and allfour negative-overflow BANKJUMP successors are mapped and independently reviewed. All921,088 complete original/native/full-sanitizer states and448 actual C64 states match. The invalid register-Q fault frontier is retained before any external instruction fetch, alongside hardened compiled-caller/saved-return bounds, quiesced producers and all11 unrelated CM I/O fields unchanged. The two-image target fixture establishes standalone stock C64 behavior; production overlay allocation remains separate. See `docs/COMANCHE-INTERBANK.md`.

## Complete original Comanche interbank source

All14 entries and76 emitted words, including allfour negative-overflow MAKECADR successors, are qualified by independent review,921088 complete source/native/sanitizer states and448 actual C64 states. The certificate uses the historical baseline and retains the compiled-caller domain and external return/fault boundaries.37 files are complete;138 files and both full applications remain open. See `docs/COMANCHE-INTERBANK.md`.

## Translate original Comanche blocked display REST source

Original NVDSP blocked return, REST/RESTSLEP and UPENT2 alias effects match3072 complete original/native/full-sanitizer states,30 unchanged invalid-input guards and60 actual C64 states. Real ENDOFJOB and NVSUBUSY entries remain explicit source handoffs; selected lifecycle body execution is separate. See `docs/COMANCHE-DISPLAY-REST.md`.

## Compose original Comanche clock display refusal and real sleeping job transfer

Seven unchanged original clock mission paths preserve blocked NVMONOPT, actual REST/NVSUBUSY/JOBSLEEP and real restored-interpreter selection across175 uninterrupted full-source/native/sanitizer checkpoints. Actual C64 checks the seven original before-JOBSLEEP states through source transfer plus28 unchanged invalid guards; a combined resident disk/device proof remains separate. See `docs/COMANCHE-CLOCK-BLOCKED-DISPLAY.md`.

## Canonical Luminary waitlist source entries

Literal WAITLIST/TWIDDLE and their real caller-return/1203 BAILOUT/1204 POODOO handoffs match16384 complete2050-field original/native/full-sanitizer states,17 unchanged guard domains and64 actual C64 disk-overlay states. Delay/longcall/SVCT3, selected task and recovery bodies remain separate. Retained-original checks require no emulator or stale build outputs. See `docs/LM-CANONICAL-WAITLIST.md`.

## Translate original shared internal verb noun display continuations

Nine original internal NVSUB continuation entries per mission match18432 complete original/native/full-sanitizer states,180 unchanged invalid-input guards and72 actual C64 states. Actual noun admission, formatter returns, bank selection and saved source control handoffs remain explicit; selected external bodies retain their separate qualifications. See `docs/DSKY-NV-CONTINUATION.md`.

## Translate original shared verb noun display update source bodies

Actual UPDATVB/UPDATNN and minus-one entries, immutable noun table selection, real mission two-digit formatters and original caller return match8192 complete original/native/full-sanitizer states,96 unchanged guards and128 actual C64 states. All source verb/noun values0..99 and canonical bank/control effects are retained. See `docs/DSKY-NV-UPDATE.md`.

## Canonical Luminary basic context change source entries

Literal CHANG1/CHANJOB basic or interpreter exchanges match16384 complete2053-field original/native/full-sanitizer states,25 unchanged guard domains and64 actual C64 disk-overlay states. Raw incoming L to Q overflow and actual selected transfers are retained. Selected application bodies remain separate. Retained-original checks require no emulator or stale build outputs. See `docs/LM-BASIC-CHANGE.md`.

## Translate original shared octal and bit display formatter bodies

Actual DSPOCTWD/DSPOCTWO and DSP2BIT source bodies match8192 complete original/native/full-sanitizer states and64 actual C64 states across all coherent erasable banks and four upper SUPER forms. Clean positive-zero source branches retain the real CCSHOLE image; external exceptional recovery remains separate. See `docs/DSKY-OCTAL-DISPLAY.md`.

## Translate original shared machine address display continuation

Actual INTMCTBS and plus-two entries, SETNCADR, verb5 bypass, real octal formatting and source VERBFAN return match8192 complete original/native/full-sanitizer states and64 actual C64 states. Canonical registers, bank/control ownership and CCSHOLE propagation remain exact. See `docs/DSKY-MACHINE-CADR.md`.

## Translate original shared five character numeric blank entries

Actual5BLANK, minus-one, plus-two and CLR5 bodies match8192 complete original/native/full-sanitizer states and64 actual C64 states, covering all numeric row positions and source paired/isolated hole branches. The ordinary numeric calling domain and separate verb/noun blank controls remain explicit. See `docs/DSKY-FIVE-BLANK.md`.

## Translate original RTB angle conversion time and PIPA source groups

Both original mission converters, CDU controls and clock reads plus Comanche PIPA reads match1048576 complete original/native/full-sanitizer states and512 actual C64 states across16 targets. Source mapping and binary checks retain containing file boundaries:72 of130 Luminary words and97 of206 Comanche words are qualified; other RTB entries remain open. See `docs/RTB-ANGLES.md`.

## Translate original Comanche abort snapshot and alarm entry chronology

Qualify BAILOUT POODOO and CCSHOLE through actual VAC5 GENTRAN and BORTENT against uninterrupted original machine checkpoints, retaining the corrected diagnostic failure evidence and explicit recovery frontiers. See `docs/COMANCHE_ABORT_SNAPSHOT.md`.

## Translate original Comanche WHIMPER resume and recovery handoff

Qualify actual WHIMPER RESUME POSTJUMP and ENEMA handoff against8192 original whole-machine states and64 C64 states with explicit finite return metadata. See `docs/COMANCHE_ABORT_WHIMPER.md`.

## Translate original Comanche phase cleanup recovery entries

Qualify MR.KLEAN P00KLEAN and V37KLEAN source phase clearing against12288 full original native and sanitizer states plus48 C64 states preserving the inactive zero complement pair. See `docs/COMANCHE_PHASE_CLEANUP.md`.

## Translate original RTB sign agreement and Comanche matrix transpose entries

Qualify original sign agreement and Comanche transpose entries against393216 complete original native and full sanitizer states plus192 actual C64 states across six targets. Remaining RTB entries stay open. See `docs/RTB-AGREEMENT.md`.

## Translate original shared address and data request source paths

Qualify REQADD USEADD REQDAT and REQCOM with actual BANKCALL five blank SWRETURN and saved data transfers against8192 complete original native sanitizer states plus64 C64 states preserving real alarm cycle boundaries. See `docs/DSKY-ADDRESS-REQUEST.md`.

## Translate original internal display blank source continuation

Qualify original BLANKDSP through ENTSET minus2 with source relay CCS and NOUT wrapping against8192 complete original native sanitizer states and128 C64 states without resetting canonical machine state. See `docs/DSKY-NV-BLANK.md`.

## Compose original continuous internal display consumers and returns

Qualify1792 uninterrupted original calls and5888 complete checkpoints through admission verb noun blank and true saved caller return plus28 C64 sequences keeping combined VN actual ENTPAS0 as the explicit remaining consumer. See `docs/DSKY-NV-CONSUMER.md`.

## Translate both complete original service routine source bodies

Both mission SERVICE_ROUTINES bodies including flags delays wake and generic GENTRAN loops match106496 complete original native sanitizer states and416 actual C64 states. All219 original emitted words independently match immutable ropes; external scheduler alarm and caller continuations remain explicit. See `docs/NATIVE-SERVICE-ROUTINES.md`.

## Complete original mission service routines

Both original SERVICE_ROUTINES files are certified for their compiled caller domains: all219 emitted words independently match the unchanged original ropes,106496 full source/native/sanitizer states and416 actual C64 states pass, and independent reviewers checked every source path. External scheduler, alarm and caller continuations remain explicit.39 source files are complete;136 files and both full applications remain open. See `docs/NATIVE-SERVICE-ROUTINES.md`.

## Translate original shared alarm recycle call and return source bridges

Qualify actual ALMCYCLE prefix BANKCALL UPDATVB minus1 selection and SWRETURN ENTER bridges against8192 complete original native sanitizer states plus64 C64 states leaving the selected update body separately qualified. See `docs/DSKY-ALARM-CYCLE.md`.

## Qualify generalized original verb noun update signed zero paths

Extend separately native source qualified update bodies to either signed zero all source selecting SUPER banks and actual clean relay CCSHOLE branches with8192 full original native sanitizer states and128 C64 states preserving prior frozen updater. See `docs/DSKY-NV-UPDATE-GENERAL.md`.

## Canonical Luminary DANZIG context change source entries

Literal DANZIG/NOIBNKSW through CHANG2 basic or interpreter exchanges match16384 complete2053-field original/native/full-sanitizer states,25 unchanged guard domains and64 actual C64 disk-overlay states. Raw incoming L to Q overflow and actual selected transfers are retained. Selected application bodies remain separate. Retained-original checks require no emulator or stale build outputs. See `docs/LM-DANZIG-CHANGE.md`.

## Translate original RTB vector agreement and literal normalization handoffs

Qualify complete VECAGREE and VECSGNAG control plus literal UNIT and IMUPULSE handoffs against655360 complete original native and full sanitizer states plus320 actual C64 states across ten targets. Selected UNIT and IMUPULSE bodies remain separately open. See `docs/RTB-VECTOR.md`.

## Translate original Comanche canonical ENEMA recovery image

Qualify actual ENEMA whole physical image recovery against4096 original native sanitizer states and32 C64 checkpoints with explicit MMDSPLAY PTBAD DOFSTART continuations retaining failed automatic stack build and qualified reset transient scratch workaround. See `docs/COMANCHE_ENEMA_IMAGE.md`.

## Compose uninterrupted original alarm recycle and display update

Qualify actual ALMCYCLE lamp event BANKCALL widened UPDATVB SWRETURN POSTJUMP and ENTER or CCSHOLE over8192 uninterrupted original native sanitizer images plus64 C64 states with all signed zero and selecting SUPER contexts. See `docs/DSKY-ALARM-RECYCLE.md`.

## Translate original generic decimal display fresh data load

Qualify both actual DECDSP loads to DSPDCPUT with normal mixed nouns CPU and editing aliases over8192 complete original native sanitizer images plus128 C64 states keeping scale factor and formatter selection separate. See `docs/DSKY-DECIMAL-LOAD.md`.

## Compose original uninterrupted Comanche POODOO recovery chronology

Qualify actual POODOO mode test three DOWNFLAG operations BANKCALL MR.KLEAN SWRETURN and true WHIMPER over4096 uninterrupted original sequences with16384 complete native sanitizer checkpoints plus16 C64 sequences retaining harness failure proofs and unchanged native220. See `docs/COMANCHE_ABORT_RECOVERY.md`.

## Translate original operator major mode request source controls

Qualify actual MMCHANG REQMM validation numeric blank POSTJUMP and genuine V37 or alarm cycle handoff over8192 full original native sanitizer states plus64 C64 states preserving explicit selected program bodies. See `docs/DSKY-MAJOR-REQUEST.md`.

## Qualify original Comanche positive zero core admission scan

Extend canonical source admission to defined positive zero core scan paths over86016 complete original native sanitizer states plus56 C64 cases correcting earlier NO.CORES interpretation additively while preserving frozen proofs. See `docs/COMANCHE_ADMISSION_ZERO.md`.

## Compose original RTB and UNIT numerical normalization continuations

Qualify NORMUNIT NORMUNX1 and normal UNIT callers through complete125 word UNIT UNITDV and VSQSUB source group per mission against1179648 original native sanitizer states and576 C64 cases across eighteen targets. Generic interpreter and IMU output bodies remain separate. See `docs/RTB-UNIT.md`.

## Translate original operator request and scheduler call source stages

Qualify actual VBRQEXEC VBRQWAIT REQUESTC and V30 V31 dispatch through literal RELDSP NOVAC FINDVAC WAITLIST entry boundaries over8192 full original native sanitizer states plus64 C64 states retaining original downstream priority and delay values. See `docs/DSKY-REQUEST-CONTROL.md`.

## Translate original decimal display factor and scale selection

Qualify actual DSPDCPUT factor selection across all fourteen routines both noun tables all rows normal mixed contexts and signed zero limits with8192 original native sanitizer whole images plus128 C64 states retaining selected scale body frontier. See `docs/DSKY-DECIMAL-SELECT.md`.

## Compose original uninterrupted operator request and display release

Qualify actual operator request RELDSP JOBWAKE and genuine scheduler entry over8192 original native sanitizer full images plus64 C64 states preserving source wake variants unused Executive fields and true downstream service boundaries. See `docs/DSKY-OPERATOR-REQUEST.md`.

## Compose original uninterrupted Comanche abort and restart lifecycle

Qualify BAILOUT POODOO CCSHOLE snapshot BORTENT flag phase cleanup WHIMPER ENEMA over1536 uninterrupted original paths with14848 complete native sanitizer states and12 C64 sequences retaining MMDSPLAY PTBAD DOFSTART as genuine selected program entries. See `docs/COMANCHE_ABORT_LIFECYCLE.md`.

## Translate original shared vector cross product and projection dependencies

Qualify native VXV and VPROJ with erasable aliases and immutable fixed operands over262144 complete original native sanitizer states and384 actual C64 states. Original source words and constants independently match baseline ropes; passive input injections and unexecuted reviewed overflow exit are explicit. Antenna and interpreter files remain partial. See `docs/NATIVE-SBAND-VECTORS.md`.

## Compose original IMUPULSE lead in reservation and source returns

Qualify original IMUPULSE lead in actual successful WAITLIST registration free command reservation TPAGREE exchanges and MODEEXIT SWRETURN against393216 complete original native sanitizer states plus192 C64 states. Root stream wrapper argument ordering was corrected before release and initial failure evidence retained; frozen service bytes are unchanged. Busy sleep and deferred gyro output tasks remain separate. See `docs/NATIVE-IMUPULSE.md`.

## Translate original shared scalar display output scaling bodies

Qualify source degree arc and fractional output scaling plus mission specific CDU landing radar and optics scales through true DSPDCEND with20480 complete original native sanitizer states and160 C64 states preserving source channel33 and fresh READLO effects. See `docs/DSKY-SCALAR-OUTPUT.md`.

## Translate original Comanche priority alarm source lead in

Qualify actual PRIOLARM raw registers and FAILREG recording through true PRIODSPR or POODOO selection against65536 complete original native sanitizer states and32 C64 states preserving actual incoming register patterns and explicit display recovery continuations. See `docs/COMANCHE_PRIOLARM.md`.
