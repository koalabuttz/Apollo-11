# DISPLAY_INTERFACE_ROUTINES full-file certificate (Luminary099, Comanche055)

Lane C. Milestone `lanec-di-cert`.

This drafts the `pending_complete_certificate` of both
`DISPLAY_INTERFACE_ROUTINES.agc` files in `docs/port-coverage.json` for the
auditor's review. The files stay `partial` until the auditor flips them.

## What it claims

Every assembled word of each file is owned by exactly one Lane C display
milestone, or by the constant table at the end of the file:

| Words (LM / CM) | Owner | Native module |
|---|---|---|
| 81 / 81 | `lanec-di-mark`: BLANKET..CLEARMRK +7 (fixed-fixed), MARKEND..COPYPACS +7 | `src/lanec_di_mark.c` |
| 143 / 140 | `lanec-di-dsp`: CHKPRIO..PRIODSP +2, EXDSPRET..SKIPADD +3 | `src/lanec_di_dsp.c` |
| 105 / 105 | `lanec-di-play`: MAKEPRIO..PRIOPLAY +1, WITCHONE..RELINTQ +1, LINUSCHR | `src/lanec_di_play.c` |
| 60 / 60 | `lanec-di-sleep`: GOSLEEPS..INITDSP +8 | `src/lanec_di_sleep.c` |
| 93 / 93 | `lanec-di-nvdsp`: PINBRNCH..V50PASTE +1 | `src/lanec_di_nvdsp.c` |
| 149 / 149 | `lanec-di-idle`: CLEANEND..TERMATE +1, PROCEED..ERASER +3 | `src/lanec_di_idle.c` |
| 39 / 39 | constant table, PERFMASK to the end of the file | compiled `symbols.txt` WORD rows |

Of the 39 constant words, 35 are read by the native code as compiled
constants. The other four (V06N07, BUSYMASK, OCT67777, OCT40420) are not
named by any source line of the program and stay in the rope as data.

## How it is checked

`tools/lanec/wholefile.py` (`make test-lanec-di-cert`) rebuilds the map from
the pinned listings and the unchanged original files. It fails unless:

- every word has exactly one owner;
- every instruction word was started by the unchanged original program in
  Virtual AGC under a case of the six display suites. This holds for all
  612 LM and 609 CM instruction words, measured with the oracle's
  `LANEC_TRACE`. The native modules reproduce those cases' complete
  results;
- each inline data word (an ADRES, OCT, 2CADR or GENADR after a call) sits
  inside an owner's interval as the argument of the call before it.

The map records, for every word, its original line number, address,
assembled word, owner, native module and whether the oracle executed it.

## Evidence

Hashes: `evidence/lanec-di-cert/SHA256.json`.

- Source-to-native map: `lanec-di-lm-source-map.json` and
  `lanec-di-cm-source-map.json`.
- Independent assembly comparison: `lanec-di-comparison.json`. It holds
  the six suites' `diff.py` summaries, 17,632 cases in all, with zero
  differing.
- Host and sanitizer results: `lanec-di-checks.log`. It has the strict
  and ASan/UBSan runs of all six suites and the whole-file map run.
- C64 execution results: `lanec-di-c64-results.json`. It has fresh VICE
  PAL runs of the six frozen subsets (1,406 checks, 0 failures) with the
  PRG hashes.

The supported domain and the external continuations are written into the
certificate itself. The continuations are the EXECUTIVE,
INTER-BANK_COMMUNICATION, SERVICE_ROUTINES, ALARM_AND_ABORT and PINBALL
services that the file calls.
