# Translate Luminary DAP interface subroutines as transfer-ABI routines

Lane F. Milestone `lm-dap-interface`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

## Scope

`Luminary099/DAP_INTERFACE_SUBROUTINES.agc`, all of it: bank 20 `02123`-`02225`,
67 assembled words on 66 source lines. Nothing is left in the file.

`src/lm_dap_interface.c` registers twelve routines in
`lm_dap_interface_routines`, one at every label a caller reaches and at every
point a call inside the file returns to. Each runs from its entry to the next
transfer and leaves Z, Q, A, L, EB and BB where the original continues:

- RESTORDB `02123` and RESTORDB+1 `02124` (ALLCOAST's entry, A = DAPBOOLS):
  DBSELECT picks the 5 degree (WIDEDB) or 0.3 degree (NARROWDB) deadband.
- SETMAXDB `02127`, SETMINDB `02140`, PFLITEDB `02142` and its return from
  ZATTEROR `02145` (1 degree, POWERDB). All four store DB, save the caller's
  return in RUPTREG1 and run CALLACCS: `CAF PRIO27; TC NOVAC` with Q at the
  `2CADR 1/ACCJOB`.
- `02137`, where NOVAC returns: `TC RUPTREG1`.
- ZATTEROR `02153` (CDUXD/Y/Z = CDUX/Y/Z) and STOPRATE `02165`: save the caller's
  EB in L under EBANK6, clear the nine rate and error words, restore EB and
  `TC Q`.
- ALLCOAST `02204` (`TC STOPRATE`), its return `02207` (clear AOSQ/AOSR,
  ALPHAQ/R and AOSQTERM/AOSRTERM, set DRIFTBIT, `TC RESTORDB +1`) and its
  return `02225` (`TC RUPTREG2`).

The job admission is the native NOVAC of `lm_exec_routines` (`lm-exec-services`),
which reads the 1/ACCJOB 2CADR from the compiled site table; the trampoline
composes the two, including the BAILOUT1 1202 ending when no core set is free.
1/ACCJOB itself (AOSTASK_AND_AOSJOB) is not run here.

Inside the file, the BZF to SETMINDB and the TCFs to SETMAXDB+1, CALLACCS and
STOPRATE+3 are composed inline; the TCs (to NOVAC, ZATTEROR, STOPRATE and
RESTORDB+1) are real transfers through the trampoline.

The returns `TC Q`, `TC RUPTREG1` and `TC RUPTREG2` execute the stored word as a
TC: Z becomes the word and Q the register's address plus one, as yaAGC does.
Refused (`EXEC_OUT_OF_SCOPE`, image unchanged), with no original producer:

- a return word that is not a TC into fixed memory (`02000`-`07777`);
- any entry with FB other than bank 20;
- `02207` with A overflowed (the original reaches it with A = +0).

## Source map

Every assembled word has one native owner (`tools/lm_dap_interface_source.py`,
`evidence/lm-dap-interface/lm-dap-interface-source-map.json`). The owner, after
the C preprocessor and with the helpers it calls, holds each erasable operand
address, the rope word of each fixed operand (so DBSELECT, DRIFTBIT, PRIO27,
EBANK6, ZERO, NARROWDB, WIDEDB and POWERDB equal the original constants), each
branch target or the inline composition named above, and each constant. The
2CADR is checked against `src/lm_exec_sites.inc`.

| Address | Source | Native |
|---|---|---|
| 20,2123 | `RESTORDB CAE DAPBOOLS` | `restordb` |
| 20,2124-2126 | `MASK DBSELECT; EXTEND; BZF SETMINDB` | `restordb_a` |
| 20,2127 | `SETMAXDB CAF WIDEDB` | `setmaxdb` |
| 20,2130-2132 | `+1 TS DB; EXTEND; QXCH RUPTREG1` | `set_db` |
| 20,2133-2134 | `CALLACCS CAF PRIO27; TC NOVAC` | `callaccs` |
| 20,2135-2136 | `2CADR 1/ACCJOB` | `lm_exec_sites` (NOVAC operand) |
| 20,2137 | `TC RUPTREG1` | `accreturn` |
| 20,2140-2141 | `SETMINDB CAF NARROWDB; TCF SETMAXDB +1` | `setmindb` |
| 20,2142-2144 | `PFLITEDB EXTEND; QXCH RUPTREG1; TC ZATTEROR` | `pflitedb` |
| 20,2145-2147 | `CAF POWERDB; TS DB; TCF CALLACCS` | `pflite_return` |
| 20,2150-2152 | `NARROWDB`, `WIDEDB`, `POWERDB` | `setmindb`, `setmaxdb`, `pflite_return` |
| 20,2153-2164 | `ZATTEROR CAF EBANK6` .. `TCF STOPRATE +3` | `zatteror` |
| 20,2165-2167 | `STOPRATE CAF EBANK6; XCH EBANK; TS L` | `save_eb` |
| 20,2170-2203 | `+3 CAF ZERO`, nine `TS`, `LXCH EBANK; TC Q` | `clear_rates` |
| 20,2204-2206 | `ALLCOAST EXTEND; QXCH RUPTREG2; TC STOPRATE` | `allcoast` |
| 20,2207-2224 | `LXCH EBANK` .. `TC RESTORDB +1` | `allcoast_clear` |
| 20,2225 | `TC RUPTREG2` | `allcoast_return` |

The older `p66_stoprate` (`docs/P66_INITIALIZATION.md`) stays as it was; it is
a specialized P66 form, not this routine.

## Evidence

Hashes: `evidence/lm-dap-interface/SHA256.json`; all four files are kept in the
tree.

- **Independent assembly comparison.** `make lm-dap-interface-differential` runs
  the unmodified Luminary099 rope in yaAGC (host-only,
  `tools/lm_dap_interface_reference.c`). Case n enters label n mod 7 (RESTORDB,
  SETMAXDB, SETMINDB, PFLITEDB, ZATTEROR, STOPRATE, ALLCOAST) from a random
  image with FB bank 20, random EB, superbank, DAPBOOLS, CDUs, A and L, and a
  caller return in Q: SWRETURN (the BANKCALL/IBNKCALL callers), a fixed-fixed
  address in `06000`-`07777` other than Q+1/Q+2 (which NOVAC's exit passes
  through), or a bank-20 address outside the file. Executive occupancy varies,
  so one state in eight has no free core set. Each case runs to the return or
  to BAILOUT1. All 7,168 equal the native run in outcome and all 2,048 erasable
  words (896 returned and 128 alarms for each NOVAC entry, 1,024 returned for
  ZATTEROR and STOPRATE); per-case digests are frozen in
  `tests/lm_dap_interface_vectors.h`. Results:
  `evidence/lm-dap-interface/lm-dap-interface-proof.json`.
- **Host and sanitizer results.** `make test-lm-dap-interface
  sanitize-lm-dap-interface` (`evidence/lm-dap-interface/lm-dap-interface-host.log`)
  checks the digests, that all twelve routines and NOVAC are reached, the
  refusals above, that ALLCOAST reaches its caller's return through STOPRATE,
  RESTORDB+1 and NOVAC, and runs the source map.
- **C64 execution results.** `make vice-lm-dap-interface`
  (`evidence/lm-dap-interface/lm-dap-interface-vice.log`) runs every sixteenth
  case (448) through the trampoline in VICE against the frozen digests.
