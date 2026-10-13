# Translate Comanche P11

Lane A2 (restart and telemetry; the file waited on Lane G's whole-file basic tooling, #107). Milestone `cm-p11`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/P11.agc` (`src/cm_p11.c`, banks 23, 34, 37 and 43, the last under superbank 4) is native C in full: P11, the Earth orbit insertion monitor. It zeroes the clock at liftoff and updates TEPHEM, starts SERVICER, computes the liftoff REFSMMAT and state vector (MATRXJOB), runs the attitude error display (ATERTASK, ATERJOB, ATTDISP, the polynomial pitch program), the boost steering takeover (STEERSAT, SATSTKON, SATSTICK, REDOSAT, S11.1) and its restart points (REP11A, REP11). It is compiled whole-file, basic and interpretive, as 148 routines of the control-transfer ABI (spec `cm-p11`, `resume_keys`). Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file.

## Scope

Entries other files reach, all registered routines: P11 (POSTJUMP from IMU_CALIBRATION_AND_ALIGNMENT), MATRXJOB, REP11A, REP11 and ATERTASK (RESTART_TABLES; MATRXJOB also by BANKCALL), ATERJOB (FINDVAC), REDOSAT (FRESH_START_AND_RESTART), S11.1 and SATSTKON (P40-P47). The other routines are the continuations after outside calls, the file's own transfer targets and every interpretive word that can follow a DANZIG (a CHANG2 resume key).

The spec states the facts the cases need:
- STICKCHK's rate indexes RMANNDX, PMANNDX and YMANNDX (E6,1656-1660) hold 0, 1 or 2.
- SATSW (E6,1725), the boost takeover switch, holds +1, +0 or -1.
- RESCALES can return with overflow in A: TS AK2 then skips into SATOUT's CADR NEEDLER, which the AGC runs as CS 2420 (on to ATERSET). The port does the same (`execute_data` 34,2472).
- CA SATSW; CCS A has a -0 exit onto STEERSAT's TLOAD TAD (34,2453), which no SATSW value takes. It is a data landing (`interp_landings`), so its basic copy adds no return points that would shadow the interpretive STORE AK.
- NOPOLYM's RTB V1STO2S runs the native `rtb_angles` vector routine; the second RTB (DELSTOR) is in the file.

Compiler additions with this file:
- `tools/interp_compile.py`: in a whole-file compile, a CALL or STCALL followed by basic code has no return point, as `call_return` already did for interpretive files (GETDOWN's STCALL 8D LALOTORV +5 leaves through the INCORPEX return GETDOWN saved).
- `tools/basic_compile.py`: `interp_landings` names interpretive words that basic flow reaches only on a path no admitted state takes.

Every other spec's generated files are unchanged (`interp_compile.py --check` on all specs).

## Evidence

Hashes: `evidence/cm-p11/SHA256.json`. Bulk files are assets of the `evidence-cm-p11` release.

- **Source map.** `cm-p11-source-map.json` lists every word with the routine that compiles it: 535 words, none untranslated.
- **Independent assembly comparison.** `make cm-p11-differential`: 256 states per routine (37,888 cases) equal the pinned flight-device yaAGC in Z, all 2,048 erasable words, the interrupt-enable state, and channels 12, 30 and 31 with the 31A/31B/32 trap flags. Every 64th case is frozen in `tests/cm_p11_vectors.h`.
- **Host and sanitizer results.** `make test-cm-p11 sanitize-cm-p11` runs the 592 frozen cases (`cm-p11-host.log`).
- **C64 execution results.** `make vice-cm-p11` runs eight diagnostics with 203 cases (`cm-p11-vice.log`). They reach all 284 compiled labels and all 2,232 edges, and take all 1,141 gcov branch arcs the host cases take over every linked source (`cm-p11-proof.json`, `c64_coverage` and `c64_gcov_branches`).
