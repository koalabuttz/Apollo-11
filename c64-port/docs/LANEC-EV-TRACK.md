# Tracking and state-vector verb glue (EXTENDED_VERBS)

Lane C. Milestone `lanec-ev-track`.

## Scope

The basic (non-interpretive) code of these EXTENDED_VERBS bodies:

- V69 (both missions): `TC VERB69` to itself, which the hardware TC trap
  turns into a restart. The native routine and the oracle both stop after
  that one transfer.
- Luminary099: V56 TRMTRAK1 (bank 42: flags down, group 2 killed, rates
  stopped, lock-on disabled, restart), V85 DSPRRLOS/RRLOSDSP (bank 40: the
  RR LOS display job and its once-a-second loop) and ATTACHIT (bank 43:
  GENTRAN of the state vector and the return through TCPIN).
- Comanche055 (bank 43): V56 TRACKTRM and CLEANOUT, SETVHFLG and RESETVHF,
  ATTACHIT, TACHEXIT, LMTOCMSV and LMTOCM.

Each `TC INTPRET` hands the interpretive words that follow it to the
interpreter, so it is a boundary here (Q = the first interpretive word),
and the word after the interpretive `EXIT` is an entry. The interpretive
words themselves are not part of this milestone.

## Source map

| Source | Boundaries |
|---|---|
| V69 | VERB69 (itself) |
| LM TRMTRAK1 and its returns | DOWNFLAG, INTPRET, PHASCHNG, IBNKCALL (STOPRATE, RESTORDB), CLRADMOD, POSTJUMP (GOPROG2) |
| LM DSPRRLOS and its returns; RRLOSDSP (DCA CDUT, DXCH MPAC); its post-EXIT loop | FINDVAC, PRIOCHNG, BANKCALL (GOMARKFR, DELAYJOB), B5OFF, BLANKET, ENDOFJOB, INTPRET, ENDEXT |
| LM ATTACHIT, its post-EXIT GENTRAN, TACHEXIT part, TCPIN set-up | INTPRET, GENTRAN, POSTJUMP (INTWAKE) |
| CM TRACKTRM and its returns, CLEANOUT | GOPIN, DOWNFLAG, STOPRATE, INTPRET, 2PHSCHNG, POSTJUMP (ENEMA) |
| CM SETVHFLG, RESETVHF, ATTACHIT, TACHEXIT, LMTOCMSV, LMTOCM | INTPRET, GOPIN, TRFAILOF, GENTRAN, FINDVAC, ENDOFJOB, POSTJUMP (INTWAKE) |

Domain: the TCPIN set-up (`INDEX FIXLOC`, `TS QPRET`) stays a TS of
erasable memory only when FIXLOC + QPRET is below 02000, which holds for
every VAC area the executive assigns; other FIXLOC values are out of scope.

## Evidence

Harness: `tools/lanec/` (see `docs/LANEC-EV-FRONT.md`).
Hashes: `evidence/lanec-ev-track/SHA256.json`.

- Independent assembly comparison: `lanec-ev-track-diff.json` (2,304
  cases: all 48 entries, 48 variants each, random flag words, EXTVBACT, EB
  and FIXLOC) and `lanec-ev-track-oracle.crc`.
- Host and sanitizer results: `make test-lanec-ev-track sanitize-lanec-ev-track`
  (`lanec-ev-track-sanitize.txt`).
- C64 execution results: `make vice-lanec-ev-track`, 38 frozen cases on a VICE PAL C64,
  picked by `tools/lanec/cover.py` so that together they take every
  branch arc the 2,304 host cases take (`lanec-ev-track-vice.txt`).
