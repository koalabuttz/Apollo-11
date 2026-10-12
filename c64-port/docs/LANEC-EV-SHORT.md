# Short extended-verb bodies (EXTENDED_VERBS)

Lane C. Milestone `lanec-ev-short`.

## Scope

The bank 43 verb bodies of `EXTENDED_VERBS.agc` that check a condition,
set or clear a flag, or start a job or task and then hand over to another
routine, for Luminary099 and Comanche055, plus SHOWSUM (V91). Each `case`
of `lm_step`/`cm_step` in `src/lanec_ev_short.c` is one basic block of the
source. Every label and every return point is an entry (after TESTXACT,
CHKPOOH, FINDVAC/NOVAC past their 2CADR, UPFLAG/DOWNFLAG past their ADRES,
CHECKMM past its MM, PHASCHNG past its OCT, BANKCALL past its CADR and the
three GOXDSPF returns), and control runs from block to block until it
leaves these bodies. `TC Q` (EXDAPOFF) is a transfer to register Q and is a
boundary; `CA SR` re-edits SR as the hardware does (CM V52).

## Source map

| Mission | Source (labels) | Boundaries |
|---|---|---|
| LM | DAPATTER, TOTATTER, LRON, LROFF, LEMVEC, CSMVEC, NOUPDOWN, UPDATOFF, SNUFFOUT, OUTSNUFF, VERB96 | UPFLAG, DOWNFLAG, GOPIN, POSTJUMP (V37) |
| LM | V47TXACT, VB64, V82PERF (V82CON), V83PERF, V89PERF, V90PERF, CREWMANU, SYSTEST, ATTACHED, V67, DAPDISP, ALINTIME, VERB85 | TESTXACT, CHKPOOH, PRIOCHNG, FINDVAC, WAITLIST, SUPDXCHZ, ENDOFJOB, POSTJUMP |
| LM | GOLOADLV (WRITE SUPERBNK), CKMODCAD, MINIMP, NOMINIMP, TRMTRACK, DNEDUMP, WMATRXNG | FLASHOFF, POSTJUMP, TCQ, ALM/END, IBNKCALL, DOWNFLAG, GOPIN |
| LM | SHOWSUM, SDISPLAY, NOKILL and its GOXDSPF returns, ENDSUMS | STSHOSUM, BANKCALL, NXTBNK, ENDEXT, SELFCHK |
| CM | V61, V62, V63, ENATMA, LEMVEC, CSMVEC, LFTFLGON, WMATRXNG, SETPRFLG, RESETPRF, SETSURF, RESTSRF, VERB96 | UPFLAG, DOWNFLAG, GOPIN, POSTJUMP |
| CM | VB64, CREWMANU, V82PERF, V83PERF, V85PERF, V89PERF, V90PERF, V67, ATTACHED, STROKON, GOTOR21, GOTOR23, REGR21, R22ALARM | TESTXACT, CHKPOOH, PRIOCHNG, FINDVAC, NOVAC, ALARM, ENDEXT, ENDOFJOB, ALM/END, POSTJUMP |
| CM | CKMODCAD (+1 entry from OPTCOARK), GOLOADLV, V60, SYSTEST, V92CONT, EXDAPOFF, CKOPTVB, CHAZFOGC, STABLISH, V86PERF, DNEDUMP, CHKPOOH, VERB94, V52 | FLASHOFF, POSTJUMP, GOPIN, POODOO, EXDAPOFF's TC Q, CHECKMM, PHASCHNG, CLEANOUT, TCQ, ALM/END |
| CM | SHOWSUM, SDISPLAY, NOKILL and its GOXDSPF returns, ENDSUMS | STSHOSUM, BANKCALL, NXTBNK, ENDEXT, SELFCHK |

Constants these bodies read (BITS9+7, LDNDUMPI, V82CON, IDLECADR,
LANDBANK, VNCON, the PRIO and flag-bit words) are compiled in from the
listing through `tools/lanec/symbols.txt` WORD rows.

## Evidence

Harness: `tools/lanec/` (see `docs/LANEC-EV-FRONT.md`).
Hashes: `evidence/lanec-ev-short/SHA256.json`.

- Independent assembly comparison: `lanec-ev-short-diff.json` (6,368
  cases: all 84 LM and 115 CM entries, 32 variants each, with MODECADR,
  MODREG and SKEEP6 at +0, -0 and random, every combination of the
  rendezvous, track and R23 flag bits, and the TVC and NODOP01 bits set and
  clear) and `lanec-ev-short-oracle.crc`. The cases take every source
  branch in `src/lanec_ev_short.c` (gcov: all outcomes but the out-of-scope
  guard and the unreachable `default` cases).
- Host and sanitizer results: `make test-lanec-ev-short sanitize-lanec-ev-short`
  (`lanec-ev-short-sanitize.txt`).
- C64 execution results: `make vice-lanec-ev-short`, 245 frozen cases
  (every entry, both missions) on a VICE PAL C64 (`lanec-ev-short-vice.txt`).
