# Translate ATTITUDE_MANEUVER_ROUTINE and ANGLFIND

Lane D1b (alignment and maneuvers). Milestone `attitude-maneuver`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/ATTITUDE_MANEUVER_ROUTINE.agc` (`src/lm_attitude_maneuver.c`) and its Comanche counterpart `Comanche055/ANGLFIND.agc` (`src/cm_attitude_maneuver.c`) are native C in full. They are KALCMANU's maneuver planning: the rotation from the initial to the desired attitude, the maneuver axis and angle, and the matrix and CDU-angle helpers the steering jobs call.

## Scope

| Program | Routines |
|---|---|
| Both | KALCMAN3 and SECAD (MIS, MFS, MFI), CHECKMAX and ALTCALC (gimbal lock avoidance), COMP12, METHOD1, METHOD3 and their sign cases, MXM3, TRANSPOS and TRNSPSPD, READCDUK, CDUTODCM with LOOPSIN, DELCOMP, DCMTOCDU with its quadrant fixes, VECOFANG, NOGO |
| Luminary099 | TOOBADF, TOOBADI (the 70-degree middle gimbal abort) |
| Comanche055 | NOGOM2 |

44 Luminary and 39 Comanche routines. Destinations outside the file (ALARM, BANKCALL, IBNKCALL, PHASCHNG, WAITLIST, STOPRATE, LOADCDUD, POSTJUMP, ENDOFJOB, GOTOPOOH) stop with `EXEC_UNMAPPED` until their files have routines. RTB CDULOGIC is native (`src/rtb_angles.c`).

## Evidence

Hashes: `evidence/attitude-maneuver/SHA256.json`. Bulk files are assets of the `evidence-attitude-maneuver` release.

- **Source map.** `attitude-maneuver-source-map.json` lists every emitted word with its routine: 480 Luminary words and 479 Comanche words, none untranslated.
- **Independent assembly comparison.** `make attitude-maneuver-differential`: 512 states per routine and program (42,496 cases) equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. The cases run under EB 6 (the file's `EBANK= BCDU`). X1 and X2 hold complemented matrix addresses, as the callers' AXC orders load them. LOOPSIN is entered with X1 3, 2 or 1, S1 1 and the pushdown list at 0D; SUHALFA and CALCPHI with it at 4D.
- **Host and sanitizer results.** `make test-attitude-maneuver sanitize-attitude-maneuver` runs the 664 frozen cases (every 64th).
- **C64 execution results.** `make vice-attitude-maneuver` runs ten diagnostics with 80 cases. Three routines (Luminary 22,2022 and Comanche 22,2003 and 22,2011, KALCMAN3's path through SECAD) reach most of the file and do not fit one diagnostic with their cases; `attitude-maneuver-proof.json` lists them as `c64_host_only`. Each routine their runs reach has its own C64 cases.
