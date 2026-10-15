# Certificate review: Luminary099/ORBITAL_INTEGRATION.agc

Reviewed 2026-10-15 by the certificate reviewer, against milestone
`lm-orbital-integration` (Certificate fixes lane), at port tip 7efafcd7
(the merge of #500).

Earlier rounds are all in issue #165. The last open item was the X1 states
at DOW.. (review 4).

Verdict: **accepted**.

## What was checked

- **DOW.. X1.** From the AGC source:
  - DIFEQ0 enters with X1 = 0 from NEXTCOL's `AXT,1 0`, and DIFEQCOM enters
    with −12 or −24. The spec pins {0, 0o77763, 0o77747}.
  - VECTAB,1 is at 11,3735. A probe over all cases shows that every flight
    X1 (0, −6, −12, −18, −24, −30) reaches VECTAB,1 and matches yaAGC.
- **The oracle reproduces.** 45,184 cases across 353 routines equal yaAGC,
  and resume_flow is 282 of 282.
  - The refreeze is clean: the proof, source map and differential log body
    are byte-identical.
  - The cases sha256 equals the proof's.
- **Host and sanitizer:** 353 checks, 0 failures.
- **VICE:** 30 PASS with 400 checks, equal to `vice_targets`.
  - 26 of the 30 PRG hashes match.
  - The other 4 differ only through a later `tests/interp_cut.c` change on
    the base. Rebuilt with the PR's harness file, all 4 match exactly.
- **Per-object cover.** An independent `gcov -b` run gives host 2,485 arcs,
  and the C64 rows take all of them. `lm_orbital_integration.c` has 1812/1812.
  The only exceptions are the `c->cm` harness arms that `IRC_LM_ONLY`
  compiles out.
- **Ledger checks.** `cert_precheck` reports 0 FAIL. `ledgers --validate`,
  `interp_compile --check` and `cert_selfcheck` (no gaps) all pass. The
  rope sha256 values equal the proof's.
- **Earlier items** from reviews 1-4 all still hold.

## Notes for the lane (not blocking)

- Past the `INCR,1 −6` at 11,3713-3716 (MIDFLAG set), flight X1 is −6, −18
  or −30. So the doc's 11,3705-3764 range is wider than it needs to be, and
  three resume points start from extra non-flight X1 values. This is wider
  than flight, which is safe.
- The `tests/interp_cut.c` drift on the base changes 4 PRG hashes. The next
  re-cite should refresh the VICE log.
