# Auditor review: Luminary099/DAP_INTERFACE_SUBROUTINES.agc

Reviewed 2026-10-12 by the port auditor against the certificate drafted by
milestone `lm-dap-interface` (PR #32, 9ff8eef0), at 75a6edc8 (the tip plus
the `hardware` initializer fix from PR #46).

Verdict: **accepted**.

## What was checked

- All 67 assembled words (66 source lines, bank 20, 02123-02225) have one
  native owner in `src/lm_dap_interface.c`; 2CADR 1/ACCJOB is checked
  against `src/lm_exec_sites.inc`. Constants and addresses from other files
  are literals checked against the rope by the source-map tool. No rope
  reads, decode loop or fixtures.
- `lm-dap-interface-proof.json` is a yaAGC comparison over 7168 cases
  (outcome plus all 2048 erasable words) and reproduces byte for byte.
- gcov over the compared cases: 100% of lines, 52/52 branches executed.
  Untaken directions are refusal guards with no original counterpart.
  BZF SETMINDB is taken both ways; ADS DAPBOOLS cannot carry; the TS
  overflow skip is unreachable (A comes from CAF, CAE or XCH EBANK).
- An extra auditor run with DAPBOOLS at ±0 and edge values, CDUX/Y/Z at ±0
  and extremes, and Q at 04707 (ISWRETRN), 04000, 05777, 07777, 02000,
  03777, 02122 and 02226 matched yaAGC in all 7168 cases.
- RESTORDB, SETMAXDB, SETMINDB, PFLITEDB, ZATTEROR, STOPRATE, ALLCOAST and
  the five internal return points are registered agc_routines; the
  trampoline round trip into native NOVAC runs on host and C64.

## Reproduced

- `test-` and `sanitize-lm-dap-interface`: 7168 cases (6528 returned, 640
  alarm), 14356 checks, 0 failures; 67 words on 66 lines map to 16 owners.
- `lm-dap-interface-differential`: 7168 cases equal the original program.
- `vice-lm-dap-interface` (VICE x64sc 3.7.1 PAL): 448 checks, 0 failures.

## Open notes for the lane (not blocking)

- Fold the auditor's ±0 and Q edge values into `tests/lm_dap_interface_cases.c`.
- `supported_domain`: CDUX/Y/Z are hardware counters read as image words;
  only the interrupts-inhibited entry state was compared (callers run in
  T5RUPT or under INHINT).
- Add oracle revision, native snapshot sha256 and an `equal` flag to the
  proof record; add a row to the "Applied so far" table in
  `docs/NATIVE_TRANSFER_ABI.md`.
- `tools/mk/lm-dap-interface.mk` doesn't list its dependency on
  `evidence/fixed-data-lm-original.rope.bin`.
- `p66_stoprate` in `src/p66_initialization.c` is a second native STOPRATE.
