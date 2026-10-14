# Translate INTEGRATION_INITIALIZATION (Luminary099)

Lane B (orbital math). Milestone `lm-integration-init`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/INTEGRATION_INITIALIZATION.agc` (`src/lm_integration_init.c`, 623 words in banks 13 and 04) is native C in full. It is the front end of orbital integration: the entries other files call to integrate a state vector, the integration stall and wake-up logic, the moves between the permanent and temporary state vectors, and the end-of-integration bookkeeping. Each of its 217 routines runs the file from its entry, or from where it resumes, to the point where control leaves the file.

## Scope

- **Entries.** INTEGRV, INTEGRVS, INTEGRV2, LEMPREC, CSMPREC, LEMCONIC, CSMCONIC, INTSTALL, SETIFLGS, INTWAKE, INTWAKE0, INTWAKE1, INTWAKEU, TESTLOOP, ATOPCSM, ATOPLEM, CLRMOON, SETMOON, CKMID2, GRP2PC, A-PCHK, STATEINT, STATINT1, AVETOMID, MIDTOAV1, MIDTOAV2, ENDINT and PHEXIT, each keyed by its physical address in `lm_integration_init_interpretive` or `lm_integration_init_basic`.
- **Resumes.** Every branch target, CALL/STCALL return, EXIT, TC INTPRET point and return from an outside TC (PHASCHNG, FINDVAC, JOBWAKE, LONGCALL, UPFLAG, DOWNFLAG, TPAGREE) is a routine of its own.
- **The state-vector moves.** MOVEACSM, MOVEALEM, MOVEPCSM, MOVEPLEM and their SETBANK (with FORTYONE and INTBANK) are reached only through RTB. They run as `lm_integration_move` (`src/lm_integration_moves.c`). The oracle runs the original basic code through each RTB, so these are checked by the same cases.
- **Leaves at.** NEWOPS with LOC outside the file (ORBITAL_INTEGRATION's TIMESTEP and the callers' returns); the interpreter's POODOO1/ALARM1 exits; and the outside basic routines ALARM, BANKJUMP, DOWNFLAG, ENDOFJOB, FINDVAC, JOBSLEEP, JOBWAKE, LONGCALL, PHASCHNG, TASKOVER, TPAGREE and UPFLAG.

Rendezvous navigation (P20-P25, MEASUREMENT_INCORPORATION) calls INTSTALL, SETIFLGS, INTEGRV, INTWAKE/INTWAKE0, LEMCONIC/CSMCONIC and LEMPREC/CSMPREC. Those entries take the image as the AGC holds it at that address, so callers need no adapter.

## States the cases start from

As in every compiled file, each case is a random 2,048-word image with the routine's legal entry state. This file also fixes:

- PBODY (0 Earth or 2 Moon);
- DIFEQCNT (0, 77763 or 77747 octal);
- the X1/X2 index registers (0 or 2);
- PUSHLOC inside the VAC area;
- INTWAKUQ at ENDOFJOB.

The cases also draw flight edge values (review #165, `integration_joint` in `tools/interp_specs.py`): the fourth case shape of `tools/edge_cases.py`; TIME2/TIME1 and the CDUs across their range (`counters`); and, in half of the cases, TDEC = TET (DT/2COMP +0, the last pass of an integration), UPSVFLAG +0 (UPDATE_PROGRAM's normal INTWAKEU) or ±2, MODREG +0 or 27 (P27 running), or a small ALPHAV. A CHANG2 resume starts from the state the original program is in when it gets there (`resume_flow`).

The file runs as a job, so the compiler's `resume_keys` makes the word after every DANZIG a routine of its own: a DANZIG that finds NEWJOB positive stops at CHANG2, and the job resumes at that routine's key. A quarter of the cases have a job waiting. MEASUREMENT_INCORPORATION reaches MOVEACSM, MOVEPCSM, MOVEALEM and MOVEPLEM directly (RTB), so they are registered routines too (spec `native_entries`). Each runs `lm_integration_move` and then its TC DANZIG as a pair's end. MEASUREMENT_INCORPORATION's `DOCSM RTB GOTO MOVEPCSM` and `DOCSM1 RTB CALL MOVEACSM` are left-hand RTBs: EDOP still holds the pair's right order (GOTO 0127, CALL 0153), and DANZIG takes it at OPJUMP. Such a run stops at the TC DANZIG, where the caller's interpreter continues; a quarter of each move's cases hold each order in EDOP, and the oracle stops there too.

STATEINT, from P00 and V37, runs INTSTALL under a NOVAC job, whose FIXLOC is LOC(MPAC +6) - LOC(QPRET) = 0110: the work area is the core set's own registers. INTSTALL, INTSTALL +1 and GOBAC +1 accept that FIXLOC (spec `novac`), and a quarter of their cases run with it. Before, the whole-program run (docs/FLIGHT-STORE.md) found the three refused and ran them in yaAGC.

TBASE2 holds INTWAKE's saved QPRET, which GOBAC's RVQ returns to, so it is always a return outside the file (spec `return_words`). A random TBASE2 that pointed into the middle of an instruction made 4 of 29,440 cases differ at 13,3436. The original never stores such a value there.

## Evidence

Hashes: `evidence/lm-integration-init/SHA256.json`. Bulk files are assets of the `evidence-lm-integration-init` release.

- **Source map.** `lm-integration-init-source-map.json` lists every word:
  - 567 compiled routine words;
  - 42 words of the RTB move routines (`lm_integration_move`);
  - 14 constant words that basic code and fixed operands read.
  - None is untranslated.
- **Independent assembly comparison.** `lm-integration-init-proof.json` and `lm-integration-init-differential.log`, made by `make lm-integration-init-differential`. 192 states per routine (41,664 cases) equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. Every 128th case is frozen in `tests/lm_integration_init_vectors.h`.
- **Host and sanitizer results.** `make test-lm-integration-init sanitize-lm-integration-init` runs the 326 frozen cases (`lm-integration-init-host.log`).
- **C64 execution results.** `make vice-lm-integration-init` runs 12 diagnostics with 259 cases in VICE (`lm-integration-init-vice.log`).
- **C64 branch outcomes.** `lm-integration-init-c64-cover.json`: the C64 cases are the fewest that take every gcov branch arc the 41,664 host cases take over all 15 linked sources (`'c64_rows': 'cover'`, `tools/c64_row_cover.py`). All 1,685 host arcs are taken on the C64; none is host-only. Before (review #165), most rows stopped at CHANG2 and 204 host outcomes never ran on the C64. The 4 KB image lives in the RAM under KERNAL at $e000. The programs build without inlining, with a static stack and with 110 zero-page bytes, so each group of routines fits below the $c000 mailbox.
