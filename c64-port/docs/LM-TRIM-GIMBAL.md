# Translate Luminary TRIM_GIMBAL_CONTROL_SYSTEM as transfer-ABI routines

Lane F. Milestone `lm-trim-gimbal`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

## Scope

`Luminary099/TRIM_GIMBAL_CONTROL_SYSTEM.agc`, all of it: bank 21
`03077`-`03645` and bank 16 `03735`-`03771`, 388 assembled words. Nothing is
left in the file.

`src/lm_trim_gimbal.c` registers four routines in `lm_trim_gimbal_routines`.
Each runs from its entry to the next transfer, and every source instruction is
one `agc_image_ops` call (register effects checked against yaAGC by
`tools/agc_image_ops_differential.py`), named with its erasable address or the
source's fixed constant. DTCB and the channel 12 orders (RAND, WAND, WRITE) are
small helpers with yaAGC's effect.

| Entry | Body |
|---|---|
| GTS `21,3077` | the trim gimbal control law for the R and then the Q axis: DEL, the NEGUSUM terms with the GTSQRT square root and its ROOTCYCL Newton steps, the new drive in NEGUQ/NEGUR, drive reversals on channel 12 and the ACDT+C12 request in RCSFLAGS; SR restored; DTCB to CLOSEOUT (Q_R-AXIS_RCS_AUTOPILOT) |
| ACDT+C12 `16,3736` | QACCDOT/RACCDOT and the channel 12 drive bits 9-12 from NEGUQ/NEGUR; `TC Q` |
| TIMEGMBL `21,3330` | the drive times that null AOSQ/AOSR (Q(R)GIMTIMR, NEGUQ/R, ALLOWGTS); `TC IBNKCALL` to ACDT+C12 |
| `21,3435` (IBNKCALL's return) | `TC RUPTREG2` to TIMEGMBL's caller |

GTSQRT and ROOTCYCL are entered only by TC from inside GTS and are compiled at
their call sites. They leave Q as the original does: 3 after each `TC Q`, and
FUNCTION +2 after `TC FUNCTION +1`, whose word is the `TC PRODUCT` that
SCALDONE's QXCH saved. TS overflow skips and the negative and -0 exits of CCS
land where the original's do, even where no admitted input reaches them. When
`TS SAVESR` overflows (SR at or above 2^13), the original skips `GTSGO+ON` and
runs the axis loop with QRCNTR = +1 or -1; the native code follows it,
indexing AOSQ, KQ, QDIFF, NEGUQ, QACCDOT and GMBLBITA by that value.

DV is yaAGC's. Its dividend conversion compares the sign-extended L with a
15-bit -0, so a positive A over L = -0 divides (A-1)·2^14 instead of A·2^14.
GTSQRT's ZEROHIGH reaches this whenever FUNCTION's high word is -0. The helper
`dv` reproduces it and leaves every other dividend to `ai_dv`.

Channel 12 is in the device record `lm_flight_io` (`include/lm_flight_io.h`).

Refused (`EXEC_OUT_OF_SCOPE`, image and io unchanged):

- an entry outside its bank, or under an EB other than EBANK6 (`EBANK= QDIFF`
  and `NEGUQ`);
- a missing io record;
- an ACDT+C12 return in Q, or a TIMEGMBL return in RUPTREG2, that is not a TC
  into fixed memory;
- a GTS pass where GTSQRT takes the zero root (ZEROOT) while ININDEX, which
  then still holds ACENTRAL, makes DOSHIFT's `INDEX SHFTFLAG; CA BIT15` select
  a word other than BIT15-BIT2. The original then executes `CA BIT15` plus
  ACENTRAL/2 as an instruction, which can be any order on any address. GTS
  saves every cell it can write on entry and restores it before refusing.

## Verification

`tests/lm_trim_gimbal_cases.c` builds 16,384 cases, 4,096 per entry. Each
starts from a random image with FB bank 21 (bank 16 for ACDT+C12), EB 6 and a
random channel 12.

- In fifteen sixteenths of the cases, the trim gimbal inputs have random bit
  widths: AOSQ/R, EDOTQ/R, KQ/R, QDIFF/RDIFF and ACCDOTQ/R. KQ/R and ACCDOTQ/R
  are positive except in one cohort, and NEGUQ/R is +1, +0, -1 or -0.
- SR is below 2^13 in three quarters of those cases; in the rest it is
  random, so `TS SAVESR` overflows.
- Q (ACDT+C12) and RUPTREG2 (21,3435) hold a caller return: SWRETURN, a
  fixed-fixed word, or a word of the entry's bank outside the file.

The trampoline runs each case with the native inter-bank routines until the
original program leaves the file:

- GTS's DTCB to CLOSEOUT;
- ACDT+C12's return to Q;
- TIMEGMBL's IBNKCALL arriving at ACDT+C12;
- 21,3435's `TC RUPTREG2`.

TIMEGMBL stops at ACDT+C12 because ACDT+C12's own `TC Q` back into IBNKCALL
leaves Q = 3, which the native ISWRETRN does not yet admit (lane A). ACDT+C12
is checked from its own entry.

- `make lm-trim-gimbal-differential` runs `tools/lm_trim_gimbal_freeze.py`.
  - It builds the yaAGC oracle `tools/lm_trim_gimbal_reference.c` and runs
    every case in the unmodified rope to the same stop.
  - It requires the native snapshot to be byte-identical in the outcome,
    channel 12 and all 2,048 erasable words for every case the native
    routines accept: 16,098 cases.
  - It leaves out the 286 GTS cases the native routine refuses. Their digest
    is of the unchanged input image, which the host test checks.
  - The digests are frozen into `tests/lm_trim_gimbal_vectors.h`, and every
    thirty-second into `tests/lm_trim_gimbal_c64_vectors.h`.
- `make test-lm-trim-gimbal` and `make sanitize-lm-trim-gimbal` check every
  digest (strict and ASAN/UBSAN). They require every case to reach its stop or
  be refused with the image unchanged, require every routine and every
  reachable branch to be taken, and check the refusals.
- `make vice-lm-trim-gimbal` runs every thirty-second case (512) on the C64
  in VICE.

Branches no admitted input reaches are documented in the host test:

- the -0 exits of `CCS QRCNTR` and the negative and -0 exits of
  `CCS QRNDXER`. QRCNTR is 2, 0, or +1 or -1 after the SAVESR overflow, and
  QRNDXER is 2 or 0.
- the -0 exits of `CCS DEL`. DEL is +1, +0 or -1.
- the negative and -0 exits of SCALDONE's `CCS A` on a MASK result.
- `CCS FUNCTION +1`'s +0 exit. Its `TCF +2` reaches `TS DEL` with A = +0
  exactly as the -0 exit does, and the -0 exit is taken. FUNCTION never ended
  (+-0, +0) in 3,000,000 random GTS passes.
- ROOTCYCL's `TS SCRATCH` overflow. The first Newton step stays below 1, and
  later steps only decrease.
- `dv`'s quirk branches with a dividend at or above the divisor. Every DV here
  divides less than its divisor, or has L = +0 or a positive L.

## Source map

Every assembled word has one native owner. This is checked by
`tools/lm_trim_gimbal_source.py`, using the shared checker
`tools/lm_basic_source.py`; the map is
`evidence/lm-trim-gimbal/lm-trim-gimbal-source-map.json`.

Line 382 of the file writes the `+2` location field after a tab, so yaYUL
reads it as an operation. `tools/lm_basic_source.py` now applies the same
indentation-only repair that `tools/fixed_data_generate.py` applies to the
data files, and records the repaired line in the map. With it, a fresh yaYUL
assembly reproduces the certified rope.

After the C preprocessor, the owner and the helpers it calls hold:

- each erasable address (for an INDEX order, the indexed base);
- the rope word of each fixed operand, including the LIMITS, BIT15 and
  GMBLBITA tables read under INDEX;
- each branch target, or the inline composition named in the tool;
- channel 12;
- each constant, including the CLOSEADR 2CADR.

The IBNKCALL CADR is checked in the compiled site table.

| Address | Native owner |
|---|---|
| 21,3077-3326, 21,3524-3635 | `gts` |
| 16,3735-3771 | `acdtc12` |
| 21,3327-3433, 21,3440 | `timegmbl` |
| 21,3434 | `src/lm_interbank_sites.inc` |
| 21,3435-3437 | `timeret` |
| 21,3441-3523 | `gtsqrt` |
| 21,3636-3645 | `rootcycl` |

## Evidence

Hashes: `evidence/lm-trim-gimbal/SHA256.json`; all four files are kept in the
tree.
