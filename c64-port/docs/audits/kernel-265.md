# Certificate review: Comanche055/ALARM_AND_ABORT.agc (Kernel lane PR #265)

Reviewed 2026-10-13 by the certificate reviewer at `port/c64-native-c` 96eeb38f,
against the certificate drafted by milestone `cm-alarm`. Compare docs/audits/lm-alarm.md.

Verdict: **accepted**.

## Reproduced

- **Evidence.** There are four distinct evidence records plus the cover record. All SHA256.json hashes match, and `port_coverage.py` reports no FAIL for this file.
- **Host tests.** `make test-cm-alarm sanitize-cm-alarm`: 4,096 cases, 8,381 checks, 0 failures. The output is identical to the host log.
- **Differential.** It runs the real Comanche055 rope in yaAGC, and all 4,096 cases are equal. The proof and data JSON regenerate byte for byte, and the vectors show no diff.
- **VICE.** 256 checks, 0 failures, which equals vice_targets. The PRG sha matches the log, and `lane_a_c64_cover.py --check` passes.
- **Independent coverage.** I ran `gcc -O0 --coverage` per object. The stride-16 C64 set is exactly what the diagnostic runs, and it takes a branch pattern identical to the full host run. Every outcome left untaken in `src/cm_alarm.c` is a defensive guard or cannot occur:
  - TS-overflow refusals;
  - CCS negative after a MASK;
  - MULTEXIT low-word refusals;
  - the ALMCADR, io and hardware guards.

## Transfer ABI

- **Entries.** All 21 entries are registered at their listing addresses.
- **Call sites.** The other files hold 61 genuine TC ALARM, ALARM2, BAILOUT and POODOO sites. Adding CURTAINS's own TC ALARM2 gives 62, and that set equals the site table line for line.
- **Other callers.**
  - CCSHOLE (18 sites), CURTAINS (7) and VARALARM (4) callers reach registered entries.
  - `CADR PRIOLARM` goes through BANKCALL to the registered entry.
- **NEWJOB.** NEWJOB is random in every case and compared.
- **Refusals.** No genuine caller is refused.

## Edge states

- **Covered.** These are all taken:
  - FAILREG full states;
  - all four CCS classes of CHKFAIL1, CHKFAIL2 and MULTFAIL;
  - both V37 outcomes;
  - PRIODSPR, and the WHIMPER-to-ENEMA path.
- **Not applicable.**
  - No real caller passes a ±0 alarm code.
  - MPAC and TIME2 are not read.
  - The file is basic code, so the steward's `index_carry16` rule does not apply.

## Notes (not blocking)

- `tools/lane_a_c64_cover.py:148`: `--check` counts the `0u` placeholder as a cover row. This is harmless, because case 0 is already a stride row.
- The erasable-Q path at `src/cm_alarm.c:49` is never compared. A refusal would be more honest there.
- Return addresses for non-site entries are drawn only from 02000-03777.
- The routines are not linked into the CM flight image yet, the same as lm-alarm.
