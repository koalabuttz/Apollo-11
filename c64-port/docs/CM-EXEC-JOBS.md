# Run Comanche Executive job services as native routines

Lane A. Milestone `cm-exec-jobs`. Interface: `docs/NATIVE_TRANSFER_ABI.md`. Follows `docs/CM-EXEC-SERVICES.md`; Luminary counterpart `docs/LM-EXEC-JOBS.md`.

The rest of Comanche055 EXECUTIVE.agc's entries are now routines of the control-transfer ABI (`cm_exec_job_routines`, `src/cm_exec_jobs.c`):
- SPVACIN 05154 and SPVAC 05160;
- CHANG1 05164 and CHANG2 05170;
- JOBSLEEP 05175, PRIOCHNG 05210 and ENDOFJOB 05217;
- JOBWAKE 05201.

Each routine composes the qualified image body of its source path and leaves Z where the original continues. That is one of:
- the caller;
- the selected basic job;
- NEWOPS 06037 with LOC and BANKSET set, for an interpretive job;
- ADVAN 01,3231 when nothing is ready, with COMP ACTY off in `io->channel11`;
- JOBWAKE's exceptional source transfers: TC 014 for a +0 priority word, and CCSHOLE 05747;
- BAILOUT 07755 when SPVAC finds no core set or VAC area.

Unlike Luminary's job routines, these need no `program->hardware`: the Comanche bodies keep channel 11 in `cm_native_io`, and they already select interpretive successors.

## Source map

| AGC | Native |
|---|---|
| SPVACIN 05154, SPVAC 05160 | `spvac` → `cm_admission_zero` (`docs/COMANCHE_ADMISSION_ZERO.md`) |
| CHANG1 05164, CHANG2 05170 | `chang1`, `chang2` → `cm_context_change` |
| JOBSLEEP 05175 | `jobsleep` → `cm_basic_sleep` |
| JOBWAKE 05201 | `jobwake` → `cm_wake_complete` |
| PRIOCHNG 05210 | `priochng` → `cm_priority_complete` |
| ENDOFJOB 05217 | `endofjob` → `cm_basic_finish` |

The input domains are those of the bodies. Anything outside them is refused (`EXEC_OUT_OF_SCOPE`), with the image and io unchanged.

## Evidence

Hashes: `evidence/cm-exec-jobs/SHA256.json`.

- **Independent assembly comparison.** `make cm-exec-jobs-differential` runs the unmodified Comanche055 rope in yaAGC (host-only, `tools/cm_exec_jobs_reference.c`). The cases are 1,024 per service group, drawn from each body's own qualified input generator:
  - SPVAC and SPVACIN from `cm_admission_zero`;
  - CHANG1 and CHANG2 from `cm_context_change`;
  - JOBSLEEP from `cm_sleep`;
  - JOBWAKE from `cm_wake_complete`;
  - PRIOCHNG from `cm_priority_complete`;
  - ENDOFJOB from `cm_basic_finish`.

  The native routine runs once. The original then runs from the same entry until it first reaches the continuation the native run left in Z (and FB, in banked memory), or BAILOUT/POODOO. Every case equals the original in its outcome and all 2,048 erasable words, SUPERBNK, the interrupt-enable bit and channel 11. The continuation only picks where to compare, so a wrong continuation shows up as a timeout or a differing image.

  Results: 5,848 continuations and 296 SPVAC alarms. The continuations split as 3,862 to a job or the caller, 1,326 to NEWOPS, 496 to ADVAN, 123 to TC 014 and 41 to CCSHOLE. Per-case digests are frozen into `tests/cm_exec_jobs_vectors.h`; see `evidence/cm-exec-jobs/cm-exec-jobs-proof.json`.

  One kind of input is skipped: a caller return or saved job location that lies inside EXECUTIVE.agc itself (fixed-fixed 05134-05234, or 01,2626-01,3245). No job calls these services from the Executive's own code. The comparison could not tell such a return from the Executive passing that address on its way, so 96 of these generated inputs are replaced by the next input of the same generator.
- **Host and sanitizer results.** `make test-cm-exec-jobs sanitize-cm-exec-jobs` (`evidence/cm-exec-jobs/cm-exec-jobs-host.log`) checks:
  - the digests;
  - that all eight entries and every kind of continuation are reached;
  - four refusals with the image unchanged.
- **C64 execution results.** `make vice-cm-exec-jobs` (`evidence/cm-exec-jobs/cm-exec-jobs-vice.log`) runs every sixteenth case (384).
