# Translate Comanche JET_SELECTION_LOGIC (CM RCS jet selection)

Lane G. Milestone `cm-jet-select`.

## Scope

`Comanche055/JET_SELECTION_LOGIC.agc`, all of it: bank 17 `2552`-`3736`,
629 assembled words. That includes the XLNNDX, PYTABLE, XLN1NDX, RTABLE,
YZTABLE, NJET and DFTMAX tables, the RCS2CADR 2CADR, and the DELTATT3,
DELATT20, XLNMASK, PJETS, YJETS, ACRJETS, BDZJETS, BDRJETS, ACYJETS, =14MS,
=+.1SEC and =-.1SEC constants. Nothing is left in the file.

`cm_jet_select(image, io)` runs one entry to its exit. `io` holds channels
5 and 6 (WRITE and RAND of the pitch/yaw and roll jet words), 13 (WOR of
BIT15, the T6 enable) and 31 (read only: the translation hand-controller
bits `07700`).

| Entry | Reached from |
|---|---|
| `2552` JETSLECT | T5RUPT via T5LOC (set by RCS-CSM_DIGITAL_AUTOPILOT) |
| `3617` T6START | T6RUPT via T6ADDR |

Admitted state: FB bank 17 (`036000`), EB E6, BB `036006`, any raw A/L/Q,
erasable contents and channels, and:

- at JETSLECT, XTRANS -4..+4 (not -0). CCS XTRANS stores the translation
  index the XLNNDX table reads.
- at T6START, BLAST+1, BLAST1+1 and BLAST2+1 each 0, 4 or 11: the
  REPLACER/REPLACEP/REPLACEY offsets T6SETUP stores and `INDEX BLASTn +1;
  TC REPLACER` adds.

`CM_JET_SELECT_OK` leaves `Z` at RESUME, the only exit of both tasks.
Anything else is `CM_JET_SELECT_OUT_OF_SCOPE`, image and io unchanged.
`CM_JET_SELECT_FAULT` (an INDEX outside the file's tables) cannot occur
from an admitted entry.

### How it is translated

As in `cm-rcs-dap`: one qualified `agc_image_ops` primitive per original
word, under C labels named for the original branch targets. RAND, WRITE,
WOR and RXOR are written from the yaAGC definitions.

- INDEX words add the 2-bit translation indices (channel 31 fields or CCS
  XTRANS, 0..3), the 0..2 rate indices on the 0/3/6/9/12 bases, or the jet
  counts NRJETS/NPJETS/NYJETS (-3..+3) to the next word. INDEX reads of
  fixed memory come from native tables equal to the rope.
- T6START's three `INDEX BLASTn +1; TC REPLACER` words are one loop over
  the three blasts. Each sets Q to its return (`3663`, `3670`, `3675`) and
  runs REPLACER, REPLACEP or REPLACEY, which end in TC Q.
- `TS TIME5` at `2555` overflows with a large T5TIME. The skip runs
  `CAF DELATT20`. The other TS words cannot overflow: they store masks, MP
  results, table words, CCS results, small jet counts or times.

## Source map

`evidence/cm-jet-select/cm-jet-select-source-map.json` (`tools/cm_jet_select_source.py`)
records every word with its source line, rope check and native label. It
also matches against the rope:

- the 97 fixed-constant reads, to the native literals;
- RCS2CADR and the seven INDEX tables;
- the internal TC returns, to the native REPLACER loop. The file makes no
  external TC.

Independent assembly comparison: `evidence/cm-jet-select/cm-jet-select-assembly-comparison.json`
(`tools/cm_flight_assembly_comparison.py`).

## Evidence

Hashes: `evidence/cm-jet-select/SHA256.json`; bulk files in release
`evidence-cm-jet-select`.

- Oracle: `deps/cm-flight-fixture` (original CM rope `6e3150ae...`).
  - The runner stops at RESUME.
  - Channel 13 starts with its trap and device-only bits clear.
  - The scaler is held, so TIME5/TIME6 do not count.
- Host and sanitizer results: `make test-cm-jet-select sanitize-cm-jet-select`.
  - 65536 cases (`tests/cm_jet_select_cases.c`): JETSLECT in three of four,
    T6START in the fourth. TAU, TAU1, TAU2 cover the 14 ms and 0.1 s blast
    limits; RACFAIL, RBDFAIL and ACORBD each take all four CCS ways; a
    quarter of the cases have no translation command.
  - Every case reaches RESUME, and the two sides agree on all of them.
  - Native branch coverage is complete apart from the entry refusals
    (checked by the host test) and the unreachable fault.
  - Stream identity `cm-jet-select-source-stream.json`, deltas
    `cm-jet-select-reference.deltas.xz`, log `cm-jet-select-checks.log`.
- C64 execution results: `make vice-cm-jet-select`, 128 frozen rows.
  `cm-jet-select-results.json`.

## Open

ABI registration of the entries follows in the Lane G routine-table step,
after Lane A's ISWRETRN fix.
