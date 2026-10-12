# Gimbal lock avoidance as native routines

Lane D1 (IMU and hardware). Milestone `gimbal-lock`, for issue #129. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/GIMBAL_LOCK_AVOIDANCE.agc` (`src/lm_gimbal_lock.c`) and `Comanche055/GIMBAL_LOCK_AVOIDANCE.agc` (`src/cm_gimbal_lock.c`) are native C in full, as registered routines of the control-transfer ABI. Both files are part of KALCMANU, which runs as a job, in bank 22 (`SETLOC KALCMON1`):
- LOCSKIRT (LM NOGIMLOC, CM WCALC) picks the maneuver rate from RATEINDX;
- it calls DELCOMP for the one-second rotation matrix, and forms BRATE, TM and (CM) the overshoot bias;
- it ends with SETGO CALCMAN2 to NEWANGL +1.

## Scope

| Program | Routines |
|---|---|
| Luminary099 | 9: NOGIMLOC (22,2744), WCALC (22,2746), the CALL DELCOMP return and every word after a DANZIG |
| Comanche055 | 11: WCALC (22,2737), the CALL DELCOMP return and every word after a DANZIG |

KALCMANU reaches LOCSKIRT by interpretive GOTO and BPL: LM ATTITUDE_MANEUVER_ROUTINE (522, 598, 603, 618 and on), and CM ANGLFIND 137. Those transfers land at NEWOPS with LOC at the entry, which is now a key of the routine tables (`lm_gimbal_lock_interpretive` and `cm_gimbal_lock_interpretive`). Every word where the job can resume after CHANG2 is a key too (`resume_keys`).

Destinations outside the file stop with `EXEC_UNMAPPED` until their files have routines:
- the CALL to DELCOMP (ATTITUDE_MANEUVER_ROUTINE and ANGLFIND);
- the GOTO to NEWANGL +1;
- CHANG2, when a DANZIG finds NEWJOB positive.

ARATE,1 reads one of the four rates. The index guard (`index_guard`) keeps the starred operand inside a window of the rope words it can reach.

These routines replace `src/gimbal_complete.c` as the file's translation. Nothing in `src/` called that code, and it had no routine table.

## Evidence

Hashes: `evidence/gimbal-lock/SHA256.json`. Bulk files are assets of the `evidence-gimbal-lock` release.

- **Source map.** `gimbal-lock-source-map.json` lists every emitted word of both files with its routine, and none is untranslated.
- **Independent assembly comparison.** `make gimbal-lock-differential` runs 512 states per routine (10,240 cases) through the unmodified ropes in yaAGC. Every case equals the native run in Z, all 2,048 erasable words and the interrupt-enable state. RATEINDX is 0, 2, 4 or 6, and NEWJOB is positive in a quarter of the cases. Record: `gimbal-lock-proof.json` (ropes, case and oracle hashes, stops per routine) and `gimbal-lock-differential.log`.
- **Host and sanitizer results.** `make test-gimbal-lock sanitize-gimbal-lock` runs the 320 frozen cases (every 32nd).
- **C64 execution results.** `make vice-gimbal-lock` runs two diagnostics, one per mission, with 40 cases (18 Luminary, 22 Comanche), at least one per routine. The log gives each PRG's SHA-256.
