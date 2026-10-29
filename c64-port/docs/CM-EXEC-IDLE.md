# Run the Comanche idle loop, SUPDXCHZ and SVCT3 as native routines

Lane A. Milestone `cm-exec-idle`. Interface: `docs/NATIVE_TRANSFER_ABI.md`. Follows `docs/CM-EXEC-JOBS.md`.

When a job switch finds nothing ready, `cm-exec-jobs` leaves Z at ADVAN 01,3231. This milestone makes that address, and the other places programs enter the idle loop, routines of the control-transfer ABI (`cm_exec_idle_routines`, `src/cm_exec_idle.c`):
- DUMMYJOB 01,3223, and DUMMYJOB+2 01,3225, which the restart enters through its CADR;
- ADVAN 01,3231;
- NUCHANG2 01,3032;
- SUPDXCHZ 05227 and SUPDXCHZ+1 05230, used by PINBALL, P20 and the idle loop itself;
- SVCT3 05306, the task ENDTASK's 2CADR names, with its two resumptions: 05323 after its NOVAC, and 07721 where SVCT3X's FIXDELAY returns.

Each routine composes the qualified image body of its source path and leaves Z where the original continues. That is one of:
- the selected basic job (NUDIRECT, with COMP ACTY back on in `io->channel11`);
- NEWOPS 06037, for an interpretive job;
- the self-check at SELFRET in bank 033 with SUPERBNK 0100, when NEWJOB is -0;
- the SUPDXCHZ target 2CADR;
- TASKOVER 05324, NOVAC 05134 (A = priority 35, Q at the NBDONLY 2CADR) or FIXDELAY 05267 (Q at DEC 500), from SVCT3.

NOVAC and FIXDELAY are `cm-exec-services` routines whose site table already holds SVCT3's operands, so a program that registers both tables runs the task through.

## Source map

| AGC | Native |
|---|---|
| DUMMYJOB 01,3223 (CS ZERO, TS NEWJOB) and DUMMYJOB+2 01,3225 (RELINT, CS TWO, WAND DSALMOUT) | `dummyjob`: those effects, then `cm_idle_dispatch` from ADVAN |
| ADVAN 01,3231 through NUDIRECT 01,3242 or the SELFRET dispatch | `advan` → `cm_idle_dispatch` (`docs/COMANCHE_IDLE_DISPATCH.md`) |
| NUCHANG2 01,3032 (INHINT, CCS NEWJOB, CHANJOB or ADVAN+2) | `nuchang2` → `cm_idle_recheck` |
| SUPDXCHZ 05227, +1 05230 | `supdxchz` → `cm_super_dispatch` |
| SVCT3 05306, 05323, SVCT3X+2 07721 | `svct3` → `cm_svct3_phase` 0, 1, 2 |

The A register DUMMYJOB leaves (-2 AND channel 11) is not kept, because every ADVAN tail loads A again; the comparison below covers it. NUDIRECT and ADVAN+2 are not entries: only ADVAN and NUCHANG2 reach them. The GOTOV56 caller of SUPDXCHZ at 04622 belongs to P20-P25 and is not registered here.

The input domains are those of the bodies. Anything outside them is refused (`EXEC_OUT_OF_SCOPE`), with the image and io unchanged. NUCHANG2 and DUMMYJOB put back the Z, NEWJOB, interrupt and channel 11 values they set before a refusing ADVAN tail.

## Evidence

Hashes: `evidence/cm-exec-idle/SHA256.json`.

- **Independent assembly comparison.** `make cm-exec-idle-differential` runs the unmodified Comanche055 rope in yaAGC (host-only, `tools/cm_exec_idle_reference.c`). The cases are drawn from each body's own qualified input generator:
  - ADVAN, 1,024 from `cm_idle_dispatch` (NEWJOB +0, a basic job, an interpretive job, -0);
  - NUCHANG2, 1,024 from `cm_idle_recheck`;
  - DUMMYJOB and DUMMYJOB+2, 512 from `cm_idle_dispatch` with SELFRET anywhere in the self-check bank and channel 11 varied;
  - SUPDXCHZ and SUPDXCHZ+1, 1,024 from `cm_super_dispatch`;
  - SVCT3, all 1,536 `cm_svct3` cases (512 per entry).

  The native routine runs once. The original then runs from the same entry until it first reaches the continuation the native run left in Z (and FB, in banked memory), or BAILOUT/POODOO. Every case equals the original in its outcome and all 2,048 erasable words, SUPERBNK, the interrupt-enable bit and channel 11.

  Results: all 5,120 cases continue. They split as 2,340 to a job or SUPDXCHZ target, 661 to NEWOPS, 582 to the self-check, 1,025 to TASKOVER, 64 to NOVAC and 448 to FIXDELAY. Per-case digests are frozen into `tests/cm_exec_idle_vectors.h`; see `evidence/cm-exec-idle/cm-exec-idle-proof.json`.
- **Host and sanitizer results.** `make test-cm-exec-idle sanitize-cm-exec-idle` (`evidence/cm-exec-idle/cm-exec-idle-host.log`) checks:
  - the digests;
  - that all nine entries and every kind of continuation are reached;
  - five refusals with the image unchanged.
- **C64 execution results.** `make vice-cm-exec-idle` (`evidence/cm-exec-idle/cm-exec-idle-vice.log`) runs every sixteenth case (320).
