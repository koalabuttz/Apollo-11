# Translate Luminary TJET_LAW as a transfer-ABI routine

Lane F. Milestone `lm-tjet-law`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

## Scope

`Luminary099/TJET_LAW.agc`, all of it: bank 17 `03252`-`03743`, 314
assembled words. Nothing is left in the file.

`src/lm_tjet_law.c` registers one routine, TJETLAW (`17,3252`), in
`lm_tjet_law_routines`. It runs from the caller's `TC TJETLAW` to `TC HOLDQ`,
and every source instruction is one `agc_image_ops` call (register effects
checked against yaAGC by `tools/agc_image_ops_differential.py`), named with
its erasable address or the source's fixed constant.

| Part | Body |
|---|---|
| entry | HOLDQ = Q; ADRSDIF1/2 from AXISDIFF by AXISCTR; NEGEDOT mirrors E, EDOT and the indexers for a negative rate; ROTSENSE = +-SENSOR |
| SCALEE, SCALEDOT | E and EDOT rescaled; the rough laws when either is too large |
| ERRTEST, SENSTEST, MAXJETS, TJCALC | the 3-degree offset, the FIREDB test, NUMBERT, FIREFCT |
| ZONE4,5, JETSON, DRIFT/ON, COASTTJ | coast or fire in zones 4 and 5, the minimum-impulse FLAT band |
| ZONE5, FORMULA1, FORMULA2 | the zone 5 parabola with ACCFCTZ5 and 1/ANET2 |
| ZON1,2,3, ZONE1, ZONE2, ZONE3, FORMULA3, CHKMINTJ | zones 1-3, TJMAX and TJMIN |
| RUFLAW1, RUFLAW2, RUFLAW3, SMALRATE, RUFLAW12 | the rough laws for large errors |
| FULLTIME, RETURNTJ | TJETU = jet time x ROTSENSE, NUMBERT 4 when ACCSWU says so; `TC HOLDQ` |

The subroutines Z123COMP and RUFSETUP (also entered at RUFSETUP +2) are only
called by `TC` from inside the file. They are compiled as the helpers
`z123comp` and `rufsetup` and leave Q as the original's `TC Q` does (Q = 3).
`TC CCSHOLE` (ALARM_AND_ABORT) is a transfer.

INDEX on the erasable indexers AXISCTR, ADRSDIF1 and ADRSDIF2 adds the
indexer's signed value to the operand, as the original's instruction-word
sum does. TS overflow skips are followed. When `TS FIREFCT` overflows it skips
the EXTEND before `BZMF ZON1,2,3`, so the BZMF word runs as `AD 03542` (the
rope word there is `TC Z123COMP`, 03526); the routine composes that AD. When
`CCS FLAT` sees -0 it lands past `INDEX ADRSDIF1`, so `AD AXISDIST` runs
unindexed, as in the original.

Refused (`EXEC_OUT_OF_SCOPE`, image unchanged), with no original producer:

- an entry outside bank 17 or under an EB other than EBANK6 (`EBANK= TJP`);
- an AXISCTR other than -1, 0 or +1 (the P-axis and Q,R-axis callers set only
  these; another value would make `INDEX AXISCTR` read outside AXISDIFF and
  TJETU);
- a return in Q that is not a TC into fixed memory.

## Verification

`tests/lm_tjet_law_cases.c` builds 16,384 cases, one axis per case
(`n % 3`). Each starts from a random image with FB bank 17, EB 6 and random
A and L, and runs through the trampoline to the caller's return in Q
(SWRETURN, fixed-fixed `06000`-`06777`, or bank 17 outside the file). Seven
eighths of the cases put E, EDOT and the 1/ACCS parameter block in the
fine-law ranges, with +0 and -0 mixed in. One in 64 sets up an exact
`RUFRATE + EDOT = 1/8 / (1/ANET1 +2)` so RUFLAW12's `BZF FULLTIME` is taken.
The rest are fully random, which drives the rough laws and the overflow
paths.

- `make lm-tjet-law-differential` runs `tools/lm_tjet_law_freeze.py`. It
  builds the yaAGC oracle `tools/lm_tjet_law_reference.c`, runs every case
  in the unmodified rope to the same return, and requires the native
  snapshot to be byte-identical in all 2,048 erasable words and A, L, Q. The
  digests are frozen into `tests/lm_tjet_law_vectors.h`, and every
  thirty-second into `tests/lm_tjet_law_c64_vectors.h`.
- `make test-lm-tjet-law` and `make sanitize-lm-tjet-law` check every
  digest (strict and ASAN/UBSAN), require every case to reach the caller's
  return, require every reachable branch to be taken, and check the
  refusals.
- `make vice-lm-tjet-law` runs every thirty-second case (512) on the C64 in
  VICE.

Branches no admitted input reaches, documented in the host test:

- NEGEDOT's `TS ADRSDIF2` overflow skip (the index is one word);
- ERRTEST's `CCS E` at -0 (yaAGC's `MP L` of the -0 low word that a -0 E
  leaves gives +0);
- `CCS ROTSENSE` at +0 or -0 in ZONE5 and SMALRATE (ROTSENSE is +-SENSOR);
- `CCS EDOT` negative in ZONE3 (EDOT is positive or -0 by then).

These are the `TC CCSHOLE` exits and their -0 neighbours.

## Source map

Every assembled word has one native owner (`tools/lm_tjet_law_source.py`,
using the shared checker `tools/lm_basic_source.py`;
`evidence/lm-tjet-law/lm-tjet-law-source-map.json`). After the C
preprocessor, the owner and the helpers it calls hold several things:

- each erasable address (for an INDEX order, the indexed base);
- the rope word of each fixed operand;
- each branch target, or the inline composition named in the tool;
- each constant, including AXISDIFF and the 2DEC `.00375A8`.

| Address | Native owner |
|---|---|
| 17,3252-3525, 17,3542-3711, 17,3721-3743 | `tjetlaw` |
| 17,3526-3541 (Z123COMP) | `z123comp` |
| 17,3712-3720 (RUFSETUP) | `rufsetup` |

## Evidence

Hashes: `evidence/lm-tjet-law/SHA256.json`; all four files are kept in the tree.
