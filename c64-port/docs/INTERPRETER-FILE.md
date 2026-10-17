# INTERPRETER.agc as a whole file: the core plus the generated subroutines

Interpreter lane. Milestones `lm-interpreter` and `cm-interpreter`. Interface: `docs/NATIVE_TRANSFER_ABI.md`. Runtime: `docs/INTERPRETER-RUNTIME.md`.

INTERPRETER.agc (Luminary099 and Comanche055) runs on the port as two parts, per the steward's ruling ("core + generated"):

- **The compact core** (`src/lm_interp_core.c`; `src/cm_interp_core.c` includes it) runs the interpretive code itself from the rope image: the dispatcher (INTPRET, the order-pair decode, NEWOPS, DANZIG and the CHANG2 job switch), every interpretive order body and the store, branch and index orders. On the C64 it is the $0A01 job slot and its $0A02 $D000 extension. The flight build carries it (#254, #295, #306, #335, #350).
- **The generated subroutines** (`src/lm_interpreter.c`, `src/cm_interpreter.c`) are the file's basic subroutines that other files call from basic code: DMP, DMPSUB, SHORTMP, TPAGREE, DPAGREE, BRANCH, POLY, POWRSERS, MPACVBUF, ALSIGNAG, UNITDV, VECAGREE, ROUNDSUB and the rest of the cross-file entries `tools/cert_precheck.py` lists. `tools/interp_compile.py lm-interpreter cm-interpreter` compiles them with `tools/basic_compile.py` (spec in `tools/interp_specs.py`): LM 55 routines, CM 50.

The generated routines do not go into the core's slot, which has about 350 B left. They are ordinary control-transfer ABI routines for basic callers.

## What the spec keeps

`'inside'` names the label segments the generated code compiles (BRANCH through VXV, and every segment their flow reaches: DDV's normalization and divide, DSQSUB, VSQSUB, SQRTSUB and its paths, ENDDOT, ENDVXV, SETOVF, OVERFLOW, VROTATEX, COMP and DCOMP; the list is in the spec). Every other word stays with the core. A run that reaches a word outside those segments stops there at Z, as it does at any transfer out of a file. That is now only DANZIG and NEWOPS, plus SQRTABRT's alarm 1302 (POODOO1 in LM, POODOO in CM; `'noreturn'`). Until the internal exits were compiled (COMP, BUFPOS, DSQSUB, ENDVXV and the rest), the flight builds had no native entry for them and the LM P63 scenario stopped there. The source map gives each such word to the core (`"routine": "interpreter core"`).

Five additions to the compiler and the differential. None of them changes any other spec's output (`tools/interp_compile.py --check`):

- **`'inside'`** (interp_compile): the label segments to compile. Flow that runs on into a left-out segment stops natively at that word.
- **`'bb_writes'`** (basic_compile): INTRSM's `LXCH BBANK` (and CM's twin) may write BBANK, as the original does.
- **`'fixed_fixed_any_fb'`** (interp_compile): an entry in fixed-fixed memory (banks 2 and 3) runs under the FB and SUPERBNK its caller left. The generated basic_sane() then checks only coherence.
- **`'fixed_rope'`** (basic_compile): an INDEX read of fixed memory reads the rope image (`include/rope_image.h`) under FB and SUPERBNK. DMP reads the ADRES after its TC this way, and POLY its inline coefficients. The host test loads the yaYUL binary. The C64 diagnostic reads the REU image (`build/self-check.reu`, as `vice-self-check` does). `tools/ci_guard.py` lists the two generated sources and the two targets as rope readers.
- **`'q_callers'`** (differential): DMP's and POLY's cases return to the mission's real `TC DMP` and `TC POLY` sites outside the file (LM 21 and 7, CM 15 and 10), under the bank of each site. The words after the TC are then the real ADRES and coefficient words.

## The states each entry starts from

- **ADDRWD** (all entries): a VAC-relative operand (VAC+0, 2, 6, 14, 22) or unswitched erasable (0200, 0344, 0600, 01000, 01200, 01350). There is room for a vector past each one. This is what DMPSUB, TLOAD, VXV and the OVERFLOW returns index. MPAC and 01400 up are left out, because EBANK 0 puts 01400 over the central registers.
- **POWRSERS**: A is the coefficient table (01660, 01720, VAC+6 or VAC+20), and L is N-1 (0 to 5). These match the callers' `DCA MPAC +3` and `DCA MPAC +5`.
- **After TC SQRTSUB** (UNIT+16): MPTEMP is SQRTSUB's right-shift count, 0 to 28 (14 and up: SMALL, the input's first two words 0).
- **SN1+16**: where POLY returns past SN1's coefficients (Q+2N+4).
- **SQRTSUB** (an entry of its own, so its paths get states UNIT's overflowing dot rarely reaches): any MPAC, plus MPAC -1 with MPAC +1 -0 (SQRTNEG's last exit) and the largest argument, 37777 37777.
- **OVERFLOW**: A the overflow of the sum just stored (+1 or -1), L the component offset (0, 1, 3 or 5).
- **MAXTEST-1 to -14, MAXTEST-21**: the targets of DVNORM's `INDEX DVNORMCT; TC MAXTEST` and UNIT's `INDEX MPTEMP; TCF MAXTEST -1` (the DDOUBL chain, and -21D). The generated code reaches them through its dispatch, so each is an entry. MAXDVSW is 0 or -1, as DDV leaves it; MAXTEST-1 also gets equal majors, for MAXDV's +2.
- **BRANCH** returns to Q, Q+1 or Q+2 (`'call_skips'`).

## The subroutines flight calls most

The steward asked for the hot set. The counts below are static `TC` sites across the whole rope, both missions:

| Subroutine | LM sites | CM sites |
|---|---|---|
| DMP | 21 | 15 |
| TPAGREE | 19 | 15 |
| SHORTMP | 10 | 8 |
| BRANCH | 10 | 7 |
| POLY | 7 | 10 |
| DMPSUB | 7 | 4 |
| ALSIGNAG | 5 | 4 |
| MPACVBUF | 4 | 3 |
| ROUNDSUB, UNITDV | 3 each | 3 each |
| POWRSERS, VECAGREE | 2 each | 1 and 2 |

The steward flagged DMP, SHORTMP and TPAGREE as hot: flight calls them on every pass of its periodic jobs (static counts here, not a measured per-pass profile). INTPRET (238 LM sites, 242 CM) is the core's entry and is not a generated routine.

## Evidence

Hashes: `evidence/lm-interpreter/SHA256.json`, `evidence/cm-interpreter/SHA256.json`, `evidence/interpreter/SHA256.json`.

**Generated subroutines**, per mission:
- **Source map** (`<m>-interpreter-source-map.json`). Every word of the file maps to either a generated routine or the core:
  - LM: 2,083 words. 416 basic words compiled, 1,614 core, 53 constants.
  - CM: 2,080 words. 381 basic words compiled, 1,646 core, 53 constants.
- **Independent assembly comparison** (`<m>-interpreter-proof.json`, `make <m>-interpreter-differential`). The unmodified rope runs in yaAGC with 512 cases at each routine:
  - LM: 28,148 cases, all equal.
  - CM: 25,592 cases, all equal.
  - It compares Z, all 2,048 erasable words and the interrupt-enable state.
  - Of the cases, LM refuses 12 and CM refuses 8 (a GOTO to an operand word of the file, as #224).
- **Executed-instruction closure** (`<m>-interpreter-closure.json`, `make <m>-interpreter-closure`). Every compiled instruction runs in the oracle:
  - LM: 769 of 772. Two are DPAGREE's words (03,3255-3256). POLY counts POLYCNT down to +0 and returns there, so `CCS POLYCNT` never takes its negative exit into DPAGREE, and no other Luminary099 code enters DPAGREE. The third is SQRTSUB's overflow exit (00,3452 `CAF POSMAX`, below).
  - CM: 658 of 659. The one left is SQRTSUB's overflow exit (00,3451). `DAS MPAC` overflows only for a root of 1 or more, and SQRTSUB's largest argument (37777 37777, one of its states) gives a root below 1.
  - The UNIT resume point (after SQRTSUB) gets zero vectors in some of its cases, because UNITDV returns to UNIT only on a zero component. That way the exit through VROTATEX runs too.
- **Host and sanitizer results** (`<m>-interpreter-host.log`, `make test-<m>-interpreter sanitize-<m>-interpreter`). LM ran 1,777 frozen cases and CM 1,611, with 0 failures.
- **C64 execution results** (`<m>-interpreter-vice.log`, `make vice-<m>-interpreter`). Cases are picked by edge coverage, so every trace edge the host cases take is also taken on the C64. LM ran 105 checks in four groups and CM 101 in three, with 0 failures. The diagnostics read the rope from the REU.

**Core** (`evidence/interpreter/`):
- `interp-flight-vice.json` (#335): yaAGC's LM and CM P00 from Fresh Start, 240/240 arrivals at the interpreter through the flight resident in VICE.
- `interp-core-fuzz.log`: `make interp-core-fuzz`, the core's slot records against the verification interpreter. 100,000 cases, 0 differ.
- `vice-interp-core.log`: `make vice-interp-core`, the flight build's records on a PAL C64 in VICE, IRQs running. 181 checks per mission, 0 failures.

`make interp-core-differential` (the core against yaAGC on every compiled milestone's cases) currently shows 127 differences, all in `measurement-inc` (MEASUREMENT_INCORPORATION's milestone, not this file). That result is not cited here, and it has been reported to the steward.
