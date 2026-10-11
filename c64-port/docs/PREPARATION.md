# RODCOMP snapshots and pre-interpreter vector preparation

The eighth milestone extends the P66 rate update through the basic instructions
in unchanged `LUNAR_LANDING_GUIDANCE_EQUATIONS.agc:955..999`. The full native
composition covers RODCOMP at physical bank 32,3275 through 32,3352, stopping
before `TC INTPRET` at 32,3353. Interpreter arithmetic and complete P66 execution
remain future work. The six-instruction rate API and its evidence are unchanged
(GUIDANCE.md).

## State and stage boundaries

`p66_preparation` holds 39 modeled words, occupying 78 target bytes including
the embedded 12-byte rate state. Memory words are valid 15-bit one's-complement
values, including both zeros. A/L are retained as raw 16-bit registers. MODE is
represented once as MPAC[7]; it is not an independent duplicate.

| Fields | Historical storage | Behavior |
|---|---|---|
| Embedded rate state | RODCOUNT, RODSCAL1, VDGVERT, A/L | Existing rate update; later stages overwrite register scratch |
| PIPA[3] | 0037..0041 | Stable sensor inputs, retained |
| Time pair | TIME2/TIME1 at 0024/0025 | Stable snapshot input, high word first |
| OLDPIPA[3] | E7,1763..1765 | Current PIPA values replace prior readings |
| RUPTREG[3] | 0070..0072 | Receives prior OLDPIPA readings |
| THISTPIP pair | E7,1761..1762 | Current time replaces prior snapshot |
| PIPATMP[3] | 1160..1162 | Added to current PIPA components, retained |
| TEM[3] | 1254..1256 | Used in delta preparation; final stores follow actual skip behavior |
| MPAC[8] | 0154..0163 | Vector highs at +0/+3/+5, lows at +1/+4/+6; +2 retained; +7 is MODE |
| DELVROD[6] | E7,1766..1773 | Only high words +0/+2/+4 replaced; low words retained |

`p66_snapshot` translates bank 32,3303..3314, stopping at 32,3315. The original
DCA/DXCH and CA/XCH sequence copies current PIPAs to OLDPIPA and prior OLDPIPA
to RUPTREG. Intermediate register debris from prior RUPTREG is overwritten by
DCA TIME2. DXCH THISTPIP then leaves the previous time snapshot in A/L. The
native stage folds these independent exchanges into direct state assignments
and preserves the observable A/L debris at the stage boundary. Inputs remain
unchanged; no counter reset or time increment is introduced.

`p66_vector_inputs` covers 32,3315..3352. It performs each AD in the raw 16-bit
one's-complement accumulator before TS correction. A normal-memory TS stores
corrected A. On overflow it replaces A with sign-extended +1/-1 and skips the
next instruction; without overflow it preserves the full accumulator.

There are six arithmetic TS instructions. Overflow on a velocity component skips
the following CA, including the transition to the first CS in delta preparation.
Overflow on a delta skips the next CS. Final delta overflow skips `CAF ZERO`,
so MPAC low words and TEM receive +1/-1 instead of +0. The native translation
retains that behavior even for boundary inputs outside a nominal flight workload.
The final CS ONE sets MODE and A to -1; L retains prior THISTPIP's low word.
MPAC +2 and DELVROD low words remain unchanged in every path.

The return value records the six arithmetic TS skips in bits 0..5. This is a
native boundary observation, not a new historical memory word, alarm, saturation
or rejection. `p66_prepare` composes the existing rate update, snapshot and
vector-input stages. It contains no interpreter, opcode decoder or rope image.

## Ownership and adaptations

The caller owns the modeled state and excludes count, PIPA, temporary and time
producers for the complete call. The original INHINT remains active at the
oracle's final boundary. Native helpers do not alter interrupt permission; target
callers use restoring guards. The diagnostic verifies enabled and initially
masked callers. Stable explicit time inputs replace asynchronous hardware
counter progress; no live sensor/time acquisition or atomic hardware-time claim
is made. Scratch is separate from the Executive; this API does not implicitly
copy MPAC into a saved job context or install a guidance callback.

PIPATMP and TEM remain raw source words. No SI conversion, sensor calibration,
arithmetic saturation, vector normalization or assumed uplink value is added.
The next original interpreter instruction scales MPAC with KPIP1. The ninth
milestone implements that bounded operation with integer arithmetic; see
SCALING.md. The preparation API and its pre-interpreter stop remain as documented.

## Verification and evidence

The independent host adapter executes unchanged assembled RODCOMP bytes. It
links input constructors and Virtual AGC only, with no native preparation,
multiword arithmetic or TS helper. It reads every modeled word from original
physical memory and records TS skips from the actual next-PC transitions.
It also checks six adjacent-memory guards, BBANK and INHINT at each boundary.
Counter progress is held during bounded calls. All nine historical fixture
sources remain byte-identical and the rope SHA256 is unchanged.

| Cases | Checkpoints and coverage |
|---|---|
| 0..63 | Snapshot and final preparation; all 64 skip masks with positive PIPAs |
| 64..127 | The same masks with negative PIPAs and mirrored overflow signs |
| 128..143 | Snapshot/word edges, signed zeros and stale scratch |
| 144..146 | Three four-step full-prefix histories, with explicit count/sensor/TEM/time updates |
| 147..1170 | 1024 deterministic full-prefix cases over all modeled inputs |

1171 cases produce 3396 checkpoints at 32,3303 / 3315 / 3353. All 39 words and
the six skip observations agree exactly. For cases 0..127 the observed mask is
also independently required to equal the input case modulo 64. Snapshot SHA256:
`50488aed2af58a9297f6f33b8c201f32497f710dbe1555b95d15105ff287f3be`.

`freeze_prepare.py` permits freezing only after exact native/reference equality.
It retains 324 complete 39-word expectations and skip masks, without hashing the
target state. Host and C64 checks compare them directly. Three composed histories
also call `p66_prepare` directly and compare final results with the same original
expectations. Host tests separately check a nominal numerical example, snapshot
debris, retained scratch/MODE alias and both signed final-CAF skip effects.

All seven host suites pass normal and ASan/UBSan builds (leak inspection disabled
for this executor). Every earlier differential snapshot remains unchanged.
The seventh C64 PRG passes all 324 states, composed histories, restoring I and
compiler-ZP/stack checks in PAL VICE. Mailbox is `a500004401034e2724`. Five timed
calls and stack dumps are retained; PERFORMANCE.md records their practical limits.
No physical C64, complete P66 workload, live sensor source or flight deadline has
been tested.

```sh
make test
ASAN_OPTIONS=detect_leaks=0 make sanitize
python3 tools/prepare_reference.py
make differential REFERENCE="$PWD/deps/fixture"
python3 tools/freeze_prepare.py
make c64-prepare MOS_CC=/path/to/mos-c64-clang
python3 tools/prepare_vice_test.py --vice /path/to/x64sc \
  --rom-dir /path/to/C64 --data-dir /path/to/VICE/data
```
