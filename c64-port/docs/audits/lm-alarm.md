# Certificate review: Luminary099/ALARM_AND_ABORT.agc

Reviewed 2026-10-12 by the certificate reviewer at `port/c64-native-c` e362a687
(the merge of PR #78), against the certificate drafted by milestone `lm-alarm`.

Verdict: **accepted**.

## What was checked

- The four evidence categories cite four distinct records, and none cites the
  source map. The comparison is a real yaAGC run of the unmodified rope: 4096
  cases equal the native result.
- Callers: a grep of every other Luminary099 file finds 51 TC ALARM, 1 ALARM2,
  1 BAILOUT, 8 POODOO, 7 BAILOUT1, 2 POODOO1 and 1 ALARM1. With this file's own
  CCSHOLE (OCT 1103) and CURTAINS (OCT 217), that gives 73 inline-code sites,
  equal to `lm_alarm_site_list` (`src/lm_alarm_sites.inc`, regenerated with
  `--check`: 73 sites, 91 words). The other caller forms are also registered
  `agc_transfer` entries at their original addresses (26 in all):
  - TC CCSHOLE (22), TC CURTAINS (11), TC VARALARM (4);
  - PRIOLARM, reached by 7 BANKCALL CADRs;
  - WHIMPER (TCF from SERVICER);
  - GOPOODOO (the RESTART_TABLES 2CADR);
  - the MULTEXIT, BANKCALL and SUPERSW return points;
  - the erasable ALMCADR.
- Inline alarm codes resolve through `agc_transfer_operand` and the compiled
  site table, keyed by physical address and FB/SUPERBNK. No rope reader is
  linked.
- Every abort and restart exit ends in the trampoline:
  - BAILOUT → BORTENT → WHIMPER → POSTJUMP ENEMA;
  - POODOO → GOPOODOO → BANKCALL FLAGS / V37KLEAN / MR.KLEAN, or SUPERSW +
    BANKCALL SERVIDLE;
  - BAILOUT1/POODOO1 through their TCF words.

  The test classifies the end points as 495 ENEMA, 68 KLEAN and 68 SERVIDLE.

## Reproduced

- `test-`, `sanitize-`, `-differential` and `vice-lm-alarm`:
  - 9049 checks, 0 failures;
  - 4096 cases equal yaAGC;
  - the proof regenerates byte for byte, and the vectors have no diff;
  - VICE passes 256 checks with 0 failures. The PRG sha256 `eca12c28…` equals
    the log.
- gcov over the oracle producer leaves untaken only these:
  - the ctx/TS-overflow guards;
  - FAIL3's CCS after MASK POSMAX (never negative);
  - MULTEXIT's word < 010 and non-TC/TCF refusals;
  - V37FLBIT CCS negative;
  - an erasable Q in `code()`;
  - the FLAGS/ALMCADR word guards.

  All four CCS classes of CHKFAIL1/CHKFAIL2 are taken.
- The C64 subset (every 16th case; gcd(16, 91) = 1, so every case kind runs)
  has line and branch coverage identical to the host stream.

## Notes

- Returning through SWRETURN after FLAGS' `TC Q` leaves Q = 3. The accepted
  `lm_interbank` refuses that state, so the GOPOODOO → FLAGS path stops at
  SWRETURN. That is declared in external_continuations and filed against
  INTER-BANK as #94.
- `lm_alarm_operands` holds 18 constants and CADRs besides the 73 site codes.
  A Q pointing at one of them would read a constant instead of refusing. No
  genuine caller produces such a Q.
