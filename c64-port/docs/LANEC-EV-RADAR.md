# Radar test verbs (EXTENDED_VERBS, Luminary099)

Lane C. Milestone `lanec-ev-radar`.

## Scope

Luminary099 bank 43, R77 through R77END: V63 (R04, sample the rendezvous
or landing radar once a second and display the data), V78 (R77, the LR
spurious-return test, which shares R04's set-up) and V79 (R77END).
Comanche055 has no counterpart. As in `lanec-ev-short`, each `case` of
`step` in `src/lanec_ev_radar.c` is one basic block, every label and return
point is an entry, and control runs from block to block until it leaves
these bodies.

## Source map

| Source | Boundaries |
|---|---|
| R77, R04 (RDRUSECK, R04FLAG test, TESTXACT, UPFLAG) | RDRUSECK, ALM/END, TESTXACT, UPFLAG |
| R04Z (EBANK4, RSAMPDT, RTSTLOC, RFAILCNT, RADMODES LR position bits from channel 33) | R04K, R04A |
| R04A option display and its returns, R04X/R04B radar selection and power check (channel 33) | BANKCALL (GOMARKFR, GOMARK4), BLANKET, ENDOFJOB, R04END |
| R04C (channel 12 bit 14, RTSTMAX, WAITLIST RADSAMP), RELINT and the R77/RR/LR split | WAITLIST, GOPIN |
| R04RR, R04LR displays and returns, R04Y (pause and step RTSTDEX), R04K, R04L | BANKCALL (GOMARKF, DELAYJOB), R04END |
| R04END (stop sampling, channel 12, R04FLAG), R77END | BANKCALL (DELAYJOB), DOWNFLAG, ENDEXT, GOPIN |

Domain: R04X -1 and R04C +4 begin with a TS of the A their callers leave
(never overflowed); an overflowed A there is out of scope.

## Evidence

Harness: `tools/lanec/` (see `docs/LANEC-EV-FRONT.md`).
Hashes: `evidence/lanec-ev-radar/SHA256.json`.

- Independent assembly comparison: `lanec-ev-radar-diff.json` (2,448
  cases: all 51 entries, 48 variants each, R04FLAG set and clear, RR and LR
  selections in RTSTDEX under E4 and the current EB, channel 33 radar
  bits, OPTIONX +1 values) and `lanec-ev-radar-oracle.crc`. gcov: every
  branch outcome in `step` is taken.
- Host and sanitizer results: `make test-lanec-ev-radar sanitize-lanec-ev-radar`
  (`lanec-ev-radar-sanitize.txt`).
- C64 execution results: `make vice-lanec-ev-radar`, 223 frozen cases
  (every entry) on a VICE PAL C64 (`lanec-ev-radar-vice.txt`).
