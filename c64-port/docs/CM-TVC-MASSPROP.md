# Translate Comanche TVCMASSPROP (MASSPROP and FIXCW mass properties)

Lane G. Milestone `cm-tvc-massprop`.

## Scope

`Comanche055/TVCMASSPROP.agc`, all of it: bank 05 `3366`-`3520`, 91 assembled
words (61 executable, 30 data) on source lines 137-245. Nothing is left in the
file.

- `MASSPROP` (`05,3366`): ten passes load the LEM-off breakpoints (NOLEMVAL)
  or the LEM-on line (INTVALUE + 2·LEMMASS·SLOPEVAL) into VARST0..+9, the
  DXTEST descent-stage adjustment (DAPDATR1 negative), then FIXCW.
- `FIXCW` (`05,3422`), also entered directly by TVCEXECUTIVE through IBNKCALL:
  three outputs IXX, IAVG, IAVG/TLX from the slope chosen by the sign of the
  CSM weight delta, then WEIGHT/G, then `ENDMASSP TC Q`.

`cm_tvc_massprop(image)` runs from either entry to the source return. Admitted
entries: Z `3366` or `3422`, FB bank 05 (`012000`), EB E6 (`03000`), BB
`012006`; any raw 16-bit A/L, any Q, any erasable contents. Results:

- `CM_TVC_MASSPROP_OK`: the image as `ENDMASSP TC Q` leaves it. TC Q executes
  the TC word held in Q, so Z = the caller's return and Q = 3 (as the oracle
  shows and as Lane B recorded for Luminary).
- `CM_TVC_MASSPROP_NO_EXIT`: the source never returns (below).
- `CM_TVC_MASSPROP_OUT_OF_SCOPE`: entry outside the contract, image unchanged.
- `CM_TVC_MASSPROP_FAULT`: cannot occur (an INDEX value outside ±9 or a fixed
  operand outside the table); the loop counters are written only by the
  source's own CAF/CCS/DIM/TS words.

### How it is translated

Each original word is one call to the qualified basic-instruction primitives
(`src/agc_image_ops.c`) on the image, under a C label named for its address.
INDEX is the ones'-complement sum of the counter and the next word's address.
Every TS honours its overflow skip and every CCS its four-way skip, so the
detours the source takes with raw erasable contents are the original ones:

- `TS TEMP333` overflowing skips EXTEND, so the BZMF word at `3432` executes as
  `AD 3435` (adding the fixed word `51506`) before falling into `CAF NEG2`.
- `TS PSI333` overflowing would skip EXTEND and run DIM's word as `ADS PHI333`.
- `TS VARST0` overflowing reaches `TCF MASSPROP +1` with A = ±1, so PHI333
  becomes ±1; `TS IXX` and `TS WEIGHT/G` overflowing reach BOKKEP2.

Some of these detours cycle forever: e.g. LEMMASS large enough that pass 1
overflows sets PHI333 to 1 again on every pass. The native code detects this
exactly. Neither loop writes a word it reads (MASSPROP reads DAPDATR1, LEMMASS
and the fixed tables and writes PHI333 and VARST0-1..+9; PEGGY reads
VARST0-1..+7, TEMP333, DAPDATR1, LEMMASS, CSMMASS and writes IXX-1..+2,
PHI333, PSI333, WEIGHT/G), and A and L are overwritten before use, so the loop
state at the MASSPROP head is A and at the PEGGY head the PHI333/PSI333 pair.
A repeated head state means the source never leaves; the native code returns
`NO_EXIT` there. The oracle runner gives up at 4000 instruction steps; no
returning case needs more than a few hundred, and the 65536-state comparison
would fail if a native `NO_EXIT` case returned in the oracle or the reverse.

Unreachable but translated anyway: the TS skips at `3367`, `3423`, `3424`,
`3434` and `3457` (A there is a constant, a CCS result or ±1), CCS −0 at `3411`
and `3446` (counters are never −0 there), CCS negative/−0 at `3452` (A is
`MASK BIT14`). Line coverage of the native file over the corpus reaches every
other branch.

## Source map

| Original | Native |
|---|---|
| MASSPROP `3366`, LEMTEST `3370`, LEMNO `3374`, LEMYES `3377`, STOINST `3407` | `L3367`-`L3412` in `src/cm_tvc_massprop.c` |
| DXTEST `3413`-`3421` | `L3413` |
| FIXCW `3422`-`3434` | `L3422`-`L3433` |
| PEGGY `3435`-`3455`, ENDMASSP `3456` | `L3435`-`L3456` |
| BOKKEP2 `3457`-`3462` | `L3457` |
| NOLEMVAL, INTVALUE, SLOPEVAL, NEGBPW, DXITFIX `3463`-`3520` | `bank05[]`, `cm_tvc_massprop_fixed` |

Per-word map: `evidence/cm-tvc-massprop/cm-tvc-massprop-source-map.json`
(`tools/cm_tvc_massprop_source.py`).

Independent assembly comparison: `evidence/cm-tvc-massprop/cm-tvc-massprop-assembly-comparison.json`
(`tools/cm_flight_assembly_comparison.py`), every word yaYUL emits for the
unchanged file against the original Comanche055 rope, without reading the
native code.

## Evidence

Hashes: `evidence/cm-tvc-massprop/SHA256.json`. Bulk files are assets of the
`evidence-cm-tvc-massprop` release (`python3 tools/evidence.py fetch cm-tvc-massprop`).

- Oracle: `tools/cm_flight_prepare_reference.py` builds `deps/cm-flight-fixture`,
  the original CM rope with every Lane G source file taken from `Comanche055/`
  unchanged apart from yaYUL's offset-label indentation (one patch per file);
  the rope must equal `6e3150ae...` (the pinned byte-original CM rope).
  REENTRY_CONTROL is not in the list yet: the repository copy has a label
  transcription difference (`GETDLEWD`) that yaYUL rejects; it is handled with
  that file.
- Independent assembly comparison: `tools/cm_tvc_massprop_source.py` reads all
  91 words from the yaYUL listing, checks each against the assembled rope, maps
  each to its native construct, and checks the 32 native operand words against
  the rope.
- Host and sanitizer results: `make test-cm-tvc-massprop sanitize-cm-tvc-massprop`.
  `tools/cm_tvc_massprop_differential.py` runs all 65536 cases
  (`tests/cm_tvc_massprop_cases.c`: both entries; every DAPDATR1 CCS class with
  BIT13/BIT14 combinations; physical and raw masses; random A/L/Q, IRQ and
  unowned words) in the oracle and natively and compares every 2052-word row
  (59237 return, 6299 never return); the full-row SHA256 is pinned in
  `cm-tvc-massprop-source-stream.json`, and the oracle is kept as per-case
  deltas (`cm-tvc-massprop-reference.deltas.xz`). Log:
  `cm-tvc-massprop-checks.log`.
- C64 execution results: `make vice-cm-tvc-massprop` runs 128 frozen oracle
  rows (`tools/cm_tvc_massprop_freeze.py`; 105 returning with the whole image
  compared, 23 non-returning) on the C64. `cm-tvc-massprop-results.json`.

What this does not claim: TVCEXECUTIVE's IBNKCALL into FIXCW and the TVC/RCS
interrupt temporaries' ownership belong to the callers' milestones.
