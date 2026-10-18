# Compile CONIC_SUBROUTINES into transfer routines (Luminary099)

Lane B (orbital math). Milestone `lm-conic-compiled`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/CONIC_SUBROUTINES.agc` (`src/lm_conic_compiled.c`, 1,072 words in banks 12 and 04) is native C in full, as 455 routines of the control-transfer ABI. The earlier milestones (`lm-conic-kepler`, `lm-conic-timethet`, `lm-conic-timerad`, `lm-conic-lambert`) ran each subroutine as native steps from one caller's entry. This one keys every routine by its physical address, so any caller reaches it through `agc_transfer_run`, and a job switched out at a DANZIG resumes where the Executive puts it.

## Scope

- **Entries.** KEPLERN, TIMETHET, LAMBERT, TIMERAD, APSIDES, DELTIME, ITERATOR, LAMENTER, PARAM, GEOM, GETX, INITV and TARGETV, each keyed by its address in `lm_conic_compiled_interpretive` or `lm_conic_compiled_basic`.
- **Resumes.** Every branch target, CALL/STCALL return, EXIT and TC INTPRET point is a routine, and so is the word after every DANZIG (`resume_keys`), where CHANG2 resumes the job.
- **CHECKCTR.** The basic ITERCTR countdown that ITERATOR and the Kepler loop reach through RTB runs as `lm_conic_checkctr` (`src/lm_conic_checkctr.c`). When ITERCTR is -16383, the TS overflows and skips TS MPAC, as on the AGC.
- **Leaves at.** NEWOPS with LOC outside the file (the callers' returns), CHANG2, the interpreter's POODOO1/ALARM1 exits, and the outside basic routines POLY and POODOO.

## States the cases start from

As in every compiled file, each case is a random 2,048-word image with the routine's legal entry state. This file also fixes:

- X1 -2 or -10D (MUTABLE,1: Earth or Moon, as SELECTMU's `AXC,1` and INTEGRATION_INITIALIZATION's `AXT,1` leave it, so the loads at :600, :1143 and :1364 read MUTABLE +2 to +7 or +10 to +15; review #165), X2 0 to 3 and S2 1 (GETX and INVRSEQN's AXT,2 3 / SSP S2 1 loop);
- ITERCTR (22D) 1 or 2 at BRNCHCTR (in flight it is anything from 1 to 20 there; the cases take the low end), so CHECKCTR's count runs out and KEPLOOP's iteration-limit exit (`BHIZ KEPCONVG`) is taken, as after 20 passes in flight (review #165: the flight-µ cases converge sooner);
- PUSHLOC 0D at GETX's entry, where its three callers CALL it. A PUSHLOC near the top of the VAC area pushes into the index registers and sends 1/WLOOP's `TIX,2` round thousands of times;
- the registers each routine is reached with on the file's own runs (`flow_states`): PUSHLOC, MODE, X1, X2, S1 and S2 at every routine an entry reaches;
- TAU. +0 at resumes from KEPLERN to PERIODCH, and a period of at least 10000 octal against a TAU. of a few inside PERIODCH's loop (with PUSHLOC 0D at PERIODCH and its GOTO, and 2D in between). A random TAU. over a random period loops for millions of passes.

The cases also draw flight edge values (review #165): a quarter of them take the fourth case shape of `tools/edge_cases.py` (±0 high words, full-scale words, zero vectors, equal times). KEPLERN's routines before PERIODCH do not (spec `edge_skip`): from an edge state vector the period can stay where PERIODCH never gets TAU. below it, and the original then loops forever (yaAGC times out; the native run stops at `max_steps`).

`max_steps` is 5000: a LAMBERT case runs up to 20 ITERATOR passes, each a few hundred pairs.

## Evidence

Hashes: `evidence/lm-conic-compiled/SHA256.json`. Bulk files are assets of the `evidence-lm-conic-compiled` release.

- **Source map.** `lm-conic-compiled-source-map.json` lists every word: 1,036 compiled routine words, 7 words of CHECKCTR (`lm_conic_checkctr`) and 29 constants that fixed operands read. None is untranslated.
- **Independent assembly comparison.** `lm-conic-compiled-proof.json` and `lm-conic-compiled-differential.log`, made by `make lm-conic-compiled-differential`. 64 states per routine (29,120 cases, a quarter with a job waiting at each DANZIG) equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. Every 128th case is frozen in `tests/lm_conic_compiled_vectors.h`.
- **Host and sanitizer results.** `make test-lm-conic-compiled sanitize-lm-conic-compiled` runs the 228 frozen cases (`lm-conic-compiled-host.log`).
- **C64 execution results.** `make vice-lm-conic-compiled` runs 25 diagnostics with 516 cases in VICE (`lm-conic-compiled-vice.log`). The image lives in the RAM under KERNAL at $e000, and the programs build without inlining, with a static stack and with 110 zero-page bytes.
- **C64 branch outcomes.** `lm-conic-compiled-c64-cover.json`: the C64 cases are the fewest that take every gcov branch arc the 29,120 host cases take over all 13 linked sources (`'c64_rows': 'cover'`, `tools/c64_row_cover.py`). All 3,088 host arcs are taken on the C64; none is host-only.
