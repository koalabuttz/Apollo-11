# Translate Comanche CM_ENTRY_DIGITAL_AUTOPILOT (CM entry DAP)

Lane G. Milestone `cm-entry-dap`.

## Scope

`Comanche055/CM_ENTRY_DIGITAL_AUTOPILOT.agc`, all of it: bank 15
`2434`-`3754` and bank 20 `3612`-`3737`, 807 assembled words. That includes
the ATDOTCAD, T5IDLER1 and T5IDLER2 2CADRs, the WAITLIST 2CADRs, the
YJETCODE and P/RJCODE jet tables and the dead-zone, timing and scaling
constants (NSEC through XS/360, CM/SWIC1, -CDUT+1). Nothing is left in the
file.

`cm_entry_dap(image, io)` runs one entry to its exit. `io` holds channels 5
(PYJETS) and 6 (ROLLJETS), which the file writes.

| Entry | Reached from |
|---|---|
| `2434` READGYMB | WAITLIST task, restart table |
| `2665` ATTRATES | T5RUPT via T5LOC (ATDOTCAD) |
| `3207` SETJTAG | WAITLIST task (P61-P67) |
| `3650` JETCALL | WAITLIST task (roll jet timing) |
| `2446`, `2476`; `2500`, `3056`, `3215`; `2507`, `3064`, `3642` | returns from FLUSHJET; PHASCHNG; WAITLIST |
| `2540`, `2543`, `2551`, `2607`, `2631`, `2641`, `2700`, `2712`, `2724` | returns from SPCOS/SPSIN |
| `3717` | return from IBNKCALL NEEDLER |
| bank 20 `3612` CM/DAPON, `3672` CM/DAPIC | POSTJUMP and BANKCALL from P62 |
| bank 20 `3616`, `3620`; `3627`; `3636`; `3716`; `3732` | returns from DOWNFLAG; FLUSHJET; BANKCALL DELAYJOB; WAITLIST; 2PHSCHNG |

Admitted state: EB E6, FB/BB for the entry's bank (`032000`/`032006` or
`040000`/`040006`), any raw A/L/Q, erasable contents and channels, and the
values the file itself keeps in three words:

- JETAG -1, +0 or +1. With -0, CCS JETAG at `2775` would fall into 3DDZ
  and its TC Q would run address 3.
- CMDAPMOD -1, +0, -0 or +1. CCS CMDAPMOD at `3104` indexes RREL with it.
- SW/NDX -13..+13 (or -0), the telemetry index CM/DUMPR keeps. DXCH
  ENDBUF -1 and TS ENDBUF +1 are indexed with it.

`CM_ENTRY_DAP_OK` leaves `Z` at TASKOVER, WAITLIST, PHASCHNG, 2PHSCHNG,
FLUSHJET, SPSIN, SPCOS, IBNKCALL, RESUME, DOWNFLAG, BANKCALL or POSTJUMP
with Q the TC's return. One more exit is bank 20's `2434` with Q `3715`:
when CM/DAPIC's TS CM/GYMDT overflows, it skips TC WAITLIST and the 2CADR
word `02434` runs as TC 2434. Anything else is
`CM_ENTRY_DAP_OUT_OF_SCOPE`, image and io unchanged.

### How it is translated

As in `cm-rcs-dap`: one qualified `agc_image_ops` primitive per original
word, under C labels named for the original addresses (`L20_` in bank 20).
TC to a label in the file sets Q and jumps. The TC Q subroutines
(ANGOVCOR, RATEAVG, 3DDZ, BIASEDZ with 4D/SDZ, 2D/SDZ) return through one
dispatch on the return address. TC to another file is a frontier. MSU,
WRITE and ROR LCHAN (L as channel 1) are written from the yaAGC
definitions.

- INDEX words: `INDEX A; TC +1` dispatches on two CM flag bits.
  ANGOVCOR and TRTAGXPI index LIMITS and HALFPR with an overflow sign.
  The jet tables are indexed with dead-zone tags and ROR results. CA RREL,
  CA ROLLTM and TS A are indexed with a CCS result or a masked bit, and the
  ENDBUF words with SW/NDX. Fixed reads come from native tables equal to
  the rope.
- TS words that can overflow take their skip:
  - TS COSM, SINO, COSO, SINM, CALFA and SALFA right after the
    SPCOS/SPSIN returns (raw A). A skipped EXTEND turns the next MP into
    MASK.
  - TS JETAG after the FLUSHJET return.
  - TS CM/SAVE (DOUBLE of SR).
  - TS QREL, RREL, PREL and JETEM around RATEAVG, and TS RAXERR.
  - TS JETEM +1 (the dead-zone tag), TS A in 4D/SDZ and EXDAP.
  - TS L in ANGOVCOR, CM/RCS and WHICHALF; TS -VT/180, XD/360, VDT/180
    and TON2.
  - TS CM/GYMDT in CM/DAPIC.

  The other TS words store loads, constants, masks, MP, MSU or DV results,
  CCS results or small counts.
- CM/DAPIC's `CCS A; AD -CDUT+1; TCF -2` loop runs up to about 1600
  passes. Its +0 way would spin forever; A is never +0 there, and the
  native code reports a fault instead of spinning.

## Source map

`evidence/cm-entry-dap/cm-entry-dap-source-map.json` (`tools/cm_entry_dap_source.py`)
records every word with its source line, rope check and native label. It
also matches against the rope:

- the 108 fixed-constant reads, to the native literals;
- the three 2CADRs and the four INDEX tables;
- the TC Q dispatch, to the returns of the TCs into the TC Q subroutines,
  which hold every TC Q word;
- the frontier returns, to the listing's external TCs.

Independent assembly comparison: `evidence/cm-entry-dap/cm-entry-dap-assembly-comparison.json`
(`tools/cm_flight_assembly_comparison.py`).

## Evidence

Hashes: `evidence/cm-entry-dap/SHA256.json`; bulk files in release
`evidence-cm-entry-dap`.

- Oracle: `deps/cm-flight-fixture` (original CM rope `6e3150ae...`).
  - The runner stops at the frontiers above.
  - The scaler is held, so TIME1, TIME5 and TIME6 do not count.
- Host and sanitizer results: `make test-cm-entry-dap sanitize-cm-entry-dap`.
  - 65536 cases (`tests/cm_entry_dap_cases.c`), covering every entry, with
    ATTRATES and its returns weighted.
  - Every case reaches an exit, and the two sides agree on all of them.
  - Native line coverage is complete apart from the refusals, the fault
    returns and the JETAG -0 fall into 3DDZ.
  - CCS ways and TS skips that were not sampled:
    - JETAG -0 (refused at entry).
    - CALFA +0/-0 at `3046` (TS A overflows only when |CALFA| > cos 45).
    - -VT/180 -0 (a zero MP product is +0).
    - CCS Q negative (Q is GETON1's return or +0).
    - The CM/DAPIC loop's +0.
    - The WHICHALF TS L and GETON1 TS VDT/180 overflows (VSQ/4API stays
      below 0.31, and DDOUBL of half of XD/360 stays below 1).
    - LCX/360 +0 at `3264` and TON1 -0 at `3615`, which are reachable but
      did not occur in the sample.
  - Stream identity `cm-entry-dap-source-stream.json`, deltas
    `cm-entry-dap-reference.deltas.xz`, log `cm-entry-dap-checks.log`.
- C64 execution results: `make vice-cm-entry-dap`, 136 frozen rows.
  `cm-entry-dap-results.json`.

## Open

ABI registration of the entries follows in the Lane G routine-table step,
after Lane A's ISWRETRN fix.
