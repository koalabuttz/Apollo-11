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
