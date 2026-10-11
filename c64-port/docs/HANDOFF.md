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
