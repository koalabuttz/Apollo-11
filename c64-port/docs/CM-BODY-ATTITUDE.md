# Translate Comanche CM_BODY_ATTITUDE

Lane G (CM flight). Milestone `cm-body-attitude`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/CM_BODY_ATTITUDE.agc` (`src/cm_body_attitude.c`) is native C in full: CM/POSE (entry body attitude), CM/POSE2/3, CM/ATUP and REDOPOSE. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

## Scope

8 routines, keyed by physical address in `cm_body_attitude_basic` and `cm_body_attitude_interpretive`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 37,3470 CM/POSE | basic | entry CM/POSE (P61-P67.agc 2CADR CM/POSE) |
| 37,3471  | interpretive | after TC INTPRET |
| 37,3517 CM/POSE2 | interpretive | TIX target |
| 37,3627  | interpretive | STCALL return |
| 37,3645  | interpretive | STCALL return |
| 37,3655 CM/ATUP | basic | after EXIT |
| 37,3744 REDOPOSE | basic | 2CADR REDOPOSE |
| 37,3753 CM/POSE3 | interpretive | after TC INTPRET |

Destinations outside the file: only the caller's return through Q. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. Any values of the attitude erasables; interpretive routines entered at NEWOPS with FIXLOC a VAC area.

## Evidence

Hashes: `evidence/cm-body-attitude/SHA256.json`; all files are in the tree.

- **Source map.** `cm-body-attitude-source-map.json` lists all 195 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-body-attitude-differential` runs 256 states per routine (2,048 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC, its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state. The cases and the stops reached per routine are in `cm-body-attitude-proof.json` (`cm-body-attitude-differential.log` is the run). One case in 32 is frozen in `tests/cm_body_attitude_vectors.h`.
- **Host and sanitizer results.** `make test-cm-body-attitude sanitize-cm-body-attitude` runs the 64 frozen cases (`cm-body-attitude-host.log`).
- **C64 execution results.** `make vice-cm-body-attitude` runs 2 diagnostic programs with 12 cases (`cm-body-attitude-vice.log`, which starts with each PRG's SHA-256). The cases are chosen per routine so the C64 runs reach every compiled label the host cases reach: 72 of 72.
