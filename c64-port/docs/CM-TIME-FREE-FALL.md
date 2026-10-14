# Translate Comanche TIME_OF_FREE_FALL

Lane G (CM flight). Milestone `cm-time-free-fall`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/TIME_OF_FREE_FALL.agc` (`src/cm_time_free_fall.c`) is native C in full: The conic time-of-free-fall routines: CALCTPER and CALCTFF (time to the perigee or a radius), TFFCONIC (the conic parameters), with their helpers TFFRP/RA, TFFQ1, TFFX and the DUMPTFF exits, all interpretive. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

## Scope

121 routines, keyed by physical address in `cm_time_free_fall_basic` and `cm_time_free_fall_interpretive`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 27,2757 TFFCONIC | interpretive | entry TFFCONIC (P61-P67.agc  TFFCONIC) |
| 27,2760 TFFCONMU | interpretive | entry TFFCONMU (R30.agc  TFFCONMU) |
| 27,2762  | interpretive | CHANG2 resume |
| 27,2764  | interpretive | CHANG2 resume |
| 27,2765  | interpretive | CHANG2 resume |
| 27,2767  | interpretive | CHANG2 resume |
| 27,2771  | interpretive | CHANG2 resume |
| 27,2773  | interpretive | CHANG2 resume |
| 27,2774  | interpretive | CHANG2 resume |
| 27,2776  | interpretive | CHANG2 resume |
| 27,2777  | interpretive | CHANG2 resume |
| 27,3001  | interpretive | CHANG2 resume |
| 27,3004  | interpretive | CHANG2 resume |
| 27,3005  | interpretive | CHANG2 resume |
| 27,3006  | interpretive | CHANG2 resume |
| 27,3010  | interpretive | CHANG2 resume |
| 27,3012  | interpretive | CHANG2 resume |
| 27,3014  | interpretive | CHANG2 resume |
| 27,3015  | interpretive | CHANG2 resume |
| 27,3017  | interpretive | CHANG2 resume |
| 27,3020  | interpretive | CHANG2 resume |
| 27,3021  | interpretive | CHANG2 resume |
| 27,3024  | interpretive | BZE target |
| 27,3025 DUMPCNIC | interpretive | CHANG2 resume |
| 27,3026 TFFRP/RA | interpretive | entry TFFRP/RA (R30.agc  TFFRP/RA) |
| 27,3031  | interpretive | CHANG2 resume |
| 27,3033  | interpretive | CHANG2 resume |
| 27,3035  | interpretive | CHANG2 resume |
| 27,3037  | interpretive | CHANG2 resume |
| 27,3041  | interpretive | CHANG2 resume |
| 27,3044  | interpretive | CHANG2 resume |
| 27,3045  | interpretive | CHANG2 resume |
| 27,3050  | interpretive | CHANG2 resume |
| 27,3053  | interpretive | CHANG2 resume |
| 27,3056  | interpretive | CHANG2 resume |
| 27,3060 MAXRA | interpretive | BZE target |
| 27,3062  | interpretive | BPL target |
| 27,3063 DUMPRPRA | interpretive | CHANG2 resume |
| 27,3064 CALCTPER | interpretive | entry CALCTPER (R30.agc  CALCTPER) |
| 27,3067 CALCTFF | interpretive | entry CALCTFF (R30.agc  CALCTFF) |
| 27,3071  | interpretive | CHANG2 resume |
| 27,3072  | interpretive | CHANG2 resume |
| 27,3074  | interpretive | CHANG2 resume |
| 27,3075  | interpretive | CHANG2 resume |
| 27,3100  | interpretive | CHANG2 resume |
| 27,3102  | interpretive | CHANG2 resume |
| 27,3105  | interpretive | CHANG2 resume |
| 27,3106  | interpretive | CHANG2 resume |
| 27,3110  | interpretive | CHANG2 resume |
| 27,3114  | interpretive | switch branch target |
| 27,3116  | interpretive | CHANG2 resume |
| 27,3120  | interpretive | CHANG2 resume |
| 27,3122  | interpretive | CHANG2 resume |
| 27,3124  | interpretive | CHANG2 resume |
| 27,3125  | interpretive | CHANG2 resume |
| 27,3130  | interpretive | CHANG2 resume |
| 27,3132  | interpretive | CHANG2 resume |
| 27,3134 SAVEDEN | interpretive | CHANG2 resume |
| 27,3135  | interpretive | CHANG2 resume |
| 27,3140  | interpretive | CHANG2 resume |
| 27,3142  | interpretive | CHANG2 resume |
| 27,3144  | interpretive | CHANG2 resume |
| 27,3146 DUMPTFF1 | interpretive | CHANG2 resume |
| 27,3147 INBOUND | interpretive | BMN target |
| 27,3150  | interpretive | CHANG2 resume |
| 27,3153  | interpretive | CHANG2 resume |
| 27,3155  | interpretive | CHANG2 resume |
| 27,3160 TFFXTEST | interpretive | BOV target |
| 27,3163  | interpretive | CHANG2 resume |
| 27,3166  | interpretive | CHANG2 resume |
| 27,3167  | interpretive | CHANG2 resume |
| 27,3172  | interpretive | CHANG2 resume |
| 27,3174  | interpretive | CHANG2 resume |
| 27,3175  | interpretive | CHANG2 resume |
| 27,3177  | interpretive | CHANG2 resume |
| 27,3202  | interpretive | CHANG2 resume |
| 27,3203  | interpretive | CHANG2 resume |
| 27,3205  | interpretive | CHANG2 resume |
| 27,3207  | interpretive | CHANG2 resume |
| 27,3210  | interpretive | CHANG2 resume |
| 27,3212  | interpretive | after RTB T(X) |
| 27,3214  | interpretive | CHANG2 resume |
| 27,3216  | interpretive | CHANG2 resume |
| 27,3220  | interpretive | CHANG2 resume |
| 27,3222  | interpretive | CHANG2 resume |
| 27,3225  | interpretive | CHANG2 resume |
| 27,3227  | interpretive | CHANG2 resume |
| 27,3230  | interpretive | CHANG2 resume |
| 27,3233  | interpretive | CHANG2 resume |
| 27,3236  | interpretive | CHANG2 resume |
| 27,3240 ENDTFF | interpretive | GOTO target |
| 27,3243 DUMPTFF2 | interpretive | CHANG2 resume |
| 27,3244 NEGTFF | interpretive | BPL target |
| 27,3245  | interpretive | CHANG2 resume |
| 27,3247 MAXTFF1 | interpretive | BMN target |
| 27,3250 MAXTFF | interpretive | BOV target |
| 27,3252 TFFELL | interpretive | BOV target |
| 27,3253  | interpretive | CHANG2 resume |
| 27,3254 TFFEL1 | interpretive | BMN target |
| 27,3257  | interpretive | CHANG2 resume |
| 27,3260  | interpretive | CHANG2 resume |
| 27,3262  | interpretive | CHANG2 resume |
| 27,3265  | interpretive | CHANG2 resume |
| 27,3267  | interpretive | CHANG2 resume |
| 27,3272  | interpretive | CHANG2 resume |
| 27,3273  | interpretive | CHANG2 resume |
| 27,3276  | interpretive | after RTB T(X) |
| 27,3300  | interpretive | CHANG2 resume |
| 27,3301  | interpretive | CHANG2 resume |
| 27,3304  | interpretive | CHANG2 resume |
| 27,3306  | interpretive | CHANG2 resume |
| 27,3310  | interpretive | CHANG2 resume |
| 27,3313  | interpretive | CHANG2 resume |
| 27,3316  | interpretive | CHANG2 resume |
| 27,3320  | interpretive | CHANG2 resume |
| 27,3322  | interpretive | CHANG2 resume |
| 27,3323  | interpretive | CHANG2 resume |
| 27,3326  | interpretive | CHANG2 resume |
| 27,3331  | interpretive | CHANG2 resume |
| 27,3334 T(X) | basic | RTB target |
| 27,3352 ENDT(X) | basic | BOVB target |

Destinations outside the file: BOVB SIGNMPAC, CHANG2, DANZIG, POLY. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. `NEWJOB` is +0 or, in a quarter of the cases, positive: a job waiting, so any `DANZIG` goes to `CHANG2`, where the routine stops; every interpretive word that can follow a `DANZIG` is a `CHANG2` resume key of its own. Any values of the conic erasables; interpretive routines entered at NEWOPS with FIXLOC a VAC area.

## Evidence

Hashes: `evidence/cm-time-free-fall/SHA256.json`; all files are in the tree.

- **Source map.** `cm-time-free-fall-source-map.json` lists all 268 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-time-free-fall-differential` runs 256 states per routine (30,976 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC, its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state. The cases and the stops reached per routine are in `cm-time-free-fall-proof.json` (`cm-time-free-fall-differential.log` is the run). One case in 32 is frozen in `tests/cm_time_free_fall_vectors.h`.
- **Host and sanitizer results.** `make test-cm-time-free-fall sanitize-cm-time-free-fall` runs the 968 frozen cases (`cm-time-free-fall-host.log`).
- **C64 execution results.** `make vice-cm-time-free-fall` runs 6 diagnostic programs with 193 cases (`cm-time-free-fall-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 30,976 differential cases take: 2145 of 2145 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 2 of 2 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 1017 of the 1017 branch outcomes all the cases take (`c64_gcov_branches` in the proof).
