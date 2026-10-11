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
