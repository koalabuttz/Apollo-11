# Translate Comanche PHASE_TABLE_MAINTENANCE as transfer-ABI routines

Lane A2 (restart, telemetry and support), split from Lane A. Milestone `cm-phase-table`. Interface: `docs/NATIVE_TRANSFER_ABI.md` (owned by Lane A; this milestone adds routines and operands only, no ABI change).

Comanche055 PHASE_TABLE_MAINTENANCE keeps the major-mode display and the restart phase tables. Each of its entries is a native routine of the control-transfer ABI. Its callers pass their arguments as words after the `TC`, so those words are compiled from the unmodified source listing into a site table (`src/cm_phase_sites.inc`, `tools/cm_phase_sites_generate.py`) and checked against the rope; native code never reads the rope.

## Scope

- `Comanche055/PHASE_TABLE_MAINTENANCE.agc`, every instruction:
  - FFTAG1: NEWMODEX, NEWMODEA, MMDSPLAY, PREBJUMP (LXCH BBANK, TCF BANKJUMP with CADR SETUPDSP), CHECKMM, 2PHSCHNG, PHASCHNG, ONEORTWO, OLDPRIO, CON1, GETPRIO, GETNEWNM, TOCON2 and CHECKB.
  - Bank 10 (PHASETAB): SETUPDSP and its NOVAC return, PHSCHNG2, BELOW1-BELOW4, CON2.
  - The constants the file reads as data (CADR SETUPDSP, ADRPCHN2, CON2ADR, BIT14COM) are words of this file and checked against the listing.
- Inline operands: the major mode after every `TC NEWMODEX` and `TC CHECKMM`; the packed phase word after every `TC PHASCHNG` (and the extra group word first after `TC 2PHSCHNG`) followed by a new priority or delta time and a new 2CADR when the type and phase bits ask for them; and the 2CADR DSPMMJOB after SETUPDSP's own `TC NOVAC`.
- External continuations: BANKJUMP (Lane A's `cm_interbank_routines`), NOVAC and the DSPMMJOB job, CHECKMM's Q+1/Q+2 exits in INTERPRETER, and each caller's return. They resolve to `EXEC_UNMAPPED` (or the registered interbank routine) as for any other file.

Paths with no producer: no caller's phase word sets both TBASE and LONGBASE (TEMPSW 060000). The native code follows the source there (BELOW2 sets TBASE, then the -0 from `AD BIT14COM` runs BELOW3), but no case reaches it, so the oracle does not compare it. The two "can't get here" branches of the source cannot be reached by any input; the native code refuses those CCS classes with `EXEC_OUT_OF_SCOPE` rather than invent behaviour. The legacy `cm_phase_cleanup` module (MR.KLEAN) is not touched.

## Source map

| AGC | Native |
|---|---|
| NEWMODEX 5354, NEWMODEA 5357, MMDSPLAY 5360, PREBJUMP | `mode` in `src/cm_phase_table.c` (LXCH BB, Z = BANKJUMP, A = CADR SETUPDSP) |
| CHECKMM 5364 | `checkmm` (to Q+1 or Q+2) |
| 2PHSCHNG 5372, PHASCHNG 5412, ONEORTWO, OLDPRIO, CON1, GETPRIO, GETNEWNM, TOCON2, CHECKB | `phaschng_entry`, `phschng2` |
| SETUPDSP 10,2440 and its NOVAC return 10,2446 | `setupdsp`, `setupdsp_return` |
| PHSCHNG2 10,2451, BELOW1-BELOW4 (final DTCB) | `phschng2`, `below1` |
| CON2 10,2537 | `con2` |
| Words after each `TC NEWMODEX/CHECKMM/PHASCHNG/2PHSCHNG`, 2CADR DSPMMJOB | `cm_phase_table_operands` (`src/cm_phase_sites.inc`) |

Each routine follows the source one instruction at a time with `agc_image_ops`. The routines need the mission's `cm_native_io` as io (INHINT/RELINT is `interrupt_enabled`; `ROR SUPERBNK` reads `superbank`), refuse a Q that is no compiled site, and refuse PHSCHNG2/CON2 entered with any FB other than bank 10.

## Evidence

Hashes: `evidence/cm-phase-table/SHA256.json`. Bulk files are assets of the `evidence-cm-phase-table` release (`python3 tools/evidence.py fetch cm-phase-table`).

- Independent assembly comparison: `evidence/cm-phase-table/cm-phase-table-proof.json`. `make cm-phase-table-differential` runs the unmodified Comanche055 rope (assembled from this repository by `tools/setup_cloud.sh`) in yaAGC (host-only). Case n enters from caller site n mod 164 (162 sites, then NEWMODEA with a random mode in A and MMDSPLAY from random banks and superbanks) with random erasable contents, EB and interrupt state; CHECKMM finds a matching MODREG in half its cases. Each case runs native until no table owns the next point and the oracle until it reaches the same point, then compares all 2,048 erasable words, the interrupt-enable state and SETUPDSP's NOVAC request (A, L, Q and the 2CADR). The digests are frozen into `tests/cm_phase_table_vectors.h`. `evidence/cm-phase-table/cm-phase-table-sites.json` lists every operand word and site checked against the source and rope.
- Host and sanitizer results: `make test-cm-phase-table sanitize-cm-phase-table` (`evidence/cm-phase-table/cm-phase-table-host.log`). Beyond the 4,096 oracle cases: both CHECKMM exits taken, every one of the ten registered entries reached, and refusals for a non-site Q, a missing io and PHSCHNG2 outside bank 10.
- C64 execution results: `make vice-cm-phase-table` (`evidence/cm-phase-table/cm-phase-table-vice.log`), every sixteenth case.
