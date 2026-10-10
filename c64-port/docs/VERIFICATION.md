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
files and overlays **only the requested upstream EXECUTIVE.agc, unchanged**.
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
| `make test` | PASS; 32768 word encodings, 65536 register encodings, 524288 word-addition pairs, 26 Executive scenarios, 53 dispatch cases / 109 checkpoints, eight native two-job sequences | `evidence/host-test.txt` |
| `ASAN_OPTIONS=detect_leaks=0 make sanitize` | PASS; AddressSanitizer and UndefinedBehaviorSanitizer | `evidence/sanitize.txt` |
| `python3 tools/prepare_reference.py --virtualagc /path/to/pinned/virtualagc --fixture /path/to/fixture` | PASS; tools built, original Executive identity checked, fixture assembled | `evidence/prepare-reference.txt`, `evidence/assembly-summary.txt` |
| `make differential REFERENCE=/path/to/fixture` | PASS; 26 admission/completion and 109 dispatch full-state/target comparisons; numerical helper comparison | `evidence/differential.txt`, both `.snapshots`, `reference.trace` |
| `python3 tools/freeze_vectors.py` | 26 admission/completion and 109 dispatch fingerprints generated | Header records snapshot SHA256 |
| `make c64 MOS_CC=/path/to/mos-c64-clang` | PASS; native PRG + ELF + link map | `evidence/c64-build.txt`, `executive.prg`, `executive.map` |
| `python3 tools/vice_test.py --vice /path/to/x64sc --rom-dir /path/to/C64 --data-dir /path/to/data` | PASS; native C64 execution, 135 state fingerprints, eight two-job sequences | `evidence/vice-result.txt`, `vice-monitor.log`, `vice.log`, RAM dumps |
| `git diff --exit-code 911e5c0283c629c50cb97666f34065e8c07d71a5 -- ':!c64-port'` | PASS; no historical changes | `evidence/source-preservation.txt` |

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

## Review limits and unavailable checks

The source review identified the inclusive CCS counter, complete packed-word
comparison, DXCH debris, and non-rollback of a VAC on 1202. Executable independent
reference tests exposed and corrected implementation mistakes before handoff.
There was no independent human/second-agent review in this session.

Unavailable: mission replay, actual landing computations, interpreter context
dispatch, concurrent interrupt integration, alarm side effects/restarts, exhaustive
Executive state-space comparison, hardware run, and end-to-end flight deadlines.
These remain explicitly incomplete rather than being replaced with passing mocks.
