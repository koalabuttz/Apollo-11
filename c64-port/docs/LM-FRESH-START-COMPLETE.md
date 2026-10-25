# Luminary FRESH_START_AND_RESTART: completion certificate

Lane A2 (restart, telemetry and support). One `PORT_SCOPE` certificate for `Luminary099/FRESH_START_AND_RESTART.agc`, aggregating the three milestones that translate it. Records: `evidence/lm-fresh-start-complete/`, written and checked by `tools/lm_fresh_start_complete.py` (it fails on any hash mismatch, ownership gap or overlap, oracle difference, unexecuted instruction or failed check; `--check` also fails if a record would change).

| Milestone | Owns | Native | Source map |
|---|---|---|---|
| `lm-fresh-start` | bank 05 2447-3401 (but STARTSIM), fixed-fixed 6001-6010 | `src/lm_fresh_start.c` | `docs/LM-FRESH-START.md` |
| `lm-v37` | bank 04 2024-2535, but R00's interpretive 2144-2146 | `src/lm_v37.c` | `docs/LM-V37.md` |
| `lm-stateup` | bank 13 2026-2067, bank 04 2144-2146 (interpretive) | `src/lm_stateup.c` | `docs/LM-STATEUP.md` |

Excluded: STARTSIM (05,2452-2455). It runs only if the STARTSW word is patched to `TCF STARTSIM`; in the rope it is `TCF SKIPSIM`, so no input reaches it.

## Records (one per category)

- Source-to-native map: `lm-fresh-start-complete-source-map.json` names the three source maps and native files with their sha256, and the ownership check: the 847 assembled words of the file are 843 owned by exactly one milestone plus the 4 STARTSIM words, with no gap or overlap. Per word (`lm-fresh-start-complete-line-map.json`): owner, kind (instruction, interpretive pair or operand, data), native location, and whether the oracle runs executed it.
- Independent assembly comparison: `lm-fresh-start-complete-comparison.json` names the three yaAGC proof records (`evidence/<milestone>/<milestone>-proof.json`, 12,288 cases). The tool reruns each oracle on the unmodified rope, built with the coverage trace (`tools/cert_trace.h`), and requires the output to equal the snapshot the milestone froze (which its freeze script checked byte-identical to the native output). The trace shows every one of the 655 translated instructions and interpretive pairs executed. Instructions after EXTEND or INDEX count with their prefix, because yaAGC runs them in the same step. Data words are compiled tables checked against the rope.
- Host and sanitizer results: `lm-fresh-start-complete-host.json` names the three host logs with their sha256, PASS lines and check counts: 13,251, 8,633 and 11,766 checks, no failures.
- C64 execution results: `lm-fresh-start-complete-c64.json` names the three VICE logs: 259, 136 and 256 checks, no failures. The rows are chosen by native branch outcome (`tools/a2_c64_rows.py`, over `tools/c64_row_cover.py`): each freeze fails unless its C64 rows take every gcov branch arc of every linked source that its 4,096 host cases take (392, 494 and 224 arcs), and records the counts in its proof. Each freeze also adds the first case of any entry slot the cover leaves out, so every registered entry runs on the C64. `lm-fresh-start-complete-c64-cover.json` is the c64_branch_outcomes record: it names the three milestones' `evidence/<milestone>/<milestone>-c64-cover.json` with their sha256 (each linked source with its sha256 and [host, rows] arc counts, the rows, the host cases and rows per entry slot) and fails on a missed host arc or a linked source changed since the cover was taken. The C64 record also names each VICE log's PRG sha256.

## What the certificate added to the milestones

The coverage trace found three gaps, now closed:

- R00's interpretive `CALL INTSTALL; DUMMYAD EXIT` (04,2144-2146) had no routine; `lm-v37` stopped at INTPRET. These are now two `lm-stateup` pairs, keyed in bank 04.
- PCLOOP's `+0` branch (`-PHASEn` equal to `PHASEn`) had no input. `lm-fresh-start` cases now include it.
- STATQUO (P20 or P25 with only its own bit of OCT500 in FLAGWRD0) had no input. `lm-v37` cases now include it.

The three milestones' vectors, proofs, host logs and VICE logs were refreshed.

## Cross-bank entries

Every label of the file reached from another bank is a registered `agc_transfer` routine: SLAP1, MR.KLEAN, P00KLEAN, V37KLEAN, GOPROG, ENEMA, GOPROG2, GOTOPOOH (`lm_fresh_start_routines`), GOP00FIX and V37 (`lm_v37_routines`), and STATEUP and THISVINT (`lm_stateup_interpretive`).
