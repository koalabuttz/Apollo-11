# Display sleep and wake (DISPLAY_INTERFACE_ROUTINES)

Lane C. Milestone `lanec-di-sleep`.

## Scope

The code of `DISPLAY_INTERFACE_ROUTINES.agc` that puts a display request
to sleep while the DSKY is busy, swaps a sleeping display for a new one,
and wakes it again, plus the restart entry INITDSP: Luminary099 lines
942-1005 and 1013-1024, Comanche055 lines 967-1028 and 1035-1046. Both
files are the same code at different addresses (bank 10); Comanche055
spells MKOVBIT as BIT3.

JOBXCHS and XCHSLEEP run WITCHONE and XCHTOEND/XCHNYLOC from
`lanec-di-play` through `include/lanec_di_internal.h`.

## Source map

| Source | Native | Boundary |
|---|---|---|
| GOSLEEPS (INDEX COPINDEX, CA PRIOOCT, MASK WAITMASK, TC UPENT2) | `lanec_di_sleep_dispatch` | UPENT2 |
| WAITMASK +1 (CS ONE, AD COPINDEX, TS FACEREG), XCHSLEEP | `xchsleep` | JOBWAKE |
| XCHSLEEP +4 (TC XCHTOEND), XCHSLEEP +5 | `lanec_di_xchnyloc`, `xchsleep5` | JOBSLEEP |
| JOBXCHS (TS FACEREG, TC WITCHONE, TC JOBWAKE) | `lanec_di_witchone` | JOBWAKE |
| JOBXCHS +3 (INDEX LOCCTR, TS FACEREG, CAF XCHQADD, TC XCHNYLOC), JOBXCHS +8 | `lanec_di_xchnyloc`, `jobxchs8` | DOWNENT2 |
| IDLEMASK +1 (INDEX FACEREG, CA BIT5, AD FOUR) | `lanec_di_sleep_dispatch` | UPENT2 |
| XCHQADD +1, GENMARK | `lanec_di_sleep_dispatch` | MARKPLAY or OKTOCOPY |
| MARKWAKE, WAKEPLAY (a TS overflow skips the INDEX) | `wakeplay1` | DOWNENT2 |
| MARKFMSK +1, MARKFMSK +5 | `lanec_di_sleep_dispatch` | JOBWAKE, ENDRET |
| INITDSP (`TS EBANK` also sets BB), INITDSP +4, INITDSP +6 | `lanec_di_sleep_dispatch` | SUPERSW, PRIOCHNG, BANKJUMP |

Domain: COPINDEX 0-2 at GOSLEEPS and 1-2 at WAITMASK +1 (the PRIOOCT
table has three words); FACEREG and TEMPOR2 0, 1 or -0 where they index
the two-word WAKECADR, MARKOCT, BIT5 and BITS5+11 tables (an INDEX by -0
adds nothing, and COPINDEX 1 leaves -0 in FACEREG); LOCCTR -1 (no sleeping
job) or 0 to 01376-LOC after JOBWAKE, any negative value at XCHSLEEP +4;
A 0 or 1 at JOBXCHS and 0, 1 or overflowed at WAKEPLAY. Anything else is
out of scope.

## Evidence

Harness: `tools/lanec/` (see `docs/LANEC-EV-FRONT.md`).
Hashes: `evidence/lanec-di-sleep/SHA256.json`.

- Independent assembly comparison: `lanec-di-sleep-diff.json` (2,176
  cases: all 17 entries, both missions, PRIO, MARK and NORM indices,
  FACEREG -0, absent, idle and sleeping LOCCTR, mark-over-norm set and
  clear, WAKEPLAY with and without overflow) and
  `lanec-di-sleep-oracle.crc`. The cases run 112 of the 113 lines of
  `src/lanec_di_sleep.c` (gcov; the other is the unknown-entry return).
- Host and sanitizer results: `make test-lanec-di-sleep sanitize-lanec-di-sleep`
  (`lanec-di-sleep-sanitize.txt`).
- C64 execution results: `make vice-lanec-di-sleep`, 242 frozen cases
  (every entry, both missions) on a VICE PAL C64 (`lanec-di-sleep-vice.txt`).
