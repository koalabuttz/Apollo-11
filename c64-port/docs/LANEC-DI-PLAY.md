# Display job selection (DISPLAY_INTERFACE_ROUTINES)

Lane C. Milestone `lanec-di-play`.

## Scope

The code of `DISPLAY_INTERFACE_ROUTINES.agc` that decides which display
job runs, in both Luminary099 (lines 742-801, 1144-1169, 1299-1312) and
Comanche055 (lines 771-830, 1164-1189, 1316-1329): MAKEPLAY's priority
change, the PRIO/NORM/MARK selection, the PRIO and NORM paths and their
abort, the LINUS check, WITCHONE and the sleeping-display exchange. Both
files are the same code at different addresses (bank 10).

MAKEPLAY +5 can go to MAKEMARK; that path runs the mark code of
`lanec-di-mark` (`lanec_di_makemark`) and continues through its SETNORM and
PRIOPLAY exits here. OKTOCOPY and OKTOPLAY run COPYNORM/COPIES2 from
`lanec-di-dsp` through `include/lanec_di_internal.h`.

## Source map

| Source | Native | Boundary |
|---|---|---|
| MAKEPLAY (CA PRIORITY, MASK PRIO37, TS USERPRIO, CAF PRIO33, TC PRIOCHNG) | `lanec_di_play_dispatch` | PRIOCHNG |
| MAKEPLAY +5 (PLAYTEM4 bits 15 and 7 pick PRIO, NORM or MARK) | `makeplay5` | the exits below, or MAKEMARK's |
| MAKEPRIO, MAKEPRIO +4, HIPRIO, SETMARK, ASKIFNRM, SETNORM | `makeprio`, `makeprio4`, `hiprio`, `askifnrm`, `jobxchs` | Q+1, POODOO, JOBXCHS |
| OKTOCOPY, OKTOCOPY +3, REDOPRIO, KEEPPRIO | `oktocopy`, `redoprio` | JOBWAKE, GOPLAY |
| IFLEGAL, IFLEGAL +4, PRIOBORT | `iflegal`, `iflegal4`, `prioabort` | Q+1, POODOO |
| OKTOPLAY, OKTOPLAY +13, PLAYJUM1, PRIOPLAY | `oktoplay`, `prioplay` | GOSLEEPS, JOBWAKE, GOPLAY, EXDSPRET (TS overflow) |
| LINUSCHR | `linuschr` | Q+1, ENDOFJOB, the caller's return |
| WITCHONE (`WAND DSALMOUT` clears KEY REL), XCHTOEND, XCHNYLOC, RELINTQ | `witchone`, `xchnyloc` | the caller's return |

Domain: DSPLOCK below 040000 where LINUSCHR runs (its CCS then has two
outcomes); LOCCTR negative, zero, or positive up to 01376-LOC so the
indexed `TS LOC` stays in erasable; COPINDEX up to 0300 where LINUSCHR or
COPIES2 index with it; a caller's return (`TC Q`) in fixed memory outside
the code translated here. Anything else is out of scope. Channel 011
(DSALMOUT) is kept as a value; its light is not modelled.

## Evidence

Harness: `tools/lanec/` (see `docs/LANEC-EV-FRONT.md`).
Hashes: `evidence/lanec-di-play/SHA256.json`.

- Independent assembly comparison: `lanec-di-play-diff.json` (2,208 cases:
  all 23 entries, both missions, PLAYTEM4 selecting PRIO, NORM and MARK
  with and without LINUS, matching and non-matching flash addresses with a
  free or locked DSKY, FLAGWRD4 busy and sleep combinations, EBANKTEM with
  and without BIT4, sleeping, idle and negative LOCCTR, A with and without
  overflow, callers in fixed-fixed and bank 10) and
  `lanec-di-play-oracle.crc`. All cases are in scope and run 194 of the
  195 lines of `src/lanec_di_play.c` (gcov; the other is the unknown-entry
  return).
- Host and sanitizer results: `make test-lanec-di-play sanitize-lanec-di-play`
  (`lanec-di-play-sanitize.txt`).
- C64 execution results: `make vice-lanec-di-play`, 246 frozen cases
  (every entry, both missions) on a VICE PAL C64 (`lanec-di-play-vice.txt`).
