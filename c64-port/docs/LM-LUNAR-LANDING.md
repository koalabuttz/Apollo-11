# Translate Luminary THE_LUNAR_LANDING as transfer-ABI routines

Lane F. Milestone `lm-lunar-landing`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

## Scope

`Luminary099/THE_LUNAR_LANDING.agc`, all of it: P63 in bank 32
`02776`-`03262` and P68's LANDJUNK in bank 31 `02144`-`02215`, 223
assembled words. Nothing is left in the file.

`src/lm_lunar_landing.c` registers twenty-one basic and ten interpretive
routines (`lm_lunar_landing_routines`, `lm_lunar_landing_interpretive`).
One step machine runs from each key (Z, or LOC at NEWOPS for an
interpretive key) through the file's basic code, its interpretive code and
the interpreter, until control leaves the file:

- Basic code: each instruction is one `agc_image_ops` call under a case
  label at its address, in source order; a TC or TCF within the file is a
  goto, one to another file ends the run with Q and Z set. WAND CHAN12 and
  RAND CHAN33 act on the io record's channels as yaAGC's do.
- Interpretive code: each order-pair or store word is a case of its own,
  running the compiled `lm_interp_steps` / `lm_interp_orders` paths with the
  pair's words as literal arguments (SET, CLEAR, SETPD, VLOAD, DLOAD, SLOAD,
  PDDL, PDVL, PUSH, SSP, STORE, STODL, STOVL, STCALL, CALL, RTB, GOTO, BMN,
  DAD, DSU, BDSU, DMPR, DDV, DSQ, ABS, ABVAL, UNIT, VXV, DOT, MXV, the short
  shifts and SR/SRR), as `src/lm_imu_performance.c` does. The four orders
  after `PDVL ABVAL` in the ignition iteration take their operands from the
  pushdown list (PUSHUP). DANZIG with NEWJOB positive goes to CHANG2, as
  the original's does.

| Key | Body |
|---|---|
| P63LM `32,2776`, `32,3000` | `TC PHASCHNG`; `TC BANKCALL` (R02BOTH) |
| `32,3002` | R02BOTH's return: WHICH, DVTHRUSH, DVCNTR, WCHPHASE, FLPASS0, `WAND CHAN12`, the FLAGORGY flags, IGNALG's set-up to `STCALL TPIP RP-TO-R` |
| `32,3040` `3044` `3051` | RP-TO-R's, GUIDINIT's and LEMPREC's returns: the landing site, the CG and DELTAH set-up, IGNALOOP to `STCALL R MUNGRAV` |
| `32,3100` | MUNGRAV's return: `STCALL GDT/2 ?GUIDSUB` |
| DDUMCALC `32,3102` | ?GUIDSUB's return (POSTJUMP): the DDUM iteration to `BMN CALL DDUMGOOD INTSTALL`; DDUMGOOD's TIG, OUTOFPLN and R60VSAVE to IGNALGRT's `TC PHASCHNG` |
| `32,3146` `3161` | INTSTALL's and INTEGRVS's returns, `GOTO IGNALOOP` |
| `32,3204` `3207` | PHASCHNG's return (`TC BANKCALL` STCLOK2) and STCLOK2's (`TCF ENDOFJOB`) |
| ASTNRET `32,3210` | the job ASTNRETN starts: `SSP RTB QMAJ (FCADR P63SPOT2) R51P63` |
| P63SPOT2 `32,3215` | R51P63's `GOTO QMAJ`: POINTVSM, SCAXIS, `TS EBANK`, `TC IBNKCALL` (PFLITEDB) |
| `32,3230` `3233` | PFLITEDB's and R60LEM's returns |
| P63SPOT3 `32,3235` | PHASCHNG's return: the LR antenna test (`RAND CHAN33`), GOPERF1 or SETPOS1 |
| `32,3245` `3246` `3247` `3251` | GOPERF1's three returns and SETPOS1's (`TC POSTJUMP` BURNBABY) |
| LANDJUNK `31,2144` | P68: `TC PHASCHNG` |
| `31,2146` `2151` `2153` | its, ZATTEROR's and SETMAXDB's returns, to `SET CALL LUNAFLAG LAT-LONG` |
| `31,2167` `2176` | LAT-LONG's and R-TO-RP's returns: RLS, the V06N43 display |
| `31,2203` `2204` `2205` | GOFLASH's returns |
| `31,2213` | REFMF's return: EXIT, `TCF GOTOPOOH` |

The runs end at `TC PHASCHNG`, `TC BANKCALL` (R02BOTH, STCLOK2, PFLITEDB's
R60LEM, GOPERF1, SETPOS1, ZATTEROR, SETMAXDB, GOFLASH), `TC IBNKCALL`
(PFLITEDB), `TC POSTJUMP` (BURNBABY), `TCF ENDOFJOB`, `TCF GOTOPOOH`, the
RTB target R51P63, NEWOPS outside the file (the CALL and STCALL targets
RP-TO-R, GUIDINIT, LEMPREC, MUNGRAV, ?GUIDSUB, INTSTALL, INTEGRVS,
LAT-LONG, R-TO-RP, REFMF), POODOO1 at a UNIT or ABVAL alarm, and CHANG2.

The four `TC PHASCHNG` OCT words are in the shared
`src/lm_phase_sites.inc`, the BANKCALL, IBNKCALL and POSTJUMP CADRs in
`src/lm_interbank_sites.inc`. The interpretive fixed operands are the
file's own data (99999CON, GUIDDURN, DDUMCRIT; `B32DATA`, with P63ADRES and
CODE500) and the interpretive constants UNITZ, UNITY, UNITX, ZEROVECS and
HI6ZEROS (`B23DATA`), read when ADDRWD names them; all are checked against
the rope.

io is `lm_lld_io` (`include/lm_lunar_landing.h`): the superbank bits, the
interrupt-enable bit (INHINT, RELINT, the switch orders and `TC INTPRET` set
it), channels 12 and 33, and the undo image.

Refused (`EXEC_OUT_OF_SCOPE`, image and io restored from the undo image):

- a basic key under an FB other than its bank;
- an interpretive key whose BANKSET or FB is not its bank;
- a work area that is not one of the five VAC areas, PUSHLOC outside it, or
  EDOP not +0;
- NEWJOB negative at a DANZIG;
- DDUMCALC with overflow in A (its `TS NIGNLOOP` would skip `TC INTPRET`
  into the order words; POSTJUMP never leaves one);
- a fixed operand outside the data above;
- a missing io record or undo image.

## Verification

`tests/lm_lunar_landing_cases.c` builds 15,872 cases, 512 for each of the
31 keys. Each case starts from a random image with FB the key's bank.

- EB is E7 (`EBANK= E2DPS`), another bank in a quarter of the states; the
  superbank bits, the interrupt-enable bit and channel 12 vary; channel 33
  BIT6 (the LR antenna) is set or clear.
- NEWJOB is positive (a job waiting) in a fifth of the states.
- The work area is one of the five VAC areas with PUSHLOC low in it, MODE
  one of the three.
- At DDUMCALC, three sevenths of the states have the ignition guesses
  converge (the gains, RGU, RIGNX/Z and VGU's X and Y zero, so DDUM is +0
  and BMN takes DDUMGOOD), with V zero in a third of those; at P63SPOT2,
  R60VSAVE is zero in a quarter (UNIT of a zero vector).

- `make lm-lunar-landing-differential` runs `tools/lm_lunar_landing_freeze.py`.
  - It builds the yaAGC oracle `tools/lm_lunar_landing_reference.c` on
    yaAGC with the hardware divide (`build/agc_engine_hardware_dv.o`), which
    runs every case in the unmodified rope until Z leaves the file, the
    interpreter (fixed-fixed 06037-07707, bank 0 and bank 1's interpretive
    code, its erasable words including UNIT's `TC A`, and SWCALL) or, at
    NEWOPS, the file's interpretive words (`lld_leaves`).
  - It requires the native snapshot to be byte-identical for every case:
    all 15,872. The comparison covers all 2,048 erasable words, channels 12
    and 33 and the interrupt-enable bit.
  - The digests are frozen into `tests/lm_lunar_landing_vectors.h`.
  - `tools/lane_f_c64_rows.py` chooses the C64 rows
    (`tests/lm_lunar_landing_c64_vectors.h`, 120 cases): every key, outcome,
    path and basic block of `src/lm_lunar_landing.c` the host cases take
    (33 cases), then a stride of 97. The counts
    are in `evidence/lm-lunar-landing/lm-lunar-landing-c64-cover.json`.
  - The proof (`evidence/lm-lunar-landing/lm-lunar-landing-proof.json`)
    counts where each key's cases leave the file.
- `make test-lm-lunar-landing` and `make sanitize-lm-lunar-landing` check
  every digest (strict and ASAN/UBSAN). They require every case to leave the
  file, every key, instruction, order pair and decision outcome to be taken,
  and the refusals.
- `make vice-lm-lunar-landing` runs the C64 rows in VICE as two PRGs that
  together run all 120 rows: the whole file does not fit C64 RAM, so group
  2 (`LLD_C64_GROUP=2`) registers DDUMCALC and P63SPOT2 with the code they
  reach (9 rows) and group 1 the other 29 keys (111 rows). Both are built
  without inlining, with the image and the undo image in the RAM under
  KERNAL ($E000, $F000; ROM banked out); the build fails if code or BSS
  reaches the mailbox page. The VICE log records each PRG's SHA-256.

Paths no admitted input reaches are documented in the host test: UNIT's
and ABVAL's alarm 1302 (SQRTSUB's abort on a negative argument). Both take
the square root of VSQ's sum of squares, which the sampled vectors, zero
vectors included, never make negative (inferred from the samples; no
input was found that reaches it).

## Source map

`tools/lm_lunar_landing_source.py` reads the assembly listing and the rope
and checks, writing `evidence/lm-lunar-landing/lm-lunar-landing-source-map.json`:

- every interpretive case carries the file's words as its octal literals,
  in order, from its LOC (an order pair named twice in a row is the PUSHUP
  word of two pushdown operands);
- every basic `AT()` label is an instruction word of the file, and its code
  holds (after the file's #defines) the instruction's erasable address, its
  branch target, the channel it names, the interrupt state INHINT or RELINT
  leaves, or the rope word of its fixed operand;
- every word of the file has an owner: an `AT` label, an interpretive case,
  the word after EXTEND, a site operand in the shared PHASCHNG or
  INTER-BANK table (checked against the rope), `B32DATA` (checked against
  the rope), or a VN constant;
- `B23DATA` equals the rope at UNITZ-HI6ZEROS, the `K_*` constants equal
  the rope words at their symbols, and the `X_*` addresses equal their
  symbols.

## Evidence

Hashes: `evidence/lm-lunar-landing/SHA256.json`; all five files are kept in
the tree.
