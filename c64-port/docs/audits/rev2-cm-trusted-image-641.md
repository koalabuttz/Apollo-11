# Second review: CM trusted_image batch (#641)

Second certificate reviewer, 2026-10-18. The Certificate fixes lane's #641 (merged as cf0f690b) sets `trusted_image` on `measurement-inc`, `lat-long`, `gimbal-lock` and `sband-antenna` and refreezes them. This record covers the four Comanche055 certificates: MEASUREMENT_INCORPORATION, LATITUDE_LONGITUDE_SUBROUTINES, GIMBAL_LOCK_AVOIDANCE and S-BAND_ANTENNA_FOR_CM. Reviewer 1 re-checks the Luminary099 ones. **Verdict: accepted and re-cited.** The previous records are [measurement-inc.md](measurement-inc.md), [a2-204.md](a2-204.md) and [lane-d1-208.md](lane-d1-208.md).

## Code

- **Change.** In each of the eight generated files, the only source change wraps `sane()`'s whole-image scan in `#if !RK_FIXED_IMAGE`. Each spec gains `'trusted_image': True`. Host and diagnostic builds keep the scan. This is the same pattern as cm-r31 and cm-tpi-search (#630, [rev2-cm-r31-tpi-630.md](rev2-cm-r31-tpi-630.md)).
- **Domain.** Unchanged.

## Evidence (at cf0f690b)

The cases, cases_sha256, oracle_sha256 and frozen vectors are byte-identical to the previous freeze for all four specs. I re-ran the host tests locally, and every C64 row set takes every host branch arc.

| Spec | Cases | Host (re-run) | VICE | Arcs host = C64 |
|---|---|---|---|---|
| measurement-inc | 163,840 | 2,560/2,560 | 18 diagnostics, 354 checks, 0 failures | 2,256 |
| lat-long | 38,400 | 600/600 | 9 diagnostics, 196 checks, 0 failures (rows 197 → 196) | 1,621 |
| gimbal-lock | 10,240 | 325/325 | 2 diagnostics, 36 checks, 0 failures | 621 |
| sband-antenna | 49,664 | 1,599/1,599 | 9 diagnostics, 224 checks, 0 failures | 1,293 |

The arc totals rose (for example gimbal-lock went from 488 to 621) with the same linked sources. That is because they were re-measured against the current shared interpreter files. C64 still equals host. `evidence.py fetch` matches every hash, and `cert_precheck` gives 0 FAIL.

## Notes (non-blocking)

1. As in #630, gimbal-lock, lat-long and sband-antenna still lack `index_carry16` and `image_w` (precheck WARN). Without `index_carry16`, a negative INDEX refuses on the C64 instead of computing a wrong value. Set both keys at the next refreeze.
