# Certificate review: Luminary099/RADAR_LEADIN_ROUTINES.agc

Reviewed 2026-10-14 by the certificate reviewer, against milestone
`lm-radar-leadin-routines` (Lane D2).

- **First review:** at 3309df78, after #343, rejected in issue #348.
- **Second review:** at b19baf2e, after #352. RTSTLOC −0 was still never drawn.
- **Third review:** at b9a0a3c1, after #356.

Verdict: **accepted**.

## What the earlier reviews found

1. **Resume 25,2024 was not registered.** This is the `TC BANKCALL; CADR RADSTALL`
   that DORSAMP's `TC VARADAR` returns to through SWCALL/SWRETURN.
2. **RTSTLOC = −0 and RTSTDEX = −0 were never compared.** DORSAMP2 stores −0
   in RTSTLOC on every wrap, so −0 is the normal value during
   rendezvous-radar sampling.
   - #352 added −0 to RTSTMAX ('2352') instead of RTSTLOC ('2354').

## What was checked at b9a0a3c1

- **The resume is registered and compared.** DORSAMP+1 is registered at
  21524. It stops at BANKCALL 04616 with Q = 2025, which matches
  `lm_interbank_sites.inc:217`.
- **RTSTLOC −0 is compared.** RTSTLOC is −0 in 395 of the 2,048 cases, and
  RTSTDEX is −0 in 308. The −0 paths run:
  - RADSAMP's `MP RTSTLOC`;
  - DORSAMP2's `INDEX`/`DXCH RSTACK`, `CS`, and `CA`/`AD TWO`;
  - its BZF, both ways.

  All 2,048 cases equal yaAGC.
- **The oracle reproduces.** The proof and source map regenerate
  byte-identical, and the cases hash matches.
- **Host and sanitizer:** 64 checks each, 0 failures.
- **VICE:** 17 checks, 0 failures, equal to `vice_targets`. The PRG hash
  equals the log.
- **Per-object cover.** An independent `gcov -b` run shows the C64 rows
  taking all 186 host branch outcomes.
- **Ledger checks.** The evidence hashes match, `cert_precheck.py` reports
  0 FAIL, and `ledgers.py --validate` passes.
- **NEWJOB.** cert_precheck's NEWJOB WARN comes from the unused `sane()`
  helper. Every routine uses `basic_sane`.
- **#356's DAP compaction** changes nothing this milestone links.

## Notes for the lane (not blocking)

- No C64 row runs `INDEX` or `CA` with RTSTLOC = −0, but no branch outcome
  depends on it.
