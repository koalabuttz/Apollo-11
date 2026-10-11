# Verification record

## Provenance and scope

Apollo source base: `911e5c0283c629c50cb97666f34065e8c07d71a5`.
Virtual AGC tools and supporting Luminary transcription:
`1b5d6474841a7ab53237ead070f76f35f162f68d`.
Full version/asset/ROM hashes are in `tools/versions.json`.

The requested upstream MAIN has a missing include:
`LAMBERT_AIMPOINT_GUIDANCE.agc` (its file is named
`GENERAL_LAMBERT_AIMPOINT_GUIDANCE.agc`). Supplying that filename only in a
scratch copy leaves ten unrelated assembly errors, including 1/ACCJOB, 1/ACCS,
1/ACCSET, ACDT+C12, 1/ANETP, SPSRCS, TIMEGMBL and GTS references. The historical
source was not repaired in the repository.

Instead, `prepare_reference.py` uses the pinned Virtual AGC Luminary supporting
files and overlays **the requested upstream EXECUTIVE.agc, WAITLIST.agc and
INTERRUPT_LEAD_INS.agc, unchanged**.
It checks byte identity and selected symbol addresses. This is an executable
fixture for the original Executive, not evidence that the requested full Apollo
repository builds. A trial overlay of the requested ERASABLE_ASSIGNMENTS in the
newer support also produced three assembly errors; the successful fixture uses
Virtual AGC's definitions. The relevant Executive addresses and logical layouts
are explicitly checked/documented.

Executed fixture result: **0 unresolved symbols, 0 fatal errors, 1 warning**.
The warning is a supporting erasable-bank allocation warning, not a native-build
warning. Executive SHA256:
`c0d9b157f4e517f949001f25e9f77887d1c6497cff7718f3d5b80ab7c1823f8d`.
Full fixture rope SHA256:
`1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e`.

## Executed commands and results

Commands below are relative to `c64-port/`. During this run, tools were installed
in scratch siblings; the command transcripts preserve their actual absolute
paths. The portable commands are:

| Command | Actual result | Evidence |
|---|---|---|
| `make test` | PASS; all seven host suites, including pre-interpreter preparation | `evidence/prepare-host.log`; earlier per-milestone logs retained |
| `ASAN_OPTIONS=detect_leaks=0 make sanitize` | PASS; all seven host suites under AddressSanitizer and UndefinedBehaviorSanitizer | `evidence/prepare-sanitize.log` |
| `python3 tools/prepare_reference.py --virtualagc /path/to/pinned/virtualagc --fixture /path/to/fixture` | PASS; tools built, nine original source identities checked, fixture assembled | `evidence/prepare-fixture.log` |
| `make differential REFERENCE=/path/to/fixture` | PASS; all earlier snapshots unchanged plus 3396 preparation checkpoints | `evidence/prepare-differential.log`, preparation snapshots/trace and retained earlier snapshots |
| `python3 tools/freeze_vectors.py` | 26 admission/completion and 109 dispatch fingerprints generated | Header records snapshot SHA256 |
| `python3 tools/freeze_lifecycle.py` | 115 original-derived lifecycle fingerprints generated after exact equality | `tests/lifecycle_vectors.h` records snapshot SHA256 |
| `make c64 MOS_CC=/path/to/mos-c64-clang` | PASS; native PRG + ELF + link map | `evidence/c64-build.txt`, `executive.prg`, `executive.map` |
| `python3 tools/vice_test.py --vice /path/to/x64sc --rom-dir /path/to/C64 --data-dir /path/to/data` | PASS; native C64 execution, 135 state fingerprints, eight two-job sequences | `evidence/vice-result.txt`, `vice-monitor.log`, `vice.log`, RAM dumps |
| `make c64-irq MOS_CC=/path/to/mos-c64-clang` | PASS; dynamic-frame, restricted-ZP IRQ PRG and map | `evidence/c64-build.txt`, `irq.prg`, `irq.map` |
| `python3 tools/irq_vice_test.py --vice /path/to/x64sc --rom-dir /path/to/C64 --data-dir /path/to/data` | PASS; nine real raster IRQs, interrupted C, register/stack/decimal checks, eight reference checkpoints, overflow/masking and 74 timings | `evidence/irq-result.txt`, `irq-timing.json`, IRQ monitor/log/dumps |
| `make c64-lifecycle MOS_CC=/path/to/mos-c64-clang` | PASS; dynamic-frame, restricted-ZP lifecycle PRG and map | `evidence/c64-build.txt`, `lifecycle.prg`, `lifecycle.map` |
| `python3 tools/lifecycle_vice_test.py --vice /path/to/x64sc --rom-dir /path/to/C64 --data-dir /path/to/data` | PASS; 115 fingerprints, five native sequences, real IRQ-admitted waker, retained VAC payload, 76 guarded timings | `evidence/lifecycle-result.txt`, `lifecycle-timing.json`, lifecycle monitor/log/dumps |
| `python3 tools/freeze_waitlist.py` | 306 original-derived task fingerprints generated after exact equality | `tests/waitlist_vectors.h` |
| `make c64-waitlist MOS_CC=/path/to/mos-c64-clang` | PASS; dynamic-frame, restricted-ZP task/timer PRG | `evidence/waitlist-build.txt`, `waitlist.prg`, `waitlist.map` |
| `python3 tools/waitlist_vice_test.py --vice /path/to/x64sc --rom-dir /path/to/C64 --data-dir /path/to/data` | PASS; 306 fingerprints, ten native task sequences, six real CIA B IRQs, interrupted C, register/decimal/stack checks, task-admitted/woken jobs, 156 timings | `evidence/waitlist-result.txt`, `waitlist-timing.json`, monitor/log/dumps |
| `python3 tools/freeze_guidance.py` | 568 complete six-word expectations generated after exact equality | `tests/guidance_vectors.h` |
| `make c64-guidance MOS_CC=/path/to/mos-c64-clang` | PASS; native numerical/P66 diagnostic and map | `evidence/guidance-c64-build.log`, `guidance.prg`, `guidance.map` |
| `python3 tools/guidance_vice_test.py --vice /path/to/x64sc --rom-dir /path/to/C64 --data-dir /path/to/data` | PASS; 568 complete states, three rate histories, restoring I, six timings and stack/ZP checks | `evidence/guidance-vice-test.log`, guidance monitor/log/mailbox/stack dumps |
| `python3 tools/freeze_prepare.py` | 324 complete 39-word expectations and six-TS skip masks generated after exact equality | `tests/prepare_vectors.h` |
| `make c64-prepare MOS_CC=/path/to/mos-c64-clang` | PASS; native snapshot/vector diagnostic and map | `evidence/prepare.prg`, `prepare.map` |
| `python3 tools/prepare_vice_test.py --vice /path/to/x64sc --rom-dir /path/to/C64 --data-dir /path/to/data` | PASS; 324 complete states, three composed histories, all skip masks, restoring I and five timings | `evidence/prepare-vice-test.log`, preparation monitor/log/mailbox/stack dumps |
| `git diff --exit-code 911e5c0283c629c50cb97666f34065e8c07d71a5 -- ':!c64-port'` | PASS; no historical changes (run from repository root) | `evidence/prepare-source-preservation.txt` |

LeakSanitizer's default process inspection failed in this container (`/proc/...`
unavailable). Only leak scanning was disabled; ASan memory checks and UBSan
remained enabled. The native port allocates no heap memory. This restriction is
not reported as a passing default LeakSanitizer run.

## Original Executive executions

The host-only GPL adapter calls Virtual AGC's actual `agc_engine` on the assembled
rope. It loads fresh test state for every scenario, starts at the real NOVAC or
FINDVAC public entry, and places the requested inline 2CADR at Q in erasable
storage. For completion it enters bank-01 ENDJOB1. Peripheral inputs and trace
callbacks are disabled. AGC interrupts are inhibited, and hardware watchdog
alarm generation is suppressed in the emulator; **Executive bailout detection
is not suppressed**. These are controlled routine runs, not simulated missions.

Stop boundaries:

- Admission success: immediately before ENDFIND bank restoration/return.
- Resource failure: immediately before BAILOUT1 body; alarm code is read from the
  actual inline rope word referenced by Q.
- Completion with work: immediately before CHANJOB-2 begins dispatch.
- Completion without work: DUMMYJOB entry, before it writes NEWJOB=-0.

Each comparison checks 96 core words, 220 VAC words, seven saved Executive globals
(NEWJOB, NEWPRIO, two NEWLOC words, LOCCTR, FIXLOC, OVFIND), plus the native alarm
boundary annotation and return result. It does not compare A/L/Q, FBANK/BBANK,
EXECTEM1/2, BUF or I/O channels. The adaptation of those register/return
mechanisms is explicit. The oracle links **no native Executive or arithmetic
implementation**; shared scenario code only constructs inputs.

| Scenarios | Reference coverage |
|---|---|
| 0 | Idle NOVAC into core zero |
| 1–5 | First through fifth free VAC; use words reserved in order |
| 6 | All VACs reserved -> 1201 |
| 7–10 | Lower/equal/higher packed priority; maximum upper priority with VAC |
| 11 | Compare against a queued highest candidate, not just core zero |
| 12–13 | Eighth core succeeds, both NOVAC and FINDVAC |
| 14 | Dormant negative priority skipped, -0 slot allocated |
| 15 | Position six allocation |
| 16–17 | Completion to idle, NOVAC and VAC release |
| 18–19 | Highest waiting job; first wins exact packed tie |
| 20 | Highest job in eighth position selected |
| 21 | Dormant higher word skipped during completion scan |
| 22 | Free core zero with a queued candidate; SPECTEST/SETLOC boundary |
| 23 | Stale LOC/BANK and NEWLOC debris; uncleared MPAC/VAC work words retained as original |
| 24–25 | All eight cores occupied -> 1202; prior VAC reservation remains |

Input states and the small `scenario_expected` diagnostic assertions are manually
constructed. Reference output files are produced by executing assembly; they are
never inferred from the native implementation. Full host comparisons are exact
word comparisons. C64 checks use FNV-1a fingerprints of result plus saved state;
hash collisions are possible, so they are a compact regression check rather than
a substitute for host word-by-word comparison.

## Sequential basic dispatch verification

The new `dispatch_reference.c` adapter continues each two-job sequence in one
Virtual AGC instance. The shared dispatch fixtures supply inputs only; they are
compiled with `DISPATCH_INPUT_ONLY` and link neither the native Executive,
dispatcher nor native numerical implementation. It executes CHANG1, CHANJOB,
ENDPRCHG/DTCB, ENDJOB1/EJSCAN and bounded idle start paths. Native output for the
eight sequences is produced by **actual mapped C callbacks**, including admission
inside the low job, rather than replaying the event input script in native C.
The script supplies equivalent calls and payload writes to the original engine.

All 53 cases / 109 checkpoints match full saved state, result and selected 2CADR.
Dispatch snapshot SHA256:
`97906753f385b75ce8847696d18f15b5d995ad6dc1fea938f1b065cd6f8a15bb`.
The old 26-scenario snapshot SHA256 remains
`f2815915515cf792679f480d932b92d8a28296ba9907b2aa2606e613d03a23c0`.
The oracle checks actual Z, BBANK and SUPERBNK at every successful dispatch.
Idle stops at DUMMYJOB+2, after NEWJOB=-0 and before RELINT. Basic no-pending yields
execute a real CCS NEWJOB poll in a tiny erasable fixture caller. Idle entry
scratch A=2/L=0 is an explicit startup convention. A/L/Q and hardware return
mechanisms are adapted to C; their saved LOC/BANKSET/NEWLOC debris is compared.

The channel-7 output stub now retains Virtual AGC's SocketAPI.c channel-7 storage
semantics. This is necessary for original bank transitions; the first admission
fixture had no need to write that channel. Peripheral socket/trace callbacks
remain disabled. The prepared rope and historical Executive hashes are unchanged.
DISPATCH.md details every case group, ABI adaptation and excluded path.

C64 verification checks the 109 dispatch fingerprints at the same boundaries,
runs all eight native callback sequences, and checks restored MPAC/MODE/VAC data
inside resumed jobs. Three representative timed paths repeat validated fixtures
outside the full check loop. Four timing intervals exclude setup and hashing.
The helper discovers the entry from the actual BASIC SYS header: initialized
zero-page callback tables move it beyond the previous hard-coded $080d address.

Host-only fault tests cover malformed NEWJOB, interpreter selection before
resource release, invalid basic addresses/superbank bits, pending bailout,
duplicate mappings, unmapped bank/address pairs, invalid callback actions and
blocked callback repetition after a latched fault. Sanitizers exercise the full
native test suite. Reference comparison, fingerprints and internal payload checks
are complementary; none claims arbitrary-state or mission equivalence.

## Arithmetic validation

The host tests exhaust word encoding, zero classification, complement,
diminished absolute value and sign extension/correction. All 15-bit words are
paired with 16 boundary operands for word-sized addition using a separate
quotient/remainder oracle (524288 pairs). All 65536 accumulator patterns are
checked for correction and overflow sign analytically.

Separate native/reference binaries emit matching 1,245,184-byte streams containing
32768 sign extensions, 65536 overflow corrections, and 524288 sign-extended
accumulator additions. The reference uses Virtual AGC's `SignExtend`,
`OverflowCorrected`, and `AddSP16` helper API. Stream SHA256:
`a96b7101bbd6394b7eeb6a33bc4c3f9ba255cda419f448d3120a29e019401fe1`.
This validates helpers rather than every original AD/TS/CCS instruction sequence.
The register-addition operand space is not exhaustively paired.

## C64 execution evidence

The helper runs PAL x64sc 3.7.1 with unaccelerated C64 settings, standard ROMs,
and warp enabled only to run emulation faster on the host. Native target CPU
cycles remain normal PAL cycles. The monitor loads the PRG after BASIC is ready,
starts its SYS entry, and waits for a native store of $a5 at $c000. The 13-byte mailbox
contains zero failures, scenario count $1a, dispatch case count $35, checkpoint
count $6d and native sequence count $08. Byte 7 records three additional timed
path checks; byte 8 contains the final timing marker $23; byte 12 is the high
failure-count byte. Missing output, timeout, nonzero
failure count, or emulator failure fails the helper.

The screen prints `26 EXEC + 53 DISPATCH CASES PASS` and
`8 NATIVE TWO-JOB SEQUENCES PASS`. The latest ROM/PRG hashes and
link-map details accompany the evidence. No real C64 has been tested.

## Bounded IRQ delivery verification

The IRQ host suite uses explicit simulated producer entry and exclusion, not
host threads. It verifies full-state equality with ordinary admission over 600
starting-ticket cases, byte wrap and FIFO order, admission only at safe points,
idle startup, higher-priority exchange and restored job data. Overflow retains
four entries and saturates rejected count; failed delivery preserves exact
1201/1202 state, including the pre-1202 VAC reservation. Invalid/unmapped inputs,
pending bailout and callbacks that fault through a helper are covered. All
three host suites run under ASan/UBSan.

The separate target harness actually installs a finite VIC raster source through
the stock KERNAL vector, then removes it. It records nine events: one register /
decimal poison probe, two interrupts during live C arithmetic, five events for
overflow, and one delayed by masked foreground work. The source cannot mutate
Executive state; only guarded foreground calls admit requests. The checked
two-job run reuses eight original-derived saved-state fingerprints, while a
second run excludes hashing from measured latency. ISR volatile locals exercise
the isolated stack, and KERNAL jiffy advancement checks continued timer service.

The helper checks `a5000008094a20` at $c020: done, two zero failure bytes, eight
reference checkpoints, nine IRQs, 74 timing intervals and a 32-byte queue. All
179 marker events must appear in order; missing delivery hits the emulator
cycle limit or timeout. Linked extra ZP allocation fails the helper. JSON retains
every operation interval, IRQ interval, latency boundary and stack observation.
IRQ.md describes ownership/ABI constraints; PERFORMANCE.md reports the retained
measurements and the excluded portions of hardware entry/exit.

## Basic lifecycle verification

The host-only `lifecycle_reference.c` executes public JOBSLEEP, JOBWAKE and
PRIOCHNG in the unchanged assembled Executive. Shared fixtures compile in
input-only mode; no native core, dispatcher, arithmetic or callback is linked.
Each sequential case retains machine/scratch state. Signed wake CADRs are
sign-extended into A, and target recognition checks physical bank as well as PC.
Wake stops at ENDFIND; dispatch checks actual BBANK/SUPERBNK; idle stops after
NEWJOB=-0 before RELINT. LIFECYCLE.md describes all 81 cases / 115 checkpoints,
including the synthetic adjacent-priority and both-zero CADR boundaries.

Native and original lifecycle snapshots both have SHA256
`6e5c7afa4a31a2d7fde7b981d89f9ccecb1a6896866ef505432927ab3623c00b`.
The host lifecycle suite adds malformed-input preflight, pending bailout, native
action/staging/mapping faults, step re-entry rejection, nonfatal no-match wake
and initially enabled/masked restoring guards. Five sequences run actual mapped
callbacks and check restored MPAC/MODE/OVFIND/VAC payloads.

The third C64 diagnostic checks all 115 frozen fingerprints and these sequences.
A real raster IRQ then publishes a waker admission while a VAC-owning job sleeps.
Foreground code verifies unchanged core state before admission, wakes the sleeper,
lowers the waker priority, checks resumed payloads and completes both. This bridge
is an integration check; foreground case 80 independently compares the same
lifecycle path with original assembly. The helper checks mailbox
`a50000517305014c948802d204` at $c040 and all 157 marker events, rejects compiler
ZP allocations beyond the preserved 32 registers, and retains 76 primitive
intervals, four bridge intervals and stack dumps. Its final trace is $04d2
(decimal 1234). PERFORMANCE.md reports samples without a deadline claim.

## WAITLIST and timer verification

The independent host adapter executes public WAITLIST, the original fixed-fixed
T3 lead-in, T3RUPT2 and TASKOVER. It links input constructors and Virtual AGC only.
All 196 cases / 306 rows agree, including TIME3/LST1/LST2/RUPTAGN, exact retained
1203/1204 state and bank-qualified transfers. Ten sequences execute actual native
task callbacks, including rescheduling inside task B; order is A/B/D/C.
WAITLIST snapshot SHA256:
`bd71aad75050b3df0fb7753b9519599bda9ca2b22abcecdbd41984001841182b`.
All previous snapshots and the rope hash remain unchanged. New fixture source
SHA256 values are in versions.json; byte identity and listing labels are checked.

Reference counter increments use the real CounterPINC; the engine scaler is held
between bounded calls so target CPU time cannot introduce fixture ticks. Task
transfer stops at actual target PC/physical bank. The original lead-in's saved
BANKRUPT and return-path BBANK/SUPERBNK are independently checked. WAIT scratch,
shared Executive scratch aliases and hardware RESUME are outside this comparison.
Native alarm markers annotate entry to original alarm routines, not their bodies.

The host adds exhaustive positive timer overflow positions, restoring guards,
capacity/mapping/helper faults, clock publication saturation and the 128cs pending
phase limit. All four suites pass ASan/UBSan. The C64 helper checks mailbox
`a50000c432010a06963a000500`, all 331 markers and no extra compiler ZP allocation.
Six CIA B events include a register/decimal poison canary, four events during
live arithmetic and one event for task admission/wake of a sleeping job.
It checks unchanged state before collection, masked task callbacks, retained VAC
payload, final release, Timer A jiffy advancement and vector/Timer B restoration.
The Executive bridge is an integration check; task state has its own full oracle.

The PAL source uses 19705 cycles per two-centisecond sample. All three observed
periods differ by at most one cycle. CIA coalescing and foreground/IRQ mask budgets
remain explicit limits. No physical C64, continuous workload or flight deadline
has been tested. WAITLIST.md records every case group and ABI/timing adaptation;
PERFORMANCE.md gives the retained binary and timing measurements.

## Review limits and unavailable checks

The source review identified the inclusive CCS counter, complete packed-word
comparison, DXCH debris, and non-rollback of a VAC on 1202. Executable independent
reference tests exposed and corrected implementation mistakes before handoff.
There was no independent human/second-agent review in this session.

Unavailable: mission replay, complete landing computations, interpreter context
dispatch, general/multi-source or NMI interrupt integration, remaining alarm/restart routes (RESTART.md), exhaustive
Executive state-space comparison, hardware run, and end-to-end flight deadlines.
These remain explicitly incomplete rather than being replaced with passing mocks.

## Sixth milestone: bounded alarm and protected job recovery

Executed `make test`, `ASAN_OPTIONS=detect_leaks=0 make sanitize`, the full
`make differential REFERENCE=.../deps/fixture`, and `make c64-restart` using the
existing pinned LLVM-MOS toolchain. All five host/sanitizer suites pass. Leak
scanning is disabled because process inspection is unavailable in this executor;
address and undefined-behavior instrumentation remain enabled.

The independent `restart_reference.c` links only the real Virtual AGC engine and
input-only constructors. Five additional alarm/restart source files are overlaid
byte-identically; all eight selected source files assemble without errors and the
rope SHA256 is still `1f5326e038de5b741b2f27b01ec949dbd688cf1906994e997402587c8628f40e`.
`restart_differential.py` compares 190 cases / 226 checkpoints, status and all
448 modeled words (including two explicit native boundary markers). Protected
phase/name/priority records are read back from original memory. Native/reference
snapshot SHA256 is `81c884b026fbdd4ae8050cddc75f2d07a91a168ccf1d3cf324a8f60cc39631a8`.
The four earlier snapshot hashes are unchanged. `freeze_restart.py` permits
fingerprint generation only after exact independent agreement.

Five suites include a failed FINDVAC's retained 1202 reservation, successful
bounded reconstruction, six actual native job completions, retained alarm history,
stale-runtime fail-stop, initially enabled/masked restoring guards, unsupported
phase/POODOO/double-pending rejection without mutation, phase-corruption retention,
and five-VAC exhaustion during six protected FINDVAC admissions.

PAL VICE executes the fifth standalone PRG, checks all 226 frozen fingerprints,
completes six recovered native jobs and verifies restoring I. Its mailbox is
`a50000bee206823c2486038202`, with zero failures. The monitor retains five timed
samples and instrumented hardware/software stack dumps. The helper rejects extra
compiler ZP allocation or exhaustion of the instrumented stacks. The large full
state hash workload requires a one-billion-cycle harness limit; this is a test
budget, not an application runtime constraint. Performance is in PERFORMANCE.md.

RESTART.md gives exact original entry/stop boundaries. The sequential oracle sets
explicit register/bank inputs between stages and holds counter progress; it does
not execute uninterrupted ENEMA glue. The composed recovery is a native adaptation.
Original hardware GOJAM, fresh start, display/key/flag glue, phase updates, table,
Type B, timed/longcall, servicer and guidance routes remain unimplemented.

## Seventh milestone: real P66 rate adjustment and multiword arithmetic

All six host suites pass `make test` and ASan/UBSan with
`ASAN_OPTIONS=detect_leaks=0 make sanitize`. The full differential command passes
all earlier comparisons with unchanged snapshots, plus 12,835 numerical/rate
cases / 12,856 checkpoints. The fixture now overlays nine selected historical
files byte-identically, including LUNAR_LANDING_GUIDANCE_EQUATIONS, with the
same rope hash. The new source SHA256 is in tools/versions.json.

`guidance_reference.c` links only Virtual AGC and input constructors. It executes
actual MP/DAS opcodes for primitive tests and original RODCOMP's six-instruction
prefix for rate tests. Complete count/scale/desired-high/desired-low/raw-A/raw-L
output agrees; adjacent memory, BBANK and original INHINT are checked separately.
Scalers are held during these bounded calls. Three histories retain desired rate
for eight steps each. GUIDANCE.md lists the exact cases and valid input domain.
Snapshot SHA256 is
`919925e42e327b9a0d426444be5f96af636bdf69812723f946db07ce086aed55`.

`freeze_guidance.py` freezes 568 full six-word expectations only after exact
native/reference equality. The PAL VICE diagnostic checks these words directly,
all three histories, restoring I, no extra compiler ZP and stack sentinels.
Its mailbox is `a500003802030c0d25`, with zero errors. Six numerical/rate call
intervals and target image/memory observations are retained (PERFORMANCE.md).
No AGC engine or native opcode decoder enters the target binary. The entire
RODCOMP/P66 interpreter continuation, live switch input, complete guidance task
integration, physical hardware and deadline guarantees remain outside the test.

## Eighth milestone: snapshots and pre-interpreter RODCOMP preparation

All seven host and sanitizer suites pass. Full differential verification retains
every earlier snapshot and rope hash, and adds 1171 cases / 3396 checkpoints over
all 39 modeled words and six TS-skip observations. `prepare_reference.c` executes
unchanged original basic instructions with the real engine and input-only
constructors. It reads physical state and observes actual next-PC transitions,
checks BBANK/INHINT and six adjacent-memory guards, and holds asynchronous counter
progress. Source identity and the new data-address assertions pass.

Checkpoints are before the snapshot stage at 32,3303, before vector preparation
at 32,3315, and before TC INTPRET at 32,3353. All 64 skip masks occur in both sign
families, with word-edge inputs, three four-step sequential histories and 1024
full-prefix deterministic cases. Snapshot SHA256 is
`50488aed2af58a9297f6f33b8c201f32497f710dbe1555b95d15105ff287f3be`.
`freeze_prepare.py` requires exact native/reference equality before generating
324 full 39-word expectations and skip masks. No target state hashing is used.

Host/C64 checks include direct composed histories and preserved scratch/register
debris. PAL VICE checks all states, both final-CAF overflow polarities, enabled
and initially masked caller permission, compiler-ZP allocation and stack sentinels.
Mailbox is `a500004401034e2724`, with zero failures. Five timed calls and maps/
stack dumps are retained. PREPARATION.md specifies stable sensor/time inputs,
MODE alias and native scratch/return adaptations; PERFORMANCE.md gives samples.
Interpreter execution, complete guidance scheduling/restart, live sensor inputs,
physical hardware and flight deadlines remain future work.

## Ninth milestone: VXSC KPIP1

The fixture now overlays 11 unchanged files, adding INTERPRETER.agc and
CONTROLLED_CONSTANTS.agc. All earlier snapshots and the assembled rope hash
remain unchanged. `scale_reference.c` links input constructors and Virtual AGC
only. It executes direct VXSC calls and full RODCOMP/interpreter-entry calls,
checks the resolved constant bytes/bank/address, LOC/BANKSET, RELINT permission,
five memory guards and the stop at DANZIG before PDDL. All 41 modeled words match
3355 cases / 3364 checkpoints. Original VROUND observations include exact half
in both signs and six low-to-high carry events; no OVFIND event is claimed.
SCALING.md lists the case domains and native control-state exclusions.

| Command | Result | Evidence |
|---|---|---|
| `python3 tools/prepare_reference.py` | PASS; 11 source identities and unchanged rope | `scale-fixture.log` |
| `make test` | PASS; eight host suites | `scale-host-tests.log` |
| `ASAN_OPTIONS=detect_leaks=0 make sanitize` | PASS; ASan/UBSan, eight suites | `scale-sanitize.log` |
| `make differential REFERENCE=/path/to/fixture` | PASS; all prior snapshots plus 3364 complete scaling states | `scale-differential.log`, `scale-*.snapshots`, `scale-reference.trace` |
| `python3 tools/freeze_scale.py` | 292 complete expectations, gated on exact independent equality | `tests/scale_vectors.h` |
| `make c64-scale MOS_CC=/path/to/mos-c64-clang` | PASS; eighth standalone PRG | `scale-c64-build.log`, `scale.prg`, `scale.map` |
| `python3 tools/scale_vice_test.py --vice /path/to/x64sc --rom-dir /path/to/C64 --data-dir /path/to/data` | PASS; zero errors, 292 full states and three four-step histories, restoring I | `scale-vice-result.txt`, monitor/log/mailbox/stack dumps, `scale-timing.json` |

Snapshot SHA256 is
`11993e66747e7685e964c555e08e72c73d9782d2674a46d3ec0b2d4b00c36f2a`.
All 119 prior evidence files retain their hashes; historical files remain
unchanged. Stack extents are finite diagnostic observations, not maximum call
bounds. The largest timed sample is 26.122 ms under masking; this milestone
makes no live P66 scheduling or hardware feasibility claim.

## Tenth milestone: PDDL THISTPIP

The independent adapter executes unchanged PDDL bytes directly and through
original RODCOMP/interpreter dispatch, with two integrated boundaries: after
scaling and after PDDL before DSU. It reads all 86 modeled physical words,
including FIXLOC/PUSHLOC and the entire selected 43-word VAC. Operand/bank/LOC,
MODE, pointer advance, RELINT and preparation/VAC guards are checked. An explicit
coverage assertion requires all five VAC mappings at all 38 legal offsets.
2323 cases / 3496 checkpoints match, including a 12 by 12 timestamp word-edge
matrix, both skip-input sign families, random full VACs and retained histories.

| Command | Result | Evidence |
|---|---|---|
| `python3 tools/prepare_reference.py` | PASS; same 11 source identities and rope | `push-fixture.log` |
| `make test` | PASS; nine host suites, unchanged-on-scope-rejection checks | `push-host-tests.log` |
| `ASAN_OPTIONS=detect_leaks=0 make sanitize` | PASS; ASan/UBSan, nine suites | `push-sanitize.log` |
| `make differential REFERENCE=/path/to/fixture` | PASS; prior snapshots plus 3496 complete pushdown checkpoints | `push-differential.log`, `push-*.snapshots`, `push-reference.trace` |
| `python3 tools/freeze_push.py` | 200 full final expectations after independent equality | `tests/push_vectors.h` |
| `make c64-push MOS_CC=/path/to/mos-c64-clang` | PASS; ninth standalone PRG | `push-c64-build.log`, `push.prg`, `push.map` |
| `python3 tools/push_vice_test.py --vice /path/to/x64sc --rom-dir /path/to/C64 --data-dir /path/to/data` | PASS; 200 full final states and three four-push histories, restoring I and capacity rejection | `push-vice-result.txt`, monitor/log/mailbox/stack dumps, `push-timing.json` |

Snapshot SHA256:
`33fdd2b7febd6827521fc95b72f27732ab1653786a27b80fd020e567bf594f3c`.
Earlier 138 evidence hashes, all original files and the assembled rope remain
unchanged. PUSHDOWN.md defines the native scope check and omitted machine-control
state. Full-prefix timing is a finite 21.101 ms sample, not a live guidance deadline.

## Eleventh milestone: DSU PIPTIME

The independent adapter executes actual DSU/ENDVXV/OVERFLOW/SETOVF2 bytes,
with no native arithmetic linked into the oracle. 3491 cases yield 4664 exact
89-word/path checkpoints, including both signs of overflow masks 1/5/7, all
signed-zero corners and three retained full-prefix histories. It asserts the
E2 PIPTIME operand, final BBANK, MODE, ZERO register, nine adjacent memory guards
and VAC bounds. ELAPSED.md records the full domain and numerical contract.

| Command | Result | Evidence under `evidence/` |
|---|---|---|
| `python3 tools/prepare_reference.py` | PASS; 12 unchanged overlays and rope | `elapsed-fixture.log` |
| `make test` | PASS; ten host suites and explicit resolved-overflow tests | `elapsed-host-tests.log` |
| `ASAN_OPTIONS=detect_leaks=0 make sanitize` | PASS; ten ASan/UBSan suites | `elapsed-sanitize.log` |
| `make differential REFERENCE="$PWD/deps/fixture"` | PASS; earlier comparisons plus 4664 elapsed checkpoints | `elapsed-differential.log`, matched snapshots and trace |
| `python3 tools/freeze_elapsed.py` | PASS; 172 complete final states and paths after snapshot equality | `tests/elapsed_vectors.h` |
| `make c64-elapsed MOS_CC=/path/to/mos-c64-clang` | PASS; LLVM-MOS native PRG | `elapsed-c64-build.log`, `elapsed.prg`, `elapsed.map` |
| `python3 tools/elapsed_vice_test.py --vice /path/to/x64sc --rom-dir /path/to/C64 --data-dir /path/to/data` | PASS; 172 complete states, restoring I, ZP/stack, five intervals | `elapsed-vice-result.txt`, monitor/log/mailbox/stack dumps, timing JSON |
| Historical-source diff and evidence hashes | PASS; 157 earlier identities preserved, 176 total | `elapsed-source-preservation.txt`, `SHA256.json` |

New source pool hash is recorded in tools/versions.json. The unchanged repository
underscore-named constant pool is copied byte-for-byte to the pinned fixture's
hyphenated include path. The rope and every earlier snapshot remain unchanged.
After expanding the oracle's memory guards, the elapsed differential comparison
was rerun with the identical snapshot hash; no native code changed afterward.

## Twelfth milestone: STORE 30D

3681 cases / 5044 complete 89-word/prior-path checkpoints match genuine original
DANZIG/DOSTORE/STORJUMP/STORE/STARTSTO execution. The adapter links input-only
constructors and original engine objects, with no native store/arithmetic code.
It counts all 190 VAC/push positions, including 35 overlapping saves, and checks
nine memory guards, VAC bounds, banks, LOC, ADDRWD, MODE, Q and IRQ permission.
STORE.md records the domain, corners, random cases and retained histories.

| Command | Result | Evidence under `evidence/` |
|---|---|---|
| `python3 tools/prepare_reference.py` | PASS; 12 source identities and rope | `store-fixture.log` |
| `make differential REFERENCE="$PWD/deps/fixture"` | PASS; earlier comparisons and 5044 STORE checkpoints | `store-differential.log`, matched snapshots/trace |
| `make test` | PASS; eleven suites, complete states and native rejection | `store-host-tests.log` |
| `ASAN_OPTIONS=detect_leaks=0 make sanitize` | PASS; eleven ASan/UBSan suites | `store-sanitize.log` |
| `python3 tools/freeze_store.py` | PASS; 176 full final states and prior paths | `tests/store_vectors.h` |
| `make c64-store MOS_CC=/path/to/mos-c64-clang` | PASS; native LLVM-MOS PRG | `store-c64-build.log`, PRG/map |
| `python3 tools/store_vice_test.py --vice /path/to/x64sc --rom-dir /path/to/C64 --data-dir /path/to/data` | PASS; 176 states, restoring I, rejection, RAM/ZP/stack and five intervals | `store-vice-result.txt`, monitor/log/mailbox/stack dumps, timing JSON |
| Historical-source diff and manifest | PASS; 176 earlier identities retained, 195 total | `store-source-preservation.txt`, `SHA256.json` |

The initial larger expectation table overlapped diagnostic RAM. The final 176-row
table retains all integrated skip inputs/histories and uniform corners; the host
still checks all 5044 rows. The helper now rejects image/state overlap with
reserved RAM before VICE launch. Final native code and final target checks pass.

## Thirteenth milestone: DDV 4SEC(28)

6735 cases / 8098 complete 93-word/result checkpoints match unchanged DDV and
bank-zero divide assembly with the pinned engine's hardware DV model. A generated
host-only engine copy selects existing SimulateDV via a one-line patch; the pinned
fast-DV signed-zero discrepancy is independently audited, not translated into
native arithmetic. No native code is linked to either oracle. DIVIDE.md details
all boundaries, source identities, numerical domains and model qualifications.

| Command | Result | Evidence under `evidence/` |
|---|---|---|
| `python3 tools/prepare_reference.py` | PASS; 12 overlays, original constant declaration, same rope | `divide-fixture.log` |
| `make differential REFERENCE="$PWD/deps/fixture"` | PASS; earlier comparisons and DDV hardware-model checkpoints | `divide-differential.log`, matched snapshots/trace |
| `python3 tools/divide_differential.py deps/fixture` | PASS; final 11 guards/bank/Q assertions, 8098 rows, two-model audit | `divide-standalone.log` |
| `python3 tools/audit_divide_models.py` | PASS; 798 discrepancies only L/MPAC/+1, negative-zero reproducer | `divide-dv-model-audit.json`, audit log |
| `python3 tools/prepare_hardware_dv.py` | PASS; original engine hash, one-line generated branch change | `divide-hardware-dv.patch`, logged source hashes, tools/versions.json |
| `make test` | PASS; twelve suites; all unsupported modes reject unchanged | `divide-host-tests.log` |
| `ASAN_OPTIONS=detect_leaks=0 make sanitize` | PASS; twelve ASan/UBSan suites | `divide-sanitize.log` |
| `python3 tools/freeze_divide.py` | PASS; 156 full 93-word/result states | `tests/divide_vectors.h` |
| `make c64-divide MOS_CC=/path/to/mos-c64-clang` | PASS; native LLVM-MOS PRG | `divide-c64-build.log`, PRG/map |
| `python3 tools/divide_vice_test.py --vice /path/to/x64sc --rom-dir /path/to/C64 --data-dir /path/to/data` | PASS; all 156 states, I/ZP/RAM/stack and five intervals | `divide-vice-result.txt`, logs/monitor/mailbox/stack dumps, timing JSON |
| Source diff and evidence manifest | PASS; 195 earlier identities retained, 218 total | `divide-source-preservation.txt`, `SHA256.json` |

The fixed divisor's declaration is checked in unchanged original SERVICER and its
assembled words are asserted; the support fixture remains at its prior rope hash.
All earlier comparisons continue to use the original unmodified pinned engine.
No physical Apollo hardware/C64 or full trajectory/flight timing proof is claimed.

## PDVL GDT/2

All 5218 complete99-word checkpoints match original assembly; all13 host and
sanitizer suites pass. PAL VICE checks156 exact states with mailbox
a500009c0003c66324. See GRAVITY.md and evidence/gravity-* (snapshots gzip).
All218 earlier hashes retained; manifest239 entries.

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

## Combined host build after the basic and keyboard milestones

The isolated a727be4 checkout plus the recorded build/test patches passes the
complete `make -j2 all test` suite. The first attempts exposed portable-target
aliases, a checker-variable collision, absent build-time rope fixtures and three
omitted basic test/freezer sources. All failures and the successful rerun are
retained with exact patch hashes in `evidence/integration-a727-host-proof.json`.
The target recipes and production kernels retain their qualified behavior.

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

## GitHub milestone publication through native matrix qualification

The fork branch `koalabuttz/Apollo-11:port/c64-native-c` contains all 172 milestones through local snapshot `4bc6b7c957200541d23fcd527eb69d703a3b7225`. `evidence/github-milestone-publication-audit.json` verifies every local and published Git tree identity. Existing GitHub history is preserved; remote commit IDs can differ from local IDs because their parents differ. Large retained proof artifacts were transferred intact with standard authenticated Git after the connector request-size limit was reached. This is a publication audit, not complete-port acceptance.

## Translate actual Luminary and Comanche DSKY Clear key editing

Both original Clear paths now preserve negative-zero DIM semantics and perform CLPASS component backtracking and actual UPDATVB redisplay. 16,384 full source/native/sanitized states and 64 complete C64 checkpoints pass. Superseded diagnostic-build failure evidence is retained beside the successful target build. See `docs/DSKY-CLEAR-ENTRY.md`.

## Qualify exact passive original fixed word storage for both applications

Both original fixed-word datasets now have data-only native readers: 36 populated banks and four zero banks per mission. Strict and sanitizer audits exhaust all 65,536 reader arguments per link group. Stock C64 reads match all 81,920 individual words per mission across both IRQ states while preserving the canonical image and ROM mapping. The LM allocation-only disk retains every existing file; executable registry allocation and source caller controls remain separate work. See `docs/PASSIVE_FIXED_STORAGE.md`.

## Run original Comanche fixed memory display and checksum call prefixes

Actual DSPFMEM and COMADRS lead-ins now read the entire 40,960-address original fixed-word domain through native SUPDACAL and return to genuine callers. 81,920 source sequences and 163,840 complete source/native/sanitized checkpoints pass; stock C64 verifies 64 sequences and 128 checkpoints with full-state samples. Passive data is separately exact-word qualified. Later octal display/checksum bodies and whole application acceptance remain pending. See `docs/COMANCHE_FIXED_CALLERS.md`.

## Translate actual Luminary and Comanche Reset and Error key behavior

Both original RSET/ERROR paths now preserve their differing failure registers and restart-light effects, including the negative-zero CADRSTOR key-release corner. 8,192 complete source/native/sanitized states and 64 complete C64 checkpoints pass. See `docs/DSKY-RESET-ENTRY.md`.

The isolated committed release at `e4ccffc` passed every portable host build and test recipe; see `evidence/integration-release-host-proof.json`. The Comanche fixed caller source/native/sanitizer streams also passed an independent every-byte comparison of 673,054,720 bytes; see `evidence/cm-fixed-compat-integration-byte-comparison.json`. These audit results do not certify complete applications.

## Adopt source-selected canonical interpreter exchanges in the native scheduler

Native scheduler adoption now preserves the exact source-selected CHANG2 context without repeating exchange or admitting pending IRQs. All 16,384 complete source transitions and 64 real three-overlay C64 cases are qualified; historical scheduler and complete P66 regressions pass. Remaining application bodies and live scheduling paths retain separate completion gates. See `docs/NATIVE-CANONICAL-EXCHANGE.md`.

## Translate remaining original Comanche extended interbank control groups

Fourteen original interbank groups are qualified across 901,120 source-executed complete states and 16,384 explicitly rejected invalid typed BANKJUMP inputs, with full sanitizer equality and 56 actual C64 contexts. Ordinary caller return contracts and whole-file completion remain separate work. See `docs/COMANCHE_INTERBANK_EXTENDED.md`.

A clean isolated checkout exposed the missing original-Comanche assembly prerequisite in the fixed-memory caller differentials. `prepare-cm-original-rope` now independently assembles and compares the byte-original fixture before those recipes. The initial failed audit is retained in `evidence/cm-original-rope-clean-prerequisite.log`; full suite validation follows separately.

## Translate original Comanche dynamic display numerical caller and callee route

Original DYNDISP, P40CNV85 and S41.1 now compose CDU, matrix and rotation helpers through 27 native stages. All 13,824 complete uninterrupted host source states match strict and full sanitizer output; 68 frozen caller transitions pass on C64. Noun85 display formatting and live Executive scheduling remain separate integration work. See `docs/COMANCHE-DYNDISP-NUMERICS.md`.

## Translate original Luminary and Comanche invalid keyboard alarms

Actual CHARALRM and FALTON behavior, octal8/9 rejection and blocked negative-count editing match 8,192 complete original states under strict and full sanitizer comparison. All64 frozen complete states pass on C64. Wider Pinball continuation composition remains ongoing. See `docs/DSKY-INVALID-KEY.md`.

## Translate all original Luminary powered flight numerical and return groups

All 267 original emitted source words and six constants now map to seven reviewed native groups. Complete source/native/full sanitizer proofs cover 1,982,464 states; 38 actual C64 diagnostics qualify 1,216 complete states. Independent whole-file review includes exact composed FLESHPOT return contracts. The whole-file certificate and application runtime acceptance remain separate steps. See `docs/LUMINARY-POWERED-FLIGHT.md`.

## Complete original Luminary powered-flight subroutine source

Independent review reconstructed every one of 267 original emitted words, verified all seven complete source/sanitizer qualification dependencies and 38 actual C64 diagnostics, and exhausted the six-word constant accessor. The whole source file is certified within its published numerical and return contracts, including composed normalized FLESHPOT return inputs. The inventory has 30 complete files (24 native/data/declaration and six comment-only files), with 145 source files and both integrated applications still requiring complete acceptance. See `docs/LUMINARY-POWERED-FLIGHT.md`.

The authenticated GitHub branch reference was verified at `4878b9ada7fde3a56bc14ee4375195c93cd3da54`, including 185 exact milestone trees through the Luminary powered-flight certificate `3140fc1`. The immutable record is `evidence/github-milestone-publication-185-audit.json`. Native application completion remains governed by the coverage and integrated-application gates.

## Translate real Luminary and Comanche Key Release source continuations

Actual KEYREL, RELDSP and display job wake paths match8,192 complete original source states under strict and full sanitizer comparison. Both CADRSTOR signed zeros, source literal-DAS fallthrough and true END/PINBRNCH/BLNKSUB1 continuations are retained. All64 complete target states pass; later continuation bodies remain active work. See `docs/DSKY-KEY-RELEASE.md`.

The corrected isolated committed release at `3140fc1` passed all portable host build and test recipes, including both original fixed-memory caller differentials, all seven Luminary powered groups and canonical scheduler exchange. Exact log and exporter identities are retained in `evidence/integration-powered-release-host-proof.json`. This result does not certify complete resident application behavior.

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

## Inverse-trig corpus coverage correction

The original inverse-trig comparison remains valid, but its exhaustive high-word wording omitted2,045 values per function. The authoritative correction is `docs/INVERSE-TRIG-COVERAGE-ERRATUM.md`; a separate supplement is being qualified.

## Translate complete original Luminary and Comanche system test lead ins

Both containing source files are translated with every original entry and bank-return branch,35,328 complete original/source/full-sanitizer states and352 actual C64 checkpoints. Independent review corrected and separately tested the inherited IRQ state at INTPRET+2. External caller, display and recovery bodies remain separate. See `docs/SYSTEM_TEST_LEAD_INS.md`.

## Complete original system test lead-in sources

Independent original-source review, complete source/sanitizer comparisons and actual C64 execution certify both SYSTEM_TEST_STANDARD_LEAD_INS files within their original caller and external continuation contracts. The inventory has33 complete files (27 native/data/declaration and six comment-only files), with142 source files and both integrated applications still requiring complete acceptance. See `docs/SYSTEM_TEST_LEAD_INS.md`.

## Comanche scheduler and inter-bank source domain inventories

The original Executive/Waitlist inventory records589 assembled words and every source line while preserving explicit unfinished caller, body, recovery and IRQ domains. The separate inter-bank inventory records passive SUPDACAL address closure and its source/C64 evidence without granting instruction execution credit. Neither inventory certifies a complete containing file. See `COMANCHE_SCHEDULER_SOURCE_INVENTORY.md` and `COMANCHE_INTERBANK_DOMAINS.md`.

## Translate original Luminary and Comanche display branch selection

Original PINBRNCH and UPFLAG paths through ERASER, MARKPLAY, KEEPPRIO and PLAYJUM1 match16,384 complete original and full-sanitizer states, with64 actual C64 checkpoints. Each retains the source-selected GOPLAY continuation and original register and IRQ ownership. See `docs/DSKY-PINBRANCH.md`.

## Qualify exhaustive inverse trig high word inputs for both missions

A separate supplementary corpus independently inspects all32,768 high-word values per function in both missions, closing the2,045-value omission in the original corpus. All327,668 complete original/native/full-sanitizer states match; eight actual C64 diagnostics pass256 states and all36 constants. Original frozen algorithms and proof identities are unchanged. See `docs/INVERSE-TRIG-SWEEP.md`.

## Translate original Luminary and Comanche ordinary display blanking

Actual BLANKSUB caller setup, source selected buffer and unchanged BLNKSUB1 bank return match8,192 complete source/full-sanitizer states and64 actual C64 checkpoints. The native return preserves actual busy Q and available Q+1 selection without an extra Executive poll. See `docs/DSKY-BLANKSUB.md`.
