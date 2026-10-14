# Certificate review: Comanche055/POWERED_FLIGHT_SUBROUTINES.agc

Reviewed 2026-10-14 by the certificate reviewer, against the certificate
drafted by milestone `cm-powered-subs` (Kernel lane A).

- **First review:** rejected in issue #314.
- **Re-review:** at 7691b7ac, after #317.

Verdict: **accepted**.

## What the first review found

1. DDV zero and edge operands were never compared: bufzero, x/0 and 0/0,
   the zero-dividend exit, maxdv and divisor major ±1 in
   src/lm_interp_orders.c.
2. A = ±0 was never compared at the SPSIN/SPCOS return entries 23,3566 and
   23,3575.
3. entry_contracts described `sane()` NEWJOB/FIXLOC refusals for entries that
   use `basic_sane()`, and quoted a stale case count.

## What was checked at 7691b7ac

- **The DDV gaps are closed.** SMCDURES (23,3742) and its DDV (23,3751) start
  from edge operands. Every flagged lm_interp_orders.c line (614-622,
  647-663, 669-670, 682-683, 695-704) runs on the host and in the C64 rows,
  and all 24,064 cases equal yaAGC.
- **A = ±0 is compared:** 41 +0 and 55 −0 of 512 cases at 3566, 48 and 54 at
  3575, all equal to yaAGC.
- **The domain text is fixed.** It matches `sane()`/`at_newops`
  (src/cm_powered_subs.c:43-55) and `basic_sane()` (:917-922) separately.
- **The closure reproduces.** proof, closure and source map regenerate
  byte-identical, and the vectors are unchanged.
- **Host and sanitizer:** 1,512 checks each, 0 failures.
- **VICE:** 51 + 33 = 84 checks, 0 failures, equal to `vice_targets`. Both
  PRG hashes equal the evidence log.
- **Per-object cover.** An independent `gcc -O0 --coverage` run finds 0 host
  arcs missing on the C64; the proof's 666/666 regenerates.
- **Ledger checks.** Evidence hashes match, `cert_precheck.py` reports
  0 FAIL, and `ledgers.py --validate` passes.

## Notes for the lane (not blocking)

- No C64 row has A = ±0 at the return entries. Native `agc_multiply` has no
  arc that depends on it, and the oracle compares it.
- The shared DDV body's remainder-equals-divisor arc
  (src/lm_interp_orders.c:611) is not taken. Per earlier precedent, shared
  interpreter arcs belong to the INTERPRETER.agc certificate.
- The release log headers name 773644c1, which predates the fix; their
  content reproduces exactly at 7691b7ac.
