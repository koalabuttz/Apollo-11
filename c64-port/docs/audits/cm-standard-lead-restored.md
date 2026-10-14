# Certificate review: Comanche055/SYSTEM_TEST_STANDARD_LEAD_INS.agc (restored)

Reviewed 2026-10-14 by the certificate reviewer, against milestone
`standard-lead-oracle` (Kernel lane A).

- **First accepted:** docs/audits/kernel-212.md.
- **Withdrawn:** docs/audits/cm-standard-lead-withdrawn.md, after auditor
  issue #318 found that CM E/JOBWAK refused a sleeping core set 0.
- **Re-review at #334:** not accepted. The refusal was fixed, but SPECTEST
  and free core sets were never compared (comment on #318).
- **Re-review at d0ee0eb9, after #337:** accepted.

Verdict: **accepted**.

## What was checked at d0ee0eb9

- **The refusal is fixed.** `cm_basic_wake` scans from core set 0, like
  JOBWAKE and `lm_basic_wake`. With fresh seeds 0x6d21 and 0xa5c3, at 256
  cases per kind, every E/JOBWAK refusal is one of two kinds:
  - SPECTEST with NEWJOB=+0, which no flight state reaches;
  - a random return that lands outside this file.
- **SPECTEST is compared.** Eight CM cases have core set 0 as the matching
  sleeper with NEWJOB −0 (tests/standard_lead_oracle_cases.c:41-47). All
  eight equal yaAGC. `cm_dispatch.c:198-200` runs in them and in two C64 rows.
- **Free core sets are compared.** About a third of core sets 1-7 have
  PRIORITY −0. The skip arc is taken in `cm_basic_wake.c:7` and
  `lm_basic_wake.c:8`, on the host and in the C64 rows.
- **The oracle reproduces.** 1,408 cases equal yaAGC, and the proof
  regenerates byte-identical.
- **Host and sanitizer:** 1,414 checks, 0 failures.
- **VICE:** 480 checks, equal to `vice_targets`. The PRG hash equals the log.
- **Per-object cover.** An independent `gcov -b` run finds 0 native arcs
  missing on the C64.
- **Ledger checks.** The evidence hashes match, `cert_precheck.py` reports
  0 FAIL, and `ledgers.py --validate` passes.
- **The LM certificate** stays consistent with the refreshed evidence. Only
  the E/JOBWAK digests changed, and the LM native code is unchanged.

## Notes for the lane (not blocking)

- Four E/CALL cases at fresh seeds differ because the oracle stops early
  when the callee is in E/CALL's own bank. A stop that also checks LOC
  makes them equal.
- The C64 rows skip two CM harness checks
  (tests/standard_lead_oracle_check.c:18,28). No native arc is missing.
- ALARM's write-before-refuse and the cm-exec-services RESUME generator gap
  (#318 items 2 and 3) are separate and do not affect this file.
