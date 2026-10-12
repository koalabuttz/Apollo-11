# Translate Comanche RESTARTS, RESTART_TABLES and NEWPHASE as transfer-ABI routines

Lane A2 (restart, telemetry and support), split from Lane A. Milestone `cm-restart-routines`. Interface: `docs/NATIVE_TRANSFER_ABI.md` (owned by Lane A; this milestone adds routines and operands only, no ABI change).

Comanche055 RESTARTS, GETPART2 and PHSPART2 select variable, table and X.1 display restarts natively from RESTART_TABLES words compiled from the source. They run as routines of the control-transfer ABI from RESTARTS to SWRETURN, handing each selected service its 2CADR. NEWPHASE, the one executable routine in RESTART_TABLES, is a routine of the same ABI whose group operand comes from a site table compiled from its nine callers.

## Scope

- `Comanche055/RESTARTS_ROUTINE.agc`: every instruction, 01,3520-3767 and the FFTAG2 parts (ITSINDIR, ITSLGCL1). The instruction sequence is identical to Luminary099 RESTARTS_ROUTINE (the files differ only in COUNT cards), so the native code is `lm_restart_select`'s at Comanche addresses. That covers variable restarts of type A (job, waitlist, longcall) and type B (job plus table part), the X.1 display restart, odd and even table phases, and PHSPART2 second entries. It also covers direct, indirect and immediate waitlist times, longcall times including the source constant 600SECS (13,2654), and FINDVAC/NOVAC by priority sign.
- `Comanche055/RESTART_TABLES.agc`: the 249 data words at 01,2000-2370, consumed through the selection above (all 79 table entries, counting each even phase's second entry, are selected and delivered to their service), and NEWPHASE/NUFAZ+10 at 4114-4137.
- External continuations: the bodies of FINDVAC, NOVAC, WAITLIST, LONGCALL (Executive and Waitlist) and SWRETURN, the jobs and tasks named by the 2CADRs, and NEWPHASE's callers at their L+2. Those files resolve to `EXEC_UNMAPPED` until they register routines.

The legacy `cm_restart_select`/`cm_restart_timing` modules (docs/COMANCHE_RESTART_SELECT.md) stay as they were; this milestone does not use or change them.

## Source map

| AGC | Native |
|---|---|
| RESTARTS 01,3520, ITSAVAR, ITSLIKEB, ITSAJOB, CHKNOVAC, ITSNOVAC | `cm_restart_routines_select(CM_RR_GROUP)`, `job` |
| GETPART2 01,3533 (X.1 display, ITSATBL, ITSEVEN) | `cm_restart_routines_select(CM_RR_TYPEB_TABLE)` |
| PHSPART2 01,3755, CONTBL2, ITSAJOB2, ITSWTLST | `cm_restart_routines_select(CM_RR_PART2)` and the shared table tail |
| ITSAWAIT, TIMETEST, ITSINDIR, FINDTIME, IMEDIATE | `timed`, `wait_delay` |
| ITSLNGCL, ITSLGCL1, ITSLGCL2, LONGCLCL | `timed` (longcall), `long_delay` |
| SIZETAB, PRDTTAB, CADRTAB, all n.mSPOT words | `cm_restart_table_words` (`src/cm_restart_table_data.inc`, `tools/cm_restart_tables_generate.py`) |
| TC GOLOC-1, TC at GOLOC+2, 01,3542 TC RTRNCADR / 01,3557 TC SWRETURN | `cm_restart_routines` (`src/cm_restart_routines.c`), erasable keys 0704 and 0707 |
| 2CADR INITDSP after TC FINDVAC (01,3540) | `cm_restart_operands` |
| NEWPHASE 4114, NUFAZ+10 4126 | `newphase` in `cm_restart_routines` |
| `OCT n` after each `TC NEWPHASE` (P40-P47 x5, SERVICER207 x3, IMU_CALIBRATION_AND_ALIGNMENT x1) | `cm_restart_operands` (from `agc_sites_generate.sites`, unchanged) |

NEWPHASE refuses an A with overflow (TS L would skip; no caller loads one) and a Q that is no `TC NEWPHASE` site. It ends at the caller's L+2 with Q = 3, as TC Q does in the original. Its INHINT ... RELINT leaves interrupts enabled; that is `cm_native_io.interrupt_enabled`, compared with yaAGC's AllowInterrupt, so NEWPHASE needs the mission's `cm_native_io` as io.

## Evidence

Hashes: `evidence/cm-restart-routines/SHA256.json`. Bulk files are assets of the `evidence-cm-restart-routines` release (`python3 tools/evidence.py fetch cm-restart-routines`).

- Independent assembly comparison: `evidence/cm-restart-routines/cm-restart-routines-proof.json`. `make cm-restart-routines-differential` runs the unmodified Comanche055 rope (assembled from this repository by `tools/setup_cloud.sh`) in yaAGC (host-only). It compares 7,168 selections, 5,120 RESTARTS-to-SWRETURN compositions and 1,024 NEWPHASE calls in all 2,048 erasable words, A and L, the status, and each service's entry registers and 2CADR, then freezes per-case digests into `tests/cm_restart_routines_vectors.h`. The last 1,024 selections (and compositions 4,096-5,119) target the signed-zero and overflow branches as in Luminary issue #29: CHKNOVAC priorities 0 and 077777; FINDTIME with TBASE equal to the complement of TIME1, both +0, both -0, sums that overflow positive and negative, and direct and indirect deltas; and LONGCALL times left whose high and low words take every combination of +, +0, - and -0. NEWPHASE cases enter from each of the nine sites with phases positive, +0, negative and -0 (and the edges 1, 037777, -1, -037777), interrupts enabled or inhibited on entry, and also compare the interrupt-enable state. `evidence/cm-restart-routines/cm-restart-routines-table.json` lists each table word and site checked against the source and rope.
- Host and sanitizer results: `make test-cm-restart-routines sanitize-cm-restart-routines` (`evidence/cm-restart-routines/cm-restart-routines-host.log`).
- C64 execution results: `make vice-cm-restart-routines` (`evidence/cm-restart-routines/cm-restart-routines-vice.log`), every sixteenth case of each kind (832 checks).
