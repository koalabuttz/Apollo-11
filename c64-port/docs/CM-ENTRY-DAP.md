# Translate Comanche CM_ENTRY_DIGITAL_AUTOPILOT

Lane G (CM flight). Milestone `cm-entry-dap`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/CM_ENTRY_DIGITAL_AUTOPILOT.agc` (`src/cm_entry_dap.c`) is native C in full: The entry autopilot: READGYMB and the body rates, the atmospheric and exo-atmospheric DAP modes, CM/RCS roll control, the FDAI needles and the CM/DUMPR telemetry file. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

This replaces the earlier hand translation, which the certificate reviewer rejected in #84: its comparison was a listing check rather than a yaAGC run, and none of its entries was a routine of the control-transfer ABI. Now every entry another file names is a registered routine, and the comparison runs the original rope in yaAGC.

## Scope

28 routines, keyed by physical address in `cm_entry_dap_basic`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 15,2434 READGYMB | basic | entry READGYMB (RESTART_TABLES.agc -2CADR READGYMB) |
| 15,2500  | basic | return from PHASCHNG |
| 15,2507  | basic | return from WAITLIST |
| 15,2540  | basic | return from SPCOS |
| 15,2543  | basic | return from SPSIN |
| 15,2551  | basic | return from SPCOS |
| 15,2607  | basic | return from SPSIN |
| 15,2631  | basic | return from SPSIN |
| 15,2641  | basic | return from SPCOS |
| 15,2665 ATTRATES | basic | 2CADR ATTRATES |
| 15,2700  | basic | return from SPCOS |
| 15,2712  | basic | return from SPSIN |
| 15,2724  | basic | return from SPSIN |
| 15,3056  | basic | return from PHASCHNG |
| 15,3064 EXDAP4 | basic | return from WAITLIST |
| 15,3207 SETJTAG | basic | entry SETJTAG (RESTART_TABLES.agc -2CADR SETJTAG) |
| 15,3215  | basic | return from PHASCHNG |
| 15,3642 JETACTN | basic | return from WAITLIST |
| 15,3650 JETCALL | basic | 2CADR JETCALL |
| 15,3717  | basic | return from IBNKCALL |
| 20,3612 CM/DAPON | basic | entry CM/DAPON (P61-P67.agc CADR CM/DAPON) |
| 20,3616  | basic | return from DOWNFLAG |
| 20,3620  | basic | return from DOWNFLAG |
| 20,3636  | basic | return from BANKCALL |
| 20,3672 CM/DAPIC | basic | entry CM/DAPIC (P61-P67.agc  CM/DAPIC) |
| 20,3715  | basic | return from 2434 |
| 20,3716  | basic | return from WAITLIST |
| 20,3732  | basic | return from 2PHSCHNG |

Destinations outside the file: 2434, 2PHSCHNG, BANKCALL, DOWNFLAG, IBNKCALL, PHASCHNG, POSTJUMP, RESUME, SPCOS, SPSIN, TASKOVER, WAITLIST. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. CMDAPMOD +1, +0, -0 or -1 and SW/NDX alternating the needles (+1 to +13) with the telemetry index (-13 to -1) in steps of three. YJETCODE, P/RJCODE, LIMITS and HALFPR are read at -1 to +1. 2D/SDZ's TC Q with Q = 3 (after CCS JETAG -0) runs RELINT and then the word in EB as TC 3000, as the AGC does; this needs EB = E6.

## Evidence

Hashes: `evidence/cm-entry-dap/SHA256.json`; all files are in the tree.

- **Source map.** `cm-entry-dap-source-map.json` lists all 813 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-entry-dap-differential` runs 256 states per routine (7,168 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC with hardware DV and without the IMU/optics device block (tools/prepare_flight_device_engine.py), its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state, the I/O channels 05, 06 and the 31A/31B/32 trap flags. The cases and the stops reached per routine are in `cm-entry-dap-proof.json` (`cm-entry-dap-differential.log` is the run). One case in 32 is frozen in `tests/cm_entry_dap_vectors.h`.
- **Host and sanitizer results.** `make test-cm-entry-dap sanitize-cm-entry-dap` runs the 224 frozen cases (`cm-entry-dap-host.log`).
- **C64 execution results.** `make vice-cm-entry-dap` runs 2 diagnostic programs with 79 cases (`cm-entry-dap-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 7,168 differential cases take: 1062 of 1062 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 802 of 802 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 669 of the 669 branch outcomes all the cases take (`c64_gcov_branches` in the proof).
