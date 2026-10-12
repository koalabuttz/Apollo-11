# Translate Luminary P-AXIS_RCS_AUTOPILOT as transfer-ABI routines

Lane F. Milestone `lm-p-axis`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

## Scope

`Luminary099/P-AXIS_RCS_AUTOPILOT.agc`, all of it: bank 16 `02213`-`03640`,
790 assembled words. Nothing is left in the file.

`src/lm_p_axis.c` registers fifteen routines in `lm_p_axis_routines`. Each
runs from its entry until the original program leaves the file, and every
source instruction is one `agc_image_ops` call (register effects checked
against yaAGC by `tools/agc_image_ops_differential.py`), named with its
erasable address or the source's fixed constant. MSU, DV, DTCB and the
channel orders (READ, RAND, WAND, WOR, RXOR on L) are small helpers with
yaAGC's effect.

| Entry | Body |
|---|---|
| PAXIS `16,2213` | the T5 lead-in: TIME5, BANKRUPT and QRUPT; `TC BAILOUT` when DAPZRUPT shows the last pass still running; otherwise `TC CHEKBITS` (DAPIDLER_PROGRAM) |
| `16,2223` | CHEKBITS's return after the `OCT 02000` word, which runs as `TC CHEKBITS` when DAPZRUPT is +0: `TC CHEKBITS` again |
| `16,2224` | CHEKBITS's return: the CDU samples, the CDU desired update through 1STOTWOS, TCP and TCQR; PAXFILT's `TC ACDT+C12` when CALLGMBL is set |
| `16,2653` | ACDT+C12's return: the Kalman filter set up as an interrupted job (ARUPT, BRUPT = `TCF RATELOOP`, QRUPT, ZRUPT = SUPERJOB +1); `TCF NOQBRSM +1` |
| SUPERJOB `16,2666` | the RESUME of that job (Z = ZRUPT - 1): `TCF RATELOOP` (KALMAN_FILTER) |
| BACKP `16,2316` | from KALMAN_FILTER: rate derivation through SUBDIVDE and OVERSUB, the trap filters, the AOS terms, the trim gimbal timers, translation (CHKVISFZ, TRYUORV, ALTERYZ), the manual, detent and rate command modes, the P-axis RCS law (PURGENCY, PEGI) and jet selection (PJETSLEC, SELECTP), until the first call that leaves the file |
| `16,2773` `16,3154` `16,3403` `16,3424` `16,3443` `16,3500` `16,3512` `16,3550` | the returns from those calls (ALARM in ABORTYZ, IBNKCALL to ZATTEROR, `TC WRITEP`, IBNKCALL to JTLST, `TC WRITEP -1`, IBNKCALL to SPSRCS, IBNKCALL to TJETLAW, ALARM in SELECTP), continuing the same flow |
| CALCPERR `16,3446` | the P-axis attitude error; `TC Q` |

The calls that leave the file end a routine with Z at the destination and
Q (or the IBNKCALL operand) as the original leaves it: `TC BAILOUT`,
`TC CHEKBITS`, `TC ACDT+C12`, `TCF NOQBRSM +1`, `TCF RATELOOP`, `TC ALARM`,
`TC IBNKCALL`, `TC WRITEP`, `TC WRITEP -1`, QRAXIS's `DTCB` to CALLQERR
(Q_R-AXIS_RCS_AUTOPILOT) and CALCPERR's `TC Q`. 1STOTWOS, SUBDIVDE, OVERSUB,
ZEROENBL, CALCPERR, ALTSYST and SELECTP are entered by TC from inside the file
and are compiled as C calls that leave Q as the original does (3 after each
`TC Q`). The jumps written as `TC` (`TC +3`, `TC PEGI`, `TC PURGENCY +4`,
`TC CHKVISFZ`) set Q as well.

The original's index tricks are followed:

- INDXYZ entries of `07776` make SELECTP's `INDEX ROTINDEX; MASK JETSALL` a
  `TC JETSALL -1`, whose word is `TCF ABORTYZ +2` (Q = `16,3537`).
- OVERSUB's `INDEX A; CS BIT15 -1` limits an overflowed sum to +-POSMAX.
- The CCS -0 exits that land past an `EXTEND` run the next word as a basic
  order: CHKRTIMR's -0 exit runs `DIM PJETCTR`'s word as `ADS PJETCTR`.
- CCS OLDPMIN's negative exit enters FIREP at its RAND's `EXTEND`.

TS overflow skips and the negative and -0 exits of CCS land where the
original's do, even where no admitted input reaches them. INHINT and RELINT
set the interrupt-enable bit of the device record.

DV is yaAGC's, for every dividend. The helper `dv` joins A and L as yaAGC's
SpToDecent does, including its comparison of a sign-extended L with a 15-bit
-0. It takes yaAGC's sign rules for a zero dividend, an equal dividend and a
zero quotient (+0). A dividend at or above the divisor is left to `ai_dv`,
which follows yaAGC's register-level SimulateDV. The freeze checks `dv`
against yaAGC executing `EXTEND; DV K` on 2,000,000 random A, L and K,
including +-0.

Channels 12, 13 and 31 and the interrupt-enable bit are in the device record
`lm_flight_io` (`include/lm_flight_io.h`, which gains channel 13). Bits 12-14
written to channel 13 are the HANDRUPT trap enables, which do not read back,
so the record keeps `043777`, as yaAGC does.

The two `TC ALARM` codes (`OCT 02001` after ABORTYZ, `OCT 02003` in SELECTP)
are compiled site operands in `src/lm_p_axis_sites.inc`
(`lm_p_axis_sites`). The four IBNKCALL CADRs are in
`src/lm_interbank_sites.inc`. The file's data words that no code in it reads
are answered by `lm_p_axis_data` for passive fixed-data reads:

- KAOS, 1/10SEC, NORMSCL and PSKIPADR, which have no reference in Luminary099;
- -100MS, which KALMAN_FILTER reads.

Refused (`EXEC_OUT_OF_SCOPE`, image and io unchanged):

- an entry under an FB other than bank 16, or an EB other than EBANK6
  (`EBANK= PERROR`);
- a missing io record;
- a CALCPERR return in Q that is not a TC into fixed memory;
- a `16,3403` entry with ROTINDEX other than 0 or 1. It indexes MINTIMES, and
  the `TC WRITEP` it returns from follows PJETSLEC's `LXCH ROTINDEX` of 0 or 1;
- a BACKP-flow entry with OLDPMIN -0. CCS OLDPMIN would then run FIREP's
  `RAND CHAN31` word without its `EXTEND`, as `TC 02031`. The file only stores
  +0, 1 and OCT14 bits in OLDPMIN.

## Verification

`tests/lm_p_axis_cases.c` builds 16,384 cases, 1,024 for each of the sixteen
entry slots; BACKP has two slots. Each case starts from a random image with
FB bank 16, EB 6, random channels 12, 13 (its readable bits) and 31, and a
random interrupt-enable bit.

- In three quarters of the cases, every word above the counters has a random
  bit width, with +0 and -0 frequent.
- In one cohort in eight, three quarters of those words are +0 or -0.
- One BACKP cohort in 32 sets the rate command inputs that make PEGI's EDOTP
  +0 on the path through CHKVISFZ and RHCMOVED.
- A `16,3403` entry has ROTINDEX 0 or 1 in seven eighths of its cases.
- OLDPMIN is -0 only in one cohort in 32.
- Q holds a caller return for CALCPERR: SWRETURN, a fixed-fixed word, or a
  bank 16 word outside the file.

The trampoline runs each case with this file's routines until the original
program would leave the file: a TC, TCF, DTCB or `TC Q` whose destination is
outside `16,2213`-`3640`.

- `make lm-p-axis-differential` runs `tools/lm_p_axis_freeze.py`.
  - It builds the yaAGC oracle `tools/lm_p_axis_reference.c`, which runs every
    case in the unmodified rope until Z leaves the range at an instruction
    boundary. Z in the central registers is a `TC Q` on its way to the word in
    Q.
  - It requires the native snapshot to be byte-identical for every case the
    native routines accept: 16,235 cases. The comparison covers the outcome,
    channels 12 and 13, the interrupt-enable bit and all 2,048 erasable words.
  - It leaves out the 149 refused cases. Their digest is of the unchanged
    input image, which the host test checks.
  - The digests are frozen into `tests/lm_p_axis_vectors.h`, and every
    sixty-fourth into `tests/lm_p_axis_c64_vectors.h`.
  - It also builds `tools/lm_p_axis_dv_reference.c` and requires `dv` to
    equal yaAGC's `EXTEND; DV K` on 2,000,000 random A, L and K.
  - The proof (`evidence/lm-p-axis/lm-p-axis-proof.json`) counts where the
    cases leave the file: every exit above is taken.
- `make test-lm-p-axis` and `make sanitize-lm-p-axis` check every digest
  (strict and ASAN/UBSAN). They require every case to leave the file or be
  refused with the image unchanged, and require every entry and every
  reachable branch to be taken. They also check the refusals, the IBNKCALL
  CADRs against the compiled interbank table, and `lm_p_axis_data`.
- `make vice-lm-p-axis` runs every sixty-fourth case (256) on the C64 in
  VICE. Every thirty-second case did not fit VICE's 3,000,000,000-cycle
  limit.

Branches no admitted input reaches are documented in the host test:

- 1STOTWOS's `CCS A` -0 exit. A is an MSU difference, which is never -0.
- SUBDIVDE's second `CCS OMEGAU` +0 and -0 exits. They are reached only with
  |OMEGAU| above `0631`.
- The overflow skips of `TS DAPBOOLS` (ALTSYST) and `TS RCSFLAGS` (ALTERYZ,
  ABORTYZ +2). A is BIT1 or AORBSYST RXOR a sign-extended L, so its two sign
  bits stay equal.
- `CCS OLDPMIN`'s -0 exit, which is refused.
- DV with a dividend equal to the divisor:
  - DIVIDER divides |OMEGAU| of at most `0631` by DAPTREG4 = 1/40 (`0632`);
  - `DV NPTRAPS`, `DV NQTRAPS` and `DV NRTRAPS` divide a zero dividend;
  - `DV DAPTEMP1` and `DV DAPTEMP2` divide 5x, whose high word is at most 4
    and whose low word is zero only for x = +-0.

## Source map

Every assembled word has one native owner. This is checked by
`tools/lm_p_axis_source.py`, using the shared checker
`tools/lm_basic_source.py`; the map is
`evidence/lm-p-axis/lm-p-axis-source-map.json`.

After the C preprocessor, the owner and the helpers it calls hold:

- each erasable address (for an INDEX order, the indexed base);
- the rope word of each fixed operand, including the JETSALL, TYPEP, INDXYZ,
  MINTIMES and BIT15 -1 words read under INDEX;
- each branch target, or the inline composition named in the tool (the
  transfers among this file's own words);
- channels 1 (L), 12, 13 and 31;
- each constant, including the SUPERADR word and the QERRCALL 2CADR.

The ALARM codes and IBNKCALL CADRs are checked in their site tables.

| Address | Native owner |
|---|---|
| 16,2213-2222 | `paxis` |
| 16,2223 | `chekbits2` |
| 16,2224-2255, 16,2647-2652 | `afterchek_body` |
| 16,2256-2262 | `onetotwos` |
| 16,2263-2307, 16,2315 | `subdivde` |
| 16,2310-2314 | `oversub` |
| 16,2653-2665 | `afteracdt_body` |
| 16,2666 | `superjob` |
| 16,3205-3217, 16,3617 | `zeroenbl` |
| 16,3431-3440 | `altsyst` |
| 16,3446-3462 | `calcperr_body` |
| 16,3531-3546, 16,3551-3571 | `selectp` |
| 16,2623, 16,3132, 16,3614-3615, 16,3623 | `pa_data` |
| 16,2772, 16,3547 | `src/lm_p_axis_sites.inc` |
| 16,3153, 16,3423, 16,3477, 16,3511 | `src/lm_interbank_sites.inc` |
| the rest of 16,2316-3640 | `flow` |

## Evidence

Hashes: `evidence/lm-p-axis/SHA256.json`; all four files are kept in the
tree.
