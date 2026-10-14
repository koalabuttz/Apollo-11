# Certificate re-review: DOWN-TELEMETRY (CM and LM) after #436

Reviewed 2026-10-14 by the certificate reviewer at a9760dfd. It covers
milestones `lm-downtelemetry` (Luminary099/DOWN_TELEMETRY_PROGRAM.agc) and
`cm-downtelemetry` (Comanche055/DOWN-TELEMETRY_PROGRAM.agc). The earlier
acceptance (downtelemetry.md) was voided by Lane A2's fix in #436.

Verdict: **accepted** for both files.

## The fix

WOZERO's `TC Q` (LM 05,3610 / CM 05,3623) left Q at the next address. The AGC
RETURN leaves Q = 3, and the native code now uses `ai_tc(s,3u)`. The milestone
cases now compare Q after every DOWNRUPT, and the old code fails them.

## What was checked

- **Q handling.** WOZERO's `TC Q` is the only RETURN in either file. Every
  other `ai_tc` sets Q as yaAGC does.
- **What is compared.** Each downrupt compares channels 13/34/35 and Q, and
  all 2,048 erasable words are compared at the end of each case. A, L and
  EB/FB/BB are dead at RESUME, which overwrites them (WAITLIST.agc:433-439).
- **Breadth.** Every downlist word goes out and every entry is reached. Each
  hardware restart resets DNTMGOTO through STARTSUB, which matches the cases'
  start state.
- **Reproduced.**
  - Host, sanitize and differential: 48 cases each, 0 failures, no vector
    diff.
  - The proof and data JSON regenerate byte-identical, and the evidence hashes
    match.
  - VICE: LM 48/0 and CM 24/0. The PRG hashes equal the release logs.
- **Branch outcomes.** The ledger has no c64_branch_outcomes record, so the
  reviewer ran an independent per-object `gcov -b` over all linked sources,
  comparing the host stream with the C64 rows. No arc is taken on the host
  but not on the C64. Every untaken CCS/BZMF outcome is impossible or a
  refusal, and ±0 and edge values reach every outcome that can occur.
- **Ledger checks.** `cert_precheck.py` reports 0 FAIL.

## Notes for the lane (not blocking)

- Add a c64_branch_outcomes record. Its absence causes 2 of the 3 precheck
  warnings.
- The vectors header comment does not mention Q.
- The CM C64 run uses stride 2, so the 1-dump and 4-dump cases run only on
  the host. Their arcs are still taken on the C64.
