# Auditor review: Luminary099/IMU_COMPENSATION_PACKAGE.agc

Reviewed 2026-10-12 by the port auditor against the certificate drafted by
milestone `imu-compensation` (PR #35, bfb60480).

Verdict: **accepted** for the Luminary099 file, once the `hardware`
initializer fix in PR #46 lets the milestone's targets build. The Comanche055
file from the same milestone is not accepted yet (issue #47).

## What was checked

- `independent_assembly_comparison` is a yaAGC run;
  `make imu-compensation-differential` rebuilds proof.json byte for byte
  (sha256 014c002a…, native snapshot 3c6bb822…). The source map regenerates
  byte-identical. All evidence hashes verify.
- Tracing every Z yaAGC executed over all 13,824 cases, every instruction
  word of the LM file ran except the impossible NOOP at 06,3327; the rest
  never executed are data words.
- gcov over the same cases: 99.4% of lines, 91.9% of branches; untaken
  branches have no producer (-0 GCOMPSW, -0 index, the TEM1 refusal tested
  separately).
- CADRs at 06,3534, 3536, 3574 and 3664 are in `lm_interbank_sites`;
  2CADR 1/GYRO (06,3350) is in `lm_exec_sites`. 1/PIPA, GCOMPZER, NBDONLY
  and LASTBIAS are registered routines.

## Reproduced (with hardware = 0, as in PR #46)

- `test-` and `sanitize-imu-compensation`: 13824 cases, 38450 checks,
  0 failures.
- `imu-compensation-differential`: 13824 cases equal the original programs.
- VICE x64sc 3.7.1 PAL: imu-compensation.prg 277 checks, lm-trips 32,
  cm-trips 64, 0 failures.

## Open notes for the lane (not blocking)

- Map the erasable `#define`s (BUF, MPAC, GCOMP, 1/PIPADT…) in the source map.
- `supported_domain`: each routine runs to its exit without interrupts
  (the oracle zeroes ScalerCounter).
- NOVAC round trips run on the host only; add the file to the "Applied so
  far" table in `docs/NATIVE_TRANSFER_ABI.md`.
