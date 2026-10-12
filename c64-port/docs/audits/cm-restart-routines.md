# Certificate review: Comanche055/RESTARTS_ROUTINE.agc and RESTART_TABLES.agc

Reviewed on 2026-10-12 by the certificate reviewer, at `port/c64-native-c` 9b1bdb76. The certificates under review were drafted by milestone `cm-restart-routines` (PR #62).

Verdict: **accepted** for both files.

## What was checked

- **Evidence.** The four records are distinct: `docs/CM-RESTART-ROUTINES.md`, the yaAGC proof, the host log and the VICE log. All hashes match.
- **Signed zeros (the LM gap from issue #29).** yaAGC was instrumented with `--wrap=agc_engine` to record the successor of every CCS and BZMF in 01,3520-3767 and of NEWPHASE's `CCS L` across all compared cases.
  - Every reachable +, +0, - and -0 outcome is compared, including the A overflow classes at FINDTIME. These sites were checked:
    - TIMETEST 3566
    - FINDTIME 3601 and 3606
    - CHKNOVAC BZMF 3633
    - ITSLGCL2 3710 and 3714, including the "CAN'T GET HERE" NOOP, which is reached in 20 cases
    - CONTBL2 3663
    - NEWPHASE 4123
  - The outcomes no case reaches cannot happen: a CCS after MASK, the NEG2 sum at 3555, the CYR bound at 3644, and 077777 appearing only in PRDTTAB.
- **Transfer ABI.** The RESTARTS entry (RACTCADR, 01,3520), GOLOC-1 and GOLOC+2 (erasable 0704 and 0707), and the display return 01,3542 are registered routines. The nine `TC NEWPHASE` operands are compiled from the listing.
- **Tables.** The 79 table entries and their sites are compiled from the listing, and `cm_restart_tables_generate.py --check` reproduces them. Nothing reads the rope at run time.
- **Site tables.** No exec or WAITLIST site table is needed. The service 2CADR is read from erasable GOLOC, and INITDSP is in `cm_restart_operands`.

## Reproduced

- **Oracle comparison.** `cm-restart-routines-differential`: 7,168 select, 5,120 compose and 1,024 NEWPHASE cases equal the original program, and `proof.json` rebuilds byte for byte.
- **Host and sanitizer.** `test-` and `sanitize-`: 15,908 checks, 0 failures.
- **C64.** `vice-` (x64sc 3.7.1 PAL): 832 checks, 0 failures. The PRG hash equals the one in the log.
- **Coverage.** gcov reaches 111/111 lines of `src/cm_restart_routines.c`. Every untaken branch arm is an input-refusal guard.

## Notes for the lane

- `cm-restart-routines-vice.log` holds a stale first monitor session from a different PRG. The PASS line belongs to the second session. Recapture the log.
- The composition uses recording service stubs, as LM does. Once Comanche FINDVAC, NOVAC and WAITLIST are registered routines, add a round trip through them.
