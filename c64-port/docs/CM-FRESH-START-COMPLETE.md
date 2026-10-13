# Comanche FRESH_START_AND_RESTART: completion certificate

Lane A2 (restart, telemetry and support). One `PORT_SCOPE` certificate for `Comanche055/FRESH_START_AND_RESTART.agc`, aggregating the three milestones that translate it. Records: `evidence/cm-fresh-start-complete/`, written and checked by `tools/cm_fresh_start_complete.py` (it fails on any hash mismatch, ownership gap or overlap, oracle difference, unexecuted instruction or failed check; `--check` also fails if a record would change). It is the Comanche counterpart of `docs/LM-FRESH-START-COMPLETE.md`.

| Milestone | Owns | Native | Source map |
|---|---|---|---|
| `cm-fresh-start` | bank 05 2342-3217 (but STARTSIM), fixed-fixed 4106-4113 | `src/cm_fresh_start.c` | `docs/CM-FRESH-START.md` |
| `cm-v37` | bank 04 2000-2565, but R00's interpretive 2131-2133; bank 20 2000-2064 (VAC5STOR) | `src/cm_v37.c` | `docs/CM-V37.md` |
| `cm-stateup` | bank 13 2000-2035, bank 04 2131-2133 (interpretive) | `src/cm_stateup.c` | `docs/CM-STATEUP.md` |

Excluded: STARTSIM (05,2345-2350). It runs only if the STARTSW word is patched to `TCF STARTSIM`; in the rope it is `TCF SKIPSIM`, so no input reaches it.

## Records (one per category)

- Source-to-native map: `cm-fresh-start-complete-source-map.json` names the three source maps and native files with their sha256, and the ownership check: the 893 assembled words of the file are 889 owned by exactly one milestone plus the 4 STARTSIM words, with no gap or overlap. Per word (`cm-fresh-start-complete-line-map.json`): owner, kind (instruction, interpretive pair or operand, data), native location, and whether the oracle runs executed it.
- Independent assembly comparison: `cm-fresh-start-complete-comparison.json` names the three yaAGC proof records (`evidence/<milestone>/<milestone>-proof.json`, 12,288 cases). The tool reruns each oracle on the unmodified rope, built with the coverage trace (`tools/cert_trace.h`, with Comanche's NEWOPS 06037 for the interpretive pairs), and requires the output to equal the snapshot the milestone froze (which its freeze script checked byte-identical to the native output). The trace shows every one of the 667 translated instructions and interpretive pairs executed. Instructions after EXTEND or INDEX count with their prefix, because yaAGC runs them in the same step. Data words are compiled tables checked against the rope.
- Host and sanitizer results: `cm-fresh-start-complete-host.json` names the three host logs with their sha256, PASS lines and check counts: 8,965, 12,965 and 11,722 checks, no failures.
- C64 execution results: `cm-fresh-start-complete-c64.json` names the three VICE logs: 259, 169 and 242 checks, no failures. The rows are chosen by native branch outcome (`tools/a2_c64_rows.py`, over `tools/c64_row_cover.py`): each freeze fails unless its C64 rows take every gcov branch arc of every linked source that its 4,096 host cases take (428, 510 and 219 arcs), and records the counts in its proof. Each freeze also adds the first case of any entry slot the cover leaves out, so every registered entry runs on the C64. `cm-fresh-start-complete-c64-cover.json` is the c64_branch_outcomes record: it names the three milestones' `evidence/<milestone>/<milestone>-c64-cover.json` with their sha256 (each linked source with its sha256 and [host, rows] arc counts, the rows, the host cases and rows per entry slot) and fails on a missed host arc or a linked source changed since the cover was taken. The C64 record also names each VICE log's PRG sha256.

## What the certificate added to the milestones

- PCLOOP's `+0` branch (05,2701, `-PHASEn` equal to `PHASEn`) had no input. `cm-fresh-start` cases now include it, its freeze now fails on any unexecuted instruction, and its C64 rows are chosen by outcome.
- CANV37 (04,2124), the POSTJUMP target of SERVICER207's SERVEXIT, was not a registered entry. It is now a `cm-v37` routine.
- R00's interpretive `CALL INTSTALL; DUMMYAD EXIT` (04,2131-2133) and bank 13 are `cm-stateup`.

The milestones' vectors, proofs, host logs and VICE logs were refreshed.

## Cross-bank entries

Every label of the file reached from another bank is a registered `agc_transfer` routine: SLAP1, MR.KLEAN, GOPROG, ENEMA, T5IDLOC and GOTOPOOH (`cm_fresh_start_routines`), V37, CANV37 and VAC5STOR (`cm_v37_routines`), and STATEUP and THISVINT (`cm_stateup_interpretive`). GOP00FIX (the CADR at 4111), P00KLEAN and V37KLEAN (CADRs at 04,2206-2212) are referenced inside the file and are registered too. OCT24 and OCT31 (4112-4113) are constants other files read.
