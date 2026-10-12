# DAP data load (EXTENDED_VERBS, V48)

Lane C. Milestone `lanec-ev-dap`.

## Scope

V48, "request DAP data load", in both missions.

- Luminary099 R03: DAPDATA1 in bank 20 (noun 46 configuration display,
  APSFLAG, DAPBOOLS, MASS, NJETSFLAG and RATEINDX) and STIKLOAD through
  TRIMDONE in bank 01 (stick sensitivity and rate deadband, noun 47 masses
  with their minimum checks, RESTORDB, noun 48 trim display and the
  TRIMGIMB task).
- Comanche055 DAPDISP in bank 43 (refused while the TVC DAP runs) and
  DAPDISP1 in bank 42 (nouns 46, 47 and 48, the maximum-deadband flag,
  S41.2, MASSPROP and S40.14).

The LM DAPDISP front end in bank 43 is already in `lanec-ev-short`. As
there, each `case` of `src/lanec_ev_dap.c` is one basic block, every label
and return point is an entry of its bank, and control runs from block to
block until it leaves these bodies.

## Source map

| Source | Boundaries |
|---|---|
| LM DAPDATA1, CHKDATA1, FORCEONE, MSKDATR1 and the noun 46 returns | BANKCALL (GOXDSPFR), BLANKET, ENDOFJOB, ENDEXT |
| LM DPDAT1 (flags, DAPBOOLS, MASS with its TS overflow skip, NJETSFLAG, RATEINDX) | POSTJUMP (STIKLOAD) |
| LM STIKLOAD (EBANK6, MP 1/10 when docked), DAPDATA2 and its returns, ENDR03 | BANKCALL (GOXDSPFR), IBNKCALL (RESTORDB), BLANKET, ENDOFJOB, ENDEXT |
| LM DAPDAT2 minimum-mass checks, LEMALONE (ZL, DXCH MASS) | IBNKCALL (RESTORDB) |
| LM DAPDATA3, DPDAT3, TRIMDONE and their returns | BANKCALL (GOXDSPFR, GOMARK3R), WAITLIST (TRIMGIMB), BLANKET, ENDOFJOB, ENDEXT |
| CM DAPDISP | ALM/END, TESTXACT, BANKCALL (DAPDISP1) |
| CM DAPDISP1, DONOUN46/47/48 and their returns, MAXIN, MAXOUT | PRIOCHNG, BANKCALL (GOXDSPF, S41.2, S40.14), IBNKCALL (MASSPROP), UPFLAG, DOWNFLAG, ENDEXT |

`MP` follows Virtual AGC exactly, including the signs of zero products
(`lanec_ev_mp` in `include/lanec_ev_internal.h`).

## Evidence

Harness: `tools/lanec/` (see `docs/LANEC-EV-FRONT.md`).
Hashes: `evidence/lanec-ev-dap/SHA256.json`.

- Independent assembly comparison: `lanec-ev-dap-diff.json` (2,832 cases:
  all 59 entries, 48 variants each, random DAPBOOLS, DAPDATR1, FLGWRD10 and
  FLAGWRD6 bits, masses above and below their minimums and sums that
  overflow into MASS, random EB) and `lanec-ev-dap-oracle.crc`. gcov: every
  reachable branch outcome is taken; the untaken ones are CCS classes a
  positive mask cannot produce.
- Host and sanitizer results: `make test-lanec-ev-dap sanitize-lanec-ev-dap`
  (`lanec-ev-dap-sanitize.txt`).
- C64 execution results: `make vice-lanec-ev-dap`, 258 frozen cases (every
  entry) on a VICE PAL C64 (`lanec-ev-dap-vice.txt`).
