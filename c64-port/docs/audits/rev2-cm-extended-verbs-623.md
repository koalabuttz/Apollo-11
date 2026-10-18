# Second review: cm-extended-verbs-file NOVAC entries (#623)

Second certificate reviewer, 2026-10-18. Lane D1b's #623 (head 422005ef) refreezes `cm-extended-verbs-file` with `novac`, fixing the CM P20 C64 stop at 43,3024 (V87's interpretive SET refused). **Verdict: accepted and re-cited.** The previous record is [cm-extended-verbs.md](cm-extended-verbs.md).

## Code

- **Spec.** `'novac': ['SETVHFLG+1', 'RESETVHF+1', 'TRACKTRM+23']`. KEYRUPT starts CHARIN with NOVAC, so V87 (SETVHFLG, `EXTENDED_VERBS.agc:1130`), V88 (RESETVHF, :1138) and V56 (TRACKTRM's INTSTALL call) reach the interpreter with FIXLOC 0110 (NOVAC's LOC(MPAC +6) - LOC(QPRET)) rather than a VAC area.
- **Generated code.** The one change is `sane()`'s FIXLOC line, which accepts 0110. This is the same mechanism as R30 (#604) and cm-p40-p47 (#616).
- **Domain.** The certificate's `supported_domain` now lists FIXLOC 0110 at those three entries, in a quarter of their cases.

## Evidence (at 422005ef)

- **Precheck.** `evidence.py fetch` gets both release files and every hash matches. `ledgers.py --validate` passes, and `cert_precheck` gives 0 FAIL.
- **Differential.** 93,184 cases over 182 routines equal the original programs.
- **Closure.** 540 instructions executed by the oracle and 35 interpreted, of 575.
- **Host.** 5,864/5,864. Re-run locally with `make test-cm-extended-verbs-file`.
- **VICE.** 294 checks, 0 failures, matching vice_targets.
- **Coverage.** gcov host 1,310 = C64 1,310; edges 970/970.

## Notes (non-blocking)

1. As in R30 and P40-P47, `sane()` accepts 0110 at every interpretive entry of the file. The cases draw it only at the three novac points, and a per-point gate is planned for the generator.
