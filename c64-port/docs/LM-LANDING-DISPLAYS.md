# Translate Luminary LANDING_ANALOG_DISPLAYS as transfer-ABI routines

Lane F. Milestone `lm-landing-displays`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

## Scope

`Luminary099/LANDING_ANALOG_DISPLAYS.agc`, all of it: bank 21 `02154`-`03076`,
467 assembled words. Nothing is left in the file.

`src/lm_landing_displays.c` registers four routines in
`lm_landing_displays_routines`. Each runs from its entry until the original
program leaves the file, and every source instruction is one `agc_image_ops`
call (register effects checked against yaAGC by
`tools/agc_image_ops_differential.py`), named with its erasable address or
the source's fixed constant. The channel orders (RAND, WOR, WAND) are small
helpers with yaAGC's effect.

| Entry | Body |
|---|---|
| LANDISP `21,2154` | the R10 waitlist task: TBASE2 and PIPCTR1, the SWANDBIT test (DISPRSET, ABORTON), IMODES33 bit 7 choosing ALTROUT or ALTOUT, DISINDAT (the mode switch in channel 30, DIDFLAG and the first-pass set-up to `TC TWIDDLE`) and SPEEDRUN's velocity update to `TC VARDELAY` |
| `21,2364` | TWIDDLE's return: `TCF TASKOVER` |
| INTLZE `21,2365` | TWIDDLE's task: channel 12 bit 2, IMODES33 bit 8; `TCF TASKOVER` |
| `21,2476` | VARDELAY's return: the R10FLAG test, VHY and VHZ, GET22/32 (M22 and M32 under EBANK6), LATVEL and FORVEL, the velocity monitor (VMONITOR, CHKLASTY, LASTOK, LVLIMITS and the limit paths to ZEROLSTY) to CDUTCMD/CDUSCMD and channel 14, then `TC LADQSAVE` |

`TC LADQSAVE` runs the word in LADQSAVE (which is PIPCTR1, EBANK7 `01714`).
DISINDAT's `QXCH LADQSAVE` stores the return of `TC DISINDAT` there: ALTROUT
+1 or ALTOUT +1, each a TC back into the file with Q = LADQSAVE +1. With
either word the routine continues in the file at ALTROUT +1 (ARCOMP, the
altitude rate to ALTM) or ALTOUT +1 (the altitude extrapolation through
OLDDATA, NEWDATA and ZERODATA to ALTM); with any other word it ends with
Z = LADQSAVE, where the original executes it.

The calls that leave the file end a routine with Z at the destination and Q
as the original leaves it: `TCF TASKOVER`, `TC TWIDDLE` (Q at its ADRES
INTLZE operand) and `TC VARDELAY`. The `TC DISINDAT` calls are compiled as C
gotos that leave Q as the original does. TS overflow skips and the negative
and -0 exits of CCS land where the original's do, even where no admitted
input reaches them: VMONITOR's CCS ITEMP5 -0 exit runs ZERODATA -1's
`WOR CHAN14` word as `TC 05014`, and ABORTON's TS FLAGWRD1 skip runs BITS8/7
as `TC 00300`.

Channels 12, 14 and 30 are in the device record `lm_flight_io`
(`include/lm_flight_io.h`).

The `ADRES INTLZE` operand of `TC TWIDDLE` is a compiled site operand in
`src/lm_landing_displays_sites.inc` (`lm_landing_displays_sites`).

Refused (`EXEC_OUT_OF_SCOPE`, image and io unchanged):

- an entry under an FB other than bank 21, or an EB other than EBANK7
  (`EBANK= UNIT/R/`);
- a missing io record.

## Verification

`tests/lm_landing_displays_cases.c` builds 16,384 cases, 2,048 for each of 8
entry slots: LANDISP and 21,2476 in three slots each, INTLZE and 21,2364 in
one. Each case starts from a random image with FB bank 21, EB 7, random
channels 12 and 30, and channel 14 without the CDU and gyro drive bits (yaAGC
drains those as counter commands).

- In three quarters of the cases, every word above the counters has a random
  bit width, with +0 and -0 frequent.
- The flag bits the file tests (FLAGWRD7 SWANDBIT, FLAGWRD0 R10FLBIT,
  FLAGWRD1 DIDFLBIT, IMODES33 bits 7 and 8, channel 30 bit 6) are set or
  clear one case in two.
- LADQSAVE holds ALTROUT +1 or ALTOUT +1 in three quarters of the cases.

The trampoline runs each case with this file's routines until the original
program would leave the file.

- `make lm-landing-displays-differential` runs
  `tools/lm_landing_displays_freeze.py`.
  - It builds the yaAGC oracle `tools/lm_landing_displays_reference.c` on
    yaAGC with the hardware divide (`build/agc_engine_hardware_dv.o`), which
    runs every case in the unmodified rope until Z leaves the file at an
    instruction boundary. Z in the central registers, or at LADQSAVE holding
    ALTROUT +1 or ALTOUT +1, is in the file.
  - It requires the native snapshot to be byte-identical for every case:
    all 16,384. The comparison covers the outcome, channels 12 and 14 and
    all 2,048 erasable words.
  - The digests are frozen into `tests/lm_landing_displays_vectors.h`.
  - `tools/lane_f_c64_rows.py` chooses the C64 rows
    (`tests/lm_landing_displays_c64_vectors.h`, 128 cases). It runs every
    host case with branch counting (`tools/lane_f_path_trace.c`) and takes,
    greedily, cases until every entry slot, outcome and branch the host
    cases take is taken (20 cases), then spreads the rest with a stride of
    97. The per-branch host and C64 counts are in
    `evidence/lm-landing-displays/lm-landing-displays-c64-cover.json`.
  - The proof (`evidence/lm-landing-displays/lm-landing-displays-proof.json`)
    counts where the cases leave the file: TASKOVER, TWIDDLE, VARDELAY and
    LADQSAVE holding another word. `TC 05014` and `TC 00300` are on branches
    no admitted input reaches.
- `make test-lm-landing-displays` and `make sanitize-lm-landing-displays`
  check every digest (strict and ASAN/UBSAN). They require every case to
  leave the file, every entry and every reachable branch to be taken, and
  check the refusals.
- `make vice-lm-landing-displays` runs the 128 C64 rows in VICE: every entry,
  every outcome and every branch the host cases take. The VICE log records
  the PRG's SHA-256.

Branches no admitted input reaches are documented in the host test:

- the negative and -0 exits of a CCS A after MASK of a positive constant
  (SWANDBIT, BIT7, R10FLBIT, BIT8) or RAND of BIT6, which is never negative;
- the overflow skips of TS IMODES33 and TS FLAGWRD1 after a MASK, whose
  result never overflows;
- TS ALTBITS after CCS ALTBITS of -0, which stores +0 and does not skip;
- CCS ALTSAVE +1 and CCS ALTSAVE negative and -0: OLDDATA keeps each word
  only after a positive overflow of POSMAX plus a non-negative carry, and
  ZERODATA stores +0, +0;
- CCS LATVEL -0: LATVEL and LATVEL +1 are DDOUBL of VELCONV (9,422, above
  half of 2^14) times a word, whose upper half is +-0 only for a factor of
  +-0, and MP by +-0 gives +0;
- CCS ITEMP5 negative and -0: VMONITOR stores 1, then 0.

## Source map

Every assembled word has one native owner. This is checked by
`tools/lm_landing_displays_source.py`, using the shared checker
`tools/lm_basic_source.py`; the map is
`evidence/lm-landing-displays/lm-landing-displays-source-map.json`.

After the C preprocessor, the owner and the helpers it calls hold:

- each erasable address (for an INDEX order, the indexed base; for DXCH and
  DCA, the high word K);
- the rope word of each fixed operand (ARCONV, ARTOA, ARTOA2, VELCONV,
  KPIP1(5), MAXVBITS and the fixed-fixed constants);
- each branch target, or the inline composition named in the tool (the
  transfers among this file's own words);
- channels 12, 14 and 30;
- each constant, including BITS8/7.

The ADRES INTLZE operand is checked in its site table.

| Address | Native owner |
|---|---|
| 21,2363 | `src/lm_landing_displays_sites.inc` |
| the rest of 21,2154-3076 | `flow` |

## Evidence

Hashes: `evidence/lm-landing-displays/SHA256.json`; all five files are kept in
the tree.
