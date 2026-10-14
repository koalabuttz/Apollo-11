# Certificate review: Lane G Comanche055 files after PRs #258 and #264

Reviewed 2026-10-13 by the second certificate reviewer at `port/c64-native-c`
1816baab, in a detached worktree. None of these files' sources, vectors,
evidence or makefiles changed between 1816baab and 96fb4fcd. This covers four
new CM flight certificates, the re-review of TVCSTROKETEST after auditor issue
#219 and its fix in #258, and a re-confirmation of ten accepted certificates
whose evidence was re-frozen in #258 or whose C64 groups were split into parts
in #264.

| File | Milestone | Verdict |
|---|---|---|
| TIME_OF_FREE_FALL | `cm-time-free-fall` | accepted |
| CONIC_SUBROUTINES | `cm-conics` | accepted |
| INTEGRATION_INITIALIZATION | `cm-integration-init` | accepted |
| STABLE_ORBIT (P38-P39) | `cm-stable-orbit` | accepted |
| TVCSTROKETEST | `cm-tvc-stroke` | accepted (vice_targets corrected) |
| TVCMASSPROP | `cm-tvc-massprop` | accepted (re-confirmed, vice_targets corrected) |
| TVCEXECUTIVE | `cm-tvc-exec` | accepted (re-confirmed) |
| TVCRESTARTS | `cm-tvc-restarts` | accepted (re-confirmed) |
| TVCINITIALIZE | `cm-tvc-init` | accepted (re-confirmed) |
| TVCROLLDAP | `cm-tvc-roll` | accepted (re-confirmed) |
| TVCDAPS | `cm-tvc-daps` | accepted (re-confirmed) |
| AUTOMATIC_MANEUVERS | `cm-auto-man` | accepted (re-confirmed) |
| JET_SELECTION_LOGIC | `cm-jet-select` | accepted (re-confirmed, vice_targets corrected) |
| CM_ENTRY_DIGITAL_AUTOPILOT | `cm-entry-dap` | accepted (re-confirmed) |
| RCS-CSM_DIGITAL_AUTOPILOT | `cm-rcs-dap` | accepted (re-confirmed) |

## Correction: stale vice_targets

Three certificates named a `vice_targets` count that was not the count in the
VICE log they cite. The counts were last set at 76ed9fbb; the logs changed at
b77b4873 (#258) and 87f9c491 (#264). In each case the log's sha256 matches the
certificate, the log has no FAIL, and its check count equals the proof's C64
rows and the rows in `tests/<m>_c64_vectors.h`. This PR sets each count to the
log's:

| Milestone | Was | Log and C64 rows |
|---|---|---|
| cm-tvc-stroke | 12 | 13 |
| cm-tvc-massprop | 6 | 7 |
| cm-jet-select | 28 | 27 |

`cert_precheck` compares `vice_targets` only against the nightly manifest, so
it did not flag these. A scan of every certificate's cited log against its
`vice_targets` found no other mismatch in a log of this format.

## What was checked

- **Precheck.** `tools/evidence.py verify` passes all fifteen manifests, and
  `tools/cert_precheck.py` gives 0 FAIL on all fifteen. Every
  `source_revision` is 911e5c02.
  - The `index_carry16` WARN does not apply: no generated source calls `ixw`.
  - The NEWJOB WARN does not apply to the eleven basic-code files: `sane()`
    has no live caller there, and `basic_sane()` does not read NEWJOB.
  - The refusal-site NOTEs spot-checked are entry guards, TC Q/TC K guards,
    dispatcher defaults and the BUNKER accept list.
- **yaAGC comparison, rerun by the reviewer.**
  `tools/interp_routine_differential.py <m> --cases 256` prints PASS with
  every case equal on all fifteen. Every regenerated case set hashes to the
  proof's `cases_sha256`, and the committed vectors came out unchanged.
- **Host and sanitizer.** `make test-<m> sanitize-<m>` passes with 0 failures
  on all fifteen, with the same counts as the committed host logs.
- **VICE.** Each log's sha256 matches its certificate, and none contains FAIL.
- **Independent C64 branch check.** Every case regenerated, every header row
  matched to a case (drawing cases from `case_points`, as the differential has
  done since #264), per-object gcov `-b`, then each C64 group built with its
  own KEEP flags and run on only its rows. 0 outcomes and 0 lines missing on
  all fifteen:

  | Milestone | Cases equal | Host checks | VICE checks (PRGs) | Outcomes | Lines |
  |---|---|---|---|---|---|
  | cm-time-free-fall | 30,976 | 968 | 192 (6) | 1,039 | 2,023 |
  | cm-conics | 113,920 | 3,560 | 616 (32) | 2,143 | 4,938 |
  | cm-integration-init | 60,160 | 470 | 322 (9) | 1,373 | 3,100 |
  | cm-stable-orbit | 26,368 | 206 | 139 (3) | 883 | 1,788 |
  | cm-tvc-stroke | 1,024 | 32 | 13 | 230 | 422 |
  | cm-tvc-massprop | 512 | 16 | 7 | 262 | 433 |
  | cm-tvc-exec | 3,072 | 96 | 25 | 294 | 694 |
  | cm-tvc-restarts | 1,792 | 56 | 19 | 249 | 472 |
  | cm-tvc-init | 2,560 | 80 | 14 | 439 | 1,008 |
  | cm-tvc-roll | 512 | 16 | 16 | 352 | 1,098 |
  | cm-tvc-daps | 2,304 | 72 | 18 | 335 | 1,707 |
  | cm-auto-man | 1,536 | 48 | 24 | 484 | 1,297 |
  | cm-jet-select | 512 | 16 | 27 | 732 | 2,148 |
  | cm-entry-dap | 7,168 | 224 | 79 | 865 | 3,089 |
  | cm-rcs-dap | 4,352 | 136 | 43 | 697 | 2,146 |

## TVCSTROKETEST after #219

`src/cm_tvc_stroke.c:113` holds FCARD +0..+9 (4, 5, 11, 15, 6, 5, 4, 6, 23657,
35016), matching the listing at 20,3524-3535, and both `INDEX CARD` reads go
through it. An instrumented copy logged every indexed read over all 1,024 host
cases: CARD at the INDEX is -4..-1, +1 or +2, the post-STROKILL reads are
ix +1 (28 cases) and ix +2 (22 cases), and none refuses. The C64 rows take the
ix +2 path, so a post-STROKILL read (FCARD+6, +9) runs on the C64.

Source domain: HACK is called only from PSTROKER/YSTROKER and schedules one
HACKWLST per step, so after STROKILL one HACKWLST can still run with CARD -0;
STROKER is then ±0 and HACK is not called again. Even CARD +1 at entry reads
at most FCARD+9. The only writes to CARD are TSTINIT and INCR.

## Domain review of the four new certificates

Each restriction was dropped in turn and a reduced differential rerun; where
the native then refused, the AGC callers were checked. Each restriction that
matters matches every real caller:

- **Conics.** X1 -2/-10 is set by every caller (ORBITAL_INTEGRATION AXC,1;
  VECSHIFT; P34; P37 LXC,1 CONICX1; TPI_SEARCH). The MODE, PUSHLOC and period
  states are what the preceding orders leave at those resume points. "TAU.
  small" only bounds loop length: with TAU. free and 60,000 steps, 3,559 of
  3,560 cases are equal and the last also times out in yaAGC.
- **Integration init.** PBODY 0/2, X2 0/2, PUSHLOC and LOCCTR/STALTEM match
  their writers and callers. INTWAKUQ = 03704 at 27,2745 matches its only
  caller (27,3703 UPOUT).
- **Stable orbit.** RTRN at 04,3272 holds one of the four VNDSPLY returns
  (2750, 3041, 3152, 3154), which match its only callers.

## Notes (not blocking)

- The integration-init domain text does not name the INTWAKUQ cell.
- TVCSTROKETEST's ix +1 reads run only on the host; they take the same branch
  outcomes as the ix +2 rows.
- PRGs were not rebuilt and VICE was not rerun; the PRG hashes were checked
  against the logs only.
