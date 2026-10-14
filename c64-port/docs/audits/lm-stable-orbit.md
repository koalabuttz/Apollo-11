# Certificate review: Luminary099/STABLE_ORBIT.agc

Reviewed 2026-10-14 by the certificate reviewer, against milestone
`lm-stable-orbit` (Certificate fixes lane, from Lane B), after #329.
Reviewed at ec35425b. Earlier rejection: issue #165.

Verdict: **accepted**.

## What was checked

- **The #165 gaps are closed.** C64 rows now take every host branch arc. An
  independent per-object `gcov -b` run over the host cases and the 122 C64
  rows (rebuilt from tests/lm_stable_orbit_c64_vectors.h) gives 1,033 arcs
  each: 0 host-only and 0 C64-only.
- **Edge operands.** DDV DELTAR (34,3416) sees a +0 divisor (49 cases), a ±0
  dividend and overflow. DSU (34,3430) gives +0, −0 and overflow.
- **The oracle reproduces.**
  - `make lm-stable-orbit-differential` gives 25,856 cases equal to yaAGC.
  - The refreeze is clean: vectors, C64 vectors and groups are unchanged, and
    the log is identical to the release log.
  - proof.json differs only in its gcov arc counts (1,033 against 1,029),
    which come from shared-source drift.
- **Host and sanitizer:** 202 checks each, 0 failures.
- **VICE:** 3 PRGs, 122 checks, 0 failures. The PRG hashes differ from the
  evidence log only through drift in the shared linked sources
  (lm_interp_orders.c, lm_interp_steps.c, cm_interp_orders.c, agc_transfer.c,
  tests/interp_case.c). The milestone's own sources are unchanged.
- **Transfer ABI.** P38/P78/P39/P79 (FCADRMM1) and the GOFLASH/GOFLASHR
  returns are routines at their listing addresses. Outgoing transfers end in
  the trampoline. No rope is read at run time.
- **Rule #414** (BOVB TCDANZIG / ENDT) does not apply: the file has none.
- **Evidence:** hashes match and `port_coverage.py` passes.

## Notes for the lane (not blocking)

- The BHIZ −0 arm at 34,3335 (OPTION2 77776/77777) is never compared. Only a
  crew N06 load gives it, and it takes the same branch as +0. An OPTION2
  77777 state would close it.
- The arc counts disagree: LM-STABLE-ORBIT.md says 1,024, cover.json says
  1,029, and proof.json `c64_gcov_branches` says 1,003.
