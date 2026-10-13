# Certificate review: Lane D1b attitude maneuver after PR #195

Reviewed 2026-10-13 by the second certificate reviewer at `port/c64-native-c`
9f98cfe4 (PR #195 merged). This is a re-review after issue #172.

| File | Milestone | Verdict |
|---|---|---|
| Luminary099/ATTITUDE_MANEUVER_ROUTINE | `attitude-maneuver` | accepted |
| Comanche055/ANGLFIND | `attitude-maneuver` | accepted |

## What was checked

- `tools/cert_precheck.py` gives 0 FAIL on both files. Every
  `source_revision` is 911e5c02. The four evidence categories cite four
  distinct records, and none cites the source map.
- **yaAGC comparison, rerun by the reviewer.** I ran
  `tools/interp_routine_differential.py attitude-maneuver --cases 512`. All
  235,520 cases over 460 routines equal the original programs, and the
  regenerated case set hashes to the proof's `cases_sha256` (03112a0b…). The
  committed vectors came out unchanged.
- **Host and sanitizer.** `make test-attitude-maneuver
  sanitize-attitude-maneuver` gives 1,170 checks with 0 failures each.
- **VICE.** The log's sha256 equals the certificate's. Its 22 PRGs each record
  their sha256 and pass, with 556 checks and 0 failures, which equals
  `vice_targets`.
- **Independent C64 branch check.** I regenerated every case and matched all
  556 rows of `tests/attitude_maneuver_c64_vectors.h` by edit list.
  - First pass: per-object gcov `-b` over every linked source, every host case
    against every row. 0 missing.
  - The six `c64_segments` rows are cut at the next CHANG2 resume LOC by their
    group's KEEP mask. A full build would credit them with the rest of the
    maneuver. So I also built each of the 22 groups with its own KEEP flags
    (`tools/mk/attitude-maneuver.groups`) and ran only that group's rows
    (`cov_groups.py`, kept in the reviewer's notes).
  - Every one of the 2,946 branch outcomes and 5,612 lines the host cases take
    is still taken by a C64 row. That includes the NEWJOB-positive CHANG2 arms
    that #172 named, in `lm_attitude_maneuver.c` and `cm_attitude_maneuver.c`.
- **Segment rows.** Each one is compared against yaAGC stopped at NEWOPS with
  LOC at the resume LOC (`IRC_SNAPROW`). The differential first checks it on
  the host with the same KEEP mask.

## Notes (not blocking)

- The LM certificate's `supported_domain` still says three outcomes at
  `lm_interp_orders.c:277` and `:1136` are compared on the host only. Those
  line numbers no longer point at code. Both the outcome cover
  (`edges_not_on_c64: []`) and my check find no host outcome missing on the
  C64, so the sentence understates the evidence and can be dropped on a later
  edit.
- `cert_precheck` notes about 250 refusal sites per object that are never
  compared. These are the routines' entry guards. They are the same kind as in
  the KALCMANU review (`lane-d1b-128.md`), and the differential reports no
  refusal for any generated case.
