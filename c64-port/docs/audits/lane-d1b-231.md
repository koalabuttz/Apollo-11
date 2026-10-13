# Certificate review: Lane D1b certificates after PR #231

Reviewed 2026-10-13 by the second certificate reviewer at `port/c64-native-c`
05a2c045 (PR #231 merged). PR #231 adds the `rtb-op-codes` milestone. It also
adds the spec key `any_fixloc` and refreezes `kalcmanu-steering`, `geometry`
and `r60-r62`. This record covers the two new certificates. It also
re-confirms the three accepted certificates whose evidence changed, which
`lane-d1b-128.md` accepted before.

| File | Milestone | Verdict |
|---|---|---|
| Luminary099/RTB_OP_CODES | `rtb-op-codes` | accepted |
| Comanche055/RT8_OP_CODES | `rtb-op-codes` | accepted |
| Luminary099/KALCMANU_STEERING | `kalcmanu-steering` | accepted (re-confirmed) |
| Comanche055/KALCMANU_STEERING | `kalcmanu-steering` | accepted (re-confirmed) |
| Luminary099/LEM_GEOMETRY | `geometry` | accepted (re-confirmed) |

The R60_62 files have no certificate, and their refreeze needs no review.

## What changed in #231

- **`any_fixloc`.** A basic routine whose compiled body never names FIXLOC
  (0120) and never enters the interpreter checks `sane_nv` instead of `sane`
  (`tools/interp_compile.py`, `novac()`). That check drops the VAC-area test on
  FIXLOC and keeps the rest. Half the cases for those routines then put FIXLOC
  anywhere. The flight store needed this for CM STOPRATE, which tasks and V37
  call outside a VAC area. RTB runs in NOVAC jobs too, so `rtb-op-codes` uses
  it as well. Routines that read through FIXLOC, such as CDUINC (which falls
  into PULSEIMU's `INDEX FIXLOC` when its `TS 0` overflows) and PULSEIMU,
  keep the full check.
- **RTB_OP_CODES and RT8_OP_CODES** (130 and 206 words) are new routine
  tables: 24 Luminary and 44 Comanche routines. The hand-written `src/rtb_*.c`
  bodies stay, since compiled interpretive files still call them.

## What was checked

- `tools/cert_precheck.py` gives 0 FAIL on all five, after
  `tools/evidence.py fetch`. Every `source_revision` is 911e5c02.
- **yaAGC comparison, rerun by the reviewer.** I ran
  `tools/interp_routine_differential.py <m> --cases 512`. Each regenerated case
  set hashes to its proof's `cases_sha256`. The committed vectors came out
  unchanged.

  | Milestone | Cases | Routines | Result |
  |---|---|---|---|
  | rtb-op-codes | 34,816 | 68 | all equal |
  | kalcmanu-steering | 26,624 | 52 | all equal |
  | geometry | 49,664 | 97 | all equal |

- **Host and sanitizer.** `make test-<m> sanitize-<m>` gives 1,132, 840 and
  426 checks, with 0 failures each.
- **VICE.** Each log's sha256 equals the certificate's, and its PASS totals
  equal `vice_targets` with 0 failures: 144 checks (3 PRGs), 77 (2) and 136
  (6).
- **Domain.** The domain restricts some inputs, so I checked those limits
  against the callers in the source:
  - CDUINC's BUF is limited to THETAD, +1 or +2. Its only callers are the
    three `TC CDUINC` in Comanche's INCRCDUS, which set BUF to LOCTHETA and
    step it. Nothing in Luminary names CDUINC.
  - The `INDEX A` / `CAF LIMITS` reads take A of +1 or -1. That is what A
    holds after a `TS` that skips on overflow.
- **Independent C64 branch check (every case, per object, per group).** I
  regenerated every case and matched every C64 row by edit list: 144, 77 and
  136 rows, none unmatched. I ran per-object gcov `-b` over every linked
  source, all host cases against the rows. Then I built each C64 group with
  its own KEEP flags and ran only that group's rows (`cov_groups.py`). Every
  branch outcome and line the host cases take is taken by a C64 row, in the
  LM and CM objects alike:

  | Milestone | Groups | Outcomes | Lines | Missing |
  |---|---|---|---|---|
  | rtb-op-codes | 3 | 869 | 1,742 | 0 |
  | kalcmanu-steering | 2 | 779 | 1,567 | 0 |
  | geometry | 6 | 1,350 | 2,419 | 0 |

## Notes (not blocking)

- `cert_precheck` notes 52 and 91 refusal sites that are never compared.
  These are the routines' entry guards and the guards on indexed addresses
  outside the domain above (BUF not THETAD..+2, a LIMITS index other than
  ±1). The differential reports no refusal for any generated case.
