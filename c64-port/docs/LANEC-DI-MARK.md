# Mark displays (DISPLAY_INTERFACE_ROUTINES)

Lane C. Milestone `lanec-di-mark`.

## Scope

The mark-display code of `DISPLAY_INTERFACE_ROUTINES.agc` in both
Luminary099 (lines 474-493 in fixed-fixed, 508-608 in bank 10) and
Comanche055 (lines 509-528, 543-643): the blanking request, the
extended-verb mark cleanup, every GOMARK entry, the mark request copy and
its scheduling, and the copy of a stored display back into the working
cells. Both files are the same code at different addresses (Comanche055
spells MRKNVBIT and XDSPBIT as BIT9 and BIT1 and has no `+2` label).

The GOMARK entries end in the display-start paths translated by
`lanec-di-dsp` (GOFLASH2, GODSPR2, GODSPRS); this module calls those
through `include/lanec_di_internal.h` and the cases here run them again
from the mark entries.

## Source map

| Source | Native | Boundary |
|---|---|---|
| BLANKET (TS MPAC +6, CS PLAYTEM4, MASK, INDEX MPAC +5, ADS PLAYTEM4, TC Q) | `lanec_di_mark_dispatch` | the caller's return |
| ENDMARK | `lanec_di_mark_dispatch` | POSTJUMP (CADR MARKEND) |
| CLEARMRK, CLEARMRK +2 | `clearmrk`, `clearmrk2` | the caller's return |
| MARKEND (TC CLEARMRK, TCF MARKOVER) | `lanec_di_mark_dispatch` | MARKOVER |
| GOMARK, GOMARS, KLEENEX, GOMARKF, GOMARK2, MARKFORM, GOMARK3, GOMARK4 | `lanec_di_mark_dispatch` | GOFLASH2's exits (`lanec_di_goflash2`) |
| GOMARKR | `lanec_di_mark_dispatch` | GODSPR2's exits (`lanec_di_godspr2`) |
| GOMARKFR, GOMARK2R, GOMARK3R | `lanec_di_mark_dispatch` | GODSPRS's exits (`lanec_di_godsprs`) |
| MAKEMARK (TC COPIES with index 1, then the DSKY-busy tests) | `makemark` | CHKPRIO's exits (MARSLEEP, SETNORM), MARKPLAY's exit, or ENDOFJOB |
| MARKPLAY, GOGOMARK, MARKCOP | `markplay`, `gogomark`, `markcop` | PRIOPLAY |
| COPYTOGO, COPYPACS (`TS EBANK` also sets BB, as the AGC does) | `copypacs` | the caller's return |

Domain: BLANKET's offset in MPAC +5 keeps its ADS in unswitched erasable
(0-01216); COPINDEX is 0-2 for COPYPACS (the PRIOOCT table has three
words); a caller's return (`TC Q`) is in fixed memory outside the code
translated here. Anything else is out of scope.

## Evidence

Harness: `tools/lanec/` (see `docs/LANEC-EV-FRONT.md`).
Hashes: `evidence/lanec-di-mark/SHA256.json`.

- Independent assembly comparison: `lanec-di-mark-diff.json` (2,944 cases:
  all 23 entries, both missions, A with and without overflow, flashing and
  non-flashing R displays, BLANKET offsets including ones that overlap
  MPAC, every COPYPACS index, MAKEMARK with the DSKY busy, the mark asleep
  or free, MARKPLAY with and without the perform bit, callers in fixed-fixed
  and bank 10) and `lanec-di-mark-oracle.crc`.
- Host and sanitizer results: `make test-lanec-di-mark sanitize-lanec-di-mark`
  (`lanec-di-mark-sanitize.txt`).
- C64 execution results: `make vice-lanec-di-mark`, 36 frozen cases on a VICE PAL C64,
  picked by `tools/lanec/cover.py` so that together they take every
  branch arc the 2,944 host cases take (`lanec-di-mark-vice.txt`).
