# P66 VXSC KPIP1 scaling milestone handoff

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
```

## Completed work

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
- All seven host suites under AddressSanitizer/UndefinedBehaviorSanitizer (leak scanning disabled).
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
and P66 preparation through its first VXSC KPIP1 operation. Complete flight OS and guidance workloads
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

Next: translate PDDL THISTPIP, preserving the six scaled words in an explicit
native pushdown representation and verifying timestamp/MODE/scratch effects.
Interpreter dispatch, flight deadlines and physical hardware verification remain open. See ROADMAP.md.

Acceptance for basic dispatch, IRQ delivery, lifecycle, bounded WAITLIST,
bounded Type C job recovery, pre-interpreter P66 preparation and VXSC KPIP1 is met.
No master/upstream merge is needed.
No second-agent/human review was performed. Native C is MIT/public-domain
translation; the separate GPL-2.0-or-later original-assembly adapters are host
verification tools and are never linked into the C64 executable.
