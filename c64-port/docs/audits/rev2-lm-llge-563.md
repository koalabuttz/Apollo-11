# Second review: lm-llge after #557 and #563

Second certificate reviewer, 2026-10-17. Lane F's #557 made the CADR words after `TC USPRCADR` into routines that return through DANZIG (`usprcadr_exits`, 246 to 249 routines). #563 (head 1ff6317f) fixes the domain finding raised in #558. **Verdict: accepted and re-cited.** The previous record is rev2-lm-llge.md.

## #557 (reviewed at 3cab5c25)

- **USPRCADR** (INTER-BANK_COMMUNICATION.agc:126) leaves this state at the CADR word: LOC at the word, EDOP = 1, BANKSET = the caller's BBANK. The routine it names ends at DANZIG, which skips the NEWJOB test and runs EXIT to L+2.
- **The three CADRs:** ABS at 31,3575 (ROOTPSRS+18), DDV/BDDV at 31,3642 (ROOTLOOP+14) and ABS at 31,3647 (ROOTLOOP+19).
  - ABS writes only MPAC to MPAC+2. MODE is never negative in ROOTPSRS, so ABS takes DCOMP.
  - DDV writes MPAC, BUF, DVSIGN, DVNORMCT, MAXDVSW and OVFIND.
  - Neither touches RETROOT, MPAC+3..+6, WCHPHASE, PUSHLOC, POLISH or TEM1, so the pins on those words hold at the new keys.
- **Coverage of the sites.** The file has exactly three `TC USPRCADR` (lines 1291, 1337, 1344), and all three are covered.
- **Generated C.** r63575, r63642 and r63647 check at_exit, refuse unless EDOP is 1, and then run DANZIG's bank load, the pending order and EXIT. The DANZIG marker comes first among the target's interpretive tables, and `-DAGC_TRANSFER_DANZIG_MARKER` is set in c64_flags and in lm-llge.mk.

## Domain fix (#558, fixed by #563)

- **The problem.** At #557 the ROOTLOOP keys carried the DERCLOOP pins for 0117 (PWRPTR = POLISH) and 0141 (DERPTR = TEM1 = POLYRET). POWRSERS (INTERPRETER.agc:1021) rewrites both words on every ROOTLOOP pass, so flight holds other values there:

  | Point | 0141 | 0117 |
  |---|---|---|
  | ROOTLOOP+6 | 03632 | 0145 |
  | ROOTLOOP+13 to +20 | 03641 | 01561 |

- **The fix.** #563 gives the six ROOTLOOP keys `ROOTLOOP_STATES`, which is `ROOTPS_STATES` without 0117 and 0141. ROOTPSRS+17/+18/+19 and DERCLOOP+3 keep the pointer pins, which are correct there.
- **Docs.** The domain text in port-coverage.json and LM-LLGE.md now says this. The PR changes only the spec, the docs and the regenerated evidence; src/lm_llge.c is unchanged.

## Evidence (at 1ff6317f)

- `ledgers.py --validate` and `cert_precheck` pass, with 0 FAIL. The one WARN is the uncached vice manifest.
- Every evidence sha256 in the certificate matches its file.
- Differential: 127,488 cases over 249 routines equal yaAGC.
- Host and sanitizer: 3,984 checks.
- VICE: 47 PRGs, 384 rows, matching vice_targets. They take all 2,475 traced arcs; the gcov proof is 2,432/2,432.

## Notes (non-blocking)

- The PR text gives octal offsets (+22/+16/+23); the spec keys are decimal (+18/+14/+19).
- The pins 0160 {1,2} and 0162 {0,1} are supersets of the flight values.
- agc_transfer.c's table-linkage count (35/5) is a shared-source NOTE, as before.
