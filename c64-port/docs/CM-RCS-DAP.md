# Translate Comanche RCS-CSM DIGITAL AUTOPILOT

Lane G (CM flight). Milestone `cm-rcs-dap`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/RCS-CSM_DIGITAL_AUTOPILOT.agc` (`src/cm_rcs_dap.c`) is native C in full: The CSM RCS autopilot: REDORCS, RCSATT with the rate filter loops (DRHOLOOP, ADOTLOOP, DELOOP), the needles (NEEDLER, NEEDLE11, NEEDLER2, NEEDLES), REINIT, ZEROJET, T6PROGM and STICKCHK. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

This replaces the earlier hand translation, which the certificate reviewer rejected in #84: its comparison was a listing check rather than a yaAGC run, and none of its entries was a routine of the control-transfer ABI. Now every entry another file names is a registered routine, and the comparison runs the original rope in yaAGC.

## Scope

71 routines, keyed by physical address in `cm_rcs_dap_basic`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 21,2105 REDORCS | basic | entry REDORCS (FRESH_START_AND_RESTART.agc 2CADR REDORCS) |
| 21,2122 RCSATT | basic | entry RCSATT (FRESH_START_AND_RESTART.agc 2CADR RCSATT) |
| 21,2154  | basic | part key |
| 21,2216 DRHOLOOP | basic | part key |
| 21,2221  | basic | part key |
| 21,2222  | basic | part key |
| 21,2226  | basic | part key |
| 21,2236  | basic | return from SMALLMP |
| 21,2311 ADOTLOOP | basic | part key |
| 21,2317  | basic | part key |
| 21,2324  | basic | part key |
| 21,2350 KMATRIX | basic | part key |
| 21,2360  | basic | return from NOVAC |
| 21,2366 DELOOP | basic | part key |
| 21,2374  | basic | part key |
| 21,2400  | basic | part key |
| 21,2401  | basic | return from DPADD |
| 21,2407  | basic | part key |
| 21,2411 DACNDLS | basic | part key |
| 21,2417 KRESUME1 | basic | part key |
| 21,2420 NEEDLER | basic | entry NEEDLER (P11.agc CADR NEEDLER) |
| 21,2425  | basic | part key |
| 21,2443 NEEDLE11 | basic | entry NEEDLE11 (EXTENDED_VERBS.agc CADR NEEDLE11) |
| 21,2462 NEEDLER2 | basic | entry NEEDLER2 (EXTENDED_VERBS.agc CADR NEEDLER2) |
| 21,2465  | basic | part key |
| 21,2466  | basic | part key |
| 21,2471 NEEDLES3 | basic | part key |
| 21,2474  | basic | part key |
| 21,2476 NEEDLES | basic | entry NEEDLES (EXTENDED_VERBS.agc CADR NEEDLES) |
| 21,2477 DACLOOP | basic | part key |
| 21,2505  | basic | part key |
| 21,2533  | basic | return from BIT4 |
| 21,2534 REINIT | basic | entry REINIT (AUTOMATIC_MANEUVERS.agc TCF REINIT) |
| 21,2544 FRESHDAP | basic | part key |
| 21,2546 REDAP | basic | part key |
| 21,2550  | basic | return from IBNKCALL |
| 21,2552  | basic | return from IBNKCALL |
| 21,2562  | basic | part key |
| 21,2563  | basic | part key |
| 21,2603  | basic | part key |
| 21,2616  | basic | return from NOVAC |
| 21,2632 ZEROJET | basic | entry ZEROJET (P11.agc CADR ZEROJET) |
| 21,2660  | basic | part key |
| 21,2661 T5PHASE2 | basic | part key |
| 21,2674  | basic | part key |
| 21,2675 NOHIAUTO | basic | part key |
| 21,2722  | basic | part key |
| 21,2724 NOCHANGE | basic | part key |
| 21,2732  | basic | part key |
| 21,2755 SETWBODY | basic | part key |
| 21,2757  | basic | part key |
| 21,2760  | basic | part key |
| 21,2761  | basic | part key |
| 21,2772  | basic | part key |
| 21,3000 ZEROER | basic | part key |
| 21,3005 NORATE | basic | part key |
| 21,3015 MERUPDAT | basic | part key |
| 21,3023 SPNDXCHK | basic | part key |
| 21,3025  | basic | part key |
| 21,3051 FREEFUNC | basic | part key |
| 21,3067 T6PROGM | basic | entry T6PROGM (AUTOMATIC_MANEUVERS.agc TCF T6PROGM) |
| 21,3130 STICKCHK | basic | entry STICKCHK (P11.agc CADR STICKCHK) |
| 21,3146 KALUPDT | basic | part key |
| 21,3156  | basic | part key |
| 21,3160 FDAIDSP2 | basic | part key |
| 21,3171  | basic | part key |
| 21,3175 FDAITOTL | basic | part key |
| 21,3203  | basic | part key |
| 21,3205 GETAKS | basic | part key |
| 21,3233  | basic | part key |
| 21,3243 WRTN17 | basic | part key |

The step machine is split into part functions (spec `basic_parts`) so each routine's link closure fits its flight-store slot. A transfer across a part hands Z to the target word, whose routine is a part key, and the transfer runner runs it as the next step. Part keys are not entries: their runs come from the 17 entry routines, which draw all the cases, so the case set and the host vectors are the same as before the split.

Destinations outside the file: AHFNOROT, BIT4, DPADD, IBNKCALL, JETS, NOVAC, RESUME, SMALLMP, T6PROG. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. SPNDX 0-2 and DPNDX 0, 2, 4 (the axis loops), ATTKALMN from -3 (JETSLECT's translation gains) to 11 (the Kalman initialisation count, CAF ELEVEN at 21,2571), and RMANNDX/PMANNDX/YMANNDX 0-3 as STICKCHK leaves them. GAIN1/GAIN2 are read at -3 to +11, DACLIMIT at -1 to +1, FREETAU at 0 to 3.

## Evidence

Hashes: `evidence/cm-rcs-dap/SHA256.json`; all files are in the tree.

- **Source map.** `cm-rcs-dap-source-map.json` lists all 611 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-rcs-dap-differential` runs 256 states per entry routine (4,352 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC with hardware DV and without the IMU/optics device block (tools/prepare_flight_device_engine.py), its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state, the I/O channels 12, 13, 14, 31 and the 31A/31B/32 trap flags. The cases and the stops reached per routine are in `cm-rcs-dap-proof.json` (`cm-rcs-dap-differential.log` is the run). One case in 32 is frozen in `tests/cm_rcs_dap_vectors.h`.
- **Host and sanitizer results.** `make test-cm-rcs-dap sanitize-cm-rcs-dap` runs the 136 frozen cases (`cm-rcs-dap-host.log`).
- **C64 execution results.** `make vice-cm-rcs-dap` runs 2 diagnostic programs with 45 cases (`cm-rcs-dap-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 4,352 differential cases take: 1222 of 1222 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 535 of 535 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 742 of the 742 branch outcomes all the cases take (`c64_gcov_branches` in the proof).

## Flight build

The spec sets `trusted_image`, `image_w` and `index_carry16`. In flight builds (`RK_FIXED_IMAGE`), the entry checks skip their scan of the whole erasable image, which costs about 121,000 cycles a routine on the C64. That scan was the top cost of the CM entry window in VICE (lane-g/cm-entry-profile-80342904.md in the project files). Host tests and C64 diagnostics keep the scan.

The spec also sets `fast_window`: every image access goes through `include/agc_fast_redirect.h`. A cell in the EB 6 window goes to a one-byte `afw_*` helper, any other plain cell to `afo_*` (`src/agc_fast_window.c`, `src/agc_fast_ops.c`), and the rest to the `ai_*` wrappers. Each routine's entry binds the image before it calls its part function. `-DAFR_GENERIC` builds the same file on `agc_image_ops` alone. On the C64 cases (mos-sim, flight image, -Oz) the routines take 39% fewer cycles: 2.263M down to 1.390M, with the same digests.
