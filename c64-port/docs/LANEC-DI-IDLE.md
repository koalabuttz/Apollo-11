# ENDIDLE handling and display returns (DISPLAY_INTERFACE_ROUTINES)

Lane C. Milestone `lanec-di-idle`.

## Scope

The code of `DISPLAY_INTERFACE_ROUTINES.agc` that puts a finished display
into ENDIDLE, sorts out how the astronaut answered it, and returns to the
user or wakes the next display: Luminary099 lines 1171-1297 and 1316-1390,
Comanche055 lines 1191-1314 and 1333-1405 (CLEANEND through TERMATE, and
PROCEED through ERASER; LINUSCHR between them is in `lanec-di-play`).
Both files are the same code at different addresses (bank 10).

The module translates the code block by block: each case of `step` in
`src/lanec_di_idle.c` is one basic block of the source, and every block
start is an entry. Control runs from block to block until it reaches a
boundary.

## Source map

| Source blocks | Boundary |
|---|---|
| CLEANEND, CLEANEND +4 (FINDVAC's return) | FINDVAC; FLASHSUB +1 |
| ISITPRIO | PRIOBORT or ENDOFJOB |
| REST, RESTSLEP, RESTSLEP +4 (INDEX COPINDEX, CAF NVCADR) | ENDOFJOB, UPENT2, NVSUBUSY |
| FLASHSUB, FLASHSUB +1, ITISMASK +1 (repeat and return, someone in ENDIDLE) | FLASHON, UPENT2, UNSETR1, ISITPRIO, ENDIDLE |
| IDLERET1, +1, +2 (verb 21-23 check with `DIM A`), OKTOENT, ENDOUT | TERMATE, PROCEED, LOADITIS, TIMECHEK, NORMRET, MARKRET |
| TIMECHEK (two seconds since the PRIO display) | KEEPPRIO or NORMRET |
| NORMWAKE | WAKEPLAY |
| ENDRET, ENDRET +4 (a TS overflow skips CA GENMASK), PINIDMSK +1, +3, ENDIT, ENDIT +3 | ENDOFJOB, DOWNENT2, NVSUB, PRIOCHNG, BANKJUMP |
| UNSETR1, UNSETR1 +6, IMMEDRET | SUPERSW, BANKJUMP |
| TERMATE, PROCEED | ENDOUT |
| MARKRET, MARKOVER, NORMRET, its three later tests, the NOVAC return | ENDRET, NORMBNCH, MARKWAKE, NORMWAKE, NOVAC |
| MARSLEEP | ENDOFJOB or GOSLEEPS |
| LOADITIS | OKTOENT or PINBRNCH |
| ERASER, ERASER +2, +3 | NVSUB, ENDOFJOB |

Domain: COPINDEX and COPMPAC 0-2 (they index the three-word NVCADR and
BIT3 tables and the CADRFLSH and NVWORD cells). Anything else is out of
scope.

## Evidence

Harness: `tools/lanec/` (see `docs/LANEC-EV-FRONT.md`).
Hashes: `evidence/lanec-di-idle/SHA256.json`.

- Independent assembly comparison: `lanec-di-idle-diff.json` (3,840 cases:
  all 40 block entries, both missions, every COPINDEX and COPMPAC,
  CADRSTOR and OUTHERE of each sign including -0, verbs either side of
  V21-V23, loads of the same and another noun, PRIO times equal, exactly
  two seconds and random, A with and without overflow) and
  `lanec-di-idle-oracle.crc`. The cases take 108 of the 116 branch
  outcomes in `src/lanec_di_idle.c` (gcov); the others are the
  out-of-scope guards and the unreachable `default` cases.
- Host and sanitizer results: `make test-lanec-di-idle sanitize-lanec-di-idle`
  (`lanec-di-idle-sanitize.txt`).
- C64 execution results: `make vice-lanec-di-idle`, 226 frozen cases
  (every entry, both missions) on a VICE PAL C64 (`lanec-di-idle-vice.txt`).
