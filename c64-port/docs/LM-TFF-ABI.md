# Compile Luminary TIME_OF_FREE_FALL for the flight store

Lane E. Milestone `lm-tff-abi`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Luminary099/TIME_OF_FREE_FALL.agc` is already complete as the hand-written `src/lm_time_free_fall.c` (Lane B, `docs/LM-TIME-FREE-FALL.md`), which has its own API. The LM flight store needs the file as routines of the control-transfer ABI, keyed by address, so V82 (R30) and the other interpretive callers reach it without leaving native code. This milestone adds that compiled form, `src/lm_tff_abi.c`, as `cm-time-free-fall` does for Comanche. The file's coverage certificate is unchanged.

Found by the LM DOI flight scenario (`lm-doi-nassp` in `tools/flight_host_differential.py`): V82 after the burn bridged 95 TIME_OF_FREE_FALL points and 16 interpreter points to yaAGC. With this form placed they are 0.

## Scope

123 routines, keyed by physical address in `lm_tff_abi_basic` and `lm_tff_abi_interpretive`. TFFCONIC (27,3360, `STORE TFF/RTMU`) has no caller in Luminary099; it is an entry of its own so every word of the file compiles.

| Entry | Kind | Why it is a routine |
|---|---|---|
| 27,3360 TFFCONIC | interpretive | entry TFFCONIC |
| 27,3361 TFFCONMU | interpretive | entry TFFCONMU (R30.agc  TFFCONMU) |
| 27,3363  | interpretive | CHANG2 resume |
| 27,3365  | interpretive | CHANG2 resume |
| 27,3366  | interpretive | CHANG2 resume |
| 27,3370  | interpretive | CHANG2 resume |
| 27,3372  | interpretive | CHANG2 resume |
| 27,3374  | interpretive | CHANG2 resume |
| 27,3375  | interpretive | CHANG2 resume |
| 27,3377  | interpretive | CHANG2 resume |
| 27,3400  | interpretive | CHANG2 resume |
| 27,3402  | interpretive | CHANG2 resume |
| 27,3405  | interpretive | CHANG2 resume |
| 27,3406  | interpretive | CHANG2 resume |
| 27,3407  | interpretive | CHANG2 resume |
| 27,3411  | interpretive | CHANG2 resume |
| 27,3413  | interpretive | CHANG2 resume |
| 27,3415  | interpretive | CHANG2 resume |
| 27,3416  | interpretive | CHANG2 resume |
| 27,3420  | interpretive | CHANG2 resume |
| 27,3421  | interpretive | CHANG2 resume |
| 27,3422  | interpretive | CHANG2 resume |
| 27,3425  | interpretive | BZE target |
| 27,3426 DUMPCNIC | interpretive | CHANG2 resume |
| 27,3427 TFFRP/RA | interpretive | entry TFFRP/RA (R30.agc  TFFRP/RA) |
| 27,3432  | interpretive | CHANG2 resume |
| 27,3434  | interpretive | CHANG2 resume |
| 27,3436  | interpretive | CHANG2 resume |
| 27,3440  | interpretive | CHANG2 resume |
| 27,3442  | interpretive | CHANG2 resume |
| 27,3445  | interpretive | CHANG2 resume |
| 27,3446  | interpretive | CHANG2 resume |
| 27,3451  | interpretive | CHANG2 resume |
| 27,3454  | interpretive | CHANG2 resume |
| 27,3457  | interpretive | CHANG2 resume |
| 27,3461 MAXRA | interpretive | BZE target |
| 27,3463  | interpretive | BPL target |
| 27,3464 DUMPRPRA | interpretive | CHANG2 resume |
| 27,3465 CALCTPER | interpretive | entry CALCTPER (R30.agc  CALCTPER) |
| 27,3470 CALCTFF | interpretive | entry CALCTFF (R30.agc  CALCTFF) |
| 27,3472  | interpretive | CHANG2 resume |
| 27,3473  | interpretive | CHANG2 resume |
| 27,3475  | interpretive | CHANG2 resume |
| 27,3476  | interpretive | CHANG2 resume |
| 27,3501  | interpretive | CHANG2 resume |
| 27,3503  | interpretive | CHANG2 resume |
| 27,3506  | interpretive | CHANG2 resume |
| 27,3507  | interpretive | CHANG2 resume |
| 27,3511  | interpretive | CHANG2 resume |
| 27,3515  | interpretive | switch branch target |
| 27,3517  | interpretive | CHANG2 resume |
| 27,3521  | interpretive | CHANG2 resume |
| 27,3523  | interpretive | CHANG2 resume |
| 27,3525  | interpretive | CHANG2 resume |
| 27,3526  | interpretive | CHANG2 resume |
| 27,3531  | interpretive | CHANG2 resume |
| 27,3533  | interpretive | CHANG2 resume |
| 27,3535 SAVEDEN | interpretive | CHANG2 resume |
| 27,3536  | interpretive | CHANG2 resume |
| 27,3541  | interpretive | CHANG2 resume |
| 27,3543  | interpretive | CHANG2 resume |
| 27,3545  | interpretive | CHANG2 resume |
| 27,3547 DUMPTFF1 | interpretive | CHANG2 resume |
| 27,3550 INBOUND | interpretive | BMN target |
| 27,3551  | interpretive | CHANG2 resume |
| 27,3554  | interpretive | CHANG2 resume |
| 27,3556  | interpretive | CHANG2 resume |
| 27,3561 TFFXTEST | interpretive | BOV target |
| 27,3564  | interpretive | CHANG2 resume |
| 27,3567  | interpretive | CHANG2 resume |
| 27,3570  | interpretive | CHANG2 resume |
| 27,3573  | interpretive | CHANG2 resume |
| 27,3575  | interpretive | CHANG2 resume |
| 27,3576  | interpretive | CHANG2 resume |
| 27,3600  | interpretive | CHANG2 resume |
| 27,3603  | interpretive | CHANG2 resume |
| 27,3604  | interpretive | CHANG2 resume |
| 27,3606  | interpretive | CHANG2 resume |
| 27,3610  | interpretive | CHANG2 resume |
| 27,3611  | interpretive | CHANG2 resume |
| 27,3612  | basic | pending DANZIG after RTB T(X) |
| 27,3613  | interpretive | after RTB T(X) |
| 27,3615  | interpretive | CHANG2 resume |
| 27,3617  | interpretive | CHANG2 resume |
| 27,3621  | interpretive | CHANG2 resume |
| 27,3623  | interpretive | CHANG2 resume |
| 27,3626  | interpretive | CHANG2 resume |
| 27,3630  | interpretive | CHANG2 resume |
| 27,3631  | interpretive | CHANG2 resume |
| 27,3634  | interpretive | CHANG2 resume |
| 27,3637  | interpretive | CHANG2 resume |
| 27,3641 ENDTFF | interpretive | GOTO target |
| 27,3644 DUMPTFF2 | interpretive | CHANG2 resume |
| 27,3645 NEGTFF | interpretive | BPL target |
| 27,3646  | interpretive | CHANG2 resume |
| 27,3650 MAXTFF1 | interpretive | BMN target |
| 27,3651 MAXTFF | interpretive | BOV target |
| 27,3653 TFFELL | interpretive | BOV target |
| 27,3654  | interpretive | CHANG2 resume |
| 27,3655 TFFEL1 | interpretive | BMN target |
| 27,3660  | interpretive | CHANG2 resume |
| 27,3661  | interpretive | CHANG2 resume |
| 27,3663  | interpretive | CHANG2 resume |
| 27,3666  | interpretive | CHANG2 resume |
| 27,3670  | interpretive | CHANG2 resume |
| 27,3673  | interpretive | CHANG2 resume |
| 27,3674  | interpretive | CHANG2 resume |
| 27,3675  | basic | pending DANZIG after RTB T(X) |
| 27,3677  | interpretive | after RTB T(X) |
| 27,3701  | interpretive | CHANG2 resume |
| 27,3702  | interpretive | CHANG2 resume |
| 27,3705  | interpretive | CHANG2 resume |
| 27,3707  | interpretive | CHANG2 resume |
| 27,3711  | interpretive | CHANG2 resume |
| 27,3714  | interpretive | CHANG2 resume |
| 27,3717  | interpretive | CHANG2 resume |
| 27,3721  | interpretive | CHANG2 resume |
| 27,3723  | interpretive | CHANG2 resume |
| 27,3724  | interpretive | CHANG2 resume |
| 27,3727  | interpretive | CHANG2 resume |
| 27,3732  | interpretive | CHANG2 resume |
| 27,3735 T(X) | basic | RTB target |
| 27,3753 ENDT(X) | basic | return from POLY |

The two RTB T(X) operand words are routines keyed at DANZIG (`pending_danzig`): T(X)'s TC DANZIG comes back with DMP still in EDOP and LOC at the word. The program puts the mission's DANZIG marker first among its interpretive tables (`tests/lm_tff_abi_target.c`). A BOVB TCDANZIG (on overflow) runs ENDT(X)'s TC DANZIG natively (`lm_integration_tcdanzig`).

Destinations outside the file: BOVB SIGNMPAC, CHANG2, DANZIG, POLY.

## Domain

Canonical 2048-word image (every word random, 010-061 included) under any EB, FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. `NEWJOB` is +0 or, in a quarter of the cases, positive, so any `DANZIG` goes to `CHANG2`, where the routine stops; every interpretive word that can follow a `DANZIG` is a `CHANG2` resume key of its own. Interpretive routines are entered at NEWOPS with FIXLOC a VAC area.

## Evidence

Hashes: `evidence/lm-tff-abi/SHA256.json`; all files are in the tree.

- **Source map.** `lm-tff-abi-source-map.json` lists all 266 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make lm-tff-abi-differential` runs 256 states per routine (31,488 cases) through the original Luminary099 rope in yaAGC (pinned yaAGC, its ScalerCounter held at 0) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state. The cases and the stops reached per routine are in `lm-tff-abi-proof.json` (`lm-tff-abi-differential.log` is the run). One case in 32 is frozen in `tests/lm_tff_abi_vectors.h`.
- **Host and sanitizer results.** `make test-lm-tff-abi sanitize-lm-tff-abi` runs the 984 frozen cases (`lm-tff-abi-host.log`).
- **C64 execution results.** `make vice-lm-tff-abi` runs 11 diagnostic programs with 196 cases (`lm-tff-abi-vice.log`, which starts with each PRG's SHA-256). The C64 cases take every control-flow edge the 31,488 differential cases take: 2,332 of 2,332 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 2 of 2 compiled labels; with gcov -b, 1,293 of 1,293 branch outcomes (`c64_gcov_branches` in the proof).
