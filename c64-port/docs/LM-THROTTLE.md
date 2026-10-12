# Translate Luminary THROTTLE_CONTROL_ROUTINES and FASTCHNG as transfer-ABI routines

Lane F. Milestone `lm-throttle`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

## Scope

- `Luminary099/THROTTLE_CONTROL_ROUTINES.agc`, all of it: bank 31
  `02216`-`02417`, 130 assembled words. Nothing is left in the file.
- `Luminary099/LUNAR_LANDING_GUIDANCE_EQUATIONS.agc` FASTCHNG, `31,3740`-`3745`
  (6 words). The rest of that file is open.

`src/lm_throttle.c` registers nine routines in `lm_throttle_routines`. Each
runs from its entry to the next transfer, and every source instruction is one
`agc_image_ops` call (register effects checked against yaAGC by
`tools/agc_image_ops_differential.py`), named with its erasable address or the
source's fixed constant. TS overflow skips are followed as the original does.

| Entry | Body |
|---|---|
| THROTTLE `31,2216` | A = ABDELV x /AF/CNST, then THROTTLE +3 |
| THROTTLE +3 `31,2221` | return saved in RTNHOLD; AFDUMP `TC MASSMULT` |
| `31,2224` (MASSMULT return) | FP = present thrust; `TC MASSMULT` for /AFC/ |
| `31,2230` (MASSMULT return) | FC, FCODD, the 3-second FWEIGHT test, WHERETO (LOWCRIT/HIGHCRIT from EBANK5), the LOWFCOLD/FCOMPSET/FLATOUT1/FLATOUT2 bands, `TC FASTCHNG` at DOPIF, or straight on when `TS PIFPSET` overflows |
| `31,2303` (FASTCHNG return) | FCOLD, PIF = FC - FP, DOIT: PSEUDO55, THRUST, channel 14 bit 4, TTHROT; the P66 or 2-second FWEIGHT compensation; `TC RTNHOLD` |
| FLATOUT `31,2370`, WHATOUT `31,2371` | PIFPSET, FCOLD = PIF = -0, return to RTNHOLD, DOIT |
| MASSMULT `31,2400` | MPAC x MASS x SCALEFAC, TPAGREE, the POSMAX limit; `TC BUF` |
| FASTCHNG `31,3740` | PHSNAME3 = return; `TC A` |

MASSMULT's `TC DMP` (twice) and `TC TPAGREE` call the INTERPRETER's fixed-fixed
basic subroutines DMP/DMPSUB (`07103`) and TPAGREE with BRANCH (`07257`,
`06723`). They are compiled inline at the call sites, with the caller's ADRES
word (MASS, SCALEFAC) as a constant, and end with the same Q and L as the
original returns (`TC Q`: Q = 3, `TC L`: Q = 2). This milestone does not claim
INTERPRETER coverage; Lane A owns that file and may register DMP/TPAGREE as
routines later.

Channel 14 is the device output: `lm_flight_io.chan14` (`include/lm_flight_io.h`),
which carries `p66_bias_io` first so Executive routines can share the record.
`WOR CHAN14` sets bit 4, the thrust drive, as the original does.

Refused (`EXEC_OUT_OF_SCOPE`, image unchanged), with no original producer:

- any entry outside bank 31 or under an EB other than EBANK7 (the callers run
  under `EBANK= PIF`/`E2DPS`; under another EB the stored return and RTNHOLD
  would be in different banks);
- a return word (Q at entry, RTNHOLD, BUF, A at FASTCHNG's `TC A`) that is not a
  TC into fixed memory, or a FASTCHNG Q with its sixteenth bit set.

Branches no admitted input reaches, documented in the host test: `TS THRUST`
skipping after a `TS PSEUDO55` of the same unoverflowed A; `TS FC` and
`TS FCOLD` of a 15-bit word; `TS PHSNAME3` of a fixed return; WHATOUT's
`TS PIFPSET` (A not overflowed, and no caller enters WHATOUT except FLATOUT).

## Source map

Every assembled word has one native owner (`tools/lm_throttle_source.py`, using
the shared checker `tools/lm_basic_source.py`;
`evidence/lm-throttle/lm-throttle-source-map.json`). After the C preprocessor,
the owner and the helpers it calls hold each erasable address (the high word of
a double-word order), the rope word of each fixed operand, each branch target
or the inline composition named in the tool, channel 14, and each constant.
Changing any constant or address in `src/lm_throttle.c` fails the check.

| Address | Native owner |
|---|---|
| 31,2216-2220, 31,2417 (/AF/CNST) | `throttle` |
| 31,2221-2223 | `throttle3_body` |
| 31,2224-2227 | `present` |
| 31,2230-2302 | `desired` |
| 31,2303-2311 | `pif` |
| 31,2312-2367 (DOIT .. THDUMP) | `doit` |
| 31,2370 | `flatout` |
| 31,2371-2377 | `whatout_body` |
| 31,2400-2416 | `massmult` (with `dmp`, `tpagree`) |
| 31,3740-3745 | `fastchng` |

## Evidence

Hashes: `evidence/lm-throttle/SHA256.json`; all four files are kept in the tree.

- **Independent assembly comparison.** `make lm-throttle-differential` runs the
  unmodified Luminary099 rope in yaAGC (host-only, `tools/lm_throttle_reference.c`).
  Case n enters THROTTLE, THROTTLE +3, FLATOUT or WHATOUT (n mod 4) from a
  random image with FB bank 31 and EB 7, random channel 14 bits 1-5, MODREG 66
  in a quarter of the cases, and a caller return in Q: SWRETURN, a fixed-fixed
  address in `06000`-`06677` (below BRANCH, DMP and TPAGREE, which MASSMULT
  passes through) or a bank-31 address outside the routines. A THROTTLE +3
  cohort sets /AFC/ next to the acceleration in A,L so PIF is small (all-zero
  MPAC for TPAGREE, and a +0 high word for `CCS PIF`). All 16,384 equal the
  native run in channel 14 and all 2,048 erasable words; per-case digests are
  frozen in `tests/lm_throttle_vectors.h`. Results:
  `evidence/lm-throttle/lm-throttle-proof.json`.
- **Host and sanitizer results.** `make test-lm-throttle sanitize-lm-throttle`
  (`evidence/lm-throttle/lm-throttle-host.log`) checks the digests, that every
  case reaches its caller's return, that all nine routines and every reachable
  branch are taken (counted through `LM_THR_PATH` in the test build), the
  refusals above, and runs the source map.
- **C64 execution results.** `make vice-lm-throttle`
  (`evidence/lm-throttle/lm-throttle-vice.log`) runs every thirty-second case
  (512) through the trampoline in VICE against the frozen digests.
