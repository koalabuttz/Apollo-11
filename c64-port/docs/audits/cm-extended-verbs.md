# Certificate review: Comanche055/EXTENDED_VERBS.agc

Reviewed 2026-10-14 by the certificate reviewer, against milestone
`cm-extended-verbs-file` (Kernel lane A).

- **First review:** at 34f9a0e1, after #364. Rejected in issue #369.
- **Re-review:** at d55b30fe, the head of #387. It merged as 7262c653 after a base merge (#308) that regenerated 10 lines of src/cm_extended_verbs_file.c on the shared channel model. `interp_compile --check` and cert_precheck pass at the merge.

Verdict: **accepted**.

## What the first review found

1. **Five cross-bank return points were not registered.**
   - 43,2113: NVSUB's normal return.
   - 43,2135: IMUSTALL's success return.
   - 43,2162, 43,2224 and 43,2247: the data-loaded returns.

   The cause: `basic_compile.py` `outside_tc` registered only L+1.
2. **V55's `BZF UPDATIME` (43,2337) was never taken.**
3. **The NVSUB-return domain left out UPDATE_PROGRAM's 03737.**

## What was checked at d55b30fe

- **The returns are registered.** All five are registered routines at their
  listing addresses: SETXTACT+6, IMUZEROK+6, ICORK2, OPTC1+5 and IMUFINEK+7.
  An `agc_transfer_resolve` probe resolves each one. Each has 512 cases, and
  all equal yaAGC.
- **V55 is compared.** The `BZF UPDATIME` is taken in 261 cases and not taken
  in 251. The C64 rows take both arms.
- **The NVSUB-return domain is complete.**
  - The 18-word list covers every `TC TESTXACT`/`TC SETXTACT` in bank 43,
    plus UPDATE_PROGRAM's 03737. No other file calls them.
  - The 03737 cases leave the file at Z = 03737, and all 40 equal yaAGC.
  - Joint states reach every REGR21, R23CSM and R22ALARM arm.
- **GOEXTVB.** Its MPAC 0-59 domain is exactly the reachable set.
- **`trusted_image`.** It drops only the guards' 15-bit word check, and only
  in flight builds. Every evidence build keeps the check, and every word is
  still compared.
- **The oracle reproduces.**
  - 93,184 cases over 182 routines equal yaAGC.
  - The closure is 575 words: 540 run and 35 interpreted, with 0 unreached.
  - The proof, map and closure regenerate byte-identical, and
    `interp_compile --check` passes.
- **Host and sanitizer:** 5,865 checks each, 0 failures.
- **VICE:** 294 checks, 0 failures, equal to `vice_targets`. The PRG hashes
  match the log.
- **Per-object cover.** An independent `gcov -b` run gives 1,138 branch
  outcomes on the host, all of them taken on the C64.
- **Transfer ABI.** Every label used by another file is registered at its
  listing address, and so are the in-file CADR targets.
- **Ledger checks.** `cert_precheck.py` reports 0 FAIL, and
  `ledgers.py --validate` passes.

## Notes for the lane (not blocking)

- One new line never runs: a jump to ICORK2 that no return value reaches.
