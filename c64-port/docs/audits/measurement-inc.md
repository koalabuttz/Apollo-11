# Certificate review: MEASUREMENT_INCORPORATION (LM and CM)

Reviewed 2026-10-14 by the certificate reviewer, against milestone
`measurement-inc` (Rendezvous navigation lane D2).

- **First review:** at 8e022a15. Rejected in issue #373.
- **Re-review:** at df172441, after #393. Rechecked at 8c9c304d, the port tip
  at flip time: the only drift is one removed line in src/cm_interp_orders.c
  and tools/interp_compile.py changes; `interp_compile --check` and
  cert_precheck pass there.

Verdict: **accepted** for both Luminary099/MEASUREMENT_INCORPORATION.agc and
Comanche055/MEASUREMENT_INCORPORATION.agc.

## What the first review found

1. **The W-matrix loop exits were never reached.** Each pass stopped at
   PHASCHNG, so the TIX 9-D exit and the BHIZ 6-D exit were not compared.
2. **The DDV zero dividend never ran.** The zero-dividend exit in
   lm_interp_orders.c was not compared and no C64 row took it.

## What was checked at df172441

- **The loop exits run.**
  - The TIX 9-D exit runs in 455 LM and 486 CM cases, with 3 C64 rows each.
  - The BHIZ 6-D exit runs in 226 LM and 158 CM cases, with 1 C64 row each.
  - The lmi_bhiz ±0 arm runs (as −0).
- **The DDV zero dividend runs** (lm_interp_orders.c:668-670) with +0 and −0,
  on the host and on the C64, in both mission objects.
- **flow_range_states** values are flight-faithful.
- **The oracle reproduces.** The cases file hashes to the proof's
  cases_sha256 (dc509e7b…). 163,840 cases over 320 routines equal yaAGC, and
  the output is line-identical to the evidence log up to its `froze` line.
- **Host and sanitizer:** 2,560 checks each, 0 failures.
- **VICE:** 18 PRGs whose hashes match the evidence log; 354 checks,
  0 failures, equal to `vice_targets`.
- **Per-object cover.** An independent `gcov -b` run gives 2,251 branch
  outcomes on the host, all of them taken on the C64.
- **Evidence integrity.** All files match SHA256.json; the certificate cites
  4 distinct evidence categories.
- **Ledger checks.** `cert_precheck.py` reports 0 FAIL, and
  `ledgers.py --validate` passes.

## Notes for the lane (not blocking)

- The BOV FAZAB2 overflow arm is never taken.
- sane() does not check the EB=5 / PUSHLOC limits that the domain text states.
- "X1 as WIXA" in the scope text overstates slightly.
- The INTERPRETER certificate still cites the old measurement-inc case set.
