# V55 clock update and the ATTCK2 task (EXTENDED_VERBS)

Lane C. Milestone `lanec-ev-time`.

## Scope

- V55, "increment AGC time": Luminary099 R33 in bank 42 and Comanche055
  ALINTIME in bank 43 (the noun 24 load, the V23 check and UPDATIME, which
  adds the loaded increment to TIME2/TIME1 through MPAC and TPAGREE).
- Luminary099 ATTCK2 in bank 42, the waitlist task V43 (IMUATTCK, already in
  `lanec-ev-align`) starts: THETAD times 0.1 into CDUZCMD, CDUYCMD and
  CDUXCMD, then channel 14 bits 15-13 to drive the CDUs.

The LM ALINTIME front end in bank 43 is already in `lanec-ev-short`. Each
`case` of `src/lanec_ev_time.c` is one basic block, every label and return
point is an entry, and control runs from block to block until it leaves
these bodies. `DXCH` and `DAS` of memory follow Virtual AGC exactly
(`lanec_ev_dxch` and `lanec_ev_das` in `include/lanec_ev_internal.h`).

## Source map

| Source | Boundaries |
|---|---|
| LM R33 (PRIOCHNG, noun 24 load and its returns); CM ALINTIME (TESTXACT, noun 24 load and its returns) | PRIOCHNG, TESTXACT, BANKCALL (GOXDSPF, GOMARKF), ENDEXT |
| V23 check, UPDATIME (TIME2/TIME1 into MPAC, add the increment) | TPAGREE, ENDEXT |
| TPAGREE return (new clock into TIME2/TIME1), UPDTMEND | ENDEXT |
| LM ATTCK2 and its axis loop (INDEX, MP, XCH CDUxCMD, CCS Q), WOR CHAN14 | TASKOVER |

Domain: ATTCK2 +1 is entered with the axis index 0, 1 or 2 its loop leaves
in A; any other A there is out of scope.

## The CDU drive in the oracle

Virtual AGC simulates the coarse-align CDU drive between instructions:
whenever channel 14 bits 15-13 are set it empties CDUXCMD..CDUZCMD on a
schedule kept in engine statics that carry over from one case to the next.
That is hardware outside the program, like the timers the harness already
holds still, so `tools/lanec/oracle.c` keeps those bits out of the engine's
view except during an instruction that does channel 14 I/O itself (indexed
I/O included), and the ATTCK2 cases start with the drive off. Every earlier
Lane C suite produces the same oracle CRCs as before this change.

## Evidence

Harness: `tools/lanec/` (see `docs/LANEC-EV-FRONT.md`).
Hashes: `evidence/lanec-ev-time/SHA256.json`.

- Independent assembly comparison: `lanec-ev-time-diff.json` (1,152 cases:
  all 18 entries, 64 variants each, V23 and other verbs in MPAC, random
  clock words and increments with small values for carries, random THETAD,
  each ATTCK2 axis index) and `lanec-ev-time-oracle.crc`. gcov: every
  branch outcome in the step functions is taken.
- Host and sanitizer results: `make test-lanec-ev-time sanitize-lanec-ev-time`
  (`lanec-ev-time-sanitize.txt`).
- C64 execution results: `make vice-lanec-ev-time`, 13 frozen cases on a VICE PAL C64,
  picked by `tools/lanec/cover.py` so that together they take every
  branch arc the 1,152 host cases take (`lanec-ev-time-vice.txt`).
