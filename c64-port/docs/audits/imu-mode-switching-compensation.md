# Certificate review: IMU_MODE_SWITCHING_ROUTINES (LM, CM) and Comanche055 IMU_COMPENSATION_PACKAGE

Reviewed 2026-10-13 by certificate reviewer 1 at `port/c64-native-c` a2aabc1f
(after Lane D1 PR #187). The certificates come from milestones
`imu-mode-switching` and `imu-compensation`. This is a re-review of #65 and
of #47 item 2.

Verdict: **accepted** for Luminary099 and Comanche055
IMU_MODE_SWITCHING_ROUTINES and Comanche055 IMU_COMPENSATION_PACKAGE. The
existing Luminary099 IMU_COMPENSATION_PACKAGE certificate still holds with the
reworked evidence.

## Reproduced

- **Differentials.** `imu-mode-switching-differential` gives 64,256 cases equal
  to yaAGC, and `imu-compensation-differential` gives 14,080. The vectors and
  C64 vectors show no git diff, and the proofs and source maps are byte-equal.
- **Host and sanitizer.** 128,990 and 38,966 checks, with 0 failures.
- **C64.** All 12 PRG sha256 values equal fresh builds. VICE gives 1,288 and 381
  checks, with 0 failures, and these equal `vice_targets`.
- **Independent gcov `-b`.** This was run over every linked source, over all host
  cases and over exactly the committed C64 rows.
  - Mode switching: 0 host-only outcomes. LM 477/477, CM 483/483.
  - Compensation: no host-only outcome in `imu_compensation.c` or the INTER-BANK
    files. The 86 (LM) and 102 (CM) host-only outcomes are all in Executive
    sources, reached only by the host round trips through NOVAC. The trips PRGs
    build with `-DICP_NO_EXEC`. The Executive belongs to the Kernel milestones,
    which certify it, and the lane lists these outcomes exactly as
    `executive_round_trips_only`.
- **#65.**
  - The COARS2 FIXDELAY exit, both directions of GSELECT's skip, STRTGYR2 −0
    and 8192AUG −0 are all compared.
  - STRTGYR2 +0 is impossible: x + (−1) is never +0 in ones' complement. The lane
    is right about this.
  - R02ZERO is registered in both missions.
- **#47 item 2.** The CM 1/GYRO 2CADR is registered, and the round trips run
  through native NOVAC.
  - BAILOUT is an honest stop for the job-table-full (1202) case.
  - Every cross-bank label and 2CADR is a registered routine at its listing
    address.
  - The 37 sampled refusals are defensive.

## Notes for the lane (not blocking)

- **Cover drift.** The `*-c64-cover.json` records no longer regenerate byte for
  byte, because the shared INTER-BANK sources changed. Both covers re-derive with
  0 missing outcomes, which is drift under the steward's policy.
  - Re-freeze at the next touch.
  - Record the source sha256 values in the proof and cover JSON.
  - Add the `check` key to the IMS `c64_branch_outcomes` ledger entry.
- **CM certificate wording.** State in the CM compensation certificate that the
  C64 does not run the Executive round trips.
- **Stale docs.** The counts in docs/milestones/imu-mode-switching.md and
  imu-compensation.md are stale.
- **JOBWAKE not-found.** Name the JOBWAKE not-found refusal (c3702) in
  `supported_domain`, or compare it.
- **LM NBD2 edge.** Pin the elapsed −0 edge in LM NBD2 (TIME1 = 1/PIPADT).
- **#65 domain notes.** Name the device-adapter obligation and the T4RUPT
  entry state in `supported_domain`.
