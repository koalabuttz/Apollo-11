# Translate Luminary FRESH_START_AND_RESTART (bank 13: STATEUP and THISVINT) as interpretive transfer-ABI routines

Lane A2 (restart, telemetry and support), split from Lane A. Milestone `lm-stateup`. Interface: `docs/NATIVE_TRANSFER_ABI.md` (owned by Lane A; this milestone adds an interpretive routine table only, no ABI change).

STATEUP is the interpretive code INTEGRATION_INITIALIZATION calls for P00 integration (R00): it extrapolates the CSM state vector (with the 6x6 W matrix when the LM is on the lunar surface and W is valid for rendezvous navigation), then the LM state vector (with the 9x9 W matrix when W is valid), unless the LM is on the surface, and ends through ENDINT. THISVINT clears VINTFLAG and returns. R00 in bank 04 is two interpretive pairs after its TC INTPRET (CALL INTSTALL, then EXIT back to basic code), which also belong here. With banks 04 and 05 (`lm-v37`, `lm-fresh-start`) this covers every instruction of the file but the patch-only STARTSIM.

## Scope

- `Luminary099/FRESH_START_AND_RESTART.agc`, every order pair of bank 13 (2026-2067): STATEUP, DOINT, NO-INT, DOINT2 and THISVINT; and R00's two pairs in bank 04 (2144-2146: CALL INTSTALL; DUMMYAD EXIT), added with the file's certificate.
- Routines (`lm_stateup_interpretive`, 13): one per order-pair word, keyed by LOC under BANKSET at the interpreter fetch point NEWOPS (06070).
- Each pair is the compiled `lm_interp_steps` / `lm_interp_orders` path for its words in source order: NEWOPS, the left order, DANZIG's right-hand dispatch, the right order and DANZIG. The pair's words are literal constants; nothing is read from code memory.
  - Switch orders (SET, CLEAR, BOF, BON, CLRGO) go through SWITCHES under INHINT/RELINT and branch through SWBRANCH.
  - CALL and STCALL store QPRET and GOTO the callee; STCALL first stores MPAC in TDEC1. DLOAD TETCSM reads the erasable pair.
  - RVQ returns to the CADR in QPRET. EXIT leaves the interpreter for the basic word after it (R00's TC DOWNFLAG, an `lm-v37` entry).
- A run goes from pair to pair through the trampoline until LOC leaves the file (INTEGRV, INTSTALL, SETIFLGS, ENDINT, THISVINT's caller), R00's EXIT reaches basic code, or DANZIG finds NEWJOB positive and goes to CHANG2.
- Refused: a state the interpreter does not run in at NEWOPS (EDOP nonzero, BBANK not under BANKSET or not the pair's bank (13, or 04 for R00), a work area that is neither one of the five VAC areas nor FAKEPRET (0110, a NOVAC job's) or PUSHLOC outside it), NEWJOB negative, a CLRGO that does not branch, and an RVQ with an erasable QPRET (CALL always leaves a fixed one).
- Devices: io is a `p66_bias_io`; the switch orders' INHINT/RELINT leave interrupts enabled.

## Source map

| AGC | Native (`src/lm_stateup.c`) |
|---|---|
| STATEUP ... THISVINT (13,2026-2067), R00 CALL/EXIT (04,2144-2146) | `pair`, one `case` per order-pair word |
| NEWOPS, DANZIG, SWITCHES, CALL, STCALL, DLOAD, RVQ, EXIT | `lm_interp_steps` / `lm_interp_orders` (existing compiled interpreter paths) |

## Evidence

Hashes: `evidence/lm-stateup/SHA256.json`. Bulk files are assets of the `evidence-lm-stateup` release (`python3 tools/evidence.py fetch lm-stateup`).

- Independent assembly comparison: `evidence/lm-stateup/lm-stateup-proof.json`. `make lm-stateup-differential` runs the unmodified Luminary099 rope in yaAGC (host-only), 4,096 cases.
  - Case n enters pair n % 13 at NEWOPS with LOC there and BANKSET = BBANK (FB 13, or 04 for R00).
  - The image is pseudo-random within the program's domain: the flag words, so each switch order goes both ways; EB, SUPERBNK, the interrupt-enable state and MODE; EDOP +0; a work area of one of the five VAC areas or, in a sixth of the cases, FAKEPRET (0110: the NOVAC V37 job that runs R00 after CANV37, whose QPRET is MPAC +6) with PUSHLOC inside it; NEWJOB +0, or positive in a fifth of the cases; at THISVINT, QPRET as CALL leaves it (its caller's return at 13,3603, or another bank 13 word outside the file).
  - Each case runs until the original leaves the interpreter's code with Z at NEWOPS and LOC outside the file, at R00's TC DOWNFLAG after EXIT, or at CHANG2.
  - Compared: the interrupt-enable state, SUPERBNK and all 2,048 erasable words. The digests are frozen into `tests/lm_stateup_vectors.h`.
- Host and sanitizer results: `make test-lm-stateup sanitize-lm-stateup` (`evidence/lm-stateup/lm-stateup-host.log`). Beyond the oracle digests, the test checks that:
  - all 13 pairs are reached;
  - each case ends at NEWOPS of INTEGRV, INTSTALL, SETIFLGS, ENDINT, THISVINT's caller or another bank 13 return, at R00's TC DOWNFLAG, or at CHANG2, and every one is reached; CALL, STCALL and CLRGO targets are in bank 13;
  - LOC on an operand word has no routine; BBANK not under BANKSET, an R00 pair outside bank 04, a work area outside the VAC areas and FAKEPRET, an erasable QPRET at RVQ and a missing io are refused.
- C64 execution results: `make vice-lm-stateup` (`evidence/lm-stateup/lm-stateup-vice.log`): 256 C64 rows chosen by native branch outcome (`tools/a2_c64_rows.py`: every 16th case plus a greedy cover of the gcov branch arcs of every linked source; the freeze fails unless the rows take all 222 arcs the host cases take); every 16th case already takes them all, so the cover adds none. The host test checks every C64 row is a host case with its host digest.
