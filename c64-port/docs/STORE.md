# P66 STORE 30D

`p66_store_elapsed` writes the elapsed DP pair from MPAC 0/+1 into the owned
VAC payload at decimal offsets 30/31. It translates the STORE at bank 32,3361
(LUNAR_LANDING_GUIDANCE_EQUATIONS:1006), stopping at DANZIG before the pending
DDV/PDVL opcode pair at 32,3362. Later division is not executed.

## State, ownership and scope

The caller owns distinct preparation and pushdown objects and excludes their
producers. MODE must be DP (+0); FIXLOC must be one of the five supported VAC
bases 0401,0455,0531,0605,0661. All memory words are ordinary 15-bit raw words,
including both zeros and mixed-sign high/low pairs. No conversion or arithmetic
normalization is applied. A/L are 16-bit sign-extended AGC registers.

The source DCA MPAC / DXCH places the former destination pair in A/L. CCS MODE
with +0 then clears A and returns through TC Q; L retains sign-extended old
VAC word 31. MPAC, MODE, FIXLOC, PUSHLOC, PIPTIME, BUF, MPTEMP, OVFIND and all
other modeled words stay unchanged. Only VAC words 30/31 and A/L change.
PUSHLOC is unused by STORE: even an out-of-range push pointer does not affect
this operation. No additional persistent state is needed; the complete test
context remains 89 words / 178 bytes.

The ordinary P66 sequence saves its vector at the start of the VAC, separate
from this elapsed-time pair. If a caller earlier pushed across offsets 30/31,
STORE overwrites those slots. The verification deliberately exercises these
overlaps rather than promising preservation of the overwritten words.

Unsupported MODE or FIXLOC returns `P66_STORE_OUT_OF_SCOPE` without mutation.
This native scope check is not a historical alarm or bounds check. Full
interpreter control state is outside the C API: LOC, ADDRWD, BANKSET, EDOP, Q,
NEWJOB and physical bank transfers are asserted by the oracle, not represented
as native resume state. The target uses a restoring I guard, not Apollo dispatch.

## Independent original-assembly verification

The host adapter links only input constructors and genuine pinned Virtual AGC
objects. No native numerical or store code is linked into that oracle. Direct
cases start at DANZIG with LOC=03360, EDOP=+0, NEWJOB=+0, BANKSET=064007 and
initial BBANK=064002. The dispatcher fetches the actual STORE word 00037 from
32,3361; CCS reduces it to relative operand 036 (decimal 30). DOSTORE (6362)
and STORJUMP (6372) dispatch STORE (6422), whose address processor adds FIXLOC.
STARTSTO (6434) executes the genuine DCA/DXCH and DP-mode return.

Integrated cases execute unchanged RODCOMP from 32,3275 through preparation,
scaling, PDDL and DSU. They capture all modeled words and DSU path observations
after DSU and again after STORE. At STORE entry Q=06373; the final TC Q leaves
Q=3. Final LOC=03361, ADDRWD=FIXLOC+30, BBANK=064007, MODE=+0 and interrupt
permission are independently asserted. Nine adjacent memory guards, ZERO
register 7, VACUSE and the word following the VAC remain intact. Hardware
counter progress is held and watchdog alarms suppressed for bounded oracle calls.

| Cases | Coverage |
|---|---|
| 0..31 | Raw signed-zero and arithmetic edge inputs |
| 32..159 | All 128 integrated preparation skip inputs in both sign families |
| 160..162 | Three four-step retained full-prefix histories |
| 163..352 | All five VACs at all 38 legal PDDL positions; 35 overlapping saves |
| 353..608 | All 256 four-word source/destination corners {+0,-0,+max,-max} |
| 609..2656 | 2048 direct deterministic random inputs |
| 2657..3680 | 1024 integrated deterministic random inputs |

3681 cases produce 5044 checkpoints: 2336 direct final states and 1354
integrated calls with both post-DSU and final STORE boundaries. Every row
compares all 89 words and the preceding DSU path. The oracle explicitly counts
all 190 VAC/push positions. Matched snapshot SHA256:
`77f7decebaefea2b67883a6c3d062e6fd41b0c5633ebf0e31c284c7ef6357340`.
All twelve source overlays, the rope and earlier snapshots remain unchanged.

All eleven host and ASan/UBSan suites pass (managed-environment leak scanning
disabled). Native rejection tests check all unsupported 15-bit bases and three
unsupported modes without mutation, and verify unused PUSHLOC behavior. Frozen
expectations select 176 complete 89-word/path final states: the first 163 cases
including histories, plus four uniform source/destination corners. The eleventh
standalone C64 PRG passes every expectation, all integrated skip inputs,
overlapping slots, rejection and enabled/already-masked restoring I checks.
Mailbox `a50000b00003b25924` reports zero failures.

## C64 measurements and continuation

The PRG is 46651 file bytes / 46649 loaded bytes at $0801..$be39, SYS $080d.
Text is 14792 bytes, rodata 31845, BSS 178 and noinit one byte. Expected words
use 31328 bytes and paths another 176. Image plus BSS/noinit is 46828 bytes,
ending at $beec. The expectation selection keeps image/state below $c000;
the VICE helper rejects overlap with reserved mailbox/stack RAM before launch.
It uses 32 imaginary ZP registers, no extra ZP, heap, floating point or opcode
decoder. Standalone diagnostic images are not a combined flight workload.

| Guarded sample | PAL cycles | At 985248 Hz |
|---|---:|---:|
| Direct STORE case 0 | 279 | 0.283 ms |
| Direct STORE case 1 | 317 | 0.322 ms |
| Direct STORE case 15 | 236 | 0.240 ms |
| Direct STORE case 18 | 314 | 0.319 ms |
| Complete RODCOMP through STORE, history step 0 | 21841 | 22.168 ms |

Setup/full comparisons are outside these intervals; marker/call and VIC bus
costs are included. Full-prefix masking exceeds one PAL frame in this diagnostic.
These finite samples do not establish WCET, producer-rate bounds or a flight
deadline. Hardware-stack extent is 44 bytes below entry, 58 including the ROM
frame; software-stack extent is 498 including full-state rejection fixtures.
Evidence is retained under `store-*`, alongside 176 unchanged earlier files.

```sh
make test
ASAN_OPTIONS=detect_leaks=0 make sanitize
python3 tools/prepare_reference.py
make differential REFERENCE="$PWD/deps/fixture"
python3 tools/freeze_store.py
make c64-store MOS_CC=/path/to/mos-c64-clang
python3 tools/store_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
```

`DDV 4SEC(28)` is now implemented and verified using the hardware-DV oracle in
DIVIDE.md, including the pinned fast-path discrepancy. This STORE API keeps its
original pre-DDV boundary. Pending PDVL GDT/2 is next.
