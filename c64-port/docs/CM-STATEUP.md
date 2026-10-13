# Translate Comanche FRESH_START_AND_RESTART (bank 13: STATEUP and THISVINT; R00's CALL/EXIT) as interpretive transfer-ABI routines

Lane A2 (restart, telemetry and support). Milestone `cm-stateup`. Interface: `docs/NATIVE_TRANSFER_ABI.md` (owned by Lane A; this milestone adds an interpretive routine table only, no ABI change). It is the Comanche055 counterpart of `lm-stateup` (`docs/LM-STATEUP.md`) and the first module built on the Comanche055 interpreter bodies (`include/cm_interp.h`, milestone `cm-interp-bodies`).

STATEUP is the interpretive code INTEGRATION_INITIALIZATION calls for P00 integration (R00): it sets VINTFLAG and, when ORBWFLAG says W is valid for orbital navigation, DIM0FLAG, and extrapolates the CSM state vector (INTEGRV); then, unless the LM is on the lunar surface, it stores TETCSM in TDEC1, calls INTSTALL, clears VINTFLAG, calls SETIFLGS and extrapolates the LM state vector (with the W matrix when RENDWFLG says W is valid for rendezvous navigation). STATEND ends through ENDINT. THISVINT sets VINTFLAG and returns. R00 in bank 04 is two interpretive pairs after ROC's TC INTPRET (CALL INTSTALL, then DUMMYAD EXIT back to basic code), which also belong here. With bank 05 and fixed-fixed (`cm-fresh-start`) and banks 04 and 20 (`cm-v37`) this covers every instruction of the file but the patch-only STARTSIM.

## Scope

- `Comanche055/FRESH_START_AND_RESTART.agc`, every order pair of bank 13 (2000-2035): STATEUP, STATEND and THISVINT; and R00's two pairs in bank 04 (2131-2133: CALL INTSTALL; DUMMYAD EXIT).
- Routines (`cm_stateup_interpretive`, 12): one per order-pair word, keyed by LOC under BANKSET at the interpreter fetch point NEWOPS (06037).
- Each pair is the compiled Comanche055 interpreter path (`cm_interp_steps` / `cm_interp_orders`) for its words in source order: NEWOPS, the left order, DANZIG's right-hand dispatch, the right order and DANZIG. The pair's words are literal constants; nothing is read from code memory.
  - Switch orders (SET, CLEAR, BOF, BON, CLRGO) go through SWITCHES (01,2543) under INHINT/RELINT and branch through SWBRANCH.
  - CALL and STCALL go through CALL/ITA (01,2534), store QPRET and GOTO the callee; STCALL first stores MPAC in TDEC1. DLOAD TETCSM reads the erasable pair (DLOAD 06021).
  - RVQ returns to the CADR in QPRET. EXIT leaves the interpreter for the basic word after it (R00's CS OCT1400 at 04,2134, a `cm-v37` entry).
- A run goes from pair to pair through the trampoline until LOC leaves the file (INTEGRV, INTSTALL, SETIFLGS, ENDINT, THISVINT's caller), R00's EXIT reaches basic code, or DANZIG finds NEWJOB positive and goes to CHANG2 (05170).
- Refused: a state the interpreter does not run in at NEWOPS (EDOP nonzero, BBANK not under BANKSET or not the pair's bank (13, or 04 for R00), a work area outside the five VAC areas or PUSHLOC outside it), NEWJOB negative, a CLRGO that does not branch, and an RVQ with an erasable QPRET (CALL always leaves a fixed one).
- Devices: io is a `p66_bias_io`; the switch orders' INHINT/RELINT leave interrupts enabled.

## Source map

| AGC | Native (`src/cm_stateup.c`) |
|---|---|
| STATEUP ... THISVINT (13,2000-2035), R00 CALL/EXIT (04,2131-2133) | `pair`, one `case` per order-pair word |
| NEWOPS, DANZIG, SWITCHES, CALL, STCALL, DLOAD, RVQ, EXIT | `cm_interp_steps` / `cm_interp_orders` (the Comanche055 builds of the compiled interpreter paths) |

## Evidence

Hashes: `evidence/cm-stateup/SHA256.json`. Bulk files are assets of the `evidence-cm-stateup` release (`python3 tools/evidence.py fetch cm-stateup`).

- Independent assembly comparison: `evidence/cm-stateup/cm-stateup-proof.json`. `make cm-stateup-differential` runs the unmodified Comanche055 rope in yaAGC (host-only), 4,096 cases.
  - Case n enters pair n % 12 at NEWOPS with LOC there and BANKSET = BBANK (FB 13, or 04 for R00).
  - The image is pseudo-random within the program's domain: the flag words, so each switch order goes both ways; EB, SUPERBNK, the interrupt-enable state and MODE; EDOP +0; a work area of one of the five VAC areas with PUSHLOC inside it; NEWJOB +0, or positive in a fifth of the cases; at THISVINT, QPRET as CALL leaves it (its caller's return at 13,3576, or another bank 13 word outside the file).
  - Each case runs until the original leaves the interpreter's code (fixed-fixed 06006-07656, 00,2000-3731, 01,2371-2625) with Z at NEWOPS and LOC outside the file, at R00's CS OCT1400 after EXIT, or at CHANG2.
  - Compared: the interrupt-enable state, SUPERBNK and all 2,048 erasable words. The digests are frozen into `tests/cm_stateup_vectors.h`.
- Host and sanitizer results: `make test-cm-stateup sanitize-cm-stateup` (`evidence/cm-stateup/cm-stateup-host.log`). Beyond the oracle digests, the test checks that:
  - all 12 pairs are reached;
  - each case ends at NEWOPS of INTEGRV, INTSTALL, SETIFLGS, ENDINT, THISVINT's caller or another bank 13 return, at R00's CS OCT1400, or at CHANG2, and every one is reached; CALL, STCALL and CLRGO targets are in bank 13;
  - LOC on an operand word has no routine; BBANK not under BANKSET, an R00 pair outside bank 04, a work area outside the VAC areas, an erasable QPRET at RVQ and a missing io are refused.
- C64 execution results: `make vice-cm-stateup` (`evidence/cm-stateup/cm-stateup-vice.log`): 242 C64 rows chosen by native branch outcome (`tools/a2_c64_rows.py`: every 17th case plus a greedy cover of the gcov branch arcs of every linked source; the freeze fails unless the rows take all 211 arcs the host cases take). The host test checks every C64 row is a host case with its host digest.
