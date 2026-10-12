# Translate Luminary PHASE_TABLE_MAINTENANCE as transfer-ABI routines

Lane A2 (restart, telemetry and support), split from Lane A. Milestone `lm-phase-table`. Interface: `docs/NATIVE_TRANSFER_ABI.md` (owned by Lane A; this milestone adds routines and operands only, no ABI change).

Luminary099 PHASE_TABLE_MAINTENANCE keeps the major-mode display and the restart phase tables. Its instruction sequence is Comanche055's (`docs/CM-PHASE-TABLE.md`) except for the PHASCHNG entry: Luminary splits it into PHASCHNG (phase word inline) and PHSCHNGA (phase word in A), and 2PHSCHNG joins them at PHASJUMP. Each entry is a native routine of the control-transfer ABI. The words callers pass after their `TC` are compiled from the unmodified source listing into a site table (`src/lm_phase_sites.inc`, `tools/lm_phase_sites_generate.py`, which reuses the Comanche generator's decoder) and checked against the rope; native code never reads the rope.

## Scope

- `Luminary099/PHASE_TABLE_MAINTENANCE.agc`, every instruction:
  - FFTAG1: NEWMODEX, NEWMODEA, MMDSPLAY, PREBJUMP (LXCH BBANK, TCF BANKJUMP with CADR SETUPDSP), CHECKMM, 2PHSCHNG, PHASCHNG, PHSCHNGA, PHASJUMP, ONEORTWO, OLDPRIO, CON1, TOCON2, GETPRIO and GETNEWNM.
  - FFTAG6: CHECKB.
  - Bank 10 (PHASETAB): SETUPDSP and its NOVAC return, PHSCHNG2, BELOW1-BELOW4, CON2.
  - The constants the file reads as data (CADR SETUPDSP, ADRPCHN2, CON2ADR, BIT14COM) are words of this file and checked against the listing.
- Inline operands: the major mode after every `TC NEWMODEX` and `TC CHECKMM`; the packed phase word after every `TC PHASCHNG` (and the extra group word first after `TC 2PHSCHNG`) followed by a new priority or delta time and a new 2CADR when the type and phase bits ask for them; and the 2CADR DSPMMJOB after SETUPDSP's own `TC NOVAC`. The one `TC PHSCHNGA` site (FRESH_START_AND_RESTART V37RET) passes no inline word; its producer computes three phase words (2.0, 2.11 and 2.7 SPOT), all type C with no follow-on words.
- External continuations: BANKJUMP (Lane A's `lm_interbank_routines`), NOVAC and the DSPMMJB job, CHECKMM's Q+1/Q+2 exits in INTERPRETER, and each caller's return.

Paths with no producer: no caller's phase word sets both TBASE and LONGBASE (TEMPSW 060000), the source's two "can't get here" branches cannot be reached, and no PHSCHNGA producer overflows A. The native code refuses those with `EXEC_OUT_OF_SCOPE`. A PHSCHNGA phase word that asked for inline words would refuse too, since its site has none. The existing `src/phase.c` and `src/p66_major_mode.c` modules stay as they were.

## Source map

| AGC | Native |
|---|---|
| NEWMODEX 5311, NEWMODEA 5314, MMDSPLAY 5315, PREBJUMP | `mode` in `src/lm_phase_table.c` (LXCH BB, Z = BANKJUMP, A = CADR SETUPDSP) |
| CHECKMM 5321 | `checkmm` (to Q+1 or Q+2) |
| 2PHSCHNG 5327, PHASCHNG 5353, PHSCHNGA 5357, PHASJUMP 5363 | `phaschng_entry` |
| ONEORTWO, OLDPRIO, CON1, GETPRIO, GETNEWNM, TOCON2, CHECKB 7750 | `phschng2` |
| SETUPDSP 10,2213 and its NOVAC return 10,2221 | `setupdsp`, `setupdsp_return` |
| PHSCHNG2 10,2224, BELOW1-BELOW4 (final DTCB) | `phschng2`, `below1` |
| CON2 10,2312 | `con2` |
| Words after each `TC NEWMODEX/CHECKMM/PHASCHNG/2PHSCHNG`, 2CADR DSPMMJOB | `lm_phase_table_operands` (`src/lm_phase_sites.inc`) |

Each routine follows the source one instruction at a time with `agc_image_ops`. The routines need the mission's `p66_bias_io` as io (INHINT/RELINT is `interrupt_enabled`; `ROR SUPERBNK` reads `superbank`), refuse a Q that is no compiled site wherever they read an inline word, and refuse SETUPDSP, PHSCHNG2 and CON2 entered with any FB other than bank 10.

## Evidence

Hashes: `evidence/lm-phase-table/SHA256.json`. Bulk files are assets of the `evidence-lm-phase-table` release (`python3 tools/evidence.py fetch lm-phase-table`).

- Independent assembly comparison: `evidence/lm-phase-table/lm-phase-table-proof.json`. `make lm-phase-table-differential` runs the unmodified Luminary099 rope (assembled from this repository by `tools/setup_cloud.sh`) in yaAGC (host-only). Case n enters from caller site n mod 144 (142 sites, then NEWMODEA with a random mode in A and MMDSPLAY from random banks and superbanks) with random erasable contents, EB and interrupt state. The PHSCHNGA site takes the 2.0, 2.11 and 2.7 SPOT phase words in turn; CHECKMM finds a matching MODREG in half its cases. Each case runs native until no table owns the next point and the oracle until it reaches the same point, then compares all 2,048 erasable words, the interrupt-enable state and SETUPDSP's NOVAC request (A, L, Q and the 2CADR). The digests are frozen into `tests/lm_phase_table_vectors.h`. `evidence/lm-phase-table/lm-phase-table-sites.json` lists every operand word and site checked against the source and rope.
- Host and sanitizer results: `make test-lm-phase-table sanitize-lm-phase-table` (`evidence/lm-phase-table/lm-phase-table-host.log`). Beyond the 4,096 oracle cases: both CHECKMM exits taken, every one of the eleven registered entries reached, and refusals for a non-site Q, a missing io and PHSCHNG2 outside bank 10.
- C64 execution results: `make vice-lm-phase-table` (`evidence/lm-phase-table/lm-phase-table-vice.log`), every sixteenth case.
