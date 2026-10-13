# INTERP-ENTRY: INTPRET, INTPRET +2 and INTRSM for any caller

Lane A, milestone `interp-entry`.

Basic code enters the interpreter in three ways. Each runs a few words of INTERPRETER.agc and ends at NEWOPS, the interpreter's fetch point:

| Entry | LM | CM | Words |
|---|---|---|---|
| INTPRET | 6037 | 6006 | `RELINT; EXTEND; QXCH LOC`. LOC becomes the word after the caller's `TC INTPRET`. |
| INTPRET +2 | 6041 | 6010 | `LXCH LOC`. Interpretive branches finish here. |
| INTRSM | 6050 | 6017 | `LXCH BBANK; TCF INTPRET +3`. Resumes a suspended interpretive job. |

All three continue at INTPRET +3: `CA BBANK; TS BANKSET; MASK BIT15; TS INTBIT15; TS EDOP; TCF NEWOPS`. NEWOPS is LM 6070 and CM 6037.

`src/interp_entry.c` runs them for any caller, and `src/standard_lead_routines.c` registers them in `lm_interpreter_routines` and `cm_interpreter_routines`. Before this milestone those tables held only the E/CALL standard lead-in's composite paths. INTPRET accepted only E/CALL's return address. INTPRET +2 accepted only bank 04 (bank 13 for CM). INTRSM was not registered.

At NEWOPS the trampoline continues with the interpretive routine keyed by LOC under BANKSET. That routine is whatever a lane registers for the interpretive code that follows the caller's `TC INTPRET`.

## Domain

- **State.**
  - Any A, L and Q as raw 16-bit registers, and any erasable contents.
  - Coherent EB, FB and BB, with ZERO = 0.
  - SUPERBNK and the interrupt-enable latch in the mission's io record. INTPRET's RELINT sets the latch.
- **INTPRET from E/CALL.** Q at E/CALL's CADR word, with FB 04 (CM: 13), keeps the standard lead-in's composite path. That path runs on through E/CALL's CALL to the callee, or refuses. NEWOPS at E/CALL's CALL word has no interpretive routine.
- **INTPRET +2.** The new LOC, taken from L, must be at most 3777. Interpretive branches stay in their bank, and the E/CALL finish has the same domain. Other values are refused with nothing changed.
- **Register widths.** These follow the original as yaAGC runs it:
  - QXCH and LXCH store the overflow-corrected 15-bit value and return the sign-extended old one.
  - INTPRET also normalizes L to its corrected, sign-extended value.
  - `LXCH BBANK` writes BB from the corrected L.
  - `TS EDOP` keeps bits 14-8 shifted down, so EDOP is 0.

## Verification

- **Oracle comparison.** `make interp-entry-differential`:
  - runs 512 states at each entry of each mission (3,072 cases) on the unmodified ropes in yaAGC, from the entry until Z first reaches NEWOPS;
  - requires every erasable word, the 16-bit A, L and Q, SUPERBNK and the interrupt latch to equal the native routine's;
  - freezes digests into `tests/interp_entry_vectors.h`.
- **Inputs.**
  - Callers' L and Q: a fixed return address, an in-bank LOC, or a sign-extended BBCON.
  - In one case in four, any 16-bit value, overflow included.
- **Host and sanitizer results.** `make test-interp-entry sanitize-interp-entry` checks the digests and each entry's LOC, BANKSET, BBANK and latch results. It also checks:
  - that an INTPRET +2 LOC outside the bank, and incoherent banks, are refused with nothing changed;
  - that E/CALL's INTPRET still takes the CALL.

  `lm-standard-lead-transfer` and `cm-standard-lead-transfer` pass unchanged.
- **C64 execution results.** `make vice-interp-entry` runs every sixteenth case in VICE.

Evidence is in `evidence/interp-entry/`.
