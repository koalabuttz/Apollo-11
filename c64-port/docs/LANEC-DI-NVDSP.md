# Display start-up (DISPLAY_INTERFACE_ROUTINES)

Lane C. Milestone `lanec-di-nvdsp`.

## Scope

The code of `DISPLAY_INTERFACE_ROUTINES.agc` that starts a chosen display
on the DSKY (NVDSP, which the source also calls GOPLAY) and the PINBRANCH
re-entry before it: Luminary099 lines 1026-1142, Comanche055 lines
1048-1162 (PINBRNCH through V50PASTE). Both files are the same code at
different addresses (bank 10); Comanche055 spells PRIODBIT as BIT14.

As in `lanec-di-idle`, each case of `step` in `src/lanec_di_nvdsp.c` is one
basic block of the source, every block start is an entry, and control runs
from block to block until it reaches a boundary.

## Source map

| Source blocks | Boundary |
|---|---|
| PINBRNCH (RELINT, MARK2PAC back to MPAC +2, PINMASK test) | ERASER, MARKPLAY, NORMBNCH |
| NORMBNCH, NORMBNCH +2 (PRIODBIT test) | UPFLAG; KEEPPRIO or PLAYJUM1 |
| NVDSP, NVDSP +1 (blank bits into L, DSPFLG bit 13, TEM1, MPAC2SAV/MARK2PAC, CCS NVWORD), the MARKNV path (a TS overflow skips the MASK), NVDSP1 | COPYPACS, CLEANEND, NV50DSP |
| NV50DSP and its busy and done returns, the three DOWNFLAG returns | NVMONOPT, REST, FLASHOFF, COPYTOGO, DOWNFLAG |
| BLANKCHK, BLANKCHK +2 | BLANKSUB, NVDSP |
| PERFCHEK, GOANIDLE and its two later tests, ISITN00 | 1STOR2ND, FLASHSUB, ENDIT, ENDOFJOB |
| 1STOR2ND, 1STOR2ND +4 (ADS DSPFLG, ZL, BZMF, BIT12 test), V50PASTE | GOANIDLE, MARKPERF, NV50DSP |

Domain: COPINDEX 0-2 where a block indexes with it (it selects the PRIO,
MARK or NORM DSPFLG, NVWORD and CADRFLSH cells: NVDSP +1, GOANIDLE +8,
ISITN00, FIRSTOR2ND +4). Each of those blocks checks it, so PINBRNCH and the
other blocks run as the original does whatever COPINDEX holds (Pinball's
GOPIN reaches PINBRNCH with COPINDEX unset, `docs/LANEC-EV-TRANSFER.md`). The
blocks run before an indexing one change only A, which a refusal puts back
before returning `LANEC_OUT_OF_SCOPE`.

## Evidence

Harness: `tools/lanec/` (see `docs/LANEC-EV-FRONT.md`).
Hashes: `evidence/lanec-di-nvdsp/SHA256.json`.

- Independent assembly comparison: `lanec-di-nvdsp-diff.json` (3,600
  cases: all 25 block entries, both missions, every COPINDEX and 400 with
  COPINDEX above 2, of which 114 are refused with the state unchanged
  (`diff.py --refusals`), NVWORD of each
  sign including -0 and pastes, PINBRANCH flag combinations, the perform,
  flash, paste and second-part bits of TEMPOR2, DSPFLG values whose ADS
  gives -0, A with and without overflow) and `lanec-di-nvdsp-oracle.crc`.
  The cases take every source branch in `src/lanec_di_nvdsp.c` (gcov: 69
  of 76 branch outcomes; the others are the out-of-scope guards and the
  unreachable `default` cases).
- Host and sanitizer results: `make test-lanec-di-nvdsp sanitize-lanec-di-nvdsp`
  (`lanec-di-nvdsp-sanitize.txt`).
- C64 execution results: `make vice-lanec-di-nvdsp`, 32 frozen cases on a VICE PAL C64,
  picked by `tools/lanec/cover.py` so that together they take every
  branch arc the 3,600 host cases take (`lanec-di-nvdsp-vice.txt`).
