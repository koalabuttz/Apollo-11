# Translate Luminary RESTARTS and run its restart table entries through the trampoline

Lane A. Milestone `lm-restart-select`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

Luminary099 RESTARTS, GETPART2 and PHSPART2 select variable, table and X.1 display restarts natively from RESTART_TABLES words compiled from the source. They run as routines of the control-transfer ABI from RESTARTS to SWRETURN, handing each selected service its 2CADR.

## Scope

- `Luminary099/RESTARTS_ROUTINE.agc`: every instruction, 01,3526-3775 and FFTAG2 5436-5456. That covers variable restarts of type A (job, waitlist, longcall) and type B (job plus table part), the X.1 display restart, odd and even table phases, and PHSPART2 second entries. It also covers direct, indirect and immediate waitlist times, longcall times including the source constant 600SECS, and FINDVAC/NOVAC by priority sign.
- `Luminary099/RESTART_TABLES.agc`: the 132 data words, consumed through the selection above. All 40 table entries are selected and delivered to their service.
- External continuations: the bodies of FINDVAC, NOVAC, WAITLIST, LONGCALL (Executive and Waitlist) and SWRETURN, and the jobs and tasks named by the 2CADRs. Those files resolve to `EXEC_UNMAPPED` until they register routines.

The instruction sequence is identical to Comanche055 RESTARTS (`cm_restart_select`) at different addresses, and the native code follows it.

## Source map

| AGC | Native |
|---|---|
| RESTARTS 01,3526, ITSAVAR, ITSLIKEB, ITSAJOB, CHKNOVAC, ITSNOVAC | `lm_restart_select(LM_RS_SELECT_GROUP)`, `job` |
| GETPART2 01,3541 (X.1 display, ITSATBL, ITSEVEN) | `lm_restart_select(LM_RS_SELECT_TYPEB_TABLE)` |
| PHSPART2 01,3763, CONTBL2, ITSAJOB2, ITSWTLST | `lm_restart_select(LM_RS_SELECT_PART2)` and the shared table tail |
| ITSAWAIT, TIMETEST, ITSINDIR, FINDTIME, IMEDIATE | `timed`, `wait_delay` |
| ITSLNGCL, ITSLGCL1, ITSLGCL2, LONGCLCL | `timed` (longcall), `long_delay` |
| SIZETAB, PRDTTAB, CADRTAB, all n.mSPOT words | `lm_restart_table_data` (`src/lm_restart_table_data.inc`, `tools/lm_restart_tables_generate.py`) |
| TC GOLOC-1, TC at GOLOC+2, 01,3550 TC RTRNCADR / TC SWRETURN | `lm_restart_routines` (`src/lm_restart_routines.c`), erasable keys 0704 and 0707 |
| 2CADR INITDSP after TC FINDVAC | `lm_restart_operands` |

## Evidence

Hashes: `evidence/lm-restart-select/SHA256.json`. Bulk files are assets of the `evidence-lm-restart-select` release
(`python3 tools/evidence.py fetch lm-restart-select`).

- Independent assembly comparison: `evidence/lm-restart-select/lm-restart-select-proof.json`. `make lm-restart-select-differential` runs the unmodified Luminary099 rope in yaAGC (host-only). It compares 7,168 selections and 4,096 compositions in all 2,048 erasable words, the status, and each service's entry registers and 2CADR, then freezes per-case digests into `tests/lm_restart_vectors.h`. The last 1,024 selections (issue #29) target the signed-zero and overflow branches: CHKNOVAC priorities 0 and 077777; FINDTIME with TBASE equal to the complement of TIME1, both +0, both -0, sums that overflow positive and negative, and direct and indirect deltas; and LONGCALL times left whose high and low words take every combination of +, +0, - and -0 (16 pairs, each reached at least eight times), with TIME2, TIME1 and LONGBASE in their nonnegative range. `evidence/lm-restart-select/lm-restart-select-table.json` lists each table word checked against the source and rope.
- Host and sanitizer results: `make test-lm-restart-select sanitize-lm-restart-select` (`evidence/lm-restart-select/lm-restart-select-host.log`).
- C64 execution results: `make vice-lm-restart-select` (`evidence/lm-restart-select/lm-restart-select-vice.log`), every sixteenth case of each kind.
