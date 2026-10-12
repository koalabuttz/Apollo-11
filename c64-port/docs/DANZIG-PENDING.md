# Resolve a pending right-hand order at DANZIG

Lane A. Milestone `danzig-pending`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

An interpretive order pair whose left order is RTB, such as Luminary R63's `RTB DAD` with operands LOADTIME and DP1MIN, runs the RTB routine first. That routine returns with `TC Q` through SWCALL to DANZIG, and the right order's code is still in EDOP. DANZIG then loads BBANK from BANKSET, and `CCS EDOP` branches to OPJUMP for the right order. It does not go back to NEWOPS. The point is neither a NEWOPS key nor a basic routine anyone can write once for every pair: what runs next depends on the pair. Until now the trampoline had no key for it, so lanes stopped before such pairs. Lane F counted them in R63, P12, P70-P71 and the landing guidance.

## The rule

When Z is DANZIG (LM 06061, CM 06030), EDOP is positive (1-037777, the values for which the original's `CCS EDOP` takes OPJUMP), and the program's interpretive tables include the mission's marker table (`agc_lm_danzig_marker` or `agc_cm_danzig_marker`), the point is interpretive. Its key is LOC under BANKSET, like NEWOPS. At that moment LOC holds the RTB's operand word, the pair word +1. The file that owns the pair registers a routine at that key in its interpretive table. The routine:

- accepts exactly the EDOP code its pair leaves;
- performs DANZIG's `CA BANKSET; TS BBANK` and the right order, including its operand fetch at LOC +1;
- refuses any other state with `EXEC_OUT_OF_SCOPE`.

An RTB operand word is never an order-pair word, so these keys never meet NEWOPS keys. A program without the marker resolves DANZIG by Z, as before, so no existing program changes. EDOP +0, negative or -0 also resolve by Z, because the original then goes on to the NEWJOB check and NEWOPS.

The marker is an entry at DANZIG's own address in the interpretive tables. That address is never a LOC, so the entry is never called. A marker instead of a new `agc_native_program` field keeps the 91 positional initializers in the tree, and those in open PRs, compiling unchanged.

## Evidence

Hashes: `evidence/danzig-pending/SHA256.json`.

- **Independent assembly comparison.** `make danzig-pending-differential` (`tools/danzig_pending_freeze.py`, host-only `tools/danzig_pending_reference.c`) finds every pair in both listings whose opcode field starts with RTB and has a right order: 42 in Luminary099 and 47 in Comanche055. For each one it runs the unmodified rope in yaAGC from NEWOPS, with LOC at the pair word, BANKSET, BBANK and FB on its bank, and an otherwise zero image, until Z first reaches DANZIG with EDOP nonzero.
  - 88 pairs reach that point. In every one, LOC under BANKSET is the pair word +1 and EDOP is positive.
  - The one that does not is Comanche KEP (25,3313, `RTB SSP` to P66). Its RTB target does not return from a zero image within 200,000 cycles.
  - `tests/danzig_pending_vectors.h` freezes each reached state: mission, pair, LOC, BANKSET, EDOP and FB.
  - `evidence/danzig-pending/danzig-pending-proof.json` lists every site with its listing line, right order and result.
- **Host and sanitizer results.** `make test-danzig-pending sanitize-danzig-pending` (`evidence/danzig-pending/danzig-pending-host.log`) runs 9 checks on each of the 88 states, 792 in all, through `agc_transfer_point`, `agc_transfer_resolve` and `agc_transfer_run`:
  - with the mission's marker, the point is the pair word +1, it resolves to the routine registered there, and the trampoline runs it once;
  - with EDOP +0, 040000 or 077777, without the marker, and with the other mission's marker, the point is DANZIG by Z and resolves to the basic routine;
  - NEWOPS still resolves by LOC alongside the marker.
- **C64 execution.** `make vice-danzig-pending` (`evidence/danzig-pending/danzig-pending-vice.log`) runs the same 792 checks in VICE.
