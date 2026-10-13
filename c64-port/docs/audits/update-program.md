# Certificate review: Luminary099 and Comanche055 UPDATE_PROGRAM.agc

Reviewed 2026-10-13 by the certificate reviewer at `port/c64-native-c` a90e77ac
(after Lane A2 PR #124), against the certificates drafted by milestones
`lm-update` and `cm-update`. This is the third review; the earlier rejections
are in #93.

Verdict: **accepted** for both files.

## What was checked

- Every item of #93 is fixed:
  - the V72 even count;
  - BZF STORLP71 −0;
  - the CM EBANK overflow;
  - the EB0 targets (LM 13, CM 12);
  - CM UPACTOFF registration;
  - C64 branch-outcome coverage;
  - the stale LM VICE log.
- Transfer ABI. These are registered at their original addresses and exit through the trampoline:
  - the V70-V73 fan;
  - the TIMEDIDR restart 2CADR;
  - CM UPACTOFF;
  - UPOUT+1 and UPJOB;
  - every return point;
  - the new interpretive entries UPJOB CALL INTSTALL and UPWAKE EXIT (LM 04,3552/3554, CM 27,3544/3546).

  The new entries are compared against yaAGC with NEWJOB set. Neither path passes DANZIG, so no other resume keys are needed.

## Reproduced

- Evidence verify and `port_coverage.py` pass.
- Host, sanitize and differential targets give 0 failures, and 4096 of 4096 cases per mission equal yaAGC.
- The proofs and cover JSONs regenerate byte for byte.
- VICE gives LM 261 checks and CM 285 checks, both with 0 failures. Fresh tip PRG sha256 values equal the logs (`8cef3d2f…`, `b597251d…`).
- Independent gcov `-b` over all 11 linked sources per milestone, run over exactly the C64 rows and over the host set, finds no host-only line and no host-only branch outcome. Every registered entry has a C64 row, including both UPACTOFF callers.

## Notes for the lane (not blocking)

- The arc cover keys outcomes by `file:line branch`, so it can't distinguish call sites inside the shared helpers. Per call site, these are compared on the host but never run on the C64:
  - CM MODREG = +0 (P00) at `cm_update.c:101`;
  - the no-overflow DAS results of V70/V73 (`cm_update.c:275, 345-347`);
  - the LM TEPHEM+1 borrow (`lm_update.c:329`);
  - the BZMF −0 boundaries.

  The helper arcs themselves do run on the C64. Add rows for these, or give the cover a site-aware key.
- Add `vice_targets` to both certificates (#127 migration). `cert_precheck` WARNs until then.
- The headers and `supported_domain` call 3552/3554 (3544/3546) "CHANG2 resume LOCs". They are NEWOPS points reached from INTPRET and INTSTALL's RVQ.
- `native_files` omits the interpreter sources the milestones now link.
