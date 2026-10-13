# Translate ORBITAL_INTEGRATION (Luminary099)

Lane B (orbital math). Milestone `lm-orbital-integration`. How the routines are made and checked: `docs/INTERP-COMPILE.md`. The file's front end, INTEGRATION_INITIALIZATION, is `docs/INTEGRATION-INITIALIZATION.md`.

`Luminary099/ORBITAL_INTEGRATION.agc` (`src/lm_orbital_integration.c`, 841 words in banks 11, 12 and 13) is native C in full. It is Encke integration of a state vector: the Kepler prepare step, the time step, the disturbing accelerations (Earth oblateness, Moon and Sun), rectification, the sphere-of-influence change and the W-matrix extrapolation. Each of its 353 routines runs the file from its entry, or from where it resumes, to the point where control leaves the file.

## Scope

- **Entries and labelled resumes.** KEPPREP, FBR3, ACCOMP, GETRPSV, GAMCOMP, GOBAQUE, INT-ABRT, OBLATE, COMTERM, QUALITY1, QUALITY3, NBRANCH, COSPHIE, TIMESTEP, INLUNCHK, RECTEST, CALLRECT, INTGRATE, DIFEQ0, EARSPH, LUNSPH, DOSWITCH, ORIGCHNG, RECTIFY, MINIRECT, DIFEQ+0/+1/+2, NEXTCOL, ENDSTATE, AMOVED, RELOADSV, DIFEQCOM, WMATEND, DOW.., DOW..1 and SETITCTR. Each is keyed by its physical address in `lm_orbital_integration_interpretive` or `lm_orbital_integration_basic`.
- **Other resumes.** Every other branch target, CALL return and return from an outside TC is a routine too.
- **Job switches.** The file runs as a job, so the word after every DANZIG is a routine too (`resume_keys`). A DANZIG that finds NEWJOB positive stops at CHANG2, and the Executive resumes the job at that routine's key.
- **Constructs the compiler gained for this file:**
  - CGOTO through DIFEQTAB;
  - indexed loads of bank 13 and bank 11 constants by PBODY and the NORM count (`indexed_fixed`);
  - BOVB TCDANZIG, the RTB to TC DANZIG in TIME_OF_FREE_FALL (`lm_integration_tcdanzig`).
- **Leaves at:**
  - NEWOPS with LOC outside the file: INTEGRATION_INITIALIZATION's A-PCHK, ATOPCSM/ATOPLEM and STATEINT, and the callers' returns;
  - the interpreter's POODOO1/ALARM1 exits;
  - the outside basic routines ALARM, PHASCHNG, POODOO and UPFLAG.

## States the cases start from

As in every compiled file, each case is a random 2,048-word image with the routine's legal entry state. This file also fixes:

- PBODY (0 Earth or 2 Moon);
- DIFEQCNT (0, 77763 or 77747 octal);
- COLREG (0 or a column step, -6 to -48);
- the X1/X2 index registers (0 or 2);
- S1/S2 (0, 2, 4 or 6);
- PUSHLOC inside the VAC area.

These are the values the file itself stores.

The cases also draw flight edge values (review #165, `integration_joint` in `tools/interp_specs.py`): the fourth case shape of `tools/edge_cases.py`; TIME2/TIME1 and the CDUs across their range (`counters`); and, in half of the cases, TDEC = TET, UPSVFLAG +0 or ±2, MODREG +0 or 27 (P27 running, the BHIZ after `SLOAD DSU MODREG OCT27`), or a small ALPHAV after RECTIFY.

A CHANG2 resume starts from the state the original program is in when it gets there (`resume_flow`: the oracle runs each case of the routine before it to that LOC). Before, the 11,2674 `STORE S1` resume drew a random MODE: entered in vector mode it stored 6 words into QPRET, which the program never does, since the `DLOAD* ASCALE,1` before it always leaves MODE in DP.

A quarter of the cases have a job waiting (NEWJOB positive). A resume in the middle of a sequence keeps that sequence's invariant. Between ACCOMP's `XCHX,2 DIFEQCNT` pairs, X2 holds the DIFEQ stage and DIFEQCNT holds PBODY. Inside GAMCOMP's `XCHX,2 S1` pair, S1 holds PBODY. NBRANCH's `DMP CGOTO` has stage 0 loaded (NBRANCH itself runs every stage). TIMESTEP's W stores follow a VLOAD and index by COLREG. A case may run up to 1,000 routines, because one DIFEQ step runs several hundred pairs.

## Evidence

Hashes: `evidence/lm-orbital-integration/SHA256.json`. Bulk files are assets of the `evidence-lm-orbital-integration` release.

- **Source map.** `lm-orbital-integration-source-map.json` lists every word:
  - 789 compiled routine words;
  - 49 constant words that basic code and fixed operands read;
  - the 3 CADRs of DIFEQTAB, which CGOTO reads.
  - None is untranslated.
- **Independent assembly comparison.** `lm-orbital-integration-proof.json` and `lm-orbital-integration-differential.log`, made by `make lm-orbital-integration-differential`. 128 states per routine (45,184 cases) equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. Every 128th case is frozen in `tests/lm_orbital_integration_vectors.h`.
- **Host and sanitizer results.** `make test-lm-orbital-integration sanitize-lm-orbital-integration` runs the 353 frozen cases (`lm-orbital-integration-host.log`).
- **C64 execution results.** `make vice-lm-orbital-integration` runs 9 diagnostics with 706 cases in VICE (`lm-orbital-integration-vice.log`). The image lives under KERNAL at $e000, and the programs build without inlining, with a static stack and with 110 zero-page bytes. A routine whose cases run through more code than one program holds keeps the case that visits the fewest routines. The largest is KEPPREP with the oblateness terms.
