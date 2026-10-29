# Translate Comanche CM_BODY_ATTITUDE

Lane G (CM flight). Milestone `cm-body-attitude`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/CM_BODY_ATTITUDE.agc` (`src/cm_body_attitude.c`) is native C in full: CM/POSE (entry body attitude), CM/POSE2/3, CM/ATUP and REDOPOSE. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

## Scope

70 routines, keyed by physical address in `cm_body_attitude_basic` and `cm_body_attitude_interpretive`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 37,3470 CM/POSE | basic | entry CM/POSE (P61-P67.agc 2CADR CM/POSE) |
| 37,3471  | interpretive | after TC INTPRET |
| 37,3474  | interpretive | CHANG2 resume |
| 37,3477  | interpretive | CHANG2 resume |
| 37,3502  | interpretive | CHANG2 resume |
| 37,3504  | interpretive | CHANG2 resume |
| 37,3506  | interpretive | CHANG2 resume |
| 37,3507  | interpretive | CHANG2 resume |
| 37,3511  | interpretive | CHANG2 resume |
| 37,3514  | interpretive | CHANG2 resume |
| 37,3517 CM/POSE2 | interpretive | TIX target |
| 37,3520  | interpretive | CHANG2 resume |
| 37,3521  | interpretive | CHANG2 resume |
| 37,3523  | interpretive | CHANG2 resume |
| 37,3524  | interpretive | CHANG2 resume |
| 37,3525  | interpretive | CHANG2 resume |
| 37,3527 CM/TRIO | interpretive | CHANG2 resume |
| 37,3531  | interpretive | CHANG2 resume |
| 37,3533  | interpretive | CHANG2 resume |
| 37,3534  | interpretive | CHANG2 resume |
| 37,3535  | interpretive | CHANG2 resume |
| 37,3536  | interpretive | CHANG2 resume |
| 37,3540  | interpretive | CHANG2 resume |
| 37,3542  | interpretive | CHANG2 resume |
| 37,3543  | interpretive | CHANG2 resume |
| 37,3545  | interpretive | CHANG2 resume |
| 37,3547  | interpretive | CHANG2 resume |
| 37,3550  | interpretive | CHANG2 resume |
| 37,3552  | interpretive | CHANG2 resume |
| 37,3554  | interpretive | CHANG2 resume |
| 37,3556  | interpretive | CHANG2 resume |
| 37,3557  | interpretive | CHANG2 resume |
| 37,3561  | interpretive | CHANG2 resume |
| 37,3563  | interpretive | CHANG2 resume |
| 37,3565  | interpretive | CHANG2 resume |
| 37,3567  | interpretive | CHANG2 resume |
| 37,3570  | interpretive | CHANG2 resume |
| 37,3573  | interpretive | CHANG2 resume |
| 37,3575  | interpretive | CHANG2 resume |
| 37,3577  | interpretive | CHANG2 resume |
| 37,3600  | interpretive | CHANG2 resume |
| 37,3602  | interpretive | CHANG2 resume |
| 37,3603  | interpretive | CHANG2 resume |
| 37,3606  | interpretive | CHANG2 resume |
| 37,3610  | interpretive | CHANG2 resume |
| 37,3611  | interpretive | CHANG2 resume |
| 37,3613  | interpretive | CHANG2 resume |
| 37,3615  | interpretive | CHANG2 resume |
| 37,3617  | interpretive | CHANG2 resume |
| 37,3621  | interpretive | CHANG2 resume |
| 37,3623  | interpretive | CHANG2 resume |
| 37,3625  | interpretive | CHANG2 resume |
| 37,3627  | interpretive | STCALL return |
| 37,3631  | interpretive | CHANG2 resume |
| 37,3633  | interpretive | CHANG2 resume |
| 37,3634  | interpretive | CHANG2 resume |
| 37,3636  | interpretive | CHANG2 resume |
| 37,3640  | interpretive | CHANG2 resume |
| 37,3641  | interpretive | CHANG2 resume |
| 37,3643  | interpretive | CHANG2 resume |
| 37,3645  | interpretive | STCALL return |
| 37,3647  | interpretive | CHANG2 resume |
| 37,3651  | interpretive | CHANG2 resume |
| 37,3652  | interpretive | CHANG2 resume |
| 37,3653  | interpretive | CHANG2 resume |
| 37,3655 CM/ATUP | basic | after EXIT |
| 37,3744 REDOPOSE | basic | 2CADR REDOPOSE |
| 37,3753 CM/POSE3 | interpretive | after TC INTPRET |
| 37,3755  | interpretive | CHANG2 resume |
| 37,3756  | interpretive | CHANG2 resume |

Destinations outside the file: CHANG2. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. `NEWJOB` is +0 or, in a quarter of the cases, positive: a job waiting, so any `DANZIG` goes to `CHANG2`, where the routine stops; every interpretive word that can follow a `DANZIG` is a `CHANG2` resume key of its own. Any values of the attitude erasables; interpretive routines entered at NEWOPS with FIXLOC a VAC area.

## Evidence

Hashes: `evidence/cm-body-attitude/SHA256.json`; all files are in the tree.

- **Source map.** `cm-body-attitude-source-map.json` lists all 195 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-body-attitude-differential` runs 256 states per routine (17,920 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC, its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state. The cases and the stops reached per routine are in `cm-body-attitude-proof.json` (`cm-body-attitude-differential.log` is the run). One case in 32 is frozen in `tests/cm_body_attitude_vectors.h`.
- **Host and sanitizer results.** `make test-cm-body-attitude sanitize-cm-body-attitude` runs the 560 frozen cases (`cm-body-attitude-host.log`).
- **C64 execution results.** `make vice-cm-body-attitude` runs 12 diagnostic programs with 97 cases (`cm-body-attitude-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 17,920 differential cases take: 1682 of 1682 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 72 of 72 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 811 of the 811 branch outcomes all the cases take (`c64_gcov_branches` in the proof).

## Flight build

The spec sets `trusted_image`, `image_w` and `index_carry16`. In flight builds (`RK_FIXED_IMAGE`), the entry checks skip their scan of the whole erasable image, which costs about 121,000 cycles a routine on the C64. That scan was the top cost of the CM entry window in VICE (lane-g/cm-entry-profile-80342904.md in the project files). Host tests and C64 diagnostics keep the scan.
