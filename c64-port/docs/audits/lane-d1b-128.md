# Certificate review: Lane D1b certificates after PR #128

Reviewed 2026-10-13 by the second certificate reviewer at `port/c64-native-c`
94935917 (PR #128 merged at d87a3d57). This is a re-review after issues #117
and #118.

| File | Milestone | Verdict |
|---|---|---|
| Luminary099/KALCMANU_STEERING | `kalcmanu-steering` | accepted |
| Comanche055/KALCMANU_STEERING | `kalcmanu-steering` | accepted |
| Luminary099/INFLIGHT_ALIGNMENT_ROUTINES | `inflight-align` | accepted |
| Comanche055/INFLIGHT_ALIGNMENT_ROUTINES | `inflight-align` | accepted |
| Luminary099/LEM_GEOMETRY | `geometry` | accepted |
| Luminary099/ATTITUDE_MANEUVER_ROUTINE | `attitude-maneuver` | **not accepted** (#172) |
| Comanche055/ANGLFIND | `attitude-maneuver` | **not accepted** (#172) |

R60_62 (both missions) and CSM_GEOMETRY stay partial. They have no
certificate.

## What was checked

- `tools/cert_precheck.py` gives 0 FAIL on all seven files once the release
  evidence is fetched. Every `source_revision` is 911e5c02. The four evidence
  categories cite four distinct records, and none cites the source map.
- **yaAGC comparison, rerun by the reviewer.** I ran
  `tools/interp_routine_differential.py <spec> --cases 512` on my own build of
  the pinned reference. It passed for `kalcmanu-steering` (26,624 cases),
  `inflight-align` (92,160), `geometry` (49,664), `attitude-maneuver`
  (235,520) and `r60-r62`, with no mismatch and no oracle timeout. The
  regenerated case sets hash to the values in each proof: 47dc4fb5, 7b36174c,
  1f9c5681 and 03112a0b. The stale r60-r62 `cases_sha256` and the LEM_GEOMETRY
  source-map citation the auditor reported on 416b2939 are gone.
- **Host and sanitizer.** `make test-<m> sanitize-<m>` gives kalcmanu-steering
  839, inflight-align 792 and geometry 426 checks, with 0 failures each.
- **VICE.** The fetched logs record each PRG sha256. Their totals equal
  `vice_targets`: kalcmanu-steering 78, inflight-align 224, geometry 136 and
  attitude-maneuver 550, all with 0 failures.
- **Independent C64 branch check (all cases, per object).** I regenerated every
  differential case with the spec's generator. I then matched each C64 row in
  `tests/<m>_c64_vectors.h` to its case by edit list, and all rows matched. I
  built the native runner at `-O0 --coverage` over every linked source and ran
  `gcov -b` per object. The CM objects' include of `lm_interp_orders.c` is
  counted on its own. Every branch outcome the host cases take is also taken
  by a C64 row: 0 missing for kalcmanu-steering, inflight-align and geometry.
  attitude-maneuver misses 6, all of them NEWJOB-positive CHANG2 arms (#172).

## Prior blockers (#117, #118)

- **TIME1/TIME2 and CDUs** vary in every case. TM is placed before, at and past
  TIME2/TIME1, and NEXTIME around TIME1's wrap. TIMECHK's end-of-maneuver path
  (`CCS TTEMP` −0 arm, then `CCS TTEMP +1`) runs in 250 LM cases and is
  compared. The +0 arms need TIME2/TIME1 to hold −0, which PINC and software
  never store; this is argued in `docs/KALCMANU-STEERING.md`. My gcov confirms
  that these arms and the TS overflow skips are the only unexecuted steering
  code, apart from inline copies of return points that have their own routines
  (for example 21,2003 ENDROLL).
- **±0 arms** (BRATE, HOLDFLAG, RATEINDX, NEXTIME = TIME1, small-angle
  INCRDCDU) come from the joint-state families and are taken on host and C64.
- **ISITAUTO** returns +0 or 020000 (`tools/interp_specs.py`, the doc).
- **CHANG2 resume LOCs.** `resume_keys` makes every word after a DANZIG a
  routine, and NEWJOB-positive cases go to CHANG2 through the trampoline.
- **Edge values** (±0, one LSB, half-unit axes, signed-permutation matrices,
  coordinate-plane and tiny vectors, CDUs at 0/45/90/180/270°) are in a
  quarter of the cases. My gcov finds the interpreter's SQRTSUB negative-input
  path unexecuted. Its only caller here is UNIT, whose argument is a sum of
  squares and never negative.
- **ARCTRIG** is in LEM_GEOMETRY's `external_continuations`.
- **C64 rows** are chosen by trace-pc-guard edge cover over every linked
  source (`<m>-c64-outcomes.json`), and the independent gcov check agrees.
