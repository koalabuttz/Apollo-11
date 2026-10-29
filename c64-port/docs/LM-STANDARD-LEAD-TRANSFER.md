# Run Luminary standard lead-ins through the native trampoline

Lane A. Milestone `lm-standard-lead-transfer`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

The Luminary099 SYSTEM_TEST_STANDARD_LEAD_INS entries (E/BKCALL, its three returns, E/CALL, the resume point, E/JOBWAK) and the INTPRET/INTPRET+2 and EXIT forms they reach are registered native routines. 3,072 stage cases match the certified stage functions, and 511 E/BKCALL and 384 E/CALL round trips go through the trampoline into a native callee and back (10,241 strict and sanitizer checks). 896 sampled cases match the host digests in VICE.

## Scope

Luminary099 SYSTEM_TEST_STANDARD_LEAD_INS.agc, auditor finding 4. The flight program has no E/BKCALL or E/CALL caller, so the round trips use a test caller with callees in bank 027. The operand address is BUF2 (0133) for the basic lead-ins and LOC+1 for the interpretive ones; both are erasable.

## Source map

| AGC | Native |
|---|---|
| E/BKCALL 04,2576; returns 04,2607/2610/2611; E/CALL 04,2615; resume 04,2627; E/JOBWAK 04,2631 | `standard_lead_routines` (`src/standard_lead_routines.c`) wrapping the certified `standard_lead*` stages |
| INTPRET 06037, INTPRET+2 06041 | `lm_interpreter_routines` |
| EXIT at LOC 04,2626 | `standard_lead_interpretive_routines`, fetch point `LM_INTERPRETER_FETCH` |

## Evidence

Hashes: `evidence/lm-standard-lead-transfer/SHA256.json`. Bulk files are assets of the `evidence-lm-standard-lead-transfer` release
(`python3 tools/evidence.py fetch lm-standard-lead-transfer`).

- `evidence/lm-standard-lead-transfer/lm-standard-lead-transfer-proof.json`: stage comparison and round-trip composition checks
- Host and sanitizer results: `make test-lm-standard-lead-transfer sanitize-lm-standard-lead-transfer` (`evidence/lm-standard-lead-transfer/lm-standard-lead-transfer-host.log`)
- C64 execution results: `make vice-lm-standard-lead-transfer` (`evidence/lm-standard-lead-transfer/lm-standard-lead-transfer-vice.log`)
