# Run the Luminary idle loop, SUPDXCHZ and SVCT3 as native routines

Lane A. Milestone `lm-exec-idle`. Interface: `docs/NATIVE_TRANSFER_ABI.md`. Follows `docs/LM-EXEC-JOBS.md`; Comanche counterpart `docs/CM-EXEC-IDLE.md`.

When a job switch finds nothing ready, `lm-exec-jobs` leaves Z at ADVAN 01,3214. This milestone makes that address, and the other places programs enter the idle loop, routines of the control-transfer ABI (`lm_exec_idle_routines`, `src/lm_exec_idle.c`):
- DUMMYJOB 01,3206, and DUMMYJOB+2 01,3210, which the restart enters through its CADR;
- ADVAN 01,3214;
- NUCHANG2 01,3011;
- SUPDXCHZ 05165 and SUPDXCHZ+1 05166, used by PINBALL, V82, P20's GOTOV56 and the idle loop itself;
- SVCT3 05240, the task ENDTASK's -2CADR names, with its two resumptions: 05255 after its NOVAC, and 05260 where SVCT3X's FIXDELAY returns.

Each routine leaves Z where the original continues. That is one of:
- the selected basic job (NUDIRECT, with COMP ACTY back on in `program->hardware->channel11`);
- NEWOPS 06070, for an interpretive job;
- the self-check at SELFRET in bank 033 with SUPERBNK 0100, when NEWJOB is -0;
- the SUPDXCHZ target 2CADR;
- TASKOVER 05261, NOVAC 05072 (A = PRIO35, Q at the NBDONLY 2CADR) or FIXDELAY 05221 (Q at DEC 500), from SVCT3.

NOVAC and FIXDELAY are `lm-exec-services` and `lm-exec-jobs` routines whose site table already holds SVCT3's operands, so a program that registers those tables runs the task through. The idle entries need `program->hardware`.

## Source map

| AGC | Native |
|---|---|
| DUMMYJOB 01,3206 (CS ZERO, TS NEWJOB) and DUMMYJOB+2 01,3210 (RELINT, CS TWO, WAND DSALMOUT) | `dummyjob`: those effects, then `lm_idle_dispatch` from ADVAN |
| ADVAN 01,3214 through NUDIRECT 01,3225 or the SELFRET dispatch | `advan` → `lm_idle_dispatch` (ADVAN entry) |
| NUCHANG2 01,3011 (INHINT, CCS NEWJOB, CHANJOB or ADVAN+2) | `nuchang2` → `lm_idle_dispatch` (NUCHANG2 entry) |
| SUPDXCHZ 05165, +1 05166 | `supdxchz` → `lm_super_dispatch` (`docs/LM-SUPER-DISPATCH.md`) |
| SVCT3 05240, CKIMUSE 05244, +3 05251, 05255, SVCT3X 05256, 05260 | `svct3` → `lm_svct3_phase` 0, 1, 2 (new in this milestone) |

`lm_svct3_phase` (`src/lm_svct3.c`) is new: CCS FLAGWRD2 (076) ends a positive or +0 drift flag at TASKOVER; otherwise CKIMUSE's CCS IMUCADR (01304) calls NOVAC for NBDONLY when the IMU is free (+0) and FIXDELAY 500 otherwise. A holds each CCS's diminished absolute value, then PRIO35 (035000) before NOVAC. It requires the bank the Waitlist dispatches ENDTASK's -2CADR with (BB 04003, EB 01400) and refuses anything else with the image unchanged.

The A register DUMMYJOB leaves (-2 AND channel 11) is not kept, because every ADVAN tail loads A again; the comparison below covers it. NUDIRECT and ADVAN+2 are not entries: only ADVAN and NUCHANG2 reach them. The input domains are those of the bodies. Anything outside them is refused (`EXEC_OUT_OF_SCOPE`), with the image, io and hardware unchanged; DUMMYJOB puts back the Z, NEWJOB, interrupt and channel 11 values it set before a refusing ADVAN tail.

## Evidence

Hashes: `evidence/lm-exec-idle/SHA256.json`.

- **Independent assembly comparison.** `make lm-exec-idle-differential` runs the unmodified Luminary099 rope in yaAGC (host-only, `tools/lm_exec_idle_reference.c`). The cases are:
  - ADVAN, 1,024 of `lm_idle_dispatch`'s ADVAN states (NEWJOB +0, a basic job, an interpretive job, -0);
  - NUCHANG2, 768 of its NUCHANG2 states;
  - DUMMYJOB and DUMMYJOB+2, 512 over the ADVAN states with SELFRET anywhere in the self-check bank and channel 11 varied;
  - SUPDXCHZ and SUPDXCHZ+1, 1,024 from `lm_super_dispatch`. A SUPDXCHZ case whose target is SUPDXCHZ+1 is replaced by the next of its kind, because the comparison could not tell that target from SUPDXCHZ's own next instruction;
  - SVCT3, 1,536 (512 per entry, every CCS class of FLAGWRD2 and IMUCADR), run inside the T3 interrupt as the Waitlist runs it.

  The native routine runs once. The original then runs from the same entry until it first reaches the continuation the native run left in Z (and FB, in banked memory), or BAILOUT1/POODOO1. Every case equals the original in its outcome and all 2,048 erasable words, SUPERBNK, the interrupt-enable bit, channel 11 and InIsr.

  Results: all 4,864 cases continue. They split as 1,981 to a job or SUPDXCHZ target, 768 to NEWOPS, 579 to the self-check, 1,024 to TASKOVER, 64 to NOVAC and 448 to FIXDELAY. Per-case digests are frozen into `tests/lm_exec_idle_vectors.h`; see `evidence/lm-exec-idle/lm-exec-idle-proof.json`.
- **Host and sanitizer results.** `make test-lm-exec-idle sanitize-lm-exec-idle` (`evidence/lm-exec-idle/lm-exec-idle-host.log`) checks:
  - the digests;
  - that all nine entries and every kind of continuation are reached;
  - five refusals with the image unchanged.
- **C64 execution results.** `make vice-lm-exec-idle` (`evidence/lm-exec-idle/lm-exec-idle-vice.log`) runs every sixteenth case (304).
