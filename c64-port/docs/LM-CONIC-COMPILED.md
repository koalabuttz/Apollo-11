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

- X1 0 or -8 (MUTABLE,1: Earth or Moon), X2 0 to 3 and S2 1 (GETX and INVRSEQN's AXT,2 3 / SSP S2 1 loop);
- the registers each routine is reached with on the file's own runs (`flow_states`): PUSHLOC, MODE, X1, X2, S1 and S2 at every routine an entry reaches;
- TAU. +0 at resumes from KEPLERN to PERIODCH, and a period of at least 10000 octal against a TAU. of a few inside PERIODCH's loop (with PUSHLOC 0D at PERIODCH and its GOTO, and 2D in between). A random TAU. over a random period loops for millions of passes.

`max_steps` is 5000: a LAMBERT case runs up to 20 ITERATOR passes, each a few hundred pairs.

## Evidence

Hashes: `evidence/lm-conic-compiled/SHA256.json`. Bulk files are assets of the `evidence-lm-conic-compiled` release.

- **Source map.** `lm-conic-compiled-source-map.json` lists every word: 1,036 compiled routine words, 7 words of CHECKCTR (`lm_conic_checkctr`) and 29 constants that fixed operands read. None is untranslated.
- **Independent assembly comparison.** `lm-conic-compiled-proof.json` and `lm-conic-compiled-differential.log`, made by `make lm-conic-compiled-differential`. 64 states per routine (29,120 cases, a quarter with a job waiting at each DANZIG) equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. Every 128th case is frozen in `tests/lm_conic_compiled_vectors.h`.
- **Host and sanitizer results.** `make test-lm-conic-compiled sanitize-lm-conic-compiled` runs the 228 frozen cases (`lm-conic-compiled-host.log`).
- **C64 execution results.** `make vice-lm-conic-compiled` runs 13 diagnostics with 910 cases in VICE (`lm-conic-compiled-vice.log`). The image lives in the RAM under KERNAL at $e000, and the programs build without inlining, with a static stack and with 110 zero-page bytes.
