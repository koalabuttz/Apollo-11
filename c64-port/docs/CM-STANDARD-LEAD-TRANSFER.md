# Run Comanche standard lead-ins through the native trampoline

Lane A. Milestone `cm-standard-lead-transfer`. Interface: `docs/NATIVE_TRANSFER_ABI.md`.

The Comanche055 SYSTEM_TEST_STANDARD_LEAD_INS entries and the INTPRET forms they reach are registered native routines. 3,072 stage cases match the certified stage functions, and 512 E/BKCALL and 384 E/CALL round trips go through the trampoline into a native callee and back (10,241 strict and sanitizer checks). 896 sampled cases match the host digests in VICE.

## Scope

Comanche055 SYSTEM_TEST_STANDARD_LEAD_INS.agc, the CM counterpart of `lm-standard-lead-transfer`. As in Luminary, the flight program has no E/BKCALL or E/CALL caller; the round trips use a test caller.

## Source map

| AGC | Native |
|---|---|
| E/JOBWAK 04,2606; E/BKCALL 13,2036; returns 13,2047/2050/2051; E/CALL 13,2055; resume 13,2067 | `cm_standard_lead_routines` (`src/standard_lead_routines.c`) |
| COMPVER 33,2000, GTSCPSS1 33,2001, REDO 33,2002 (one TC each; milestone `cm-lead-tests`, 384 yaAGC cases) | `cm_standard_lead_routines` |
| INTPRET 06006, INTPRET+2 06010 | `cm_interpreter_routines` |
| EXIT at LOC 13,2066 | `cm_standard_lead_interpretive_routines`, fetch point `CM_INTERPRETER_FETCH` |

## Evidence

Hashes: `evidence/cm-standard-lead-transfer/SHA256.json`. Bulk files are assets of the `evidence-cm-standard-lead-transfer` release
(`python3 tools/evidence.py fetch cm-standard-lead-transfer`).

- `evidence/cm-standard-lead-transfer/cm-standard-lead-transfer-proof.json`: stage comparison and round-trip composition checks
- Host and sanitizer results: `make test-cm-standard-lead-transfer sanitize-cm-standard-lead-transfer` (`evidence/cm-standard-lead-transfer/cm-standard-lead-transfer-host.log`)
- C64 execution results: `make vice-cm-standard-lead-transfer` (`evidence/cm-standard-lead-transfer/cm-standard-lead-transfer-vice.log`)
