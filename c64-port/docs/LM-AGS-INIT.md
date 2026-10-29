# Translate Luminary AGS_INITIALIZATION as transfer-ABI routines

Lane F. Milestone `lm-ags-init`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

## Scope

`Luminary099/AGS_INITIALIZATION.agc` (R47, entered by V47), all of it: bank
32 `02005`-`02216`, 138 assembled words. Nothing is left in the file.

`src/lm_ags_init.c` registers fifteen basic and twenty-four interpretive
routines (`lm_ags_init_routines`, `lm_ags_init_interpretive`). Every
interpretive label is a key, and so is every word after a DANZIG, where the
Executive resumes a job that CHANG2 suspended. One step machine runs
from each key (Z, or LOC at NEWOPS for an interpretive key) through the
file's basic code, its interpretive code and the interpreter, until control
leaves the file:

- Basic code: each instruction is one `agc_image_ops` call under a case
  label at its address, in source order; a TC or TCF within the file is a
  goto, one to another file ends the run with Q and Z set.
- Interpretive code: each order-pair or store word is a case of its own,
  running the compiled `lm_interp_steps` / `lm_interp_orders` paths with the
  pair's words as literal arguments (SET, RTB, STCALL, CALL, STODL, STOVL,
  DSU, DDV, STORE, EXIT, VLOAD, MXV, VXSC, VSL2, VSL8, VAD, LXA, SXA, RVQ),
  as `src/lm_imu_performance.c` does. DANZIG with NEWJOB positive goes to
  CHANG2, as the original's does.

| Key | Body |
|---|---|
| AGSINIT `32,2005` | REFSMFLG: `TC ALARM` (OCT 220), or REDSPTEM and the AGSK display (`TC BANKCALL`, GOMARKF) |
| `32,2013` | ALARM's return: `TC ENDEXT` |
| `32,2025` `2026` `2027` | GOMARKF's terminate (`TC ENDEXT`), proceed (AGSVCALC: SET NODOFLAG, SET XDSPFLAG, EXIT, the EXDSPRET display) and enter (V32 takes the keystroke time into AGSK; V25 the keyed AGSK) |
| `32,2046` | EXDSPRET's return: `TC INTPRET`, `RTB LOADTIME` |
| `32,2051` `2053` `2055` `2061` `2063` | the interpretive returns from LOADTIME, LEMPREC, SCALEVEC, CSMPREC and SCALEVEC: `STCALL TDEC1 LEMPREC`, `CALL SCALEVEC`, the LM and CSM vectors to AGSBUFF, the AGS clock (`DSU DDV AGSK TSCALE`), EXIT, the AGS downlist and `TC BANKCALL` (DELAYJOB) |
| `32,2144` `2156` | VECSGNAG's returns inside SCALEVEC, to SCALEVEC's RVQ (back into the file at 32,2055 or 32,2063, or to a caller elsewhere) |
| `32,2077` | DELAYJOB's return: the old downlist, the IMUSE test, CKSTALL |
| `32,2114` | CKSTALL's DELAYJOB return: `TCF CKSTALL` |
| `32,2117` `2121` `2122` | IMUZERO's and IMUSTALL's returns, AGSEND (`TC DOWNFLAG`) |
| `32,2124` | DOWNFLAG's return: the V50N16 display (`TC BANKCALL`, GOMARK3) |
| `32,2127` `2130` `2131` | GOMARK3's returns: ENDEXT |

The runs end at: `TC ALARM`, `TC ENDEXT` and `TCF ENDEXT`, `TC BANKCALL`
(GOMARKF, EXDSPRET, DELAYJOB, IMUZERO, IMUSTALL, GOMARK3), `TC DOWNFLAG`,
the RTB targets LOADTIME and VECSGNAG, NEWOPS at LEMPREC or CSMPREC
(STCALL), SCALEVEC's RVQ to a QPRET outside the file, and CHANG2.

The CCS exits after a MASK that no admitted input reaches still land where
the original's do: AGSINIT's -0 exit runs `TC ENDEXT`, its negative exit
runs `OCT 220` as `TC 00220`.

`TC ALARM`'s OCT 220 and `TC DOWNFLAG`'s ADRES NODOFLAG are compiled site
operands (`src/lm_ags_init_sites.inc`, `lm_ags_init_sites`). The seven
BANKCALL CADRs are in the shared `src/lm_interbank_sites.inc`. The
interpretive fixed operands (TSCALE, RSCALE, VSCALE, AGSRND1, AGSRND2) are
the file's own data words in `B32DATA`, read when ADDRWD names them.

io is `lm_ags_io` (`include/lm_ags_init.h`): the superbank bits, the
interrupt-enable bit (SET's INHINT/RELINT and `TC INTPRET` set it) and the
undo image.

Refused (`EXEC_OUT_OF_SCOPE`, image and io restored from the undo image):

- a basic key under an FB other than bank 32;
- an interpretive key whose BANKSET or FB is not bank 32;
- a work area that is not one of the five VAC areas, PUSHLOC outside it, or
  EDOP not +0;
- NEWJOB negative at a DANZIG;
- SCALEVEC's RVQ with a QPRET outside switched fixed memory;
- a missing io record or undo image.

## Verification

`tests/lm_ags_init_cases.c` builds 29,952 cases, 768 for each of the 39
keys. Each case starts from a random image with FB bank 32.

- EB is E4 (`EBANK= AGSBUFF`, as V47's 2CADR sets it), another bank in a
  quarter of the states; the superbank bits and the interrupt-enable bit
  vary.
- NEWJOB is positive (a job waiting) in a fifth of the states.
- REFSMFLG and IMUSE are set or clear; MPAC holds BIT6 (V32) in half the
  enter returns; IMUCADR is of each CCS class.
- The work area is one of the five VAC areas with PUSHLOC inside it, MODE
  one of the three.
- At the SCALEVEC keys, QPRET is the CALL's (32,2055 or 32,2063), or a CADR
  elsewhere in a quarter.
- For `DSU DDV AGSK TSCALE`, a sixth of the states under E4 sample AGSK near
  TAT, as a V32 ENTER followed soon by LOADTIME leaves them: TAT - AGSK is
  0, or under 2^14 cs of either sign (a +-0 dividend high word). Others take
  an old AGSK, with TAT - AGSK at TSCALE's major part (equal majors) or
  exactly TSCALE (an exact division).
- At the VAD AGSRND1 and AGSRND2 keys (32,2137, 32,2151, 32,2153), a sixth
  of the states hold a component (MPAC, +3 or +5) whose sum carries into a
  high word of 037777.

- `make lm-ags-init-differential` runs `tools/lm_ags_init_freeze.py`.
  - It builds the yaAGC oracle `tools/lm_ags_init_reference.c` on yaAGC
    with the hardware divide (`build/agc_engine_hardware_dv.o`), which runs
    every case in the unmodified rope until Z leaves the file, the
    interpreter (fixed-fixed 06037-07707, bank 0 and bank 1's interpretive
    code, its erasable words and SWCALL) or, at NEWOPS, the file's
    interpretive words (`ags_leaves`).
  - It requires the native snapshot to be byte-identical for every case:
    all 29,952. The comparison covers all 2,048 erasable words and the
    interrupt-enable bit.
  - The digests are frozen into `tests/lm_ags_init_vectors.h`.
  - `tools/lane_f_c64_rows.py` chooses the C64 rows
    (`tests/lm_ags_init_c64_vectors.h`, 128 cases): every key, outcome,
    path and control-flow edge of every linked source (clang
    `-fsanitize-coverage=edge,trace-pc-guard`) the host cases take (46
    cases), then a stride of 97. The counts
    are in `evidence/lm-ags-init/lm-ags-init-c64-cover.json`.
  - The proof (`evidence/lm-ags-init/lm-ags-init-proof.json`) counts where
    each key's cases leave the file.
- `make test-lm-ags-init` and `make sanitize-lm-ags-init` check every
  digest (strict and ASAN/UBSAN). They require every case to leave the file,
  every key, instruction, order pair and decision outcome to be taken, the
  QPRET each `CALL SCALEVEC` stores to be the one the cases use, and the
  refusals.
- `make vice-lm-ags-init` runs the 128 C64 rows in VICE. The undo image
  lives at $F000, outside the program. The VICE log records the PRG's
  SHA-256.

Paths no admitted input reaches are documented in the host test:

- the negative and -0 exits of AGSINIT's and the IMUSE test's CCS A after
  MASK of a positive constant (REFSMBIT, IMUSEBIT), which is never
  negative;
- TS DNLSTCOD's overflow skip after CA AGSWORD, which never overflows;
- ROUNDSUB's overflow into OVFIND after `VXSC VSCALE` and `VXSC RSCALE`:
  each component is below 1 in magnitude and the scale factors are 0.641
  (VSCALE, 3.280839 E2 B-9) and 0.410 (RSCALE, 3.280839 B-3), so the
  rounded product's major word stays below 0.65 and never overflows;
- DDV's MAXDV and MAXTEST's +0 and -0 exits: MAXDVSW becomes +0 only when
  the dividend's and TSCALE's major words are equal, and TSCALE's minor
  word is +0 (100 B-10 is exactly 03100 B-14), so the minor test that
  follows always takes DVOVF;
- GENDDV's minor corrections `DIM MPAC` (a negative remainder term) and
  `INCR MPAC` (a remainder at least the divisor): TSCALE's minor word is +0,
  so the correction term is the division's remainder alone, which is never
  negative and always below the divisor. The exact division (remainder +0
  to FINALDV +3) is taken.

## Source map

`tools/lm_ags_init_source.py` reads the assembly listing and the rope and
checks, writing `evidence/lm-ags-init/lm-ags-init-source-map.json`:

- every interpretive case carries the file's words as its octal literals,
  in order, from its LOC;
- every basic `AT()` label is an instruction word of the file, and its code
  holds (after the file's #defines) the instruction's erasable address (the
  high word K of DCA and DXCH), its branch target, or the rope word of its
  fixed operand;
- every word of the file has an owner: an `AT` label, an interpretive case,
  the word after EXTEND, a compiled site operand (checked against the rope),
  a CADR in the INTER-BANK site table, `B32DATA` (checked against the rope),
  or a VN constant;
- the `K_*` constants equal the rope words at their symbols, and the `X_*`
  addresses equal their symbols.

## Evidence

Hashes: `evidence/lm-ags-init/SHA256.json`; all five files are kept in the
tree.
