# Certificate re-confirmation: RTB_OP_CODES and RT8_OP_CODES after PR #330

Reviewed 2026-10-14 by the second certificate reviewer at `port/c64-native-c`
838a8864 (PR #330 merged). `lane-d1b-245.md` re-confirmed these certificates.
The strict cover check (`interp_routine_differential.py --cover-check`)
later found that their C64 rows no longer took one host branch outcome,
`src/agc_transfer.c:26` branch 5. That outcome is in the shared transfer-table
lookup, which 2db70a83 reshaped after the rows were picked. PR #330 re-picks
the C64 rows (145, was 144) with the case set unchanged and refreshes the
evidence, so this record becomes the certificates' `independent_review`.

| File | Milestone | Verdict |
|---|---|---|
| Luminary099/RTB_OP_CODES | `rtb-op-codes` | accepted (re-confirmed) |
| Comanche055/RT8_OP_CODES | `rtb-op-codes` | accepted (re-confirmed) |

## What was checked

- `tools/evidence.py fetch rtb-op-codes` fetches all 4 release files, and
  `tools/cert_precheck.py` gives 0 FAIL on both files.
- **yaAGC comparison, rerun by the reviewer.** `--cases 512` prints PASS with
  all 34,816 cases equal across 68 routines. The case set hashes to the
  proof's `cases_sha256` (bb0301ac), and the committed vectors came out
  unchanged.
- **Strict cover check.** `--cover-check` verifies the record: the host cases
  take 852 branch outcomes and the 145 C64 rows take all 852, 0 missing.
- **Host and sanitizer.** `make test-rtb-op-codes sanitize-rtb-op-codes`
  gives 1,132 checks with 0 failures each.
- **VICE.** The log's sha256 equals both certificates', and its PASS lines
  sum to 145 checks with 0 failures, equal to `vice_targets`.
