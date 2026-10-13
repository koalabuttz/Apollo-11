# Certificate review: Comanche055/P11.agc

Reviewed 2026-10-13 by the certificate reviewer at `port/c64-native-c` 952bec48
(after Lane A2 PR #191), against the certificate drafted by milestone `cm-p11`.
This is the second review. The first (Lane A2 PR #178, tip 1ff46b12) was
rejected because the case generator never drew TIME2 = +0, the clock state of
the whole boost phase.

Verdict: **accepted**.

## What was checked

- The earlier blocker is fixed. `joint_states` now seed TIME2 = +0 with TIME1 at
  0, 1, 100, 199, 200 (2SECS), 201, 3000 and 037777: 18,958 of the 37,888 cases,
  and 103 of the 202 committed C64 rows (8 of them at TIME1 = 2SECS).
- Per call site in `src/cm_p11.c` (host cases / C64 rows), all compared equal to yaAGC:

  | P11.agc | outcome | host | C64 |
  |---|---|---|---|
  | 380 CCS TIME2 (REP11) | +0, clock already zeroed, DCA LIFTTEMP | 39 | 1 |
  | 344-351 CS TIME1; AD 2SECS; CCS A | −0 at TIME1 = 2SECS | 12 | 1 |
  | 524-529 CA TIME2; BZMF; TC POWRSERS | ATTDISP pitch polynomial | 7 | 1 |

- `src/cm_p11.c` is unchanged since the first review, so its transfer ABI
  findings stand. Every cross-bank entry, restart 2CADR and PHASCHNG resume point
  is registered at its original address. Outgoing transfers exit through the
  trampoline, and no left-hand RTB or EDOP caller is refused. All 24
  EXEC_INVALID_ACTION sites are unreachable or defensive (SATSW −0 cannot occur
  in flight, because P11 writes SATSW only as ±BIT1 or ZERO).
- The supported_domain text and docs/CM-P11.md ("all 292 labels the host cases
  reach") are corrected. The 10 unreached labels of 302 are the CANNOT GET HERE
  CCS +0, a negative up-counter, dispatch-only twins, the SATSW −0 landing and
  the dead basic shadow of S11.1.

## Reproduced

- `evidence.py fetch/verify cm-p11` passes, and `cert_precheck.py` reports 0 FAIL.
- `make cm-p11-differential`: 37,888 cases equal yaAGC across 148 routines.
  The stop table, vectors, groups and source map are byte-equal.
- `make test-cm-p11 sanitize-cm-p11`: 592 checks, 0 failures.
- `make vice-cm-p11`: 8 PRGs, 202 checks, 0 failures. Per-PRG counts equal the log.
- Independent `gcc -O0 --coverage` over all linked sources gives host 1,174 arcs
  and C64 rows 1,174 arcs, with 0 host-only arcs.

## Notes for the lane (not blocking)

- Shared-source drift since the freeze (`agc_transfer.c`, `tests/interp_case.c`)
  has the following effects:
  - the greedy row cover picks one different C64 row (group 1 row 1: case 114
    becomes case 90);
  - the proof counts move to 1148/2243 from 1144/2236;
  - all 8 PRG hashes differ from the log.

  `interp_routine_differential.py cm-p11 --cover-check` confirms the committed
  202 rows still cover all 1,148 host arcs at the tip. Re-freeze at the next touch.
- supported_domain still says "the rest of 010-061 is +0", but QRUPT 012,
  BANKRUPT 016 and TIME5 030 are drawn. The TIME1 list mixes decimal and octal.
