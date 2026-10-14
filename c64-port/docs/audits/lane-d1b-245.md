# Certificate review: Lane D1b certificates after PRs #245, #252, #272 and #279

Reviewed 2026-10-13 by the second certificate reviewer.

- P51-P53 was reviewed at `port/c64-native-c` d84ed3f2 (#245 merged). I
  re-ran its host tests at 1816baab. Its own sources and vectors have not
  changed since then: only `p51_p53_rtb.c` swapped its local `W` define for
  `agc_image_w.h`, and `interp_case.c` gained the LM channel harness.
- The rest was reviewed at 1816baab, after #252 (CSM_GEOMETRY), #272
  (rtb-op-codes refrozen with `index_carry16`) and #279 (rtb-op-codes
  fixed-image W).

| File | Milestone | Verdict |
|---|---|---|
| Luminary099/P51-P53 | `p51-p53` | accepted |
| Comanche055/P51-P53 | `p51-p53` | accepted |
| Comanche055/CSM_GEOMETRY | `geometry` | accepted |
| Luminary099/LEM_GEOMETRY | `geometry` | accepted (re-confirmed) |
| Luminary099/RTB_OP_CODES | `rtb-op-codes` | accepted (re-confirmed) |
| Comanche055/RT8_OP_CODES | `rtb-op-codes` | accepted (re-confirmed) |

## What was checked

- `tools/cert_precheck.py` gives 0 FAIL on all six, after
  `tools/evidence.py fetch`. Every `source_revision` is 911e5c02. The
  `index_carry16` WARN on `p51-p53` and `geometry` is covered under "INDEX
  carry" below.
- **yaAGC comparison, rerun by the reviewer.** I ran
  `tools/interp_routine_differential.py <m> --cases N` with each proof's case
  count: 256 for p51-p53, 512 for the others. Each regenerated case set
  hashes to its proof's `cases_sha256`, and the committed vectors came out
  unchanged.

  | Milestone | Cases | Routines | Result |
  |---|---|---|---|
  | p51-p53 | 327,936 | 1,281 | all equal (also all 655,872 at 512 per routine) |
  | geometry | 76,288 | 149 | all equal |
  | rtb-op-codes | 34,816 | 68 | all equal |

- **Host and sanitizer.** `make test-<m> sanitize-<m>` gives 2,174, 658 and
  1,132 checks, with 0 failures each.
- **VICE.** Each log's sha256 equals the certificate's, and its PASS totals
  equal `vice_targets` with 0 failures: 1,522 checks (62 PRGs), 197 (8) and
  144 (3).
- **Independent C64 branch check (every case, per object, per group).** I
  regenerated every case and matched every C64 row by edit list: 1,522, 197
  and 144 rows, none unmatched. p51-p53 is too large for the reviewer's
  in-memory matcher, so it used a streaming variant that keys cases by a hash
  of their edits; the case-set hash still matches the proof. I ran
  per-object gcov `-b` over every linked source, all host cases against the
  rows, with 0 missing in every object. Then I built each C64 group with its
  own KEEP flags and ran only that group's rows (`cov_groups.py`). Every
  branch outcome and line the host cases take is taken by a C64 row:

  | Milestone | Groups | Outcomes | Lines | Missing |
  |---|---|---|---|---|
  | p51-p53 | 62 | 5,658 | 12,259 | 0 |
  | geometry | 8 | 1,607 | 2,986 | 0 |
  | rtb-op-codes | 3 | 874 | 1,745 | 0 |

  This holds in the LM and CM objects of the shared interpreter sources
  alike. For example, `lm_interp_orders.c` takes 389 outcomes in the LM
  object and 358 in the CM object for p51-p53.
- **INDEX carry.** `p51-p53` and `geometry` don't set `index_carry16`, so
  their C64 builds would drop the end-around carry of an INDEX by a negative
  value. I built host runners whose `ixw` counts every call that takes the
  carry and ran all the cases. For p51-p53 that is 31,456 INDEX calls (LM
  17,151, CM 14,305) with 0 carries. For geometry it is 0 calls. No case in
  either domain reaches the 16-bit difference. `rtb-op-codes` now sets the
  key (#272), so the 18 C64 rows reviewer 1 found reading LIMITS -2 now take
  the carry on the C64 as yaAGC does.
- **Domain.** I checked each restricted state against the source's writers
  and callers:
  - **BESTI and BESTJ** (any of the 37 stars ×6 or 228). R52, PICAPAR and the
    planet path take these values from AOTCODE and the catalog. PLANET and
    R59A compute BESTI from AOTCODE, which their cases set to random values,
    so codes 0 and above 37 also run on that path. R52's `CA BESTI` and the CATLOG read
    (`indexed_fixed` -228..0) don't refuse on 0.
  - **CM MARKSTAT.** It is restricted to the five mark-VAC addresses. That
    is what SXTMARK's ENTANSWR leaves (`MASK LOW9`) before R53A's
    `INDEX MARKSTAT` reads run.
  - **STARCODE and POSCODE.** These are the codes R52 and R59 build (code
    plus detent bits).
  - **CSM_GEOMETRY.** VAC+034 (SXTANG's `GOTO 28D` return) is a CADR outside
    the file, and scalar MODE is pinned at the SXTNB and SXTANG resume
    points, as the code leaves them.

## Notes (not blocking)

- `cert_precheck` notes 810 and 691 refusal sites in p51-p53 that are never
  compared. These are the routines' entry guards and the guards on indexed
  addresses outside the domain. The differential reports no refusal for any
  generated case.
- Set `index_carry16` on `p51-p53` and `geometry` at their next refreeze, as
  the precheck hint says, so that inputs outside today's case domains stay
  right on the C64 too.
