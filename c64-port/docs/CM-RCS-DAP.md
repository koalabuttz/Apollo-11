# Translate Comanche RCS-CSM DIGITAL AUTOPILOT

Lane G (CM flight). Milestone `cm-rcs-dap`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/RCS-CSM_DIGITAL_AUTOPILOT.agc` (`src/cm_rcs_dap.c`) is native C in full: The CSM RCS autopilot: REDORCS, RCSATT with the rate filter loops (DRHOLOOP, ADOTLOOP, DELOOP), the needles (NEEDLER, NEEDLE11, NEEDLER2, NEEDLES), REINIT, ZEROJET, T6PROGM and STICKCHK. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

This replaces the earlier hand translation, which the certificate reviewer rejected in #84: its comparison was a listing check rather than a yaAGC run, and none of its entries was a routine of the control-transfer ABI. Now every entry another file names is a registered routine, and the comparison runs the original rope in yaAGC.

## Scope

17 routines, keyed by physical address in `cm_rcs_dap_basic`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 21,2105 REDORCS | basic | entry REDORCS (FRESH_START_AND_RESTART.agc 2CADR REDORCS) |
| 21,2122 RCSATT | basic | entry RCSATT (FRESH_START_AND_RESTART.agc 2CADR RCSATT) |
| 21,2236  | basic | return from SMALLMP |
| 21,2360  | basic | return from NOVAC |
| 21,2401  | basic | return from DPADD |
| 21,2420 NEEDLER | basic | entry NEEDLER (P11.agc CADR NEEDLER) |
| 21,2443 NEEDLE11 | basic | entry NEEDLE11 (EXTENDED_VERBS.agc CADR NEEDLE11) |
| 21,2462 NEEDLER2 | basic | entry NEEDLER2 (EXTENDED_VERBS.agc CADR NEEDLER2) |
| 21,2476 NEEDLES | basic | entry NEEDLES (EXTENDED_VERBS.agc CADR NEEDLES) |
| 21,2533  | basic | return from BIT4 |
| 21,2534 REINIT | basic | entry REINIT (AUTOMATIC_MANEUVERS.agc TCF REINIT) |
| 21,2550  | basic | return from IBNKCALL |
| 21,2552  | basic | return from IBNKCALL |
| 21,2616  | basic | return from NOVAC |
| 21,2632 ZEROJET | basic | entry ZEROJET (P11.agc CADR ZEROJET) |
| 21,3067 T6PROGM | basic | entry T6PROGM (AUTOMATIC_MANEUVERS.agc TCF T6PROGM) |
| 21,3130 STICKCHK | basic | entry STICKCHK (P11.agc CADR STICKCHK) |

Destinations outside the file: AHFNOROT, BIT4, DPADD, IBNKCALL, JETS, NOVAC, RESUME, SMALLMP, T6PROG. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. SPNDX 0-2 and DPNDX 0, 2, 4 (the axis loops), ATTKALMN from -3 (JETSLECT's translation gains) to 10 (the Kalman initialisation count), and RMANNDX/PMANNDX/YMANNDX 0-3 as STICKCHK leaves them. GAIN1/GAIN2 are read at -3 to +10, DACLIMIT at -1 to +1, FREETAU at 0 to 3.

## Evidence

Hashes: `evidence/cm-rcs-dap/SHA256.json`; all files are in the tree.

- **Source map.** `cm-rcs-dap-source-map.json` lists all 611 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-rcs-dap-differential` runs 256 states per routine (4,352 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC with hardware DV and without the IMU/optics device block (tools/prepare_flight_device_engine.py), its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state, the I/O channels 12, 13, 14, 31 and the 31A/31B/32 trap flags. The cases and the stops reached per routine are in `cm-rcs-dap-proof.json` (`cm-rcs-dap-differential.log` is the run). One case in 32 is frozen in `tests/cm_rcs_dap_vectors.h`.
- **Host and sanitizer results.** `make test-cm-rcs-dap sanitize-cm-rcs-dap` runs the 136 frozen cases (`cm-rcs-dap-host.log`).
- **C64 execution results.** `make vice-cm-rcs-dap` runs 2 diagnostic programs with 72 cases (`cm-rcs-dap-vice.log`, which starts with each PRG's SHA-256). The cases are chosen per routine so the C64 runs reach every compiled label the host cases reach: 474 of 474.
