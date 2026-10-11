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
| `make test` | PASS; all six host suites, including restart and numerical/P66 checks | `evidence/guidance-host.log`; earlier per-milestone logs retained |
| `ASAN_OPTIONS=detect_leaks=0 make sanitize` | PASS; all six host suites under AddressSanitizer and UndefinedBehaviorSanitizer | `evidence/guidance-sanitize.log` |
| `python3 tools/prepare_reference.py --virtualagc /path/to/pinned/virtualagc --fixture /path/to/fixture` | PASS; tools built, nine original source identities checked, fixture assembled | `evidence/guidance-fixture.log` |
| `make differential REFERENCE=/path/to/fixture` | PASS; all earlier snapshots unchanged plus 12,856 numerical/P66 checkpoints | `evidence/guidance-differential.log`, guidance snapshots/trace and retained earlier snapshots |
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
| `git diff --exit-code 911e5c0283c629c50cb97666f34065e8c07d71a5 -- ':!c64-port'` | PASS; no historical changes (run from repository root) | `evidence/guidance-source-preservation.txt` |

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
