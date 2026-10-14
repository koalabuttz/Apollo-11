# Certificate review: Luminary099/POWERED_FLIGHT_SUBROUTINES.agc

Reviewed 2026-10-14 by the certificate reviewer, against the certificate
drafted by milestone `lm-powered-subs` (Kernel lane A).

- **First review:** rejected in issue #316.
- **Re-review:** at 7691b7ac, after #317.

Verdict: **accepted**.

## What the first review found

1. A = ±0 was never compared at the SPSIN/SPCOS return entries 23,3626 and
   23,3635, where the following multiply takes yaAGC's ±0-multiplicand path.
2. entry_contracts described `sane()` NEWJOB/FIXLOC refusals for entries that
   use `basic_sane()`.

## What was checked at 7691b7ac

- **A = ±0 is fixed.** `A_PM0` (tools/interp_specs.py:284, applied at :710)
  gives ±0 in 105 of 512 cases at 3626 and 94 of 512 at 3635. All 23,040
  cases equal yaAGC. The host frozen vectors hold ±0 at both entries.
- **The domain text is fixed.** The interpretive entries' text matches
  `sane()` (src/lm_powered_subs.c:44-56) and the basic entries' text matches
  `basic_sane()` (:1454-1460). The ABI wrappers call the guard assigned to them.
- **The closure reproduces.** proof, closure and source map regenerate
  byte-identical, and the vectors are unchanged. 237 of 238 basic
  instructions run; 23,3757 is the one named unreached.
- **Host and sanitizer:** 1440 checks each, 0 failures.
- **VICE:** 67 checks, 0 failures, equal to `vice_targets`. The PRG hash
  equals the evidence log.
- **Per-object cover.** An independent `gcc -O0 --coverage` run gives 521
  branch outcomes for all cases and the same 521 for the 67 C64 rows.
- **Ledger checks.** All 6 evidence hashes match, `cert_precheck.py` reports
  0 FAIL, and `ledgers.py --validate` passes.

## Notes for the lane (not blocking)

- No C64 row has A = −0, and none has ±0 at 3635. Native `agc_multiply` has
  no separate arc for it, and the oracle and host vectors compare it. Pinning
  one −0 row would be a cheap addition.
- The release log headers name 773644c1, which predates the spec change;
  their content reproduces exactly at 7691b7ac.
- "Returns EXEC_OUT_OF_SCOPE before any change" holds for the entry guards.
  The mid-run defensive guards (e.g. :880, :1022, :1395/:1405) can refuse
  after changes, for resume states outside the contract. A clause saying so
  would make the text exact.
