# Certificate review: Luminary099 SPS_BACK-UP_RCS_CONTROL after PRs #379 and #388

Reviewed 2026-10-14 by the second certificate reviewer, first at 393f7a9a
(PR #379; not accepted in #386), then again at c8576d11 (PR #388), with the
yaAGC reference rebuilt from the tip.

| File | Milestone | Verdict |
|---|---|---|
| Luminary099/SPS_BACK-UP_RCS_CONTROL | `lm-sps-backup` | accepted |

## What was checked

- **Precheck.** `tools/evidence.py fetch` passes and `tools/cert_precheck.py`
  gives 0 FAIL (the one WARN is the uncached nightly VICE manifest). The
  `source_revision` is 911e5c02. The 15 refusal sites are the entry guard,
  the INDEX opcode and register guards, the UTIME index guard and the TC Q/TC
  E guards. The withdrawn #85 certificate kept beside it confuses neither the
  precheck nor the ledgers.
- **yaAGC comparison, rerun by the reviewer.**
  `tools/interp_routine_differential.py lm-sps-backup --cases 512` prints
  PASS with all 512 cases equal; 274 return to ISWRETRN 04707. The case set
  hashes to the proof's `cases_sha256` (34e64b1c). The strict `--cover-check`
  verifies the record, 201 of 201.
- **Host and sanitizer.** 36 checks with 0 failures each.
- **VICE.** The log's sha256 equals the certificate's and it has no FAIL. Its
  9 checks equal `vice_targets`, the proof's C64 rows and the header rows. The
  PRG rebuilt at the tip is byte-identical to the logged one.
- **Independent C64 branch check.** All 9 header rows match a regenerated
  case. Per-object gcov over all host cases shows 0 missing, and the KEEP
  group takes all 229 host outcomes and 324 lines. Every host arm that can
  run is taken, including both arms of the BZMF POSTHRST at 21,3765 (23 and
  4) and the outer rate limit at |EDOT| 01166/01167.
- **Closure.** The 82 words are 78 compiled words and 4 constants, matching
  the listing at 21,3646-3767.
- **Domain.** The callers are P-AXIS (P-AXIS_RCS_AUTOPILOT.agc 928-936) and
  Q,R-AXIS (536-538), through INHINT and IBNKCALL under E6, returning to
  ISWRETRN. SPSRCS runs in T5RUPT over whatever job is current, so its entry
  guard (`sane_nv`) no longer checks FIXLOC and accepts NEWJOB -0: 269 cases
  draw a non-VAC FIXLOC and 126 NEWJOB -0. The file writes only TJx, OLDSENSE
  and the jet counters, compared through all erasable words. `trusted_image`
  affects only `RK_FIXED_IMAGE` builds, which no cited build is.
- **Fixed since #386.** The first review found the VAC FIXLOC and NEWJOB
  guard refusing normal interrupt-context states, and the BZMF fall-through
  into TJZERO never taken. Both are fixed in #388.
- **Not blocking.** `docs/LM-SPS-BACKUP.md` 23 lists only TS OLDSENSE among
  the overflow skips that cannot run; TS TJU (3651, 3671), TS UJETCTR (3666)
  and TS OLDSENSE (3734) cannot overflow either.
