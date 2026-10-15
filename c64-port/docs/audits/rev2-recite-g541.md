# Second review: re-cite 12 CM certificates after #541

Second certificate reviewer, 2026-10-15. Lane G's #541 (merged as d419f887) refroze 12 Comanche055 specs, adding `trusted_image` everywhere and `image_w` / `index_carry16` where they were missing. **Verdict: all 12 re-cited.**

## Code

- **Changes.** The changes match the ones accepted for cm-p30-p37 (rev2-recite-p30-p33.md). Across the 12 files there are 21 image-scan guards under `!RK_FIXED_IMAGE`, 11 `agc_image_w.h` includes and 10 exact `ixw` end-around carries. Nothing else changes, and every remaining whole-image scan is guarded.
- **Certificates.** Only the proof and VICE-log hashes change, plus cm-p40-p47's vice_targets, which go from 646 to 644. No domain or scope text changes.
- **Shared tools.** None changed (interp_specs.py only inside these 12 entries).
- **P40-P47 646 → 644 rows.** The measured branch outcomes rose: host edges went from 4,603 to 4,638 and gcov from 2,517 to 2,726. This let 15 cases that were too big for one PRG fit, and the row set was rebuilt. Four rows were dropped and two added, and three 1-case PRGs with the same KEEP mask were merged (49 → 48). The C64 still takes every host outcome: 0 host-only edges, gcov 2,726/2,726.

## Evidence at d419f887 (reproduced)

| Spec | File | Prior review | VICE PRGs: checks = vice_targets |
|---|---|---|---|
| cm-auto-man | AUTOMATIC_MANEUVERS | lane-g-258 | 1: 24 |
| cm-jet-select | JET_SELECTION_LOGIC | lane-g-258 | 1: 27 |
| cm-tvc-massprop | TVCMASSPROP | lane-g-258 | 1: 7 |
| cm-tvc-exec | TVCEXECUTIVE | lane-g-258 | 1: 25 |
| cm-tvc-restarts | TVCRESTARTS | lane-g-258 | 1: 19 |
| cm-tvc-stroke | TVCSTROKETEST | lane-g-258 | 1: 13 |
| cm-tvc-init | TVCINITIALIZE | lane-g-359 | 1: 15 |
| cm-tvc-roll | TVCROLLDAP | lane-g-258 | 1: 16 |
| cm-tvc-daps | TVCDAPS | lane-g-258 | 1: 18 |
| cm-p40-p47 | P40-P47 | lane-g-107 | 48: 644 |
| cm-stable-orbit | STABLE_ORBIT | lane-g-258 | 3: 139 |
| cm-ephemeris | LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES | lane-g-277 | 2: 38 |

The following hold for all 12 specs:
- All evidence files match SHA256.json and the certificates.
- The differentials equal yaAGC at the frozen `--cases`, and the case sets equal the proofs.
- Host and sanitizer counts equal the logs.
- All 68 PRGs are byte-identical to the logs, with 0 VICE failures.
- `--cover-check` reports verified with 0 missing, and `cert_selfcheck` reports OK with no gaps.
- `cert_precheck` reports 0 FAIL. The 11 WARNs are 9 NEWJOB refusals and P40's rtb_angles measurement, both unchanged since earlier freezes, plus the uncached vice manifest.

**NOTE (not blocking).** The cm-jet-select spec now sets `'trusted_image'` twice, both True.
