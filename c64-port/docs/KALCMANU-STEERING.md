# Translate KALCMANU_STEERING

Lane D1b (alignment and maneuvers). Milestone `kalcmanu-steering`. How the routines are made and checked: `docs/INTERP-COMPILE.md`.

`Luminary099/KALCMANU_STEERING.agc` (`src/lm_kalcmanu_steering.c`) and `Comanche055/KALCMANU_STEERING.agc` (`src/cm_kalcmanu_steering.c`) are native C in full. These are the once-a-second steering jobs and tasks of an automatic attitude maneuver. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file.

## Scope

| Luminary099 | Source | Leaves at |
|---|---|---|
| 22,3003 NEWDELHI, 22,3005 | BANKCALL ISITAUTO, CCS A | ISITAUTO, NOGO |
| 22,3007 NEWANGL, 22,3010, 22,3015, 22,3026 | new attitude matrix (MXM3), DCMTOCDU, RTB V1STO2S, BONCLR | MXM3, DCMTOCDU, MANUSTAT, EXIT |
| 22,3034 (INCRDCDU loop), 22,3065, 22,3073 | CDU increments, TIMECHK (with its INCR Q returns), MANUSTAL | WAITLIST, ENDOFJOB |
| 22,3122 MANUSTAT, 22,3123 | maneuver start: TM, RATEBIAS loop | WAITLIST |
| 22,3203, 22,3206 UPDTCALL, 22,3212 | the update task and its job | FINDVAC, TASKOVER |
| 22,3213 MANUSTOP, 22,3234 GOODMANU, 22,3241 | maneuver end | SPVAC, the caller |

| Comanche055 | Source | Leaves at |
|---|---|---|
| 22,3026 NEWDELHI, 22,3031 NEWANGL, 22,3032, 22,3037, 22,3050 | as Luminary | MXM3, DCMTOCDU, MANUSTAT, EXIT |
| 22,3056 (INCRDCDU-1), 22,3105, 22,3113 | CDU increments, TIMECHK, POSTJUMP CONTMANU | POSTJUMP, WAITLIST |
| 20,2065 MANUSTAT, 20,2066, 20,2131 CONTMANU, 20,2144 | maneuver start, RATEBIAS, the next update | POSTJUMP, WAITLIST |
| 20,2150 UPDTCALL, 20,2154, 20,2155 ZEROEROR | the update job, error reset | FINDVAC, the caller |
| 21,2000 MANUSTOP, 21,2001, 21,2003 ENDROLL, 21,2006 ENDMANU, 21,2013 | maneuver end | IBNKCALL, SPVAC, the caller |
| 22,3142 LOADCDUD, 22,3144 LOADYZ, 02,2526 STOPRATE | the desired-CDU and rate loads | the caller |

Destinations outside the file (BANKCALL, IBNKCALL, POSTJUMP, WAITLIST, FINDVAC, SPVAC, TASKOVER, ENDOFJOB, MXM3, DCMTOCDU) stop with `EXEC_UNMAPPED` until their files have routines.

## Interface for the CM flight lane

KALCMANU's entries are the routines above, keyed by physical address in `cm_kalcmanu_steering_basic` and `cm_kalcmanu_steering_interpretive` (and the `lm_` tables). The steering state is the original erasable (BCDU, NCDU, DELDCDU, CDUXD, TM, NEXTIME, OMEGAPD, DELPEROR), all in E6, the file's `EBANK=`. A caller enters with the image as the AGC holds it at that address, with EB = 6 for basic entries.

## Evidence

Hashes: `evidence/kalcmanu-steering/SHA256.json`. Bulk files are assets of the `evidence-kalcmanu-steering` release.

- **Source map.** `kalcmanu-steering-source-map.json` lists every emitted word with the routine that compiles it. Luminary has 159 words (155 in routines, and the four constants basic instructions read: DT/TAU, ONESEK, BIASCALE). Comanche has 179 words, all in routines.
- **Independent assembly comparison.** `make kalcmanu-steering-differential`: 512 states per routine and program (26,624 cases) equal yaAGC in Z, all 2,048 erasable words and the interrupt-enable state. The cases run under EB 6, and NEWDELHI's return from ISITAUTO has A = +0 or 020000 (CHAN31 & BIT14), as ISITAUTO leaves it. A quarter of the cases put edge values into MPAC and every operand the file names: +0 and -0, one LSB, 1/2, 1/4, just below 1, half-unit axis vectors (with a matrix's rows as a signed permutation, so identity and 90° rotations occur), unit vectors in a coordinate plane, and tiny vectors. In those cases the CDUs also sit at 0, 45, 90, 180 or 270°. TIME1, TIME2 and the CDUs vary in every case. Half of the cases also get one joint state: TM just before, at and past TIME2/TIME1, NEXTIME around TIME1's wrap, BCDU and NCDU a few bits apart, BRATE at ±0 and small values, or (Comanche) HOLDFLAG and RATEINDX at each value the code tests. The stops reached per routine are in `kalcmanu-steering-proof.json`. The frozen host vectors are every 32nd case plus a greedy cover of every branch outcome (clang edge coverage over all linked sources) that any case reaches, 840 in all; `outcome_cover` in the proof lists the counts (754 edges, all on host and C64). The basic routines that never name FIXLOC or enter the interpreter (STOPRATE, which tasks and NOVAC jobs such as V37 call, among them) take any FIXLOC: half their cases put FIXLOC outside the VAC areas (spec `any_fixloc`). Those routines also run as waitlist tasks over any job (UPDTCALL, MANUSTOP and GOODMANU, which TC WAITLIST and the 2CADR UPDTCALL start; in the LM P63 flight run they arrive with NEWJOB -0 while DUMMYJOB idles), so a quarter of their cases have NEWJOB -0 (spec `any_newjob`). `trusted_image` drops the entry checks' image scan in flight builds only.
- **Resume points.** Resume points: a DANZIG that finds NEWJOB positive goes to CHANG2 and the job resumes later at the next word, so with `resume_keys` every word after a DANZIG is a routine of its own (the routine counts include them). A quarter of the cases at those routines have NEWJOB positive, and the run stops at CHANG2 as the original does. Each resume point starts from the MODE, PUSHLOC and index registers the code before it leaves there (per-point `states`). Runs may take up to 1,000 routines (`max_steps`).
- **Unreachable arms.** TIMECHK's `CCS TTEMP` / `CCS TTEMP +1` +0 arms, and CONTMANU's `CCS A` +0 arm, need TIME2/TIME1 to hold -0. They never do: the counters only change by PINC, which wraps 037777 to +0, or by software writes of non-negative times. The overflow skips of the file's `TS` stores are unreachable too: A at each of them comes from a load, CCS or COM and never holds overflow. No cases put -0 in TIME1/TIME2.
- **Host and sanitizer results.** `make test-kalcmanu-steering sanitize-kalcmanu-steering` runs the 840 frozen cases (`kalcmanu-steering-host.log`).
- **C64 execution results.** `make vice-kalcmanu-steering` runs two diagnostics, one per program, with 78 cases picked to reach every branch outcome the host vectors reach (`kalcmanu-steering-vice.log`, which records each PRG's sha256).
