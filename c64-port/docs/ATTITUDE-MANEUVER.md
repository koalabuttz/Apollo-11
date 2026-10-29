# Translate ATTITUDE_MANEUVER_ROUTINE and ANGLFIND

Lane D1b (alignment and maneuvers). Milestone `attitude-maneuver`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/ATTITUDE_MANEUVER_ROUTINE.agc` (`src/lm_attitude_maneuver.c`) and its Comanche counterpart `Comanche055/ANGLFIND.agc` (`src/cm_attitude_maneuver.c`) are native C in full. They are KALCMANU's maneuver planning: the rotation from the initial to the desired attitude, the maneuver axis and angle, and the matrix and CDU-angle helpers the steering jobs call.

## Scope

| Program | Routines |
|---|---|
| Both | KALCMAN3 and SECAD (MIS, MFS, MFI), CHECKMAX and ALTCALC (gimbal lock avoidance), COMP12, METHOD1, METHOD3 and their sign cases, MXM3, TRANSPOS and TRNSPSPD, READCDUK, CDUTODCM with LOOPSIN, DELCOMP, DCMTOCDU with its quadrant fixes, VECOFANG, NOGO |
| Luminary099 | TOOBADF, TOOBADI (the 70-degree middle gimbal abort) |
| Comanche055 | NOGOM2 |

224 Luminary and 236 Comanche routines (with the resume points below; Comanche also enters NOGO -2, where KALCMANU_STEERING's `BZMF NOGO -2` lands). Destinations outside the file (ALARM, BANKCALL, IBNKCALL, PHASCHNG, WAITLIST, STOPRATE, LOADCDUD, POSTJUMP, ENDOFJOB, GOTOPOOH) stop with `EXEC_UNMAPPED` until their files have routines. RTB CDULOGIC is native (`src/rtb_angles.c`).

## Evidence

Hashes: `evidence/attitude-maneuver/SHA256.json`. Bulk files are assets of the `evidence-attitude-maneuver` release.

- **Source map.** `attitude-maneuver-source-map.json` lists every emitted word with its routine: 480 Luminary words and 479 Comanche words, none untranslated.
- **Independent assembly comparison.** `make attitude-maneuver-differential`: 512 states per routine and program (235,520 cases) equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. A quarter of the cases put edge values into MPAC and every operand the file names: +0 and -0, one LSB, 1/2, 1/4, just below 1, half-unit axis vectors (with a matrix's rows as a signed permutation, so identity and 90° rotations occur), unit vectors in a coordinate plane, and tiny vectors. In those cases the CDUs sit at 0, 45, 90, 180 or 270°. TIME1, TIME2 and the CDUs vary in every case. The cases run under EB 6 (the file's `EBANK= BCDU`). X1 and X2 hold complemented matrix addresses, as the callers' AXC orders load them. LOOPSIN is entered with X1 3, 2 or 1, S1 1 and the pushdown list at 0D; SUHALFA and CALCPHI with it at 4D. Half of the cases start from one joint set: MFI at the identity or a 180° axis rotation (at both .25 and .5 scale), so CAM is exactly .5 and AM = ARCCOS(1), or COF with each ordering of its components, ties included, for COFMAXGO's BMN arms.
- **Resume points.** Resume points: a DANZIG that finds NEWJOB positive goes to CHANG2 and the job resumes later at the next word, so with `resume_keys` every word after a DANZIG is a routine of its own (the routine counts include them). A quarter of the cases at those routines have NEWJOB positive, and the run stops at CHANG2 as the original does. Each resume point starts from the MODE, PUSHLOC and index registers the code before it leaves there (per-point `states`). Runs may take up to 1,000 routines (`max_steps`).
- **Host and sanitizer results.** `make test-attitude-maneuver sanitize-attitude-maneuver` runs the 1,170 frozen cases (every 256th plus a greedy cover of every branch outcome any case reaches).
- **C64 execution results.** `make vice-attitude-maneuver` runs 22 diagnostics with 556 cases, chosen by outcome cover; the log records each PRG's sha256. The diagnostic keeps the 4 KB image at $e000 under the KERNAL ROM (interrupts off, `$01=$35`) and builds with `-fno-inline`, so every routine, including KALCMAN3's path through SECAD, has C64 cases. All 3,153 branch outcomes the host vectors reach are reached on the C64 (`attitude-maneuver-c64-outcomes.json`, cited as `c64_branch_outcomes`). Six of the rows are segments (`c64_segments` in the proof). The NEWJOB +0 arm of KALCMAN3's first resume points (Luminary 22,2012, 22,2015 and 22,2022, Comanche 22,2003, 22,2004 and 22,2011) runs on through the whole maneuver setup, which does not fit one C64 diagnostic. So each of those rows runs from its resume routine to the next resume LOC, which the group's KEEP mask leaves out, and yaAGC stops at NEWOPS with LOC there (`IRC_SNAPROW`). Each segment equals the oracle on the host with the same mask before it is frozen. The trampoline's step-limit arm (`agc_transfer.c:50`) the earlier freeze listed was an artifact of the edge runner's 200-routine default; it now takes the spec's `max_steps`.
