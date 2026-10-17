# Translate Comanche CM_ENTRY_DIGITAL_AUTOPILOT

Lane G (CM flight). Milestone `cm-entry-dap`. How the routines are made and checked: `docs/INTERP-COMPILE.md` (whole-file basic code: `tools/basic_compile.py`).

`Comanche055/CM_ENTRY_DIGITAL_AUTOPILOT.agc` (`src/cm_entry_dap.c`) is native C in full: The entry autopilot: READGYMB and the body rates, the atmospheric and exo-atmospheric DAP modes, CM/RCS roll control, the FDAI needles and the CM/DUMPR telemetry file. Each routine runs the file from its entry, or from where it resumes, to the point where control leaves the file. Every original word is one call of the basic-instruction primitives (`src/agc_image_ops.c`) under a label named for its address, with TS overflow skips, CCS four-way skips and INDEX on the whole word kept; computed transfers (TC Q, indexed TCF) go through one dispatch on the target address.

This replaces the earlier hand translation, which the certificate reviewer rejected in #84: its comparison was a listing check rather than a yaAGC run, and none of its entries was a routine of the control-transfer ABI. Now every entry another file names is a registered routine, and the comparison runs the original rope in yaAGC.

## Scope

138 routines, keyed by physical address in `cm_entry_dap_basic`:

| Entry | Kind | Why it is a routine |
|---|---|---|
| 03,2000 FLUSHJET | basic | part key |
| 15,2434 READGYMB | basic | entry READGYMB (RESTART_TABLES.agc -2CADR READGYMB) |
| 15,2446  | basic | part key |
| 15,2447 READGYM1 | basic | part key |
| 15,2472  | basic | part key |
| 15,2473  | basic | part key |
| 15,2474  | basic | part key |
| 15,2475  | basic | part key |
| 15,2476  | basic | part key |
| 15,2500  | basic | return from PHASCHNG |
| 15,2507  | basic | return from WAITLIST |
| 15,2511  | basic | part key |
| 15,2522 ANGOVCOR | basic | part key |
| 15,2527  | basic | part key |
| 15,2536 BODYRATE | basic | part key |
| 15,2540  | basic | return from SPCOS |
| 15,2543  | basic | return from SPSIN |
| 15,2551  | basic | return from SPCOS |
| 15,2570  | basic | part key |
| 15,2604  | basic | part key |
| 15,2607  | basic | return from SPSIN |
| 15,2623  | basic | part key |
| 15,2626  | basic | part key |
| 15,2627  | basic | part key |
| 15,2631  | basic | return from SPSIN |
| 15,2636  | basic | part key |
| 15,2641  | basic | return from SPCOS |
| 15,2651 NOGAMDUT | basic | part key |
| 15,2665 ATTRATES | basic | 2CADR ATTRATES |
| 15,2676  | basic | part key |
| 15,2700  | basic | return from SPCOS |
| 15,2712  | basic | return from SPSIN |
| 15,2724  | basic | return from SPSIN |
| 15,2730  | basic | part key |
| 15,2734  | basic | part key |
| 15,2741  | basic | part key |
| 15,2753  | basic | part key |
| 15,2754  | basic | part key |
| 15,2763  | basic | part key |
| 15,2770 EXDAPIN | basic | part key |
| 15,2777  | basic | part key |
| 15,3000  | basic | part key |
| 15,3001 3DDZ | basic | part key |
| 15,3006 BIASEDZ | basic | part key |
| 15,3016  | basic | part key |
| 15,3037 EXDAP | basic | part key |
| 15,3042  | basic | part key |
| 15,3043  | basic | part key |
| 15,3044  | basic | part key |
| 15,3052  | basic | part key |
| 15,3053  | basic | part key |
| 15,3054  | basic | part key |
| 15,3056  | basic | return from PHASCHNG |
| 15,3064 EXDAP4 | basic | return from WAITLIST |
| 15,3101 EXDAP1 | basic | part key |
| 15,3106  | basic | part key |
| 15,3112  | basic | part key |
| 15,3123  | basic | part key |
| 15,3131  | basic | part key |
| 15,3141  | basic | part key |
| 15,3150  | basic | part key |
| 15,3152  | basic | part key |
| 15,3167  | basic | part key |
| 15,3171 NSEC | basic | part key |
| 15,3207 SETJTAG | basic | entry SETJTAG (RESTART_TABLES.agc -2CADR SETJTAG) |
| 15,3215  | basic | return from PHASCHNG |
| 15,3216 CM/RCS | basic | part key |
| 15,3252  | basic | part key |
| 15,3253  | basic | part key |
| 15,3254  | basic | part key |
| 15,3255  | basic | part key |
| 15,3267  | basic | part key |
| 15,3270  | basic | part key |
| 15,3300 COMPAT | basic | part key |
| 15,3306  | basic | part key |
| 15,3307  | basic | part key |
| 15,3355 REFLECT | basic | part key |
| 15,3356  | basic | part key |
| 15,3357  | basic | part key |
| 15,3362  | basic | part key |
| 15,3364 DZ1 | basic | part key |
| 15,3401  | basic | part key |
| 15,3415 MAXVTIM1 | basic | part key |
| 15,3431 OVRLINE | basic | part key |
| 15,3437 OVRLINE1 | basic | part key |
| 15,3457 GETOFF | basic | part key |
| 15,3460  | basic | part key |
| 15,3462  | basic | part key |
| 15,3467  | basic | part key |
| 15,3501 TOFFOVFL | basic | part key |
| 15,3512  | basic | part key |
| 15,3566 TON2TEST | basic | part key |
| 15,3572  | basic | part key |
| 15,3601  | basic | part key |
| 15,3603 TIMETST3 | basic | part key |
| 15,3605 TIMETST4 | basic | part key |
| 15,3606 TIMETST5 | basic | part key |
| 15,3610 JETCALL1 | basic | part key |
| 15,3617 JETCALL3 | basic | part key |
| 15,3620  | basic | part key |
| 15,3621  | basic | part key |
| 15,3633 JETCALL2 | basic | part key |
| 15,3642 JETACTN | basic | return from WAITLIST |
| 15,3647 ROLLDUMP | basic | part key |
| 15,3650 JETCALL | basic | 2CADR JETCALL |
| 15,3657 DZCALL | basic | part key |
| 15,3662  | basic | part key |
| 15,3664  | basic | part key |
| 15,3666  | basic | part key |
| 15,3670  | basic | part key |
| 15,3675 DZCALL1 | basic | part key |
| 15,3677 CM/FDAI | basic | part key |
| 15,3700  | basic | part key |
| 15,3704  | basic | part key |
| 15,3705 CM/FDAIR | basic | part key |
| 15,3717  | basic | return from IBNKCALL |
| 15,3720 CMTMFILE | basic | part key |
| 15,3724  | basic | part key |
| 15,3726 SAVENDX | basic | part key |
| 15,3736 CM/END | basic | part key |
| 20,3612 CM/DAPON | basic | entry CM/DAPON (P61-P67.agc CADR CM/DAPON) |
| 20,3616  | basic | return from DOWNFLAG |
| 20,3620  | basic | return from DOWNFLAG |
| 20,3622  | basic | part key |
| 20,3627  | basic | part key |
| 20,3632  | basic | part key |
| 20,3633 NOTYET | basic | part key |
| 20,3634  | basic | part key |
| 20,3636  | basic | return from BANKCALL |
| 20,3653  | basic | part key |
| 20,3672 CM/DAPIC | basic | entry CM/DAPIC (P61-P67.agc  CM/DAPIC) |
| 20,3702  | basic | part key |
| 20,3715  | basic | return from 2434 |
| 20,3716  | basic | return from WAITLIST |
| 20,3722  | basic | part key |
| 20,3723  | basic | part key |
| 20,3726  | basic | part key |
| 20,3732  | basic | return from 2PHSCHNG |

The step machine is split into part functions (spec `basic_parts`) so each routine's link closure fits its flight-store slot. A transfer across a part hands Z to the target word, whose routine is a part key, and the transfer runner runs it as the next step. Part keys are not entries: their runs come from the 28 entry routines, which draw all the cases, so the case set and the host vectors are the same as before the split.

Destinations outside the file: 2434, 2PHSCHNG, BANKCALL, DOWNFLAG, IBNKCALL, PHASCHNG, POSTJUMP, RESUME, SPCOS, SPSIN, TASKOVER, WAITLIST. Lane A has not yet registered the Comanche Executive and Waitlist routines (WAITLIST, NOVAC, FINDVAC, TASKOVER, ENDOFJOB, IBNKCALL with ISWRETRN, POSTJUMP, BANKCALL, RESUME) or the T5RUPT/T6RUPT vectors that dispatch T5LOC/T6LOC; until it does, a transfer to them stops with EXEC_UNMAPPED at the frontier.

## Domain

Canonical 2048-word image (every word random, the counters, `TIME1`-`TIME6`, the CDUs and the save registers at 010-061 included) under EB 6 (the file's `EBANK=`), FB of the routine's bank, any A, L and Q unless stated, and any values of the other erasables. CMDAPMOD +1, +0, -0 or -1 and SW/NDX alternating the needles (+1 to +13) with the telemetry index (-13 to -1) in steps of three. YJETCODE, P/RJCODE, LIMITS and HALFPR are read at -1 to +1. 2D/SDZ's TC Q with Q = 3 (after CCS JETAG -0) runs RELINT and then the word in EB as TC 3000, as the AGC does; this needs EB = E6.

## Evidence

Hashes: `evidence/cm-entry-dap/SHA256.json`; all files are in the tree.

- **Source map.** `cm-entry-dap-source-map.json` lists all 813 words of the file with the routine that compiles each (or why a constant is never run as code); 0 are untranslated.
- **Independent assembly comparison.** `make cm-entry-dap-differential` runs 256 states per entry routine (7,168 cases) through the original Comanche055 rope in yaAGC (pinned yaAGC with hardware DV and without the IMU/optics device block (tools/prepare_flight_device_engine.py), its ScalerCounter held at 0 (TIME1-TIME6 and the timer interrupts do not advance during a case)) and the native routines; all are equal in Z, 2048 erasable words (A, L 16-bit) and the interrupt-enable state, the I/O channels 05, 06 and the 31A/31B/32 trap flags. The cases and the stops reached per routine are in `cm-entry-dap-proof.json` (`cm-entry-dap-differential.log` is the run). One case in 32 is frozen in `tests/cm_entry_dap_vectors.h`.
- **Host and sanitizer results.** `make test-cm-entry-dap sanitize-cm-entry-dap` runs the 224 frozen cases (`cm-entry-dap-host.log`).
- **C64 execution results.** `make vice-cm-entry-dap` runs 3 diagnostic programs with 82 cases (`cm-entry-dap-vice.log`, which starts with each PRG's SHA-256). The cases are chosen so the C64 runs take every control-flow edge (both outcomes of each branch) of every linked source that the 7,168 differential cases take: 1320 of 1320 edges and comparison outcomes (clang -O0 trace-pc-guard,trace-cmp) and 815 of 815 compiled labels. Checked again with gcov -b over gcc -O0 `--coverage` builds of the same sources: the C64 cases take 910 of the 910 branch outcomes all the cases take (`c64_gcov_branches` in the proof).

## Flight build

The spec sets `trusted_image`, `image_w` and `index_carry16`. In flight builds (`RK_FIXED_IMAGE`), the entry checks skip their scan of the whole erasable image, which costs about 121,000 cycles a routine on the C64. That scan was the top cost of the CM entry window in VICE (lane-g/cm-entry-profile-80342904.md in the project files). Host tests and C64 diagnostics keep the scan.

The spec's `hot_points` name the 26 basic routines one entry DAP pass (T5RUPT) reaches in that window. `cm_entry_dap_hot_routines` holds them, with the same wrappers as `cm_entry_dap_basic`, for the flight store's entry pack: a pass then loads one module instead of several chunks of the whole table. `make test-cm-entry-dap-hot` checks that each hot key calls the same function through either table.

The spec also sets `fast_window`: every image access goes through `include/agc_fast_redirect.h`. A cell in the EB 6 window goes to a one-byte `afw_*` helper, any other plain cell to `afo_*` (`src/agc_fast_window.c`, `src/agc_fast_ops.c`), and the rest to the `ai_*` wrappers. Each routine's entry binds the image before it calls its part function. `-DAFR_GENERIC` builds the same file on `agc_image_ops` alone. On the C64 cases (mos-sim, flight image, -Oz) the routines take 39% fewer cycles: 4.996M down to 3.057M, with the same digests.
